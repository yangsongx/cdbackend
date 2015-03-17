#ifndef _REG_OPERATION_H
#define _REG_OPERATION_H


#include "UserRegister.pb.h"
#include "UserRegConfig.h"
#include "uuid.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserRegister.pb.h"
#include "data_access.h"

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
