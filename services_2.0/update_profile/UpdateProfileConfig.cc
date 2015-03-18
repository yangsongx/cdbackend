#include "UpdateProfileConfig.h"

UpdateProfileConfig::UpdateProfileConfig()
{
    strcpy(m_strSqlIP, "127.0.0.1");
    strcpy(m_strMemIP, "127.0.0.1");
    m_iMemPort = 11211;
}

int UpdateProfileConfig::prepare_job()
{
    int ret = -1;

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
            ret = 0;
        }
    }


    // next, it is the memcached.
    char mem_cfg [128];
    snprintf(mem_cfg, sizeof(mem_cfg),
            "--SERVER=%s:%d", m_strMemIP, m_iMemPort);
    m_Memcached = memcached(mem_cfg, strlen(mem_cfg));
    if(m_Memcached != NULL)
    {
        INFO("Connecting to Memcached ... [OK]\n");
    }
    else
    {
        ERR("failed connect to Memcached!\n");
        ret = -1;
    }

    return ret; 
}

int UpdateProfileConfig::init(const char *config_file)
{
    char buffer[32]; // be consistent with UserRegConfig's data member size
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(config_file, F_OK) != 0)
    {
        ERR("\'%s\' not existed!\n", config_file);
        return -1;
    }

    doc = xmlParseFile(config_file);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("//service_2/update_user_profile/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/update_user_profile/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/update_user_profile/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            get_node_via_xpath("//service_2/update_user_profile/memcache/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));

            get_node_via_xpath("//service_2/update_user_profile/memcache/port",
                    ctx, buffer, sizeof(buffer));
            m_iMemPort = atoi(buffer);

            LOG("the memcached ip:%s, port:%d\n", m_strMemIP, m_iMemPort);

            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_job();
}

int UpdateProfileConfig::reconnect_sql()
{
    int ret = -1;

    mysql_close(m_Sql);

    if(!mysql_real_connect(m_Sql, m_strSqlIP,
                m_strSqlUserName,
                m_strSqlUserPassword,
                "", // db keep blank
                0,  // port , take default
                NULL,
                0))
    {
        ERR("**failed re-connecting to MySQL:%s\n",
                mysql_error(m_Sql));
        mysql_close(m_Sql);
        m_Sql = NULL;
    }
    else
    {
        INFO("Re-Connecting to MySQL ... [OK]\n");
        ret = 0;
    }

    return ret;
}

