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

    p_obj->set_result_code(code);
    if(code != CDS_OK && errmsg != NULL)
    {
        p_obj->set_extra_msg(errmsg);
    }

    len = p_obj->ByteSize();
    if(len >= DATA_BUFFER_SIZE)
    {
        ERR("Attention, exceed the max len, set it to a safe len\n");
        len = 4;
    }

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
        char uuiddata[64]; // string like '3ab554e6-1533-4cea-9f6d-26edfd869c6e'
        gen_uuid(uuiddata);
        struct user_session us;
        us.us_cid = cid;
        us.us_sessionid = reqobj->login_session().c_str();
        us.us_token = uuiddata;
        update_usercenter_session(m_cfgInfo->m_Sql, &us);

        respobj->set_token(uuiddata);
        //...
    }
    else
    {
        // failure case...
        ERR("User login is incorrect!\n");
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for login result\n");
    }

    return ret;
}
