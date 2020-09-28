
#ifndef __TBL_PVLAN_DEFINE_H__
#define __TBL_PVLAN_DEFINE_H__

#include "gen/ds_pvlan_community_define.h"

/* TBL_PVLAN field defines */
typedef enum
{
    TBL_PVLAN_FLD_KEY                  = 0 ,  /* READ */
    TBL_PVLAN_FLD_COMMUNITY_GROUP_ID   = 1 ,  /* READ */
    TBL_PVLAN_FLD_COMMUNITY_GROUPS     = 2 ,  /* SUB */
    TBL_PVLAN_FLD_ISLOATE_PORTS        = 3 ,  /* RW */
    TBL_PVLAN_FLD_PROMISCUOUS_PORTS    = 4 ,  /* RW */
    TBL_PVLAN_FLD_MAX                  = 5 
} tbl_pvlan_field_id_t;

/* TBL_PVLAN defines */
typedef struct
{
    vid_t                primary_vid;
} tbl_pvlan_key_t;

typedef struct
{
    tbl_pvlan_key_t      key;
    bmp_32_t             community_group_id;  /* bitmap of alloced community group ID */
    cdb_reference_list_t community_groups;    /* list of community group */
    cdb_reference_list_t isloate_ports;       /* list of isloate ports */
    cdb_reference_list_t promiscuous_ports;   /* list of promiscuous ports */
} tbl_pvlan_t;

typedef struct
{
    tbl_pvlan_t          *pvlan_array[GLB_MAX_VLAN_ID];
} tbl_pvlan_master_t;

#endif /* !__TBL_PVLAN_DEFINE_H__ */

