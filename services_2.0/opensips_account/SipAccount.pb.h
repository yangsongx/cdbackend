// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: SipAccount.proto

#ifndef PROTOBUF_SipAccount_2eproto__INCLUDED
#define PROTOBUF_SipAccount_2eproto__INCLUDED

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
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_SipAccount_2eproto();
void protobuf_AssignDesc_SipAccount_2eproto();
void protobuf_ShutdownFile_SipAccount_2eproto();

class SipAccountRequest;
class SipAccountResponse;

// ===================================================================

class SipAccountRequest : public ::google::protobuf::Message {
 public:
  SipAccountRequest();
  virtual ~SipAccountRequest();

  SipAccountRequest(const SipAccountRequest& from);

  inline SipAccountRequest& operator=(const SipAccountRequest& from) {
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
  static const SipAccountRequest& default_instance();

  void Swap(SipAccountRequest* other);

  // implements Message ----------------------------------------------

  SipAccountRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SipAccountRequest& from);
  void MergeFrom(const SipAccountRequest& from);
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

  // required string user_name = 1;
  inline bool has_user_name() const;
  inline void clear_user_name();
  static const int kUserNameFieldNumber = 1;
  inline const ::std::string& user_name() const;
  inline void set_user_name(const ::std::string& value);
  inline void set_user_name(const char* value);
  inline void set_user_name(const char* value, size_t size);
  inline ::std::string* mutable_user_name();
  inline ::std::string* release_user_name();
  inline void set_allocated_user_name(::std::string* user_name);

  // @@protoc_insertion_point(class_scope:SipAccountRequest)
 private:
  inline void set_has_user_name();
  inline void clear_has_user_name();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* user_name_;
  friend void  protobuf_AddDesc_SipAccount_2eproto();
  friend void protobuf_AssignDesc_SipAccount_2eproto();
  friend void protobuf_ShutdownFile_SipAccount_2eproto();

  void InitAsDefaultInstance();
  static SipAccountRequest* default_instance_;
};
// -------------------------------------------------------------------

class SipAccountResponse : public ::google::protobuf::Message {
 public:
  SipAccountResponse();
  virtual ~SipAccountResponse();

  SipAccountResponse(const SipAccountResponse& from);

  inline SipAccountResponse& operator=(const SipAccountResponse& from) {
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
  static const SipAccountResponse& default_instance();

  void Swap(SipAccountResponse* other);

  // implements Message ----------------------------------------------

  SipAccountResponse* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SipAccountResponse& from);
  void MergeFrom(const SipAccountResponse& from);
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

  // required int32 code = 1;
  inline bool has_code() const;
  inline void clear_code();
  static const int kCodeFieldNumber = 1;
  inline ::google::protobuf::int32 code() const;
  inline void set_code(::google::protobuf::int32 value);

  // optional string user_credential = 2;
  inline bool has_user_credential() const;
  inline void clear_user_credential();
  static const int kUserCredentialFieldNumber = 2;
  inline const ::std::string& user_credential() const;
  inline void set_user_credential(const ::std::string& value);
  inline void set_user_credential(const char* value);
  inline void set_user_credential(const char* value, size_t size);
  inline ::std::string* mutable_user_credential();
  inline ::std::string* release_user_credential();
  inline void set_allocated_user_credential(::std::string* user_credential);

  // @@protoc_insertion_point(class_scope:SipAccountResponse)
 private:
  inline void set_has_code();
  inline void clear_has_code();
  inline void set_has_user_credential();
  inline void clear_has_user_credential();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* user_credential_;
  ::google::protobuf::int32 code_;
  friend void  protobuf_AddDesc_SipAccount_2eproto();
  friend void protobuf_AssignDesc_SipAccount_2eproto();
  friend void protobuf_ShutdownFile_SipAccount_2eproto();

  void InitAsDefaultInstance();
  static SipAccountResponse* default_instance_;
};
// ===================================================================


// ===================================================================

// SipAccountRequest

