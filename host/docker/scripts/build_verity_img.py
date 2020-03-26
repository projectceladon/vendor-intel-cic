import logging
import os
import os.path
import re
import shutil
import sys
import tempfile
import struct
import math
import subprocess

FIXED_SALT = "aee087a5be3b982978c923f566a94613496b417f2af592639bc80d141e34dfe7"
BLOCK_SIZE = 4096

VERITY_VERSION = 0x1
VERITY_MAGIC_NUMBER = 0xdeadbeef

def RunCommand(cmd, verbose=None):
  """Echo and run the given command.

  Args:
    cmd: the command represented as a list of strings.
    verbose: show commands being executed.
  Returns:
    A tuple of the output and the exit code.
  """
  if verbose:
    print("Running: " + " ".join(cmd))
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  output, _ = p.communicate()

  if verbose:
    print(output.rstrip())
  return (output, p.returncode)

def build_verity_table(input_file, output_file, prop_dict):
  # get properties
  data_dev = os.path.basename(input_file)
  hash_dev = os.path.basename(output_file)
  block_num = int(os.path.getsize(input_file)/BLOCK_SIZE)
  start_num = 0
  root_hash = prop_dict["verity_root_hash"]
  salt = prop_dict["verity_salt"]
  # build the verity table
  table = "1,%s,%s,%s,%s,%s,%s,sha256,%s,%s"
  table %= (  data_dev,
              hash_dev,
              BLOCK_SIZE,
              BLOCK_SIZE,
              block_num,
              start_num,
              root_hash,
              salt)
  return table

def sign_verity_table(table, signer_path, key_path):
  with tempfile.NamedTemporaryFile(suffix='.table') as table_file:
    with tempfile.NamedTemporaryFile(suffix='.sig') as signature_file:
      table_file.write(table)
      table_file.flush()
      cmd = [signer_path, table_file.name, key_path, signature_file.name]
      RunCommand(cmd)
      return signature_file.read()

"""typedef struct {
    int magic;
    int version;
    int verity_hashtree_size;
    int verity_table_size;
    int size_of_struct;
    int reserved;
	char signature[256];
} verity_header_t;
"""
def pack_verity_header(verity_metadata_img, verity_table, prop_dict, need_sign):
  verity_hashtree_size = int(os.path.getsize(verity_metadata_img))
  verity_table_size = len(verity_table)
  signature = '0'.ljust(256,'0')
  if need_sign:
	# sign the verity table
	signing_key = prop_dict["verity_key"]
	signer_path = prop_dict["verity_signer_cmd"]
	signature = sign_verity_table(verity_table, signer_path, signing_key)
  verity_hashtree_header = struct.pack("IIIIII256s",
      VERITY_MAGIC_NUMBER,
      VERITY_VERSION,
      verity_hashtree_size,
      verity_table_size,
      280, #size_of_struct
      0,
      signature)
  verity_hashtree_header += verity_table
  # pack the header at the begining of the verity image
  with open(verity_metadata_img,"r+") as f:
    old = f.read()
    f.seek(0)
    f.write(verity_hashtree_header)
    f.write(old)
    f.flush()

  return True

def build_hash_tree(input_img, verity_hashtree, prop_dict):
  cmd = ["veritysetup", "format", "-s", FIXED_SALT, input_img,
         verity_hashtree]
  output, exit_code = RunCommand(cmd)
  if exit_code != 0:
    print("Could not build verity tree! Error: %s" % output)
    return False
  for line in output.split("\n"):
    if (line.find("Root hash") != -1):
      root = line.split(":")[-1].strip()
      prop_dict["verity_root_hash"] = root
    if (line.find("Salt") != -1):
      salt = line.split(":")[-1].strip()
      prop_dict["verity_salt"] = salt

  return True

def main(argv):
	if len(argv) != 2:
		print("Usage: build_verity_img.py system.img verity_metadata.img")
		sys.exit(1)

	prop_dict = {
	  "verity_root_hash":None,
      "verity_salt" : None,
      "verity_key" : "verity_key.pk8",
      "verity_signer_cmd" : "verity_signer",
      "verity_fec" : False,
      "verity_disable":False,
	}

	input_file = argv[0]
	output_file = argv[1]
	print(input_file)
	print(output_file)

	# build the verity tree and get the root hash and salt
	if not build_hash_tree(input_file, output_file, prop_dict):
		return False

	# build the verity table
	verity_table = build_verity_table(input_file, output_file, prop_dict)
	print(verity_table)

	# pack the verity header
	pack_verity_header(output_file, verity_table, prop_dict, False)

if __name__ == '__main__':
	main(sys.argv[1:])
