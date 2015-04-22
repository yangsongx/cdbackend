/**
 * Base class for all services' config, whose main job is prepare
 * SQL, memcache related info, based on a config file.
 *
 * \history
 * [2015-04-09] call mysql_set_character_set() after connection,
 *              to make it consistent with MySQL dev doc.
 *              (http://dev.mysql.com/doc/refman/5.5/en/mysql-set-character-set.html)
 * [2015-04-08] Let MySQL support UTF-8, as we need Chinese Character
 */
#include "Config.h"
#include <errmsg.h>

using namespace com::caredear;

/**
 * Try connecting to memached server via @ip:@port
 *
 * return NULL if failed connect to the server.
 */
memcached_st *Config::conn_to_memcached(const char *ip, int port)
{
    char cfg[128];

    snprintf(cfg, sizeof(cfg), "--SERVER=%s:%d", ip, port);
    m_Memc = memcached(cfg, strlen(cfg));
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

    return m_Memc;
}

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

            if(mysql_set_character_set(s, "utf8"))
            {
                ERR("Warning , set UTF-8 failed\n");
            }
            else
            {
                INFO("Set SQL character set as UTF-8 [OK]\n");
            }
        }

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

    // here means memcach is available
    conn_to_memcached(m_strMemIP, m_iMemPort);

    return 0;
}

/**
 * reconnecting to @ip SQL server on the @disconnectS
 *
 * return reconnected SQL, if sth wrong, it is NULL
 */
MYSQL *Config::reconnect_sql(MYSQL *disconnectS, const char *ip, const char *usr, const char *passwd)
{
    MYSQL *m;
    INFO("try re-connecting to mySQL...\n");

    mysql_close(disconnectS);

    m = conn_to_mysql(ip, usr, passwd);

    return m;
}
