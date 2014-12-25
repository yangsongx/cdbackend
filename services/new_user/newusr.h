/**
 * Common Definition for Circle Management Service(CMS)
 */
#ifndef _NUS_DEF_H
#define _NUS_DEF_H


#include <my_global.h>
#include <mysql.h>

#ifdef __cplusplus
#undef min
#undef max
#endif

#include <libmemcached/memcached.hpp>
#include "cds_public.h"
#include "NewUserMessage.pb.h"

using namespace com::caredear;


struct nus_config
{
    struct sql_server_info sql_cfg;
    char memcach_ip[32];
    int  memcach_port;
};

extern memcached_st *memc;

extern int INIT_DB_MUTEX();
extern int CLEAN_DB_MUTEX();

extern MYSQL *GET_NUSSQL(struct sql_server_info *server);
extern void FREE_NUSSQL(MYSQL *m);

extern int keep_db_connected(MYSQL *ms);
extern int set_memcache(struct nus_config *p_cfg, const char *key, const char *value);
extern int is_user_registered(MYSQL *ms, const char *username);
#endif
