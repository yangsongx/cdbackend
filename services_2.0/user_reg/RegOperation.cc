/**
 *
 * \history
 * [2015-04-13] use a new sip domain for insert record to SIPS DB
 * [2015-04-10] use new API prototype to avoid use static variable
 */
#include "RegOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;


int RegOperation::cb_check_user_existence(MYSQL_RES *p_result, void *p_extra)
{
    MYSQL_ROW row;
    struct user_existence *data = (struct user_existence *)p_extra;

    /* set a default value for new user case */
    data->ue_cid = (uint64_t ) -1;
    data->ue_active_status = 100; // MAGIC number

    row = mysql_fetch_row(p_result);
    if(row == NULL)
    {
        INFO("blank DB check result, a new user\n");
    }
    else
    {
        data->ue_cid = atol(row[0]);
        data->ue_active_status = atoi(row[1]);
        INFO("an existed entry(id-%lu,status-%d)\n",
                data->ue_cid, data->ue_active_status);
    }

    return 0;
}


/** TODO, this API should be delted as base::gen_random_code() already do the same thing!
 *
 * Generate a random 6-digit number, used for email/sms verify code
 *
 */
int RegOperation::gen_verifycode(char *result)
{
    char a[2];
    uuid_t id;
    char a1,a2,a3,a4,a5,a6;

    uuid_generate(id);

    // extract first-two and last-one digit hex as verifiy code
    *(unsigned short *) a  = (unsigned short )(id[0] << 16 | id[1] << 8 | id[15]);

    a1 = (a[0] & 0x0F);
    a2 = ((a[0] & 0xF0) >> 4);
    a3 = (a[1] & 0x0F);
    a4 = ((a[1] & 0xF0) >> 4);
    a5 = (a[2] & 0x0F);
    a6 = ((a[2] & 0xF0) >> 4);

    sprintf(result, "%c%c%c%c%c%c",
             a1 > 9 ? (a1 - 6) + 0x30 : a1 + 0x30,
             a2 > 9 ? (a2 - 6) + 0x30  : a2 + 0x30,
             a3 > 9 ? (a3 - 6) +0x30 : a3 + 0x30,
             a4 > 9 ? (a4 - 6) + 0x30 : a4 + 0x30,
             a5 > 9 ? (a5 - 6) +0x30 : a5 + 0x30,
             a6 > 9 ? (a6 - 6) + 0x30 : a6 + 0x30
            );

    return *(int *) a;
}

int RegOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    RegisterResponse *p_obj = (RegisterResponse *) obj;

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

/**
 * After parsed all request fields, begin handle it one-by-one
 *
 */
int RegOperation::handling_request(::google::protobuf::Message *reg_req, ::google::protobuf::Message *reg_resp, int *len_resp, void *resp)
{
    int ret = 0;
    char buf[32];
    RegisterRequest *reqobj = (RegisterRequest*)reg_req;
    RegisterResponse *respobj = (RegisterResponse *) reg_resp;

    /* error checking */
    if(m_pCfg == NULL)
    {
        ERR("Non-Config info found!\n");
        compose_result(CDS_GENERIC_ERROR, "NULL config obj", respobj, len_resp, resp);
        return CDS_GENERIC_ERROR;
    }

    unsigned long usr_id;
    int status_flag;
    bool existed = user_already_exist(reqobj, &status_flag, &usr_id);

    if(existed && status_flag == 1)
    {
        ret = CDS_ERR_USER_ALREADY_EXISTED;
        /* FIXME, as .proto already define the numeric cid as string
         * we set it as string here */
        snprintf(buf, sizeof(buf), "%lu", usr_id);
        respobj->set_caredear_id(buf);
        compose_result(ret, "User Existed Already", respobj, len_resp, resp);
    }
    else
    {
        if(existed == false)
        {
            // a new user, record it into DB
            ret = add_new_user_entry(reqobj, &usr_id);
        }
        else
        {
            INFO("Found an existed user in DB(ID-%ld), but not activated, so overwrite it!\n", usr_id);
            ret = overwrite_inactive_user_entry(reqobj, usr_id);
        }

        if( ret == CDS_OK
                && (reqobj->reg_type() == RegLoginType::MOBILE_PHONE
                        || reqobj->reg_type() == RegLoginType::EMAIL_PASSWD
                        || reqobj->reg_type() == RegLoginType::PHONE_PASSWD))
        {
            // tell a verification code here
            char verifycode[12] = {'8'};
            gen_verifycode(verifycode);
            respobj->set_reg_verifycode(verifycode);

            LOG("==>the verification code=%s\n", verifycode);

            // need update them into DB for later verification...
            ret = record_user_verifiy_code_to_db(reqobj, respobj, (UserRegConfig *)m_pCfg);
        }

        snprintf(buf, sizeof(buf), "%lu", usr_id);
        respobj->set_caredear_id(buf);

        compose_result(ret, NULL, respobj, len_resp, resp);
    }


    return ret;
}

