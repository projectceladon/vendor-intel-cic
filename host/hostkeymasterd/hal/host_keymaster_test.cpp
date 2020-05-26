/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <keymaster/keymaster_configuration.h>

#include <stdio.h>
#include <memory>

#include <openssl/evp.h>
#include <openssl/x509.h>

#include <hostkeymaster/host_keymaster_device.h>
#include <hostkeymaster/host_keymaster_ipc.h>

using keymaster::HostKeymasterClient;
using keymaster::HostKeymasterDevice;

unsigned char rsa_privkey_pk8_der[] = {
        0x30, 0x82, 0x02, 0x75, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
        0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82, 0x02, 0x5f, 0x30, 0x82, 0x02, 0x5b,
        0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xc6, 0x09, 0x54, 0x09, 0x04, 0x7d, 0x86, 0x34,
        0x81, 0x2d, 0x5a, 0x21, 0x81, 0x76, 0xe4, 0x5c, 0x41, 0xd6, 0x0a, 0x75, 0xb1, 0x39, 0x01,
        0xf2, 0x34, 0x22, 0x6c, 0xff, 0xe7, 0x76, 0x52, 0x1c, 0x5a, 0x77, 0xb9, 0xe3, 0x89, 0x41,
        0x7b, 0x71, 0xc0, 0xb6, 0xa4, 0x4d, 0x13, 0xaf, 0xe4, 0xe4, 0xa2, 0x80, 0x5d, 0x46, 0xc9,
        0xda, 0x29, 0x35, 0xad, 0xb1, 0xff, 0x0c, 0x1f, 0x24, 0xea, 0x06, 0xe6, 0x2b, 0x20, 0xd7,
        0x76, 0x43, 0x0a, 0x4d, 0x43, 0x51, 0x57, 0x23, 0x3c, 0x6f, 0x91, 0x67, 0x83, 0xc3, 0x0e,
        0x31, 0x0f, 0xcb, 0xd8, 0x9b, 0x85, 0xc2, 0xd5, 0x67, 0x71, 0x16, 0x97, 0x85, 0xac, 0x12,
        0xbc, 0xa2, 0x44, 0xab, 0xda, 0x72, 0xbf, 0xb1, 0x9f, 0xc4, 0x4d, 0x27, 0xc8, 0x1e, 0x1d,
        0x92, 0xde, 0x28, 0x4f, 0x40, 0x61, 0xed, 0xfd, 0x99, 0x28, 0x07, 0x45, 0xea, 0x6d, 0x25,
        0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x81, 0x80, 0x1b, 0xe0, 0xf0, 0x4d, 0x9c, 0xae, 0x37,
        0x18, 0x69, 0x1f, 0x03, 0x53, 0x38, 0x30, 0x8e, 0x91, 0x56, 0x4b, 0x55, 0x89, 0x9f, 0xfb,
        0x50, 0x84, 0xd2, 0x46, 0x0e, 0x66, 0x30, 0x25, 0x7e, 0x05, 0xb3, 0xce, 0xab, 0x02, 0x97,
        0x2d, 0xfa, 0xbc, 0xd6, 0xce, 0x5f, 0x6e, 0xe2, 0x58, 0x9e, 0xb6, 0x79, 0x11, 0xed, 0x0f,
        0xac, 0x16, 0xe4, 0x3a, 0x44, 0x4b, 0x8c, 0x86, 0x1e, 0x54, 0x4a, 0x05, 0x93, 0x36, 0x57,
        0x72, 0xf8, 0xba, 0xf6, 0xb2, 0x2f, 0xc9, 0xe3, 0xc5, 0xf1, 0x02, 0x4b, 0x06, 0x3a, 0xc0,
        0x80, 0xa7, 0xb2, 0x23, 0x4c, 0xf8, 0xae, 0xe8, 0xf6, 0xc4, 0x7b, 0xbf, 0x4f, 0xd3, 0xac,
        0xe7, 0x24, 0x02, 0x90, 0xbe, 0xf1, 0x6c, 0x0b, 0x3f, 0x7f, 0x3c, 0xdd, 0x64, 0xce, 0x3a,
        0xb5, 0x91, 0x2c, 0xf6, 0xe3, 0x2f, 0x39, 0xab, 0x18, 0x83, 0x58, 0xaf, 0xcc, 0xcd, 0x80,
        0x81, 0x02, 0x41, 0x00, 0xe4, 0xb4, 0x9e, 0xf5, 0x0f, 0x76, 0x5d, 0x3b, 0x24, 0xdd, 0xe0,
        0x1a, 0xce, 0xaa, 0xf1, 0x30, 0xf2, 0xc7, 0x66, 0x70, 0xa9, 0x1a, 0x61, 0xae, 0x08, 0xaf,
        0x49, 0x7b, 0x4a, 0x82, 0xbe, 0x6d, 0xee, 0x8f, 0xcd, 0xd5, 0xe3, 0xf7, 0xba, 0x1c, 0xfb,
        0x1f, 0x0c, 0x92, 0x6b, 0x88, 0xf8, 0x8c, 0x92, 0xbf, 0xab, 0x13, 0x7f, 0xba, 0x22, 0x85,
        0x22, 0x7b, 0x83, 0xc3, 0x42, 0xff, 0x7c, 0x55, 0x02, 0x41, 0x00, 0xdd, 0xab, 0xb5, 0x83,
        0x9c, 0x4c, 0x7f, 0x6b, 0xf3, 0xd4, 0x18, 0x32, 0x31, 0xf0, 0x05, 0xb3, 0x1a, 0xa5, 0x8a,
        0xff, 0xdd, 0xa5, 0xc7, 0x9e, 0x4c, 0xce, 0x21, 0x7f, 0x6b, 0xc9, 0x30, 0xdb, 0xe5, 0x63,
        0xd4, 0x80, 0x70, 0x6c, 0x24, 0xe9, 0xeb, 0xfc, 0xab, 0x28, 0xa6, 0xcd, 0xef, 0xd3, 0x24,
        0xb7, 0x7e, 0x1b, 0xf7, 0x25, 0x1b, 0x70, 0x90, 0x92, 0xc2, 0x4f, 0xf5, 0x01, 0xfd, 0x91,
        0x02, 0x40, 0x23, 0xd4, 0x34, 0x0e, 0xda, 0x34, 0x45, 0xd8, 0xcd, 0x26, 0xc1, 0x44, 0x11,
        0xda, 0x6f, 0xdc, 0xa6, 0x3c, 0x1c, 0xcd, 0x4b, 0x80, 0xa9, 0x8a, 0xd5, 0x2b, 0x78, 0xcc,
        0x8a, 0xd8, 0xbe, 0xb2, 0x84, 0x2c, 0x1d, 0x28, 0x04, 0x05, 0xbc, 0x2f, 0x6c, 0x1b, 0xea,
        0x21, 0x4a, 0x1d, 0x74, 0x2a, 0xb9, 0x96, 0xb3, 0x5b, 0x63, 0xa8, 0x2a, 0x5e, 0x47, 0x0f,
        0xa8, 0x8d, 0xbf, 0x82, 0x3c, 0xdd, 0x02, 0x40, 0x1b, 0x7b, 0x57, 0x44, 0x9a, 0xd3, 0x0d,
        0x15, 0x18, 0x24, 0x9a, 0x5f, 0x56, 0xbb, 0x98, 0x29, 0x4d, 0x4b, 0x6a, 0xc1, 0x2f, 0xfc,
        0x86, 0x94, 0x04, 0x97, 0xa5, 0xa5, 0x83, 0x7a, 0x6c, 0xf9, 0x46, 0x26, 0x2b, 0x49, 0x45,
        0x26, 0xd3, 0x28, 0xc1, 0x1e, 0x11, 0x26, 0x38, 0x0f, 0xde, 0x04, 0xc2, 0x4f, 0x91, 0x6d,
        0xec, 0x25, 0x08, 0x92, 0xdb, 0x09, 0xa6, 0xd7, 0x7c, 0xdb, 0xa3, 0x51, 0x02, 0x40, 0x77,
        0x62, 0xcd, 0x8f, 0x4d, 0x05, 0x0d, 0xa5, 0x6b, 0xd5, 0x91, 0xad, 0xb5, 0x15, 0xd2, 0x4d,
        0x7c, 0xcd, 0x32, 0xcc, 0xa0, 0xd0, 0x5f, 0x86, 0x6d, 0x58, 0x35, 0x14, 0xbd, 0x73, 0x24,
        0xd5, 0xf3, 0x36, 0x45, 0xe8, 0xed, 0x8b, 0x4a, 0x1c, 0xb3, 0xcc, 0x4a, 0x1d, 0x67, 0x98,
        0x73, 0x99, 0xf2, 0xa0, 0x9f, 0x5b, 0x3f, 0xb6, 0x8c, 0x88, 0xd5, 0xe5, 0xd9, 0x0a, 0xc3,
        0x34, 0x92, 0xd6};
