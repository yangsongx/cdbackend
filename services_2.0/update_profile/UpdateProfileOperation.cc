/**
 *
 * \history
 * [2015-07-31] The bind a phone number under email logic is incorrect
 * [2015-04-27] Add SIPs DB entry for mobile case
 * [2015-04-12] Fix the  unique SQL check logic
 */
#include "UpdateProfileOperation.h"
#include "UpdateProfileConfig.h"

int UpdateProfileOperation::cb_check_code(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    int *data = (int *)p_extra;

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        *data = 1;
    }
    else
    {
        *data = 0;
    }

    return 0;
}

int UpdateProfileOperation::cb_get_repeat_id(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    uint64_t *data = (uint64_t *)p_extra;

    *data = (uint64_t)-1;
    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(row[0] != NULL)
            *data = atol(row[0]);
    }

    return 0;
}

int UpdateProfileOperation::cb_check_sipaccount(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    int *data = (int *)p_extra;

    *data = 0;

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) != 1)
        {
            ERR("Warning, not single record found in SIPs DB\n");
        }
        *data = 1;
    }

    return 0;
}

/**
 *
 * Try remove redundent mobile  record
 */
void UpdateProfileOperation::strip_repeat_mobile_number(UpdateRequest *reqobj)
{
    char sqlcmd[1024];
    uint64_t id;
    int ret;

    if(reqobj->reg_type() != Updatetype::MOBILE_PHONE)
    {
        ERR("Only available for mobile case!\n");
        return;
    }
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE usermobile=\'%s\' AND status=0",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str());

    ret = sql_cmd(sqlcmd, cb_get_repeat_id, &id);
    if(ret == CDS_OK && id!= (uint64_t)-1)
    {
        INFO("Attention, need remove this guy(id-%lu)...\n", id);
        snprintf(sqlcmd, sizeof(sqlcmd),
                "DELETE FROM %s WHERE id=%lu",
                USERCENTER_MAIN_TBL, id);
        ret = sql_cmd(sqlcmd, NULL, NULL);
        int flags = 0;
        UpdateProfileConfig *c =
            (UpdateProfileConfig *)m_pCfg;
        flags = mysql_affected_rows(c->m_SipsSql);
        if(flags != 1)
        {
            ERR("warning, deletetion got %d affected lines\n", flags);
        }
        else
        {
            INFO("Delete this reconrd [OK]\n");
            // TODO, below should put into a DB in the future...
            FILE *p = fopen("/opt/native/maping.id.table", "a+");
            if(p) {
                fprintf(p, "%lu =====> %s\n", id, reqobj->uid().c_str());
                fclose(p);
            }
        }
    }
}

/**
 *
 * return 1 means code correct, otherwise it is failed
 */
int UpdateProfileOperation::pass_code_verify(UpdateRequest *reqobj)
{
    int ret;
    int result = 0;
    char sqlcmd[1024];
    if(reqobj->reg_type() == Updatetype::MOBILE_PHONE)
    {
        snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE usermobile=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str(), reqobj->vcode().c_str());
    }
    else if(reqobj->reg_type() == Updatetype::EMAIL)
    {
        snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE email=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str(), reqobj->vcode().c_str());
    }
    else
    {
        ERR("unknow type(%d), ignore this action, mark as failure\n", reqobj->reg_type());
        return 0;
    }

    ret = sql_cmd(sqlcmd, cb_check_code, &result);
    if(ret == CDS_OK && result == 1)
    {
        return 1;
    }
    else
    {
        ERR("verify code failed, sql:%s\n", sqlcmd);
        return 0;
    }
}

/**
 *
 * return 1 means the record to be added is unique, otherwise it already existed
 */
int UpdateProfileOperation::unique_record(UpdateRequest *reqobj)
{
    char sqlcmd[1024];
    int ret;
    int result;

    switch(reqobj->reg_type())
    {
        case Updatetype::MOBILE_PHONE:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND status=1",
                USERCENTER_MAIN_TBL, reqobj->update_data().c_str());
            break;

        case Updatetype::EMAIL:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND status=1",
                USERCENTER_MAIN_TBL, reqobj->update_data().c_str());
            break;

        case Updatetype::USER_NAME:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE username=\'%s\' AND status=1",
                    USERCENTER_MAIN_TBL, reqobj->update_data().c_str());
            break;

        default:
            break;
    }

    // can reuse the cb
    ret = sql_cmd(sqlcmd, cb_check_code, &result);
    if(ret == CDS_OK && result == 0)
    {
        return 1;
    }
    else
    {
        // for error case, logout the sql cmd
        LOG("the SQL:%s\n", sqlcmd);
        return 0;
    }


    return 0;
}

