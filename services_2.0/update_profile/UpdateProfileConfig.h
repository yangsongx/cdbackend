#ifndef _UUP_CONFIG_H
#define _UUP_CONFIG_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <pthread.h>
#include <errno.h>

#include "cds_public.h"

/* NOTE , below MySQL header MUST NOT put ahead of C++ header,
 * which would cause min/max macro definition confliction! */
#include <my_global.h>
#include <mysql.h>

#include <libmemcached/memcached.h>

#include "Config.h"

class UpdateProfileConfig : public com::caredear::Config {


    public:
        UpdateProfileConfig(){
            m_strMemIP[0] = '\0';
        }

        virtual int parse_cfg(const char *config_file);
};

#endif
