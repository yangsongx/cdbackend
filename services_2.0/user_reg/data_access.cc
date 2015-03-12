/**
 * This is a wrapper layer for Data Access(contain both memcach + MySQL)
 *
 */
#include <my_global.h>
#include <mysql.h>
#include <errmsg.h> //MySQL error code
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
#undef max
#undef min
#endif

#include "cds_public.h"

#include "data_access.h"

/* DATABASE table/field names... */
#define NEW_REG_TABLE   "uc.uc_passport"

using namespace com::caredear;

extern pthread_mutex_t urs_mutex; // defined in main()

int keep_urs_db_connected(MYSQL *ms)
{
    int ret = CDS_OK;
    char sqlcmd[128];
    MYSQL_RES *mresult;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s;", NEW_REG_TABLE);

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);
        ERR("failed execute the ping sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_CDS(urs_mutex);

    if(mresult)
    {
        mysql_fetch_row(mresult);
        //DO NOTHING HERE, we just call a store result code,
        //since MySQL connection timeout is removed when
        //code come here.
    }
    else
    {
        ERR("got a null result for ping SQL cmd\n");
        ret = CDS_ERR_SQL_EXECUTE_FAILED;
    }

    return ret;
}

/**
 * user's register passwd is md5(passwd), we need do md5(CID+md5(passwd) again, then input
 * that md5 into DB, can't directly save the user's request's password.
 *
 */
int add_user_password_to_db(MYSQL *ms, RegisterRequest *pRegInfo, unsigned long user_id, const char *passwd)
{
    int ret = CDS_OK;
    char sqlcmd[1024];


    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET loginpassword=\'%s\' WHERE id=%ld",
            NEW_REG_TABLE, passwd, user_id);

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);
        ERR("failed execute add usr passwd sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    if(mysql_store_result(ms) == NULL)
    {
        INFO("add user passwd in DB [OK]\n");
    }

    UNLOCK_CDS(urs_mutex);

    return ret;
}

/**
 * Add a new user in the DB.
 *
 */
int add_new_user_entry(MYSQL *ms, RegisterRequest *pRegInfo)
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
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(), pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current,
                    bypassactivate == true ? 1 : 0);
            break;

        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (usermobile,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',%d)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current,
                    bypassactivate == true ? 1 : 0);
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (username,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',1)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(),current);
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (email,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',%d)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current,
                    bypassactivate == true ? 1 : 0);
            break;

        case RegLoginType::OTHERS:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (third,device,source,createtime,status) "
                    "VALUES (\'%s\',%d,\'%s\',\'%s\',1)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->reg_device(),
                    pRegInfo->reg_source().c_str(), current);
            break;

        default:
            ERR("**Warning, unknow reg type\n");
            break;
    }

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);
        ERR("failed execute the new user sql cmd:%s\n", mysql_error(ms));
        ERR("the cmd txt:%s\n", sqlcmd);
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(urs_mutex);

    if(mresult == NULL)
    {
        INFO("mysql_store_result()==NULL, new reg usr insertion [OK]\n");

        // Note here, we still need do a SQL query again, to get User's CID,
        // after get this CID, we can update the password finally!
        unsigned long cid = 0;
        int active_flag;
        if(user_already_exist(ms, pRegInfo, &active_flag, &cid))
        {
            INFO("this new user's CID=%ld\n", cid);
            char finaly_passwd[64];
            // re-use the long sqlcmd buff
            sprintf(sqlcmd, "%ld-%s",
                    cid, pRegInfo->has_reg_password() ? pRegInfo->reg_password().c_str() : "");
            get_md5(sqlcmd, strlen(sqlcmd), finaly_passwd);
            LOG("%s --MD5--> %s\n", sqlcmd, finaly_passwd);

            ret = add_user_password_to_db(ms, pRegInfo, cid, finaly_passwd);
        }
        else
        {
            ERR("SHOULD NEVER Happend as we already insert right now!\n");
            ret = CDS_ERR_SQL_EXECUTE_FAILED;
        }
    }

    return ret;
}


/**
 *
 *
 */
