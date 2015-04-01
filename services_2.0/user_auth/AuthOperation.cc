#include "AuthOperation.h"

struct auth_data_wrapper AuthOperation::m_AuthWrapper;

int AuthOperation::cb_token_info_query(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    if(mysql_num_rows(mresult) > 1)
    {
        ERR("**Warning, multiple hit for token auth!\n");
    }

    // set below magic number to indciate error case
    m_AuthWrapper.adw_cid = -1;
    m_AuthWrapper.adw_lastlogin = -1;

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(row[0] != NULL)
        {
            m_AuthWrapper.adw_cid = atol(row[0]);
        }

        if(row[1] != NULL)
        {
            m_AuthWrapper.adw_lastlogin = atol(row[1]);
        }
    }
    else
    {
        ERR("shit, didn't find any match auth data in DB!\n");
    }

    return 0;
}

/**
 * The auth memcache layout:
 *
 *     KEY        |   CID   |  session  |   lastlogin |
 * ---------------+-----------------------------------------
 *   token value  |   xx        xxx           xxxxxx
 * ---------------+-----------------------------------------
 *
 *
 */
int AuthOperation::split_val_into_fields(char *value, struct auth_data_wrapper *w)
{
    char *c, *s, *l;
    char *saveptr;

    if(!value)
    {
        return -1;
    }

    if((c = strtok_r((char *)value, " ", &saveptr)) != NULL)
    {
        w->adw_cid = atol(c);
        if((s = strtok_r(NULL, " ", &saveptr)) != NULL)
        {
            strcpy(w->adw_session, s);
            if((l = strtok_r(NULL, " " , &saveptr)) != NULL)
            {
                w->adw_lastlogin = atol(l);
            }
        }
    }

    return 0;
}

int AuthOperation::set_token_info_to_db(AuthRequest *reqobj)
{
    int ret = -1;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET lastoperatetime=NOW() WHERE ticket=\'%s\'",
            USERCENTER_SESSION_TBL, reqobj->auth_token().c_str());

    KPI("Will try update last login DB data...\n");
    ret = sql_cmd(sqlcmd, NULL);
    if(ret == CDS_OK)
    {
        KPI("Update the DB ... [OK]\n");
    }
    else
    {
        KPI("Update the DB failed(%d)!\n", ret);
    }

    return ret;
}

/**
 * Will get all needed info from SQL, and store them into @w
 *
 *
 */
int AuthOperation::get_token_info_from_db(AuthRequest *reqobj, AuthResponse *respobj, struct auth_data_wrapper *w)
{
    int ret = -1;
    char sqlcmd[1024];

    /* FIXME how can we handle with XMPP AUTH case ? */
    if(is_xmpp_auth(reqobj))
    {
        LOG("For XMPP case, don't consider the session\n");
        snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT caredearid,UNIX_TIMESTAMP(lastoperatetime) FROM %s "
            "WHERE ticket=\'%s\'",
            USERCENTER_SESSION_TBL, reqobj->auth_token().c_str());
    }
    else
    {
        snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT caredearid,UNIX_TIMESTAMP(lastoperatetime) FROM %s "
            "WHERE ticket=\'%s\' AND session=\'%s\'",
            USERCENTER_SESSION_TBL, reqobj->auth_token().c_str(), reqobj->auth_session().c_str());
    }

    ret = sql_cmd(sqlcmd, cb_token_info_query);
    if(ret == CDS_OK)
    {
        if(m_AuthWrapper.adw_cid == (uint64_t)-1 && m_AuthWrapper.adw_lastlogin == -1)
        {
            ERR("SQL query the %s failed\n", reqobj->auth_token().c_str());
            return CDS_ERR_UMATCH_USER_INFO;
        }

        // well, copy the callback's value into the @w parameter
        memcpy(w, &m_AuthWrapper, sizeof(m_AuthWrapper));
    }

    return ret;
}

bool AuthOperation::is_xmpp_auth(AuthRequest *reqobj)
{
    bool xmpp = false;

    if(!strcmp(reqobj->auth_session().c_str(), "XMPP")
            && reqobj->auth_sysid() == 99)
        xmpp = true;

    return xmpp;
}

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

    /* FIXME special handling for XMPP */
    if(is_xmpp_auth(reqobj))
    {
        INFO("==An XMPP REQ, consider it as allowed==\n");
        return true;
    }

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
    p_val = get_mem_value(reqobj->auth_token().c_str(), &val_len, NULL /* don't need the CAS */);
    if(p_val)
    {
        // got value from mem, parse it...
        split_val_into_fields(p_val, w);

        /* FIXME - currently, for XMPP case, we don't need check
         * session with mem/db value */
        if(!is_xmpp_auth(reqobj) && strcmp(reqobj->auth_session().c_str(), w->adw_session))
        {
            ERR("(%s %s) <---->mem(%s), SESSION-mismacth, re-count on DB..\n",
                    reqobj->auth_token().c_str(), reqobj->auth_session().c_str(),
                    w->adw_session);

            ret = get_token_info_from_db(reqobj, respobj, w);
        }
        free(p_val);
    }
    else
    {
        // didn't get the mem val, try on DB...
        INFO("not found in mem, go to DB...\n");
        ret = get_token_info_from_db(reqobj, respobj, w);
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
    char value[128];
    time_t cur;

#if 1
    if(is_xmpp_auth(reqobj)){
        printf("the session in w:%s\n", w->adw_session);
    }
#endif

    time(&cur);
    sprintf(value, "%ld %s %ld",
            w->adw_cid, w->adw_session, cur);

    // mem FIXME - directly set, not using CAS
    rc = (memcached_return_t) set_mem_value(reqobj->auth_token().c_str(), value);
    if(rc != MEMCACHED_SUCCESS)
    {
        ERR("**failed set mem :%d\n", rc);
    }

    // SQL
    set_token_info_to_db(reqobj);

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

