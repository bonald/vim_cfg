
#ifndef __HSRV_BRGIF_H__
#define __HSRV_BRGIF_H__

#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_fea_brg_if_define.h"
#include "gen/tbl_fea_brg_if.h"

#define FEA_BRGIF_IS_LAG_GROUP(IF)      (GLB_IF_TYPE_LINKAGG_IF == (IF)->hw_type)
#define FEA_BRGIF_IS_LAG_MEMBER(IF)     ((GLB_IF_TYPE_LINKAGG_IF != (IF)->hw_type) && (IF)->lag_id)

int32
hsrv_brgif_set_mlag_id(uint32 ifindex, uint32 mlag_id);

int32
hsrv_brgif_set_mlag_peer_conf(uint32 ifindex, uint32 enable);

int32
hsrv_brgif_set_mlag_peer_if_up(uint32 ifindex, uint32 enable);

int32
hsrv_brgif_set_mlag_is_group(uint32 ifindex, uint32 is_group);

int32
hsrv_brgif_set_mac_learning_en(uint32 ifindex, uint32 enable);

int32
hsrv_brgif_set_port_security_en(uint32 ifindex, uint32 enable);

int32
hsrv_brgif_set_flags(uint32 ifindex, uint32 flags);

int32
hsrv_brgif_set_max_mac(uint32 ifindex, uint32 count);

int32
hsrv_brgif_set_lag_id(uint32 ifindex, uint32 lag_id);

tbl_fea_brg_if_t*
hsrv_brgif_get(uint32 ifindex);

int32
hsrv_brgif_add(tbl_interface_t *p_if, ds_brgif_t *p_brgif);

int32
hsrv_brgif_del(tbl_interface_t *p_if);

int32
hsrv_brgif_set_mstp_port_state(uint32 ifindex, uint32 fea_state);

int32
hsrv_brgif_set_msti_port_state(uint32 ifindex, uint32 instance, uint32 fea_state);

int32
hsrv_brgif_set_mirror_dest_port_enable(uint32 ifindex, uint32 is_mirror_dest);

int32
hsrv_brgif_start();

#endif /* !__HSRV_BRGIF_H__ */

