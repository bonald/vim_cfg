
#ifndef __TBL_VRRP_VMAC_DEFINE_H__
#define __TBL_VRRP_VMAC_DEFINE_H__

/* TBL_VRRP_VMAC field defines */
typedef enum
{
    TBL_VRRP_VMAC_FLD_KEY                  = 0 ,  /* READ */
    TBL_VRRP_VMAC_FLD_IFINDEX              = 1 ,  /* RW */
    TBL_VRRP_VMAC_FLD_MAX                  = 2 
} tbl_vrrp_vmac_field_id_t;

/* TBL_VRRP_VMAC defines */
typedef struct
{
    mac_addr_t           mac;
} tbl_vrrp_vmac_key_t;

typedef struct
{
    tbl_vrrp_vmac_key_t  key;
    uint32               ifindex;
} tbl_vrrp_vmac_t;

typedef struct
{
    ctclib_hash_t        *vrrp_vmac_hash;
    ctclib_slist_t       *vrrp_vmac_list;
} tbl_vrrp_vmac_master_t;

#endif /* !__TBL_VRRP_VMAC_DEFINE_H__ */

