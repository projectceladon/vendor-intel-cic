/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef HOST_KEYMASTER_IPC_CLIENT_H
#define HOST_KEYMASTER_IPC_CLIENT_H

#include <cstdint>
#include <vector>

#include <android/hardware/hostkeymaster/IHostKeymasterd.h>
#include <keymaster/android_keymaster_messages.h>

namespace keymaster {


#define MyLog(fmt, ...) \
    fprintf(stderr, fmt, ## __VA_ARGS__); fprintf(stderr, "\n");


using ::android::hardware::hostkeymaster::IHostKeymasterd;

const char hostkeymaster_name[] = "HostKeymasterd";

const uint32_t TRUSTY_KEYMASTER_RECV_BUF_SIZE = 2 * PAGE_SIZE;
const uint32_t TRUSTY_KEYMASTER_SEND_BUF_SIZE = PAGE_SIZE;


enum keymaster_command {
    KM_GENERATE_KEY = 0,
    KM_BEGIN_OPERATION = 1,
    KM_UPDATE_OPERATION = 2,
    KM_FINISH_OPERATION = 3,
    KM_ABORT_OPERATION = 4,
    KM_IMPORT_KEY = 5,
    KM_EXPORT_KEY = 6,
    KM_GET_VERSION = 7,
    KM_ADD_RNG_ENTROPY = 8,
    KM_GET_KEY_CHARACTERISTICS = 9,
    KM_ATTEST_KEY = 10,
    KM_UPGRADE_KEY = 11,
    KM_CONFIGURE = 12,
    KM_DELETE_KEY = 13,
    KM_DELETE_ALL_KEYS = 14,
};

class HostKeymasterClient {
    ::android::sp<IHostKeymasterd> _hostkeymasterd;

public:
	HostKeymasterClient() = default;
	~HostKeymasterClient();

	 /**
     * Opens a connection to the default Nugget device.
     *
     * If this fails, isOpen() will return false.
     */
    void Open();

    /**
     * Closes the connection to Nugget.
     */
    void Close();

    /**
     * Checked whether a connection is open to Nugget.
     */
    bool IsOpen();

	keymaster_error_t host_keymaster_send(
			uint32_t command,
			const keymaster::Serializable& req,
			keymaster::KeymasterResponse* rsp);
};

} // namespace keymaster

#endif
