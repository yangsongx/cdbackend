// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: UserRegister.proto

#ifndef PROTOBUF_UserRegister_2eproto__INCLUDED
#define PROTOBUF_UserRegister_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace caredear {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_UserRegister_2eproto();
void protobuf_AssignDesc_UserRegister_2eproto();
void protobuf_ShutdownFile_UserRegister_2eproto();

class RegisterRequest;
class RegisterResponse;

enum Regtype {
  MOBILE_PHONE = 1,
  EMAIL = 2,
  USER_NAME = 3,
  OTHERS = 4
};
bool Regtype_IsValid(int value);
const Regtype Regtype_MIN = MOBILE_PHONE;
const Regtype Regtype_MAX = OTHERS;
const int Regtype_ARRAYSIZE = Regtype_MAX + 1;

const ::google::protobuf::EnumDescriptor* Regtype_descriptor();
inline const ::std::string& Regtype_Name(Regtype value) {
  return ::google::protobuf::internal::NameOfEnum(
    Regtype_descriptor(), value);
}
inline bool Regtype_Parse(
    const ::std::string& name, Regtype* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Regtype>(
    Regtype_descriptor(), name, value);
}
enum DeviceType {
  ANDROID = 0,
  IOS = 1,
  CAREDEAROS = 2,
  PC = 3
};
bool DeviceType_IsValid(int value);
const DeviceType DeviceType_MIN = ANDROID;
const DeviceType DeviceType_MAX = PC;
const int DeviceType_ARRAYSIZE = DeviceType_MAX + 1;

const ::google::protobuf::EnumDescriptor* DeviceType_descriptor();
inline const ::std::string& DeviceType_Name(DeviceType value) {
  return ::google::protobuf::internal::NameOfEnum(
    DeviceType_descriptor(), value);
}
inline bool DeviceType_Parse(
    const ::std::string& name, DeviceType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<DeviceType>(
    DeviceType_descriptor(), name, value);
}
// ===================================================================

class RegisterRequest : public ::google::protobuf::Message {
 public:
  RegisterRequest();
  virtual ~RegisterRequest();

  RegisterRequest(const RegisterRequest& from);

  inline RegisterRequest& operator=(const RegisterRequest& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RegisterRequest& default_instance();

  void Swap(RegisterRequest* other);

  // implements Message ----------------------------------------------

  RegisterRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RegisterRequest& from);
  void MergeFrom(const RegisterRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required .com.caredear.Regtype reg_type = 1;
  inline bool has_reg_type() const;
  inline void clear_reg_type();
  static const int kRegTypeFieldNumber = 1;
  inline ::com::caredear::Regtype reg_type() const;
  inline void set_reg_type(::com::caredear::Regtype value);

  // required .com.caredear.DeviceType reg_device = 2;
  inline bool has_reg_device() const;
  inline void clear_reg_device();
  static const int kRegDeviceFieldNumber = 2;
  inline ::com::caredear::DeviceType reg_device() const;
  inline void set_reg_device(::com::caredear::DeviceType value);

  // required string reg_source = 3;
  inline bool has_reg_source() const;
  inline void clear_reg_source();
  static const int kRegSourceFieldNumber = 3;
  inline const ::std::string& reg_source() const;
  inline void set_reg_source(const ::std::string& value);
  inline void set_reg_source(const char* value);
  inline void set_reg_source(const char* value, size_t size);
  inline ::std::string* mutable_reg_source();
  inline ::std::string* release_reg_source();
  inline void set_allocated_reg_source(::std::string* reg_source);

  // required string reg_name = 4;
  inline bool has_reg_name() const;
  inline void clear_reg_name();
  static const int kRegNameFieldNumber = 4;
  inline const ::std::string& reg_name() const;
  inline void set_reg_name(const ::std::string& value);
  inline void set_reg_name(const char* value);
  inline void set_reg_name(const char* value, size_t size);
  inline ::std::string* mutable_reg_name();
  inline ::std::string* release_reg_name();
  inline void set_allocated_reg_name(::std::string* reg_name);

  // optional string reg_password = 5;
  inline bool has_reg_password() const;
  inline void clear_reg_password();
  static const int kRegPasswordFieldNumber = 5;
  inline const ::std::string& reg_password() const;
  inline void set_reg_password(const ::std::string& value);
  inline void set_reg_password(const char* value);
  inline void set_reg_password(const char* value, size_t size);
  inline ::std::string* mutable_reg_password();
  inline ::std::string* release_reg_password();
  inline void set_allocated_reg_password(::std::string* reg_password);

  // @@protoc_insertion_point(class_scope:com.caredear.RegisterRequest)
 private:
  inline void set_has_reg_type();
  inline void clear_has_reg_type();
  inline void set_has_reg_device();
  inline void clear_has_reg_device();
  inline void set_has_reg_source();
  inline void clear_has_reg_source();
  inline void set_has_reg_name();
  inline void clear_has_reg_name();
  inline void set_has_reg_password();
  inline void clear_has_reg_password();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  int reg_type_;
  int reg_device_;
  ::std::string* reg_source_;
  ::std::string* reg_name_;
  ::std::string* reg_password_;
  friend void  protobuf_AddDesc_UserRegister_2eproto();
  friend void protobuf_AssignDesc_UserRegister_2eproto();
  friend void protobuf_ShutdownFile_UserRegister_2eproto();

  void InitAsDefaultInstance();
  static RegisterRequest* default_instance_;
};
// -------------------------------------------------------------------

class RegisterResponse : public ::google::protobuf::Message {
 public:
  RegisterResponse();
  virtual ~RegisterResponse();

