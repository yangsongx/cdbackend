/**
 * C code to operate the MySQL
 */


/*
   SQL Info for token:

   DATABASES - ucen
   TABLE     - USERS | USER_DETAILS


 */
#include <my_global.h>
#include <mysql.h>
#include <string.h>

#include "cds_public.h"
#include "token_def.h"

/* DATABASE Table Names */
#define USER_TBL        "ucen.USERS"
#define USER_DETAIL_TBL "ucen.USER_DETAILS"

/* Table's Column Name */
#define USER_TBL_USERNAME_COL  "username"

int ping_sql(MYSQL *ms)
{
    int ret = 0;
#if 0
    char sqlcmd[128];

    snprintf(sqlcmd, sizeof(sqlcmd), "SELECT * FROM %s WHERE username=\'13911111111\';",
            USER_TBL);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        ERR("**failed execute the ping SQL command:%s,error:%s\n",
                sqlcmd, mysql_error(ms));
        ret = -1;
    }
    else
    {
        /* MUST store the result, otherwise, will cauase CR_COMMANDS_OUT_OF_SYNC */
        MYSQL_RES *mresult;
        mresult = mysql_store_result(ms);
    }

    UNLOCK_SQL;
#endif
    return ret;
}

int find_user_details(MYSQL *ms, struct token_string_info *info)
{
    int  i;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd), "SELECT * FROM %s WHERE %s=\'%s\';",
            USER_TBL,
            USER_TBL_USERNAME_COL,
            info->tsi_userid);
    LOG("\tSQL:%s\n", sqlcmd);

    LOCK_SQL;

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**failed execute the SQL command:%s\n",
                mysql_error(ms));
        return -1;
    }

    LOG("execute the SQL cmd ... [OK]\n");

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    /* FIXME - MySQL doc said lock/unock between _query and _store_result */
    UNLOCK_SQL;

    if(mresult)
    {
        int b = mysql_num_fields(mresult);

        LOG("\tSQL RESULT:\n\trow lines=%llu, column numbers=%d\n",
                mysql_num_rows(mresult), b);
        while((row = mysql_fetch_row(mresult)))
        {
            for(i = 0; i < b; i++)
            {
                LOG("%s | ", row[i] ? row[i] : "NULL");
            }
            LOG("\n");
        }
    }

    mysql_free_result(mresult);

    return 0;
}

/**
 *Note - currently, we didn't use this as we use one single privateky to decrypt.
 */
int get_app_pubkey(MYSQL *ms, struct token_string_info *s, char *pk, int pk_size)
{
    char sqlcmd[256];
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT PUB_KEY from APPCERT WHERE APP_NAME=\'%s\';",
            s->tsi_appid);

    LOCK_SQL;

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**failed execute the SQL command:%s\n",
                mysql_error(ms));
        return -1;
    }

    LOG("execute the (%s) [OK]\n", sqlcmd);

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);

    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            /* got the first-match if multi hit */
            strncpy(pk, row[0], pk_size);
        }
        else
        {
            ERR("**failed fetch DB row:%s\n",
                    mysql_error(ms));
        }
    }
    return 0;
}
/**
 *
 *@c: cipher_token_string data.
 *@lastlogin_sec:  time_t format in DB LAST_LOGIN column.
 *@expiration_sec: time_t format in DB TOKEN_EXPIRATE column.
 *
 * Currently directly return 0, for successful.
 */
int update_token_time_to_db(MYSQL *ms, struct token_string_info *s, struct cipher_token_string_info *c, int lastlogin_sec, int expiration_sec)
{
    char sqlcmd[1024];
    time_t lt= (time_t)(s->tsi_login);
    char   readable_login[24];

    KPI("-->Update Time DB...\n");

    if((expiration_sec - s->tsi_login) < DELTA_OF_TOKEN_UPDATE)
    {
        /* user near expiration, so update TOKEN_EXPIRATE in DB */
        char readable_expire[24];

        strftime(readable_login, sizeof(readable_login), "%Y-%m-%d %H:%M:%S", localtime(&lt));

        expiration_sec += DELTA_OF_TOKEN_UPDATE;
        lt = (time_t)expiration_sec;
        strftime(readable_expire, sizeof(readable_expire),
                "%Y-%m-%d %H:%M:%S", localtime(&lt));

        snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET TOKEN_EXPIRATE=\'%s\',LAST_LOGIN=\'%s\' WHERE USER_TOKEN=\'%s\';",
                USER_DETAIL_TBL,
                readable_expire,
                readable_login,
                c->csi_expire);
        INFO("enlarge expiration:%s\n", sqlcmd);
    }
    else
    {

        if((s->tsi_login >= lastlogin_sec) && (s->tsi_login - lastlogin_sec) <= FREQUENT_VISIT_TIME)
        {
            INFO("Uer login within short time, avoid touch DB!\n");
            return 0;
        }

        strftime(readable_login, sizeof(readable_login), "%Y-%m-%d %H:%M:%S", localtime(&lt));

        LOG("Will update LAST_LOGIN with %s\n", readable_login);
        /* Just only update LAST_LOGIN */
        snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET LAST_LOGIN=\'%s\' WHERE USER_TOKEN=\'%s\';",
                USER_DETAIL_TBL,
                readable_login,
                c->csi_expire);
    }

    LOCK_SQL;

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**SQL command %s failed, reason:%s\n",
                sqlcmd, mysql_error(ms));
        return -1;
    }

    UNLOCK_SQL;

    KPI("<--Update the time in DB [OK]\n");

    return 0;
}