/**
 * Determin if the new register name existed in DB or not.
 *
 * @p_active_status : store the selected User's status.
 * @p_index: stored the found user's ID(also as caredear id currently)
 *
 * return true means user existed, and @p_index stored the user's ID in DB
 */
bool RegOperation::user_already_exist(RegisterRequest *reqobj, int *p_active_status, uint64_t *p_index)
{
    int ret;
    char sqlcmd[1024];
    struct user_existence usrdata;

    switch(reqobj->reg_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE usermobile=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->reg_name().c_str());
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE username=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->reg_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE email=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->reg_name().c_str());
            break;

        case RegLoginType::OTHERS:
            // user use QQ/WeiXin/Weibo/etc...
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE third=\'%s\'",
                    USERCENTER_MAIN_TBL, reqobj->reg_name().c_str());
            break;

        default:
            ERR("****FATAL**** unknow req type(%d)\n", reqobj->reg_type());
            break;
    }

    ret = sql_cmd(sqlcmd, cb_check_user_existence, &usrdata);
    if(ret == CDS_OK)
    {
        if(usrdata.ue_cid == (uint64_t)-1 && usrdata.ue_active_status == 100)
        {
            // a new user
            return false;
        }
        else
        {
            *p_active_status = usrdata.ue_active_status;
            *p_index = usrdata.ue_cid;
            return true;
        }
    }
    else
    {
        // FIXME = for such error case, how to handle?
        ERR("failed execute the SQL for check user existence, mark this as new temply\n");
        return false;
    }
}

int RegOperation::add_new_user_entry(RegisterRequest *pRegInfo, uint64_t *cid)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char current[32]; // date '1990-12-12 12:12:12' is 20 len
    bool bypassactivate = false;

    if(current_datetime(current, sizeof(current)) != 0)
    {
        /* FIXME - This SHOULD NEVER happen! */
        ERR("Warning, failed compose time, set a default time\n");
        strcpy(current, "1990-12-12 12:12:12");
    }


    if(pRegInfo->has_bypass_activation() && pRegInfo->bypass_activation() == 1)
    {
        bypassactivate = true;
    }

    switch(pRegInfo->reg_type())
    {
        case RegLoginType::MOBILE_PHONE:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (usermobile,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',%d)",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(), pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current,
                    bypassactivate == true ? 1 : 0);
            break;

        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (usermobile,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',%d)",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current,
                    bypassactivate == true ? 1 : 0);
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (username,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',1)",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(),current);
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (email,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',%d)",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current,
                    bypassactivate == true ? 1 : 0);
            break;

        case RegLoginType::OTHERS:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (third,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',1)",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current);
            break;

        default:
            ERR("**Warning, unknow reg type\n");
            break;
    }

    ret = sql_cmd(sqlcmd, NULL, NULL);

    if(ret == CDS_OK)
    {
        INFO("new reg usr insertion [OK](name:%s)\n", pRegInfo->reg_name().c_str());

        // opensips-specific
        if(add_opensips_entry(pRegInfo) != CDS_OK)
        {
            ERR("Warning, failed add conresponding opensips DB item,please check!\n");
        }
        // end of opensips-specific

        // Note here, we still need do a SQL query again, to get User's CID,
        // after get this CID, we can update the password finally!
        //unsigned long cid = 0;
        int active_flag;
        if(user_already_exist(pRegInfo, &active_flag, cid))
        {
            INFO("this new user's CID=%ld\n", *cid);
            // we don't add password for mobile+SMS verify code case
            if(pRegInfo->reg_type() != RegLoginType::MOBILE_PHONE)
            {
                char finaly_passwd[64];
                // re-use the long sqlcmd buff
                sprintf(sqlcmd, "%s%lu",
                        pRegInfo->has_reg_password() ? pRegInfo->reg_password().c_str() : "", *cid);
                get_md5(sqlcmd, strlen(sqlcmd), finaly_passwd);
                LOG("%s --MD5--> %s\n", sqlcmd, finaly_passwd);

                ret = add_user_password_to_db(pRegInfo, *cid, finaly_passwd);
            }
        }
        else
        {
            ERR("SHOULD NEVER Happend as we already insert right now!\n");
            ret = CDS_ERR_SQL_EXECUTE_FAILED;
        }
    }
    else
    {
        ERR("execute the SQL cmd got error:%d, insertion failed\n", ret);
    }

    return ret;
}

int RegOperation::add_user_password_to_db(RegisterRequest *pRegInfo, unsigned long user_id, const char *passwd)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET loginpassword=\'%s\' WHERE id=%ld",
            USERCENTER_MAIN_TBL, passwd, user_id);

    return sql_cmd(sqlcmd, NULL, NULL);
}