  RegisterResponse(const RegisterResponse& from);

  inline RegisterResponse& operator=(const RegisterResponse& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RegisterResponse& default_instance();

  void Swap(RegisterResponse* other);

  // implements Message ----------------------------------------------

  RegisterResponse* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RegisterResponse& from);
  void MergeFrom(const RegisterResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required int32 result_code = 1;
  inline bool has_result_code() const;
  inline void clear_result_code();
  static const int kResultCodeFieldNumber = 1;
  inline ::google::protobuf::int32 result_code() const;
  inline void set_result_code(::google::protobuf::int32 value);

  // optional string extra_msg = 2;
  inline bool has_extra_msg() const;
  inline void clear_extra_msg();
  static const int kExtraMsgFieldNumber = 2;
  inline const ::std::string& extra_msg() const;
  inline void set_extra_msg(const ::std::string& value);
  inline void set_extra_msg(const char* value);
  inline void set_extra_msg(const char* value, size_t size);
  inline ::std::string* mutable_extra_msg();
  inline ::std::string* release_extra_msg();
  inline void set_allocated_extra_msg(::std::string* extra_msg);

  // optional string reg_verifycode = 3;
  inline bool has_reg_verifycode() const;
  inline void clear_reg_verifycode();
  static const int kRegVerifycodeFieldNumber = 3;
  inline const ::std::string& reg_verifycode() const;
  inline void set_reg_verifycode(const ::std::string& value);
  inline void set_reg_verifycode(const char* value);
  inline void set_reg_verifycode(const char* value, size_t size);
  inline ::std::string* mutable_reg_verifycode();
  inline ::std::string* release_reg_verifycode();
  inline void set_allocated_reg_verifycode(::std::string* reg_verifycode);

  // optional string caredear_id = 4;
  inline bool has_caredear_id() const;
  inline void clear_caredear_id();
  static const int kCaredearIdFieldNumber = 4;
  inline const ::std::string& caredear_id() const;
  inline void set_caredear_id(const ::std::string& value);
  inline void set_caredear_id(const char* value);
  inline void set_caredear_id(const char* value, size_t size);
  inline ::std::string* mutable_caredear_id();
  inline ::std::string* release_caredear_id();
  inline void set_allocated_caredear_id(::std::string* caredear_id);

  // @@protoc_insertion_point(class_scope:com.caredear.RegisterResponse)
 private:
  inline void set_has_result_code();
  inline void clear_has_result_code();
  inline void set_has_extra_msg();
  inline void clear_has_extra_msg();
  inline void set_has_reg_verifycode();
  inline void clear_has_reg_verifycode();
  inline void set_has_caredear_id();
  inline void clear_has_caredear_id();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* extra_msg_;
  ::std::string* reg_verifycode_;
  ::std::string* caredear_id_;
  ::google::protobuf::int32 result_code_;
  friend void  protobuf_AddDesc_UserRegister_2eproto();
  friend void protobuf_AssignDesc_UserRegister_2eproto();
  friend void protobuf_ShutdownFile_UserRegister_2eproto();

  void InitAsDefaultInstance();
  static RegisterResponse* default_instance_;
};
// ===================================================================


// ===================================================================

// RegisterRequest

// required .com.caredear.Regtype reg_type = 1;
inline bool RegisterRequest::has_reg_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void RegisterRequest::set_has_reg_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void RegisterRequest::clear_has_reg_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void RegisterRequest::clear_reg_type() {
  reg_type_ = 1;
  clear_has_reg_type();
}
inline ::com::caredear::Regtype RegisterRequest::reg_type() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterRequest.reg_type)
  return static_cast< ::com::caredear::Regtype >(reg_type_);
}
inline void RegisterRequest::set_reg_type(::com::caredear::Regtype value) {
  assert(::com::caredear::Regtype_IsValid(value));
  set_has_reg_type();
  reg_type_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.RegisterRequest.reg_type)
}

