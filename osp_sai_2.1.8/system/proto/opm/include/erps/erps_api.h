#ifndef __ERPS_API_H__
#define __ERPS_ARI_H__

#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "erps_define.h"
#include "erps.h"

int32
erps_clear_erps_domain_instnace(uint32 instance_id);

int32
erps_port_report_link_state(tbl_interface_t *p_if);

int32
erps_port_lacp_add_member(tbl_interface_t* p_if, ds_lag_t* p_lagif);

int32
erps_port_lacp_delete_member(tbl_interface_t* p_if, ds_lag_t* p_lagif);

int32
erps_port_lacp_attach_member(tbl_interface_t* p_if, ds_lag_t* p_lagif);

int32
erps_port_lacp_detach_member(tbl_interface_t* p_if, ds_lag_t* p_lagif);
#endif /* !__ERPS_ARI_H__ */