unsigned int rsa_privkey_pk8_der_len = 633;

unsigned char dsa_privkey_pk8_der[] = {
        0x30, 0x82, 0x01, 0x4b, 0x02, 0x01, 0x00, 0x30, 0x82, 0x01, 0x2b, 0x06, 0x07, 0x2a, 0x86,
        0x48, 0xce, 0x38, 0x04, 0x01, 0x30, 0x82, 0x01, 0x1e, 0x02, 0x81, 0x81, 0x00, 0xa3, 0xf3,
        0xe9, 0xb6, 0x7e, 0x7d, 0x88, 0xf6, 0xb7, 0xe5, 0xf5, 0x1f, 0x3b, 0xee, 0xac, 0xd7, 0xad,
        0xbc, 0xc9, 0xd1, 0x5a, 0xf8, 0x88, 0xc4, 0xef, 0x6e, 0x3d, 0x74, 0x19, 0x74, 0xe7, 0xd8,
        0xe0, 0x26, 0x44, 0x19, 0x86, 0xaf, 0x19, 0xdb, 0x05, 0xe9, 0x3b, 0x8b, 0x58, 0x58, 0xde,
        0xe5, 0x4f, 0x48, 0x15, 0x01, 0xea, 0xe6, 0x83, 0x52, 0xd7, 0xc1, 0x21, 0xdf, 0xb9, 0xb8,
        0x07, 0x66, 0x50, 0xfb, 0x3a, 0x0c, 0xb3, 0x85, 0xee, 0xbb, 0x04, 0x5f, 0xc2, 0x6d, 0x6d,
        0x95, 0xfa, 0x11, 0x93, 0x1e, 0x59, 0x5b, 0xb1, 0x45, 0x8d, 0xe0, 0x3d, 0x73, 0xaa, 0xf2,
        0x41, 0x14, 0x51, 0x07, 0x72, 0x3d, 0xa2, 0xf7, 0x58, 0xcd, 0x11, 0xa1, 0x32, 0xcf, 0xda,
        0x42, 0xb7, 0xcc, 0x32, 0x80, 0xdb, 0x87, 0x82, 0xec, 0x42, 0xdb, 0x5a, 0x55, 0x24, 0x24,
        0xa2, 0xd1, 0x55, 0x29, 0xad, 0xeb, 0x02, 0x15, 0x00, 0xeb, 0xea, 0x17, 0xd2, 0x09, 0xb3,
        0xd7, 0x21, 0x9a, 0x21, 0x07, 0x82, 0x8f, 0xab, 0xfe, 0x88, 0x71, 0x68, 0xf7, 0xe3, 0x02,
        0x81, 0x80, 0x19, 0x1c, 0x71, 0xfd, 0xe0, 0x03, 0x0c, 0x43, 0xd9, 0x0b, 0xf6, 0xcd, 0xd6,
        0xa9, 0x70, 0xe7, 0x37, 0x86, 0x3a, 0x78, 0xe9, 0xa7, 0x47, 0xa7, 0x47, 0x06, 0x88, 0xb1,
        0xaf, 0xd7, 0xf3, 0xf1, 0xa1, 0xd7, 0x00, 0x61, 0x28, 0x88, 0x31, 0x48, 0x60, 0xd8, 0x11,
        0xef, 0xa5, 0x24, 0x1a, 0x81, 0xc4, 0x2a, 0xe2, 0xea, 0x0e, 0x36, 0xd2, 0xd2, 0x05, 0x84,
        0x37, 0xcf, 0x32, 0x7d, 0x09, 0xe6, 0x0f, 0x8b, 0x0c, 0xc8, 0xc2, 0xa4, 0xb1, 0xdc, 0x80,
        0xca, 0x68, 0xdf, 0xaf, 0xd2, 0x90, 0xc0, 0x37, 0x58, 0x54, 0x36, 0x8f, 0x49, 0xb8, 0x62,
        0x75, 0x8b, 0x48, 0x47, 0xc0, 0xbe, 0xf7, 0x9a, 0x92, 0xa6, 0x68, 0x05, 0xda, 0x9d, 0xaf,
        0x72, 0x9a, 0x67, 0xb3, 0xb4, 0x14, 0x03, 0xae, 0x4f, 0x4c, 0x76, 0xb9, 0xd8, 0x64, 0x0a,
        0xba, 0x3b, 0xa8, 0x00, 0x60, 0x4d, 0xae, 0x81, 0xc3, 0xc5, 0x04, 0x17, 0x02, 0x15, 0x00,
        0x81, 0x9d, 0xfd, 0x53, 0x0c, 0xc1, 0x8f, 0xbe, 0x8b, 0xea, 0x00, 0x26, 0x19, 0x29, 0x33,
        0x91, 0x84, 0xbe, 0xad, 0x81};
