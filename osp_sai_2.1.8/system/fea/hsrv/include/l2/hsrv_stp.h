#ifndef __HSRV_SYNC_STP_H__
#define __HSRV_SYNC_STP_H__
#include "ctc_l2.h"
#include "glb_l2_define.h"
#include "cfg_cmd.h"

#define HSRV_DEF_INSTANCE   0
#define HSRV_OF_INSTANCE    64    /* mstp&erps&g8032 use 0-63, hybrid use 64, 64 should not be blocked */

int32
hsrv_stp_set_port_state(uint32 ifindex, uint32 instance, uint32 fea_state);

int32
hsrv_mstp_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
int32
hsrv_msti_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_mstp_instance_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_mstp_instance_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
    
int32
hsrv_erps_ring_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_stp_start();

#endif /* !__HSRV_SYNC_STP_H__ */

