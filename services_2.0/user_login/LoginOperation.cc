/**
 *
 * \history
 * [2015-4-8] Use the same md5 hash algorithm as ShenZhen's
 */
#include "LoginOperation.h"

int LoginOperation::m_shenzhenFlag = 0;
int LoginOperation::m_result= 0;
char LoginOperation::m_buffer[512];

uint64_t LoginOperation::m_cid = (uint64_t) -1;

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

int LoginOperation::cb_check_name(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    m_cid = (uint64_t) -1;

    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        //Wow, DB record meet
        if(mysql_num_rows(mresult) != 1)
        {
            INFO("Warning, CID matching returned NON-1 result, please check the DB!\n");
        }

        if(row[0] != NULL)
        {
            m_cid = atol(row[0]);
        }
    }

    return 0;
}

int LoginOperation::cb_check_accode(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    m_result = 0;

    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        //Wow, DB record meet
        if(mysql_num_rows(mresult) != 1)
        {
            INFO("Warning, CID matching returned NON-1 result, please check the DB!\n");
        }

        // this is status field
        if(row[1] != NULL && atoi(row[1]) == 0)
        {
            m_result = CDS_ERR_INACTIVATED;
        }
        else
        {
            // this is OK
            m_result = CDS_OK;
        }
    }
    else
    {
        m_result = CDS_ERR_UMATCH_USER_INFO;
    }

    return 0;
}

