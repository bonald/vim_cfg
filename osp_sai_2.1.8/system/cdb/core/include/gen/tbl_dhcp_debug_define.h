
#ifndef __TBL_DHCP_DEBUG_DEFINE_H__
#define __TBL_DHCP_DEBUG_DEFINE_H__

/* TBL_DHCP_DEBUG field defines */
typedef enum
{
    TBL_DHCP_DEBUG_FLD_RELAY_ERROR          = 0 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_RELAY_EVENTS         = 1 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_RELAY_PACKET         = 2 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_RELAY_DUMP           = 3 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR       = 4 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT       = 5 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET      = 6 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP        = 7 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR     = 8 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS    = 9 ,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_PACKET    = 10,  /* RW */
    TBL_DHCP_DEBUG_FLD_SNOOPINGV6_DUMP      = 11,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENT_ERROR         = 12,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS        = 13,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENT_PACKET        = 14,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENT_DUMP          = 15,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENTV6_ERROR       = 16,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENTV6_EVENTS      = 17,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENTV6_PACKET      = 18,  /* RW */
    TBL_DHCP_DEBUG_FLD_CLIENTV6_DUMP        = 19,  /* RW */
    TBL_DHCP_DEBUG_FLD_MAX                  = 20
} tbl_dhcp_debug_field_id_t;

/* TBL_DHCP_DEBUG defines */
typedef struct
{
    uint32               flags;               /* bitmap of DHCDBG_FLAG_ */
} tbl_dhcp_debug_t;

#endif /* !__TBL_DHCP_DEBUG_DEFINE_H__ */

