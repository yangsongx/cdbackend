// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: PasswordManager.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "PasswordManager.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace caredear {

namespace {

const ::google::protobuf::Descriptor* PasswordManagerRequest_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  PasswordManagerRequest_reflection_ = NULL;
const ::google::protobuf::Descriptor* PasswordManagerResponse_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  PasswordManagerResponse_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* PasswordType_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_PasswordManager_2eproto() {
  protobuf_AddDesc_PasswordManager_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "PasswordManager.proto");
  GOOGLE_CHECK(file != NULL);
  PasswordManagerRequest_descriptor_ = file->message_type(0);
  static const int PasswordManagerRequest_offsets_[4] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerRequest, type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerRequest, caredear_id_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerRequest, new_passwd_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerRequest, old_passwd_),
  };
  PasswordManagerRequest_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      PasswordManagerRequest_descriptor_,
      PasswordManagerRequest::default_instance_,
      PasswordManagerRequest_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerRequest, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerRequest, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(PasswordManagerRequest));
  PasswordManagerResponse_descriptor_ = file->message_type(1);
  static const int PasswordManagerResponse_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerResponse, result_code_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerResponse, extra_msg_),
  };
  PasswordManagerResponse_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      PasswordManagerResponse_descriptor_,
      PasswordManagerResponse::default_instance_,
      PasswordManagerResponse_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerResponse, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(PasswordManagerResponse, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(PasswordManagerResponse));
  PasswordType_descriptor_ = file->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_PasswordManager_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    PasswordManagerRequest_descriptor_, &PasswordManagerRequest::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    PasswordManagerResponse_descriptor_, &PasswordManagerResponse::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_PasswordManager_2eproto() {
  delete PasswordManagerRequest::default_instance_;
  delete PasswordManagerRequest_reflection_;
  delete PasswordManagerResponse::default_instance_;
  delete PasswordManagerResponse_reflection_;
}

void protobuf_AddDesc_PasswordManager_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\025PasswordManager.proto\022\014com.caredear\"\177\n"
    "\026PasswordManagerRequest\022(\n\004type\030\001 \002(\0162\032."
    "com.caredear.PasswordType\022\023\n\013caredear_id"
    "\030\002 \002(\004\022\022\n\nnew_passwd\030\003 \002(\t\022\022\n\nold_passwd"
    "\030\004 \001(\t\"A\n\027PasswordManagerResponse\022\023\n\013res"
    "ult_code\030\001 \002(\005\022\021\n\textra_msg\030\002 \001(\t*&\n\014Pas"
    "swordType\022\n\n\006MODIFY\020\000\022\n\n\006FORGET\020\001", 273);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "PasswordManager.proto", &protobuf_RegisterTypes);
  PasswordManagerRequest::default_instance_ = new PasswordManagerRequest();
  PasswordManagerResponse::default_instance_ = new PasswordManagerResponse();
  PasswordManagerRequest::default_instance_->InitAsDefaultInstance();
  PasswordManagerResponse::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_PasswordManager_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_PasswordManager_2eproto {
  StaticDescriptorInitializer_PasswordManager_2eproto() {
    protobuf_AddDesc_PasswordManager_2eproto();
  }
} static_descriptor_initializer_PasswordManager_2eproto_;
const ::google::protobuf::EnumDescriptor* PasswordType_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PasswordType_descriptor_;
}
bool PasswordType_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}


// ===================================================================

#ifndef _MSC_VER
const int PasswordManagerRequest::kTypeFieldNumber;
const int PasswordManagerRequest::kCaredearIdFieldNumber;
const int PasswordManagerRequest::kNewPasswdFieldNumber;
const int PasswordManagerRequest::kOldPasswdFieldNumber;
#endif  // !_MSC_VER

PasswordManagerRequest::PasswordManagerRequest()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:com.caredear.PasswordManagerRequest)
}

void PasswordManagerRequest::InitAsDefaultInstance() {
}

PasswordManagerRequest::PasswordManagerRequest(const PasswordManagerRequest& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:com.caredear.PasswordManagerRequest)
}

void PasswordManagerRequest::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  type_ = 0;
  caredear_id_ = GOOGLE_ULONGLONG(0);
  new_passwd_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  old_passwd_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

PasswordManagerRequest::~PasswordManagerRequest() {
  // @@protoc_insertion_point(destructor:com.caredear.PasswordManagerRequest)
  SharedDtor();
}

void PasswordManagerRequest::SharedDtor() {
  if (new_passwd_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete new_passwd_;
  }
  if (old_passwd_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete old_passwd_;
  }
  if (this != default_instance_) {
  }
}