static int select_user_token_with_specified_id(MYSQL *ms, char *id, char *token_in_db, int token_size)
{
    int ret = -1;
    char sqlcmd[256];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT USER_TOKEN FROM %s WHERE USER_ID=%s",
            USER_DETAIL_TBL, id);

    LOCK_SQL;

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("*** failed execute SQL for ID - %s, error:%s\n",
                id, mysql_error(ms));
        return ret;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);

    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            /* got the first-match if multi hit */
            strncpy(token_in_db, row[0], token_size);
            LOG("(%s)--->(%s)\n", id, token_in_db);
            ret = 0;
        }
        else
        {
            ERR("Can't find %s id in DB\n", id);
            return -1;
        }
    }

    return ret;
}
/**
 *
 *@token_in_db : store the selected @uid's token data
 *@token_size  : specify @token_in_db buffer size, to avoid overflow
 *
 * return 0 means OK, -1 for failure.
 */
int get_token_from_db(MYSQL *ms, char *uid, char *token_in_db, int token_size)
{
    int ret = -1;
    char sqlcmd[256];
    char id[32];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ID FROM %s WHERE username=\'%s\'",
            USER_TBL,
            uid);

    LOCK_SQL;

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("*** failed execute SQL for UID - %s, error:%s\n",
                uid, mysql_error(ms));
        return ret;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);

    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            /* got the first-match if multi hit */
            strncpy(id, row[0], sizeof(id));
            LOG("%s=>%s\n", uid, id);
            ret = select_user_token_with_specified_id(ms, id, token_in_db, token_size);
        }
        else
        {
            ERR("Can't find %s uid in DB\n", uid);
            return -1;
        }
    }

    return ret;
}

/**
 *@cexpir_str : An ecrypt expire str.
 *@expir_str : DB's data for TOKEN_EXPIRATE
 *@expir_size: specify the @expir_str's length.
 *
 * return 0 for successful, -1 for error case, -2 for MySQL lost connection, and -3 for record NOT in DB.
 */
int fetch_expire_from_db(MYSQL *ms, const char *cexpir_str, char *lastlogin_str, int lastlogin_size, char *expir_str, int expir_size)
{
    char sqlcmd[1024];

    memset(expir_str, '\0', expir_size);
    memset(lastlogin_str, '\0', lastlogin_size);

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT TOKEN_EXPIRATE,LAST_LOGIN FROM %s WHERE USER_TOKEN=\'%s\';",
            USER_DETAIL_TBL,
            cexpir_str);

    LOCK_SQL;

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        /*
           Note -
           if a sql connection keep free for long time(default is 8h),
           SQL would close this.

           At this point, `MySQL server has gone away' is returned
           (mysql_errno() == CR_SERVER_GONE_ERROR).

           Need try re-connect to SQL if meet such case.
         */
        ERR("**failed execute the SQL command:%s\n",
                mysql_error(ms));
        if(strstr(mysql_error(ms), "server has gone away"))
        {
            /* set -2 mean MySQL disconnection due to no interactive data
               exchange during long-time(default is 8h) */
            return -2;
        }
        return -1;
    }

    LOG("execute the (%s) [OK]\n", sqlcmd);

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);

    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            /* got the first-match if multi hit */
            strncpy(expir_str, row[0], expir_size);

            //FIXME - for second column, it would be LAST_LOGIN data!
            if(mysql_num_fields(mresult) > 1)
            {
                strncpy(lastlogin_str, row[1], lastlogin_size);
            }
            else
            {
                ERR("*** can't get LAST_LOGIN column, check DB please!\n");
            }
        }
        else
        {
            ERR("**failed fetch DB row:%s, probably no record found\n",
                    mysql_error(ms));
            /* record not found, we need take a look! */
            return -3;
        }
    }

    return 0;
}

/**
 *@server: the MySQL server config info
 */
MYSQL *GET_MYSQL(struct sql_server_info *server)
{
    MYSQL *msql;

    LOG("mysql info:MYSQL-%s\n", mysql_get_client_info());
    msql = mysql_init(NULL);
    if(msql != NULL)
    {
        if(!mysql_real_connect(msql, server->ssi_server_ip,
                    server->ssi_user_name,
                    server->ssi_user_password,
                    "",//server->ssi_database,
                    server->ssi_server_port,
                    NULL,
                    0))
        {
            ERR("**failed connect MySQL:%s\n",
                mysql_error(msql));
            mysql_close(msql);
            msql = NULL;
        }
        else
        {
            LOG("\tConnecting to MySQL Database....[OK]\n");
        }
    }

    return msql;
}

void FREE_MYSQL(MYSQL *m)
{
    if(m != NULL)
        mysql_close(m);
}
