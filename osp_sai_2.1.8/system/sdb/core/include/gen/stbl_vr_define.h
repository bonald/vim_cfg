
#ifndef __STBL_VR_DEFINE_H__
#define __STBL_VR_DEFINE_H__

/* STBL_VR field defines */
typedef enum
{
    STBL_VR_FLD_KEY                  = 0 ,  /* READ */
    STBL_VR_FLD_VR_OID               = 1 ,  /* READ */
    STBL_VR_FLD_SMAC                 = 2 ,  /* READ */
    STBL_VR_FLD_V4_ADMIN             = 3 ,  /* READ */
    STBL_VR_FLD_V6_ADMIN             = 4 ,  /* READ */
    STBL_VR_FLD_RIF_LIST             = 5 ,  /* READ */
    STBL_VR_FLD_MAX                  = 6 
} stbl_vr_field_id_t;

/* STBL_VR defines */
typedef struct
{
    uint32               vrf_id;
} stbl_vr_key_t;

typedef struct
{
    stbl_vr_key_t        key;
    sai_object_id_t      vr_oid;
    mac_addr_t           smac;
    uint32               v4_admin;
    uint32               v6_admin;
    sdb_reference_list_t rif_list;            /* list of STBL_RIF */
} stbl_vr_t;

typedef struct
{
    stbl_vr_t            *array[GLB_MAX_VRFID];
} stbl_vr_master_t;

#endif /* !__STBL_VR_DEFINE_H__ */

