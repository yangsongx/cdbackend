#ifndef _VCS_LOGINOPER_H
#define _VCS_LOGINOPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "VerifyCode.pb.h"
#include "VerifyCodeConfig.h"
#include "uuid.h"


using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class VerifyCodeOperation {

    VerifyCodeConfig *m_cfgInfo;

    //int update_usercenter_session(LoginRequest *reqobj, struct user_session *u);

public:
    int set_conf(VerifyCodeConfig *c);
    int compose_result(int code, const char *errmsg, UpdateResponse *p_obj, int *p_resplen, void *p_respdata);

    int gen_verifycode(char *result);
    int do_update_vcode(UpdateRequest *reqobj, UpdateResponse *respobj, int *len_resp, void *resp, char *vcode);
};

#endif
