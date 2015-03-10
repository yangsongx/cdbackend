#include "AuthOperation.h"

#include "usr_auth_db.h"

/**
 * inner wrapper token time compare logic, including both memcach+SQL
 *
 * return 0 means don'd update time, 1 means need update time, -1 means
 * user's token expired
 */
int AuthOperation::check_token(AuthRequest *reqobj, time_t last_login)
{
    time_t current;
    session_db_cfg_t c;
    map<int, session_db_cfg_t>::iterator it;

    time(&current);
    if((current - last_login) <= MAX_FREQUENT_VISIT)
    {
        INFO("too frequently change on lastoperatetime, dont' update any time\n");
        return 0;
    }

    // map is : <sysid> --> <allow?expire?type?>
    //
    it = m_cfgInfo->m_sessionCfg.find(reqobj->auth_sysid());
    if(it != m_cfgInfo->m_sessionCfg.end())
    {
        c = it->second;
    }
    else
    {
        ERR("failed found the session conf for SYSID[%d]\n",
                reqobj->auth_sysid());
        return 1; // FIXME for such error, need update time?
    }

    if((current - last_login) >= c.sfg_expiration)
    {
        INFO("Wow, your last login happen longlong ago...\n");
        return -1;
    }

    return 1;
}

bool AuthOperation::is_allowed_access(AuthRequest *reqobj)
{
    bool allowed = false;
    session_db_cfg_t c;
    map<int, session_db_cfg_t>::iterator it;

    it = m_cfgInfo->m_sessionCfg.find(reqobj->auth_sysid());
    if(it != m_cfgInfo->m_sessionCfg.end())
    {
        c = it->second;
        allowed = c.sfg_allow_multilogin == 1 ? true : false;
    }
    else
    {
        ERR("failed found the session conf for SYSID[%d]\n",
                reqobj->auth_sysid());
    }

    return allowed;
}

int AuthOperation::set_conf(UserAuthConfig *c)
{
    m_cfgInfo = c;
    return 0;
}


int AuthOperation::auth_user(AuthRequest *reqobj, AuthResponse *respobj, int *len_resp, void *resp)
{
    int ret;
    time_t last_login;

    if(!is_allowed_access(reqobj))
    {
        ERR("This sysid[%d] is not allowed!\n", reqobj->auth_sysid());
        ret = CDS_ERR_REJECT_LOGIN;
    }
    else
    {
        ret = auth_token_in_session(m_cfgInfo->m_Sql, reqobj, respobj, &last_login);
        if(ret == CDS_OK)
        {
        // do further time check and update if possible
        ret = check_token(reqobj, last_login);
        switch(ret)
        {
            case 0:
                // as too short visit time, don't update time
                ret = CDS_OK;
                break;

            case 1:
                // update time
                update_session_lastoperatetime(m_cfgInfo->m_Sql, reqobj);
                ret = CDS_OK;
                break;

            case -1:
                ret = CDS_ERR_USER_TOKEN_EXPIRED;
                break;

            default:
                break;
        }
        }
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for auth result\n");
    }

    return ret;
}

int AuthOperation::compose_result(int code, const char *errmsg, AuthResponse *p_obj, int *p_resplen, void *p_respdata)
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
