#ifndef _UAS_AUTHOPR_H
#define _UAS_AUTHOPR_H

#include "UserAuth.pb.h"

#include "UserAuthConfig.h"

using namespace std;
using namespace com::caredear;

class AuthOperation{
    UserAuthConfig *m_cfgInfo;

    int check_token();
public:
    int set_conf(UserAuthConfig *c);

    int auth_user(AuthRequest *reqobj, AuthResponse *respobj, int *len_resp, void *resp);
};

#endif
