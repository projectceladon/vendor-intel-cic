#ifndef AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_BN_HOST_KEYMASTERD_H_
#define AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_BN_HOST_KEYMASTERD_H_

#include <binder/IInterface.h>
#include <android/hardware/hostkeymaster/IHostKeymasterd.h>

namespace android {

namespace hardware {

namespace hostkeymaster {

class BnHostKeymasterd : public ::android::BnInterface<IHostKeymasterd> {
public:
::android::status_t onTransact(uint32_t _aidl_code, const ::android::Parcel& _aidl_data, ::android::Parcel* _aidl_reply, uint32_t _aidl_flags = 0) override;
};  // class BnHostKeymasterd

}  // namespace hostkeymaster

}  // namespace hardware

}  // namespace android

#endif  // AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_BN_HOST_KEYMASTERD_H_
