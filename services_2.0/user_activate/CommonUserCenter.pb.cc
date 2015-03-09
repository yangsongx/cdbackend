// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: CommonUserCenter.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "CommonUserCenter.pb.h"

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

const ::google::protobuf::EnumDescriptor* RegLoginType_descriptor_ = NULL;
const ::google::protobuf::EnumDescriptor* DeviceType_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_CommonUserCenter_2eproto() {
  protobuf_AddDesc_CommonUserCenter_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "CommonUserCenter.proto");
  GOOGLE_CHECK(file != NULL);
  RegLoginType_descriptor_ = file->enum_type(0);
  DeviceType_descriptor_ = file->enum_type(1);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_CommonUserCenter_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
}

}  // namespace

void protobuf_ShutdownFile_CommonUserCenter_2eproto() {
}

void protobuf_AddDesc_CommonUserCenter_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\026CommonUserCenter.proto\022\014com.caredear*q"
    "\n\014RegLoginType\022\020\n\014MOBILE_PHONE\020\001\022\020\n\014EMAI"
    "L_PASSWD\020\002\022\017\n\013NAME_PASSWD\020\003\022\020\n\014PHONE_PAS"
    "SWD\020\004\022\n\n\006OTHERS\020\005\022\016\n\nCID_PASSWD\020\006*:\n\nDev"
    "iceType\022\013\n\007ANDROID\020\000\022\007\n\003IOS\020\001\022\016\n\nCAREDEA"
    "ROS\020\002\022\006\n\002PC\020\003", 213);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "CommonUserCenter.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_CommonUserCenter_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_CommonUserCenter_2eproto {
  StaticDescriptorInitializer_CommonUserCenter_2eproto() {
    protobuf_AddDesc_CommonUserCenter_2eproto();
  }
} static_descriptor_initializer_CommonUserCenter_2eproto_;
const ::google::protobuf::EnumDescriptor* RegLoginType_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return RegLoginType_descriptor_;
}
bool RegLoginType_IsValid(int value) {
  switch(value) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
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


// @@protoc_insertion_point(namespace_scope)

}  // namespace caredear
}  // namespace com

// @@protoc_insertion_point(global_scope)
