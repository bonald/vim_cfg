/**
 @file ctc_vlan.h

 @date 2009-12-11

 @version v2.0

 The file define api struct used in VLAN.
*/
#ifndef _CTC_VLAN_H
#define _CTC_VLAN_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common/include/ctc_const.h"
#include "common/include/ctc_parser.h"
#include "common/include/ctc_common.h"

/*****
 *@defgroup vlan VLAN
 *@{
 *****/


/**
 @defgroup VlanMapping VlanMapping
 @{
*/

/**
 @brief define vlan mapping vlan tag operation
*/

enum ctc_vlan_tag_op_e
{
    CTC_VLAN_TAG_OP_NONE,                                    /**< [GB.GG.D2.TM] operation invalid*/
    CTC_VLAN_TAG_OP_REP_OR_ADD,                              /**< [GB.GG.D2.TM] replace for tagged, add for no tag*/
    CTC_VLAN_TAG_OP_ADD,                                     /**< [GB.GG.D2.TM] append a new tag even if packet already has tag*/
    CTC_VLAN_TAG_OP_DEL,                                     /**< [GB.GG.D2.TM] delete packet's tag*/
    CTC_VLAN_TAG_OP_REP,                                     /**< [GB.GG.D2.TM] replace for tagged. do nothing for no tag*/
    CTC_VLAN_TAG_OP_VALID,                                   /**< [GB.GG.D2.TM] Operation valid, but do nothing. */
    CTC_VLAN_TAG_OP_MAX
};
typedef enum ctc_vlan_tag_op_e ctc_vlan_tag_op_t;

/**
 @brief define vlan mapping vid or cos operation
*/

enum ctc_vlan_tag_sl_e
{
    CTC_VLAN_TAG_SL_AS_PARSE,                                /**< [GB.GG.D2.TM] select parser result vid/cos/cfi*/
    CTC_VLAN_TAG_SL_ALTERNATIVE,                           /**< [GB.GG.D2.TM] select the other tag's vid/cos/cfi, if the other tag not present, use default*/
    CTC_VLAN_TAG_SL_NEW,                                        /**< [GB.GG.D2.TM] select user assigned vid/cos/cfi */
    CTC_VLAN_TAG_SL_DEFAULT,                                  /**< [GB.GG.D2.TM] select default vid/cos/cfi in ingress, select mapped cos/cfi in egress, but not support for vid in egress*/
    CTC_VLAN_TAG_SL_MAX
};
typedef enum ctc_vlan_tag_sl_e ctc_vlan_tag_sl_t;

/**
 @brief define key type for egress vlan mapping
*/
enum ctc_egress_vlan_mapping_key_e
{
    CTC_EGRESS_VLAN_MAPPING_KEY_NONE       = 0x00000000,       /**< [GB.GG.D2.TM] egress vlan mapping only use port as key*/
    CTC_EGRESS_VLAN_MAPPING_KEY_CVID       = 0x00000001,        /**< [GB.GG.D2.TM] egress vlan mapping use cvid as key*/
    CTC_EGRESS_VLAN_MAPPING_KEY_SVID       = 0x00000002,        /**< [GB.GG.D2.TM] egress vlan mapping use svid as key*/
    CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS   = 0x00000004,   /**< [GB.GG.D2.TM] egress vlan mapping use ctag cos as key*/
    CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS    = 0x00000008    /**< [GB.GG.D2.TM] egress vlan mapping use stag cos as key*/
};
typedef enum ctc_egress_vlan_mapping_key_e ctc_egress_vlan_mapping_key_t;

/**
 @brief define egress vlan mapping output used on epe vlan mapping
*/
enum ctc_egress_vlan_mapping_action_e
{
    CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID     = 0x00000001,       /**< [GB.GG.D2.TM] output svid in mapping result*/
    CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID     = 0x00000002,       /**< [GB.GG.D2.TM] output cvid in mapping result*/
    CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCOS     = 0x00000004,       /**< [GB.GG.D2.TM] output stag cos in mapping result*/
    CTC_EGRESS_VLAN_MAPPING_OUTPUT_CCOS     = 0x00000008,       /**< [GB.GG.D2.TM] output ctag cos in mapping result*/
    CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCFI     = 0x00000010,        /**< [GG.D2.TM] output stag cfi in mapping result*/

};
typedef enum ctc_egress_vlan_mapping_action_e ctc_egress_vlan_mapping_action_t;

