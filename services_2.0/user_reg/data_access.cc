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
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
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

int add_new_user_entry(MYSQL *ms, RegisterRequest *pRegInfo)
{
    int ret = -1;
    char sqlcmd[1024];
    char current[32]; // date '1990-12-12 12:12:12' is 20 len

    if(current_datetime(current, sizeof(current)) != 0)
    {
        /* FIXME - This SHOULD NEVER happen! */
        ERR("Warning, failed compose time, set a default time\n");
        strcpy(current, "1990-12-12 12:12:12");
    }

    switch(pRegInfo->reg_type())
    {
        case MOBILE_PHONE:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (usermobile,createtime) "
                    "VALUES (%s,%s)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(), current);
            break;

        case USER_NAME:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (username,loginpassword,createtime) "
                    "VALUES (%s,%s,%s)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(), 
                    pRegInfo->has_reg_password() ? pRegInfo->reg_password().c_str() : "",
                    current);
            break;

        case EMAIL:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "INSERT INTO %s (email,loginpassword,createtime) "
                    "VALUES (%s,%s,%s)",
                    NEW_REG_TABLE,
                    pRegInfo->reg_name().c_str(),
                    pRegInfo->has_reg_password() ? pRegInfo->reg_password().c_str() : "",
                    current);
            break;

        default:
            break;
    }

    LOCK_CDS(urs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(urs_mutex);
        ERR("failed execute the new user sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;

    // FIXME - insert into case, no result ?
    //
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(urs_mutex);

    return ret;
}

bool user_already_exist(MYSQL *ms, RegisterRequest *reqobj)
{
    char sqlcmd[1024];

    switch(reqobj->reg_type())
    {
        case MOBILE_PHONE:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE usermobile=\'%s\'",
                    NEW_REG_TABLE, reqobj->reg_name().c_str());
            break;

        case USER_NAME:
            break;

        case EMAIL:
            break;
    }
    return false;
}
