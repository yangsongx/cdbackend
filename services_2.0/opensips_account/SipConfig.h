#ifndef _CDS_SIPSCONFIG_H
#define _CDS_SIPSCONFIG_H

#include "cds_public.h"
#include "Config.h"

#include <my_global.h>
#include <mysql.h>

#include <libmemcached/memcached.h>

class SipConfig : public com::caredear::Config
{
public:
    virtual int parse_cfg(const char *config_file);
};

#endif
