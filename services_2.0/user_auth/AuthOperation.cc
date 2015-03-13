#include "AuthOperation.h"

#include "usr_auth_db.h"

/**
 * inner wrapper token time compare logic, including both memcach+SQL
 *
 * return 0 means don'd update time, 1 means need update time, -1 means
 * user's token expired
 */
int AuthOperation::check_token(AuthRequest *reqobj, struct auth_data_wrapper *w)
{
    time_t current;
    session_db_cfg_t c;
    map<int, session_db_cfg_t>::iterator it;
    UserAuthConfig *conf = (UserAuthConfig *)m_pCfg;

    time(&current);
    if((current - w->adw_lastlogin) <= MAX_FREQUENT_VISIT)
    {
        INFO("too frequently change on lastoperatetime, dont' update any time\n");
        return 0;
    }

    // map is : <sysid> --> <allow?expire?type?>
    //
    it = conf->m_sessionCfg.find(reqobj->auth_sysid());
    if(it != conf->m_sessionCfg.end())
    {
        c = it->second;
    }
    else
    {
        ERR("failed found the session conf for SYSID[%d]\n",
                reqobj->auth_sysid());
        return 1; // FIXME for such error, need update time?
    }

    if((current - w->adw_lastlogin) >= c.sfg_expiration)
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
    UserAuthConfig *conf = (UserAuthConfig *)m_pCfg;

    it = conf->m_sessionCfg.find(reqobj->auth_sysid());
    if(it != conf->m_sessionCfg.end())
    {
        c = it->second;
        allowed = c.sfg_allow_multilogin == 1 ? true : false;
    }
    else
    {
        ERR("failed found the session conf for SESSION[%s] SYSID[%d]\n",
                reqobj->auth_session().c_str(), reqobj->auth_sysid());
    }

    return allowed;
}

/**
 * Wrapper util, including memcached+DB
 *
 */
int AuthOperation::auth_token_in_session(AuthRequest *reqobj, AuthResponse *respobj, struct auth_data_wrapper *w)
{
    int ret = CDS_OK;
    char  *p_val;
    size_t val_len;


    // first try from mem
    p_val = get_token_info_from_mem(m_pCfg->m_Memc, reqobj->auth_token().c_str(), &val_len);
    if(p_val)
    {
        // got value from mem, parse it...
        split_val_into_fields(p_val, w);

        if(strcmp(reqobj->auth_session().c_str(), w->adw_session))
        {
            ERR("(%s %s) <---->mem(%s), SESSION-mismacth, re-count on DB..\n",
                    reqobj->auth_token().c_str(), reqobj->auth_session().c_str(),
                    w->adw_session);

            ret = get_token_info_from_db(m_pCfg->m_Sql, reqobj, respobj, w);
            if(ret == CDS_ERR_SQL_DISCONNECTED)
            {
                if(m_pCfg->reconnect_sql() == 0)
                {
                    ret = get_token_info_from_db(m_pCfg->m_Sql, reqobj, respobj, w);
                }
            }
        }
        free(p_val);
    }
    else
    {
        // didn't get the mem val, try on DB...
        INFO("not found in mem, go to DB...\n");
        ret = get_token_info_from_db(m_pCfg->m_Sql, reqobj, respobj, w);
        if(ret == CDS_ERR_SQL_DISCONNECTED)
        {
            if(m_pCfg->reconnect_sql() == 0)
            {
                ret = get_token_info_from_db(m_pCfg->m_Sql, reqobj, respobj, w);
            }
        }
    }

    return ret;
}

/**
 * Update both Mem and Sql
 *
 */
int AuthOperation::update_session_lastoperatetime(AuthRequest *reqobj, struct auth_data_wrapper *w)
{
    memcached_return_t rc;

    // mem
    rc = set_token_info_to_mem(m_pCfg->m_Memc, reqobj->auth_token().c_str(), w);
    if(rc != MEMCACHED_SUCCESS)
    {
        if(rc == MEMCACHED_DATA_EXISTS)
        {
            INFO("WOW, CAS found target already modified by others, need retry CAS again...\n");
            rc = set_token_info_to_mem(m_pCfg->m_Memc, reqobj->auth_token().c_str(), w);
            INFO("try again result:%d\n", rc);
        }
        else
        {
            ERR("***Failed update token info to mem:%d\n", rc);
        }
    }

    // SQL
    set_token_info_to_db(m_pCfg->m_Sql, reqobj);

    return 0;
}

/**
 * entry of a user auth procedure.
 *
 */
int AuthOperation::handling_request(::google::protobuf::Message *auth_req, ::google::protobuf::Message *auth_resp, int *len_resp, void *resp)
{
    int ret;
    struct auth_data_wrapper fields;
    AuthRequest *reqobj= (AuthRequest *)auth_req;
    AuthResponse *respobj = (AuthResponse *)auth_resp;

    if(!is_allowed_access(reqobj))
    {
        ERR("This sysid[%d] is not allowed!\n", reqobj->auth_sysid());
        ret = CDS_ERR_REJECT_LOGIN;
    }
    else
    {
        ret = auth_token_in_session(reqobj, respobj, &fields);

        if(ret == CDS_OK)
        {
            LOG("user token info get [OK]\n");
            respobj->set_caredear_id(fields.adw_cid);

            // do further time check and update if possible
            ret = check_token(reqobj, &fields);
            switch(ret)
            {
            case 0:
                // as too short visit time, don't update time
                ret = CDS_OK;
                break;

            case 1:
                // update time
                update_session_lastoperatetime(reqobj, &fields);

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

    if(compose_result(ret, NULL, auth_resp, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for auth result\n");
    }

    return ret;
}

int AuthOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    AuthResponse *p_obj = (AuthResponse *)obj;

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

