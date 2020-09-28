
#ifndef __TBL_KERNEL_IF_DEFINE_H__
#define __TBL_KERNEL_IF_DEFINE_H__

/* TBL_KERNEL_IF field defines */
typedef enum
{
    TBL_KERNEL_IF_FLD_KEY                  = 0 ,  /* READ */
    TBL_KERNEL_IF_FLD_NAME                 = 1 ,  /* RW */
    TBL_KERNEL_IF_FLD_IP_ADDR              = 2 ,  /* RW */
    TBL_KERNEL_IF_FLD_MASKLEN              = 3 ,  /* RW */
    TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6 = 4 ,  /* RW */
    TBL_KERNEL_IF_FLD_VRF_ID               = 5 ,  /* RW */
    TBL_KERNEL_IF_FLD_MAX                  = 6 
} tbl_kernel_if_field_id_t;

/* TBL_KERNEL_IF defines */
typedef struct
{
    uint32               ifindex;
} tbl_kernel_if_key_t;

typedef struct
{
    tbl_kernel_if_key_t  key;
    char                 name[IFNAME_SIZE];
    addr_ipv4_t          ip_addr;
    uint32               masklen;
    kernel_if_ipv6_addr_t ip_addr_prefix_length_v6;
    uint32               vrf_id;
} tbl_kernel_if_t;

typedef struct
{
    ctclib_hash_t        *kernel_if_hash;
    ctclib_slist_t       *kernel_if_list;
} tbl_kernel_if_master_t;

#endif /* !__TBL_KERNEL_IF_DEFINE_H__ */

