
#ifndef __TBL_IPSG_FIB_DEFINE_H__
#define __TBL_IPSG_FIB_DEFINE_H__

/* TBL_IPSG_FIB field defines */
typedef enum
{
    TBL_IPSG_FIB_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPSG_FIB_FLD_VID                  = 1 ,  /* RW */
    TBL_IPSG_FIB_FLD_IFINDEX              = 2 ,  /* RW */
    TBL_IPSG_FIB_FLD_IFNAME               = 3 ,  /* RW */
    TBL_IPSG_FIB_FLD_MAC_ADDR             = 4 ,  /* RW */
    TBL_IPSG_FIB_FLD_IP_ADDR              = 5 ,  /* RW */
    TBL_IPSG_FIB_FLD_IPSG_TYPE            = 6 ,  /* RW */
    TBL_IPSG_FIB_FLD_SET_DHCBINDING_IPSG_TBL_EXSIT = 7 ,  /* RW */
    TBL_IPSG_FIB_FLD_MAX                  = 8 
} tbl_ipsg_fib_field_id_t;

/* TBL_IPSG_FIB defines */
typedef fdb_key_t tbl_ipsg_fib_key_t;

typedef struct
{
    tbl_ipsg_fib_key_t   key;
    vid_t                vid;                 /* VLAN id */
    uint32               ifindex;
    char                 ifname[IFNAME_SIZE];
    mac_addr_t           mac_addr;
    prefix_t             ip_addr;
    uint32               ipsg_type;           /* ip source guard type */
    uint32               set_dhcbinding_ipsg_tbl_exsit; /* notify dhcp snooping the correspondence IPSG tbl status */
} tbl_ipsg_fib_t;

typedef struct
{
    ctclib_hash_t        *ipsg_fib_hash;
    ctclib_slist_t       *ipsg_fib_list;
} tbl_ipsg_fib_master_t;

#endif /* !__TBL_IPSG_FIB_DEFINE_H__ */

