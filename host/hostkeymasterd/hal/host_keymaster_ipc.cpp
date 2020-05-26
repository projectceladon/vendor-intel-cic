/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include <stdio.h>
#include <iostream>

#include <binder/IServiceManager.h>
#include <android-base/logging.h>
#include <binder/ProcessState.h>

#include <hostkeymaster/host_keymaster_ipc.h>
#include <android/hardware/hostkeymaster/IHostKeymasterd.h>

using ::android::defaultServiceManager;
using ::android::sp;
using ::android::IServiceManager;
using ::android::ProcessState;


using ::android::IBinder;
using ::android::binder::Status;
using ::android::hardware::hostkeymaster::IHostKeymasterd;

using namespace std;
using namespace android;

namespace keymaster {

HostKeymasterClient::~HostKeymasterClient() {
    Close();
}

void HostKeymasterClient::Open() {
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder = sm->getService(String16(hostkeymaster_name));

	_hostkeymasterd = interface_cast<IHostKeymasterd>(binder);
}

void HostKeymasterClient::Close() {
    _hostkeymasterd.clear();
}

bool HostKeymasterClient::IsOpen()  {
    return _hostkeymasterd != nullptr;
}

keymaster_error_t HostKeymasterClient::host_keymaster_send(
			uint32_t command,
			const keymaster::Serializable& req,
			keymaster::KeymasterResponse* rsp) {
	uint32_t req_size = req.SerializedSize();

	if (req_size > TRUSTY_KEYMASTER_SEND_BUF_SIZE) {
		MyLog("Request too big: %d Max size: %d",
			req_size, TRUSTY_KEYMASTER_SEND_BUF_SIZE);
		return KM_ERROR_INVALID_INPUT_LENGTH;
	}

	uint8_t send_buf[TRUSTY_KEYMASTER_SEND_BUF_SIZE];
	keymaster::Eraser send_buf_eraser(send_buf, TRUSTY_KEYMASTER_SEND_BUF_SIZE);
	req.Serialize(send_buf, send_buf + req_size);

	uint8_t recv_buf[TRUSTY_KEYMASTER_RECV_BUF_SIZE];
	keymaster::Eraser recv_buf_eraser(recv_buf, TRUSTY_KEYMASTER_RECV_BUF_SIZE);
	uint32_t rsp_size = TRUSTY_KEYMASTER_RECV_BUF_SIZE;

	// Send it
	std::vector<uint8_t> request(send_buf, send_buf+req_size);;
	std::vector<uint8_t> response(recv_buf, recv_buf+rsp_size);;
	uint32_t service_ret = 0;
		
	_hostkeymasterd->KMCall(command, request, &response, reinterpret_cast<int32_t*>(&service_ret));
	if (service_ret != 0) {
		MyLog("failed to call service with cmd(%d), err+%d\n", command, service_ret);
		return KM_ERROR_UNKNOWN_ERROR;
	}

	const uint8_t* p = response.data();
	if (!rsp->Deserialize(&p, p + response.size())) {
		MyLog("Error deserializing response of size %d\n", (int)rsp_size);
		return KM_ERROR_UNKNOWN_ERROR;
	} else if (rsp->error != KM_ERROR_OK) {
		MyLog("Response of size %d contained error code %d\n", (int)rsp_size, (int)rsp->error);
		return rsp->error;
	}
	return rsp->error;
}

} // namespace keymaster

