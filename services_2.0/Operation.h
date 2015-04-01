/**
 * A Base Class for all daemon operation, which usually manipulate the
 * DB or memcached.
 *
 * \history
 * [2015-04-01] Try fix the SQL time-out reconnecting BUG
 */
#ifndef _COM_CDS_OPERATION_H
#define _COM_CDS_OPERATION_H

#include <google/protobuf/message.h>
#include "Config.h"

// some common definiton, such as DB's table name
//
#define USERCENTER_MAIN_TBL     "uc.uc_passport"
#define USERCENTER_ATTR_TBL     "uc.uc_attributes"
#define USERCENTER_SESSION_TBL  "uc.uc_session"

#define NETDISK_USER_TBL        "netdisk.USERS"
#define NETDISK_FILE_TBL        "netdisk.FILES"

#define OPENSIPS_SUB_TBL        "opensips.subscriber"

using namespace com::caredear;

typedef int (*cb_sqlfunc)(MYSQL_RES *p_result);

namespace com{
namespace caredear{


    class Operation {
            int execute_sql(MYSQL **pms, const char *cmd, cb_sqlfunc sql_cb);

        public:
            Config  *m_pCfg;

            Operation();
            Operation(Config *c);

            int set_conf(Config *c);
            int keep_alive(const char *db_tbl, const char *col_name = "id");

            /* below 3 APIs aim to collect all components' memcached related
             * operation here */
            int set_mem_value(const char *key, const char *value, uint32_t flag = 0, time_t expiration = 0);
            int set_mem_value_with_cas(const char *key, const char *value, uint32_t flag = 0, time_t expiration = 0);
            char *get_mem_value(const char *key, size_t *p_valen, uint64_t *p_cas);

            int sql_cmd(const char *cmd, cb_sqlfunc sql_cb);
            int sql_cmd_via_transaction(int argc, char **argv, cb_sqlfunc sql_cb);

            int sql_cmd_with_specify_server(MYSQL **pms, const char *cmd, cb_sqlfunc sql_cb);

            /* each component's xxxOpr need override this API */
            virtual int handling_request(::google::protobuf::Message *reqobj,
                    ::google::protobuf::Message *respobj,
                    int *len_resp,
                    void *resp) = 0;

            virtual int compose_result(int code, const char *errmsg,
                    ::google::protobuf::Message *obj,
                    int *p_resplen,
                    void *p_respdata) = 0;
    };
}
}

#endif
