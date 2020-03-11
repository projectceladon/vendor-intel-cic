#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <crypto_utils/android_pubkey.h>
#include <openssl/obj_mac.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>


#define VERITY_TABLE_RSA_KEY "images/verity_key"

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t verity_hashtree_size;
	uint32_t verity_table_size;
	uint32_t size_of_struct;
	uint32_t reserved;
	uint8_t signature[256];
} verity_header_t;

static void usage()
{
    fprintf(stderr, "Usage: cic_veritysetup <create> <dm_name> <data_dev> <verityimg> | <remove> <dm_name>\n");
}

static void split(char *src, const char *separator, char **dest, int *num)
{
	char *pNext;
	int count = 0;

	if (src == NULL || strlen(src) == 0)
		return;

	if (separator == NULL || strlen(separator) == 0)
		return;

	pNext = (char *)strtok(src,separator);
	while(pNext != NULL) {
		*dest++ = pNext;
		++count;
		pNext = (char *)strtok(NULL,separator);
	}
	*num = count;
}

static int cmd_system(const char* command, char *result)
{
    FILE *fp;
fprintf(stderr, "YYY--cmd is: %s\n", command);
    fp = popen(command, "r");
    if (!fp)
        return -1;

    fgets(result,1024-1,fp);
    if(result[strlen(result)-1] == '\n')
        result[strlen(result)-1] = '\0';

    if(fp)
        pclose(fp);
    return 0;
}

static char* create_loop_dev(const char* image)
{
	char cmd[1024];

	char *loop_dev = (char *)malloc(1024);
	if (!loop_dev)
		return NULL;

	cmd_system("losetup -f", loop_dev);
	fprintf(stderr, "loop=%s\n", loop_dev);

	char a_end[3] = {0};
	strncpy(a_end, loop_dev+strlen(loop_dev)-1, 2);
	int loopX = atol(a_end);

	memset(cmd, '\0', sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "mknod -m 0660 %s b 7 %d ; losetup %s -r %s", loop_dev, loopX, loop_dev, image);
	fprintf(stderr, "cmd is %s\n", cmd);
	system(cmd);

	return loop_dev;
}

static void remove_loop_dev(const char* image)
{
	char cmd[1024];
	char loop_dev[1024];

	memset(cmd, '\0', sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "losetup -a |grep %s |cut -d: -f1 | tail -n1", image);
	cmd_system(cmd, loop_dev);

	memset(cmd, '\0', sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "losetup -d %s", loop_dev);
	system(cmd);
}

static RSA *load_key(const char*path)
{
	uint8_t key_data[ANDROID_PUBKEY_ENCODED_SIZE];

	FILE * f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "failed to open %s\n", path);
		return NULL;
	}

	if (!fread(key_data, sizeof(key_data), 1, f)) {
		fprintf(stderr, "failed to read key!\n");
		fclose(f);
		return NULL;
	}

	fclose(f);
	RSA *key = NULL;
	if (!android_pubkey_decode((const uint8_t*)key_data, sizeof(key_data), &key)) {
		fprintf(stderr, "failed to parse key\n");
		return NULL;
	}

	return key;
}

static int verify_veritytable(const uint8_t *sig,
		size_t sig_size,
		const char *table,
		uint32_t table_len)
{
	RSA *key;
	uint8_t hash_buf[SHA256_DIGEST_LENGTH];
	int ret = -1;

	// Hash the table
	SHA256((uint8_t*)table, table_len, hash_buf);

	// get the pub key from the keyfile
	key = load_key(VERITY_TABLE_RSA_KEY);
	if (!key) {
		fprintf(stderr, "failed to load key from keyfile\n");
		goto out;
	}

	// verify the table
	if (!RSA_verify(NID_sha256,
		(const uint8_t*)hash_buf,
		sizeof(hash_buf),
		sig,
		sig_size,
		key)) {
		fprintf(stderr, "the signature is invalid\n");
		goto out;
	}

	ret = 0;
out:
	RSA_free(key);
	return ret;
}

static char* get_root_hash(char *verity_table)
{
	char *split_buf[16] = {0};
	int num = 0;

	split(verity_table, ",", split_buf, &num);

	return split_buf[num-2];
}

static int create_hashtree_dev(int fd, verity_header_t *vhr, char *hash_dev)
{
	char *hashtree;
	int tree_offset;
	int err = -1;
	int tmp_fd;

	hashtree = malloc(vhr->verity_hashtree_size);
	if (!hashtree) {
		fprintf(stderr, "failed to allocate the memory for hash_tree");
		goto out;
	}

	tree_offset = sizeof(verity_header_t)+vhr->verity_table_size;
	lseek(fd, tree_offset, SEEK_SET);
	if (read(fd, hashtree, vhr->verity_hashtree_size) < 0) {
		fprintf(stderr, "failed to read hashtree\n");
		goto out;
	}

	tmp_fd = open(hash_dev, O_WRONLY | O_CREAT, 0644);
	if (tmp_fd < 0) {
		fprintf(stderr, "failed to open tmp hashtree file %s\n");
		goto out;
	}
	lseek(tmp_fd, 0, SEEK_SET);
	if (write(tmp_fd, hashtree, vhr->verity_hashtree_size) < 0) {
		fprintf(stderr, "failed to write hashtree\n");
		goto out;
	}
	close(tmp_fd);

	err = 0;
out:
	if(hashtree)
		free(hashtree);
	return err;
}

