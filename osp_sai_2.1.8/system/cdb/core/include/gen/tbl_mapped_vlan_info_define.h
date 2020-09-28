
#ifndef __TBL_MAPPED_VLAN_INFO_DEFINE_H__
#define __TBL_MAPPED_VLAN_INFO_DEFINE_H__

/* TBL_MAPPED_VLAN_INFO field defines */
typedef enum
{
    TBL_MAPPED_VLAN_INFO_FLD_KEY                  = 0 ,  /* READ */
    TBL_MAPPED_VLAN_INFO_FLD_MAPPED_SVID          = 1 ,  /* RW */
    TBL_MAPPED_VLAN_INFO_FLD_MAPPED_CVID          = 2 ,  /* RW */
    TBL_MAPPED_VLAN_INFO_FLD_RAW_VID_BMP          = 3 ,  /* RW */
    TBL_MAPPED_VLAN_INFO_FLD_MAX                  = 4 
} tbl_mapped_vlan_info_field_id_t;

/* TBL_MAPPED_VLAN_INFO defines */
typedef mapped_vlan_info_key_t tbl_mapped_vlan_info_key_t;

typedef struct
{
    tbl_mapped_vlan_info_key_t key;
    uint16               mapped_svid;         /* mapped svid */
    uint16               mapped_cvid;         /* mapped svid */
    vlan_bmp_t           raw_vid_bmp;         /* raw vlan bmp */
} tbl_mapped_vlan_info_t;

typedef struct
{
    ctclib_hash_t        *mapped_vlan_info_hash;
    ctclib_slist_t       *mapped_vlan_info_list;
} tbl_mapped_vlan_info_master_t;

#endif /* !__TBL_MAPPED_VLAN_INFO_DEFINE_H__ */