/**
 @brief define data structure for egress vlan mapping used in APIs
*/
struct ctc_egress_vlan_mapping_s
{
    /*key*/
    uint32    key;                               /**< [GB.GG.D2.TM] egress vlan mapping key type,GB support 6 kinds of key or key combination type:
                                                             CTC_EGRESS_VLAN_MAPPING_KEY_NONE /
                                                             CTC_EGRESS_VLAN_MAPPING_KEY_CVID /
                                                             CTC_EGRESS_VLAN_MAPPING_KEY_SVID /
                                                             CTC_EGRESS_VLAN_MAPPING_KEY_CVID + CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS /
                                                             CTC_EGRESS_VLAN_MAPPING_KEY_SVID + CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS /
                                                             CTC_EGRESS_VLAN_MAPPING_KEY_SVID + CTC_EGRESS_VLAN_MAPPING_KEY_CVID
                                                             you can see num ctc_egress_vlan_mapping_key_t remark*/
    uint16   old_cvid;                         /**< [GB.GG.D2.TM] orignal cvlan id in packet*/
    uint16   old_svid;                         /**< [GB.GG.D2.TM] orignal svlan id in packet*/
    uint8    old_ccos;                         /**< [GB.GG.D2.TM] orignal ccos id in packet*/
    uint8    old_scos;                         /**< [GB.GG.D2.TM] orignal scos id in packet*/
    uint16   cvlan_end;                      /**< [GB.GG.D2.TM] if vlan range check want to be used, cvlan_end must be configured
                                                             as the max cvlan id need to do range check*/
    uint16   svlan_end;                      /**< [GB.GG.D2.TM] if vlan range check want to be used, svlan_end must be configured
                                                             as the max svlan id need to do range check*/
    uint8  vrange_grpid;                    /**< [GB.GG.D2.TM] if vlan range check want to be used, vlan range group id: 0~63
                                                             the vlan range group property should be as same as the property vlan range
                                                             check on port*/
    uint8  flag;                                  /**< [GB.GG.D2.TM] see ctc_vlan_mapping_flag_t */

    /*action*/
    uint32   action;                           /**< [GB.GG.D2.TM] action of egress vlan mapping, ctc_egress_vlan_mapping_action_t*/

    ctc_vlan_tag_op_t         stag_op;         /**< [GB.GG.D2.TM] operation type of stag, see ctc_vlan_tag_op_t struct*/
    ctc_vlan_tag_sl_t          svid_sl;           /**< [GB.GG.D2.TM] operation type of svlan id, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t          scos_sl;          /**< [GB.GG.D2.TM] operation type of scos, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t          scfi_sl;           /**< [GG.D2.TM] operation type of scfi, see ctc_vlan_tag_sl_t struct*/

    ctc_vlan_tag_op_t        ctag_op;         /**< [GB.GG.D2.TM] operation type of ctag, see ctc_vlan_tag_op_t struct*/
    ctc_vlan_tag_sl_t          cvid_sl;          /**< [GB.GG.D2.TM] operation type of cvlan id, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t          ccos_sl;         /**< [GB.GG.D2.TM] operation type of ccos, see ctc_vlan_tag_sl_t struct*/

    uint16 new_svid;                              /**< [GB.GG.D2.TM] new svlan id in packet through egress vlan mapping*/
    uint16 new_cvid;                              /**< [GB.GG.D2.TM] new cvlan id in packet through egress vlan mapping*/
    uint8  new_scos;                              /**< [GB.GG.D2.TM] new scos in packet through egress vlan mapping*/
    uint8  new_ccos;                              /**< [GB.GG.D2.TM] new ccos in packet through egress vlan mapping*/
    uint8  new_scfi;                               /**< [GG.D2.TM] new scfi in packet through egress vlan mapping*/

    uint8  color;                                    /**< [GB] Color: green, yellow, or red :CTC_QOS_COLOR_XXX*/
    uint8  priority;                                 /**< [GB] Priority*/
    uint8  tpid_index;                           /**< [GB.GG.D2.TM] svlan tpid index*/
    uint8  rsv0[2];
};
typedef struct ctc_egress_vlan_mapping_s ctc_egress_vlan_mapping_t;

/**
 @brief define key type for ingress vlan mapping
*/

