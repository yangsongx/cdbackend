// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: UserRegister.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "UserRegister.pb.h"

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

const ::google::protobuf::Descriptor* RegisterRequest_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  RegisterRequest_reflection_ = NULL;
const ::google::protobuf::Descriptor* RegisterResponse_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  RegisterResponse_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* Regtype_descriptor_ = NULL;
const ::google::protobuf::EnumDescriptor* DeviceType_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_UserRegister_2eproto() {
  protobuf_AddDesc_UserRegister_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "UserRegister.proto");
  GOOGLE_CHECK(file != NULL);
  RegisterRequest_descriptor_ = file->message_type(0);
  static const int RegisterRequest_offsets_[5] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, reg_type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, reg_device_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, reg_source_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, reg_name_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, reg_password_),
  };
  RegisterRequest_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      RegisterRequest_descriptor_,
      RegisterRequest::default_instance_,
      RegisterRequest_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterRequest, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(RegisterRequest));
  RegisterResponse_descriptor_ = file->message_type(1);
  static const int RegisterResponse_offsets_[4] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterResponse, result_code_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterResponse, extra_msg_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterResponse, reg_verifycode_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterResponse, caredear_id_),
  };
  RegisterResponse_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      RegisterResponse_descriptor_,
      RegisterResponse::default_instance_,
      RegisterResponse_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterResponse, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(RegisterResponse, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(RegisterResponse));
  Regtype_descriptor_ = file->enum_type(0);
  DeviceType_descriptor_ = file->enum_type(1);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_UserRegister_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    RegisterRequest_descriptor_, &RegisterRequest::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    RegisterResponse_descriptor_, &RegisterResponse::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_UserRegister_2eproto() {
  delete RegisterRequest::default_instance_;
  delete RegisterRequest_reflection_;
  delete RegisterResponse::default_instance_;
  delete RegisterResponse_reflection_;
}

void protobuf_AddDesc_UserRegister_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\022UserRegister.proto\022\014com.caredear\"\244\001\n\017R"
    "egisterRequest\022\'\n\010reg_type\030\001 \002(\0162\025.com.c"
    "aredear.Regtype\022,\n\nreg_device\030\002 \002(\0162\030.co"
    "m.caredear.DeviceType\022\022\n\nreg_source\030\003 \002("
    "\t\022\020\n\010reg_name\030\004 \002(\t\022\024\n\014reg_password\030\005 \001("
    "\t\"g\n\020RegisterResponse\022\023\n\013result_code\030\001 \002"
    "(\005\022\021\n\textra_msg\030\002 \001(\t\022\026\n\016reg_verifycode\030"
    "\003 \001(\t\022\023\n\013caredear_id\030\004 \001(\t*A\n\007Regtype\022\020\n"
    "\014MOBILE_PHONE\020\001\022\t\n\005EMAIL\020\002\022\r\n\tUSER_NAME\020"
    "\003\022\n\n\006OTHERS\020\004*:\n\nDeviceType\022\013\n\007ANDROID\020\000"
    "\022\007\n\003IOS\020\001\022\016\n\nCAREDEAROS\020\002\022\006\n\002PC\020\003", 433);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "UserRegister.proto", &protobuf_RegisterTypes);
  RegisterRequest::default_instance_ = new RegisterRequest();
  RegisterResponse::default_instance_ = new RegisterResponse();
  RegisterRequest::default_instance_->InitAsDefaultInstance();
  RegisterResponse::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_UserRegister_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_UserRegister_2eproto {
  StaticDescriptorInitializer_UserRegister_2eproto() {
    protobuf_AddDesc_UserRegister_2eproto();
  }
} static_descriptor_initializer_UserRegister_2eproto_;
const ::google::protobuf::EnumDescriptor* Regtype_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Regtype_descriptor_;
}
bool Regtype_IsValid(int value) {
  switch(value) {
    case 1:
    case 2:
    case 3:
    case 4:
      return true;
    default:
      return false;
  }
}