int RegOperation::overwrite_inactive_user_entry(RegisterRequest *pRegInfo, unsigned long user_id)
{
    char sqlcmd[1024];
    bool bypassactivate = false;

    if(pRegInfo->reg_type() == RegLoginType::MOBILE_PHONE)
    {
        INFO("For SMS verify case, DO NOT overwrite the DB!\n");
        return 0;
    }

    if(pRegInfo->has_bypass_activation() && pRegInfo->bypass_activation() == 1)
    {
        bypassactivate = true;
    }

    // as we already knew the CID, calculate the password here.
    char finaly_passwd[64];
    char temp_buf[128];
    snprintf(temp_buf, sizeof(temp_buf), "%s%lu",
            pRegInfo->has_reg_password() ? pRegInfo->reg_password().c_str() : "", user_id);
    get_md5(temp_buf, strlen(temp_buf), finaly_passwd);
    LOG("%s --MD5--> %s\n", temp_buf, finaly_passwd);


    switch(pRegInfo->reg_type())
    {
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET usermobile=\'%s\',loginpassword=\'%s\',device=%d,source=\'%s\',createtime=NOW(),status=%d WHERE id=%ld",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(), 
                    finaly_passwd,
                    pRegInfo->reg_device(), pRegInfo->reg_source().c_str(),
                    bypassactivate == true ? 1 : 0,
                    user_id);
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET email=\'%s\',loginpassword=\'%s\',device=%d,source=\'%s\',createtime=NOW(),status=%d WHERE id=%ld",
                    USERCENTER_MAIN_TBL,
                    pRegInfo->reg_name().c_str(),
                    finaly_passwd,
                    pRegInfo->reg_device(), pRegInfo->reg_source().c_str(),
                    bypassactivate == true ? 1 : 0,
                    user_id);
            break;

        default:
            // DO nothing here
            ERR("unsupport reg type(line %d)\n", __LINE__);
            break;
    }

    return sql_cmd(sqlcmd, NULL, NULL);
}

int RegOperation::add_opensips_entry(RegisterRequest *pRegInfo)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    UserRegConfig *c = (UserRegConfig *)m_pCfg;

    if(c->m_SipsSql == NULL)
    {
        ERR("Warning, SIPs SQL unavailable!\n");
        return ret;
    }

    if(!(pRegInfo->reg_type() == RegLoginType::MOBILE_PHONE
            || pRegInfo->reg_type() == RegLoginType::PHONE_PASSWD))
    {
        // don't need take care of Non-mobile case
        return ret;
    }

    /* [2015-04-13] new user center use new domain as 'rdp2.caredear.com' */
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (username,domain) VALUES (\'%s\',\'rdp2.caredear.com\')",
            OPENSIPS_SUB_TBL, pRegInfo->reg_name().c_str());

    // OpenSIPs is another DB, so use a different API
    ret = sql_cmd_with_specify_server(&(c->m_SipsSql), sqlcmd, NULL, NULL);

    return ret;
}

int RegOperation::record_user_verifiy_code_to_db(RegisterRequest *reqobj, RegisterResponse *respobj, UserRegConfig *config)
{
    int ret;
    char sqlcmd[1024];

    switch(reqobj->reg_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET accode=\'%s\',codetime=UNIX_TIMESTAMP(date_add(now(), interval %d second)) "
                    "WHERE usermobile=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    respobj->reg_verifycode().c_str(),
                    config->m_iMobileVerifyExpir,
                    reqobj->reg_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET accode=\'%s\',codetime=UNIX_TIMESTAMP(date_add(now(), interval %d second)) "
                    "WHERE email=\'%s\'",
                    USERCENTER_MAIN_TBL,
                    respobj->reg_verifycode().c_str(),
                    config->m_iEmailVerifyExpir,
                    reqobj->reg_name().c_str());
            break;

        default:
            ERR("**** SHOULD NEVER meet this verifiy code here(%d line)!!!\n", __LINE__);
            break;
    }

    ret = sql_cmd(sqlcmd, NULL, NULL);

    return ret;
}