enum ctc_vlan_mapping_key_e
{
    CTC_VLAN_MAPPING_KEY_NONE               = 0x00000000,               /**< [GB.GG.D2.TM] vlan mapping use port as key*/
    CTC_VLAN_MAPPING_KEY_CVID               = 0x00000001,                /**< [GB.GG.D2.TM] use cvid as key*/
    CTC_VLAN_MAPPING_KEY_SVID               = 0x00000002,                /**< [GB.GG.D2.TM] use svid as key*/
    CTC_VLAN_MAPPING_KEY_CTAG_COS           = 0x00000004,           /**< [GB.GG.D2.TM] use ctag cos as key*/
    CTC_VLAN_MAPPING_KEY_STAG_COS           = 0x00000008,           /**< [GB.GG.D2.TM] use stag cos as key*/
    CTC_VLAN_MAPPING_KEY_MAC_SA             = 0x00000010,            /**< [GG.D2.TM] use mac sa as key*/
    CTC_VLAN_MAPPING_KEY_IPV4_SA            = 0x00000020,            /**< [GG.D2.TM] use ipv4 sa as key*/
    CTC_VLAN_MAPPING_KEY_IPV6_SA            = 0x00000040,            /**< [GG.D2.TM] use ipv6 sa as key*/


    /*below are used for flexible key*/

};
typedef enum ctc_vlan_mapping_key_e ctc_vlan_mapping_key_t;

/**
 @brief vlan mapping output used on ipe_usrid
*/
enum ctc_vlan_mapping_action_e
{
    CTC_VLAN_MAPPING_OUTPUT_SVID                = 0x00000001,                 /**< [GB.GG.D2.TM] output svid in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT      = 0x00000002,        /**< [GB.GG.D2.TM] output logic source port in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_APS_SELECT          = 0x00000004,            /**< [GB.GG.D2.TM] output aps select information in mapping result*/

    CTC_VLAN_MAPPING_OUTPUT_FID                 = 0x00000008,                  /**< [GB.GG.D2.TM] output fid in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_NHID                = 0x00000010,                 /**< [GB.GG.D2.TM] output nexthop in mapping result*/

    CTC_VLAN_MAPPING_OUTPUT_SERVICE_ID          = 0x00000020,             /**< [GB.D2.TM] output service ID of HQoS in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_CVID                = 0x00000040,                 /**< [GB.GG.D2.TM] output cvid in mapping result*/

    CTC_VLAN_MAPPING_OUTPUT_VLANPTR             = 0x00000080,               /**< [GB.GG.D2.TM] output VlanPtr in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_SCOS                = 0x00000100,                 /**< [GB.GG.D2.TM] output stag cos in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_CCOS                = 0x00000200,                 /**< [GB.GG.D2.TM] output ctag cos in mapping result*/
    CTC_VLAN_MAPPING_OUTPUT_VN_ID               = 0x00000400,                 /**< [GG.D2.TM] output virtual subnet id used only for overlay network tunnel initiator*/

    CTC_VLAN_MAPPING_FLAG_VPWS                  = 0x00001000,                   /**< [GB.GG.D2.TM] flag to indicate vpws application*/
    CTC_VLAN_MAPPING_FLAG_VPLS                  = 0x00002000,                    /**< [GB.GG.D2.TM] flag to indicate vpls application*/
    CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS          = 0x00004000,              /**< [GB.D2.TM] flag to indicate vpls learning will be disable*/
    CTC_VLAN_MAPPING_FLAG_MACLIMIT_EN           = 0x00008000,              /**< [GB.GG] flag to indicate vpls learning will be disable*/

    CTC_VLAN_MAPPING_FLAG_ETREE_LEAF            = 0x00010000,               /**< [GB.GG.D2.TM] flag to indicate it is leaf node in E-Tree networks */


    CTC_VLAN_MAPPING_FLAG_SERVICE_ACL_EN        = 0x00100000,            /**< [GB.GG.D2.TM] enable service acl of HQoS in mapping result*/
    CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN    = 0x00200000,         /**< [GB.GG.D2.TM] enable service policer of HQoS in mapping result*/
    CTC_VLAN_MAPPING_FLAG_STATS_EN              = 0x00400000,                 /**< [GB.GG.D2.TM] enable stats in mapping result*/
    CTC_VLAN_MAPPING_FLAG_SERVICE_QUEUE_EN      = 0x00800000,          /**< [GB.D2.TM] enable service queue */
    CTC_VLAN_MAPPING_OUTPUT_STP_ID              = 0x01000000,                  /**< [GG.D2.TM] output stp id in mapping result */

