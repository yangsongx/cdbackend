#include "MakerOperation.h"

int MakerOperation::handling_request(::google::protobuf::Message *preqobj, ::google::protobuf::Message *prespobj, int *len_resp, void *resp)
{
    int ret = CDS_GENERIC_ERROR;
    MakerRequest *reqobj = (MakerRequest *)preqobj;
    MakerResponse *respobj = (MakerResponse *)prespobj;

    switch(reqobj->req_type())
    {
        case MakerReqType::GEN_PACKAGE:
            break;

        case MakerReqType::GET_UPTOKEN:
            break;

        case MakerReqType::UPLOAD_FINISH:
            break;

        default:
            break;
    }

    /* After finished all above processing, compose response obj */
    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for password manager result\n");
    }

    return ret;
}

int MakerOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    MakerResponse *p_obj = (MakerResponse *) obj;

    p_obj->set_result_code(code);

    // TODO more code needed here...

    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);
    // TODO, more code need added here...

    return ((obj->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}
