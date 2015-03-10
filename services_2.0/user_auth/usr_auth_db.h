#ifndef UAS_AUTH_DB_H
#define UAS_AUTH_DB_H

#include "AuthOperation.h"

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
#include <string.h>

#define USER_SESSION_TABLE  "uc.uc_session"
#define USER_SESSION_CFG_TABLE  "uc.uc_sys_sessionconf"

// don't update lastlogin within 0.5 hours
#define MAX_FREQUENT_VISIT  (30*60)

extern int update_session_lastoperatetime(MYSQL *ms, AuthRequest *reqobj);
extern int auth_token_in_session(MYSQL *ms, AuthRequest *reqobj, AuthResponse *respobj, time_t *p_lastoperatetime);

extern int store_db_session_conf(MYSQL *ms, map<int, session_db_cfg_t> *pList);

#endif