const ::google::protobuf::EnumDescriptor* DeviceType_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return DeviceType_descriptor_;
}
bool DeviceType_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
    case 2:
    case 3:
      return true;
    default:
      return false;
  }
}


// ===================================================================

#ifndef _MSC_VER
const int RegisterRequest::kRegTypeFieldNumber;
const int RegisterRequest::kRegDeviceFieldNumber;
const int RegisterRequest::kRegSourceFieldNumber;
const int RegisterRequest::kRegNameFieldNumber;
const int RegisterRequest::kRegPasswordFieldNumber;
#endif  // !_MSC_VER

RegisterRequest::RegisterRequest()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:com.caredear.RegisterRequest)
}

void RegisterRequest::InitAsDefaultInstance() {
}

RegisterRequest::RegisterRequest(const RegisterRequest& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:com.caredear.RegisterRequest)
}

void RegisterRequest::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  reg_type_ = 1;
  reg_device_ = 0;
  reg_source_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  reg_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  reg_password_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

RegisterRequest::~RegisterRequest() {
  // @@protoc_insertion_point(destructor:com.caredear.RegisterRequest)
  SharedDtor();
}

void RegisterRequest::SharedDtor() {
  if (reg_source_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_source_;
  }
  if (reg_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_name_;
  }
  if (reg_password_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_password_;
  }
  if (this != default_instance_) {
  }
}

void RegisterRequest::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* RegisterRequest::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return RegisterRequest_descriptor_;
}

const RegisterRequest& RegisterRequest::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_UserRegister_2eproto();
  return *default_instance_;
}

RegisterRequest* RegisterRequest::default_instance_ = NULL;

RegisterRequest* RegisterRequest::New() const {
  return new RegisterRequest;
}

void RegisterRequest::Clear() {
  if (_has_bits_[0 / 32] & 31) {
    reg_type_ = 1;
    reg_device_ = 0;
    if (has_reg_source()) {
      if (reg_source_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        reg_source_->clear();
      }
    }
    if (has_reg_name()) {
      if (reg_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        reg_name_->clear();
      }
    }
    if (has_reg_password()) {
      if (reg_password_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        reg_password_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool RegisterRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:com.caredear.RegisterRequest)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .com.caredear.Regtype reg_type = 1;
      case 1: {
        if (tag == 8) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::com::caredear::Regtype_IsValid(value)) {
            set_reg_type(static_cast< ::com::caredear::Regtype >(value));
          } else {
            mutable_unknown_fields()->AddVarint(1, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(16)) goto parse_reg_device;
        break;
      }

      // required .com.caredear.DeviceType reg_device = 2;
      case 2: {
        if (tag == 16) {
         parse_reg_device:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::com::caredear::DeviceType_IsValid(value)) {
            set_reg_device(static_cast< ::com::caredear::DeviceType >(value));
          } else {
            mutable_unknown_fields()->AddVarint(2, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(26)) goto parse_reg_source;
        break;
      }

      // required string reg_source = 3;
      case 3: {
        if (tag == 26) {
         parse_reg_source:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_reg_source()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->reg_source().data(), this->reg_source().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "reg_source");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(34)) goto parse_reg_name;
        break;
      }

      // required string reg_name = 4;
      case 4: {
        if (tag == 34) {
         parse_reg_name:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_reg_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->reg_name().data(), this->reg_name().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "reg_name");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(42)) goto parse_reg_password;
        break;
      }

      // optional string reg_password = 5;
      case 5: {
        if (tag == 42) {
         parse_reg_password:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_reg_password()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->reg_password().data(), this->reg_password().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "reg_password");
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
  // @@protoc_insertion_point(parse_success:com.caredear.RegisterRequest)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:com.caredear.RegisterRequest)
  return false;
#undef DO_
}

void RegisterRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:com.caredear.RegisterRequest)
  // required .com.caredear.Regtype reg_type = 1;
  if (has_reg_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      1, this->reg_type(), output);
  }

  // required .com.caredear.DeviceType reg_device = 2;
  if (has_reg_device()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      2, this->reg_device(), output);
  }

  // required string reg_source = 3;
  if (has_reg_source()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_source().data(), this->reg_source().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_source");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      3, this->reg_source(), output);
  }

  // required string reg_name = 4;
  if (has_reg_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_name().data(), this->reg_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_name");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      4, this->reg_name(), output);
  }

  // optional string reg_password = 5;
  if (has_reg_password()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_password().data(), this->reg_password().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_password");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      5, this->reg_password(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:com.caredear.RegisterRequest)
}

