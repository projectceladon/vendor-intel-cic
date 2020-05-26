#ifndef AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_I_HOST_KEYMASTERD_H_
#define AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_I_HOST_KEYMASTERD_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Status.h>
#include <cstdint>
#include <utils/StrongPointer.h>
#include <vector>

namespace android {

namespace hardware {

namespace hostkeymaster {

class IHostKeymasterd : public ::android::IInterface {
public:
DECLARE_META_INTERFACE(HostKeymasterd)
virtual ::android::binder::Status KMCall(int32_t cmd, const ::std::vector<uint8_t>& inbuf, ::std::vector<uint8_t>* outbuf, int32_t* _aidl_return) = 0;
enum Call {
  KMCALL = ::android::IBinder::FIRST_CALL_TRANSACTION + 0,
};
};  // class IHostKeymasterd

}  // namespace hostkeymaster

}  // namespace hardware

}  // namespace android

#endif  // AIDL_GENERATED_ANDROID_HARDWARE_HOSTKEYMASTER_I_HOST_KEYMASTERD_H_
