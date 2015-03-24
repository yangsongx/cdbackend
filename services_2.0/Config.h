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

        int  m_iSqlConnTimeout;
        /* below are SQL R/W timeout(in second Unit) */
        int  m_iSqlRdTimeout;
        int  m_iSqlWtTimeout;

    protected:
        // SQL related config info...
        char  m_strSqlIP[32];
        char  m_strSqlUserName[32];
        char  m_strSqlUserPassword[32];
        int   m_iSqlPort;  // Note, currently, SQL port not used (consider 0 as the default)

        // memcached related config info...
        char  m_strMemIP[32];
        int   m_iMemPort;

    protected:
        int   conn_to_mysql();
        int   prepare_db_and_mem();

    public:
        ///////////////////////////////////////////
        MYSQL            *m_Sql;
        pthread_mutex_t   m_SqlMutex;
        memcached_st     *m_Memc;
        ///////////////////////////////////////////

        Config() {
            m_strSqlIP[0] = '\0';
            m_strMemIP[0] = '\0';

            m_iSqlConnTimeout = 5;
            /* FIXME - by default, set it to a shorter 4-second */
            m_iSqlRdTimeout = m_iSqlWtTimeout = 4;
        }

        /* each components' config XML path is different,
         * so they MUST override this function */
        virtual int parse_cfg(const char *config_file) = 0;

        int reconnect_sql();
    };
}   //caredear
} // com

#endif
