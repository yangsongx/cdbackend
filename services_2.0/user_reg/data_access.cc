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