int overwrite_inactive_user_entry(MYSQL *ms, RegisterRequest *pRegInfo, unsigned long user_id)
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
    snprintf(temp_buf, sizeof(temp_buf), "%ld-%s",
            user_id, pRegInfo->has_reg_password() ? pRegInfo->reg_password().c_str() : "");
    get_md5(temp_buf, strlen(temp_buf), finaly_passwd);
    LOG("%s --MD5--> %s\n", temp_buf, finaly_passwd);


    switch(pRegInfo->reg_type())
    {
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET usermobile=\'%s\',loginpassword=\'%s\',device=%d,source=\'%s\',createtime=NOW(),status=%d WHERE id=%ld",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(), 
                    finaly_passwd,
                    pRegInfo->reg_device(), pRegInfo->reg_source().c_str(),
                    bypassactivate == true ? 1 : 0,
                    user_id);
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET email=\'%s\',loginpassword=\'%s\',device=%d,source=\'%s\',createtime=NOW(),status=%d WHERE id=%ld",
                    NEW_REG_TABLE,
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

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);
        ERR("failed execute the overwrite an inactive user sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    if(mysql_store_result(ms) == NULL)
    {
        INFO("mysql_store_result()==NULL, inactive usr overwrite[OK]\n");
    }

    UNLOCK_CDS(urs_mutex);

    return 0;
}

/**
 * Determin if the new register name existed in DB or not.
 *
 * @p_active_status : store the selected User's status.
 * @p_index: stored the found user's ID(also as caredear id currently)
 *
 * return true means user existed, and @p_index stored the user's ID in DB
 */
bool user_already_exist(MYSQL *ms, RegisterRequest *reqobj, int *p_active_status, unsigned long *p_index)
{
    char sqlcmd[1024];

    switch(reqobj->reg_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE usermobile=\'%s\'",
                    NEW_REG_TABLE, reqobj->reg_name().c_str());
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE username=\'%s\'",
                    NEW_REG_TABLE, reqobj->reg_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE email=\'%s\'",
                    NEW_REG_TABLE, reqobj->reg_name().c_str());
            break;

        case RegLoginType::OTHERS:
            // user use QQ/WeiXin/Weibo/etc...
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id,status FROM %s WHERE third=\'%s\'",
                    NEW_REG_TABLE, reqobj->reg_name().c_str());
            break;

        default:
            // TODO
            break;
    }

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);

        /* Note - As this function prototype is boolean, we re-use incoming paremeters
         * to indicate the SQL auto-disconnect case... */
        if(mysql_errno(ms) == CR_SERVER_GONE_ERROR)
        {
            ERR("SQL GONE AWAY after long time, need reconnecting!\n");
            *p_active_status = -1;
            *p_index = (unsigned long) -1;
            return true; /* force it as already existed, and caller check another two paremeters */
        }
        else
        {
            ERR("Warning, failed execute the existence check sql cmd:%s\n", mysql_error(ms));
            /* FIXME - for SQL check failure case, we consider user not existed */
            return false;
        }
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;

    mresult = mysql_store_result(ms);
    UNLOCK_CDS(urs_mutex);

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            return false;
        }
        else
        {
            INFO("the %s record existed in DB(id=%s)!\n",
                    reqobj->reg_name().c_str(), row[0]);
            *p_index = (unsigned long) atol(row[0]);

            if(reqobj->reg_type() != RegLoginType::MOBILE_PHONE)
            {
                *p_active_status = atoi(row[1]);
            }
            else
            {
                // here it is mobile verify SMS case, we don't need the active flag,
                // so set it 0.
                *p_active_status = 0;
            }

            return true;
        }
    }
    else
    {
        // SHOULD NEVER BE NULL,
        // we consider it as not existed anyway.
        return false;
    }

}

/**
 *
 */
int record_user_verifiy_code(MYSQL *ms, RegisterRequest *reqobj, RegisterResponse *respobj, UserRegConfig *config)
{
    char sqlcmd[1024];

    switch(reqobj->reg_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET accode=\'%s\',codetime=UNIX_TIMESTAMP(date_add(now(), interval %d second)) "
                    "WHERE usermobile=\'%s\'",
                    NEW_REG_TABLE,
                    respobj->reg_verifycode().c_str(),
                    config->m_iMobileVerifyExpir,
                    reqobj->reg_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET accode=\'%s\',codetime=UNIX_TIMESTAMP(date_add(now(), interval %d second)) "
                    "WHERE email=\'%s\'",
                    NEW_REG_TABLE,
                    respobj->reg_verifycode().c_str(),
                    config->m_iEmailVerifyExpir,
                    reqobj->reg_name().c_str());
            break;

        default:
            ERR("**** SHOULD NEVER meet this verifiy code here(%d line)!!!\n", __LINE__);
            break;
    }

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);
        ERR("failed update verify code to DB:%s\n", mysql_error(ms));
        LOG("txt:%s\n", sqlcmd);
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    if(mysql_store_result(ms) == NULL)
    {
        INFO("mysql_store_result()==NULL, verify code insertion [OK]\n");
    }

    UNLOCK_CDS(urs_mutex);

    return 0;
}
