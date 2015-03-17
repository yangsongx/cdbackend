#ifndef _COM_CDS_OPERATION_H
#define _COM_CDS_OPERATION_H

#include <google/protobuf/message.h>
#include "Config.h"

// some common definiton, such as DB's table name
//
#define USERCENTER_MAIN_TBL     "uc.uc_passport"

using namespace com::caredear;

typedef int (*cb_sqlfunc)(MYSQL_RES *p_result);

namespace com{
namespace caredear{


    class Operation {
            int execute_sql(const char *cmd, cb_sqlfunc sql_cb);

        public:
            Config  *m_pCfg;

            Operation();
            Operation(Config *c);

            int set_conf(Config *c);
            int keep_alive(const char *db_tbl, const char *col_name = "id");

            /* below 3 APIs aim to collect all components' memcached related
             * operation here */
            int set_mem_value();
            int set_mem_value_with_cas(const char *key, uint64_t cas);
            char *get_mem_value(const char *key, size_t *p_valen, uint64_t *p_cas);

            int sql_cmd(const char *cmd, cb_sqlfunc sql_cb);

            /* each component's xxxOpr need override this API */
            virtual int handling_request(::google::protobuf::Message *reqobj,
                    ::google::protobuf::Message *respobj,
                    int *len_resp,
                    void *resp) = 0;

            virtual int compose_result(int code, const char *errmsg,
                    ::google::protobuf::Message *p_obj,
                    int *p_resplen,
                    void *p_respdata) = 0;
    };
}
}

#endif
