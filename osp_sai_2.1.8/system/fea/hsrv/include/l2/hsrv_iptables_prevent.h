#ifndef __HSRV_SYNC_IPTABLES_PREVENT_H__
#define __HSRV_SYNC_IPTABLES_PREVENT_H__
#include "ctc_l2.h"
#include "glb_l2_define.h"
#include "cfg_cmd.h"

int32
hsrv_iptables_prevent_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_IPTABLES_PREVENT_H__ */
