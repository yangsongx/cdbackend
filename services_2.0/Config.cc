#include "Config.h"
#include <errmsg.h>

using namespace com::caredear;


MYSQL *Config::conn_to_mysql(const char *ip, const char *usr, const char *passwd)
{
    MYSQL *s;

    s = mysql_init(NULL);
    if(s != NULL)
    {
        mysql_options(s, MYSQL_OPT_CONNECT_TIMEOUT, &m_iSqlConnTimeout);

        mysql_options(s, MYSQL_OPT_READ_TIMEOUT, &m_iSqlRdTimeout);
        mysql_options(s, MYSQL_OPT_WRITE_TIMEOUT, &m_iSqlWtTimeout);

        if(!mysql_real_connect(s, ip,
                    usr,
                    passwd,
                    "", // db keep blank
                    0,  // port , take default
                    NULL,
                    0))
        {
            ERR("**failed connecting to MySQL:(%d)%s\n",
                    mysql_errno(s), mysql_error(s));
            mysql_close(s);
            s = NULL;
        }
        else
        {
            INFO("Connecting to MySQL ... [OK]\n");
        }

        // TODO see not in the header
        if(pthread_mutex_init(&m_SqlMutex, NULL) != 0)
        {
            ERR("*** Warning, failed create mutex IPC objs:%d\n", errno);
        }
        else
        {
            INFO("Init SQL MUTEX ... [OK]\n");
        }
    }
    else
    {
        ERR("***Failed init the SQL env\n");
    }

    return s;
}


/**
 * Get MySQL and memcached handler.
 *
 */
int Config::prepare_db_and_mem()
{
    // first, MySQL
    m_Sql = conn_to_mysql(m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

    // next, memcached
    if(strlen(m_strMemIP) == 0)
    {
        INFO("No memcache config found\n");
        return 0;
    }

    // next, it is the memcached.
    char mem_cfg [128];
    snprintf(mem_cfg, sizeof(mem_cfg),
            "--SERVER=%s:%d", m_strMemIP, m_iMemPort);
    m_Memc = memcached(mem_cfg, strlen(mem_cfg));
    if(m_Memc != NULL)
    {
        INFO("Connecting to Memcached ... [OK]\n");
        if(memcached_behavior_set(m_Memc,
                MEMCACHED_BEHAVIOR_SUPPORT_CAS, 1) == MEMCACHED_SUCCESS)
        {
            INFO("Set CAS Support... [OK]\n");
        }
        else
        {
            ERR("Set CAS Support... [Failed]\n");
        }
    }
    else
    {
        ERR("failed connect to Memcached!\n");
    }

    return 0;
}

/**
 * reconnecting to @ip SQL server on the @disconnectS
 *
 * return reconnected SQL, if sth wrong, it is NULL
 */
MYSQL *Config::reconnect_sql(MYSQL *disconnectS, const char *ip, const char *usr, const char *passwd)
{
    INFO("try re-connecting to mySQL...\n");

    mysql_close(disconnectS);
    disconnectS = conn_to_mysql(ip, usr, passwd);

    return disconnectS;
}
