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
#include <stdio.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <iostream>
#include <android/hardware/hostkeymaster/BnHostKeymasterd.h>
#include <hostkeymaster/host_keymaster_ipc.h>
#include <hostkeymaster/host_keymaster_logger.h>

using namespace std;
using namespace android;
using namespace android::hardware::hostkeymaster;
using namespace keymaster;
using ::android::sp;
using ::android::binder::Status;

int main(int argc, char *argv[])
{
	String16 serviceName(hostkeymaster_name);
	int ret;

	sp<HostKeymasterServer> HkmServer = new HostKeymasterServer();

	sp<IServiceManager> sm = defaultServiceManager();
	if (!sm.get()) {
		LOG_E("Error: Default Service Manager is NULL", 0);
		return -1;
	}

	ret = sm->addService(serviceName, HkmServer);
	if (ret != 0) {
		LOG_E("Error: Failed(%d) to add HostKeymasterd.", ret);
		return -1;
	}
	LOG_I("HostKeymasterd is added successfully.", 0);

	// Start handling binder requests with multiple threads
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();

	return 0;
}
