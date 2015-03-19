#include "NetdiskConfig.h"

/* NOTE , below global definition MUST be consistent with Qiniu SDK! */
const char  *QINIU_ACCESS_KEY;
const char  *QINIU_SECRET_KEY;

int NetdiskConfig::parse_cfg(const char *config_file)
{
    char buffer[32]; // be consistent with the data member size
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
            /* NOTE Using the same config XML section as 1.0-architecture */

            get_node_via_xpath("/config/netdisk/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("/config/netdisk/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("/config/netdisk/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            if(get_node_via_xpath("/config/netdisk/memcache/ip",
                    ctx, buffer, sizeof(buffer)) == 0)
            {
                strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));
            }

            if(get_node_via_xpath("/config/netdisk/memcache/port",
                    ctx, buffer, sizeof(buffer)) == 0)
            {
                m_iMemPort = atoi(buffer);
            }

            // QINIU-specific
            get_node_via_xpath("/config/netdisk/qiniu/access_key", ctx,
                    m_accessKey, sizeof(m_accessKey));

            get_node_via_xpath("/config/netdisk/qiniu/secret_key", ctx,
                    m_secretKey, sizeof(m_secretKey));

            QINIU_ACCESS_KEY = m_accessKey;
            QINIU_SECRET_KEY = m_secretKey;

            get_node_via_xpath("/config/netdisk/qiniu/domain", ctx,
                    m_qiniuDomain, sizeof(m_qiniuDomain));

            get_node_via_xpath("/config/netdisk/qiniu/bucket", ctx,
                    m_qiniuBucket, sizeof(m_qiniuBucket));

            get_node_via_xpath("/config/netdisk/qiniu/expiration", ctx,
                    buffer, sizeof(buffer));
            m_qiniuExpire = atoi(buffer);

            get_node_via_xpath("/config/netdisk/qiniu/quota", ctx,
                    buffer, sizeof(buffer));
            m_qiniuQuota = atoi(buffer);

            LOG("Access:%s,Secret:%s,domain:%s,bucket:%s\n",
                    m_accessKey, m_secretKey, m_qiniuDomain, m_qiniuBucket);

            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_db_and_mem();
}