void PasswordManagerRequest::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* PasswordManagerRequest::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PasswordManagerRequest_descriptor_;
}

const PasswordManagerRequest& PasswordManagerRequest::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_PasswordManager_2eproto();
  return *default_instance_;
}

PasswordManagerRequest* PasswordManagerRequest::default_instance_ = NULL;

PasswordManagerRequest* PasswordManagerRequest::New() const {
  return new PasswordManagerRequest;
}

void PasswordManagerRequest::Clear() {
  if (_has_bits_[0 / 32] & 15) {
    type_ = 0;
    caredear_id_ = GOOGLE_ULONGLONG(0);
    if (has_new_passwd()) {
      if (new_passwd_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        new_passwd_->clear();
      }
    }
    if (has_old_passwd()) {
      if (old_passwd_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        old_passwd_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool PasswordManagerRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:com.caredear.PasswordManagerRequest)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .com.caredear.PasswordType type = 1;
      case 1: {
        if (tag == 8) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::com::caredear::PasswordType_IsValid(value)) {
            set_type(static_cast< ::com::caredear::PasswordType >(value));
          } else {
            mutable_unknown_fields()->AddVarint(1, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(16)) goto parse_caredear_id;
        break;
      }

      // required uint64 caredear_id = 2;
      case 2: {
        if (tag == 16) {
         parse_caredear_id:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &caredear_id_)));
          set_has_caredear_id();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(26)) goto parse_new_passwd;
        break;
      }

      // required string new_passwd = 3;
      case 3: {
        if (tag == 26) {
         parse_new_passwd:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_new_passwd()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->new_passwd().data(), this->new_passwd().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "new_passwd");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(34)) goto parse_old_passwd;
        break;
      }

      // optional string old_passwd = 4;
      case 4: {
        if (tag == 34) {
         parse_old_passwd:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_old_passwd()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->old_passwd().data(), this->old_passwd().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "old_passwd");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:com.caredear.PasswordManagerRequest)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:com.caredear.PasswordManagerRequest)
  return false;
#undef DO_
}

void PasswordManagerRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:com.caredear.PasswordManagerRequest)
  // required .com.caredear.PasswordType type = 1;
  if (has_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      1, this->type(), output);
  }

  // required uint64 caredear_id = 2;
  if (has_caredear_id()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(2, this->caredear_id(), output);
  }

  // required string new_passwd = 3;
  if (has_new_passwd()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->new_passwd().data(), this->new_passwd().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "new_passwd");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      3, this->new_passwd(), output);
  }

  // optional string old_passwd = 4;
  if (has_old_passwd()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->old_passwd().data(), this->old_passwd().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "old_passwd");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      4, this->old_passwd(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:com.caredear.PasswordManagerRequest)
}

::google::protobuf::uint8* PasswordManagerRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:com.caredear.PasswordManagerRequest)
  // required .com.caredear.PasswordType type = 1;
  if (has_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      1, this->type(), target);
  }

  // required uint64 caredear_id = 2;
  if (has_caredear_id()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(2, this->caredear_id(), target);
  }

  // required string new_passwd = 3;
  if (has_new_passwd()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->new_passwd().data(), this->new_passwd().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "new_passwd");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->new_passwd(), target);
  }

  // optional string old_passwd = 4;
  if (has_old_passwd()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->old_passwd().data(), this->old_passwd().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "old_passwd");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->old_passwd(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:com.caredear.PasswordManagerRequest)
  return target;
}

