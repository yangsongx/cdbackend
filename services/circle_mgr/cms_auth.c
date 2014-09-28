/**
 * Do authentication before send the SYNC circle msg to user
 *
 */
#include <stdlib.h>
#include <string.h>

#include "cds_public.h"
#include "cms_def.h"

int extract_cms_req(char *str, struct circle_sync_req *sync_req)
{
    char *savedptr;
    char *time_from;

    /* NOTE  [2014-07-31]

       I temp define the req (WebServer(Java)->CMS(C/C++) as string:

       ---------------------------------------------------------------------------
       | LEN | token base 64 str |  uid  | time(sync start point) | ... |
       ---------------------------------------------------------------------------
             |<------------------- req (with size LEN) ---------------->|

       when coming here, the leading-LEN is stripped by the CDS framework.
     */
    if((sync_req->sreq_token = strtok_r(str, "#", &savedptr)) != NULL)
    {
        if((sync_req->sreq_uid = strtok_r(NULL, "#", &savedptr)) != NULL)
        {
            if((time_from = strtok_r(NULL, "#", &savedptr)) != NULL)
            {
                sync_req->sreq_from = atoi(time_from);
                LOG("GOOD to extract all string sections.\n");
            }
        }
    }
    return 0;
}

/**
 * TODO  - we need put decrypt/encrypt code into
 * the common CDS section!
 */
char *decrypt_data(const char *encrypt_data, char *key)
{
    return "MAGIC";
}

/**
 *@str : send from client(Java Web,usually)
 *@sync_req : stored the parsed req data sections
 *
 * return 1 means auth pass, otherwise return 0
 */
int cms_auth(char *str, struct circle_sync_req *sync_req)
{
    int ret = 0;
    char *plain_data = NULL;

    extract_cms_req(str, sync_req);

    LOG("Phase-I of processing, the incoming data:\n");
    LOG("token:%s,uid:%s,time:%d\n", sync_req->sreq_token,
            sync_req->sreq_uid, sync_req->sreq_from);

    plain_data = decrypt_data(sync_req->sreq_token, NULL);
    if(plain_data != NULL)
    {
        ret = 1;

        // TODO more code to do auth here.
    }

    return ret;
}
