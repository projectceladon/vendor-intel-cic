#ifndef AIDL_GENERATED_MULTIPLY_BN_MULTIPLY_H_
#define AIDL_GENERATED_MULTIPLY_BN_MULTIPLY_H_

#include <binder/IInterface.h>
#include <multiply/IMultiply.h>

namespace multiply {

class BnMultiply : public ::android::BnInterface<IMultiply> {
public:
::android::status_t onTransact(uint32_t _aidl_code, const ::android::Parcel& _aidl_data, ::android::Parcel* _aidl_reply, uint32_t _aidl_flags = 0) override;
};  // class BnMultiply

}  // namespace multiply

#endif  // AIDL_GENERATED_MULTIPLY_BN_MULTIPLY_H_