    CTC_VLAN_MAPPING_FLAG_L2VPN_OAM             = 0x02000000,               /**< [GG.D2.TM] indecate L2VPN OAM, need config l2vpn_oam_id*/
    CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN         = 0x04000000,            /**< [GG.D2.TM] output vlan domain in mapping result */

    CTC_VLAN_MAPPING_OUTPUT_VRFID               = 0x08000000,                   /**< [GB.GG.D2.TM] output vrfid in mapping result */
    CTC_VLAN_MAPPING_FLAG_IGMP_SNOOPING_EN      = 0x10000000,           /**< [GB] enable IGMP snooping in mapping result */
    CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN  = 0x20000000            /**< [D2.TM] enable mac limit discard */
};
typedef enum ctc_vlan_mapping_action_e ctc_vlan_mapping_action_t;

/**
 @brief enum value used in vlan mapping flag APIs
*/
enum ctc_vlan_mapping_flag_e
{
    CTC_VLAN_MAPPING_FLAG_USE_FLEX = 0x01,                                       /**< [GB.GG.D2.TM] if set, vlan mapping key use tcam, only support ingress vlan mapping*/
    CTC_VLAN_MAPPING_FLAG_RESOLVE_CONFLICT = CTC_VLAN_MAPPING_FLAG_USE_FLEX,     /**< [D2.TM] if set, vlan mapping entry is used for resolving conflict */
    CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT = 0x02,                                 /**< [GB.GG.D2.TM] if set, vlan mapping key use logic port as port*/
    CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY = 0x04,                                  /**< [GB.GG.D2.TM] if set, indicate vlan mapping default entry */
    CTC_VLAN_MAPPING_FLAG_MAX
};
typedef enum ctc_vlan_mapping_flag_e ctc_vlan_mapping_flag_t;

/**
 @brief define data structure used in APIs
*/
struct ctc_vlan_mapping_s
{

    /*key*/
    uint32 key;                                /**< [GB.GG.D2.TM] vlan mapping key type,GB support any kinds of key or key combination type,
                                                           see enum ctc_vlan_mapping_key_t remark*/
    uint16 old_cvid;                        /**< [GB.GG.D2.TM] orignal start cvlan id in packet*/
    uint16 old_svid;                        /**< [GB.GG.D2.TM] orignal start svlan id in packet*/
    uint16 cvlan_end;                     /**< [GB.GG.D2.TM] orignal max cvlan id need to do vlan mapping*/
    uint16 svlan_end;                     /**< [GB.GG.D2.TM] orignal max svlan id need to do vlan mapping*/
    uint8  old_ccos;                        /**< [GB.GG.D2.TM] orignal ccos id in packet*/
    uint8  old_scos;                        /**< [GB.GG.D2.TM] orignal scos id in packet*/
    uint8  vrange_grpid;                 /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint8 flag;                                /**< [GB.GG.D2.TM] Supported vlan mapping bitmap, see ctc_vlan_mapping_flag_t */

    mac_addr_t macsa;                   /**< [GG.D2.TM] macsa as key*/
    ip_addr_t ipv4_sa;                    /**< [GG.D2.TM] ipv4 sa as key*/
    ipv6_addr_t ipv6_sa;                 /**< [GG.D2.TM] ipv6 sa as key*/


    /*action*/
    uint16 user_vlanptr;                    /**< [GB.GG.D2.TM] to use this feature. set ctc_vlan_global_cfg_s.vlanptr_mode = CTC_VLANPTR_MODE_USER_DEFINE1 / USER_DEFINE2*/
    uint32 action;                              /**< [GB.GG.D2.TM] action type of vlan mapping, see ctc_vlan_mapping_action_t remark*/

    /*vlan edit*/
    ctc_vlan_tag_op_t      stag_op;         /**< [GB.GG.D2.TM] operation type of stag, see ctc_vlan_tag_op_t struct*/
    ctc_vlan_tag_sl_t      svid_sl;            /**< [GB.GG.D2.TM] operation type of svlan id, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t      scos_sl;           /**< [GB.GG.D2.TM] operation type of scos, see ctc_vlan_tag_sl_t struct*/

