#ifndef _REG_OPERATION_H
#define _REG_OPERATION_H


#include "UserRegister.pb.h"
#include "UserRegConfig.h"
#include "uuid.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserRegister.pb.h"

/* NOTE , below MySQL header MUST NOT put ahead of C++ header,
 * which would cause min/max macro definition confliction! */
#include <my_global.h>
#include <mysql.h>
#include "uuid.h" //libuuid

#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;


/**
 * Define the action for an incoming user reg request
 *
 */
class RegOperation : public com::caredear::Operation{

        int record_user_verifiy_code_to_db(RegisterRequest *reqobj, RegisterResponse *respobj, UserRegConfig *config);
        bool user_already_exist(RegisterRequest *reqobj, int *p_active_status, uint64_t *p_index);
        int add_new_user_entry(RegisterRequest *pRegInfo, uint64_t *cid);
        int add_user_password_to_db(RegisterRequest *pRegInfo, unsigned long user_id, const char *passwd);
        int overwrite_inactive_user_entry(RegisterRequest *pRegInfo, unsigned long user_id);
        int add_opensips_entry(RegisterRequest *pRegInfo);

        static uint64_t m_cid;
        static int m_active_status;
        static int cb_check_user_existence(MYSQL_RES *p_result);

    public:
        // constructors...
        RegOperation() {
        }

        RegOperation(Config *c) : com::caredear::Operation(c) {
        }
        // end of constructors...

        int gen_verifycode(char *result);

        virtual int handling_request(::google::protobuf::Message *reg_req,
            ::google::protobuf::Message *reg_resp,
            int *len_resp,
            void *resp);

        virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *obj,
            int *p_resplen,
            void *p_respdata);
};

#endif
