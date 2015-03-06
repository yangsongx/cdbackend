#ifndef UAS_AUTH_DB_H
#define UAS_AUTH_DB_H

#include "AuthOperation.h"

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
#include <string.h>

#define USER_SESSION_TABLE  "uc.uc_session"
#define USER_SESSION_CFG_TABLE  "uc.uc_sys_sessionconf"

extern int auth_token_in_session(MYSQL *ms, AuthRequest *reqobj);
extern int store_db_session_conf(MYSQL *ms, map<int, session_db_cfg_t> *pList);

#endif