    ctc_vlan_tag_op_t      ctag_op;         /**< [GB.GG.D2.TM] operation type of ctag, see ctc_vlan_tag_op_t struct*/
    ctc_vlan_tag_sl_t      cvid_sl;            /**< [GB.GG.D2.TM] operation type of cvlan id, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t      ccos_sl;           /**< [GB.GG.D2.TM] operation type of ccos, see ctc_vlan_tag_sl_t struct*/

    uint16 new_svid;                        /**< [GB.GG.D2.TM] new svid append on packet*/
    uint16 new_cvid;                        /**< [GB.GG.D2.TM] new cvid append on packet*/
    uint8 new_scos;                         /**< [GB.GG.D2.TM] new stag cos*/
    uint8 new_ccos;                         /**< [GB.GG.D2.TM] new ctag cos*/

    uint16 logic_src_port;                  /**< [GB.GG.D2.TM] logic source port*/
    uint16 aps_select_group_id;        /**< [GB.GG.D2.TM] aps selector group id*/
    uint16 service_id;                       /**< [GB.D2.TM] service id for do service acl/queueu/policer */
    uint32 stats_id;                          /**< [GB.GG.D2.TM] stats id*/

    union
    {
        uint16 fid;                            /**< [GB.GG.D2.TM] forwarding instance, maybe EVC concept on MEF10*/
        uint32 nh_id;                       /**< [GB.GG.D2.TM] nexthop id, need create nexthop first*/
        uint32 vn_id;                       /**< [GG.D2.TM] virtual subnet id , used only for overlay network tunnel initiator*/
        uint16 vrf_id;                       /**< [GB.GG.D2.TM] Vrf ID of the route*/
    } u3;

    uint16 l2vpn_oam_id;               /**< [GG.D2.TM] vpws oam id or vpls fid, used in Port + Oam ID mode, refer to CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE*/

    uint8 is_working_path;             /**< [GB.GG.D2.TM] indicate the flow is working path or protecting path, used in APS*/
    uint16 protected_vlan;             /**< [GB.GG.D2.TM] protected vlan, used in APS, general equal to vlan of working path*/
    uint8 logic_port_type;              /**< [GB.GG.D2.TM] logic-port-type, used for VPLS horizon split discard */
    uint8 stp_id;                            /**< [GG.D2.TM] stp id: 1 - 127*/
    uint8 scl_id;                            /**< [GG.D2.TM] TCAM  SCL lookup ID.There are 2 scl lookup<0-1>, and each has its own feature */
    uint8 vlan_domain;                  /**< [GG.D2.TM] new vlan domain, <0-2>, see ctc_scl_vlan_domain_t */
    uint16 policer_id;                     /**< [GB.GG.D2.TM] policer id for do service/flow policer */

    uint8  color;                           /**< [GB.GG.D2.TM] Color: green, yellow, or red :CTC_QOS_COLOR_XXX*/
    uint8  priority;                        /**< [GB.GG.D2.TM] Priority*/
    uint8  tpid_index;                      /**< [GB.GG.D2.TM] svlan tpid index*/

};
typedef struct ctc_vlan_mapping_s ctc_vlan_mapping_t;

/**
 @brief define action of default entry of vlan mapping and vlan class
*/
enum ctc_vlan_miss_flag_e
{
    CTC_VLAN_MISS_ACTION_DO_NOTHING,           /**< [GB.GG.D2.TM] default action do nothing, will be forwarding normal*/
    CTC_VLAN_MISS_ACTION_DISCARD,                 /**< [GB.GG.D2.TM] default action discard, will be discard*/
    CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR,    /**< [GB.GG.D2.TM] default action output vlanptr*/

    /*below are for ingress only*/
    CTC_VLAN_MISS_ACTION_TO_CPU,                  /**< [GB.GG.D2.TM] default action to cpu, will be redirect to cpu only support by ingress vlan mapping*/
    CTC_VLAN_MISS_ACTION_APPEND_STAG         /**< [GB.GG.D2.TM] default action add a stag with new_svid even if packet tagged*/
};
typedef enum ctc_vlan_miss_flag_e ctc_vlan_miss_flag_t;

