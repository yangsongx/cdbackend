// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: NetdiskMessage.proto

#ifndef PROTOBUF_NetdiskMessage_2eproto__INCLUDED
#define PROTOBUF_NetdiskMessage_2eproto__INCLUDED

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
void  protobuf_AddDesc_NetdiskMessage_2eproto();
void protobuf_AssignDesc_NetdiskMessage_2eproto();
void protobuf_ShutdownFile_NetdiskMessage_2eproto();

class NetdiskRequest;
class NetdiskResponse;

enum Opcode {
  UPLOADING = 0,
  UPLOADED = 1,
  DELETE = 2,
  RENAME = 3,
  SHARE = 4,
  LISTFILE = 5
};
bool Opcode_IsValid(int value);
const Opcode Opcode_MIN = UPLOADING;
const Opcode Opcode_MAX = LISTFILE;
const int Opcode_ARRAYSIZE = Opcode_MAX + 1;

const ::google::protobuf::EnumDescriptor* Opcode_descriptor();
inline const ::std::string& Opcode_Name(Opcode value) {
  return ::google::protobuf::internal::NameOfEnum(
    Opcode_descriptor(), value);
}
inline bool Opcode_Parse(
    const ::std::string& name, Opcode* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Opcode>(
    Opcode_descriptor(), name, value);
}
// ===================================================================

class NetdiskRequest : public ::google::protobuf::Message {
 public:
  NetdiskRequest();
  virtual ~NetdiskRequest();

  NetdiskRequest(const NetdiskRequest& from);

  inline NetdiskRequest& operator=(const NetdiskRequest& from) {
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
  static const NetdiskRequest& default_instance();

  void Swap(NetdiskRequest* other);

  // implements Message ----------------------------------------------

  NetdiskRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const NetdiskRequest& from);
  void MergeFrom(const NetdiskRequest& from);
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

  // required string user = 1;
  inline bool has_user() const;
  inline void clear_user();
  static const int kUserFieldNumber = 1;
  inline const ::std::string& user() const;
  inline void set_user(const ::std::string& value);
  inline void set_user(const char* value);
  inline void set_user(const char* value, size_t size);
  inline ::std::string* mutable_user();
  inline ::std::string* release_user();
  inline void set_allocated_user(::std::string* user);

  // required .com.caredear.Opcode opcode = 2;
  inline bool has_opcode() const;
  inline void clear_opcode();
  static const int kOpcodeFieldNumber = 2;
  inline ::com::caredear::Opcode opcode() const;
  inline void set_opcode(::com::caredear::Opcode value);

  // optional string md5 = 3;
  inline bool has_md5() const;
  inline void clear_md5();
  static const int kMd5FieldNumber = 3;
  inline const ::std::string& md5() const;
  inline void set_md5(const ::std::string& value);
  inline void set_md5(const char* value);
  inline void set_md5(const char* value, size_t size);
  inline ::std::string* mutable_md5();
  inline ::std::string* release_md5();
  inline void set_allocated_md5(::std::string* md5);

  // optional string filename = 4;
  inline bool has_filename() const;
  inline void clear_filename();
  static const int kFilenameFieldNumber = 4;
  inline const ::std::string& filename() const;
  inline void set_filename(const ::std::string& value);
  inline void set_filename(const char* value);
  inline void set_filename(const char* value, size_t size);
  inline ::std::string* mutable_filename();
  inline ::std::string* release_filename();
  inline void set_allocated_filename(::std::string* filename);

  // optional int32 filesize = 5;
  inline bool has_filesize() const;
  inline void clear_filesize();
  static const int kFilesizeFieldNumber = 5;
  inline ::google::protobuf::int32 filesize() const;
  inline void set_filesize(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:com.caredear.NetdiskRequest)
 private:
  inline void set_has_user();
  inline void clear_has_user();
  inline void set_has_opcode();
  inline void clear_has_opcode();
  inline void set_has_md5();
  inline void clear_has_md5();
  inline void set_has_filename();
  inline void clear_has_filename();
  inline void set_has_filesize();
  inline void clear_has_filesize();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* user_;
  ::std::string* md5_;
  int opcode_;
  ::google::protobuf::int32 filesize_;
  ::std::string* filename_;
  friend void  protobuf_AddDesc_NetdiskMessage_2eproto();
  friend void protobuf_AssignDesc_NetdiskMessage_2eproto();
  friend void protobuf_ShutdownFile_NetdiskMessage_2eproto();

