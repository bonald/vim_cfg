
#ifndef __PTP_OPS_H__
#define __PTP_OPS_H__

#include "ptp_datatype.h"

uint32
ptp_ops_is_tc(void);

uint32
ptp_ops_is_oc(void);

uint32
ptp_ops_ifp_invalid(tbl_ptp_port_t * p_port);

int32
ptp_ops_global_init(void);

void
ptp_ops_port_init_prop(tbl_ptp_port_t* p_port);

void
ptp_ops_init_dynamic_properties(void);

void
ptp_ops_intf_init_port_config(tbl_ptp_port_t* p_port);

int32
ptp_ops_intf_init_port(tbl_ptp_port_t* p_port);

int32
ptp_ops_intf_fini_port(tbl_ptp_port_t* p_port);

int32
ptp_ops_switch_device_type(uint32 dev_type);

int32
ptp_ops_link_up_switch(tbl_ptp_port_t* p_port);

int32
ptp_ops_tocpu_vid_by_global_enable(uint32 enable);

int32
ptp_ops_set_global_enable(uint32 enable);

int32
ptp_ops_set_port_enable(tbl_ptp_port_t* p_port, uint32 enable);

uint32
ptp_ops_portid_identical_with_self(tbl_ptp_port_t* p_port, glb_ptp_port_id_t* p_port_id);

int32
ptp_ops_port_free_all_cache(tbl_ptp_port_t* p_port);
#endif /*!__PTP_OPS_H__*/

