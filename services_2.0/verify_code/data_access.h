#ifndef _VCS_DATA_ACCESS_H
#define _VCS_DATA_ACCESS_H

#include "VerifyCode.pb.h"
#include "VerifyCodeConfig.h"

using namespace com::caredear;


extern int record_user_verifiy_code(MYSQL *ms, UpdateRequest *reqobj, UpdateResponse *respobj, VerifyCodeConfig *config, char *vcode);



#endif
