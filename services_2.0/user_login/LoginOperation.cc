#include "LoginOperation.h"
#include "usr_login_db.h"

int LoginOperation::m_shenzhenFlag = 0;

int LoginOperation::cb_get_shenzhen_flag(MYSQL_RES *mresult)
{
    MYSQL_ROW row;
    row = mysql_fetch_row(mresult);

    m_shenzhenFlag = 0;
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) > 1)
        {
            ERR("Warning, unique-CID SHOULD only match one single result!\n");
        }

        if(row[0] != NULL)
            m_shenzhenFlag = atoi(row[0]);
    }

    return 0;
}
/**
 * Inner wrapper util for mem+DB set
 *
 * Need obsolete existed token if found.
 */
int LoginOperation::update_usercenter_session(LoginRequest *reqobj, struct user_session *u)
{
    memcached_return_t rc;

    //MEM
    rc = set_session_info_to_mem(m_pCfg->m_Memc, reqobj, u);
    if(rc != MEMCACHED_SUCCESS)
    {
        // FIXME as login set mem won't be the same as all existed mem key,
        // so we don't need CAS here.
        ERR("***Failed update token info to mem:%d\n", rc);
    }

    // SQL
    char oldtoken[64]; // store old token before overwrite the DB
    if(set_session_info_to_db(m_pCfg->m_Sql, u, oldtoken) == 1)
    {
        // need delete old token's memcach
        memcached_return_t rc = rm_session_info_from_mem(m_pCfg->m_Memc, oldtoken);
        if(rc != MEMCACHED_SUCCESS)
        {
            ERR("**Failed delete key(%s) from mem:%d\n",
                    oldtoken, rc);
        }
        else
        {
            LOG("delete key(%s) [OK]\n", oldtoken);
        }
    }

    return 0;
}

int LoginOperation::delete_usercenter_session(LoginRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "DELETE FROM %s WHERE ticket=\'%s\' AND session=\'%s\'",
            USERCENTER_SESSION_TBL, reqobj->logout_ticket().c_str(), reqobj->login_session().c_str());

    ret = sql_cmd(sqlcmd, NULL);

    return ret;
}

int LoginOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    LoginResponse *p_obj = (LoginResponse *)obj;

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

int LoginOperation::process_user_and_credential(LoginRequest *reqobj, LoginResponse *respobj)
{
    int ret = -1;
    unsigned long cid = -1;

    ret = match_user_credential_in_db(m_pCfg->m_Sql, reqobj, &cid);

    if(ret == CDS_ERR_SQL_DISCONNECTED)
    {
        ERR("Oh, found MySQL disconnected, try reconnecting...\n");

        if(m_pCfg->reconnect_sql(m_pCfg->m_Sql,
                    m_pCfg->m_strSqlIP,
                    m_pCfg->m_strSqlUserName,
                    m_pCfg->m_strSqlUserPassword) != NULL)
        {
            ret = match_user_credential_in_db(m_pCfg->m_Sql, reqobj, &cid);
        }
    }

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

        update_usercenter_session(reqobj, &us);

        respobj->set_token(uuiddata);
#if 1 // add a ShenZheng-specific flag
        respobj->set_existed_in_shenzhen(get_shenzhen_flag_from_db(cid));
#endif
    }
    else
    {
        // failure case...
        ERR("User login is incorrect!\n");
    }

    return ret;
}

/* a templ util function, probably not needed after we
 * got all older products' device type */
int LoginOperation::add_device_type(LoginRequest *reqobj)
{
    char sqlcmd[1024];

    switch(reqobj->login_type()) {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET device=\'%d\' WHERE usermobile=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->device_type(), reqobj->login_name().c_str());
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET device=\'%d\' WHERE username=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->device_type(), reqobj->login_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET device=\'%d\' WHERE email=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->device_type(), reqobj->login_name().c_str());
            break;

        case RegLoginType::OTHERS:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET device=\'%d\' WHERE third=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->device_type(), reqobj->login_name().c_str());
            break;

        case RegLoginType::CID_PASSWD:
            // TODO support this?
            break;

        default:
            break;
    }

    sql_cmd(sqlcmd, NULL);

    return 0;
}

/**
 * Begin do the User's Login action
 *
 *
 */
int LoginOperation::handling_request(::google::protobuf::Message *login_req, ::google::protobuf::Message *login_resp, int *len_resp, void *resp)
{
    int ret = -1;
    LoginRequest *reqobj = (LoginRequest *)login_req;
    LoginResponse *respobj = (LoginResponse *) login_resp;

    /* TODO
     * As older version of APK didn't contained the device type,
     * we need this on 2.0 arch, so temply added a field to store
     * the device info
     *
     * We need obsolete this after all old released product report
     * the info.
     */
#if 1
    if(reqobj->has_device_type()) {
        INFO("an older product, add device type\n");
        add_device_type(reqobj);
    }
#endif

    if(reqobj->login_type() != RegLoginType::LOG_OUT)
    {
        // a normal User+Credential Log-In case
        ret = process_user_and_credential(reqobj, respobj);
    }
    else
    {
        // This is user requesting deleting log session
        ret = delete_usercenter_session(reqobj);
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for login result\n");
    }

    return ret;
}

int LoginOperation::get_shenzhen_flag_from_db(uint64_t cid)
{
    char sqlcmd[1024];
    int ret = 0;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT issync FROM %s WHERE caredearid=%lu",
            USERCENTER_ATTR_TBL, cid);

    ret = sql_cmd(sqlcmd, cb_get_shenzhen_flag);
    if(ret == CDS_OK)
    {
        ret = m_shenzhenFlag;
    }

    return ret;
}
