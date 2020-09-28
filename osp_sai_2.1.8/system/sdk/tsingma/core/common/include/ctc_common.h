/**
 @file ctc_common.h

 @date 2010-6-10

 @version v2.0

The file define all CTC SDK module's common function and APIs.
*/

#ifndef _CTC_COMMON_H
#define _CTC_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif


/**
  @brief acl property flag
*/
enum ctc_acl_prop_flag_e
{
    CTC_ACL_PROP_FLAG_USE_HASH_LKUP      = 1<<0,     /**< [D2.TM] use hash lookup */
    /* PORT_BITMAP, METADATA and LOGIC_PORT are conflict, only one can be set */
    CTC_ACL_PROP_FLAG_USE_PORT_BITMAP    = 1<<1,     /**< [D2.TM] acl use port_bitmap to lookup */
    CTC_ACL_PROP_FLAG_USE_METADATA       = 1<<2,     /**< [D2.TM] acl use metadata to lookup*/
    CTC_ACL_PROP_FLAG_USE_LOGIC_PORT     = 1<<3,     /**< [D2.TM] acl use logic port to lookup*/
    CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN    = 1<<4,     /**< [D2.TM] acl use mapped vlan to lookup, or use vlan from pkt*/
    CTC_ACL_PROP_FLAG_USE_WLAN         = 1<<5,     /**< [D2.TM] acl use wlan info to lookup*/
    CTC_ACL_PROP_FLAG_NUM
};
typedef enum ctc_acl_prop_flag_e ctc_acl_prop_flag_t;

/**
  @brief acl property parameter
*/
struct ctc_acl_property_s
{
    uint32 flag;                   /**< [D2.TM] refer to ctc_acl_prop_flag_t */
    uint8  acl_en;                 /**< [D2.TM] enable or disable acl */
    /*Tcam lookup*/
    uint8  acl_priority;            /**< [D2.TM] acl priority */
    uint8  direction;               /**< [D2.TM] direction, CTC_INGRESS or CTC_EGRESS*/
    uint8  tcam_lkup_type;          /**< [D2.TM] tcam lookup type, refer to CTC_ACL_TCAM_LKUP_TYPE_XXX */
    uint8  class_id;                /**< [D2.TM] class id */
    /*Hash lookup*/
    uint8  hash_lkup_type;          /**< [D2.TM] acl hash loopkup type*/
    uint8  hash_field_sel_id;       /**< [D2.TM] hash field select id*/
    uint8  rsv;
};
typedef struct ctc_acl_property_s ctc_acl_property_t;

/**
 @brief  Define dscp select mode
*/
enum ctc_dscp_select_mode_e
{
    CTC_DSCP_SELECT_NONE,            /**< [D2.TM] Do not change DSCP */
    CTC_DSCP_SELECT_ASSIGN,          /**< [D2.TM] Use user-define DSCP */
    CTC_DSCP_SELECT_MAP,             /**< [D2.TM] Use DSCP value from DSCP map */
    CTC_DSCP_SELECT_PACKET,          /**< [D2.TM] Copy packet DSCP */
    MAX_CTC_DSCP_SELECT_MODE
};
typedef enum ctc_dscp_select_mode_e ctc_dscp_select_mode_t;


/**
 @brief define Arp & Dhcp packet action
*/
enum ctc_exception_type_e
{
    CTC_EXCP_FWD_AND_TO_CPU,                                 /**< [GB.GG.D2.TM] forwarding and send to cpu */
    CTC_EXCP_NORMAL_FWD,                                        /**< [GB.GG.D2.TM] normal forwarding */
    CTC_EXCP_DISCARD_AND_TO_CPU,                           /**< [GB.GG.D2.TM] discard and send to cpu */
    CTC_EXCP_DISCARD,                                                 /**< [GB.GG.D2.TM] discard */
    MAX_CTC_EXCP_TYPE
};
typedef enum ctc_exception_type_e ctc_exception_type_t;


/**
@brief  define packet forward type
*/
enum ctc_acl_pkt_fwd_type_e
{
    CTC_ACL_PKT_FWD_TYPE_NONE = 0,                /**< [D2.TM] Unvalid Packet Forward Type. */
    CTC_ACL_PKT_FWD_TYPE_IPUC,                    /**< [D2.TM] Packet Forward Type of IPUC. */
    CTC_ACL_PKT_FWD_TYPE_IPMC,                    /**< [D2.TM] Packet Forward Type of IPMC. */
    CTC_ACL_PKT_FWD_TYPE_L2UC,                    /**< [D2.TM] Packet Forward Type of L2UC. */
    CTC_ACL_PKT_FWD_TYPE_L2MC,                    /**< [D2.TM] Packet Forward Type of L2MC. */
    CTC_ACL_PKT_FWD_TYPE_BC,                      /**< [D2.TM] Packet Forward Type of BC. */
    CTC_ACL_PKT_FWD_TYPE_MPLS,                    /**< [D2.TM] Packet Forward Type of MPLS. */
    CTC_ACL_PKT_FWD_TYPE_VPLS,                    /**< [D2.TM] Packet Forward Type of VPLS. */
    CTC_ACL_PKT_FWD_TYPE_L3VPN,                   /**< [D2.TM] Packet Forward Type of L3VPN. */
    CTC_ACL_PKT_FWD_TYPE_VPWS,                    /**< [D2.TM] Packet Forward Type of VPWS. */
    CTC_ACL_PKT_FWD_TYPE_TRILL_UC,                /**< [D2.TM] Packet Forward Type of TRILL UC. */
    CTC_ACL_PKT_FWD_TYPE_TRILL_MC,                /**< [D2.TM] Packet Forward Type of TRILL MC. */
    CTC_ACL_PKT_FWD_TYPE_FCOE,                    /**< [D2.TM] Packet Forward Type of FCOE. */
    CTC_ACL_PKT_FWD_TYPE_MPLS_OTHER_VPN,          /**< [D2.TM] Packet Forward Type of MPLS Other VPN. */
    CTC_ACL_PKT_FWD_TYPE_MAX
};
typedef enum ctc_acl_pkt_fwd_type_e ctc_acl_pkt_fwd_type_t;

struct ctc_property_array_s
{
   uint16  property;
   uint32  data;
   void*   ext_data;

};
typedef struct ctc_property_array_s  ctc_property_array_t;

#ifdef __cplusplus
}
#endif

#endif /* _CTC_COMMON_H*/

