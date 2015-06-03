#ifndef _CDS_MAKER_CONFIG_H
#define _CDS_MAKER_CONFIG_H

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

class MakerConfig : public com::caredear::Config {
    public:
        char   m_accessKey[128];
        char   m_secretKey[128];

        virtual int parse_cfg (const char *config_file);
};

#endif
