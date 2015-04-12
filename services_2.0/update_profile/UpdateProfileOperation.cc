/**
 *
 * \history
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

/**
 *
 * return 1 means code correct, otherwise it is failed
 */
int UpdateProfileOperation::pass_code_verify(UpdateRequest *reqobj)
{
    int ret;
    int result = 0;
    char sqlcmd[1024];
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE id=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
            USERCENTER_MAIN_TBL, reqobj->uid().c_str(), reqobj->vcode().c_str());

    ret = sql_cmd(sqlcmd, cb_check_code, &result);
    if(ret == CDS_OK && result == 1)
    {
        return 1;
    }
    else
    {
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

int UpdateProfileOperation::add_user_mobile_phone(UpdateRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

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


    // now, add user mobile phone number into DB
    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET usermobile=\'%s\' WHERE id=\'%s\'",
            USERCENTER_MAIN_TBL, reqobj->update_data().c_str(),
            reqobj->uid().c_str());
    // FIXME, UPDATE SHOULD NEVER just check return value
    ret = sql_cmd(sqlcmd, NULL, NULL);

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

