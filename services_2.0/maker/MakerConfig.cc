/**
 *
 * \history
 * [2015-06-12] Initial creation for maker system
 */
#include "MakerConfig.h"

/* NOTE , below global definition MUST be consistent with Qiniu SDK! */
const char  *QINIU_ACCESS_KEY;
const char  *QINIU_SECRET_KEY;

int MakerConfig::parse_cfg(const char *config_file)
{
    char buffer[32]; // hope it is enough
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
            get_node_via_xpath("//service_2/maker/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/maker/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/maker/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            // QINIU-specific
            get_node_via_xpath("/config/netdisk/qiniu/access_key", ctx,
                    m_accessKey, sizeof(m_accessKey));

            get_node_via_xpath("/config/netdisk/qiniu/secret_key", ctx,
                    m_secretKey, sizeof(m_secretKey));

            QINIU_ACCESS_KEY = m_accessKey;
            QINIU_SECRET_KEY = m_secretKey;

            /* FIXME - don't use memcache currently */

            xmlXPathFreeContext(ctx);
        }
        xmlFreeDoc(doc);
    }

    return prepare_db_and_mem();
}
