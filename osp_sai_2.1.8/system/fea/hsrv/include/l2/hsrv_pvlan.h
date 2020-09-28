
#if 0
#ifndef __HSRV_SYNC_PVLAN_H__
#define __HSRV_SYNC_PVLAN_H__
#if 0
#include "gen/ds_pvlan_community_define.h"
#include "gen/ds_pvlan_community.h"
#endif

int32
hsrv_pvlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_pvlan_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
      
int32
hsrv_pvlan_community_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_pvlan_community_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_pvlan_get_group_oid(vid_t primary_vid, uint32 group_id, sai_object_id_t *oid);

#endif /* !__HSRV_SYNC_PVLAN_H__ */
#endif