unsigned int dsa_privkey_pk8_der_len = 335;

unsigned char ec_privkey_pk8_der[] = {
        0x30, 0x81, 0x87, 0x02, 0x01, 0x00, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce,
        0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x04,
        0x6d, 0x30, 0x6b, 0x02, 0x01, 0x01, 0x04, 0x20, 0x73, 0x7c, 0x2e, 0xcd, 0x7b, 0x8d,
        0x19, 0x40, 0xbf, 0x29, 0x30, 0xaa, 0x9b, 0x4e, 0xd3, 0xff, 0x94, 0x1e, 0xed, 0x09,
        0x36, 0x6b, 0xc0, 0x32, 0x99, 0x98, 0x64, 0x81, 0xf3, 0xa4, 0xd8, 0x59, 0xa1, 0x44,
        0x03, 0x42, 0x00, 0x04, 0xbf, 0x85, 0xd7, 0x72, 0x0d, 0x07, 0xc2, 0x54, 0x61, 0x68,
        0x3b, 0xc6, 0x48, 0xb4, 0x77, 0x8a, 0x9a, 0x14, 0xdd, 0x8a, 0x02, 0x4e, 0x3b, 0xdd,
        0x8c, 0x7d, 0xdd, 0x9a, 0xb2, 0xb5, 0x28, 0xbb, 0xc7, 0xaa, 0x1b, 0x51, 0xf1, 0x4e,
        0xbb, 0xbb, 0x0b, 0xd0, 0xce, 0x21, 0xbc, 0xc4, 0x1c, 0x6e, 0xb0, 0x00, 0x83, 0xcf,
        0x33, 0x76, 0xd1, 0x1f, 0xd4, 0x49, 0x49, 0xe0, 0xb2, 0x18, 0x3b, 0xfe};
