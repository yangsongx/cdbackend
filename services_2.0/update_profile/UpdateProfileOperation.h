#ifndef _UUP_OPER_H
#define _UUP_OPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UpdateProfile.pb.h"
#include "UpdateProfileConfig.h"


using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class UpdateProfileOperation {

    UpdateProfileConfig *m_cfgInfo;

    //int update_usercenter_session(LoginRequest *reqobj, struct user_session *u);

public:
    int set_conf(UpdateProfileConfig *c);
    int compose_result(int code, const char *errmsg, UpdateResponse *p_obj, int *p_resplen, void *p_respdata);
   // int check_user_vcode(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, UpdateProfileConfig *config);
};

#endif
