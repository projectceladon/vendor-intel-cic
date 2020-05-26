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

#ifndef HOST_KEYMASTER_CONTEXT_H_
#define HOST_KEYMASTER_CONTEXT_H_

#include <keymaster/attestation_record.h>
#include <keymaster/keymaster_context.h>
#include <keymaster/soft_key_factory.h>

#include <keymaster/km_openssl/software_random_source.h>
#include <keymaster/contexts/pure_soft_keymaster_context.h>

namespace keymaster {

class KeyFactory;

static const int kAuthTokenKeySize = 32;
static const int kMaxCertChainLength = 3;

class HostKeymasterContext : public PureSoftKeymasterContext {
public:
	explicit HostKeymasterContext();
	~HostKeymasterContext() override {};
	// Not copyable nor assignable.
	HostKeymasterContext(const HostKeymasterContext&) = delete;
	HostKeymasterContext& operator=(const HostKeymasterContext&) = delete;

	// PureSoftKeymasterContext overrides.
	keymaster_error_t CreateKeyBlob(
			const AuthorizationSet& key_description,
			keymaster_key_origin_t origin,
			const KeymasterKeyBlob& key_material,
			KeymasterKeyBlob* key_blob,
			AuthorizationSet* hw_enforced,
			AuthorizationSet* sw_enforced) const override;

	keymaster_error_t ParseKeyBlob(
			const KeymasterKeyBlob& key_blob,
			const AuthorizationSet& additional_params,
			UniquePtr<Key>* key) const override;

	keymaster_error_t UpgradeKeyBlob(
			const KeymasterKeyBlob& key_to_upgrade,
			const AuthorizationSet& upgrade_params,
			KeymasterKeyBlob* upgraded_key) const override;

    keymaster_error_t GenerateRandom(
			uint8_t* buffer,
			size_t length) const override;

private:
	keymaster_error_t SetAuthorizations(const AuthorizationSet& key_description,
			keymaster_key_origin_t origin,
			AuthorizationSet* hw_enforced,
			AuthorizationSet* sw_enforced) const;

    keymaster_error_t BuildHiddenAuthorizations(
            const AuthorizationSet& input_set,
            AuthorizationSet* hidden) const;

    keymaster_error_t DeriveMasterKey(KeymasterKeyBlob* master_key) const;

    /*
     * CreateAuthEncryptedKeyBlob takes a key description authorization set, key
     * material, and hardware and software authorization sets and produces an
     * encrypted and integrity-checked key blob.
     *
     * This method is called by CreateKeyBlob and UpgradeKeyBlob.
     */
    keymaster_error_t CreateAuthEncryptedKeyBlob(
            const AuthorizationSet& key_description,
            const KeymasterKeyBlob& key_material,
            const AuthorizationSet& hw_enforced,
            const AuthorizationSet& sw_enforced,
            KeymasterKeyBlob* blob) const;

	Buffer verified_boot_key_;
    keymaster_verified_boot_t verified_boot_state_ =
            KM_VERIFIED_BOOT_UNVERIFIED;
    bool device_locked_ = false;

};

}  // namespace keymaster

#endif  // TRUSTY_APP_KEYMASTER_TRUSTY_KEYMASTER_CONTEXT_H_