static void remove_hashtree_dev(char *hash_dev)
{
	if (access(hash_dev, F_OK) != -1) {
		remove(hash_dev);
	}
}

static int get_verity_header(int fd, verity_header_t *vhr)
{
	lseek(fd, 0, SEEK_SET);
	if (read(fd, vhr, sizeof(verity_header_t)) < 0) {
		fprintf(stderr, "failed to read the verity_header\n");
		return -1;
	}

	return 0;
}

static int verity_remove_dm(char *dm_name)
{
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "veritysetup remove %s", dm_name);
	fprintf(stderr, "verity remove cmd: %s\n", cmd);

	system(cmd);
	char *hash_image = "tmp_hashtree.img";
	remove_loop_dev(hash_image);
	char *data_image = "system.img";
	remove_loop_dev(data_image);
	return 0;
}

static int verity_create_dm(char *dm_name, char *data_dev, char *verityimg)
{
	int fd;
	int ret = -1;
	char cmd[1024];
	verity_header_t *vhr = NULL;
	char *verity_table = NULL;
	int table_offset;

	/* open the verity img/device */
	fd = open(verityimg, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "failed to open the %s\n", verityimg);
		goto out;
	}

	vhr = (verity_header_t *)malloc(sizeof(verity_header_t));
	if (!vhr) {
		fprintf(stderr, "failed to allocate vhr\n");
		goto out;
	}
	/* get the verity header */
	if (get_verity_header(fd, vhr) < 0) {
		fprintf(stderr, "failed to get the verity header\n");
		goto out;
	}

	/* get the verity_table */
	verity_table = malloc(vhr->verity_table_size);
	if (!verity_table) {
		fprintf(stderr, "failed to allocate verity_table\n");
		goto out;
	}

	table_offset = sizeof(verity_header_t);
	lseek(fd, table_offset, SEEK_SET);
	if (read(fd, verity_table, vhr->verity_table_size) < 0) {
		fprintf(stderr, "failed to read the verity_table\n");
		goto out;
	}

#if 0
	/* verify the verity_table */
	if(verify_veritytable(vhr->signature, sizeof(vhr->signature),
		verity_table, vhr->verity_table_size) < 0) {
		fprintf(stderr, "failed to verify the verity_table\n");
		goto out;
	}
#endif

	/* create the tmp hash dev */
	char *tmp_hash_img = "tmp_hashtree.img";
	if (create_hashtree_dev(fd, vhr, tmp_hash_img) < 0) {
		fprintf(stderr, "failed to create the tmp device for hashtree\n");
		goto out;
	}
	char *loop_hash_dev = create_loop_dev(tmp_hash_img);
	char *loop_data_dev = create_loop_dev(data_dev);

	fprintf(stderr, "loop_hash_dev: %s\n", loop_hash_dev);
	fprintf(stderr, "loop_data_dev: %s\n", loop_data_dev);

	/* veritysetup create <dm_name> <data_dev> <hash_dev> <root_hash> */
	memset(cmd, '\0', sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "veritysetup create %s %s %s %s",
			dm_name,
			loop_data_dev,
			loop_hash_dev,
			get_root_hash(verity_table));
	fprintf(stderr, "verity create cmd is: %s\n", cmd);

	system(cmd);

	/* remove the tmp hash dev */
	//remove_hashtree_dev(tmp_hash_img);

	ret = 0;
out:
	if (vhr)
		free(vhr);
	if (verity_table)
		free(verity_table);
	if (loop_hash_dev)
		free(loop_hash_dev);
	if (loop_data_dev)
		free(loop_data_dev);

    close(fd);
	return ret;
}


#define TEST_DM_DEVICE "test-dm"
#define TEST_DATA_DEVICE "system.img"
#define TEST_VERITYIMG "verity_metadata.img"

/*
* cic_veritysetup
*  --<create> <dm_name> <dev_name> <hash_name> |
*  --<remove> <dm_name>
*/
int main(int argc, char *argv[])
{
	if (argc == 3 && (!strcmp(argv[1], "remove"))) {
		return verity_remove_dm(argv[2]);
	} else if (argc == 5 && (!strcmp(argv[1], "create"))){
		return verity_create_dm(argv[2], argv[3], argv[4]);
	} else {
        usage();
		//return verity_create_dm(TEST_DM_DEVICE, TEST_DATA_DEVICE, TEST_VERITYIMG);
        exit(-1);
    }
}
