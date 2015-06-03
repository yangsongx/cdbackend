#ifndef _CDS_MAKER_OPERATION_H
#define _CDS_MAKER_OPERATION_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "MakerMessage.pb.h"

#include "MakerConfig.h"
#include "Operation.h"

using namespace std;
using namespace google::protobuf::io;

class MakerOperation : public com::caredear::Operation {
    public:
        MakerOperation() {
        }

        MakerOperation(Config *c) : com::caredear::Operation(c) {
        }

        virtual int handling_request(::google::protobuf::Message *reqobj,
                ::google::protobuf::Message *respobj,
                int *len_resp,
                void *resp);

        virtual int compose_result(int code, const char *errmsg,
                ::google::protobuf::Message *obj,
                int *p_resplen,
                void *p_respdata);
};

#endif