// required .com.caredear.DeviceType reg_device = 2;
inline bool RegisterRequest::has_reg_device() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void RegisterRequest::set_has_reg_device() {
  _has_bits_[0] |= 0x00000002u;
}
inline void RegisterRequest::clear_has_reg_device() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void RegisterRequest::clear_reg_device() {
  reg_device_ = 0;
  clear_has_reg_device();
}
inline ::com::caredear::DeviceType RegisterRequest::reg_device() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterRequest.reg_device)
  return static_cast< ::com::caredear::DeviceType >(reg_device_);
}
inline void RegisterRequest::set_reg_device(::com::caredear::DeviceType value) {
  assert(::com::caredear::DeviceType_IsValid(value));
  set_has_reg_device();
  reg_device_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.RegisterRequest.reg_device)
}

// required string reg_source = 3;
inline bool RegisterRequest::has_reg_source() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void RegisterRequest::set_has_reg_source() {
  _has_bits_[0] |= 0x00000004u;
}
inline void RegisterRequest::clear_has_reg_source() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void RegisterRequest::clear_reg_source() {
  if (reg_source_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_source_->clear();
  }
  clear_has_reg_source();
}
inline const ::std::string& RegisterRequest::reg_source() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterRequest.reg_source)
  return *reg_source_;
}
inline void RegisterRequest::set_reg_source(const ::std::string& value) {
  set_has_reg_source();
  if (reg_source_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_source_ = new ::std::string;
  }
  reg_source_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.RegisterRequest.reg_source)
}
inline void RegisterRequest::set_reg_source(const char* value) {
  set_has_reg_source();
  if (reg_source_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_source_ = new ::std::string;
  }
  reg_source_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.RegisterRequest.reg_source)
}
inline void RegisterRequest::set_reg_source(const char* value, size_t size) {
  set_has_reg_source();
  if (reg_source_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_source_ = new ::std::string;
  }
  reg_source_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.RegisterRequest.reg_source)
}
inline ::std::string* RegisterRequest::mutable_reg_source() {
  set_has_reg_source();
  if (reg_source_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_source_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.RegisterRequest.reg_source)
  return reg_source_;
}
inline ::std::string* RegisterRequest::release_reg_source() {
  clear_has_reg_source();
  if (reg_source_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = reg_source_;
    reg_source_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void RegisterRequest::set_allocated_reg_source(::std::string* reg_source) {
  if (reg_source_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_source_;
  }
  if (reg_source) {
    set_has_reg_source();
    reg_source_ = reg_source;
  } else {
    clear_has_reg_source();
    reg_source_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.RegisterRequest.reg_source)
}

// required string reg_name = 4;
inline bool RegisterRequest::has_reg_name() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void RegisterRequest::set_has_reg_name() {
  _has_bits_[0] |= 0x00000008u;
}
inline void RegisterRequest::clear_has_reg_name() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void RegisterRequest::clear_reg_name() {
  if (reg_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_name_->clear();
  }
  clear_has_reg_name();
}
inline const ::std::string& RegisterRequest::reg_name() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterRequest.reg_name)
  return *reg_name_;
}
inline void RegisterRequest::set_reg_name(const ::std::string& value) {
  set_has_reg_name();
  if (reg_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_name_ = new ::std::string;
  }
  reg_name_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.RegisterRequest.reg_name)
}
inline void RegisterRequest::set_reg_name(const char* value) {
  set_has_reg_name();
  if (reg_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_name_ = new ::std::string;
  }
  reg_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.RegisterRequest.reg_name)
}
inline void RegisterRequest::set_reg_name(const char* value, size_t size) {
  set_has_reg_name();
  if (reg_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_name_ = new ::std::string;
  }
  reg_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.RegisterRequest.reg_name)
}
inline ::std::string* RegisterRequest::mutable_reg_name() {
  set_has_reg_name();
  if (reg_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.RegisterRequest.reg_name)
  return reg_name_;
}
inline ::std::string* RegisterRequest::release_reg_name() {
  clear_has_reg_name();
  if (reg_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = reg_name_;
    reg_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void RegisterRequest::set_allocated_reg_name(::std::string* reg_name) {
  if (reg_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_name_;
  }
  if (reg_name) {
    set_has_reg_name();
    reg_name_ = reg_name;
  } else {
    clear_has_reg_name();
    reg_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.RegisterRequest.reg_name)
}

// optional string reg_password = 5;
inline bool RegisterRequest::has_reg_password() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void RegisterRequest::set_has_reg_password() {
  _has_bits_[0] |= 0x00000010u;
}
inline void RegisterRequest::clear_has_reg_password() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void RegisterRequest::clear_reg_password() {
  if (reg_password_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_password_->clear();
  }
  clear_has_reg_password();
}
inline const ::std::string& RegisterRequest::reg_password() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterRequest.reg_password)
  return *reg_password_;
}
inline void RegisterRequest::set_reg_password(const ::std::string& value) {
  set_has_reg_password();
  if (reg_password_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_password_ = new ::std::string;
  }
  reg_password_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.RegisterRequest.reg_password)
}
inline void RegisterRequest::set_reg_password(const char* value) {
  set_has_reg_password();
  if (reg_password_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_password_ = new ::std::string;
  }
  reg_password_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.RegisterRequest.reg_password)
}
inline void RegisterRequest::set_reg_password(const char* value, size_t size) {
  set_has_reg_password();
  if (reg_password_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_password_ = new ::std::string;
  }
  reg_password_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.RegisterRequest.reg_password)
}
inline ::std::string* RegisterRequest::mutable_reg_password() {
  set_has_reg_password();
  if (reg_password_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_password_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.RegisterRequest.reg_password)
  return reg_password_;
}
inline ::std::string* RegisterRequest::release_reg_password() {
  clear_has_reg_password();
  if (reg_password_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = reg_password_;
    reg_password_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void RegisterRequest::set_allocated_reg_password(::std::string* reg_password) {
  if (reg_password_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_password_;
  }
  if (reg_password) {
    set_has_reg_password();
    reg_password_ = reg_password;
  } else {
    clear_has_reg_password();
    reg_password_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.RegisterRequest.reg_password)
}

// -------------------------------------------------------------------

// RegisterResponse

// required int32 result_code = 1;
inline bool RegisterResponse::has_result_code() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void RegisterResponse::set_has_result_code() {
  _has_bits_[0] |= 0x00000001u;
}
inline void RegisterResponse::clear_has_result_code() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void RegisterResponse::clear_result_code() {
  result_code_ = 0;
  clear_has_result_code();
}
inline ::google::protobuf::int32 RegisterResponse::result_code() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterResponse.result_code)
  return result_code_;
}
inline void RegisterResponse::set_result_code(::google::protobuf::int32 value) {
  set_has_result_code();
  result_code_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.RegisterResponse.result_code)
}

