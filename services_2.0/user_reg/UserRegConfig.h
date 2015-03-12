#ifndef _USRREG_CONFIG_H
#define _USRREG_CONFIG_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <pthread.h>
#include <errno.h>

#include "cds_public.h"

/* NOTE , below MySQL header MUST NOT put ahead of C++ header,
 * which would cause min/max macro definition confliction! */
#include <my_global.h>
#include <mysql.h>

#include <libmemcached/memcached.h>

class UserRegConfig{

    char  m_strSqlIP[32];
    char  m_strSqlUserName[32];
    char  m_strSqlUserPassword[32];
    int   m_iSqlPort;  // Note, currently, SQL port not used (consider 0 as the default)

    char  m_strMemIP[32];
    int   m_iMemPort;

    int prepare_job();

public:

    /* TODO , we need a constructor, for default value
     * when the config xml missing */


    // Below are two main Database Module(MySQL + Memcached)
    MYSQL *m_Sql;
    memcached_st *m_Memcached;
    ///////////////////////////////////////////////////////

    int   m_iMobileVerifyExpir;
    int   m_iEmailVerifyExpir;

    int init(const char *config_file);
    int reconnect_sql();
};

#endif
