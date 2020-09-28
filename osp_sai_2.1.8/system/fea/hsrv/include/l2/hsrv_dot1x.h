#ifndef __HSRV_SYNC_DOT1X_H__
#define __HSRV_SYNC_DOT1X_H__
#include "ctc_l2.h"
#include "glb_l2_define.h"
#include "cfg_cmd.h"
#include "ctclib_opf.h"

#define HSRV_DOT1X_CLASS_ID_MIN  1
#define HSRV_DOT1X_CLASS_ID_MAX  61

#define HSRV_DOT1X_ENTRY_ID_MIN  2048
#define HSRV_DOT1X_ENTRY_ID_MAX  4096

#define HSRV_DOT1X_LABEL_OPF g_hsrv_dot1x_mac_db.p_opf_label
#define HSRV_DOT1X_CLASS_OPF g_hsrv_dot1x_mac_db.p_opf_class

#define HSRV_DOT1X_GROUP_BASE 0x8210000

struct hsrv_dot1x_mac_db_s
{
    ctclib_opf_t* p_opf_label;  
    ctclib_opf_t* p_opf_class;  
};
typedef struct hsrv_dot1x_mac_db_s hsrv_dot1x_mac_db_t;

int32 
hsrv_dot1x_init();

int32
hsrv_dot1x_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_dot1x_mac_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
#endif /* !__HSRV_SYNC_DOT1X_H__ */