int LoginOperation::cb_wr_db_session(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    m_buffer[0] = '\0';

    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(row[0] != NULL)
        {
            strncpy(m_buffer, row[0], sizeof(m_buffer));
        }
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
    rc = set_session_info_to_mem(reqobj, u);
    if(rc != MEMCACHED_SUCCESS)
    {
        // FIXME as login set mem won't be the same as all existed mem key,
        // so we don't need CAS here.
        ERR("***Failed update token info to mem:%d\n", rc);
    }

    // SQL
    char oldtoken[64]; // store old token before overwrite the DB
    if(set_session_info_to_db(u, oldtoken) == 1)
    {
        // need delete old token's memcach
        if(rm_mem_value(oldtoken) != 0)
        {
            ERR("**Failed delete key(%s) from mem\n", oldtoken);
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

int LoginOperation::match_user_credential_in_db(LoginRequest *reqobj, unsigned long *p_cid)
{
    int ret;
    char sqlcmd[1024];

    switch(reqobj->login_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE usermobile=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE username=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE email=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::OTHERS:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE third=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::CID_PASSWD:
            // TODO how to handle with CareDear ID login?
            break;

        default:
            break;
    }

    ret = sql_cmd(sqlcmd, cb_check_name);
    if(ret == CDS_OK)
    {
        if(m_cid == (uint64_t) -1)
        {
            // this means DB didn't contain such record
            ret = CDS_ERR_UMATCH_USER_INFO;
        }
        else
        {
            *p_cid = m_cid;
            INFO("name[%s] ==> cid[%ld]\n",reqobj->login_name().c_str(), *p_cid);
            
            // next, we will try compose the passwd based on the got cid
            if(reqobj->login_type() != RegLoginType::MOBILE_PHONE)
            {
                char md5data[64];
                // can re-use the sqlcmd here.
                sprintf(sqlcmd, "%s%lu", reqobj->login_password().c_str(), *p_cid);
                get_md5(sqlcmd, strlen(sqlcmd), md5data);
                LOG("phase-I cipher[%s] ==> phase-II cipher[%s]\n", sqlcmd, md5data);

                ret = compare_user_password_wth_cid(reqobj, md5data, *p_cid);
            }
            else
            {
                // For phone+SMS, we only compare the accode..., password is SMS code,not encrypt string
                ret = compare_user_smscode_wth_cid(reqobj, reqobj->login_password().c_str(), *p_cid);
            }
        }
    }

    return ret;
}

int LoginOperation::process_user_and_credential(LoginRequest *reqobj, LoginResponse *respobj)
{
    int ret = -1;
    unsigned long cid = -1;

    ret = match_user_credential_in_db(reqobj, &cid);


    if(ret == CDS_OK)
    {
        // login info data is correct.
        INFO("%s ==> %ld,[Login OK]\n", reqobj->login_name().c_str(), cid);

        char uuiddata[64]; // string like '3ab554e6-1533-4cea-9f6d-26edfd869c6e'
#if 1
        get_uuid(uuiddata);
#else
        gen_uuid(uuiddata);
#endif
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

printf("type:%duser:%s\n", reqobj->login_type(), reqobj->login_name().c_str());

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

int LoginOperation::compare_user_password_wth_cid(LoginRequest *reqobj, const char *targetdata, uint64_t cid)
{
    int ret = CDS_GENERIC_ERROR;
    int compare_result = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id,status FROM %s WHERE id=%ld AND loginpassword=\'%s\'",
            USERCENTER_MAIN_TBL, cid, targetdata);
    ret = sql_cmd(sqlcmd, cb_check_accode); // use the same cb as accode case
    if(ret == CDS_OK)
    {
        compare_result = m_result;
        INFO("the compare result:%d\n", compare_result);
    }



    return compare_result;
}

int LoginOperation::compare_user_smscode_wth_cid(LoginRequest *reqobj, const char *code, uint64_t cid)
{
    int ret = CDS_GENERIC_ERROR;
    int compare_result = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id,status FROM %s WHERE id=%lu AND accode=\'%s\'",
            USERCENTER_MAIN_TBL, cid, code);


    ret = sql_cmd(sqlcmd, cb_check_accode);
    if(ret == CDS_OK)
    {
        compare_result = m_result;
        INFO("the compare result:%d\n", compare_result);
    }

    return compare_result;
}

/**
 *
 * return 1 will store the old session ticket(token) to @old.
 */
int LoginOperation::set_session_info_to_db(struct user_session *u, char *old)
{
    int ret = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ticket FROM %s WHERE caredearid=%lu AND session=\'%s\'",
            USERCENTER_MAIN_TBL, u->us_cid, u->us_sessionid);


    ret = sql_cmd(sqlcmd, cb_wr_db_session);
    if(ret == CDS_OK)
    {
        if(strlen(m_buffer) == 0)
        {
            // new session
            insert_new_session_in_db(u);
            ret = 1;
        }
        else
        {
            // old existed session
            strcpy(old, m_buffer);
            INFO("Will obsolete token(%s)...\n", old);
            overwrite_existed_session_in_db(u);
        }
    }

    return ret;
}

int LoginOperation::insert_new_session_in_db(struct user_session *u)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (caredearid,ticket,session,lastoperatetime) VALUES "
            "(%ld,\'%s\',\'%s\',NOW())",
            USERCENTER_SESSION_TBL,
            u->us_cid, u->us_token, u->us_sessionid);

    sql_cmd(sqlcmd, NULL);

    return 0;
}

int LoginOperation::overwrite_existed_session_in_db(struct user_session *u)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET ticket=\'%s\',lastoperatetime=NOW() WHERE caredearid=%ld AND session=\'%s\'",
            USERCENTER_MAIN_TBL, u->us_token,
            u->us_cid, u->us_sessionid);

    sql_cmd(sqlcmd, NULL);

    return 0;
}

memcached_return_t LoginOperation::set_session_info_to_mem(LoginRequest *reqobj, struct user_session *u)
{
    memcached_return_t rc;
    char val[128];
    time_t current;

    time(&current);

    /* FIXME login sys id probably different with auth sys id ? */
    sprintf(val, "%ld %s %ld",
            u->us_cid, reqobj->login_session().c_str(), current);

    rc = memcached_set(m_pCfg->m_Memc,
            u->us_token, strlen(u->us_token),
            val, strlen(val),
            0,
            0);

    return rc;
}