int UpdateProfileOperation::add_mobile_to_sips_db(UpdateRequest *reqobj)
{
    char sqlcmd[1024];
    int flags;
    int ret;
    UpdateProfileConfig *c = (UpdateProfileConfig *)m_pCfg;

    if(reqobj->reg_type() != Updatetype::MOBILE_PHONE)
    {
        ERR("don't support NON-mobile case!\n");
        return -1;
    }

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE username=\'%s\'",
            OPENSIPS_SUB_TBL, reqobj->update_data().c_str());
    ret = sql_cmd_with_specify_server(&c->m_SipsSql, sqlcmd, cb_check_sipaccount, &flags);
    if(ret == CDS_OK && flags == 0)
    {
        // didn't found existed record in DB
        snprintf(sqlcmd, sizeof(sqlcmd),
                "INSERT INTO %s (username,domain) VALUES (\'%s\',\'rdp2.caredear.com\')",
                OPENSIPS_SUB_TBL, reqobj->update_data().c_str());
        ret = sql_cmd_with_specify_server(&c->m_SipsSql, sqlcmd, NULL, NULL);
        flags = mysql_affected_rows(c->m_SipsSql);
        if(flags != 1)
        {
            ERR("warning, insertion got %d affected lines", flags);
        }
        else
        {
            INFO("also add the mobile account to Sips DB [OK]\n");
        }
    }
    else
    {
        ERR("either select or found existed user in Sips db, failed insert to sips db\n");
    }

    return 0;
}

/**
 * Adding a mobile phone under an existed email account
 *
 */
int UpdateProfileOperation::add_user_mobile_phone(UpdateRequest *reqobj)
{
    int ret = CDS_OK;
    int count;
    char sqlcmd[1024];

    /* Note on 2015-07-20
     * I think need check existence as the first step!
     */
    if(unique_record(reqobj) != 1)
    {
        ERR("this record existed!\n");
        return CDS_ERR_USER_ALREADY_EXISTED;
    }

    // next, check mobile code is correct or not
    if(pass_code_verify(reqobj) != 1)
    {
        ERR("The mobile veify code is incorrect\n");
        return CDS_ERR_INCORRECT_CODE;
    }

    // 2015-4-24 Need remove in-active mobiles for such case
    strip_repeat_mobile_number(reqobj);

    // now, add user mobile phone number into DB
    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET usermobile=\'%s\' WHERE id=\'%s\'",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str(),
            reqobj->uid().c_str());

    ret = sql_cmd(sqlcmd, NULL, NULL);
    count = mysql_affected_rows(m_pCfg->m_Sql);
    if(count != 1)
    {
        ERR("Warning, seems didn't update the record in DB(return %d)\n", count);
    }

    //[2015-04-13] Need add the mobile account into SIPs DB
    add_mobile_to_sips_db(reqobj);

    return ret;
}

int UpdateProfileOperation::add_user_email(UpdateRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    //
    // first check mobile code is correct or not
    if(pass_code_verify(reqobj) != 1)
    {
        ERR("The mobile veify code is incorrect\n");
        return CDS_ERR_INCORRECT_CODE;
    }

    if(unique_record(reqobj) != 1)
    {
        ERR("this record existed!\n");
        return CDS_ERR_USER_ALREADY_EXISTED;
    }


    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET email=\'%s\' WHERE id=\'%s\'",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str(),
            reqobj->uid().c_str());

    ret = sql_cmd(sqlcmd, NULL, NULL);

    return 0;
}

int UpdateProfileOperation::add_user_name(UpdateRequest *reqobj)
{
    int ret;
    char sqlcmd[1024];

    if(unique_record(reqobj) != 1)
    {
        ERR("this record existed!\n");
        return CDS_ERR_USER_ALREADY_EXISTED;
    }

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET username=\'%s\' WHERE id=\'%s\'",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str(), reqobj->uid().c_str());

    ret = sql_cmd(sqlcmd, NULL, NULL);

    return ret;
}

/**
 *
 * User want a password field
 *
 */
int UpdateProfileOperation::add_user_password(UpdateRequest *reqobj)
{
    int ret;
    char sqlcmd[1024];

    char targetdata[128];
    char md5[64];

    snprintf(targetdata, sizeof(targetdata),
            "%s%s", reqobj->update_data().c_str(), reqobj->uid().c_str());
    //MD5
    get_md5(targetdata, strlen(targetdata), md5);

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET loginpassword=\'%s\' WHERE id=\'%s\'",
            USERCENTER_MAIN_TBL, md5, reqobj->uid().c_str());

    ret = sql_cmd(sqlcmd, NULL, NULL);

    return ret;
}

/**
 * Handling entry point
 *
 *
 */
int UpdateProfileOperation::handling_request(::google::protobuf::Message *reg_req, ::google::protobuf::Message *reg_resp, int *len_resp, void *resp)
{
    int ret = CDS_GENERIC_ERROR;
    UpdateRequest *reqobj = (UpdateRequest *) reg_req;
    UpdateResponse *respobj = (UpdateResponse *) reg_resp;

    LOG("-(type:%d)-(data:%s)-(uid:%s)---->\n", reqobj->reg_type(), reqobj->update_data().c_str(), reqobj->uid().c_str());

    switch(reqobj->reg_type())
    {
        case Updatetype::MOBILE_PHONE:
            ret = add_user_mobile_phone(reqobj);
            break;

        case Updatetype::EMAIL:
            ret = add_user_email(reqobj);
            break;

        case Updatetype::USER_NAME:
            ret = add_user_name(reqobj);
            break;

        case Updatetype::PASSWORD:
            ret = add_user_password(reqobj);
            break;

        case Updatetype::OTHERS: // TODO need handle this?
        default:
            ERR("failed get an unknow type\n");
            break;
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
         ERR("**failed serialize data for login result\n");
    }

    return ret;
}


int UpdateProfileOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message*obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    UpdateResponse *p_obj = (UpdateResponse *)obj;

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

