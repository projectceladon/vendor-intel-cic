/*
 * Copyright 2020 The Android Open Source Project
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

#include <hostkeymaster/host_keymaster_context.h>
#include <keymaster/contexts/pure_soft_keymaster_context.h>

#include <keymaster/android_keymaster_utils.h>
#include <keymaster/km_openssl/ec_key_factory.h>
#include <keymaster/km_openssl/rsa_key_factory.h>

#include <keymaster/key_blob_utils/auth_encrypted_key_blob.h>
#include <keymaster/key_blob_utils/ocb_utils.h>
#include <keymaster/km_openssl/aes_key.h>
#include <keymaster/km_openssl/asymmetric_key.h>
#include <keymaster/km_openssl/attestation_utils.h>
#include <keymaster/km_openssl/hmac_key.h>
#include <keymaster/km_openssl/openssl_err.h>
#include <keymaster/km_openssl/software_random_source.h>
#include <openssl/rand.h>
#include <openssl/hkdf.h>

#include <hostkeymaster/host_keymaster_logger.h>

using  ::keymaster::SoftwareRandomSource;

namespace keymaster {

static const int kAesKeySize = 16;
static const uint8_t kMasterInfoData[kAesKeySize] = "KeymasterMaster";
static const uint8_t kMasterSalt[kAesKeySize] = "Salt";

bool UpgradeIntegerTag(keymaster_tag_t tag,
                       uint32_t value,
                       AuthorizationSet* set,
                       bool* set_changed) {
    int index = set->find(tag);
    if (index == -1) {
        keymaster_key_param_t param;
        param.tag = tag;
        param.integer = value;
        set->push_back(param);
        *set_changed = true;
        return true;
    }

    if (set->params[index].integer > value) {
        return false;
    }

    if (set->params[index].integer != value) {
        set->params[index].integer = value;
        *set_changed = true;
    }
    return true;
}

static keymaster_error_t TranslateAuthorizationSetError(
        AuthorizationSet::Error err) {
    switch (err) {
    case AuthorizationSet::OK:
        return KM_ERROR_OK;
    case AuthorizationSet::ALLOCATION_FAILURE:
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    case AuthorizationSet::MALFORMED_DATA:
        return KM_ERROR_UNKNOWN_ERROR;
    }
    return KM_ERROR_OK;
}

HostKeymasterContext::HostKeymasterContext() {
    LOG_I("Creating HostKeymasterContext...", 0);
    verified_boot_key_.Reinitialize("Unbound", 7);
}


keymaster_error_t HostKeymasterContext::GenerateRandom(
		uint8_t* buffer, size_t length) const {
    if (RAND_bytes(buffer, length) != 1)
        return KM_ERROR_UNKNOWN_ERROR;
    return KM_ERROR_OK;
}

keymaster_error_t HostKeymasterContext::SetAuthorizations(
        const AuthorizationSet& key_description,
        keymaster_key_origin_t origin,
        AuthorizationSet* hw_enforced,
        AuthorizationSet* sw_enforced) const {
    sw_enforced->Clear();
    hw_enforced->Clear();

    for (auto& entry : key_description) {
        switch (entry.tag) {
        case KM_TAG_INVALID:
        case KM_TAG_BOOTLOADER_ONLY:
        case KM_TAG_NONCE:
        case KM_TAG_AUTH_TOKEN:
        case KM_TAG_MAC_LENGTH:
        case KM_TAG_ASSOCIATED_DATA:
        case KM_TAG_UNIQUE_ID:
            return KM_ERROR_INVALID_KEY_BLOB;

        case KM_TAG_ROLLBACK_RESISTANT:
        case KM_TAG_APPLICATION_ID:
        case KM_TAG_APPLICATION_DATA:
        case KM_TAG_ALL_APPLICATIONS:
        case KM_TAG_ROOT_OF_TRUST:
        case KM_TAG_ORIGIN:
        case KM_TAG_RESET_SINCE_ID_ROTATION:
        case KM_TAG_ALLOW_WHILE_ON_BODY:
        case KM_TAG_ATTESTATION_CHALLENGE:
        case KM_TAG_OS_VERSION:
        case KM_TAG_OS_PATCHLEVEL:
            // Ignore these.
            break;

        case KM_TAG_PURPOSE:
        case KM_TAG_ALGORITHM:
        case KM_TAG_KEY_SIZE:
        case KM_TAG_RSA_PUBLIC_EXPONENT:
        case KM_TAG_BLOB_USAGE_REQUIREMENTS:
        case KM_TAG_DIGEST:
        case KM_TAG_PADDING:
        case KM_TAG_BLOCK_MODE:
        case KM_TAG_MIN_SECONDS_BETWEEN_OPS:
        case KM_TAG_MAX_USES_PER_BOOT:
        case KM_TAG_USER_SECURE_ID:
        case KM_TAG_NO_AUTH_REQUIRED:
        case KM_TAG_AUTH_TIMEOUT:
        case KM_TAG_CALLER_NONCE:
        case KM_TAG_MIN_MAC_LENGTH:
        case KM_TAG_KDF:
        case KM_TAG_EC_CURVE:
        case KM_TAG_ECIES_SINGLE_HASH_MODE:
            hw_enforced->push_back(entry);
            break;

        case KM_TAG_USER_AUTH_TYPE:
            if (entry.enumerated == HW_AUTH_PASSWORD)
                hw_enforced->push_back(entry);
            else
                sw_enforced->push_back(entry);
            break;

        case KM_TAG_ACTIVE_DATETIME:
        case KM_TAG_ORIGINATION_EXPIRE_DATETIME:
        case KM_TAG_USAGE_EXPIRE_DATETIME:
        case KM_TAG_USER_ID:
        case KM_TAG_ALL_USERS:
        case KM_TAG_CREATION_DATETIME:
        case KM_TAG_INCLUDE_UNIQUE_ID:
        case KM_TAG_EXPORTABLE:

            sw_enforced->push_back(entry);
            break;
        default:
            break;
        }
    }

    hw_enforced->push_back(TAG_ORIGIN, origin);
    hw_enforced->push_back(TAG_OS_VERSION, os_version_);
    hw_enforced->push_back(TAG_OS_PATCHLEVEL, os_patchlevel_);

    if (sw_enforced->is_valid() != AuthorizationSet::OK)
        return TranslateAuthorizationSetError(sw_enforced->is_valid());
    if (hw_enforced->is_valid() != AuthorizationSet::OK)
        return TranslateAuthorizationSetError(hw_enforced->is_valid());
    return KM_ERROR_OK;
}

keymaster_error_t HostKeymasterContext::DeriveMasterKey(
        KeymasterKeyBlob* master_key) const {
    LOG_D("Deriving master key", 0);

	uint8_t device_seed[32];

	/* hardcode as 1*32 for temporary, will change the real
	* device seed based on different solutions */
	memset(device_seed, 1, 32);

    if (!master_key->Reset(kAesKeySize)) {
        LOG_E("Could not allocate memory for master key buffer", 0);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }
	memset(master_key->writable_data(), 0, kAesKeySize);

	if (!HKDF(master_key->writable_data(),
			kAesKeySize,
			EVP_sha256(),
			(const uint8_t*)device_seed,
			sizeof(device_seed),
			kMasterSalt,
			sizeof(kMasterSalt),
			kMasterInfoData,
			sizeof(kMasterInfoData))) {
		LOG_E(" DeriveMasterKey with HDKF failed 0x%x.\n", ERR_get_error());
	}

    return KM_ERROR_OK;
}