/**
 @brief define action of default entry of vlan mapping and vlan class
*/
struct ctc_vlan_miss_s
{
    uint8  flag;                        /**< [GB.GG.D2.TM] if vlan mapping mismatch any entry, do action as this, see enum ctc_vlan_miss_flag_t*/
    uint16 user_vlanptr;          /**< [GG.D2.TM] to use this feature. set ctc_vlan_global_cfg_s.vlanptr_mode = CTC_VLANPTR_MODE_USER_DEFINE1 / USER_DEFINE2*/
    uint16 new_svid;               /**< [GB.GG.D2.TM] default action add a stag with this new_svid even if packet tagged only for ingress vlan mapping*/
    uint8 new_scos;                /**< [GB.GG.D2.TM] new stag cos*/
    uint8 scl_id;                  /**< [TM] used for 2 userid hash */
    uint8  rsv0[2];
    ctc_vlan_tag_sl_t      scos_sl;         /**< [GB.GG.D2.TM] operation type of scos, see ctc_vlan_tag_sl_t struct*/
};
typedef struct ctc_vlan_miss_s ctc_vlan_miss_t;

/**@} end of @defgroup vlanmapping */

/**
 @defgroup vlanclassification vlanclassification
 @{
*/

/**
 @brief enum value used in vlan classification APIs
*/
enum ctc_vlan_class_type_e
{
    CTC_VLAN_CLASS_MAC,                 /**< [GB.GG.D2.TM] mac based vlan*/
    CTC_VLAN_CLASS_IPV4,                /**< [GB.GG.D2.TM] ipv4/flow based vlan*/
    CTC_VLAN_CLASS_IPV6,                /**< [GB.GG.D2.TM] ipv6/flow based vlan*/
    CTC_VLAN_CLASS_PROTOCOL,       /**< [GB.GG.D2.TM] protocol based vlan*/
    CTC_VLAN_CLASS_MAX
};
typedef enum ctc_vlan_class_type_e ctc_vlan_class_type_t;

/**
 @brief enum value used in vlan classification flag APIs
*/
enum ctc_vlan_class_flag_e
{
    CTC_VLAN_CLASS_FLAG_USE_MACDA = 0x01,          /**< [GB.GG.D2.TM] if set, macda will be valid to use while mac based vlan class*/
    CTC_VLAN_CLASS_FLAG_OUTPUT_COS = 0x02,         /**< [GB.GG.D2.TM] if set, cos will be valid to output while vlan class*/
    CTC_VLAN_CLASS_FLAG_USE_FLEX = 0x04,           /**< [GB.GG.D2.TM] if set, flex mac ipv4 or ipv6 tcam key will be used*/
    CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT = 0x08,   /**< [D2.TM] if set, means resolve conflict */
    CTC_VLAN_CLASS_FLAG_MAX
};
typedef enum ctc_vlan_class_flag_e ctc_vlan_class_flag_t;

/**
 @brief the structure of vlan classification
*/
struct ctc_vlan_class_s
{
    ctc_vlan_class_type_t type;                 /**< [GB.GG.D2.TM] vlan classification type, based mac/ip/protocol/flow*/
    uint8 flag;                                         /**< [GB.GG.D2.TM] Supported vlan class bitmap, see ctc_vlan_class_flag_t */
    uint8  cos;                                        /**< [GB.GG.D2.TM] new cos*/
    uint16 rsv0;

    uint8  scl_id;                                     /**< [GG.D2.TM] TCAM  SCL lookup ID.There are 2 scl lookup<0-1>, and each has its own feature */
    uint8  rsv1;
    uint16 vlan_id;                                  /**< [GB.GG.D2.TM] new vlan id*/
    union
    {
        mac_addr_t mac;                         /**< [GB.GG.D2.TM] only mac address based vlan class*/
        ip_addr_t ipv4_sa;                       /**< [GB.GG.D2.TM] only IPv4 source address based vlan class*/
        ipv6_addr_t ipv6_sa;                    /**< [GB.GG.D2.TM] only IPv6 source address based vlan class*/
        uint16 ether_type;                      /**< [GG.D2.TM] ether type */

