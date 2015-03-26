/**
 * Common Definition for Circle Management Service(CMS)
 */
#ifndef _CMS_DEF_H
#define _CMS_DEF_H


#include <my_global.h>
#include <mysql.h>

#ifdef __cplusplus
#undef min
#undef max
#endif

#include "NetdiskMessage.pb.h"

using namespace com::caredear;


extern unsigned int qiniu_quota;
extern struct sql_server_info sql_cfg;

extern int INIT_DB_MUTEX();
extern int CLEAN_DB_MUTEX();

extern MYSQL *GET_CMSSQL(struct sql_server_info *server);
extern void FREE_CMSSQL(MYSQL *m);

extern int exceed_quota(MYSQL *ms, NetdiskRequest *p_obj);
extern int preprocess_upload_req (MYSQL *ms, NetdiskRequest *p_obj);
extern int update_user_uploaded_data(MYSQL *ms, NetdiskRequest *p_obj);
extern int get_netdisk_key(MYSQL *ms, NetdiskRequest *p_obj, char *p_result);
extern int remove_file_from_db(MYSQL *ms, NetdiskRequest *p_obj);
extern int share_file(MYSQL *ms, NetdiskRequest *p_obj, char *p_result);
extern int copy_shared_file(MYSQL *ms,  MYSQL_ROW  row, NetdiskRequest *p_obj);

#endif
