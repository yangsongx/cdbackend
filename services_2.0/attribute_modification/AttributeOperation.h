#ifndef CDS_ATTR_OPR_H
#define CDS_ATTR_OPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "AttributeModify.pb.h"
#include "Operation.h"

using namespace std;
using namespace google::protobuf::io;

class AttributeOperation : public com::caredear::Operation {
public:
    AttributeOperation() {
    }

    AttributeOperation(Config *c) : com::caredear::Operation(c) {
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