unsigned int ec_privkey_pk8_der_len = 138;

keymaster_key_param_t ec_params[] = {
        keymaster_param_enum(KM_TAG_ALGORITHM, KM_ALGORITHM_EC),
        keymaster_param_long(KM_TAG_EC_CURVE, KM_EC_CURVE_P_256),
        keymaster_param_enum(KM_TAG_PURPOSE, KM_PURPOSE_SIGN),
        keymaster_param_enum(KM_TAG_PURPOSE, KM_PURPOSE_VERIFY),
        keymaster_param_enum(KM_TAG_DIGEST, KM_DIGEST_NONE),
        keymaster_param_bool(KM_TAG_NO_AUTH_REQUIRED),
};
keymaster_key_param_set_t ec_param_set = {ec_params, sizeof(ec_params) / sizeof(*ec_params)};

keymaster_key_param_t rsa_params[] = {
        keymaster_param_enum(KM_TAG_ALGORITHM, KM_ALGORITHM_RSA),
        keymaster_param_int(KM_TAG_KEY_SIZE, 1024),
        keymaster_param_long(KM_TAG_RSA_PUBLIC_EXPONENT, 65537),
        keymaster_param_enum(KM_TAG_PURPOSE, KM_PURPOSE_SIGN),
        keymaster_param_enum(KM_TAG_PURPOSE, KM_PURPOSE_VERIFY),
        keymaster_param_enum(KM_TAG_PADDING, KM_PAD_NONE),
        keymaster_param_enum(KM_TAG_DIGEST, KM_DIGEST_NONE),
        keymaster_param_bool(KM_TAG_NO_AUTH_REQUIRED),
};
keymaster_key_param_set_t rsa_param_set = {rsa_params, sizeof(rsa_params) / sizeof(*rsa_params)};

