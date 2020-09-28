
#ifndef __TBL_MANAGE_IF_DEFINE_H__
#define __TBL_MANAGE_IF_DEFINE_H__

/* TBL_MANAGE_IF field defines */
typedef enum
{
    TBL_MANAGE_IF_FLD_ADDR                 = 0 ,  /* RW */
    TBL_MANAGE_IF_FLD_GATEWAY              = 1 ,  /* RW */
    TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE      = 2 ,  /* RW */
    TBL_MANAGE_IF_FLD_DHCLIENT_ADDR        = 3 ,  /* RW */
    TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY     = 4 ,  /* RW */
    TBL_MANAGE_IF_FLD_ADDR_V6              = 5 ,  /* RW */
    TBL_MANAGE_IF_FLD_GATEWAY_V6           = 6 ,  /* RW */
    TBL_MANAGE_IF_FLD_LINK_LOCAL_ADDR_STR  = 7 ,  /* RW */
    TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE    = 8 ,  /* RW */
    TBL_MANAGE_IF_FLD_DHCLIENTV6_ADDR      = 9 ,  /* RW */
    TBL_MANAGE_IF_FLD_DHCLIENTV6_GATEWAY   = 10,  /* RW */
    TBL_MANAGE_IF_FLD_MAX                  = 11
} tbl_manage_if_field_id_t;

/* TBL_MANAGE_IF defines */
typedef struct
{
    prefix_ipv4_t        addr;
    addr_ipv4_t          gateway;
    uint8                dhclient_enable;     /* dhcp client enable */
    prefix_ipv4_t        dhclient_addr;
    addr_ipv4_t          dhclient_gateway;
    prefix_t             addr_v6;
    prefix_t             gateway_v6;
    char                 link_local_addr_str[64];
    uint8                dhclientv6_enable;   /* dhcpv6 client enable */
    prefix_t             dhclientv6_addr;
    prefix_t             dhclientv6_gateway;
} tbl_manage_if_t;

#endif /* !__TBL_MANAGE_IF_DEFINE_H__ */