// required string user_name = 1;
inline bool SipAccountRequest::has_user_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void SipAccountRequest::set_has_user_name() {
  _has_bits_[0] |= 0x00000001u;
}
inline void SipAccountRequest::clear_has_user_name() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void SipAccountRequest::clear_user_name() {
  if (user_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_name_->clear();
  }
  clear_has_user_name();
}
inline const ::std::string& SipAccountRequest::user_name() const {
  // @@protoc_insertion_point(field_get:SipAccountRequest.user_name)
  return *user_name_;
}
inline void SipAccountRequest::set_user_name(const ::std::string& value) {
  set_has_user_name();
  if (user_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_name_ = new ::std::string;
  }
  user_name_->assign(value);
  // @@protoc_insertion_point(field_set:SipAccountRequest.user_name)
}
inline void SipAccountRequest::set_user_name(const char* value) {
  set_has_user_name();
  if (user_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_name_ = new ::std::string;
  }
  user_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:SipAccountRequest.user_name)
}
inline void SipAccountRequest::set_user_name(const char* value, size_t size) {
  set_has_user_name();
  if (user_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_name_ = new ::std::string;
  }
  user_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:SipAccountRequest.user_name)
}
inline ::std::string* SipAccountRequest::mutable_user_name() {
  set_has_user_name();
  if (user_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:SipAccountRequest.user_name)
  return user_name_;
}
inline ::std::string* SipAccountRequest::release_user_name() {
  clear_has_user_name();
  if (user_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = user_name_;
    user_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void SipAccountRequest::set_allocated_user_name(::std::string* user_name) {
  if (user_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete user_name_;
  }
  if (user_name) {
    set_has_user_name();
    user_name_ = user_name;
  } else {
    clear_has_user_name();
    user_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:SipAccountRequest.user_name)
}

// -------------------------------------------------------------------

// SipAccountResponse

// required int32 code = 1;
inline bool SipAccountResponse::has_code() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void SipAccountResponse::set_has_code() {
  _has_bits_[0] |= 0x00000001u;
}
inline void SipAccountResponse::clear_has_code() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void SipAccountResponse::clear_code() {
  code_ = 0;
  clear_has_code();
}
inline ::google::protobuf::int32 SipAccountResponse::code() const {
  // @@protoc_insertion_point(field_get:SipAccountResponse.code)
  return code_;
}
inline void SipAccountResponse::set_code(::google::protobuf::int32 value) {
  set_has_code();
  code_ = value;
  // @@protoc_insertion_point(field_set:SipAccountResponse.code)
}

// optional string user_credential = 2;
inline bool SipAccountResponse::has_user_credential() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void SipAccountResponse::set_has_user_credential() {
  _has_bits_[0] |= 0x00000002u;
}
inline void SipAccountResponse::clear_has_user_credential() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void SipAccountResponse::clear_user_credential() {
  if (user_credential_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_credential_->clear();
  }
  clear_has_user_credential();
}
inline const ::std::string& SipAccountResponse::user_credential() const {
  // @@protoc_insertion_point(field_get:SipAccountResponse.user_credential)
  return *user_credential_;
}
inline void SipAccountResponse::set_user_credential(const ::std::string& value) {
  set_has_user_credential();
  if (user_credential_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_credential_ = new ::std::string;
  }
  user_credential_->assign(value);
  // @@protoc_insertion_point(field_set:SipAccountResponse.user_credential)
}
inline void SipAccountResponse::set_user_credential(const char* value) {
  set_has_user_credential();
  if (user_credential_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_credential_ = new ::std::string;
  }
  user_credential_->assign(value);
  // @@protoc_insertion_point(field_set_char:SipAccountResponse.user_credential)
}
inline void SipAccountResponse::set_user_credential(const char* value, size_t size) {
  set_has_user_credential();
  if (user_credential_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_credential_ = new ::std::string;
  }
  user_credential_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:SipAccountResponse.user_credential)
}
inline ::std::string* SipAccountResponse::mutable_user_credential() {
  set_has_user_credential();
  if (user_credential_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_credential_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:SipAccountResponse.user_credential)
  return user_credential_;
}
inline ::std::string* SipAccountResponse::release_user_credential() {
  clear_has_user_credential();
  if (user_credential_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = user_credential_;
    user_credential_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void SipAccountResponse::set_allocated_user_credential(::std::string* user_credential) {
  if (user_credential_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete user_credential_;
  }
  if (user_credential) {
    set_has_user_credential();
    user_credential_ = user_credential;
  } else {
    clear_has_user_credential();
    user_credential_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:SipAccountResponse.user_credential)
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_SipAccount_2eproto__INCLUDED
