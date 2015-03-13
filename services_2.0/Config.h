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
        int   prepare_db_and_mem();

    public:
        ///////////////////////////////////////////
        MYSQL            *m_Sql;
        pthread_mutex_t   m_SqlMutex; // TODO 2015-3-13, currently, we didn't use this mutex as SQL IPC,
                                      // as it is not proper pass it to separate xxx_db.cc source with this
                                      // mutex obj.
                                      //
                                      // I still use a global mutex in main() to do SQL IPC.
                                      //
                                      // I plan re-org the class into a better way future, to embeded this mutex
                                      // into whole program.
        memcached_st     *m_Memc;
        ///////////////////////////////////////////

        /* each components' config XML path is different,
         * so they MUST override this function */
        virtual int parse_cfg(const char *config_file) = 0;

        int reconnect_sql();
    };
}   //caredear
} // com

#endif
