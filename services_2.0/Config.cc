#include "Config.h"

using namespace com::caredear;

/**
 * Get MySQL and memcached handler.
 *
 */
int Config::prepare_db_and_mem()
{
    m_Sql = mysql_init(NULL);
    if(m_Sql != NULL)
    {
        if(!mysql_real_connect(m_Sql, m_strSqlIP,
                    m_strSqlUserName,
                    m_strSqlUserPassword,
                    "", // db keep blank
                    0,  // port , take default
                    NULL,
                    0))
        {
            ERR("**failed connecting to MySQL:%s\n",
                    mysql_error(m_Sql));
            mysql_close(m_Sql);
            m_Sql = NULL;
        }
        else
        {
            INFO("Connecting to MySQL ... [OK]\n");
        }
    }
    else
    {
        ERR("***Failed init the SQL env\n");
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