        struct
        {
            ip_addr_t ipv4_sa;                      /**< [GB.GG.D2.TM] IPv4 source address*/
            ip_addr_t ipv4_sa_mask;             /**< [GB.GG.D2.TM] IPv4 source mask*/
            ip_addr_t ipv4_da;                      /**< [GB.GG.D2.TM] IPv4 destination address*/
            ip_addr_t ipv4_da_mask;            /**< [GB.GG.D2.TM] IPv4 destination mask*/
            mac_addr_t macsa;                    /**< [GB] mac source address*/
            mac_addr_t macda;                    /**< [GB] mac destination address*/
            uint16 l4src_port;                       /**< [GB.GG.D2.TM] layer4 source port*/
            uint16 l4dest_port;                     /**< [GB.GG.D2.TM] layer4 destination port*/
            ctc_parser_l3_type_t l3_type;       /**< [GB.GG.D2.TM] layer3 parser type*/
            ctc_parser_l4_type_t l4_type;       /**< [GB.GG.D2.TM] layer4 parser type*/
        } flex_ipv4;

        struct
        {
            ipv6_addr_t ipv6_sa;                     /**< [GB.GG.D2.TM] IPv6 source address*/
            ipv6_addr_t ipv6_sa_mask;           /**< [GB.GG.D2.TM] IPv6 source mask*/
            ipv6_addr_t ipv6_da;                    /**< [GB.GG.D2.TM] IPv6 destination address*/
            ipv6_addr_t ipv6_da_mask;          /**< [GB.GG.D2.TM] IPv6 destination mask*/
            mac_addr_t macsa;                      /**< [GB] mac source address*/
            mac_addr_t macda;                      /**< [GB] mac destination address*/
            uint16 l4src_port;                        /**< [GB.GG.D2.TM] layer4 source port*/
            uint16 l4dest_port;                      /**< [GB.GG.D2.TM] layer4 destination type*/
            ctc_parser_l4_type_t l4_type;       /**< [GB.GG.D2.TM] layer4 parser type*/
        } flex_ipv6;

    } vlan_class;                /**<vlan classification key information*/

};
typedef struct ctc_vlan_class_s ctc_vlan_class_t;
/**@} end of @defgroup VlanClassification*/

/**
  @brief Define vlan property flags
*/
enum ctc_vlan_property_e
{
    CTC_VLAN_PROP_RECEIVE_EN,                               /**< [GB.GG.D2.TM] vlan receive enable*/
    CTC_VLAN_PROP_BRIDGE_EN,                                 /**< [GB.GG.D2.TM] vlan bridge enable*/
    CTC_VLAN_PROP_TRANSMIT_EN,                             /**< [GB.GG.D2.TM] vlan transmit enable*/
    CTC_VLAN_PROP_LEARNING_EN,                             /**< [GB.GG.D2.TM] vlan learning enable*/
    CTC_VLAN_PROP_DHCP_EXCP_TYPE,                      /**< [GB.GG.D2.TM] dhcp packet processing type, value see enum ctc_exception_type_t */
    CTC_VLAN_PROP_ARP_EXCP_TYPE,                        /**< [GB.GG.D2.TM] arp packet processing type, value see enum ctc_exception_type_t */
    CTC_VLAN_PROP_IGMP_SNOOP_EN,                       /**< [GB.GG.D2.TM] igmp snooping enable     */
    CTC_VLAN_PROP_FID,                                           /**< [GB.GG.D2.TM] fid for general l2 bridge, VSIID for L2VPN*/
    CTC_VLAN_PROP_IPV4_BASED_L2MC,                    /**< [GB.GG.D2.TM] Ipv4 based l2mc*/
    CTC_VLAN_PROP_IPV6_BASED_L2MC,                    /**< [GB.GG.D2.TM] Ipv6 based l2mc*/
    CTC_VLAN_PROP_PTP_EN,                                     /**< [GB.GG.D2.TM] set ptp clock id, refer to ctc_ptp_clock_t */
    CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN,      /**< [GB.GG.D2.TM] vlan drop unknown ucast packet enable*/
    CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN,      /**< [GB.GG.D2.TM] vlan drop unknown mcast packet enable*/
    CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN,      /**< [GG.D2.TM] vlan drop unknown bcast packet enable*/
    CTC_VLAN_PROP_UNKNOWN_MCAST_COPY_TO_CPU,     /**< [GB.GG.D2.TM] unknown mcast packet copy to cpu enable*/
    CTC_VLAN_PROP_FIP_EXCP_TYPE,                        /**< [GG.D2.TM] fip packet processing type, value see enum ctc_exception_type_t */
    CTC_VLAN_PROP_CID,                                          /**< [D2.TM] category id*/
    CTC_VLAN_PROP_PIM_SNOOP_EN,                                 /**< [D2.TM] pim snoop enable */
    MAX_CTC_VLAN_PROP_NUM
};
typedef enum ctc_vlan_property_e  ctc_vlan_property_t;