  void InitAsDefaultInstance();
  static NetdiskRequest* default_instance_;
};
// -------------------------------------------------------------------

class NetdiskResponse : public ::google::protobuf::Message {
 public:
  NetdiskResponse();
  virtual ~NetdiskResponse();

  NetdiskResponse(const NetdiskResponse& from);

  inline NetdiskResponse& operator=(const NetdiskResponse& from) {
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
  static const NetdiskResponse& default_instance();

  void Swap(NetdiskResponse* other);

  // implements Message ----------------------------------------------

  NetdiskResponse* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const NetdiskResponse& from);
  void MergeFrom(const NetdiskResponse& from);
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

  // required .com.caredear.Opcode opcode = 1;
  inline bool has_opcode() const;
  inline void clear_opcode();
  static const int kOpcodeFieldNumber = 1;
  inline ::com::caredear::Opcode opcode() const;
  inline void set_opcode(::com::caredear::Opcode value);

  // required int32 result_code = 2;
  inline bool has_result_code() const;
  inline void clear_result_code();
  static const int kResultCodeFieldNumber = 2;
  inline ::google::protobuf::int32 result_code() const;
  inline void set_result_code(::google::protobuf::int32 value);

  // optional string errormsg = 3;
  inline bool has_errormsg() const;
  inline void clear_errormsg();
  static const int kErrormsgFieldNumber = 3;
  inline const ::std::string& errormsg() const;
  inline void set_errormsg(const ::std::string& value);
  inline void set_errormsg(const char* value);
  inline void set_errormsg(const char* value, size_t size);
  inline ::std::string* mutable_errormsg();
  inline ::std::string* release_errormsg();
  inline void set_allocated_errormsg(::std::string* errormsg);

  // optional string uploadtoken = 4;
  inline bool has_uploadtoken() const;
  inline void clear_uploadtoken();
  static const int kUploadtokenFieldNumber = 4;
  inline const ::std::string& uploadtoken() const;
  inline void set_uploadtoken(const ::std::string& value);
  inline void set_uploadtoken(const char* value);
  inline void set_uploadtoken(const char* value, size_t size);
  inline ::std::string* mutable_uploadtoken();
  inline ::std::string* release_uploadtoken();
  inline void set_allocated_uploadtoken(::std::string* uploadtoken);

  // optional string downloadurl = 5;
  inline bool has_downloadurl() const;
  inline void clear_downloadurl();
  static const int kDownloadurlFieldNumber = 5;
  inline const ::std::string& downloadurl() const;
  inline void set_downloadurl(const ::std::string& value);
  inline void set_downloadurl(const char* value);
  inline void set_downloadurl(const char* value, size_t size);
  inline ::std::string* mutable_downloadurl();
  inline ::std::string* release_downloadurl();
  inline void set_allocated_downloadurl(::std::string* downloadurl);

  // optional string netdisckey = 6;
  inline bool has_netdisckey() const;
  inline void clear_netdisckey();
  static const int kNetdisckeyFieldNumber = 6;
  inline const ::std::string& netdisckey() const;
  inline void set_netdisckey(const ::std::string& value);
  inline void set_netdisckey(const char* value);
  inline void set_netdisckey(const char* value, size_t size);
  inline ::std::string* mutable_netdisckey();
  inline ::std::string* release_netdisckey();
  inline void set_allocated_netdisckey(::std::string* netdisckey);

