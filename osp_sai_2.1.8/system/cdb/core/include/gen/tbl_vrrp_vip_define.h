
#ifndef __TBL_VRRP_VIP_DEFINE_H__
#define __TBL_VRRP_VIP_DEFINE_H__

/* TBL_VRRP_VIP field defines */
typedef enum
{
    TBL_VRRP_VIP_FLD_KEY                  = 0 ,  /* READ */
    TBL_VRRP_VIP_FLD_VMAC                 = 1 ,  /* RW */
    TBL_VRRP_VIP_FLD_INTERFACE_MAC        = 2 ,  /* RW */
    TBL_VRRP_VIP_FLD_MAX                  = 3 
} tbl_vrrp_vip_field_id_t;

/* TBL_VRRP_VIP defines */
typedef addr_t tbl_vrrp_vip_key_t;

typedef struct
{
    tbl_vrrp_vip_key_t   key;
    mac_addr_t           vmac;                /* vrrp vmac for vip */
    mac_addr_t           interface_mac;       /* route_mac for vrrp interface */
} tbl_vrrp_vip_t;

typedef struct
{
    ctclib_hash_t        *vrrp_vip_hash;
    ctclib_slist_t       *vrrp_vip_list;
} tbl_vrrp_vip_master_t;

#endif /* !__TBL_VRRP_VIP_DEFINE_H__ */