struct EVP_PKEY_Delete {
    void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); }
};

struct EVP_PKEY_CTX_Delete {
    void operator()(EVP_PKEY_CTX* p) { EVP_PKEY_CTX_free(p); }
};

static bool do_operation(HostKeymasterDevice* device, keymaster_purpose_t purpose,
                         keymaster_key_blob_t* key, keymaster_blob_t* input,
                         keymaster_blob_t* signature, keymaster_blob_t* output) {
    keymaster_key_param_t params[] = {
            keymaster_param_enum(KM_TAG_PADDING, KM_PAD_NONE),
            keymaster_param_enum(KM_TAG_DIGEST, KM_DIGEST_NONE),
    };
    keymaster_key_param_set_t param_set = {params, sizeof(params) / sizeof(*params)};
    keymaster_operation_handle_t op_handle;
    keymaster_error_t error = device->begin(purpose, key, &param_set, nullptr, &op_handle);
    if (error != KM_ERROR_OK) {
        printf("Keymaster begin() failed: %d\n", error);
        return false;
    }
    size_t input_consumed;
    error = device->update(op_handle, nullptr, input, &input_consumed, nullptr, nullptr);
    if (error != KM_ERROR_OK) {
        printf("Keymaster update() failed: %d\n", error);
        return false;
    }
    if (input_consumed != input->data_length) {
        // This should never happen. If it does, it's a bug in the keymaster implementation.
        printf("Keymaster update() did not consume all data.\n");
        device->abort(op_handle);
        return false;
    }
    error = device->finish(op_handle, nullptr, nullptr, signature, nullptr, output);
    if (error != KM_ERROR_OK) {
        printf("Keymaster finish() failed: %d\n", error);
        return false;
    }
    return true;
}

static bool test_import_rsa(HostKeymasterDevice* device) {
    printf("===================\n");
    printf("= RSA Import Test =\n");
    printf("===================\n\n");

    printf("=== Importing RSA keypair === \n");
    keymaster_key_blob_t key;
    keymaster_blob_t private_key = {rsa_privkey_pk8_der, rsa_privkey_pk8_der_len};
    int error =
            device->import_key(&rsa_param_set, KM_KEY_FORMAT_PKCS8, &private_key, &key, nullptr);
    if (error != KM_ERROR_OK) {
        printf("Error importing RSA key: %d\n\n", error);
        return false;
    }
    std::unique_ptr<const uint8_t[]> key_deleter(key.key_material);

    printf("=== Signing with imported RSA key ===\n");
    size_t message_len = 1024 / 8;
    std::unique_ptr<uint8_t[]> message(new uint8_t[message_len]);
    memset(message.get(), 'a', message_len);
    keymaster_blob_t input = {message.get(), message_len}, signature;

    if (!do_operation(device, KM_PURPOSE_SIGN, &key, &input, nullptr, &signature)) {
        printf("Error signing data with imported RSA key\n\n");
        return false;
    }
    std::unique_ptr<const uint8_t[]> signature_deleter(signature.data);

    printf("=== Verifying with imported RSA key === \n");
    if (!do_operation(device, KM_PURPOSE_VERIFY, &key, &input, &signature, nullptr)) {
        printf("Error verifying data with imported RSA key\n\n");
        return false;
    }

    printf("\n");
    return true;
}