int PasswordManagerRequest::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required .com.caredear.PasswordType type = 1;
    if (has_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->type());
    }

    // required uint64 caredear_id = 2;
    if (has_caredear_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->caredear_id());
    }

    // required string new_passwd = 3;
    if (has_new_passwd()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->new_passwd());
    }

    // optional string old_passwd = 4;
    if (has_old_passwd()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->old_passwd());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PasswordManagerRequest::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const PasswordManagerRequest* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const PasswordManagerRequest*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void PasswordManagerRequest::MergeFrom(const PasswordManagerRequest& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_type()) {
      set_type(from.type());
    }
    if (from.has_caredear_id()) {
      set_caredear_id(from.caredear_id());
    }
    if (from.has_new_passwd()) {
      set_new_passwd(from.new_passwd());
    }
    if (from.has_old_passwd()) {
      set_old_passwd(from.old_passwd());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void PasswordManagerRequest::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PasswordManagerRequest::CopyFrom(const PasswordManagerRequest& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PasswordManagerRequest::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;

  return true;
}

void PasswordManagerRequest::Swap(PasswordManagerRequest* other) {
  if (other != this) {
    std::swap(type_, other->type_);
    std::swap(caredear_id_, other->caredear_id_);
    std::swap(new_passwd_, other->new_passwd_);
    std::swap(old_passwd_, other->old_passwd_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata PasswordManagerRequest::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = PasswordManagerRequest_descriptor_;
  metadata.reflection = PasswordManagerRequest_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int PasswordManagerResponse::kResultCodeFieldNumber;
const int PasswordManagerResponse::kExtraMsgFieldNumber;
#endif  // !_MSC_VER

PasswordManagerResponse::PasswordManagerResponse()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:com.caredear.PasswordManagerResponse)
}

void PasswordManagerResponse::InitAsDefaultInstance() {
}

PasswordManagerResponse::PasswordManagerResponse(const PasswordManagerResponse& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:com.caredear.PasswordManagerResponse)
}

void PasswordManagerResponse::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  result_code_ = 0;
  extra_msg_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

PasswordManagerResponse::~PasswordManagerResponse() {
  // @@protoc_insertion_point(destructor:com.caredear.PasswordManagerResponse)
  SharedDtor();
}

void PasswordManagerResponse::SharedDtor() {
  if (extra_msg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete extra_msg_;
  }
  if (this != default_instance_) {
  }
}

void PasswordManagerResponse::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* PasswordManagerResponse::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return PasswordManagerResponse_descriptor_;
}

const PasswordManagerResponse& PasswordManagerResponse::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_PasswordManager_2eproto();
  return *default_instance_;
}

PasswordManagerResponse* PasswordManagerResponse::default_instance_ = NULL;

PasswordManagerResponse* PasswordManagerResponse::New() const {
  return new PasswordManagerResponse;
}

void PasswordManagerResponse::Clear() {
  if (_has_bits_[0 / 32] & 3) {
    result_code_ = 0;
    if (has_extra_msg()) {
      if (extra_msg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        extra_msg_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool PasswordManagerResponse::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:com.caredear.PasswordManagerResponse)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 result_code = 1;
      case 1: {
        if (tag == 8) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &result_code_)));
          set_has_result_code();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_extra_msg;
        break;
      }

      // optional string extra_msg = 2;
      case 2: {
        if (tag == 18) {
         parse_extra_msg:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_extra_msg()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->extra_msg().data(), this->extra_msg().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "extra_msg");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:com.caredear.PasswordManagerResponse)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:com.caredear.PasswordManagerResponse)
  return false;
#undef DO_
}

void PasswordManagerResponse::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:com.caredear.PasswordManagerResponse)
  // required int32 result_code = 1;
  if (has_result_code()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->result_code(), output);
  }

  // optional string extra_msg = 2;
  if (has_extra_msg()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->extra_msg().data(), this->extra_msg().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "extra_msg");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->extra_msg(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:com.caredear.PasswordManagerResponse)
}

::google::protobuf::uint8* PasswordManagerResponse::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:com.caredear.PasswordManagerResponse)
  // required int32 result_code = 1;
  if (has_result_code()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->result_code(), target);
  }

  // optional string extra_msg = 2;
  if (has_extra_msg()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->extra_msg().data(), this->extra_msg().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "extra_msg");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->extra_msg(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:com.caredear.PasswordManagerResponse)
  return target;
}

int PasswordManagerResponse::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 result_code = 1;
    if (has_result_code()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->result_code());
    }

    // optional string extra_msg = 2;
    if (has_extra_msg()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->extra_msg());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void PasswordManagerResponse::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const PasswordManagerResponse* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const PasswordManagerResponse*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void PasswordManagerResponse::MergeFrom(const PasswordManagerResponse& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_result_code()) {
      set_result_code(from.result_code());
    }
    if (from.has_extra_msg()) {
      set_extra_msg(from.extra_msg());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void PasswordManagerResponse::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PasswordManagerResponse::CopyFrom(const PasswordManagerResponse& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PasswordManagerResponse::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;

  return true;
}

void PasswordManagerResponse::Swap(PasswordManagerResponse* other) {
  if (other != this) {
    std::swap(result_code_, other->result_code_);
    std::swap(extra_msg_, other->extra_msg_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata PasswordManagerResponse::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = PasswordManagerResponse_descriptor_;
  metadata.reflection = PasswordManagerResponse_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace caredear
}  // namespace com

// @@protoc_insertion_point(global_scope)
