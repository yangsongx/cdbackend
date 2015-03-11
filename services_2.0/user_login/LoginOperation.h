#ifndef _ULS_LOGINOPER_H
#define _ULS_LOGINOPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UserLogin.pb.h"
#include "UserLoginConfig.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class LoginOperation {

    UserLoginConfig *m_cfgInfo;

    int update_usercenter_session(LoginRequest *reqobj, struct user_session *u);

public:
    int set_conf(UserLoginConfig *c);
    int compose_result(int code, const char *errmsg, LoginResponse *p_obj, int *p_resplen, void *p_respdata);


    int do_login(LoginRequest *reqobj, LoginResponse *respobj, int *len_resp, void *resp);
};

#endif
