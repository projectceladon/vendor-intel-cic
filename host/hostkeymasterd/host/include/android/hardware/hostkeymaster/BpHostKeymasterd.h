#ifndef AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_BP_HOST_KEYMASTERD_H_
#define AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_BP_HOST_KEYMASTERD_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <utils/Errors.h>
#include <android/hardware/hostkeymaster/IHostKeymasterd.h>

namespace android {

namespace hardware {

namespace hostkeymaster {

class BpHostKeymasterd : public ::android::BpInterface<IHostKeymasterd> {
public:
explicit BpHostKeymasterd(const ::android::sp<::android::IBinder>& _aidl_impl);
virtual ~BpHostKeymasterd() = default;
::android::binder::Status KMCall(int32_t cmd, const ::std::vector<uint8_t>& inbuf, ::std::vector<uint8_t>* outbuf, int32_t* _aidl_return) override;
};  // class BpHostKeymasterd

}  // namespace hostkeymaster

}  // namespace hardware

}  // namespace android

#endif  // AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_BP_HOST_KEYMASTERD_H_
