#if !defined (__CTC_SAI_COPP_H_)
#define __CTC_SAI_COPP_H_

#include <sai.h>

typedef enum
{
    SAI_COPP_PORT_ID = 1,
    SAI_COPP_REASON_PORT_ID,
    SAI_COPP_FWD_PORT_ID,                   /* forward to CPU */
    SAI_COPP_HYBIRD_PORT_ID,                /* for hybrid/openflow */
    SAI_COPP_TTL_EXPIRE_PORT_ID,            /* for TTL expired */
    SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID,    /* for L2protocol */
    SAI_COPP_L2PRO_GROUP_MAC_PORT_ID,       /* for L2protocol */
#ifndef GREATBELT
    SAI_COPP_IGMP_PORT_ID,
#endif
    SAI_COPP_MTU_CHECK_FAIL_PORT_ID,        /* for mtu check fail */
    SAI_COPP_PORT_ID_MAX,
} ctc_sai_copp_iloop_port_t;

#define SAI_COPP_REASON_MAX_COUNT  1024  //same to CTC_PKT_CPU_REASON_MAX_COUNT

typedef struct ctc_sai_copp_reason_s
{
    uint32   reason_id;
    sai_object_id_t   nexthop_oid;
    sai_object_id_t   counter_oid;
    sai_object_id_t   counter_ssh_oid;
    sai_object_id_t   counter_tln_oid;
    sai_object_id_t   counter_mlag_oid;
    sai_object_id_t   acl_entry_oid[8];
    sai_object_id_t   acl_entry_ssh_oid[8];
    sai_object_id_t   acl_entry_tln_oid[8];
    sai_object_id_t   acl_entry_mlag_oid[8];
} ctc_sai_copp_reason_t;

typedef struct ctc_sai_copp_port_info_s
{
    uint16   port_id;
    uint32   nexthop_id;

} ctc_sai_copp_port_info_t;

typedef struct ctc_sai_copp_info_s
{
    ctc_sai_copp_reason_t ctc_reason[SAI_COPP_REASON_MAX_COUNT];  
    ctc_sai_copp_port_info_t ctc_copp_port_info[SAI_COPP_PORT_ID_MAX];
    sai_object_id_t   ctc_copp_igs_tbl_oid;

}ctc_sai_copp_info_t;

typedef enum
{
    CTC_SAI_COPP_EXT_OPENFLOW               = CTC_PKT_CPU_REASON_CUSTOM_BASE + 1,
    CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC    = CTC_PKT_CPU_REASON_CUSTOM_BASE + 2,
    CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC       = CTC_PKT_CPU_REASON_CUSTOM_BASE + 3,
    CTC_SAI_COPP_EXT_CPU_MIRROR             = CTC_PKT_CPU_REASON_CUSTOM_BASE + 4,
    CTC_SAI_COPP_EXT_MAX
} ctc_sai_copp_reason_ext_t;


uint32
ctc_sai_acl_get_iloop_port_from_copp_reason_group(uint32 copp_group_id);
sai_status_t
ctc_sai_copp_init();
uint32
ctc_sai_copp_get_fwd_port();
int32 
ctc_sai_copp_get_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid);
int32 
ctc_sai_copp_get_ssh_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid);
int32 
ctc_sai_copp_get_telnet_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid);
int32 
ctc_sai_copp_get_mlag_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid);
int32 
ctc_sai_copp_get_counter_oid_from_reason(uint32 reason_id, sai_object_id_t* counter_oid);

#endif

