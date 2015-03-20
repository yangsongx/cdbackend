// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: CommonUserCenter.proto

#ifndef PROTOBUF_CommonUserCenter_2eproto__INCLUDED
#define PROTOBUF_CommonUserCenter_2eproto__INCLUDED

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
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace caredear {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_CommonUserCenter_2eproto();
void protobuf_AssignDesc_CommonUserCenter_2eproto();
void protobuf_ShutdownFile_CommonUserCenter_2eproto();


enum RegLoginType {
  MOBILE_PHONE = 1,
  EMAIL_PASSWD = 2,
  NAME_PASSWD = 3,
  PHONE_PASSWD = 4,
  OTHERS = 5,
  CID_PASSWD = 6,
  LOG_OUT = 7
};
bool RegLoginType_IsValid(int value);
const RegLoginType RegLoginType_MIN = MOBILE_PHONE;
const RegLoginType RegLoginType_MAX = LOG_OUT;
const int RegLoginType_ARRAYSIZE = RegLoginType_MAX + 1;

const ::google::protobuf::EnumDescriptor* RegLoginType_descriptor();
inline const ::std::string& RegLoginType_Name(RegLoginType value) {
  return ::google::protobuf::internal::NameOfEnum(
    RegLoginType_descriptor(), value);
}
inline bool RegLoginType_Parse(
    const ::std::string& name, RegLoginType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<RegLoginType>(
    RegLoginType_descriptor(), name, value);
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


// ===================================================================


// ===================================================================


// @@protoc_insertion_point(namespace_scope)

}  // namespace caredear
}  // namespace com

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::com::caredear::RegLoginType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::com::caredear::RegLoginType>() {
  return ::com::caredear::RegLoginType_descriptor();
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

#endif  // PROTOBUF_CommonUserCenter_2eproto__INCLUDED
