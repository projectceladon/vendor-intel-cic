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
 *
 */
#ifndef HOST_KEYMASTER_IPC_SERVER_H
#define HOST_KEYMASTER_IPC_SERVER_H

#include <android/hardware/hostkeymaster/BnHostKeymasterd.h>
#include <android/hardware/hostkeymaster/IHostKeymasterd.h>
#include <hostkeymaster/host_keymaster_context.h>

#include <keymaster/android_keymaster_messages.h>
#include <keymaster/android_keymaster.h>

#include <cstdint>
#include <vector>

using namespace std;
using namespace android;
using namespace android::hardware::hostkeymaster;
using ::android::sp;
using ::android::binder::Status;

namespace keymaster {

const char hostkeymaster_name[] = "HostKeymasterd";

const uint32_t TRUSTY_KEYMASTER_RECV_BUF_SIZE = 2 * 4096;
const uint32_t TRUSTY_KEYMASTER_SEND_BUF_SIZE = 4096;

constexpr size_t kOperationTableSize = 16;


// Commands
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


class HostKeymasterServer : public BnHostKeymasterd {
public:
	explicit HostKeymasterServer();
	~HostKeymasterServer() = default;

	Status KMCall(int32_t cmd, const ::std::vector<uint8_t>& inbuf,
					::std::vector<uint8_t>* outbuf,
					int32_t* _aidl_return);

};

}

#endif
