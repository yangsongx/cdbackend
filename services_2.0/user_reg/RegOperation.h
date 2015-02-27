#ifndef _REG_OPERATION_H
#define _REG_OPERATION_H

#include "uuid.h"

/**
 * Define the action for an incoming user reg request
 *
 */
class RegOperation {
    public:
        int init_env(const char *cfg_file);
        int sendback_reg_result();
};

#endif
