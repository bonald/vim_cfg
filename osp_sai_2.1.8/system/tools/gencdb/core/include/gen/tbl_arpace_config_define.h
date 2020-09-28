
#ifndef __TBL_ARPACE_CONFIG_DEFINE_H__
#define __TBL_ARPACE_CONFIG_DEFINE_H__

/* TBL_ARPACE_CONFIG field defines */
typedef enum
{
    TBL_ARPACE_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_ARPACE_CONFIG_FLD_ARP_TYPE             = 1 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_SRC_MAC              = 2 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_SRC_MAC_MASK         = 3 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_SRC_IP               = 4 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_SRC_IP_MASK          = 5 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_ARP_DENY             = 6 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_ARP_LOG              = 7 ,  /* RW */
    TBL_ARPACE_CONFIG_FLD_MAX                  = 8 
} tbl_arpace_config_field_id_t;

/* TBL_ARPACE_CONFIG defines */
typedef arpace_config_key_t tbl_arpace_config_key_t;

typedef struct
{
    tbl_arpace_config_key_t key;
    uint32               arp_type;            /*arp request or response or all*/
    mac_addr_t           src_mac;             /*source mac*/
    mac_addr_t           src_mac_mask;        /*source mac mask*/
    addr_ipv4_t          src_ip;              /*source ip*/
    addr_ipv4_t          src_ip_mask;         /*source ip mask*/
    uint32               arp_deny;            /*deny or permit*/
    uint32               arp_log;             /* log or not*/
} tbl_arpace_config_t;

typedef struct
{
    ctclib_hash_t        *arpace_config_hash;
    ctclib_slist_t       *arpace_config_list;
} tbl_arpace_config_master_t;

#endif /* !__TBL_ARPACE_CONFIG_DEFINE_H__ */

