#ifndef _NS_ROUTE_H_
#define _NS_ROUTE_H_

#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"

int32
ns_route_start(switch_master_t *master);

int32
ns_route_stop(switch_master_t *master);

/* support sflow by liwh for bug 43981, 2017-06-05 */
int32
ns_route_delete_route(char *input, cfg_cmd_para_t *para);
/* liwh end */

/* modified by liwh for bug 45810, 2018-02-06 */
int32
ns_route_check_inband(char *input, cfg_cmd_para_t *para);
/* liwh end */
#endif
