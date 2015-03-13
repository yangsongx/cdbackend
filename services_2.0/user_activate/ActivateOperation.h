#ifndef _ACTS_AUTHOPR_H
#define _ACTS_AUTHOPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserActivation.pb.h"

#include "ActivationConfig.h"
#include "Operation.h"
#include "activation_db.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class ActivateOperation : public com::caredear::Operation{

public:

    virtual int handling_request(::google::protobuf::Message *reqobj,
            ::google::protobuf::Message *respobj,
            int *len_resp,
            void *resp);

    virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *p_obj,
            int *p_resplen,
            void *p_respdata);
};

#endif
