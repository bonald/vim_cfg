#ifndef _NS_PORT_FORWARDING_H_
#define _NS_PORT_FORWARDING_H_

#include "gen/tbl_ns_port_forwarding_define.h"
#include "gen/tbl_ns_port_forwarding.h"

int32
ns_port_forwarding_op_entry(cdb_oper_t oper, tbl_ns_port_forwarding_t *entry);

int32
ns_port_forwarding_start(switch_master_t *master);

int32
ns_port_forwarding_stop(switch_master_t *master);

int32
ns_port_forwarding_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* _NS_PORT_FORWARDING_H_ */
