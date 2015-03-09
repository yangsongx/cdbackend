#ifndef _ACTS_AUTHOPR_H
#define _ACTS_AUTHOPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserActivation.pb.h"

#include "ActivationConfig.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class ActivateOperation{
    ActivationConfig *m_cfgInfo;

public:
    int set_conf(ActivationConfig *c);

    int compose_result(int code, const char *errmsg, ActivateResponse *p_obj, int *p_resplen, void *p_respdata);
    int begin_activation(ActivateRequest *reqobj, ActivateResponse *respobj, int *len_resp, void *resp);
};

#endif
