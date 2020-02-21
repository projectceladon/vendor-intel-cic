#ifndef AIDL_GENERATED_SUM_BP_SUM_H_
#define AIDL_GENERATED_SUM_BP_SUM_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <utils/Errors.h>
#include <sum/ISum.h>

namespace sum {

class BpSum : public ::android::BpInterface<ISum> {
public:
explicit BpSum(const ::android::sp<::android::IBinder>& _aidl_impl);
virtual ~BpSum() = default;
::android::binder::Status sum(int32_t a, int32_t b, int32_t* _aidl_return) override;
};  // class BpSum

}  // namespace sum

#endif  // AIDL_GENERATED_SUM_BP_SUM_H_
