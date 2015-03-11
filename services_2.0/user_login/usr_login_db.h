#ifndef _ULS_LOGIN_DB_H
#define _ULS_LOGIN_DB_H

#include "cds_public.h"
#include "UserLogin.pb.h"

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h> //MySQL error code
#include <string.h>
#include <stdlib.h>

#include <libmemcached/memcached.h>

// database
#define USER_MAIN_TABLE      "uc.uc_passport"
#define USER_SESSION_TABLE   "uc.uc_session"

using namespace com::caredear;

struct user_session {
    unsigned long  us_cid;       /**< caredear ID(unique for all users) */
    const char    *us_sessionid; /**< Session id description */
    const char    *us_token;     /**< This session's token string */
};

extern int match_user_credential_in_db(MYSQL *ms, LoginRequest *reqobj, unsigned long *p_cid);
extern int set_session_info_to_db(MYSQL *ms, struct user_session *session);

extern int gen_uuid(char *result);

extern memcached_return_t set_session_info_to_mem(memcached_st *memc, LoginRequest *reqobj, struct user_session *u);

#endif
