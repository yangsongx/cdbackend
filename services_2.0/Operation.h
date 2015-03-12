#ifndef _COM_CDS_OPERATION_H
#define _COM_CDS_OPERATION_H

#include "Config.h"

using namespace com::caredear;

namespace com{
namespace caredear{

    Config  *m_pCfg;

    class Operation {
        public:
            Operation(Config *c);

            int set_conf(Config *c);
            int keep_alive(const char *db_tbl, const char *col_name = "id");
    };
}
}

#endif
