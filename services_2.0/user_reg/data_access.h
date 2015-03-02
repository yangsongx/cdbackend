#ifndef _URS_DATA_ACCESS_H
#define _URS_DATA_ACCESS_H

#include "UserRegister.pb.h"

using namespace com::caredear;

extern int add_new_user_entry(MYSQL *ms, RegisterRequest *pRegInfo);
extern bool user_already_exist(MYSQL *ms, RegisterRequest *reqobj);

#endif
