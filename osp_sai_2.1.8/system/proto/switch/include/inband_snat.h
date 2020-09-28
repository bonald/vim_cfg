#ifndef _INBAND_SNAT_H_
#define _INBAND_SNAT_H_

#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"

int32
inband_snat_start(switch_master_t *master);

int32
inband_snat_stop(switch_master_t *master);

tbl_inband_snat_t *
inband_sant_get_info_by_ifname(const char *ifname);

#endif
