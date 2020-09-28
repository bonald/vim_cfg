#ifndef _CTC_SAI_CLI_H_
#define _CTC_SAI_CLI_H_


#ifdef __cplusplus
extern C {
#endif


#include <sal.h>

#define SAI_CLI_OBJECT_VALUE        "<0-0xFFFFFFFF>"
#define SAI_CLI_OBJ_PORT_ID_DESC    "SAI port object id "
#define SAI_CLI_OBJ_LAG_ID_DESC     "SAI lag object id "
#define SAI_CLI_OBJ_STP_ID_DESC     "SAI stp object id "
#define SAI_CLI_OBJ_ROUTER_ID_DESC     "SAI router object id "
#define SAI_CLI_OBJ_ROUTER_INERFACE_ID_DESC     "SAI router interface object id "
#define SAI_CLI_OBJ_ROUTE_ID_DESC      "SAI route object id "
#define SAI_CLI_OBJ_NEXTHOP_ID_DESC      "SAI nexthop object id "
#define SAI_CLI_OBJ_NEXTHOP_GROUP_ID_DESC      "SAI nexthop group object id "

#define SAI_CLI_IPV4_DESC               "Ipv4 address"
#define SAI_CLI_IPV4_FORMAT             "IPv4 address in A.B.C.D format"
#define SAI_CLI_IPV4_MASK_FORMAT        "IPv4 address mask in A.B.C.D format"
#define SAI_CLI_IPV6_DESC               "Ipv6 address"
#define SAI_CLI_IPV6_FORMAT             "IPv6 address in X:X::X:X format"
#define SAI_CLI_IPV6_MASK_FORMAT        "IPv6 address mask in X:X::X:X format"
#define SAI_CLI_MAC_DESC                "MAC address"
#define SAI_CLI_MAC_FORMAT              "MAC address in HHHH.HHHH.HHHH format"
#define SAI_CLI_VLAN_DESC               "VLAN"
#define SAI_CLI_VLAN_RANGE_DESC         "<1-4094>"
#define SAI_CLI_SET_ATTRIBUTE_DESC      "Set attribute value"
#define SAI_CLI_GET_ATTRIBUTE_DESC      "Get attribute value"


#define SAI_CLI_PASER_NAME_MAC_ADDRESS  "MAC address"
#define SAI_CLI_PASER_NAME_VLAN         "vlan id"
#define SAI_CLI_PASER_UINT16            "uint16"
#define SAI_CLI_PASER_UINT32            "uint32"

#define SAI_CLI_GET_VLAN(vlan_value)                                \
    {                                                               \
        uint8_t  index = 0xFF;                                      \
        index = CTC_CLI_GET_ARGC_INDEX("vlan");                     \
        if (0xFF != index)                                          \
        {                                                           \
            CTC_CLI_GET_UINT16_RANGE(SAI_CLI_PASER_NAME_VLAN,       \
                                    vlan_value,                     \
                                    argv[index+1],                  \
                                    0,                              \
                                    CTC_MAX_UINT16_VALUE);          \
        }                                                           \
    }
#define SAI_CLI_GET_SAI_IP_PREFIX_T(sai_ip_prefix)                   \
    {                                                               \
        uint8_t  index = 0xFF;                                      \
        index = CTC_CLI_GET_ARGC_INDEX("ipv4");                     \
        if(0xFF != index)                                           \
        {                                                           \
            (sai_ip_prefix)->addr_family = SAI_IP_ADDR_FAMILY_IPV4;   \
    	    CTC_CLI_GET_IPV4_ADDRESS("ipv4", (sai_ip_prefix)->addr.ip4, argv[index+1]);  \
    	    CTC_CLI_GET_IPV4_ADDRESS("ipv4", (sai_ip_prefix)->mask.ip4, argv[index+2]);  \
        }                                                           \
        index = CTC_CLI_GET_ARGC_INDEX("ipv6");                     \
        if(0xFF != index)                                           \
        {                                                           \
            (sai_ip_prefix)->addr_family = SAI_IP_ADDR_FAMILY_IPV6;   \
    	    CTC_CLI_GET_IPV6_ADDRESS("ipv6", (sai_ip_prefix)->addr.ip6, argv[index+1]);  \
    	    CTC_CLI_GET_IPV6_ADDRESS("ipv6", (sai_ip_prefix)->mask.ip6, argv[index+2]);  \
        }                                                           \
    }

#define SAI_CLI_GET_SAI_IP_ADDRESS_T(sai_ip_addr)                   \
    {                                                               \
        uint8_t  index = 0xFF;                                      \
        index = CTC_CLI_GET_ARGC_INDEX("ipv4");                     \
        if(0xFF != index)                                           \
        {                                                           \
            (sai_ip_addr)->addr_family = SAI_IP_ADDR_FAMILY_IPV4;   \
    	    CTC_CLI_GET_IPV4_ADDRESS("ipv4", (sai_ip_addr)->addr.ip4, argv[index+1]);  \
        }                                                           \
        index = CTC_CLI_GET_ARGC_INDEX("ipv6");                     \
        if(0xFF != index)                                           \
        {                                                           \
            (sai_ip_addr)->addr_family = SAI_IP_ADDR_FAMILY_IPV6;   \
    	    CTC_CLI_GET_IPV6_ADDRESS("ipv6", (sai_ip_addr)->addr.ip6, argv[index+1]);  \
        }                                                           \
    }


#define SAI_CLI_GET_MAC_ADDRESS(pmac_address)                       \
    {                                                               \
        uint8_t  index = 0xFF;                                      \
        index = CTC_CLI_GET_ARGC_INDEX("mac");                      \
        if (0xFF != index)                                          \
        {                                                           \
            CTC_CLI_GET_MAC_ADDRESS(SAI_CLI_PASER_NAME_MAC_ADDRESS, \
                                    ((uint8_t*)pmac_address),       \
                                    argv[index+1]);                 \
        }                                                           \
    }

#define SAI_CLI_GET_PACKET_ACTION(action)                           \
    {                                                               \
        uint8_t  index = 0xFF;                                      \
        index = CTC_CLI_GET_ARGC_INDEX("trap");                     \
        if (0xFF != index)                                          \
        {                                                           \
            action = SAI_PACKET_ACTION_TRAP;                        \
        }                                                           \
                                                                    \
        index = CTC_CLI_GET_ARGC_INDEX("log");                      \
        if (0xFF != index)                                          \
        {                                                           \
            action = SAI_PACKET_ACTION_LOG;                         \
        }                                                           \
                                                                    \
        index = CTC_CLI_GET_ARGC_INDEX("drop");                     \
        if (0xFF != index)                                          \
        {                                                           \
            action = SAI_PACKET_ACTION_DROP;                        \
        }                                                           \
                                                                    \
        index = CTC_CLI_GET_ARGC_INDEX("forward");                  \
        if (0xFF != index)                                          \
        {                                                           \
            action = SAI_PACKET_ACTION_FORWARD;                     \
        }                                                           \
    }

#define SAI_CLI_GET_OBJECT_ID(prefix_name,type,object_id)           \
{                                                                   \
    uint8_t  index = 0xFF;                                          \
    uint32_t index_id = 0;                                          \
    index = CTC_CLI_GET_ARGC_INDEX(prefix_name);                    \
    if (0xFF != index)                                              \
    {                                                               \
        CTC_CLI_GET_UINT32_RANGE(prefix_name,                       \
                                 index_id,                          \
                                 argv[index+1],                     \
                                 0,                                 \
                                 CTC_MAX_UINT32_VALUE);             \
        object_id = CTC_SAI_OBJECT_TYPE_SET(type,index_id);         \
    }                                                               \
}

#define SAI_CLI_GET_PORT_ID(port_id)                                \
    SAI_CLI_GET_OBJECT_ID("port-id",SAI_OBJECT_TYPE_PORT,port_id);

#define SAI_CLI_GET_LAG_ID(lag_id)                                  \
    SAI_CLI_GET_OBJECT_ID("lag-id",SAI_OBJECT_TYPE_LAG,lag_id);

#define SAI_CLI_GET_STP_ID(stp_id)                                  \
    SAI_CLI_GET_OBJECT_ID("stp-id",SAI_OBJECT_TYPE_STP_INSTANCE,stp_id);

#define SAI_CLI_GET_ROUTER_ID(router_id)                            \
    SAI_CLI_GET_OBJECT_ID("router-id",SAI_OBJECT_TYPE_VIRTUAL_ROUTER,router_id);

#define SAI_CLI_GET_ROUTER_INTERFACE_ID(router_interface_id)        \
    SAI_CLI_GET_OBJECT_ID("router-interface-id",SAI_OBJECT_TYPE_ROUTER_INTERFACE,router_interface_id);

#define SAI_CLI_GET_NEXTHOP_ID(nexthop_id)                          \
    SAI_CLI_GET_OBJECT_ID("next-hop-id",SAI_OBJECT_TYPE_NEXT_HOP,nexthop_id);

#define SAI_CLI_GET_NEXTHOP_GROUP_ID(nexthop_group_id)              \
    SAI_CLI_GET_OBJECT_ID("next-hop-group-id",SAI_OBJECT_TYPE_NEXT_HOP_GROUP,nexthop_group_id);

extern int32 ctc_sai_cli_init(uint8 cli_tree_mode);

extern const char* ctc_sai_get_error_desc(int32_t );

#ifdef __cplusplus
}
#endif

#endif
