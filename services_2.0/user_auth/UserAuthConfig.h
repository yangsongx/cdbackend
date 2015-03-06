#ifndef _UAS_AUTHCONFIG_H
#define _UAS_AUTHCONFIG_H

#include "cds_public.h"

#include <string>
#include <map>
#include <list>

#include <my_global.h>
#include <mysql.h>
#include <libmemcached/memcached.h>

using namespace std;

typedef struct _session_db_cfg{
    int sfg_allow_multilogin;     /**< 0 - not allowed, 1 - allow login with multiple devices */
    unsigned long sfg_expiration;  /**< time interval for an expiration(in seconds) */
    int  sfg_type;                 /**< Currently not used yet */
}session_db_cfg_t;

class UserAuthConfig{
    char  m_strSqlIP[32];
    char  m_strSqlUserName[32];
    char  m_strSqlUserPassword[32];
    int   m_iSqlPort;  // Note, currently, SQL port not used (consider 0 as the default)

    char  m_strMemIP[32];
    int   m_iMemPort;

    int readout_db_session_conf();
    int prepare_job();
public:
    ///////////////////////////////////////////
    MYSQL *m_Sql;
    memcached_st *m_Memcached;
    ///////////////////////////////////////////



    // Contained the DB's session control config info
    map<int, session_db_cfg_t> m_sessionCfg;

    UserAuthConfig();
    ~UserAuthConfig();

    int init(const char *config_file);
};

#endif