/**
  @brief Define vlan property flags with direction
*/
enum ctc_vlan_direction_property_e
{
    CTC_VLAN_DIR_PROP_ACL_EN,                                         /**< [GB.GG.D2.TM] vlan acl enable. bitmap refer CTC_ACL_EN_XXX*/
    CTC_VLAN_DIR_PROP_ACL_CLASSID,                                 /**< [GB.GG.D2.TM] vlan acl classid 0*/
    CTC_VLAN_DIR_PROP_ACL_CLASSID_1,                             /**< [GG.D2.TM] vlan acl classid 1, only for ingress*/
    CTC_VLAN_DIR_PROP_ACL_CLASSID_2,                             /**< [GG.D2.TM] vlan acl classid 2, only for ingress*/
    CTC_VLAN_DIR_PROP_ACL_CLASSID_3,                             /**< [GG.D2.TM] vlan acl classid 3, only for ingress*/
    CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY,                 /**< [GB.GG] vlan routed packet only for acl 0*/
    CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0,               /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1,               /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2,               /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3,               /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_VLAN_DIR_PROP_VLAN_STATS_ID,                          /**< [D2.TM] vlan stats id */

    CTC_VLAN_DIR_PROP_NUM
};
typedef enum ctc_vlan_direction_property_e  ctc_vlan_direction_property_t;

/**
 @brief Define vlan range group property
*/
struct ctc_vlan_range_info_s
{
    uint8 vrange_grpid;              /**< [GB.GG.D2.TM] vlan range check group id*/
    ctc_direction_t direction;       /**< [GB.GG.D2.TM] vlan range check group direction*/
};
typedef struct ctc_vlan_range_info_s  ctc_vlan_range_info_t;

/**
 @brief Define vlan range items, including start and end vlan id
*/
struct ctc_vlan_range_s
{
    uint16 vlan_start;               /**< [GB.GG.D2.TM] vlan range item start vlan id*/
    uint16 vlan_end;                /**< [GB.GG.D2.TM] vlan range item end vlan id*/
    uint8 is_acl;                       /**< [GG.D2.TM] Used in CTC_GLOBAL_VLAN_RANGE_MODE_SHARE, if set, vlan range used for acl, else for scl */
};
typedef struct ctc_vlan_range_s  ctc_vlan_range_t;

/**
 @brief Define get vlan range group info
*/
struct ctc_vlan_range_group_s
{
    ctc_vlan_range_t vlan_range[8]; /**< [GB.GG.D2.TM] 8 items info in one group*/
};
typedef struct ctc_vlan_range_group_s  ctc_vlan_range_group_t;


/**
 @brief define the mode of vlanPtr
*/
enum ctc_vlanptr_mode_e
{
    CTC_VLANPTR_MODE_USER_DEFINE1,      /**< [GG.D2.TM] set or get vlan property by user defined vlanPtr.
                                                                         (vlanptr 0~4K-1, set or get vlan property by uservlanptr, create vlan with ctc_vlan_create_uservlan )*/
};
typedef enum ctc_vlanptr_mode_e  ctc_vlanptr_mode_t;


/**
 @brief Create user vlan param
*/
struct ctc_vlan_uservlan_s
{
    uint16 vlan_id;                          /**<[GB.GG.D2.TM] Vlan id (only support vlan_id = user_vlanptr for GB)*/
    uint16 user_vlanptr;                  /**<[GB.GG.D2.TM] User vlan ptr */
    uint16 fid;                                 /**<[GB.GG.D2.TM] Fid , l2 flooding group id*/
    uint32 flag;                               /**<[GB.GG.D2.TM] Flag, use CTC_MAC_LEARN_USE_XXX / */
};
typedef struct ctc_vlan_uservlan_s ctc_vlan_uservlan_t;

/**
 @brief vlan global config information
*/
struct ctc_vlan_global_cfg_s
{
    uint8    vlanptr_mode;                /**< [GG.D2.TM] refer to enum ctc_vlanptr_mode_t.*/
    uint8    rsv0[3];
};
typedef struct ctc_vlan_global_cfg_s ctc_vlan_global_cfg_t;

/**@} end of @defgroup vlan*/
#ifdef __cplusplus
}
#endif

#endif

