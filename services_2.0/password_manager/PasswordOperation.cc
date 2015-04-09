#include "PasswordOperation.h"

char PasswordOperation::m_md5[36];

int PasswordOperation::cb_get_md5_in_db(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row = mysql_fetch_row(mresult);
    char *data = (char *)p_extra;

    if(row != NULL)
    {
        if(row[0] != NULL)
            strcpy(data, row[0]);
    }
    else
    {
        ERR("Warning, we didn't find the password field in DB!\n");
    }

    return 0;
}

/**
 *
 * return 0 means password is correct
 */
int PasswordOperation::validation_user_password(PasswordManagerRequest *reqobj)
{
    int ret = -1;
    char sqlcmd[1024];
    char buf[256];
    char md5[36]; // sotore the final DB passwd data

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT loginpassword FROM %s WHERE id=%lu",
            USERCENTER_MAIN_TBL, reqobj->caredear_id());

    if(sql_cmd(sqlcmd, cb_get_md5_in_db, md5) == CDS_OK)
    {
        char md5data[64];
        // get the md5 in db, compare it with incoming passwd
        sprintf(buf, "%s%lu", reqobj->old_passwd().c_str(), reqobj->caredear_id());
        get_md5(buf, strlen(buf), md5data);
        if(!strcmp(md5data, md5))
        {
            INFO("Good, password correct\n");
            ret = 0;
        }
    }
    else
    {
        LOG("the select passwd failed\n");
    }

    return ret;
}

int PasswordOperation::write_user_password_to_db(PasswordManagerRequest *reqobj)
{
    char sqlcmd[1024];
    char md5data[64];
    char buf[128];

    sprintf(buf, "%s%lu", reqobj->new_passwd().c_str(), reqobj->caredear_id());
    get_md5(buf, strlen(buf), md5data);

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET loginpassword=\'%s\' WHERE id=%ld",
            USERCENTER_MAIN_TBL, md5data, reqobj->caredear_id());

    return sql_cmd(sqlcmd, NULL/* UPDATE don't need callback */, NULL);
}


int PasswordOperation::modify_existed_password(PasswordManagerRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char md5[36];

    if(reqobj->has_old_passwd())
    {
        printf("you have old password\n");
        // User contained the old password, need match existed password before overwite
        // this with the new passwd
        if(validation_user_password(reqobj) == 0)
        {
            return write_user_password_to_db(reqobj);
        }
        else
        {
            ERR("the old password is incorrect, reject do anything.\n");
            ret = CDS_ERR_INCORRECT_CODE;
        }
    }
    else
    {
        printf("the req type:%d\n", reqobj->type());

        if(reqobj->type() == PasswordType::FORGET)
        {
            // for forget password, we directly overwrite the DB pasword
            ret = write_user_password_to_db(reqobj);
        }
        else
        {

        // didn't contained the old password, need make sure DB also be blank
        m_md5[0] = 0x31;
        snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT loginpassword FROM %s WHERE id=%lu",
            USERCENTER_MAIN_TBL, reqobj->caredear_id());

        md5[0] = '\0';
        ret = sql_cmd(sqlcmd, cb_get_md5_in_db, md5);
        if(ret == CDS_OK && strlen(md5) == 0)
        {
            INFO("User original password is blank, so directly set new passwd\n");
            ret = write_user_password_to_db(reqobj);
        }
        else
        {
            ERR("seems you already had a passwd, need validate that one before change to a new passwd\n");
            ret = CDS_ERR_INCORRECT_CODE;
        }

        }
    }

    return ret;
}

/**
 * Entry point of the password manager handling operation
 *
 */
int PasswordOperation::handling_request(::google::protobuf::Message *passwd_req, ::google::protobuf::Message *passwd_resp, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    PasswordManagerRequest *reqobj = (PasswordManagerRequest *)passwd_req;
    PasswordManagerResponse *respobj = (PasswordManagerResponse *)passwd_resp;

    ret = modify_existed_password(reqobj);

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for password manager result\n");
    }

    return ret;
}

int PasswordOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    PasswordManagerResponse *p_obj = (PasswordManagerResponse *) obj;

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

    return ((obj->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}
