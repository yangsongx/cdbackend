#ifndef _CDS_PASSWDMGR_CONFIG_H
#define _CDS_PASSWDMGR_CONFIG_H

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

/* NOTE , below MySQL header MUST NOT put ahead of C++ header,
 * which would cause min/max macro definition confliction! */
#include <my_global.h>
#include <mysql.h>

#include <libmemcached/memcached.h>

class PasswordConfig : public com::caredear::Config{

public:
    virtual int parse_cfg(const char *config_file);
};
#endif
