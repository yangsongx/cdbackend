#include "VerifyCodeOperation.h"
#include "data_access.h"

int VerifyCodeOperation::set_conf(VerifyCodeConfig *c)
{
    m_cfgInfo = c;
    return 0;
}



int VerifyCodeOperation::gen_verifycode(char *result)
{
    char a[2];
    uuid_t id;
    char a1,a2,a3,a4,a5,a6;

    uuid_generate(id);

    // extract first-two and last-one digit hex as verifiy code
    *(unsigned short *) a  = (unsigned short )(id[0] << 16 | id[1] << 8 | id[15]);

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

int VerifyCodeOperation::compose_result(int code, const char *errmsg, UpdateResponse *p_obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    p_obj->set_result_code(code);
    if(code != CDS_OK && errmsg != NULL)
    {
        p_obj->set_extra_msg(errmsg);
    }

    len = p_obj->ByteSize();
    if(len >= DATA_BUFFER_SIZE)
    {
        ERR("Attention, exceed the max len, set it to a safe len\n");
        len = 4;
    }

    *p_resplen = (len + 2);
    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);
    // add the leading-length
    cos.WriteRaw(&len, sizeof(len));

    return ((p_obj->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}


/**
 * Begin do the User's Login action
 *
 *
 */
int VerifyCodeOperation::do_update_vcode(UpdateRequest *reqobj, UpdateResponse *respobj, int *len_resp, void *resp, char * vcode)
{
    int ret = -1;
    unsigned long cid = -1;
    ret = record_user_verifiy_code(m_cfgInfo->m_Sql, reqobj, respobj, m_cfgInfo, vcode);
    
    return ret;
}
