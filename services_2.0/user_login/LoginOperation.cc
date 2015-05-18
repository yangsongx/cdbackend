/**
 * Login handling code logic.
 *
 * \history
 * [2015-05-18] workaround for iOS session-'(null)' bug
 * [2015-04-21] Check the return value for delete mem value.
 * [2015-04-16] Fix incorrect caredear id bug for first time of 3rd party login
 *              Before bug fix - the caredear id returned -1
 *              After bug fix - caredear id returned a valide number.
 * [2015-04-15] Support ShenZhen reqirement for 3rd party login case
 * [2015-04-14] Don't compare the password for third login case
 * [2015-04-12] Fix a buffer overflow bug for copy old token
 * [2015-04-09] Fix the update token bug(table name incorrect)
 * [2015-04-08] Use the same md5 hash algorithm as ShenZhen's
 */
#include "LoginOperation.h"

int LoginOperation::cb_get_shenzhen_flag(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    int *data = (int *)p_extra;

    row = mysql_fetch_row(mresult);

    *data = 0;
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) > 1)
        {
            ERR("Warning, unique-CID SHOULD only match one single result!\n");
        }

        if(row[0] != NULL)
        {
            *data = atoi(row[0]);
        }
    }

    return 0;
}

int LoginOperation::cb_check_name(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    uint64_t *data = (uint64_t *)p_extra;

    *data = (uint64_t) -1;

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
            *data = atol(row[0]);
        }
    }

    return 0;
}

int LoginOperation::cb_check_accode(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    int *data = (int *)p_extra;

    *data = -1;

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
            *data = CDS_ERR_INACTIVATED;
        }
        else
        {
            // this is OK
            *data = CDS_OK;
        }
    }
    else
    {
        *data = CDS_ERR_UMATCH_USER_INFO;
    }

    return 0;
}

int LoginOperation::cb_wr_db_session(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    char *data = (char *)p_extra;

    data[0] = '\0';

    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(row[0] != NULL)
        {
            // don't exceed the incoming parameter's length!
            strncpy(data, row[0], 63);
        }
    }

    return 0;
}

int LoginOperation::cb_check_null_session(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    int j;
    list<struct ios_null_session> *data = (list<struct ios_null_session> *)p_extra;
    struct ios_null_session item;

    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        j = mysql_num_rows(mresult);
        INFO("found totally %d rows for such \'(null)\' case.\n", j);
        while((row = mysql_fetch_row(mresult)) != NULL)
        {
            if(row[0] != NULL && row[1] != NULL)
            {
                item.ios_cid = atol(row[0]);
                strncpy(item.ios_token, row[1], 512);

                data->push_back(item);
            }
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
        rc = rm_mem_value(oldtoken);
        if(rc != MEMCACHED_SUCCESS)
        {
            ERR("**Failed delete key(%s) from mem(%d)\n", oldtoken, rc);
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
    int count;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "DELETE FROM %s WHERE ticket=\'%s\' AND session=\'%s\'",
            USERCENTER_SESSION_TBL, reqobj->logout_ticket().c_str(), reqobj->login_session().c_str());

    ret = sql_cmd(sqlcmd, NULL, NULL);
    count = mysql_affected_rows(m_pCfg->m_Sql);
    if(count != 1)
    {
        ERR("Warning, seems didn't delete the record from DB(return %d)\n", count);
    }

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

int LoginOperation::add_new_3rd_party_entry_to_db(LoginRequest *reqobj, uint64_t *p_cid)
{
    int ret;
    char sqlcmd[1024];

    /* FIXME, for this 3rd party case, how to handle the device/source?
     * currently, I just hard code a 88 for this special case */
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (third,device,source,createtime,status) "
            "VALUES (\'%s\',88,\'88\',NOW(),1)",
            USERCENTER_MAIN_TBL,
            reqobj->login_name().c_str());
    ret = sql_cmd(sqlcmd, NULL, NULL);
    if(ret == CDS_OK)
    {
        ret = mysql_affected_rows(m_pCfg->m_Sql);
        if(ret != 1)
        {
            ERR("Warning, insertion got %d return value\n", ret);
            // set a error code
            ret = CDS_ERR_SQL_EXECUTE_FAILED;
        }
        else
        {
            INFO("new 3rd party entry inserted [OK]\n");
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE third=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->login_name().c_str());
            ret = sql_cmd(sqlcmd, cb_check_name, p_cid);
            if(ret == CDS_OK)
            {
                LOG("3rd party login(first time) got %lu CID\n", *p_cid);
            }
        }
    }

    return ret;
}

