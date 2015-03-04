#include "LoginOperation.h"
#include "usr_login_db.h"

int LoginOperation::set_conf(UserLoginConfig *c)
{
    m_cfgInfo = c;
    return 0;
}

int LoginOperation::compose_result(int code, const char *errmsg, LoginResponse *p_obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    len = p_obj->ByteSize();

    *p_resplen = (len + 2);
    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);
    // add the leading-length
    cos.WriteRaw(&len, sizeof(len));

    return ((p_obj->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}


/**
 * Begin do the User's Login action
 *
 *
 */
int LoginOperation::do_login(LoginRequest *reqobj, LoginResponse *respobj, int *len_resp, void *resp)
{
    int ret = -1;
    unsigned long cid = -1;

    ret = match_user_credential_in_db(m_cfgInfo->m_Sql, reqobj, &cid);
    if(ret == CDS_OK)
    {
        // login info data is correct.
        INFO("%s ==> %ld,[Login OK]\n", reqobj->login_name().c_str(), cid);
        //...
    }
    else
    {
        // failure case...
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for login result\n");
    }

    return ret;
}
