#ifndef _NDS_CONFIG_H
#define _NDS_CONFIG_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <pthread.h>
#include <errno.h>

#include "cds_public.h"
#include "Config.h"


class NetdiskConfig : public com::caredear::Config {
public:
    char   m_accessKey[128];
    char   m_secretKey[128];

    char   m_qiniuDomain[128];
    char   m_qiniuBucket[128];
    unsigned int m_qiniuExpire;
    unsigned int m_qiniuQuota;

public:
    NetdiskConfig() {
        m_qiniuExpire = 2592000; // 30 days by default
        m_qiniuQuota = 10000; // TODO, this is so small, just for debug
    }

    virtual int parse_cfg(const char *config_file);
};

#endif
