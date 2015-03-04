#ifndef _URS_DATA_ACCESS_H
#define _URS_DATA_ACCESS_H

#include "UserRegister.pb.h"
#include "UserRegConfig.h"

using namespace com::caredear;

extern int add_new_user_entry(MYSQL *ms, RegisterRequest *pRegInfo);
extern int overwrite_inactive_user_entry(MYSQL *ms, RegisterRequest *pRegInfo, unsigned long user_id);

extern int record_user_verifiy_code(MYSQL *ms, RegisterRequest *reqobj, RegisterResponse *respobj, UserRegConfig *config);

extern bool user_already_exist(MYSQL *ms, RegisterRequest *reqobj, int *p_active_status, unsigned long *p_index);


#endif
