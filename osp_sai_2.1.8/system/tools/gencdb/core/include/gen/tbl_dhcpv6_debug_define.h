
#ifndef __TBL_DHCPV6_DEBUG_DEFINE_H__
#define __TBL_DHCPV6_DEBUG_DEFINE_H__

/* TBL_DHCPV6_DEBUG field defines */
typedef enum
{
    TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR       = 0 ,  /* RW */
    TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS      = 1 ,  /* RW */
    TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET      = 2 ,  /* RW */
    TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP        = 3 ,  /* RW */
    TBL_DHCPV6_DEBUG_FLD_MAX                  = 4 
} tbl_dhcpv6_debug_field_id_t;

/* TBL_DHCPV6_DEBUG defines */
typedef struct
{
    uint32               flags;               /* bitmap of DHCDV6BG_FLAG_ */
} tbl_dhcpv6_debug_t;

#endif /* !__TBL_DHCPV6_DEBUG_DEFINE_H__ */