static bool test_rsa(HostKeymasterDevice* device) {
    printf("============\n");
    printf("= RSA Test =\n");
    printf("============\n\n");

    printf("=== Generating RSA key pair ===\n");
    keymaster_key_blob_t key;
    int error = device->generate_key(&rsa_param_set, &key, nullptr);
    if (error != KM_ERROR_OK) {
        printf("Error generating RSA key pair: %d\n\n", error);
        return false;
    }
    std::unique_ptr<const uint8_t[]> key_deleter(key.key_material);

    printf("=== Signing with RSA key === \n");
    size_t message_len = 1024 / 8;
    std::unique_ptr<uint8_t[]> message(new uint8_t[message_len]);
    memset(message.get(), 'a', message_len);
    keymaster_blob_t input = {message.get(), message_len}, signature;

    if (!do_operation(device, KM_PURPOSE_SIGN, &key, &input, nullptr, &signature)) {
        printf("Error signing data with RSA key\n\n");
        return false;
    }
    std::unique_ptr<const uint8_t[]> signature_deleter(signature.data);

    printf("=== Verifying with RSA key === \n");
    if (!do_operation(device, KM_PURPOSE_VERIFY, &key, &input, &signature, nullptr)) {
        printf("Error verifying data with RSA key\n\n");
        return false;
    }

    printf("=== Exporting RSA public key ===\n");
    keymaster_blob_t exported_key;
    error = device->export_key(KM_KEY_FORMAT_X509, &key, nullptr, nullptr, &exported_key);
    if (error != KM_ERROR_OK) {
        printf("Error exporting RSA public key: %d\n\n", error);
        return false;
    }

    printf("=== Verifying with exported key ===\n");
    const uint8_t* tmp = exported_key.data;
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Delete> pkey(
            d2i_PUBKEY(NULL, &tmp, exported_key.data_length));
    std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_Delete> ctx(EVP_PKEY_CTX_new(pkey.get(), NULL));
    if (EVP_PKEY_verify_init(ctx.get()) != 1) {
        printf("Error initializing openss EVP context\n\n");
        return false;
    }
    if (EVP_PKEY_type(pkey->type) != EVP_PKEY_RSA) {
        printf("Exported key was the wrong type?!?\n\n");
        return false;
    }

    EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_NO_PADDING);
    if (EVP_PKEY_verify(ctx.get(), signature.data, signature.data_length, message.get(),
                        message_len) != 1) {
        printf("Verification with exported pubkey failed.\n\n");
        return false;
    } else {
        printf("Verification succeeded\n");
    }

    printf("\n");
    return true;
}

static bool test_import_ecdsa(HostKeymasterDevice* device) {
    printf("=====================\n");
    printf("= ECDSA Import Test =\n");
    printf("=====================\n\n");

    printf("=== Importing ECDSA keypair === \n");
    keymaster_key_blob_t key;
    keymaster_blob_t private_key = {ec_privkey_pk8_der, ec_privkey_pk8_der_len};
    int error = device->import_key(&ec_param_set, KM_KEY_FORMAT_PKCS8, &private_key, &key, nullptr);
    if (error != KM_ERROR_OK) {
        printf("Error importing ECDSA key: %d\n\n", error);
        return false;
    }
    std::unique_ptr<const uint8_t[]> deleter(key.key_material);

    printf("=== Signing with imported ECDSA key ===\n");
    size_t message_len = 30 /* arbitrary */;
    std::unique_ptr<uint8_t[]> message(new uint8_t[message_len]);
    memset(message.get(), 'a', message_len);
    keymaster_blob_t input = {message.get(), message_len}, signature;

    if (!do_operation(device, KM_PURPOSE_SIGN, &key, &input, nullptr, &signature)) {
        printf("Error signing data with imported ECDSA key\n\n");
        return false;
    }
    std::unique_ptr<const uint8_t[]> signature_deleter(signature.data);

    printf("=== Verifying with imported ECDSA key === \n");
    if (!do_operation(device, KM_PURPOSE_VERIFY, &key, &input, &signature, nullptr)) {
        printf("Error verifying data with imported ECDSA key\n\n");
        return false;
    }

    printf("\n");
    return true;
}

