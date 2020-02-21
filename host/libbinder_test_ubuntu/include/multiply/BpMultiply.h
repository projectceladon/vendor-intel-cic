#ifndef AIDL_GENERATED_MULTIPLY_BP_MULTIPLY_H_
#define AIDL_GENERATED_MULTIPLY_BP_MULTIPLY_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <utils/Errors.h>
#include <multiply/IMultiply.h>

namespace multiply {

class BpMultiply : public ::android::BpInterface<IMultiply> {
public:
explicit BpMultiply(const ::android::sp<::android::IBinder>& _aidl_impl);
virtual ~BpMultiply() = default;
::android::binder::Status multiply(int32_t a, int32_t b, int32_t* _aidl_return) override;
};  // class BpMultiply

}  // namespace multiply

#endif  // AIDL_GENERATED_MULTIPLY_BP_MULTIPLY_H_
