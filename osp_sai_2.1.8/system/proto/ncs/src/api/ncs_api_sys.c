
#include "proto.h"
#include "ncs.h"
#include "ncs_api.h"

int32
ncs_api_set_hostname(cdb_cfg_t cfg, char *hostname)
{
    char cmd[CMD_BUF_256];
    sal_snprintf(cmd, CMD_BUF_256, "hostname/%s", hostname);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_SYS_GLOBAL, cmd, sal_strlen(cmd)+1);
}

