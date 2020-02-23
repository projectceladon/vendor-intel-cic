#include <com/intel/subtract/ISubtract.h>
#include <com/intel/subtract/BpSubtract.h>

namespace com {

namespace intel {

namespace subtract {

IMPLEMENT_META_INTERFACE(Subtract, "com.intel.subtract.ISubtract")

}  // namespace subtract

}  // namespace intel

}  // namespace com
#include <com/intel/subtract/BpSubtract.h>
#include <binder/Parcel.h>

namespace com {

namespace intel {

namespace subtract {

BpSubtract::BpSubtract(const ::android::sp<::android::IBinder>& _aidl_impl)
    : BpInterface<ISubtract>(_aidl_impl){
}

::android::binder::Status BpSubtract::subtract(int32_t a, int32_t b, int32_t* _aidl_return) {
::android::Parcel _aidl_data;
::android::Parcel _aidl_reply;
::android::status_t _aidl_ret_status = ::android::OK;
::android::binder::Status _aidl_status;
_aidl_ret_status = _aidl_data.writeInterfaceToken(getInterfaceDescriptor());
if (((_aidl_ret_status) != (::android::OK))) {
goto _aidl_error;
}
_aidl_ret_status = _aidl_data.writeInt32(a);
if (((_aidl_ret_status) != (::android::OK))) {
goto _aidl_error;
}
_aidl_ret_status = _aidl_data.writeInt32(b);
if (((_aidl_ret_status) != (::android::OK))) {
goto _aidl_error;
}
_aidl_ret_status = remote()->transact(ISubtract::SUBTRACT, _aidl_data, &_aidl_reply);
if (((_aidl_ret_status) != (::android::OK))) {
goto _aidl_error;
}
_aidl_ret_status = _aidl_status.readFromParcel(_aidl_reply);
if (((_aidl_ret_status) != (::android::OK))) {
goto _aidl_error;
}
if (!_aidl_status.isOk()) {
return _aidl_status;
}
_aidl_ret_status = _aidl_reply.readInt32(_aidl_return);
if (((_aidl_ret_status) != (::android::OK))) {
goto _aidl_error;
}
_aidl_error:
_aidl_status.setFromStatusT(_aidl_ret_status);
return _aidl_status;
}

}  // namespace subtract

}  // namespace intel

}  // namespace com
#include <com/intel/subtract/BnSubtract.h>
#include <binder/Parcel.h>

namespace com {

namespace intel {

namespace subtract {

::android::status_t BnSubtract::onTransact(uint32_t _aidl_code, const ::android::Parcel& _aidl_data, ::android::Parcel* _aidl_reply, uint32_t _aidl_flags) {
::android::status_t _aidl_ret_status = ::android::OK;
switch (_aidl_code) {
case Call::SUBTRACT:
{
int32_t in_a;
int32_t in_b;
int32_t _aidl_return;
if (!(_aidl_data.checkInterface(this))) {
_aidl_ret_status = ::android::BAD_TYPE;
break;
}
_aidl_ret_status = _aidl_data.readInt32(&in_a);
if (((_aidl_ret_status) != (::android::OK))) {
break;
}
_aidl_ret_status = _aidl_data.readInt32(&in_b);
if (((_aidl_ret_status) != (::android::OK))) {
break;
}
::android::binder::Status _aidl_status(subtract(in_a, in_b, &_aidl_return));
_aidl_ret_status = _aidl_status.writeToParcel(_aidl_reply);
if (((_aidl_ret_status) != (::android::OK))) {
break;
}
if (!_aidl_status.isOk()) {
break;
}
_aidl_ret_status = _aidl_reply->writeInt32(_aidl_return);
if (((_aidl_ret_status) != (::android::OK))) {
break;
}
}
break;
default:
{
_aidl_ret_status = ::android::BBinder::onTransact(_aidl_code, _aidl_data, _aidl_reply, _aidl_flags);
}
break;
}
if (_aidl_ret_status == ::android::UNEXPECTED_NULL) {
_aidl_ret_status = ::android::binder::Status::fromExceptionCode(::android::binder::Status::EX_NULL_POINTER).writeToParcel(_aidl_reply);
}
return _aidl_ret_status;
}

}  // namespace subtract

}  // namespace intel

}  // namespace com