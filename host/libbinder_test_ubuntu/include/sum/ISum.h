#ifndef AIDL_GENERATED_SUM_I_SUM_H_
#define AIDL_GENERATED_SUM_I_SUM_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Status.h>
#include <cstdint>
#include <utils/StrongPointer.h>

namespace sum {

class ISum : public ::android::IInterface {
public:
DECLARE_META_INTERFACE(Sum)
virtual ::android::binder::Status sum(int32_t a, int32_t b, int32_t* _aidl_return) = 0;
enum Call {
  SUM = ::android::IBinder::FIRST_CALL_TRANSACTION + 0,
};
};  // class ISum

}  // namespace sum

#endif  // AIDL_GENERATED_SUM_I_SUM_H_