// optional string extra_msg = 2;
inline bool RegisterResponse::has_extra_msg() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void RegisterResponse::set_has_extra_msg() {
  _has_bits_[0] |= 0x00000002u;
}
inline void RegisterResponse::clear_has_extra_msg() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void RegisterResponse::clear_extra_msg() {
  if (extra_msg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    extra_msg_->clear();
  }
  clear_has_extra_msg();
}
inline const ::std::string& RegisterResponse::extra_msg() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterResponse.extra_msg)
  return *extra_msg_;
}
inline void RegisterResponse::set_extra_msg(const ::std::string& value) {
  set_has_extra_msg();
  if (extra_msg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    extra_msg_ = new ::std::string;
  }
  extra_msg_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.RegisterResponse.extra_msg)
}
inline void RegisterResponse::set_extra_msg(const char* value) {
  set_has_extra_msg();
  if (extra_msg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    extra_msg_ = new ::std::string;
  }
  extra_msg_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.RegisterResponse.extra_msg)
}
inline void RegisterResponse::set_extra_msg(const char* value, size_t size) {
  set_has_extra_msg();
  if (extra_msg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    extra_msg_ = new ::std::string;
  }
  extra_msg_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.RegisterResponse.extra_msg)
}
inline ::std::string* RegisterResponse::mutable_extra_msg() {
  set_has_extra_msg();
  if (extra_msg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    extra_msg_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.RegisterResponse.extra_msg)
  return extra_msg_;
}
inline ::std::string* RegisterResponse::release_extra_msg() {
  clear_has_extra_msg();
  if (extra_msg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = extra_msg_;
    extra_msg_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void RegisterResponse::set_allocated_extra_msg(::std::string* extra_msg) {
  if (extra_msg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete extra_msg_;
  }
  if (extra_msg) {
    set_has_extra_msg();
    extra_msg_ = extra_msg;
  } else {
    clear_has_extra_msg();
    extra_msg_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.RegisterResponse.extra_msg)
}

// optional string reg_verifycode = 3;
inline bool RegisterResponse::has_reg_verifycode() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void RegisterResponse::set_has_reg_verifycode() {
  _has_bits_[0] |= 0x00000004u;
}
inline void RegisterResponse::clear_has_reg_verifycode() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void RegisterResponse::clear_reg_verifycode() {
  if (reg_verifycode_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_verifycode_->clear();
  }
  clear_has_reg_verifycode();
}
inline const ::std::string& RegisterResponse::reg_verifycode() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterResponse.reg_verifycode)
  return *reg_verifycode_;
}
inline void RegisterResponse::set_reg_verifycode(const ::std::string& value) {
  set_has_reg_verifycode();
  if (reg_verifycode_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_verifycode_ = new ::std::string;
  }
  reg_verifycode_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.RegisterResponse.reg_verifycode)
}
inline void RegisterResponse::set_reg_verifycode(const char* value) {
  set_has_reg_verifycode();
  if (reg_verifycode_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_verifycode_ = new ::std::string;
  }
  reg_verifycode_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.RegisterResponse.reg_verifycode)
}
inline void RegisterResponse::set_reg_verifycode(const char* value, size_t size) {
  set_has_reg_verifycode();
  if (reg_verifycode_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_verifycode_ = new ::std::string;
  }
  reg_verifycode_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.RegisterResponse.reg_verifycode)
}
inline ::std::string* RegisterResponse::mutable_reg_verifycode() {
  set_has_reg_verifycode();
  if (reg_verifycode_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    reg_verifycode_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.RegisterResponse.reg_verifycode)
  return reg_verifycode_;
}
inline ::std::string* RegisterResponse::release_reg_verifycode() {
  clear_has_reg_verifycode();
  if (reg_verifycode_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = reg_verifycode_;
    reg_verifycode_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void RegisterResponse::set_allocated_reg_verifycode(::std::string* reg_verifycode) {
  if (reg_verifycode_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_verifycode_;
  }
  if (reg_verifycode) {
    set_has_reg_verifycode();
    reg_verifycode_ = reg_verifycode;
  } else {
    clear_has_reg_verifycode();
    reg_verifycode_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.RegisterResponse.reg_verifycode)
}

// optional string caredear_id = 4;
inline bool RegisterResponse::has_caredear_id() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void RegisterResponse::set_has_caredear_id() {
  _has_bits_[0] |= 0x00000008u;
}
inline void RegisterResponse::clear_has_caredear_id() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void RegisterResponse::clear_caredear_id() {
  if (caredear_id_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    caredear_id_->clear();
  }
  clear_has_caredear_id();
}
inline const ::std::string& RegisterResponse::caredear_id() const {
  // @@protoc_insertion_point(field_get:com.caredear.RegisterResponse.caredear_id)
  return *caredear_id_;
}
inline void RegisterResponse::set_caredear_id(const ::std::string& value) {
  set_has_caredear_id();
  if (caredear_id_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    caredear_id_ = new ::std::string;
  }
  caredear_id_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.RegisterResponse.caredear_id)
}
inline void RegisterResponse::set_caredear_id(const char* value) {
  set_has_caredear_id();
  if (caredear_id_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    caredear_id_ = new ::std::string;
  }
  caredear_id_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.RegisterResponse.caredear_id)
}
inline void RegisterResponse::set_caredear_id(const char* value, size_t size) {
  set_has_caredear_id();
  if (caredear_id_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    caredear_id_ = new ::std::string;
  }
  caredear_id_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.RegisterResponse.caredear_id)
}
inline ::std::string* RegisterResponse::mutable_caredear_id() {
  set_has_caredear_id();
  if (caredear_id_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    caredear_id_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.RegisterResponse.caredear_id)
  return caredear_id_;
}
inline ::std::string* RegisterResponse::release_caredear_id() {
  clear_has_caredear_id();
  if (caredear_id_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = caredear_id_;
    caredear_id_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void RegisterResponse::set_allocated_caredear_id(::std::string* caredear_id) {
  if (caredear_id_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete caredear_id_;
  }
  if (caredear_id) {
    set_has_caredear_id();
    caredear_id_ = caredear_id;
  } else {
    clear_has_caredear_id();
    caredear_id_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.RegisterResponse.caredear_id)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace caredear
}  // namespace com

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::com::caredear::Regtype> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::com::caredear::Regtype>() {
  return ::com::caredear::Regtype_descriptor();
}
template <> struct is_proto_enum< ::com::caredear::DeviceType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::com::caredear::DeviceType>() {
  return ::com::caredear::DeviceType_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_UserRegister_2eproto__INCLUDED
