#ifndef _UUP_DATA_ACCESS_H
#define _UUP_DATA_ACCESS_H

#include "UpdateProfile.pb.h"
#include "UpdateProfileConfig.h"

using namespace com::caredear;


extern int record_user_login_info(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, UpdateProfileConfig *config);
extern int check_user_vcode(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, UpdateProfileConfig *config);


#endif
