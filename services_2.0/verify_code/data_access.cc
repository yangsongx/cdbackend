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
#define NEW_SESSION_TABLE   "uc.uc_session"

using namespace com::caredear;

extern pthread_mutex_t vcs_mutex; // defined in main()

/**
 *
 */
int record_user_verifiy_code(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, VerifyCodeConfig *config, char *vcode)
{
    char sqlcmd[1024];
    snprintf(sqlcmd, sizeof(sqlcmd),
                    "UPDATE %s SET accode=\'%s\',codetime=UNIX_TIMESTAMP(date_add(now(), interval %d second)) "
                    "WHERE id=(SELECT caredearid FROM %s WHERE ticket=\'%s\')",
                    NEW_REG_TABLE,
                    vcode,//respobj->reg_verifycode().c_str(),
                    config->m_iMobileVerifyExpir,
                    NEW_SESSION_TABLE,
                    reqobj->reg_ticket().c_str());

    LOCK_CDS(vcs_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(vcs_mutex);
        ERR("failed update verify code to DB:%s\n", mysql_error(ms));
        LOG("txt:%s\n", sqlcmd);
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    if(mysql_store_result(ms) == NULL)
    {
        INFO("mysql_store_result()==NULL, verify code insertion [OK]\n");
    }

    UNLOCK_CDS(vcs_mutex);

    return 0;
}
