#include "UserRegConfig.h"

/**
 * Read out init config XML file
 *
 * @config_file : the XML config file name(currently, it is /etc/cds_cfg.xml)
 *
 */
int UserRegConfig::init(const char *config_file)
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
            get_node_via_xpath("//service_2/user_register_service/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            get_node_via_xpath("//service_2/user_register_service/memcache/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));

            get_node_via_xpath("//service_2/user_register_service/memcache/port",
                    ctx, buffer, sizeof(buffer));
            m_iMemPort = atoi(buffer);

            LOG("the memcached ip:%s, port:%d\n", m_strMemIP, m_iMemPort);

            get_node_via_xpath("//service_2/user_register_service/mobile_verifycode_expiration",
                    ctx, buffer, sizeof(buffer));
            m_iMobileVerifyExpir = atoi(buffer);

            get_node_via_xpath("//service_2/user_register_service/email_verifycode_expiration",
                    ctx, buffer, sizeof(buffer));
            m_iEmailVerifyExpir = atoi(buffer);

            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_job();
}

/**
 * currently, just prepared SQL and memcache handler, based on the config
 *
 */
int UserRegConfig::prepare_job()
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