keymaster_error_t HostKeymasterContext::BuildHiddenAuthorizations(
        const AuthorizationSet& input_set,
        AuthorizationSet* hidden) const {
    keymaster_blob_t entry;
    if (input_set.GetTagValue(TAG_APPLICATION_ID, &entry))
        hidden->push_back(TAG_APPLICATION_ID, entry.data, entry.data_length);
    if (input_set.GetTagValue(TAG_APPLICATION_DATA, &entry))
        hidden->push_back(TAG_APPLICATION_DATA, entry.data, entry.data_length);

    // Copy verified boot key, verified boot state, and device lock state to
    // hidden authorization set for binding to key.
    keymaster_key_param_t root_of_trust;
    root_of_trust.tag = KM_TAG_ROOT_OF_TRUST;
    root_of_trust.blob.data = verified_boot_key_.begin();
    root_of_trust.blob.data_length = verified_boot_key_.buffer_size();
    hidden->push_back(root_of_trust);

    root_of_trust.blob.data =
            reinterpret_cast<const uint8_t*>(&verified_boot_state_);
    root_of_trust.blob.data_length = sizeof(verified_boot_state_);
    hidden->push_back(root_of_trust);

    root_of_trust.blob.data = reinterpret_cast<const uint8_t*>(&device_locked_);
    root_of_trust.blob.data_length = sizeof(device_locked_);
    hidden->push_back(root_of_trust);

    return TranslateAuthorizationSetError(hidden->is_valid());
}

keymaster_error_t HostKeymasterContext::CreateAuthEncryptedKeyBlob(
        const AuthorizationSet& key_description,
        const KeymasterKeyBlob& key_material,
        const AuthorizationSet& hw_enforced,
        const AuthorizationSet& sw_enforced,
        KeymasterKeyBlob* blob) const {
    AuthorizationSet hidden;
    keymaster_error_t error =
            BuildHiddenAuthorizations(key_description, &hidden);
    if (error != KM_ERROR_OK)
        return error;

    KeymasterKeyBlob master_key;
    error = DeriveMasterKey(&master_key);
    if (error != KM_ERROR_OK)
        return error;

    Buffer nonce(OCB_NONCE_LENGTH);
    Buffer tag(OCB_TAG_LENGTH);
    if (!nonce.peek_write() || !tag.peek_write())
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    error = GenerateRandom(nonce.peek_write(), OCB_NONCE_LENGTH);
    if (error != KM_ERROR_OK)
        return error;
    nonce.advance_write(OCB_NONCE_LENGTH);

    KeymasterKeyBlob encrypted_key;
    error = OcbEncryptKey(hw_enforced, sw_enforced, hidden, master_key,
                          key_material, nonce, &encrypted_key, &tag);
    if (error != KM_ERROR_OK)
        return error;

    return SerializeAuthEncryptedBlob(encrypted_key, hw_enforced, sw_enforced,
                                      nonce, tag, blob);
}