int LoginOperation::match_user_credential_in_db(LoginRequest *reqobj, unsigned long *p_cid)
{
    int ret;
    char sqlcmd[1024];
    uint64_t cid = 0;

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
            /* We still need choose ID, to avoid requester
             * using an in-valide CID to login */
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE id=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    reqobj->login_name().c_str());
            break;

        default:
            break;
    }

    ret = sql_cmd(sqlcmd, cb_check_name, &cid);
    if(ret == CDS_OK)
    {
        if(cid == (uint64_t) -1)
        {
            if(reqobj->login_type() == RegLoginType::OTHERS)
            {
                /* Special handling for ShenZhen's 3rd login type */
                INFO("the first-time of 3rd party login, insert it to DB\n");
                ret = add_new_3rd_party_entry_to_db(reqobj, p_cid);
            }
            else
            {
                // this means DB didn't contain such record
                ret = CDS_ERR_UMATCH_USER_INFO;
            }
        }
        else
        {
            *p_cid = cid;
            INFO("name[%s] ==> cid[%ld]\n",reqobj->login_name().c_str(), *p_cid);
            
            // next, we will try compose the passwd based on the got cid
            if(reqobj->login_type() != RegLoginType::MOBILE_PHONE && reqobj->login_type() != RegLoginType::OTHERS)
            {
                char md5data[64];
                // can re-use the sqlcmd here.
                sprintf(sqlcmd, "%s%lu", reqobj->login_password().c_str(), *p_cid);
                get_md5(sqlcmd, strlen(sqlcmd), md5data);
                LOG("phase-I cipher[%s] ==> phase-II cipher[%s]\n", sqlcmd, md5data);

                ret = compare_user_password_wth_cid(reqobj, md5data, *p_cid);
            }
            else if(reqobj->login_type() == RegLoginType::OTHERS)
            {
                LOG("For third login case, we directly return OK as it already done via CAS\n");
                ret = CDS_OK;
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

/**
 * Workaround code actually, hope in the future will obsolete
 * this handling code
 *
 */
int LoginOperation::specially_handling_ios(LoginRequest *reqobj, uint64_t cid)
{
    int ret;
    int count;
    list<struct ios_null_session> contain_null;
    list<struct ios_null_session>::iterator it;
    memcached_return_t rc;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id,ticket FROM %s WHERE caredearid=%lu AND session=\'(null)\'",
            USERCENTER_SESSION_TBL, cid);

    ret = sql_cmd(sqlcmd, cb_check_null_session, &contain_null);
    if(ret == CDS_OK && contain_null.size() > 0)
    {
        INFO("Well, workaround for iOS (null)-session, force one-single token in DB\n");
        // delete the existed null
        for(it = contain_null.begin(); it != contain_null.end(); it++)
        {
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "DELETE FROM %s WHERE id=%lu",
                    USERCENTER_SESSION_TBL, it->ios_cid);
            ret = sql_cmd(sqlcmd, NULL, NULL);
            count = mysql_affected_rows(m_pCfg->m_Sql);
            if(ret == CDS_OK && count == 1)
            {
                INFO("DELETE the existedn (null) session(iOS case) [OK]\n");
            }
            else
            {
                ERR("Failed delete (null) session, ret:%d, count:%d\n", ret, count);
            }

            rc = rm_mem_value(it->ios_token);
            if(rc != MEMCACHED_SUCCESS)
            {
                ERR("Failed delete the %s from mem(%d)\n", it->ios_token, rc);
            }
        }

    }

    return 0;
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
        get_uuid(uuiddata);

        struct user_session us;
        us.us_cid = cid;
        us.us_sessionid = reqobj->login_session().c_str();
        us.us_token = uuiddata;

        // for iOS '(null)' session fix
        if(!strcmp(reqobj->login_session().c_str(), IOS_BUG_SESSION))
        {
            specially_handling_ios(reqobj, cid);
        }

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

    sql_cmd(sqlcmd, NULL, NULL);

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
        INFO("an older product, add device type(%d)\n", reqobj->device_type());
        add_device_type(reqobj);
    }
#endif

    INFO("type:%duser:%s\n", reqobj->login_type(), reqobj->login_name().c_str());

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
    int flag = 0;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT issync FROM %s WHERE caredearid=%lu",
            USERCENTER_ATTR_TBL, cid);

    ret = sql_cmd(sqlcmd, cb_get_shenzhen_flag, &flag);
    if(ret == CDS_OK)
    {
        LOG("the shenzhen flag get OK\n");
    }
    else
    {
        LOG("the shenzhen flag get failed\n");
    }

    return flag;
}

int LoginOperation::compare_user_password_wth_cid(LoginRequest *reqobj, const char *targetdata, uint64_t cid)
{
    int ret = CDS_GENERIC_ERROR;
    int compare_result = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id,status FROM %s WHERE id=%ld AND loginpassword=\'%s\'",
            USERCENTER_MAIN_TBL, cid, targetdata);
    ret = sql_cmd(sqlcmd, cb_check_accode, &compare_result); // use the same cb as accode case
    if(ret == CDS_OK)
    {
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


    ret = sql_cmd(sqlcmd, cb_check_accode, &compare_result);
    if(ret == CDS_OK)
    {
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
            USERCENTER_SESSION_TBL, u->us_cid, u->us_sessionid);


    old[0] = '\0';
    ret = sql_cmd(sqlcmd, cb_wr_db_session, old);
    if(ret == CDS_OK)
    {
        if(strlen(old) == 0)
        {
            // new session
            insert_new_session_in_db(u);
        }
        else
        {
            // old existed session
            INFO("Will obsolete token(%s)...\n", old);
            // TODO, need check the return value!
            overwrite_existed_session_in_db(u);
            ret = 1;
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

    sql_cmd(sqlcmd, NULL, NULL);

    return 0;
}

int LoginOperation::overwrite_existed_session_in_db(struct user_session *u)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET ticket=\'%s\',lastoperatetime=NOW() WHERE caredearid=%ld AND session=\'%s\'",
            USERCENTER_SESSION_TBL, u->us_token,
            u->us_cid, u->us_sessionid);

    sql_cmd(sqlcmd, NULL, NULL);

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