  // @@protoc_insertion_point(class_scope:com.caredear.NetdiskResponse)
 private:
  inline void set_has_opcode();
  inline void clear_has_opcode();
  inline void set_has_result_code();
  inline void clear_has_result_code();
  inline void set_has_errormsg();
  inline void clear_has_errormsg();
  inline void set_has_uploadtoken();
  inline void clear_has_uploadtoken();
  inline void set_has_downloadurl();
  inline void clear_has_downloadurl();
  inline void set_has_netdisckey();
  inline void clear_has_netdisckey();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  int opcode_;
  ::google::protobuf::int32 result_code_;
  ::std::string* errormsg_;
  ::std::string* uploadtoken_;
  ::std::string* downloadurl_;
  ::std::string* netdisckey_;
  friend void  protobuf_AddDesc_NetdiskMessage_2eproto();
  friend void protobuf_AssignDesc_NetdiskMessage_2eproto();
  friend void protobuf_ShutdownFile_NetdiskMessage_2eproto();

  void InitAsDefaultInstance();
  static NetdiskResponse* default_instance_;
};
// ===================================================================


// ===================================================================

// NetdiskRequest

// required string user = 1;
inline bool NetdiskRequest::has_user() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void NetdiskRequest::set_has_user() {
  _has_bits_[0] |= 0x00000001u;
}
inline void NetdiskRequest::clear_has_user() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void NetdiskRequest::clear_user() {
  if (user_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_->clear();
  }
  clear_has_user();
}
inline const ::std::string& NetdiskRequest::user() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskRequest.user)
  return *user_;
}
inline void NetdiskRequest::set_user(const ::std::string& value) {
  set_has_user();
  if (user_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_ = new ::std::string;
  }
  user_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskRequest.user)
}
inline void NetdiskRequest::set_user(const char* value) {
  set_has_user();
  if (user_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_ = new ::std::string;
  }
  user_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskRequest.user)
}
inline void NetdiskRequest::set_user(const char* value, size_t size) {
  set_has_user();
  if (user_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_ = new ::std::string;
  }
  user_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskRequest.user)
}
inline ::std::string* NetdiskRequest::mutable_user() {
  set_has_user();
  if (user_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    user_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskRequest.user)
  return user_;
}
inline ::std::string* NetdiskRequest::release_user() {
  clear_has_user();
  if (user_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = user_;
    user_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskRequest::set_allocated_user(::std::string* user) {
  if (user_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete user_;
  }
  if (user) {
    set_has_user();
    user_ = user;
  } else {
    clear_has_user();
    user_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskRequest.user)
}

// required .com.caredear.Opcode opcode = 2;
inline bool NetdiskRequest::has_opcode() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void NetdiskRequest::set_has_opcode() {
  _has_bits_[0] |= 0x00000002u;
}
inline void NetdiskRequest::clear_has_opcode() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void NetdiskRequest::clear_opcode() {
  opcode_ = 0;
  clear_has_opcode();
}
inline ::com::caredear::Opcode NetdiskRequest::opcode() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskRequest.opcode)
  return static_cast< ::com::caredear::Opcode >(opcode_);
}
inline void NetdiskRequest::set_opcode(::com::caredear::Opcode value) {
  assert(::com::caredear::Opcode_IsValid(value));
  set_has_opcode();
  opcode_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskRequest.opcode)
}

