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
#include <android/hardware/hostkeymaster/BnHostKeymasterd.h>
#include <hostkeymaster/host_keymaster_ipc.h>
#include <hostkeymaster/host_keymaster_context.h>
#include <hostkeymaster/host_keymaster_err.h>
#include <hostkeymaster/host_keymaster_logger.h>

using namespace std;
using namespace android;
using namespace android::hardware::hostkeymaster;
using ::android::sp;
using ::android::binder::Status;

namespace keymaster {

int32_t message_version = -1;
AndroidKeymaster* keymaster_;

template <typename Response>
static int32_t serialize_response(Response& rsp, ::std::vector<uint8_t>* outbuf) {
    rsp.message_version = message_version;

	outbuf->resize(rsp.SerializedSize(), 0);
	uint8_t* data = outbuf->data();

    rsp.Serialize(data, data+outbuf->size());

    return NO_ERROR;
}
							   

/*
 * deseralize_request and serialize_request are used by the different
 * overloads of the do_dispatch template to handle the new API signatures
 * that keymaster is migrating to.
 */
template <typename Request>
static int32_t deserialize_request(const ::std::vector<uint8_t>& inbuf, Request& req) {
    req.message_version = message_version;
	const uint8_t* data = inbuf.data();

    if (!req.Deserialize(&data, data+inbuf.size()))
        return ERR_NOT_VALID;

    return NO_ERROR;
}

template <typename Keymaster, typename Request, typename Response>
static int32_t do_dispatch(void (Keymaster::*operation)(const Request&, Response*),
                        const ::std::vector<uint8_t>& inbuf,
						::std::vector<uint8_t>* outbuf) {
    status_t err;
    Request req;

    err = deserialize_request(inbuf, req);
    if (err != NO_ERROR)
        return err;

    Response rsp;
    (keymaster_->*operation)(req, &rsp);

    err = serialize_response(rsp, outbuf);
    if (err != NO_ERROR) {
        LOG_E("Error serializing response\n", 0);
        return err;
    }

    return NO_ERROR;
}


/*
 * Keymaster is migrating to new API signatures.
 * This overloaded dispatch is used for methods that accept one Request argument
 * and return a Response (e.g. COMPUTE_SHARED_HMAC_RESPONSE)
 */
template <typename Keymaster, typename Request, typename Response>
static int32_t do_dispatch(Response (Keymaster::*operation)(const Request&),
                        const ::std::vector<uint8_t>& inbuf,
						::std::vector<uint8_t>* outbuf) {
    status_t err;
    Request req;

    err = deserialize_request(inbuf, req);
    if (err != NO_ERROR)
        return err;

    Response rsp = ((keymaster_->*operation)(req));

    err = serialize_response(rsp, outbuf);
    if (err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* Keymaster is migrating to new API signatures.
 * This overloaded dispatch is used for methods that do not have arguments
 * and return a Response (e.g. GET_HMAC_SHARING_PARAMETERS)
 * */
template <typename Keymaster, typename Response>
static int32_t do_dispatch(Response (Keymaster::*operation)(),
                        const ::std::vector<uint8_t>& inbuf,
						::std::vector<uint8_t>* outbuf) {
    status_t err;
    Response rsp = ((keymaster_->*operation)());

    err = serialize_response(rsp, outbuf);
    if (err != NO_ERROR)
        return err;

    return NO_ERROR;
}

static int32_t keymaster_dispatch_cmd(
                        int32_t cmd,
						const ::std::vector<uint8_t>& inbuf,
						::std::vector<uint8_t>* outbuf) {
    switch (cmd) {
	case KM_GENERATE_KEY:
        LOG_D("Dispatching GENERATE_KEY, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::GenerateKey, inbuf, outbuf);

    case KM_BEGIN_OPERATION:
        LOG_D("Dispatching BEGIN_OPERATION, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::BeginOperation, inbuf, outbuf);

    case KM_UPDATE_OPERATION:
        LOG_D("Dispatching UPDATE_OPERATION, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::UpdateOperation, inbuf, outbuf);

    case KM_FINISH_OPERATION:
        LOG_D("Dispatching FINISH_OPERATION, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::FinishOperation, inbuf, outbuf);

    case KM_ABORT_OPERATION:
        LOG_D("Dispatching ABORT_OPERATION, size %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::AbortOperation, inbuf, outbuf);

    case KM_IMPORT_KEY:
        LOG_D("Dispatching IMPORT_KEY, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::ImportKey, inbuf, outbuf);

    case KM_EXPORT_KEY:
        LOG_D("Dispatching EXPORT_KEY, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::ExportKey, inbuf, outbuf);

    case KM_GET_VERSION:
        LOG_D("Dispatching GET_VERSION, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::GetVersion, inbuf, outbuf);

    case KM_ADD_RNG_ENTROPY:
        LOG_D("Dispatching ADD_RNG_ENTROPY, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::AddRngEntropy, inbuf, outbuf);

    case KM_GET_KEY_CHARACTERISTICS:
        LOG_D("Dispatching GET_KEY_CHARACTERISTICS, size: %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::GetKeyCharacteristics, inbuf, outbuf);

    case KM_ATTEST_KEY:
        LOG_D("Dispatching ATTEST_KEY, size %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::AttestKey, inbuf, outbuf);

    case KM_UPGRADE_KEY:
        LOG_D("Dispatching UPGRADE_KEY, size %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::UpgradeKey, inbuf, outbuf);

    case KM_CONFIGURE:
        LOG_D("Dispatching CONFIGURE, size %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::Configure, inbuf, outbuf);

    case KM_DELETE_KEY:
        LOG_D("Dispatching DELETE_KEY, size %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::DeleteKey, inbuf, outbuf);

    case KM_DELETE_ALL_KEYS:
        LOG_D("Dispatching DELETE_ALL_KEYS, size %d", inbuf.size());
        return do_dispatch(&AndroidKeymaster::DeleteAllKeys, inbuf, outbuf);

	default:
        LOG_E("Cannot dispatch unknown command %d", cmd);
        return ERR_NOT_IMPLEMENTED;
    }
}

Status HostKeymasterServer::KMCall(int32_t cmd,
		const ::std::vector<uint8_t>& inbuf,
		::std::vector<uint8_t>* outbuf,
		int32_t* _aidl_return) {
	LOG_D("receive the cmd(%d) from client.", cmd);
	*_aidl_return = keymaster_dispatch_cmd(cmd, inbuf, outbuf);

	return Status::ok();
}

HostKeymasterServer::HostKeymasterServer() {
	HostKeymasterLogger::initialize();

	LOG_I("HostKeymasterServer initializing...", 0);

	keymaster_ = new AndroidKeymaster(new HostKeymasterContext, 16);

	GetVersionRequest request;
	GetVersionResponse response;
	keymaster_->GetVersion(request, &response);
	if (response.error == KM_ERROR_OK) {
	    message_version = MessageVersion(response.major_ver, response.minor_ver,
	                                     response.subminor_ver);
	} else {
	    LOG_E("Error %d determining AndroidKeymaster version.\n", response.error);
	}
}

}
