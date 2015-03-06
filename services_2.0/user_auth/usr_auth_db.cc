#include "usr_auth_db.h"


extern pthread_mutex_t uas_mutex; // defined in the main()

int auth_token_in_session(MYSQL *ms, AuthRequest *reqobj)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT lastoperatetime FROM %s WHERE ticket=\'%s\' AND session=\'%s\'",
            USER_SESSION_TABLE, reqobj->auth_token().c_str(), reqobj->auth_session().c_str());

    LOCK_CDS(uas_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uas_mutex);
        ERR("Failed check the login password:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uas_mutex);

    if(mresult)
    {
        int counts = mysql_num_rows(mresult);
        if(counts > 1)
        {
            // Multiple result
        }

        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            //
        }
        else
        {
            // didn't find the matching
        }
    }
    else
    {
    }

    return 0;
}

int store_db_session_conf(MYSQL *ms, map<int, session_db_cfg_t> *pList)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT sysid,isorder,lefttime,type FROM %s",
            USER_SESSION_CFG_TABLE);

    LOCK_CDS(uas_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uas_mutex);
        ERR("Failed check the login password:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uas_mutex);

    if(mresult)
    {
        int sysid;
        session_db_cfg_t t;

        if(mysql_num_fields(mresult) != 4)
        {
            ERR("Warning, session conf select col should be 4, don't process this!\n");
            return CDS_ERR_SQL_EXECUTE_FAILED;
        }

        // store the whole data one-by-one
        while((row = mysql_fetch_row(mresult)) != NULL)
        {
            sysid = atoi(row[0]);
            t.sfg_allow_multilogin = atoi(row[1]);
            t.sfg_expiration = atol(row[2]);
            t.sfg_type = atoi(row[3]);
            pList->insert(map<int, session_db_cfg_t>::value_type(sysid, t));
        }
    }
    else
    {
        // should never happen here.
    }

    return 0;
}