::google::protobuf::uint8* RegisterRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:com.caredear.RegisterRequest)
  // required .com.caredear.Regtype reg_type = 1;
  if (has_reg_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      1, this->reg_type(), target);
  }

  // required .com.caredear.DeviceType reg_device = 2;
  if (has_reg_device()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      2, this->reg_device(), target);
  }

  // required string reg_source = 3;
  if (has_reg_source()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_source().data(), this->reg_source().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_source");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->reg_source(), target);
  }

  // required string reg_name = 4;
  if (has_reg_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_name().data(), this->reg_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_name");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->reg_name(), target);
  }

  // optional string reg_password = 5;
  if (has_reg_password()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_password().data(), this->reg_password().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_password");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        5, this->reg_password(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:com.caredear.RegisterRequest)
  return target;
}

int RegisterRequest::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required .com.caredear.Regtype reg_type = 1;
    if (has_reg_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->reg_type());
    }

    // required .com.caredear.DeviceType reg_device = 2;
    if (has_reg_device()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->reg_device());
    }

    // required string reg_source = 3;
    if (has_reg_source()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->reg_source());
    }

    // required string reg_name = 4;
    if (has_reg_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->reg_name());
    }

    // optional string reg_password = 5;
    if (has_reg_password()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->reg_password());
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

void RegisterRequest::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const RegisterRequest* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const RegisterRequest*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void RegisterRequest::MergeFrom(const RegisterRequest& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_reg_type()) {
      set_reg_type(from.reg_type());
    }
    if (from.has_reg_device()) {
      set_reg_device(from.reg_device());
    }
    if (from.has_reg_source()) {
      set_reg_source(from.reg_source());
    }
    if (from.has_reg_name()) {
      set_reg_name(from.reg_name());
    }
    if (from.has_reg_password()) {
      set_reg_password(from.reg_password());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void RegisterRequest::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void RegisterRequest::CopyFrom(const RegisterRequest& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool RegisterRequest::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000000f) != 0x0000000f) return false;

  return true;
}

void RegisterRequest::Swap(RegisterRequest* other) {
  if (other != this) {
    std::swap(reg_type_, other->reg_type_);
    std::swap(reg_device_, other->reg_device_);
    std::swap(reg_source_, other->reg_source_);
    std::swap(reg_name_, other->reg_name_);
    std::swap(reg_password_, other->reg_password_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata RegisterRequest::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = RegisterRequest_descriptor_;
  metadata.reflection = RegisterRequest_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int RegisterResponse::kResultCodeFieldNumber;
const int RegisterResponse::kExtraMsgFieldNumber;
const int RegisterResponse::kRegVerifycodeFieldNumber;
const int RegisterResponse::kCaredearIdFieldNumber;
#endif  // !_MSC_VER

RegisterResponse::RegisterResponse()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:com.caredear.RegisterResponse)
}

void RegisterResponse::InitAsDefaultInstance() {
}

RegisterResponse::RegisterResponse(const RegisterResponse& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:com.caredear.RegisterResponse)
}

void RegisterResponse::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  result_code_ = 0;
  extra_msg_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  reg_verifycode_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  caredear_id_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

RegisterResponse::~RegisterResponse() {
  // @@protoc_insertion_point(destructor:com.caredear.RegisterResponse)
  SharedDtor();
}

void RegisterResponse::SharedDtor() {
  if (extra_msg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete extra_msg_;
  }
  if (reg_verifycode_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete reg_verifycode_;
  }
  if (caredear_id_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete caredear_id_;
  }
  if (this != default_instance_) {
  }
}

