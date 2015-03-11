#ifndef UAS_AUTH_DB_H
#define UAS_AUTH_DB_H

#include "AuthOperation.h"

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
#include <string.h>

#include <libmemcached/memcached.h>

#define USER_SESSION_TABLE  "uc.uc_session"
#define USER_SESSION_CFG_TABLE  "uc.uc_sys_sessionconf"

// don't update lastlogin within 0.5 hours
#define MAX_FREQUENT_VISIT  (30*60)

/**
 * Wrapper for mem val's splitted fields
 */
struct auth_data_wrapper{
    unsigned long adw_cid;
    char          adw_session[64];  // act like device id
    time_t        adw_lastlogin;
};


extern int set_token_info_to_db(MYSQL *ms, AuthRequest *reqobj);
extern int get_token_info_from_db(MYSQL *ms, AuthRequest *reqobj, AuthResponse *respobj, struct auth_data_wrapper *w);

extern int store_db_session_conf(MYSQL *ms, map<int, session_db_cfg_t> *pList);

extern char *get_token_info_from_mem(memcached_st *memc, const char *key, size_t *p_vallen);
extern memcached_return_t set_token_info_to_mem(memcached_st *memc, const char *key, struct auth_data_wrapper *w);

extern int split_val_into_fields(char *value, struct auth_data_wrapper *w);

#endif
