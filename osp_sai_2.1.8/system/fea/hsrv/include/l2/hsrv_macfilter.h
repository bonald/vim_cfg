#ifndef __HSRV_SYNC_MACFILTER_H__
#define __HSRV_SYNC_MACFILTER_H__
#include "ctc_l2.h"
#include "glb_l2_define.h"
#include "hsrv_brgif.h"


int32
hsrv_macfilter_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_macfilter_add_mac(tbl_macfilter_t* p_macfilter);

int32
hsrv_macfilter_del_mac(tbl_macfilter_t* p_macfilter);

int32 
hsrv_macfilter_cmd_show_macfilter(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hsrv_macfilter_start();

#endif /* !__HSRV_SYNC_MACFILTER_H__ */

