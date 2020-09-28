
#ifndef __STBL_RIF_DEFINE_H__
#define __STBL_RIF_DEFINE_H__

/* STBL_RIF field defines */
typedef enum
{
    STBL_RIF_FLD_KEY                  = 0 ,  /* READ */
    STBL_RIF_FLD_TYPE                 = 1 ,  /* READ */
    STBL_RIF_FLD_PORT_OID             = 2 ,  /* READ */
    STBL_RIF_FLD_VR_OID               = 3 ,  /* READ */
    STBL_RIF_FLD_VLAN_ID              = 4 ,  /* READ */
    STBL_RIF_FLD_SMAC                 = 5 ,  /* READ */
    STBL_RIF_FLD_NAT_ENABLE           = 6 ,  /* READ */
    STBL_RIF_FLD_NAT_CATEGORY_ID      = 7 ,  /* READ */
    STBL_RIF_FLD_MAX                  = 8 
} stbl_rif_field_id_t;

/* STBL_RIF defines */
typedef struct
{
    uint32               rif_id;
} stbl_rif_key_t;

typedef struct
{
    stbl_rif_key_t       key;
    uint32               type;                /* sai_router_interface_type_t */
    sai_object_id_t      port_oid;
    sai_object_id_t      vr_oid;
    vid_t                vlan_id;
    mac_addr_t           smac;
    uint32               nat_enable;
    uint32               nat_category_id;
} stbl_rif_t;

typedef struct
{
    stbl_rif_t           *array[GLB_MAX_L3IF_NUM];
} stbl_rif_master_t;

#endif /* !__STBL_RIF_DEFINE_H__ */

