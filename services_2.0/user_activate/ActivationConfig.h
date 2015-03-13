#ifndef _ACTS_AUTHCONFIG_H
#define _ACTS_AUTHCONFIG_H

#include "cds_public.h"
#include "Config.h"

using namespace std;

class ActivationConfig : public com::caredear::Config {

public:

    ActivationConfig();
    ~ActivationConfig();

    //Override functions
    virtual int parse_cfg(const char *config_file);
};

#endif
