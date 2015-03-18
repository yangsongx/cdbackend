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
#define NEW_PASS_TABLE   "uc.uc_passport"
#define NEW_SESSION_TABLE   "uc.uc_session"

using namespace com::caredear;

extern pthread_mutex_t uup_mutex; // defined in main()


int record_user_login_info(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, UpdateProfileConfig *config)
{
    char sqlcmd[1024];
    int ret = CDS_OK;
    switch(reqobj->reg_type())
    {
        case Updatetype::MOBILE_PHONE:
                snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET usermobile=\'%s\'' WHERE id=\'%s\'",
                NEW_PASS_TABLE,
                reqobj->update_data().c_str(),
                reqobj->uid().c_str());
            break;

        case Updatetype::EMAIL:
                snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET email=\'%s\'' WHERE id=\'%s\'",
                NEW_PASS_TABLE,
                reqobj->update_data().c_str(),
                reqobj->uid().c_str());
            break;
        case Updatetype::USER_NAME:
                snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET username=\'%s\'' WHERE id=\'%s\'",
                NEW_PASS_TABLE,
                reqobj->update_data().c_str(),
                reqobj->uid().c_str());
            break;
        case Updatetype::PASSWORD:
                 snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET loginpassword=\'%s\'' WHERE id=\'%s\'",
                NEW_PASS_TABLE,
                reqobj->update_data().c_str(),
                reqobj->uid().c_str());
            break;
        default:
            ERR("**Warning, un-support update_type\n");
            break;
        
    }    
    LOCK_CDS(uup_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uup_mutex);
        ERR("failed execute update user profile sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    if(mysql_store_result(ms) == NULL)
    {
        INFO("update user profile in DB [OK]\n");
    }

    UNLOCK_CDS(uup_mutex);

    return ret;



}

/**
 *
 */
int check_user_vcode(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, UpdateProfileConfig *config)
{
    char sqlcmd[1024];
    int ret = CDS_OK;
     switch(reqobj->reg_type())
    {
        case Updatetype::MOBILE_PHONE:
             snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT * FROM %s WHERE id=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                NEW_PASS_TABLE,
                reqobj->uid().c_str(),
                reqobj->vcode().c_str());
            break;
        case Updatetype::EMAIL:
             snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT * FROM %s WHERE id=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                NEW_PASS_TABLE,
                reqobj->uid().c_str(),
                reqobj->vcode().c_str());
            break;
        case Updatetype::USER_NAME:
                return ret;
        case Updatetype::PASSWORD:
                return ret;
        default:
            ERR("**Warning, un-support update_type\n");
            break;

    }    
   

    LOCK_CDS(uup_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uup_mutex);
        if(mysql_errno(ms) == CR_SERVER_GONE_ERROR)
        {
            ERR("SQL GONE AWAY, need reconnecting\n");
            return CDS_ERR_SQL_DISCONNECTED;
        }
        else
        {
            ERR("Failed verify code SQL:%s\n", mysql_error(ms));
            return CDS_ERR_SQL_EXECUTE_FAILED;
        }
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uup_mutex);
    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            
            ret = -1;
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        ERR("store result got NULL for check verify code\n");
        ret = CDS_GENERIC_ERROR;
    }

    return ret;

}
