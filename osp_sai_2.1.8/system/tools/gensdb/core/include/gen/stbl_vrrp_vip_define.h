
#ifndef __STBL_VRRP_VIP_DEFINE_H__
#define __STBL_VRRP_VIP_DEFINE_H__

/* STBL_VRRP_VIP field defines */
typedef enum
{
    STBL_VRRP_VIP_FLD_KEY                  = 0 ,  /* READ */
    STBL_VRRP_VIP_FLD_VMAC                 = 1 ,  /* READ */
    STBL_VRRP_VIP_FLD_IF_MAC               = 2 ,  /* READ */
    STBL_VRRP_VIP_FLD_MAX                  = 3 
} stbl_vrrp_vip_field_id_t;

/* STBL_VRRP_VIP defines */
typedef sai_ip_address_t stbl_vrrp_vip_key_t;

typedef struct
{
    stbl_vrrp_vip_key_t  key;
    mac_addr_t           vmac;
    mac_addr_t           if_mac;
} stbl_vrrp_vip_t;

typedef struct
{
    ctclib_hash_t        *vip_hash;
} stbl_vrrp_vip_master_t;

#endif /* !__STBL_VRRP_VIP_DEFINE_H__ */

