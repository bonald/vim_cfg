#if !defined (__SAI_ACL_H_)
#define __SAI_ACL_H_

#include <saiacl.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>
#include <ctc_acl.h>
#include <ctc_scl.h>

#define SAI_DEFINE_STANDAND_ACL_CTC  1

#define SAI_ATTR_ACL_TABLE_MINIMUM_PRIORITY 0
#define SAI_ATTR_ACL_TABLE_MAXIMUM_PRIORITY 0xffff

#define SAI_ATTR_ACL_ENTRY_MINIMUM_PRIORITY 0
#define SAI_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY 0xffff

#define SAI_ACL_GROUP_BASE                  36

#define SAI_ACL_WORM_FILTER_PRIORITY_BASE    10
#define SAI_ACL_WORM_FILTER_GROUP           (1022)
#define SAI_ACL_GLOBAL_INGRESS_GROUP        (1023)
#define SAI_ACL_GLOBAL_EGRESS_GROUP         (1024)

#define SAI_ACL_COPP_GROUP_PRIORITY_GB           1
#define SAI_ACL_COPP_GROUP_PRIORITY_GG           2
#define SAI_ACL_COPP_GROUP_PRIORITY_DUET2        3
#define SAI_ACL_COPP_GROUP_PRIORITY_DUET2_HYBRID 7
#define SAI_ACL_COPP_GROUP_PRIORITY_DUET2_TAP    7


typedef enum
{
    SAI_ACL_COPP_REASON_GROUP                   = 1025,
    SAI_ACL_COPP_FWD2CPU_GROUP                  = 1026,
    SAI_ACL_COPP_ACL_GROUP                      = 1027,
    SAI_ACL_COPP_OPENFLOW_TO_CONTROLLER_GROUP   = 1028,
    SAI_ACL_COPP_TTL_EXPIRE_GROUP               = 1029,
    SAI_ACL_COPP_L2PRO_PROTOCOL_MAC_GROUP       = 1030,
    SAI_ACL_COPP_L2PRO_GROUP_MAC_GROUP          = 1031,
    #ifndef GREATBELT
    SAI_ACL_COPP_IGMP_GROUP                     = 1032,
    SAI_ACL_COPP_MTU_CHECK_FAIL_GROUP           = 1033,
    SAI_ACL_COPP_REASON_GROUP_MAX               = 1034,
    #else
    SAI_ACL_COPP_MTU_CHECK_FAIL_GROUP           = 1032,
    SAI_ACL_COPP_REASON_GROUP_MAX               = 1033,
    #endif
} sai_acl_copp_reason_group_t;

#define SAI_ACL_COPP_DEFAULT_ACL_PRIORITY 0x1ffff
#define SAI_ACL_COPP_DEFAULT_ACL_ENTRY_ID 0x1ffff
#define SAI_ACL_COPP_DEFAULT_IPV6_ACL_PRIORITY 0x1fffe
#define SAI_ACL_COPP_DEFAULT_IPV6_ACL_ENTRY_ID 0x1fffe

#define SAI_ACL_MAC_WILDCARD_MASK(mac_addr) \
    mac_addr[0]=(~mac_addr[0]); \
    mac_addr[1]=(~mac_addr[1]); \
    mac_addr[2]=(~mac_addr[2]); \
    mac_addr[3]=(~mac_addr[3]); \
    mac_addr[4]=(~mac_addr[4]); \
    mac_addr[5]=(~mac_addr[5]);

#define SAI_ACL_VLAN_WILDCARD_MASK(vlanid) \
    (vlanid=((~vlanid)&0xfff));

#define SAI_ACL_COS_WILDCARD_MASK(cosid) \
    (cosid=((~cosid)&0x7));

#define SAI_ACL_DSCP_WILDCARD_MASK(dscp) \
    (dscp=((~dscp)&0x3F));

#define SAI_ACLV6_FLOW_LABEL_WILDCARD_MASK(label) \
    (label=((~label)&0xFFFFF));

#define SAI_ACL_WILDCARD_MASK(value) (value=(~value));

static const sai_ip6_t ipv6_mask_ff = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                                        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#define IS_EQ_WILDCARD_MASK_128(value) (0 == sal_memcmp(value, &ipv6_mask_ff, sizeof(sai_ip6_t)))

#define SAI_ACL_IPV6_WILDCARD_MASK(v6_addr) \
    v6_addr[0]=(~v6_addr[0]); \
    v6_addr[1]=(~v6_addr[1]); \
    v6_addr[2]=(~v6_addr[2]); \
    v6_addr[3]=(~v6_addr[3]); \
    v6_addr[4]=(~v6_addr[4]); \
    v6_addr[5]=(~v6_addr[5]); \
    v6_addr[6]=(~v6_addr[6]); \
    v6_addr[7]=(~v6_addr[7]); \
    v6_addr[8]=(~v6_addr[8]); \
    v6_addr[9]=(~v6_addr[9]); \
    v6_addr[10]=(~v6_addr[10]); \
    v6_addr[11]=(~v6_addr[11]); \
    v6_addr[12]=(~v6_addr[12]); \
    v6_addr[13]=(~v6_addr[13]); \
    v6_addr[14]=(~v6_addr[14]); \
    v6_addr[15]=(~v6_addr[15]); 


#define IS_EQ_WILDCARD_MASK_8(value) (value==0xFF)
#define IS_EQ_WILDCARD_MASK_16(value) (value==0xFFFF)
#define IS_EQ_WILDCARD_MASK_32(value) (value==0xFFFFFFFF)

typedef sai_status_t (*sai_acl_attr_mapping_cb)(
    _Out_ ctc_acl_entry_t *acl_entry,
    _In_ const sai_attribute_t *attr,
    stbl_acl_entry_t *p_sdb_entry
    );

typedef struct ctc_sai_acl_info_s
{
    sai_acl_attr_mapping_cb g_acl_action[64];
} ctc_sai_acl_info_t;


#define 	CTC_SAI_SCL_TUNNEL_RES				4096
#define 	CTC_SAI_SCL_TUNNEL_BLOCK_SIZE		32

enum
{
    L4_PROTOCOL_IPIP = 4,
    L4_PROTOCOL_TCP = 6,
    L4_PROTOCOL_UDP = 17,
    L4_PROTOCOL_GRE = 47,
};

enum
{
    L4_PROTOCOL_VXLAN_DEST_PORT = 4789,
};



typedef struct strip_header_parameter
{
    sai_acl_action_data_t    strip_header;
    sai_acl_action_data_t    strip_header_pos;
    sai_acl_action_data_t    strip_header_offset;
}strip_header_parameter_t;

enum{
    PACKET_UNKONW = 0,
    PACKET_VXLAN,
    PACKET_IPV4_GRE,
    PACKET_IPV4_IPIP,
    PACKET_MAX,
};


sai_status_t 
ctc_sai_acl_init();

int32_t
ctc_sai_acl_enable_global_port();

int 
ctc_sai_acl_port_enable(ctc_direction_t dir, sai_object_id_t port);

int
ctc_sai_acl_lag_member_enable(sai_object_id_t lag_id, sai_object_id_t lag_member_port_id);

int
ctc_sai_acl_lag_member_disable(sai_object_id_t lag_member_port_id, sai_object_id_t lag_id);

int
ctc_sai_acl_port_enable(ctc_direction_t dir, sai_object_id_t port);

int
ctc_sai_acl_lag_install(ctc_direction_t dir, sai_object_id_t lag_id);

#endif

