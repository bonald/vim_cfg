/**
 @file ctc_const.h

 @date 2009-10-27

 @version v2.0

The file define  all CTC SDK module's  common Macros and constant.
*/

#ifndef _CTC_CONST_H
#define _CTC_CONST_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common/../../sal/include/sal_types.h"
#include "common/include/ctc_mix.h"
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_macro.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup const    CONST
 @{
*/

/***************************Gport construct***********************************
 *   |-----Type----|----Resv----|----lport ext----|----gchip----|----lport----|*
 *   |    31-28    |    27-18   |      17-15      |     14-08   |    07~00    |*
 *   |-------------|------------|-----------------|-------------|-------------|*
 ******************************************************************************/

#define CTC_ETH_ADDR_LEN                6           /**< Ethernet mac address length (unit: byte) */
#define CTC_IPV6_ADDR_LEN               4           /**< IPv6 address length (unit: byte) */

#define CTC_GCHIP_LENGTH                7           /**< Gchip id length(unit: bit) */
#define CTC_GCHIP_MASK                  0x7F        /**< Gchip id mask(unit: bit) */

#define CTC_LOCAL_PORT_LENGTH           8           /**< Local port id length(unit: bit) */
#define CTC_LOCAL_PORT_MASK             0xFF        /**< Local port mask */

#define CTC_EXT_PORT_LENGTH             3           /**< Local port id ext length(unit: bit) */
#define CTC_EXT_PORT_MASK               7           /**< Local port ext bit mask */

#define CTC_RSV_PORT_LENGTH             10           /**< Reserve port length(unit: bit) */
#define CTC_RSV_PORT_MASK               0x3FF          /**< Reserve port bit mask */

#define CTC_TYPE_PORT_LENGTH             4           /**< Port type length(unit: bit) */
#define CTC_TYPE_PORT_MASK               0xF          /**< Port type bit mask */

#define CTC_MAC_NUM_PER_MACRO           8           /**< Mac number per macro */
#define CTC_LINKAGG_CHIPID              0x1F        /**< Linkagg port's global chip ID */
#define CTC_INVALID_CHIPID              0xFF        /**< Invalid chip id */
#define CTC_SDK_VERSION_STR "V5.5.2RC"
#define CTC_SDK_RELEASE_DATE "2019-06-26"
#define CTC_SDK_COPYRIGHT_TIME "2019"

/* no need any more in GB linkagg */
#define CTC_MAX_LINKAGG_MEMBER_PORT     16          /**< static load balance group_num:128,mem_num per group:8 */
#define CTC_MAX_VLAN_ID                 4095        /**< Max vlan id */
#define CTC_MIN_VLAN_ID                 1           /**< Min vlan id */
#define CTC_MAX_COS                     8           /**< Max cos*/
#define CTC_MIN_COS                     0           /**< Min cos*/
#define CTC_MAX_FID_ID                  16383       /**< Max fid id */
#define CTC_MIN_FID_ID                  0           /**< Min fid id */
#define CTC_MAX_LOGIC_PORT_ID           8191        /**< Max logic port id */
#define CTC_MIN_VRFID_ID                0           /**< Min vrfid id */
#define CTC_MAX_MTU_SIZE                0x3FFF      /**< Max MTU size */
#define BITS_NUM_OF_BYTE                8           /**< Bits num of byte */
#define BITS_NUM_OF_WORD                32          /**< Bits num of word */
#define BITS_SHIFT_OF_WORD              5           /**< Bits shift of word */
#define BITS_MASK_OF_WORD               0x1F        /**< Bits mask of word */

#define CTC_UINT32_BITS                 32
#define CTC_UINT8_BITS                  8

#define CTC_MAX_UINT16_VALUE            0xFFFF      /**< Max unsigned 16bit value */
#define CTC_MAX_UINT32_VALUE            0xFFFFFFFF  /**< Max unsigned 32bit value */
#define CTC_MAX_UINT8_VALUE             0xFF        /**< Max unsigned 8bit value */
#define CTC_PORT_BITMAP_IN_WORD         ((MAX_PORT_NUM_PER_CHIP + CTC_UINT32_BITS -1 ) / CTC_UINT32_BITS)     /**< port bitmap */
#define CTC_ACL_PORT_BITMAP_IN_BYTE     ((CTC_ACL_PORT_BITMAP_MAX_NUM + 7) / BITS_NUM_OF_BYTE)
#define CTC_ACL_PBR_CLASSID_MAX         63          /**< Max pbr classid*/
#define CTC_IPV6_ADDR_LEN_IN_BYTE       16          /**< IPv6 address length in byte */
#define CTC_IPV4_ADDR_LEN_IN_BYTE       4           /**< IPv4 address length in byte */
#define CTC_IPV6_ADDR_LEN_IN_BIT        128         /**< IPv6 address length in bit */
#define CTC_IPV4_ADDR_LEN_IN_BIT        32          /**< IPv4 address length in bit */

#define CTC_MAX_IPV4_VRFID              65534       /**< Max IPv4 vrfid */
#define CTC_MAX_IPV6_VRFID              4094        /**< Max IPv6 vrfid */

#define CTC_IPV6_ADDR_STR_LEN           44          /**< IPv6 address string length */

#define CTC_INVLD_NH_ID                 0xFFFFFFFF  /**< Invalid nexthop Id */
#define CTC_MPLS_SPACE_NUMBER           256         /**< Mpls space number */
#define CTC_MPLS_LABEL_NUM_PROCESS      3           /**< Mpls label num */
#define CTC_MPLS_OUT_SQE_NUM            64          /**< Mpls out sqe num */
#define CTC_MAX_ACL_CID                 255          /**< Max acl cid */
#define CTC_ACL_UNKNOWN_CID              255          /**< unkown acl cid, parsered cid > 255 will be classified to unkown CID */
#define CTC_ACL_DEFAULT_CID             0            /**< default CID */
#define CTC_MAX_QOS_DSCP_DOMAIN         15          /**< Max qos dscp domain */
#define CTC_MAX_QOS_DSCP_VALUE          63          /**< Max qos dscp value */

#define CTC_MAC_LEARN_USE_LOGIC_PORT    0x00000002  /**< Learning use logic-port per fid*/

#define CTC_DEFAULT_IPG                 0x14        /**< Default ipg */
#define CTC_MAX_CHIP_NUM                32
#define CTC_MAX_ACL_LKUP_NUM             8

/**
 @brief define port bitmap
*/
typedef uint32 ctc_port_bitmap_t[CTC_PORT_BITMAP_IN_WORD];

/**
 @brief define direction
*/
enum ctc_direction_e
{
    CTC_INGRESS,        /**< Ingress direction */
    CTC_EGRESS,         /**< Egress direction */
    CTC_BOTH_DIRECTION  /**< Both Ingress and Egress direction */
};
typedef enum ctc_direction_e ctc_direction_t;

/**
 @brief define scl location
*/
enum ctc_scl_location_e
{
    CTC_SCL_LOCATION_HASH   = 0,       /**< [GG] add entry to hash */
    CTC_SCL_LOCATION_TCAM0  = 1,       /**< [GG] add entry to tcam 0 */
    CTC_SCL_LOCATION_TCAM1  = 2,       /**< [GG] add entry to tcam 1 */
    CTC_SCL_LOCATION_MAX
};
typedef enum ctc_scl_location_e ctc_scl_location_t;

/**
 @brief define ip fragment
*/
enum ctc_ip_frag_e
{
    CTC_IP_FRAG_NON,              /**< [GB.GG.D2.TM] Non fragmented*/
    CTC_IP_FRAG_FIRST,            /**< [GB.GG.D2.TM] Fragmented and first fragment*/
    CTC_IP_FRAG_NON_OR_FIRST,     /**< [GB.GG.D2.TM] Non fragmented OR Fragmented and first fragment*/
    CTC_IP_FRAG_SMALL,            /**< [GB.GG.D2.TM] Small fragment */
    CTC_IP_FRAG_NOT_FIRST,        /**< [GB.GG.D2.TM] Not first fragment (Fragmented of cause) */
    CTC_IP_FRAG_YES,                /**< [GG.D2.TM] Any Fragmented */
    CTC_IP_FRAG_MAX
};
typedef enum ctc_ip_frag_e ctc_ip_frag_t;

/**
 @brief define acl enable
*/
enum ctc_acl_en_e
{
    CTC_ACL_DIS  = 0,       /**< [GB.GG.D2.TM] acl  disable */
    CTC_ACL_EN_0 = 0x1,     /**< [GB.GG.D2.TM] acl0 enable */
    CTC_ACL_EN_1 = 0x2,     /**< [GB.GG.D2.TM] acl1 enable */
    CTC_ACL_EN_2 = 0x4,     /**< [GB.GG.D2.TM] acl2 enable */
    CTC_ACL_EN_3 = 0x8      /**< [GB.GG.D2.TM] acl3 enable */
};
typedef enum ctc_acl_en_e ctc_acl_en_t;

/**
 @brief define acl ipv6 enable
*/
enum ctc_acl_ipv6_en_e
{
    CTC_ACL_IPV6_DIS  = 0,      /**< [GB] acl  ipv6 disable */
    CTC_ACL_IPV6_EN_0 = 0x1,    /**< [GB] acl0 ipv6 enable */
    CTC_ACL_IPV6_EN_1 = 0x2     /**< [GB] acl1 ipv6 enable */
};
typedef enum ctc_acl_ipv6_en_e ctc_acl_ipv6_en_t;

/**
 @brief define acl hash lookup type
*/
enum ctc_acl_hash_lkup_type_e
{
    CTC_ACL_HASH_LKUP_TYPE_DISABLE  = 0,        /**< [GG.D2.TM] Hash Lookup disable */
    CTC_ACL_HASH_LKUP_TYPE_L2       = 1,        /**< [GG.D2.TM] Hash Lookup matching L2 fields */
    CTC_ACL_HASH_LKUP_TYPE_L3       = 2,        /**< [GG.D2.TM] Hash Lookup matching L3 fields */
    CTC_ACL_HASH_LKUP_TYPE_L2_L3    = 3,        /**< [GG.D2.TM] Hash Lookup matching L2 + L3 fields */
    CTC_ACL_HASH_LKUP_TYPE_MAX
};
typedef enum ctc_acl_hash_lkup_type_e ctc_acl_hash_lkup_type_t;

/**
 @brief define acl tcam lookup type
*/
enum ctc_acl_tcam_lkup_type_e
{
    CTC_ACL_TCAM_LKUP_TYPE_L2           = 0,    /**< [GG.D2.TM] Tcam Lookup matching L2 fields */
    CTC_ACL_TCAM_LKUP_TYPE_L2_L3        = 1,    /**< [GG.D2.TM] Tcam Lookup matching L2 + L3 fields */
    CTC_ACL_TCAM_LKUP_TYPE_L3           = 2,    /**< [GG.D2.TM] Tcam Lookup matching L3 fields */
    CTC_ACL_TCAM_LKUP_TYPE_VLAN         = 3,    /**< [GG] Tcam Lookup matching L2 fields with double vlan */
    CTC_ACL_TCAM_LKUP_TYPE_L3_EXT       = 4,    /**< [D2.TM] Tcam Lookup matching abundant L3 fields */
    CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT    = 5,    /**< [D2.TM] Tcam Lookup matching abundant L2 + L3 fields */
    CTC_ACL_TCAM_LKUP_TYPE_CID          = 6,    /**< [D2.TM] Tcam Lookup matching category id */
    CTC_ACL_TCAM_LKUP_TYPE_INTERFACE    = 7,    /**< [D2.TM] Tcam Lookup matching interface */
    CTC_ACL_TCAM_LKUP_TYPE_FORWARD      = 8,    /**< [D2.TM] Tcam Lookup matching forwarding infomation */
    CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT  = 9,    /**< [D2.TM] Tcam Lookup matching abundant forwarding infomation */
    CTC_ACL_TCAM_LKUP_TYPE_COPP         = 10,    /**< [D2.TM] Tcam Lookup for copp */
    CTC_ACL_TCAM_LKUP_TYPE_UDF           = 11,      /**< [TM] Tcam Lookup for udf */
    CTC_ACL_TCAM_LKUP_TYPE_MAX
};
typedef enum ctc_acl_tcam_lkup_type_e ctc_acl_tcam_lkup_type_t;


/**
 @brief define ip version
*/
enum ctc_ip_ver_e
{
    CTC_IP_VER_4,   /**< IP version 4 */
    CTC_IP_VER_6,   /**< IP version 6 */
    MAX_CTC_IP_VER
};
typedef enum ctc_ip_ver_e ctc_ip_ver_t;

/**
 @brief  Define mpls tunnel mode
*/
enum ctc_mpls_tunnel_mode_e
{
    CTC_MPLS_TUNNEL_MODE_UNIFORM,      /**< The Tunnel mode of this label is uniform */
    CTC_MPLS_TUNNEL_MODE_SHORT_PIPE,   /**< The Tunnel mode of this label is short pipe */
    CTC_MPLS_TUNNEL_MODE_PIPE,         /**< The Tunnel mode of this label is pipe */
    CTC_MPLS_MAX_TUNNEL_MODE
};

/**
 @brief define metadata type
*/
enum ctc_metadata_type_e
{
    CTC_METADATA_TYPE_INVALID,       /**< [GG.D2.TM] metadata is invalid */
    CTC_METADATA_TYPE_METADATA,   /**< [GG.D2.TM] metadata is storing metadata */
    CTC_METADATA_TYPE_FID,               /**< [GG.D2.TM] metadata is storing fid*/
    CTC_METADATA_TYPE_VRFID,           /**< [GG.D2.TM] metadata is storing vrfid */
    CTC_METADATA_TYPE_MAX
};
typedef enum ctc_metadata_type_e ctc_metadata_type_t;


/**
 @brief define feature support
*/
enum ctc_feature_e
{
    CTC_FEATURE_PORT,
    CTC_FEATURE_VLAN,
    CTC_FEATURE_LINKAGG,
    CTC_FEATURE_CHIP,
    CTC_FEATURE_FTM,
    CTC_FEATURE_NEXTHOP,
    CTC_FEATURE_L2,
    CTC_FEATURE_L3IF,
    CTC_FEATURE_IPUC,
    CTC_FEATURE_IPMC,
    CTC_FEATURE_IP_TUNNEL,
    CTC_FEATURE_SCL,
    CTC_FEATURE_ACL,
    CTC_FEATURE_QOS,
    CTC_FEATURE_SECURITY,
    CTC_FEATURE_STATS,
    CTC_FEATURE_MPLS,
    CTC_FEATURE_OAM,
    CTC_FEATURE_APS,
    CTC_FEATURE_PTP,
    CTC_FEATURE_DMA,
    CTC_FEATURE_INTERRUPT,
    CTC_FEATURE_PACKET,
    CTC_FEATURE_PDU,
    CTC_FEATURE_MIRROR,
    CTC_FEATURE_BPE,
    CTC_FEATURE_STACKING,
    CTC_FEATURE_OVERLAY,
    CTC_FEATURE_IPFIX,
    CTC_FEATURE_EFD,
    CTC_FEATURE_MONITOR,
    CTC_FEATURE_FCOE,
    CTC_FEATURE_TRILL,
    CTC_FEATURE_WLAN,           /**< [D2.TM] wlan feature */
    CTC_FEATURE_NPM,            /**< [D2.TM] NPM feature */
    CTC_FEATURE_VLAN_MAPPING,
    CTC_FEATURE_DOT1AE,
    CTC_FEATURE_DIAG,
    CTC_FEATURE_SYNC_ETHER,
    CTC_FEATURE_APP_VLAN_PORT,
    CTC_FEATURE_DATAPATH,
    CTC_FEATURE_REGISTER,

    CTC_FEATURE_MAX
};
typedef enum ctc_feature_e ctc_feature_t;


/**@} end of @desfgroup  const CONST */

#ifdef __cplusplus
}
#endif

#endif /* _CTC_CONST_H*/

