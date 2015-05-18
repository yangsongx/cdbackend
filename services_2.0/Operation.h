/**
 * A Base Class for all daemon operation, which usually manipulate the
 * DB or memcached.
 *
 * \history
 * [2015-04-13] Fix random(6-digit) code generation bug
 * [2015-04-10] Let SQL command support extra parameter, which would avoid using static variable in caller
 * [2015-04-01] Try fix the SQL time-out reconnecting BUG
 */
#ifndef _COM_CDS_OPERATION_H
#define _COM_CDS_OPERATION_H

#include <google/protobuf/message.h>
#include "Config.h"
#include "uuid.h"

// some common definiton, such as DB's table name
//
#define USERCENTER_MAIN_TBL     "uc.uc_passport"
#define USERCENTER_ATTR_TBL     "uc.uc_attributes"
#define USERCENTER_SESSION_TBL  "uc.uc_session"

#define NETDISK_USER_TBL        "netdisk.USERS"
#define NETDISK_FILE_TBL        "netdisk.FILES"

#define OPENSIPS_SUB_TBL        "opensips.subscriber"

/* NOTE - iOS client had bug, whose session is '(null)'
 * Server had to do a workaround to resolve this
 */
#define IOS_BUG_SESSION  "(null)"

using namespace com::caredear;

typedef int (*cb_sqlfunc)(MYSQL_RES *p_result, void *p_extra);

namespace com{
namespace caredear{


    class Operation {
            memcached_return_t execute_get_mem_value(const char *key, size_t *p_valen, uint64_t *p_cas, char **pp_val);
            int execute_sql(MYSQL **pms, const char *cmd, cb_sqlfunc sql_cb, void *p_extra);

        public:
            Config  *m_pCfg;

            Operation();
            Operation(Config *c);

            int set_conf(Config *c);
            int keep_alive(const char *db_tbl, const char *col_name = "id");

            /* below 4 APIs aim to collect all components' memcached related
             * operation here */
            int set_mem_value(const char *key, const char *value, uint32_t flag = 0, time_t expiration = 0);
            int set_mem_value_with_cas(const char *key, const char *value, uint32_t flag = 0, time_t expiration = 0);
            char *get_mem_value(const char *key, size_t *p_valen, uint64_t *p_cas, memcached_return_t *rc);
            memcached_return_t  rm_mem_value(const char *key);

            int sql_cmd(const char *cmd, cb_sqlfunc sql_cb, void *p_extra);
            int sql_cmd_via_transaction(int argc, char **argv, cb_sqlfunc sql_cb);

            int sql_cmd_with_specify_server(MYSQL **pms, const char *cmd, cb_sqlfunc sql_cb, void *p_extra);

            /* each component's xxxOpr need override this API */
            virtual int handling_request(::google::protobuf::Message *reqobj,
                    ::google::protobuf::Message *respobj,
                    int *len_resp,
                    void *resp) = 0;

            virtual int compose_result(int code, const char *errmsg,
                    ::google::protobuf::Message *obj,
                    int *p_resplen,
                    void *p_respdata) = 0;

            /* FIXME seems put uuid into CDS is not a good idea, I decide pack it as a static(public) here */
            static int get_uuid(char *result) {
                uuid_t id;
                uuid_generate(id);
                /* NOTE - actually, we can call uuid_unparse() to below code line! */
                sprintf(result,
                        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        id[0], id[1], id[2], id[3],
                        id[4], id[5],
                        id[6], id[7],
                        id[8], id[9],
                        id[10], id[11], id[12], id[13], id[14], id[15]);
                /* currently always return 0 */
                return 0;
            }

            /* this is creating a random code based on uuid */
            static int gen_random_code(char *result) {
                char a[4];
                uuid_t id;
                char a1,a2,a3,a4,a5,a6;

                uuid_generate(id);

                // extract first-two and last-one digit hex as verifiy code
                *(int *) a  = (int)(id[0] << 16 | id[1] << 8 | id[15]);

                a1 = (a[0] & 0x0F);
                a2 = ((a[0] & 0xF0) >> 4);
                a3 = (a[1] & 0x0F);
                a4 = ((a[1] & 0xF0) >> 4);
                a5 = (a[2] & 0x0F);
                a6 = ((a[2] & 0xF0) >> 4);


                sprintf(result, "%c%c%c%c%c%c",
                     a1 > 9 ? (a1 - 6) + 0x30 : a1 + 0x30,
                     a2 > 9 ? (a2 - 6) + 0x30  : a2 + 0x30,
                     a3 > 9 ? (a3 - 6) +0x30 : a3 + 0x30,
                     a4 > 9 ? (a4 - 6) + 0x30 : a4 + 0x30,
                     a5 > 9 ? (a5 - 6) +0x30 : a5 + 0x30,
                     a6 > 9 ? (a6 - 6) + 0x30 : a6 + 0x30
                );

                return *(int *) a;
            }
    };
}
}

#endif
