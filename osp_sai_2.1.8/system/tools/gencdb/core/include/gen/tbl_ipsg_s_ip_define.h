
#ifndef __TBL_IPSG_S_IP_DEFINE_H__
#define __TBL_IPSG_S_IP_DEFINE_H__

/* TBL_IPSG_S_IP field defines */
typedef enum
{
    TBL_IPSG_S_IP_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPSG_S_IP_FLD_VID                  = 1 ,  /* RW */
    TBL_IPSG_S_IP_FLD_IFINDEX              = 2 ,  /* RW */
    TBL_IPSG_S_IP_FLD_IFNAME               = 3 ,  /* RW */
    TBL_IPSG_S_IP_FLD_MAC_ADDR             = 4 ,  /* RW */
    TBL_IPSG_S_IP_FLD_IP_ADDR              = 5 ,  /* RW */
    TBL_IPSG_S_IP_FLD_MAX                  = 6 
} tbl_ipsg_s_ip_field_id_t;

/* TBL_IPSG_S_IP defines */
typedef struct
{
    prefix_t             ip;
} tbl_ipsg_s_ip_key_t;

typedef struct
{
    tbl_ipsg_s_ip_key_t  key;
    vid_t                vid;                 /* VLAN id */
    uint32               ifindex;
    char                 ifname[IFNAME_SIZE];
    mac_addr_t           mac_addr;
    prefix_t             ip_addr;
} tbl_ipsg_s_ip_t;

typedef struct
{
    ctclib_hash_t        *ipsg_s_ip_hash;
    ctclib_slist_t       *ipsg_s_ip_list;
} tbl_ipsg_s_ip_master_t;

#endif /* !__TBL_IPSG_S_IP_DEFINE_H__ */

