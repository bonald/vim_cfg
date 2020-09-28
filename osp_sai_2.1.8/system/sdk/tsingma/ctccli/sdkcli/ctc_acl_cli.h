/**
 @file ctc_acl_cli.h

 @date 2009-12-22

 @version v2.0

*/

#ifndef _CTC_ACL_CLI_H_
#define _CTC_ACL_CLI_H_

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"

#define CHECK_FLAG(V, F)      ((V)&(F))
#define SET_FLAG(V, F)        (V) = (V) | (F)
#define UNSET_FLAG(V, F)      (V) = (V)&~(F)

#define ACL_IGMP_TYPE_HOST_QUERY                 0x11
#define ACL_IGMP_TYPE_HOST_REPORT                0x12
#define ACL_IGMP_TYPE_HOST_DVMRP                 0x13
#define ACL_IGMP_TYPE_PIM                        0x14
#define ACL_IGMP_TYPE_TRACE                      0x15
#define ACL_IGMP_TYPE_V2_REPORT                  0x16
#define ACL_IGMP_TYPE_V2_LEAVE                   0x17
#define ACL_IGMP_TYPE_MTRACE                     0x1f
#define ACL_IGMP_TYPE_MTRACE_RESPONSE            0x1e
#define ACL_IGMP_TYPE_PRECEDENCE                 0          /*temp, maybe change later*/
#define ACL_IGMP_TYPE_V3_REPORT                  0x22

#define CTC_CLI_ACL_ADD_ENTRY_DESC \
    CTC_CLI_ACL_STR, \
    "Add one entry to software table", \
    CTC_CLI_ACL_GROUP_ID_STR, \
    CTC_CLI_ACL_B_GROUP_ID_VALUE, \
    CTC_CLI_ACL_ENTRY_ID_STR, \
    CTC_CLI_ACL_ENTRY_ID_VALUE, \
    CTC_CLI_ACL_ENTRY_PRIO_STR, \
    CTC_CLI_ACL_ENTRY_PRIO_VALUE

#define CTC_CLI_ACL_ENTRY_TYPE_STR "\
      mac-entry| ipv4-entry|ipv6-entry\
    | pbr-ipv4-entry| pbr-ipv6-entry| ipv4-ext-entry\
    | mac-ipv4-entry| mac-ipv4-ext-entry| ipv6-ext-entry| mac-ipv6-entry\
    | cid-entry| ipfix-basic-entry| forward-entry| forward-ext-entry| copp-entry| copp-ext-entry| udf-entry"

#define CTC_CLI_ACL_ENTRY_TYPE_DESC \
    "mac entry", "ipv4 entry","ipv6 entry", \
    "pbr ipv4 entry", "pbr ipv6 entry", "ipv4 extend entry", \
    "mac ipv4 entry", "mac ipv4 extend entry", "ipv6 extend entry", "mac ipv6 entry", \
    "cid entry", "ipfix basic entry", "forward entry", "forward extend entry", "copp entry", "copp extend entry", "udf entry"


#define CTC_CLI_ACL_HASH_ENTRY_TYPE_STR " hash-mac-entry| hash-ipv4-entry| hash-l2-l3-entry| hash-mpls-entry |hash-ipv6-entry"


#define CTC_CLI_ACL_HASH_ENTRY_TYPE_DESC   "hash mac entry", "hash ipv4 entry", "hash l2-l3 entry", "hash mpls entry",  "hash ipv6 entry"
#define CTC_CLI_FIELD_SEL_ID_FILED_MODE_DESC "Use field mode to select hash field"


#define CTC_CLI_ACL_KEY_TYPE_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("mac-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_MAC;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-mac-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_HASH_MAC;\
        gid = CTC_ACL_GROUP_ID_HASH_MAC;    \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_HASH_IPV4;\
        gid = CTC_ACL_GROUP_ID_HASH_IPV4;   \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-l2-l3-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_HASH_L2_L3;\
        gid = CTC_ACL_GROUP_ID_HASH_L2_L3;  \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-mpls-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_HASH_MPLS;\
        gid = CTC_ACL_GROUP_ID_HASH_MPLS;   \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_HASH_IPV6;\
        gid = CTC_ACL_GROUP_ID_HASH_IPV6;   \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbr-ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_PBR_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbr-ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_PBR_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-ext-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_IPV4_EXT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_MAC_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-ipv4-ext-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_MAC_IPV4_EXT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-ext-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_IPV6_EXT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_MAC_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cid-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_CID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipfix-basic-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_INTERFACE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("forward-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_FWD;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("forward-ext-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_FWD_EXT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copp-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_COPP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copp-ext-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_COPP_EXT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("udf-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key_type = CTC_ACL_KEY_UDF;\
        break;\
    }\
}while(0);

#ifdef __cplusplus
extern "C" {
#endif

extern void
cli_acl_port_bitmap_mapping(ctc_port_bitmap_t dst, ctc_port_bitmap_t src);

extern int32
ctc_acl_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

