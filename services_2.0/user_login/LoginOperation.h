#ifndef _ULS_LOGINOPER_H
#define _ULS_LOGINOPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UserLogin.pb.h"
#include "UserLoginConfig.h"
#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class LoginOperation : public com::caredear::Operation {

    int update_usercenter_session(LoginRequest *reqobj, struct user_session *u);

    int delete_usercenter_session(LoginRequest *reqobj);
    int process_user_and_credential(LoginRequest *reqobj, LoginResponse *respobj);

public:
    LoginOperation() {
    }

    LoginOperation(Config *c) : Operation(c) {
    }

    virtual int handling_request(::google::protobuf::Message *reg_req,
            ::google::protobuf::Message *reg_resp,
            int *len_resp,
            void *resp);

    virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *obj,
            int *p_resplen,
            void *p_respdata);

};

#endif
