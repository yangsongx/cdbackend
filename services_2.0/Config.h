/**
 * A common base class for Configuration
 *
 */
#ifndef _COM_CDS_CONFIG_H
#define _COM_CDS_CONFIG_H 

#include "cds_public.h"

#include <my_global.h>
#include <mysql.h>

#include <libmemcached/memcached.h>

namespace com{
namespace caredear{

    class Config{
        // SQL related config info...
        char  m_strSqlIP[32];
        char  m_strSqlUserName[32];
        char  m_strSqlUserPassword[32];
        int   m_iSqlPort;  // Note, currently, SQL port not used (consider 0 as the default)

        // memcached related config info...
        char  m_strMemIP[32];
        int   m_iMemPort;

        int   prepare_db_and_mem();

    public:
        MYSQL            *m_Sql;
        pthread_mutex_t  *m_SqlMutex;
        memcached_st     *m_Memc;

        /* each components' config XML path is different,
         * so they MUST override this function */
        virtual int parse_cfg(const char *config_file) = 0;
        
    };
}   //caredear
} // com

#endif