keymaster_error_t HostKeymasterContext::CreateKeyBlob(
        const AuthorizationSet& key_description,
        keymaster_key_origin_t origin,
        const KeymasterKeyBlob& key_material,
        KeymasterKeyBlob* blob,
        AuthorizationSet* hw_enforced,
        AuthorizationSet* sw_enforced) const {
    keymaster_error_t error = SetAuthorizations(key_description, origin,
                                                hw_enforced, sw_enforced);
    if (error != KM_ERROR_OK)
        return error;

    return CreateAuthEncryptedKeyBlob(key_description, key_material,
                                      *hw_enforced, *sw_enforced, blob);
}

keymaster_error_t HostKeymasterContext::ParseKeyBlob(
        const KeymasterKeyBlob& blob,
        const AuthorizationSet& additional_params,
        UniquePtr<Key>* key) const {
    AuthorizationSet hw_enforced;
    AuthorizationSet sw_enforced;
    KeymasterKeyBlob key_material;
    keymaster_error_t error;

    auto constructKey = [&, this]() mutable -> keymaster_error_t {
        // GetKeyFactory
        if (error != KM_ERROR_OK)
            return error;
        keymaster_algorithm_t algorithm;
        if (!hw_enforced.GetTagValue(TAG_ALGORITHM, &algorithm)) {
            return KM_ERROR_INVALID_ARGUMENT;
        }
        auto factory = GetKeyFactory(algorithm);
        return factory->LoadKey(move(key_material), additional_params,
                                move(hw_enforced), move(sw_enforced), key);
    };

    Buffer nonce, tag;
    KeymasterKeyBlob encrypted_key_material;
    if (!key)
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    error = DeserializeAuthEncryptedBlob(blob, &encrypted_key_material,
                                         &hw_enforced, &sw_enforced, &nonce,
                                         &tag);
    if (error != KM_ERROR_OK)
        return error;

    if (nonce.available_read() != OCB_NONCE_LENGTH ||
        tag.available_read() != OCB_TAG_LENGTH)
        return KM_ERROR_INVALID_KEY_BLOB;

    KeymasterKeyBlob master_key;
    error = DeriveMasterKey(&master_key);
    if (error != KM_ERROR_OK)
        return error;

    AuthorizationSet hidden;
    error = BuildHiddenAuthorizations(additional_params, &hidden);
    if (error != KM_ERROR_OK)
        return error;

    error = OcbDecryptKey(hw_enforced, sw_enforced, hidden, master_key,
                          encrypted_key_material, nonce, tag, &key_material);
    return constructKey();
}


keymaster_error_t HostKeymasterContext::UpgradeKeyBlob(
        const KeymasterKeyBlob& key_to_upgrade,
        const AuthorizationSet& upgrade_params,
        KeymasterKeyBlob* upgraded_key) const {
    UniquePtr<Key> key;
    keymaster_error_t error =
            ParseKeyBlob(key_to_upgrade, upgrade_params, &key);
    if (error != KM_ERROR_OK)
        return error;

    bool set_changed = false;

    if (os_version_ == 0) {
        // We need to allow "upgrading" OS version to zero, to support upgrading
        // from proper numbered releases to unnumbered development and preview
        // releases.

        int key_os_version_pos = key->sw_enforced().find(TAG_OS_VERSION);
        if (key_os_version_pos != -1) {
            uint32_t key_os_version =
                    key->sw_enforced()[key_os_version_pos].integer;
            if (key_os_version != 0) {
                key->sw_enforced()[key_os_version_pos].integer =
                        os_version_;
                set_changed = true;
            }
        }
    }

    if (!UpgradeIntegerTag(TAG_OS_VERSION, os_version_,
                           &key->hw_enforced(), &set_changed) ||
        !UpgradeIntegerTag(TAG_OS_PATCHLEVEL, os_patchlevel_,
                           &key->hw_enforced(), &set_changed)) {
        // One of the version fields would have been a downgrade. Not allowed.
        return KM_ERROR_INVALID_ARGUMENT;
    }

    if (!set_changed) {
        // Don't need an upgrade.
        return KM_ERROR_OK;
    }

    return CreateAuthEncryptedKeyBlob(upgrade_params, key->key_material(),
                                      key->hw_enforced(), key->sw_enforced(),
                                      upgraded_key);
}

}  // namespace keymaster