// optional string md5 = 3;
inline bool NetdiskRequest::has_md5() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void NetdiskRequest::set_has_md5() {
  _has_bits_[0] |= 0x00000004u;
}
inline void NetdiskRequest::clear_has_md5() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void NetdiskRequest::clear_md5() {
  if (md5_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    md5_->clear();
  }
  clear_has_md5();
}
inline const ::std::string& NetdiskRequest::md5() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskRequest.md5)
  return *md5_;
}
inline void NetdiskRequest::set_md5(const ::std::string& value) {
  set_has_md5();
  if (md5_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    md5_ = new ::std::string;
  }
  md5_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskRequest.md5)
}
inline void NetdiskRequest::set_md5(const char* value) {
  set_has_md5();
  if (md5_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    md5_ = new ::std::string;
  }
  md5_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskRequest.md5)
}
inline void NetdiskRequest::set_md5(const char* value, size_t size) {
  set_has_md5();
  if (md5_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    md5_ = new ::std::string;
  }
  md5_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskRequest.md5)
}
inline ::std::string* NetdiskRequest::mutable_md5() {
  set_has_md5();
  if (md5_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    md5_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskRequest.md5)
  return md5_;
}
inline ::std::string* NetdiskRequest::release_md5() {
  clear_has_md5();
  if (md5_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = md5_;
    md5_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskRequest::set_allocated_md5(::std::string* md5) {
  if (md5_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete md5_;
  }
  if (md5) {
    set_has_md5();
    md5_ = md5;
  } else {
    clear_has_md5();
    md5_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskRequest.md5)
}

// optional string filename = 4;
inline bool NetdiskRequest::has_filename() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void NetdiskRequest::set_has_filename() {
  _has_bits_[0] |= 0x00000008u;
}
inline void NetdiskRequest::clear_has_filename() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void NetdiskRequest::clear_filename() {
  if (filename_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    filename_->clear();
  }
  clear_has_filename();
}
inline const ::std::string& NetdiskRequest::filename() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskRequest.filename)
  return *filename_;
}
inline void NetdiskRequest::set_filename(const ::std::string& value) {
  set_has_filename();
  if (filename_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    filename_ = new ::std::string;
  }
  filename_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskRequest.filename)
}
inline void NetdiskRequest::set_filename(const char* value) {
  set_has_filename();
  if (filename_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    filename_ = new ::std::string;
  }
  filename_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskRequest.filename)
}
inline void NetdiskRequest::set_filename(const char* value, size_t size) {
  set_has_filename();
  if (filename_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    filename_ = new ::std::string;
  }
  filename_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskRequest.filename)
}
inline ::std::string* NetdiskRequest::mutable_filename() {
  set_has_filename();
  if (filename_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    filename_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskRequest.filename)
  return filename_;
}
inline ::std::string* NetdiskRequest::release_filename() {
  clear_has_filename();
  if (filename_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = filename_;
    filename_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskRequest::set_allocated_filename(::std::string* filename) {
  if (filename_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete filename_;
  }
  if (filename) {
    set_has_filename();
    filename_ = filename;
  } else {
    clear_has_filename();
    filename_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskRequest.filename)
}

// optional int32 filesize = 5;
inline bool NetdiskRequest::has_filesize() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void NetdiskRequest::set_has_filesize() {
  _has_bits_[0] |= 0x00000010u;
}
inline void NetdiskRequest::clear_has_filesize() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void NetdiskRequest::clear_filesize() {
  filesize_ = 0;
  clear_has_filesize();
}
inline ::google::protobuf::int32 NetdiskRequest::filesize() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskRequest.filesize)
  return filesize_;
}
inline void NetdiskRequest::set_filesize(::google::protobuf::int32 value) {
  set_has_filesize();
  filesize_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskRequest.filesize)
}

// -------------------------------------------------------------------

// NetdiskResponse

// required .com.caredear.Opcode opcode = 1;
inline bool NetdiskResponse::has_opcode() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void NetdiskResponse::set_has_opcode() {
  _has_bits_[0] |= 0x00000001u;
}
inline void NetdiskResponse::clear_has_opcode() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void NetdiskResponse::clear_opcode() {
  opcode_ = 0;
  clear_has_opcode();
}
inline ::com::caredear::Opcode NetdiskResponse::opcode() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskResponse.opcode)
  return static_cast< ::com::caredear::Opcode >(opcode_);
}
inline void NetdiskResponse::set_opcode(::com::caredear::Opcode value) {
  assert(::com::caredear::Opcode_IsValid(value));
  set_has_opcode();
  opcode_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskResponse.opcode)
}

// required int32 result_code = 2;
inline bool NetdiskResponse::has_result_code() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void NetdiskResponse::set_has_result_code() {
  _has_bits_[0] |= 0x00000002u;
}
inline void NetdiskResponse::clear_has_result_code() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void NetdiskResponse::clear_result_code() {
  result_code_ = 0;
  clear_has_result_code();
}
inline ::google::protobuf::int32 NetdiskResponse::result_code() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskResponse.result_code)
  return result_code_;
}
inline void NetdiskResponse::set_result_code(::google::protobuf::int32 value) {
  set_has_result_code();
  result_code_ = value;
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskResponse.result_code)
}