static bool test_ecdsa(HostKeymasterDevice* device) {
    printf("==============\n");
    printf("= ECDSA Test =\n");
    printf("==============\n\n");

    printf("=== Generating ECDSA key pair ===\n");
    keymaster_key_blob_t key;
    int error = device->generate_key(&ec_param_set, &key, nullptr);
    if (error != KM_ERROR_OK) {
        printf("Error generating ECDSA key pair: %d\n\n", error);
        return false;
    }
    std::unique_ptr<const uint8_t[]> key_deleter(key.key_material);

    printf("=== Signing with ECDSA key === \n");
    size_t message_len = 30 /* arbitrary */;
    std::unique_ptr<uint8_t[]> message(new uint8_t[message_len]);
    memset(message.get(), 'a', message_len);
    keymaster_blob_t input = {message.get(), message_len}, signature;

    if (!do_operation(device, KM_PURPOSE_SIGN, &key, &input, nullptr, &signature)) {
        printf("Error signing data with ECDSA key\n\n");
        return false;
    }
    std::unique_ptr<const uint8_t[]> signature_deleter(signature.data);

    printf("=== Verifying with ECDSA key === \n");
    if (!do_operation(device, KM_PURPOSE_VERIFY, &key, &input, &signature, nullptr)) {
        printf("Error verifying data with ECDSA key\n\n");
        return false;
    }

    printf("=== Exporting ECDSA public key ===\n");
    keymaster_blob_t exported_key;
    error = device->export_key(KM_KEY_FORMAT_X509, &key, nullptr, nullptr, &exported_key);
    if (error != KM_ERROR_OK) {
        printf("Error exporting ECDSA public key: %d\n\n", error);
        return false;
    }

    printf("=== Verifying with exported key ===\n");
    const uint8_t* tmp = exported_key.data;
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Delete> pkey(
            d2i_PUBKEY(NULL, &tmp, exported_key.data_length));
    std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_Delete> ctx(EVP_PKEY_CTX_new(pkey.get(), NULL));
    if (EVP_PKEY_verify_init(ctx.get()) != 1) {
        printf("Error initializing openssl EVP context\n\n");
        return false;
    }
    if (EVP_PKEY_type(pkey->type) != EVP_PKEY_EC) {
        printf("Exported key was the wrong type?!?\n\n");
        return false;
    }

    if (EVP_PKEY_verify(ctx.get(), signature.data, signature.data_length, message.get(),
                        message_len) != 1) {
        printf("Verification with exported pubkey failed.\n\n");
        return false;
    } else {
        printf("Verification succeeded\n");
    }

    printf("\n");
    return true;
}

int main(void) {
    // Connect to citadeld
    HostKeymasterClient hkm_client;
    HostKeymasterDevice device(NULL, &hkm_client);

    keymaster::ConfigureDevice(reinterpret_cast<keymaster2_device_t*>(&device));
    if (device.session_error() != KM_ERROR_OK) {
        printf("Failed to initialize hostkeymaster session: %d\n", device.session_error());
        return 1;
    }
    printf("hostkeymaster session initialized\n");

    bool success = true;
    success &= test_rsa(&device);
    success &= test_import_rsa(&device);
    success &= test_ecdsa(&device);
    success &= test_import_ecdsa(&device);

    if (success) {
        printf("\nTESTS PASSED!\n");
    } else {
        printf("\n!!!!TESTS FAILED!!!\n");
    }

    return success ? 0 : 1;
}
