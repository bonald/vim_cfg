
#ifndef __HSRV_SYNC_MCFDB_H__
#define __HSRV_SYNC_MCFDB_H__
#include "ctc_l2.h"
#include "glb_l2_define.h"
#include "hsrv_brgif.h"

int32
hsrv_mcfdb_sai_add_member(tbl_mcfdb_t *p_mcfdb, uint32 ifindex);

int32
hsrv_mcfdb_sai_del_member(tbl_mcfdb_t *p_mcfdb, uint32 ifindex);

int32
hsrv_mcfdb_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
int32
hsrv_mcfdb_start();

#endif /* !__HSRV_SYNC_MCFDB_H__ */