// optional string errormsg = 3;
inline bool NetdiskResponse::has_errormsg() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void NetdiskResponse::set_has_errormsg() {
  _has_bits_[0] |= 0x00000004u;
}
inline void NetdiskResponse::clear_has_errormsg() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void NetdiskResponse::clear_errormsg() {
  if (errormsg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    errormsg_->clear();
  }
  clear_has_errormsg();
}
inline const ::std::string& NetdiskResponse::errormsg() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskResponse.errormsg)
  return *errormsg_;
}
inline void NetdiskResponse::set_errormsg(const ::std::string& value) {
  set_has_errormsg();
  if (errormsg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    errormsg_ = new ::std::string;
  }
  errormsg_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskResponse.errormsg)
}
inline void NetdiskResponse::set_errormsg(const char* value) {
  set_has_errormsg();
  if (errormsg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    errormsg_ = new ::std::string;
  }
  errormsg_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskResponse.errormsg)
}
inline void NetdiskResponse::set_errormsg(const char* value, size_t size) {
  set_has_errormsg();
  if (errormsg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    errormsg_ = new ::std::string;
  }
  errormsg_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskResponse.errormsg)
}
inline ::std::string* NetdiskResponse::mutable_errormsg() {
  set_has_errormsg();
  if (errormsg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    errormsg_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskResponse.errormsg)
  return errormsg_;
}
inline ::std::string* NetdiskResponse::release_errormsg() {
  clear_has_errormsg();
  if (errormsg_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = errormsg_;
    errormsg_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskResponse::set_allocated_errormsg(::std::string* errormsg) {
  if (errormsg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete errormsg_;
  }
  if (errormsg) {
    set_has_errormsg();
    errormsg_ = errormsg;
  } else {
    clear_has_errormsg();
    errormsg_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskResponse.errormsg)
}

// optional string uploadtoken = 4;
inline bool NetdiskResponse::has_uploadtoken() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void NetdiskResponse::set_has_uploadtoken() {
  _has_bits_[0] |= 0x00000008u;
}
inline void NetdiskResponse::clear_has_uploadtoken() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void NetdiskResponse::clear_uploadtoken() {
  if (uploadtoken_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    uploadtoken_->clear();
  }
  clear_has_uploadtoken();
}
inline const ::std::string& NetdiskResponse::uploadtoken() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskResponse.uploadtoken)
  return *uploadtoken_;
}
inline void NetdiskResponse::set_uploadtoken(const ::std::string& value) {
  set_has_uploadtoken();
  if (uploadtoken_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    uploadtoken_ = new ::std::string;
  }
  uploadtoken_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskResponse.uploadtoken)
}
inline void NetdiskResponse::set_uploadtoken(const char* value) {
  set_has_uploadtoken();
  if (uploadtoken_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    uploadtoken_ = new ::std::string;
  }
  uploadtoken_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskResponse.uploadtoken)
}
inline void NetdiskResponse::set_uploadtoken(const char* value, size_t size) {
  set_has_uploadtoken();
  if (uploadtoken_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    uploadtoken_ = new ::std::string;
  }
  uploadtoken_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskResponse.uploadtoken)
}
inline ::std::string* NetdiskResponse::mutable_uploadtoken() {
  set_has_uploadtoken();
  if (uploadtoken_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    uploadtoken_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskResponse.uploadtoken)
  return uploadtoken_;
}
inline ::std::string* NetdiskResponse::release_uploadtoken() {
  clear_has_uploadtoken();
  if (uploadtoken_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = uploadtoken_;
    uploadtoken_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskResponse::set_allocated_uploadtoken(::std::string* uploadtoken) {
  if (uploadtoken_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete uploadtoken_;
  }
  if (uploadtoken) {
    set_has_uploadtoken();
    uploadtoken_ = uploadtoken;
  } else {
    clear_has_uploadtoken();
    uploadtoken_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskResponse.uploadtoken)
}

// optional string downloadurl = 5;
inline bool NetdiskResponse::has_downloadurl() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void NetdiskResponse::set_has_downloadurl() {
  _has_bits_[0] |= 0x00000010u;
}
inline void NetdiskResponse::clear_has_downloadurl() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void NetdiskResponse::clear_downloadurl() {
  if (downloadurl_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    downloadurl_->clear();
  }
  clear_has_downloadurl();
}
inline const ::std::string& NetdiskResponse::downloadurl() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskResponse.downloadurl)
  return *downloadurl_;
}
inline void NetdiskResponse::set_downloadurl(const ::std::string& value) {
  set_has_downloadurl();
  if (downloadurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    downloadurl_ = new ::std::string;
  }
  downloadurl_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskResponse.downloadurl)
}
inline void NetdiskResponse::set_downloadurl(const char* value) {
  set_has_downloadurl();
  if (downloadurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    downloadurl_ = new ::std::string;
  }
  downloadurl_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskResponse.downloadurl)
}
inline void NetdiskResponse::set_downloadurl(const char* value, size_t size) {
  set_has_downloadurl();
  if (downloadurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    downloadurl_ = new ::std::string;
  }
  downloadurl_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskResponse.downloadurl)
}
inline ::std::string* NetdiskResponse::mutable_downloadurl() {
  set_has_downloadurl();
  if (downloadurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    downloadurl_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskResponse.downloadurl)
  return downloadurl_;
}
inline ::std::string* NetdiskResponse::release_downloadurl() {
  clear_has_downloadurl();
  if (downloadurl_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = downloadurl_;
    downloadurl_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskResponse::set_allocated_downloadurl(::std::string* downloadurl) {
  if (downloadurl_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete downloadurl_;
  }
  if (downloadurl) {
    set_has_downloadurl();
    downloadurl_ = downloadurl;
  } else {
    clear_has_downloadurl();
    downloadurl_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskResponse.downloadurl)
}

// optional string netdisckey = 6;
inline bool NetdiskResponse::has_netdisckey() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void NetdiskResponse::set_has_netdisckey() {
  _has_bits_[0] |= 0x00000020u;
}
inline void NetdiskResponse::clear_has_netdisckey() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void NetdiskResponse::clear_netdisckey() {
  if (netdisckey_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    netdisckey_->clear();
  }
  clear_has_netdisckey();
}
inline const ::std::string& NetdiskResponse::netdisckey() const {
  // @@protoc_insertion_point(field_get:com.caredear.NetdiskResponse.netdisckey)
  return *netdisckey_;
}
inline void NetdiskResponse::set_netdisckey(const ::std::string& value) {
  set_has_netdisckey();
  if (netdisckey_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    netdisckey_ = new ::std::string;
  }
  netdisckey_->assign(value);
  // @@protoc_insertion_point(field_set:com.caredear.NetdiskResponse.netdisckey)
}
inline void NetdiskResponse::set_netdisckey(const char* value) {
  set_has_netdisckey();
  if (netdisckey_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    netdisckey_ = new ::std::string;
  }
  netdisckey_->assign(value);
  // @@protoc_insertion_point(field_set_char:com.caredear.NetdiskResponse.netdisckey)
}
inline void NetdiskResponse::set_netdisckey(const char* value, size_t size) {
  set_has_netdisckey();
  if (netdisckey_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    netdisckey_ = new ::std::string;
  }
  netdisckey_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:com.caredear.NetdiskResponse.netdisckey)
}
inline ::std::string* NetdiskResponse::mutable_netdisckey() {
  set_has_netdisckey();
  if (netdisckey_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    netdisckey_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:com.caredear.NetdiskResponse.netdisckey)
  return netdisckey_;
}
inline ::std::string* NetdiskResponse::release_netdisckey() {
  clear_has_netdisckey();
  if (netdisckey_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = netdisckey_;
    netdisckey_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void NetdiskResponse::set_allocated_netdisckey(::std::string* netdisckey) {
  if (netdisckey_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete netdisckey_;
  }
  if (netdisckey) {
    set_has_netdisckey();
    netdisckey_ = netdisckey;
  } else {
    clear_has_netdisckey();
    netdisckey_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:com.caredear.NetdiskResponse.netdisckey)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace caredear
}  // namespace com

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::com::caredear::Opcode> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::com::caredear::Opcode>() {
  return ::com::caredear::Opcode_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_NetdiskMessage_2eproto__INCLUDED
