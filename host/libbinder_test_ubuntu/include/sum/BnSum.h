#ifndef AIDL_GENERATED_SUM_BN_SUM_H_
#define AIDL_GENERATED_SUM_BN_SUM_H_

#include <binder/IInterface.h>
#include <sum/ISum.h>

namespace sum {

class BnSum : public ::android::BnInterface<ISum> {
public:
::android::status_t onTransact(uint32_t _aidl_code, const ::android::Parcel& _aidl_data, ::android::Parcel* _aidl_reply, uint32_t _aidl_flags = 0) override;
};  // class BnSum

}  // namespace sum

#endif  // AIDL_GENERATED_SUM_BN_SUM_H_
