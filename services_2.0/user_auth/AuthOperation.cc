#include "AuthOperation.h"

/**
 * inner wrapper token get logic, including both memcach+SQL
 *
 */
int AuthOperation::check_token()
{
    return 0;
}

int AuthOperation::set_conf(UserAuthConfig *c)
{
    m_cfgInfo = c;
    return 0;
}


int AuthOperation::auth_user(AuthRequest *reqobj, AuthResponse *respobj, int *len_resp, void *resp)
{
    // check via token+session id
    return 0;
}
