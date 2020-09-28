
#ifndef __TBL_DHCBINDING_DEFINE_H__
#define __TBL_DHCBINDING_DEFINE_H__

/* TBL_DHCBINDING field defines */
typedef enum
{
    TBL_DHCBINDING_FLD_KEY                  = 0 ,  /* READ */
    TBL_DHCBINDING_FLD_VID                  = 1 ,  /* RW */
    TBL_DHCBINDING_FLD_IFNAME               = 2 ,  /* RW */
    TBL_DHCBINDING_FLD_IFNAME_NEW           = 3 ,  /* RW */
    TBL_DHCBINDING_FLD_CLIENT_MAC           = 4 ,  /* RW */
    TBL_DHCBINDING_FLD_CLIENT_IP            = 5 ,  /* RW */
    TBL_DHCBINDING_FLD_CIPSOUR              = 6 ,  /* RW */
    TBL_DHCBINDING_FLD_LEASE                = 7 ,  /* RW */
    TBL_DHCBINDING_FLD_LEASE_NEW            = 8 ,  /* RW */
    TBL_DHCBINDING_FLD_TRANSID              = 9 ,  /* RW */
    TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT       = 10,  /* RW */
    TBL_DHCBINDING_FLD_STATE                = 11,  /* RW */
    TBL_DHCBINDING_FLD_TYPE                 = 12,  /* RW */
    TBL_DHCBINDING_FLD_MAX                  = 13
} tbl_dhcbinding_field_id_t;

/* TBL_DHCBINDING defines */
typedef binding_key_t tbl_dhcbinding_key_t;

typedef struct
{
    tbl_dhcbinding_key_t key;
    vid_t                vid;                 /* VLAN id */
    char                 ifname[IFNAME_SIZE+1]; /* interface name */
    char                 ifname_new[IFNAME_SIZE+1]; /* interface name , the same mac client new request from diff interface*/
    mac_addr_t           client_mac;
    addr_t               client_ip;           /* ip address v4/v6 */
    uint32               cIpsour;
    uint32               lease;               /* lease time, seconds */
    uint32               lease_new;           /* lease time, seconds, for new ifname expire*/
    uint32               transid;             /* trans id, 4 bytes for v4 and 3 bytes for v6 */
    uint32               ipsg_tbl_exsit;      /* for after static IPSG tbl delete, dhcp snooping learning tbl add to ipsg */
    uint8                state;               /* entry state: request, running or all */
    uint8                type;                /* entry type: learning, configed or all */
} tbl_dhcbinding_t;

typedef struct
{
    ctclib_hash_t        *dhcbinding_hash;
    ctclib_slist_t       *dhcbinding_list;
    ctclib_hash_t        *dhcbinding_ip_hash;
    uint32               dhcbinding_count;
    uint32               dhcbindingv6_count;
} tbl_dhcbinding_master_t;

#endif /* !__TBL_DHCBINDING_DEFINE_H__ */

