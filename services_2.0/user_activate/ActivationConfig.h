#ifndef _ACTS_AUTHCONFIG_H
#define _ACTS_AUTHCONFIG_H

#include "cds_public.h"

#include <string>
#include <map>
#include <list>

#include <my_global.h>
#include <mysql.h>
#include <libmemcached/memcached.h>

using namespace std;


class ActivationConfig{
    char  m_strSqlIP[32];
    char  m_strSqlUserName[32];
    char  m_strSqlUserPassword[32];
    int   m_iSqlPort;  // Note, currently, SQL port not used (consider 0 as the default)

    char  m_strMemIP[32];
    int   m_iMemPort;

    int prepare_job();
public:
    ///////////////////////////////////////////
    MYSQL *m_Sql;
    memcached_st *m_Memcached;
    ///////////////////////////////////////////


    ActivationConfig();
    ~ActivationConfig();

    int init(const char *config_file);
};

#endif
