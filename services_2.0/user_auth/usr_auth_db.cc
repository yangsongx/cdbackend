#include "usr_auth_db.h"


extern pthread_mutex_t uas_mutex; // defined in the main()

/**
 * Modify the session's lastoperatetime if possible
 *
 */
int set_token_info_to_db(MYSQL *ms, AuthRequest *reqobj)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET lastoperatetime=NOW() WHERE ticket=\'%s\'",
            USER_SESSION_TABLE, reqobj->auth_token().c_str());

    KPI("Will try update last login DB data...\n");
    LOCK_CDS(uas_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uas_mutex);
        ERR("Failed update lastlogin:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uas_mutex);

    if(mresult)
    {
        ERR("**Warning, UPDATE should NEVER got non-NULL csse!\n");
    }
    else
    {
        KPI("Update the DB ... [OK]\n");
    }

    return 0;
}

/**
 * Check user's token and handle the expiration if possible.
 *
 *
 */
int get_token_info_from_db(MYSQL *ms, AuthRequest *reqobj, AuthResponse *respobj, struct auth_data_wrapper *w)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT caredearid,UNIX_TIMESTAMP(lastoperatetime) FROM %s "
            "WHERE ticket=\'%s\' AND session=\'%s\'",
            USER_SESSION_TABLE, reqobj->auth_token().c_str(), reqobj->auth_session().c_str());

    LOCK_CDS(uas_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uas_mutex);
        ERR("Failed check auth token:%s\n", mysql_error(ms));
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
            ERR("**Warning, multiple hit for token auth!\n");
        }

        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            //Got a lastlogin AND a caredear-ID
            if(row[0] != NULL)
            {
                respobj->set_caredear_id(atol(row[0]));
                w->adw_cid = atol(row[0]);
            }

            if(row[1] != NULL)
            {
                w->adw_lastlogin = atol(row[1]);
            }

            /* TODO - didn't set the adw_sysid here,
             * did this needed in our use case? */
        }
        else
        {
            // didn't find the matching
            ret = CDS_ERR_UMATCH_USER_INFO;
        }
    }
    else
    {
        ERR("mysql_store_result() should NEVER got NULL for SELECT!\n");
        ret = CDS_GENERIC_ERROR;
    }

    return ret;
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
