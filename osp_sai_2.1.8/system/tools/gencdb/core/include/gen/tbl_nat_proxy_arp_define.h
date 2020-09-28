
#ifndef __TBL_NAT_PROXY_ARP_DEFINE_H__
#define __TBL_NAT_PROXY_ARP_DEFINE_H__

/* TBL_NAT_PROXY_ARP field defines */
typedef enum
{
    TBL_NAT_PROXY_ARP_FLD_KEY                  = 0 ,  /* READ */
    TBL_NAT_PROXY_ARP_FLD_ENABLE               = 1 ,  /* RW */
    TBL_NAT_PROXY_ARP_FLD_MAX                  = 2 
} tbl_nat_proxy_arp_field_id_t;

/* TBL_NAT_PROXY_ARP defines */
typedef nat_proxy_arp_key_t tbl_nat_proxy_arp_key_t;

typedef struct
{
    tbl_nat_proxy_arp_key_t key;
    uint32               enable;
} tbl_nat_proxy_arp_t;

typedef struct
{
    ctclib_hash_t        *nat_proxy_arp_hash;
    ctclib_slist_t       *nat_proxy_arp_list;
} tbl_nat_proxy_arp_master_t;

#endif /* !__TBL_NAT_PROXY_ARP_DEFINE_H__ */