void RegisterResponse::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* RegisterResponse::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return RegisterResponse_descriptor_;
}

const RegisterResponse& RegisterResponse::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_UserRegister_2eproto();
  return *default_instance_;
}

RegisterResponse* RegisterResponse::default_instance_ = NULL;

RegisterResponse* RegisterResponse::New() const {
  return new RegisterResponse;
}

void RegisterResponse::Clear() {
  if (_has_bits_[0 / 32] & 15) {
    result_code_ = 0;
    if (has_extra_msg()) {
      if (extra_msg_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        extra_msg_->clear();
      }
    }
    if (has_reg_verifycode()) {
      if (reg_verifycode_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        reg_verifycode_->clear();
      }
    }
    if (has_caredear_id()) {
      if (caredear_id_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        caredear_id_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool RegisterResponse::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:com.caredear.RegisterResponse)
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
        if (input->ExpectTag(26)) goto parse_reg_verifycode;
        break;
      }

      // optional string reg_verifycode = 3;
      case 3: {
        if (tag == 26) {
         parse_reg_verifycode:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_reg_verifycode()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->reg_verifycode().data(), this->reg_verifycode().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "reg_verifycode");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(34)) goto parse_caredear_id;
        break;
      }

      // optional string caredear_id = 4;
      case 4: {
        if (tag == 34) {
         parse_caredear_id:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_caredear_id()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->caredear_id().data(), this->caredear_id().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "caredear_id");
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
  // @@protoc_insertion_point(parse_success:com.caredear.RegisterResponse)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:com.caredear.RegisterResponse)
  return false;
#undef DO_
}

void RegisterResponse::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:com.caredear.RegisterResponse)
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

  // optional string reg_verifycode = 3;
  if (has_reg_verifycode()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_verifycode().data(), this->reg_verifycode().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_verifycode");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      3, this->reg_verifycode(), output);
  }

  // optional string caredear_id = 4;
  if (has_caredear_id()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->caredear_id().data(), this->caredear_id().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "caredear_id");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      4, this->caredear_id(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:com.caredear.RegisterResponse)
}

::google::protobuf::uint8* RegisterResponse::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:com.caredear.RegisterResponse)
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

  // optional string reg_verifycode = 3;
  if (has_reg_verifycode()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->reg_verifycode().data(), this->reg_verifycode().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "reg_verifycode");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->reg_verifycode(), target);
  }

  // optional string caredear_id = 4;
  if (has_caredear_id()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->caredear_id().data(), this->caredear_id().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "caredear_id");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->caredear_id(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:com.caredear.RegisterResponse)
  return target;
}

int RegisterResponse::ByteSize() const {
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

    // optional string reg_verifycode = 3;
    if (has_reg_verifycode()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->reg_verifycode());
    }

    // optional string caredear_id = 4;
    if (has_caredear_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->caredear_id());
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

void RegisterResponse::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const RegisterResponse* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const RegisterResponse*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void RegisterResponse::MergeFrom(const RegisterResponse& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_result_code()) {
      set_result_code(from.result_code());
    }
    if (from.has_extra_msg()) {
      set_extra_msg(from.extra_msg());
    }
    if (from.has_reg_verifycode()) {
      set_reg_verifycode(from.reg_verifycode());
    }
    if (from.has_caredear_id()) {
      set_caredear_id(from.caredear_id());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void RegisterResponse::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void RegisterResponse::CopyFrom(const RegisterResponse& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool RegisterResponse::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;

  return true;
}

void RegisterResponse::Swap(RegisterResponse* other) {
  if (other != this) {
    std::swap(result_code_, other->result_code_);
    std::swap(extra_msg_, other->extra_msg_);
    std::swap(reg_verifycode_, other->reg_verifycode_);
    std::swap(caredear_id_, other->caredear_id_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata RegisterResponse::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = RegisterResponse_descriptor_;
  metadata.reflection = RegisterResponse_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace caredear
}  // namespace com

// @@protoc_insertion_point(global_scope)
