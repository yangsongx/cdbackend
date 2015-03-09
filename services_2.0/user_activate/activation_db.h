#ifndef _ACTS_DB_H
#define _ACTS_DB_H

#include "ActivateOperation.h"

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
#include <string.h>

#define ACTIVATION_MAIN_TABLE  "uc.uc_passport"

extern int verify_activation_code(MYSQL *ms, ActivateRequest *reqobj);

#endif
