#ifndef _UAS_AUTHOPR_H
#define _UAS_AUTHOPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UserAuth.pb.h"

#include "UserAuthConfig.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class AuthOperation{
    UserAuthConfig *m_cfgInfo;

    int check_token(AuthRequest *reqobj, time_t last_login);
    bool is_allowed_access(AuthRequest *reqobj);

public:
    int set_conf(UserAuthConfig *c);

    int auth_user(AuthRequest *reqobj, AuthResponse *respobj, int *len_resp, void *resp);

    int compose_result(int code, const char *errmsg, AuthResponse *p_obj, int *p_resplen, void *p_respdata);
};

#endif
