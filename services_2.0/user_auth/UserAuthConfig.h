#ifndef _UAS_AUTHCONFIG_H
#define _UAS_AUTHCONFIG_H

#include "cds_public.h"

#include <string>
#include <map>
#include <list>

#include <my_global.h>
#include <mysql.h>
#include <libmemcached/memcached.h>

#include "Config.h"

using namespace std;

typedef struct _session_db_cfg{
    int sfg_allow_multilogin;     /**< 0 - not allowed, 1 - allow login with multiple devices */
    unsigned long sfg_expiration;  /**< time interval for an expiration(in seconds) */
    int  sfg_type;                 /**< Currently not used yet */
}session_db_cfg_t;

class UserAuthConfig : public com::caredear::Config{

    int readout_db_session_conf();
public:

    // Contained the DB's session control config info
    map<int, session_db_cfg_t> m_sessionCfg;

    UserAuthConfig();
    ~UserAuthConfig();

    virtual int parse_cfg(const char *config_file);
};

#endif
