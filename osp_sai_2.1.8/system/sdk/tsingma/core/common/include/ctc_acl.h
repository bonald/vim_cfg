/**
 @file ctc_acl.h

 @date 2012-10-19

 @version v2.0

 The file define api struct used in ACL.
*/

#ifndef _CTC_ACL_H_
#define _CTC_ACL_H_

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "common/include/ctc_const.h"
#include "common/include/ctc_common.h"
#include "common/include/ctc_field.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define  CTC_ACL_GROUP_ID_NORMAL         0xFFFF0000  /**< [GB.GG.D2.TM]Max normal group id */
/* Below groups are created already, just use it */
#define  CTC_ACL_GROUP_ID_HASH_MAC       0xFFFF0001  /**< [GB.GG.D2.TM]Max hash mac group id */
#define  CTC_ACL_GROUP_ID_HASH_IPV4      0xFFFF0002  /**< [GB.GG.D2.TM]Max hash ipv4 group id */
#define  CTC_ACL_GROUP_ID_HASH_MPLS      0xFFFF0003  /**< [GG.D2.TM]Max hash mpls group id */
#define  CTC_ACL_GROUP_ID_HASH_L2_L3     0xFFFF0004  /**< [GG.D2.TM]Max hash L2_L3 group id */
#define  CTC_ACL_GROUP_ID_HASH_IPV6      0xFFFF0005  /**< [GG.D2.TM]Max hash ipv6 group id */
#define  CTC_ACL_GROUP_ID_HASH_RSV_0     0xFFFF0006  /**< [GG.D2.TM]Max hash rsv_0 group id */
#define  CTC_ACL_GROUP_ID_MAX            0xFFFF0007  /**< [GB.GG.D2.TM]Always last group id. Invalid group id */
#define  CTC_ACL_ENTRY_PRIORITY_HIGHEST  0xFFFFFFFF  /**< [GB.GG.D2.TM]Highest acl entry priority */
#define  CTC_ACL_ENTRY_PRIORITY_LOWEST   0           /**< [GB.GG.D2.TM]Lowest acl entry priority */
#define  CTC_ACL_ENTRY_PRIORITY_DEFAULT  1           /**< [GB.GG.D2.TM]Default acl entry priority */

#define  CTC_ACL_KEY_SIZE_SINGLE         0           /**< [GG]Single acl key size */
#define  CTC_ACL_KEY_SIZE_DOUBLE         1           /**< [GG]Double acl key size */

#define  CTC_ACL_UDF_BYTE_NUM            16           /**< [GB.GG.D2.TM]Acl udf byte number */
#define  CTC_ACL_UDF_FIELD_NUM           4            /**< [D2.TM] number of ACL udf field */

#define  CTC_ACL_MAX_USER_ENTRY_ID       0xFFFF0000  /**< [GB.GG.D2.TM]Max acl user entry id */
#define  CTC_ACL_MIN_USER_ENTRY_ID       0           /**< [GB.GG.D2.TM]Min acl user entry id */
/****************************************************************
*
* Data Structures
*
****************************************************************/

/**
@brief  ACl Key Type
*/
enum ctc_acl_key_type_e
{
    CTC_ACL_KEY_MAC = 0,            /**< [GB.GG.D2.TM] ACL MAC key type */
    CTC_ACL_KEY_IPV4,               /**< [GB.GG.D2.TM] ACL IPv4 key type */
    CTC_ACL_KEY_MPLS,               /**< [GB.GG] ACL MPLS key type;In DT2,replaced by CTC_ACL_KEY_MAC_IPV4*/
    CTC_ACL_KEY_IPV6,               /**< [GB.GG.D2.TM] Mode 0: ACL Mac+IPv6 key type;
                                                        Mode 1: ACL Ipv6 key type */
    CTC_ACL_KEY_HASH_MAC,           /**< [GB.GG.D2.TM] ACL MAC Hash key type */
    CTC_ACL_KEY_HASH_IPV4,          /**< [GB.GG.D2.TM] ACL IPv4 Hash key type */
    CTC_ACL_KEY_HASH_L2_L3,         /**< [GG.D2.TM] ACL L2 L3 Hash key type */
    CTC_ACL_KEY_HASH_MPLS,          /**< [GG.D2.TM] ACL MPLS Hash key type */
    CTC_ACL_KEY_HASH_IPV6,          /**< [GG.D2.TM] ACL IPv6 Hash key type */
    CTC_ACL_KEY_PBR_IPV4,           /**< [GB.GG] ACL PBR IPv4 key type ;In DT2 or TM,replaced by CTC_ACL_KEY_IPV4 */
    CTC_ACL_KEY_PBR_IPV6,           /**< [GB.GG] ACL PBR IPv6 key type ;In DT2 or TM,replaced by CTC_ACL_KEY_IPV6 */

    CTC_ACL_KEY_IPV4_EXT,           /**< [D2.TM] ACL IPv4 extend key type */
    CTC_ACL_KEY_MAC_IPV4,           /**< [D2.TM] ACL Mac+IPv4 key type */
    CTC_ACL_KEY_MAC_IPV4_EXT,       /**< [D2.TM] ACL Mac+IPv4 extend key type */
    CTC_ACL_KEY_IPV6_EXT,           /**< [D2.TM] ACL IPv6 extend key type */
    CTC_ACL_KEY_MAC_IPV6,           /**< [D2.TM] ACL Mac+IPv6 extend key type */
    CTC_ACL_KEY_CID,                /**< [D2.TM] ACL Category Id key type */
    CTC_ACL_KEY_INTERFACE,          /**< [D2.TM] ACL Interface key type */
    CTC_ACL_KEY_FWD,                /**< [D2.TM] ACL Forward key type */
    CTC_ACL_KEY_FWD_EXT,            /**< [D2.TM] ACL Forward key type */
    CTC_ACL_KEY_COPP,               /**< [D2.TM] ACL Copp key type */
    CTC_ACL_KEY_COPP_EXT,           /**< [D2.TM] ACL Copp extend key type */

    CTC_ACL_KEY_UDF,                /**< [TM] ACL UDF key type */
    CTC_ACL_KEY_NUM
};
typedef enum ctc_acl_key_type_e ctc_acl_key_type_t      ;

/**
@brief Define ACL Ad Vxlan struct
*/
struct ctc_acl_vxlan_rsv_edit_s
{
    uint8   edit_mode;                  /**< [TM] Vxlan reserved data edit mode: 0-None, 1-Merge, 2-Clear and 3-Replace */
    uint8   vxlan_flags;                 /**< [TM] Vxlan flags */
    uint8   vxlan_rsv2;                  /**< [TM] Vxlan reserved2 */
    uint8   rsv;
    uint32  vxlan_rsv1;                 /**< [TM] Vxlan reserved1 */

};
typedef struct ctc_acl_vxlan_rsv_edit_s ctc_acl_vxlan_rsv_edit_t;

/**
@brief Define Acl Ad Time Stamp struct
*/
struct ctc_acl_timestamp_s
{
    uint8 mode;                           /**< [TM] Time stamp process mode: 0-without time stamp, 1-with time stamp */
    uint8 rsv[3];
};
 typedef struct ctc_acl_timestamp_s ctc_acl_timestamp_t;

/**
@brief Define ACl Hash Mac Key structure
*/
struct ctc_acl_hash_mac_key_s
{
    uint32 field_sel_id;         /**< [GG.D2.TM] Field Select Id*/

    uint32 gport;                /**< [GB.GG.D2.TM] Global source port, share with logic port and metadata */
    uint8  gport_is_logic_port;  /**< [GB] GPort is logic port */

    mac_addr_t mac_da;           /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_sa;           /**< [GG.D2.TM] MAC-SA */

    uint16 eth_type;             /**< [GB.GG.D2.TM] Ethernet type */

    uint8  is_ctag;              /**< [GG.D2.TM] tag is ctag */
    uint8  cos;                  /**< [GB.GG.D2.TM] tag Cos */
    uint16 vlan_id;              /**< [GB.GG.D2.TM] tag Vlan Id */
    uint8  tag_valid;            /**< [GG.D2.TM] tag exist */
    uint8  cfi;                  /**< [GG.D2.TM] tag Cfi */

    uint8  vrange_grpid;         /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint16 vlan_end;             /**< [GG.D2.TM] orignal max c/s vlan id need to do vlan range acl*/
};
typedef struct ctc_acl_hash_mac_key_s  ctc_acl_hash_mac_key_t;

/**
@brief Define ACl Hash Ipv4 Key structure
*/
struct ctc_acl_hash_ipv4_key_s
{
    uint32 field_sel_id;         /**< [GG.D2.TM] Field Select Id*/

    uint32 gport;                /**< [GB.GG.D2.TM] Global source port */
    uint8  gport_is_logic_port;  /**< [GB] Gport is logic port */
    uint8  rsv;
    uint32 metadata;             /**< [GG.D2.TM] Metadata */

    uint32 ip_sa;                /**< [GB.GG.D2.TM] IP-SA */
    uint32 ip_da;                /**< [GB.GG.D2.TM] IP-DA */

    uint8  dscp;                 /**< [GB.GG.D2.TM] DSCP */
    uint8  ecn;                  /**< [GG.D2.TM] ECN */
    uint8  l4_protocol;          /**< [GB.GG.D2.TM] L4 Protocol */
    uint8  arp_packet;           /**< [GB] Is arp packet */

    uint8  icmp_type;            /**< [GG.D2.TM] ICMP type */
    uint8  icmp_code;            /**< [GG.D2.TM] ICMP code */

    uint16 l4_src_port;          /**< [GB.GG.D2.TM] Layer 4 source port.*/
    uint16 l4_dst_port;          /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint32 vni;                  /**< [GG.D2.TM] Vxlan Vni ID */
    uint32 gre_key;              /**< [GG.D2.TM] Gre or Nvgre Key*/
};
typedef struct ctc_acl_hash_ipv4_key_s  ctc_acl_hash_ipv4_key_t;


/**
@brief Define ACl Hash Mpls Key structure
*/
struct ctc_acl_hash_mpls_key_s
{
    uint32 field_sel_id;          /**< [GG.D2.TM] Field Select Id*/

    uint32 gport;                 /**< [GG.D2.TM] Global source port */
    uint32 metadata;              /**< [GG.D2.TM] Metadata */

    uint8  mpls_label_num;        /**< [GG.D2.TM] MPLS Label Number */

    uint32 mpls_label0_label;     /**< [GG.D2.TM] MPLS Label 0 */
    uint8  mpls_label0_exp;       /**< [GG.D2.TM] MPLS Exp 0 */
    uint8  mpls_label0_s;         /**< [GG.D2.TM] MPLS S-bit 0 */
    uint8  mpls_label0_ttl;       /**< [GG.D2.TM] MPLS Ttl 0 */

    uint32 mpls_label1_label;     /**< [GG.D2.TM] MPLS Label 1 */
    uint8  mpls_label1_exp;       /**< [GG.D2.TM] MPLS Exp 1 */
    uint8  mpls_label1_s;         /**< [GG.D2.TM] MPLS S-bit 1 */
    uint8  mpls_label1_ttl;       /**< [GG.D2.TM] MPLS Ttl 1 */

    uint32 mpls_label2_label;     /**< [GG.D2.TM] MPLS Label 2 */
    uint8  mpls_label2_exp;       /**< [GG.D2.TM] MPLS Exp 2 */
    uint8  mpls_label2_s;         /**< [GG.D2.TM] MPLS S-bit 2 */
    uint8  mpls_label2_ttl;       /**< [GG.D2.TM] MPLS Ttl 2 */
};
typedef struct ctc_acl_hash_mpls_key_s  ctc_acl_hash_mpls_key_t;


/**
@brief Define ACl Hash Ipv6 Key structure
*/
struct ctc_acl_hash_ipv6_key_s
{
    uint32 field_sel_id;         /**< [GG.D2.TM] Field Select Id*/

    uint32 gport;                /**< [GG.D2.TM] Global source port, share with logic port and metadata*/
    ipv6_addr_t ip_sa;           /**< [GG.D2.TM] IP-SA */
    ipv6_addr_t ip_da;           /**< [GG.D2.TM] IP-DA */

    uint8  dscp;                 /**< [GG.D2.TM] DSCP */
    uint8  l4_type;              /**< [GG.D2.TM] Layer 4 type */
    uint8  icmp_type;            /**< [GG.D2.TM] ICMP type */
    uint8  icmp_code;            /**< [GG.D2.TM] ICMP code */

    uint16 l4_src_port;          /**< [GG.D2.TM] Layer 4 source port.*/
    uint16 l4_dst_port;          /**< [GG.D2.TM] Layer 4 dest port */
    uint32 vni;                  /**< [GG.D2.TM] Vxlan Vni ID */
    uint32 gre_key;              /**< [GG.D2.TM] Gre or Nvgre Key*/
};
typedef struct ctc_acl_hash_ipv6_key_s  ctc_acl_hash_ipv6_key_t;


/**
@brief Define ACl Hash L2_L3 Key structure
*/
struct ctc_acl_hash_l2_l3_key_s
{
    uint32 field_sel_id;        /**< [GG.D2.TM] Field Select Id*/
    uint8  l3_type;             /**< [GG.D2.TM] config to distinguish mpls, ipv4. refer ctc_parser_l3_type_t  */

    uint32 gport;               /**< [GG.D2.TM] Global source port */
    uint32 metadata;            /**< [GG.D2.TM] Metadata*/

    mac_addr_t mac_da;          /**< [GG.D2.TM] MAC-DA */
    mac_addr_t mac_sa;          /**< [GG.D2.TM] MAC-DA */

    uint8  stag_cos;            /**< [GG.D2.TM] Stag Cos */
    uint16 stag_vlan;           /**< [GG.D2.TM] Stag Vlan Id */
    uint8  stag_valid;          /**< [GG.D2.TM] Stag Exist */
    uint8  stag_cfi;            /**< [GG.D2.TM] Stag Cos */

    uint8  ctag_cos;            /**< [GG.D2.TM] Ctag Cos */
    uint16 ctag_vlan;           /**< [GG.D2.TM] Ctag Vlan Id */
    uint8  ctag_valid;          /**< [GG.D2.TM] Ctag Exist */
    uint8  ctag_cfi;            /**< [GG.D2.TM] Ctag Cos */

    uint8  vrange_grpid;        /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint16 cvlan_end;           /**< [GG.D2.TM] orignal max cvlan id need to do vlan range acl*/
    uint16 svlan_end;           /**< [GG.D2.TM] orignal max svlan id need to do vlan range acl*/

    uint32 ip_sa;               /**< [GG.D2.TM] IP-SA */
    uint32 ip_da;               /**< [GG.D2.TM] IP-DA */

    uint8  dscp;                /**< [GG.D2.TM] DSCP */
    uint8  ecn;                 /**< [GG.D2.TM] ECN*/
    uint8  ttl;                 /**< [GG.D2.TM] TTL */

    uint16 eth_type;            /**< [GG.D2.TM] Ethernet type */
    uint8  l4_type;             /**< [GG.D2.TM] Layer4 type */

    uint8  mpls_label_num;      /**< [GG.D2.TM] Label Num */

    uint32 mpls_label0_label;   /**< [GG.D2.TM] MPLS Label 0 */
    uint8  mpls_label0_exp;     /**< [GG.D2.TM] MPLS Exp 0 */
    uint8  mpls_label0_s;       /**< [GG.D2.TM] MPLS S-bit 0 */
    uint8  mpls_label0_ttl;     /**< [GG.D2.TM] MPLS Ttl 0 */

    uint32 mpls_label1_label;   /**< [GG.D2.TM] MPLS Label 1 */
    uint8  mpls_label1_exp;     /**< [GG.D2.TM] MPLS Exp 1 */
    uint8  mpls_label1_s;       /**< [GG.D2.TM] MPLS S-bit 1 */
    uint8  mpls_label1_ttl;     /**< [GG.D2.TM] MPLS Ttl 1 */

    uint32 mpls_label2_label;   /**< [GG.D2.TM] MPLS Label 2 */
    uint8  mpls_label2_exp;     /**< [GG.D2.TM] MPLS Exp 2 */
    uint8  mpls_label2_s;       /**< [GG.D2.TM] MPLS S-bit 2 */
    uint8  mpls_label2_ttl;     /**< [GG.D2.TM] MPLS Ttl 2 */

    uint16 l4_src_port;         /**< [GG.D2.TM] Layer 4 source port.*/
    uint16 l4_dst_port;         /**< [GG.D2.TM] Layer 4 dest port */

    uint8  icmp_type;           /**< [GG.D2.TM] ICMP type */
    uint8  icmp_code;           /**< [GG.D2.TM] ICMP code */
    uint32 vni;                  /**< [GG.D2.TM] Vxlan Vni ID */
    uint32 gre_key;              /**< [GG.D2.TM] Gre or Nvgre Key*/
};
typedef struct ctc_acl_hash_l2_l3_key_s  ctc_acl_hash_l2_l3_key_t;

/**
@brief  Mac key field flag
*/
enum ctc_acl_mac_key_flag_e
{
    CTC_ACL_MAC_KEY_FLAG_MAC_DA      = 1U << 0,  /**< [GB.GG.D2.TM] Valid MAC-DA in MAC key */
    CTC_ACL_MAC_KEY_FLAG_MAC_SA      = 1U << 1,  /**< [GB.GG.D2.TM] Valid MAC-SA in MAC key */
    CTC_ACL_MAC_KEY_FLAG_CVLAN       = 1U << 3,  /**< [GB.GG.D2.TM] Valid C-VLAN in MAC key */
    CTC_ACL_MAC_KEY_FLAG_CTAG_COS    = 1U << 4,  /**< [GB.GG.D2.TM] Valid C-tag CoS in MAC key */
    CTC_ACL_MAC_KEY_FLAG_SVLAN       = 1U << 5,  /**< [GB.GG.D2.TM] Valid S-VLAN in MAC key */
    CTC_ACL_MAC_KEY_FLAG_STAG_COS    = 1U << 6,  /**< [GB.GG.D2.TM] Valid S-tag CoS in MAC key */
    CTC_ACL_MAC_KEY_FLAG_ETH_TYPE    = 1U << 7,  /**< [GB.GG.D2.TM] Valid eth-type in MAC key */
    CTC_ACL_MAC_KEY_FLAG_L2_TYPE     = 1U << 8,  /**< [GB.D2.TM] Valid l2-type in MAC key */
    CTC_ACL_MAC_KEY_FLAG_L3_TYPE     = 1U << 9,  /**< [GB.GG] Valid l3-type in MAC key */
    CTC_ACL_MAC_KEY_FLAG_CTAG_CFI    = 1U << 10, /**< [GB.GG.D2.TM] Valid l2-type in MAC key */
    CTC_ACL_MAC_KEY_FLAG_STAG_CFI    = 1U << 11, /**< [GB.GG.D2.TM] Valid l3-type in MAC key */
    CTC_ACL_MAC_KEY_FLAG_ARP_OP_CODE = 1U << 12, /**< [GB] Valid Arp-op-code in MAC key */
    CTC_ACL_MAC_KEY_FLAG_IP_SA       = 1U << 13, /**< [GB] Valid Ip-sa in MAC key */
    CTC_ACL_MAC_KEY_FLAG_IP_DA       = 1U << 14, /**< [GB] Valid Ip-da in MAC key */
    CTC_ACL_MAC_KEY_FLAG_STAG_VALID  = 1U << 15, /**< [GB.GG.D2.TM] Valid stag-valid in MAC key */
    CTC_ACL_MAC_KEY_FLAG_CTAG_VALID  = 1U << 16, /**< [GB.GG.D2.TM] Valid ctag-valid in MAC key */
    CTC_ACL_MAC_KEY_FLAG_VLAN_NUM    = 1U << 17, /**< [GG.D2.TM] Valid vlan number in MAC key */
    CTC_ACL_MAC_KEY_FLAG_METADATA    = 1U << 18  /**< [GG.D2.TM] Valid metadata in MAC key */
};
typedef enum ctc_acl_mac_key_flag_e ctc_acl_mac_key_flag_t;


/**
@brief Define ACl Mac Key structure
*/
struct ctc_acl_mac_key_s
{
    uint32 flag;                /**< [GB.GG.D2.TM] Bitmap of ctc_acl_mac_key_flag_t */

    mac_addr_t mac_da;          /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;     /**< [GB.GG.D2.TM] MAC-DA mask */
    mac_addr_t mac_sa;          /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;     /**< [GB.GG.D2.TM] MAC-SA mask */

    uint16 cvlan;               /**< [GB.GG.D2.TM] C-VLAN */
    uint16 cvlan_mask;          /**< [GB.GG.D2.TM] C-VLAN mask*/
    uint16 svlan;               /**< [GB.GG.D2.TM] S-VLAN */
    uint16 svlan_mask;          /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  vrange_grpid;        /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint16 cvlan_end;           /**< [GG.D2.TM] orignal max cvlan id need to do vlan range acl*/
    uint16 svlan_end;           /**< [GG.D2.TM] orignal max svlan id need to do vlan range acl*/

    uint8  ctag_cos;            /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;       /**< [GB.GG.D2.TM] C-tag CoS mask*/
    uint8  ctag_cfi;            /**< [GB.GG.D2.TM] C-tag CFI */
    uint8  stag_cos;            /**< [GB.GG.D2.TM] S-tag CoS */
    uint8  stag_cos_mask;       /**< [GB.GG.D2.TM] S-tag CoS mask*/
    uint8  stag_cfi;            /**< [GB.GG.D2.TM] S-tag CFI */

    uint8  l2_type;             /**< [GB.D2.TM] Layer 2 type. Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;        /**< [GB.D2.TM] Layer 2 type mask*/
    uint8  l3_type;             /**< [GB.GG] Layer 3 type. Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;        /**< [GB.GG] Layer 3 type mask*/
    uint16 eth_type;            /**< [GB.GG.D2.TM] Ethernet type, for unknow or flexible L3 type in GG */
    uint16 eth_type_mask;       /**< [GB.GG.D2.TM] Ethernet type mask*/


    uint16 arp_op_code;         /**< [GB] Arp op code*/
    uint16 arp_op_code_mask;    /**< [GB] Arp op code mask*/
    uint32 ip_sa;               /**< [GB] Ip source address */
    uint32 ip_sa_mask;          /**< [GB] Ip source address mask */
    uint32 ip_da;               /**< [GB] Ip destination address */
    uint32 ip_da_mask;          /**< [GB] Ip destination address mask*/
    uint8  stag_valid;          /**< [GB.GG.D2.TM] Stag valid */
    uint8  ctag_valid;          /**< [GB.GG.D2.TM] Ctag valid */

    uint32 metadata;            /**< [GG.D2.TM] metadata */
    uint32 metadata_mask;       /**< [GG.D2.TM] metadata mask*/

    uint8 vlan_num;             /**< [GG.D2.TM] vlan number */
    uint8 vlan_num_mask;        /**< [GG.D2.TM] vlan number mask */

    ctc_field_port_t port;      /**< [GB.GG.D2.TM] valid when group type CTC_ACL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */

};
typedef struct ctc_acl_mac_key_s  ctc_acl_mac_key_t;

/**
@brief  Ipv4 key field flag
*/
enum ctc_acl_ipv4_key_flag_e
{

    CTC_ACL_IPV4_KEY_FLAG_MAC_DA        = 1U << 0,  /**< [GB.GG.D2.TM] Valid MAC-DA in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_MAC_SA        = 1U << 1,  /**< [GB.GG.D2.TM] Valid MAC-SA in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_CVLAN         = 1U << 3,  /**< [GB.GG.D2.TM] Valid C-VLAN in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_CTAG_COS      = 1U << 4,  /**< [GB.GG.D2.TM] Valid C-tag CoS in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_SVLAN         = 1U << 5,  /**< [GB.GG.D2.TM] Valid S-VLAN in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_STAG_COS      = 1U << 6,  /**< [GB.GG.D2.TM] Valid S-tag CoS in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_L2_TYPE       = 1U << 7,  /**< [GB] Valid l2-type in IPv4 key, NOTE: Limited support on GB, When merge key,
                                                                    collision with FLAG_DSCP/FLAG_PRECEDENCE, And ineffective for ipv4_packet*/
    CTC_ACL_IPV4_KEY_FLAG_L3_TYPE       = 1U << 8,  /**< [GG.D2.TM] Valid l3-type in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_IP_SA         = 1U << 9,  /**< [GB.GG.D2.TM] Valid IP-SA in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_IP_DA         = 1U << 10, /**< [GB.GG.D2.TM] Valid IP-DA in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL   = 1U << 11, /**< [GB.GG.D2.TM] Valid L4-Proto in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_DSCP          = 1U << 12, /**< [GB.GG.D2.TM] Valid DSCP in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_PRECEDENCE    = 1U << 13, /**< [GB.GG.D2.TM] Valid IP Precedence in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_IP_FRAG       = 1U << 14, /**< [GB.GG.D2.TM] Valid fragment in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_IP_OPTION     = 1U << 15, /**< [GB.GG.D2.TM] Valid IP option in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_ROUTED_PACKET = 1U << 16, /**< [GB.GG.D2.TM] Valid routed packet in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_CTAG_CFI      = 1U << 17, /**< [GB] Valid C-tag cfi in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_STAG_CFI      = 1U << 18, /**< [GB.GG.D2.TM] Valid S-tag cfi in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_STAG_VALID    = 1U << 19, /**< [GB.GG.D2.TM] Valid Stag-valid in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID    = 1U << 20, /**< [GB.GG.D2.TM] Valid Ctag-valid in IPv4 key */
    CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE      = 1U << 22, /**< [GB.GG.D2.TM] Valid Ether type in IPv4 Key, NOTE:  (GB) Used when merge key, collision with L4_DST_PORT,
                                                                    Do NOT set this flag on Egress ACL. And If required matching ipv4 packet,
                                                                    Do NOT set this flag with set ether_type = 0x0800, Set CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET instead */
    CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET   = 1U << 23, /**< [GB] Valid Ipv4 packet in IPv4 Key, NOTE: (GB) used when merge key, For non-merge, Ipv4 key only for ipv4/arp packet*/
    CTC_ACL_IPV4_KEY_FLAG_ARP_PACKET    = 1U << 24, /**< [GB] Valid Arp packet in IPv4 Key, NOTE: (GB) used when merge key, For non-merge, Ipv4 key only for ipv4/arp packet*/
    CTC_ACL_IPV4_KEY_FLAG_ECN           = 1U << 25, /**< [GG.D2.TM] Valid Ecn in IPv4 Key*/
    CTC_ACL_IPV4_KEY_FLAG_METADATA      = 1U << 26,  /**< [GG.D2.TM] Valid metadata in ipv4 key */
    CTC_ACL_IPV4_KEY_FLAG_UDF           = 1U << 27,  /**< [GG.D2.TM] Valid udf in ipv4 key */
    CTC_ACL_IPV4_KEY_FLAG_VLAN_NUM      = 1U << 28,  /**< [GG.D2.TM] Valid vlan number in ipv4 key */
    CTC_ACL_IPV4_KEY_FLAG_PKT_LEN_RANGE = 1U << 29,  /**< [GG.D2.TM] Valid packet length range in ipv4 key */
    CTC_ACL_IPV4_KEY_FLAG_IP_HDR_ERROR  = 1U << 30   /**< [GG.D2.TM] Valid ip header error in ipv4 key */
};
typedef enum ctc_acl_ipv4_key_flag_e ctc_acl_ipv4_key_flag_t;


/**
@brief  Ipv4 key sub field flag
*/
enum ctc_acl_ipv4_key_sub_flag_e
{
    CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT       = 1U << 0, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP, Valid L4 source port in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT       = 1U << 1, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP, Valid L4 destination port in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS         = 1U << 2, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP, Valid TCP flag in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE         = 1U << 3, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP, Valid ICMP type in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE         = 1U << 4, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP, Valid ICMP code in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE         = 1U << 5, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = IGMP, Valid IGMP type in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_OP_CODE       = 1U << 6, /**< [GB.GG.D2.TM] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp op code in Arp Key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP     = 1U << 7, /**< [GB.GG.D2.TM] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp sender Ip in Arp Key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP     = 1U << 8, /**< [GB.GG.D2.TM] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp target Ip in Arp Key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE = 1U << 9, /**< [GG.D2.TM] (GB)Depend on ARP_PACKET, (GG)Depend on L3_TYPE,Valid Arp target Ip in Arp Key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_GRE_KEY           = 1U << 10,/**< [GG.D2.TM] Depend on L4_PROTOCOL = GRE, Valid GRE key in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_VNI               = 1U << 11,/**< [GG.D2.TM] Depend on L4_PROTOCOL = UDP, Valid vni in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_NVGRE_KEY         = 1U << 12,/**< [GG.D2.TM] Depend on L4_PROTOCOL = NVGRE, Valid NVGRE key in IPv4 key */
    CTC_ACL_IPV4_KEY_SUB_FLAG_GRE_CRKS          = 1U << 13 /**< [GG] Depend on L4_PROTOCOL = GRE, valid C R K S 4 bits in GRE header in IPv4 key */
};
typedef enum ctc_acl_ipv4_key_sub_flag_e ctc_acl_ipv4_key_sub_flag_t;

/**
@brief  Ipv4 key sub1 field flag
*/
enum ctc_acl_ipv4_key_sub1_flag_e
{

    CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL_NUM = 1U << 0,            /**< [GG.D2.TM] MPLS label number field*/
    CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL0    = 1U << 1,            /**< [GG.D2.TM] MPLS label 0 as key field*/
    CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL1    = 1U << 2,            /**< [GG.D2.TM] MPLS label 1 as key field*/
    CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL2    = 1U << 3             /**< [GG.D2.TM] MPLS label 2 as key field*/
};
typedef enum ctc_acl_ipv4_key_sub1_flag_e ctc_acl_ipv4_key_sub1_flag_t;

/**
@brief  Ipv4 key sub3 field flag
*/
enum ctc_acl_ipv4_key_sub3_flag_e
{
    CTC_ACL_IPV4_KEY_SUB3_FLAG_ETHOAM_LEVEL = 1U << 0,      /**< [GB.GG.D2.TM] Ether oam level as key field*/
    CTC_ACL_IPV4_KEY_SUB3_FLAG_ETHOAM_OPCODE = 1U << 1      /**< [GB.GG.D2.TM] Ether oam opcode as key field*/
};
typedef enum ctc_acl_ipv4_key_sub3_flag_e ctc_acl_ipv4_key_sub3_flag_t;


/**
@brief  ACL tcp field flag
*/
enum ctc_acl_tcp_flag_flag_e
{
    CTC_ACL_TCP_FIN_FLAG = 1U << 0, /**< [GB.GG] TCP fin flag */
    CTC_ACL_TCP_SYN_FLAG = 1U << 1, /**< [GB.GG] TCP syn flag */
    CTC_ACL_TCP_RST_FLAG = 1U << 2, /**< [GB.GG] TCP rst flag */
    CTC_ACL_TCP_PSH_FLAG = 1U << 3, /**< [GB.GG] TCP psh flag */
    CTC_ACL_TCP_ACK_FLAG = 1U << 4, /**< [GB.GG] TCP ack flag */
    CTC_ACL_TCP_URG_FLAG = 1U << 5  /**< [GB.GG] TCP urg flag */
};
typedef enum ctc_acl_tcp_flag_flag_e ctc_acl_tcp_flag_flag_t;


/**
@brief Define ACl IPv4 Key structure
*/
struct ctc_acl_ipv4_key_s
{
    uint32 flag;                            /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV4_KEY_FLAG_XXX */
    uint32 sub_flag;                        /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV4_KEY_SUB_FLAG_XXX */
    uint32 sub1_flag;                       /**< [GG.D2.TM] Bitmap of CTC_ACL_IPV4_KEY_SUB1_FLAG_XXX */
    uint32 sub2_flag;                       /**< [GG.D2.TM] Bitmap of CTC_ACL_IPV4_KEY_SUB2_FLAG_XXX */
    uint32 sub3_flag;                       /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV4_KEY_SUB3_FLAG_XXX */
    uint8  key_size;                        /**< [GG.D2.TM] Key Size */

    uint32 ip_sa;                           /**< [GB.GG.D2.TM] IP-SA */
    uint32 ip_sa_mask;                      /**< [GB.GG.D2.TM] IP-SA mask */
    uint32 ip_da;                           /**< [GB.GG.D2.TM] IP-DA */
    uint32 ip_da_mask;                      /**< [GB.GG.D2.TM] IP-DA mask */

    uint16 l4_src_port_0;                   /**< [GB.GG.D2.TM] Layer 4 source port,Min/Data */
    uint16 l4_src_port_1;                   /**< [GB.GG.D2.TM] Layer 4 source port,Max/Mask */

    uint8  l4_src_port_use_mask;            /**< [GB.GG.D2.TM] Use mask instead of range*/
    uint8  l4_dst_port_use_mask;            /**< [GB.GG.D2.TM] Use mask instead of range*/
    uint8  ip_option;                       /**< [GB.GG.D2.TM] Ip option */
    uint8  routed_packet;                   /**< [GB.GG.D2.TM] Routed packet */

    uint16 l4_dst_port_0;                   /**< [GB.GG.D2.TM] Layer 4 dest port */
    uint16 l4_dst_port_1;                   /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint32 pkt_len_min;                     /**< [GG.D2.TM] Packet length Min */
    uint32 pkt_len_max;                     /**< [GG.D2.TM] Packet Length Max */

    uint8  tcp_flags_match_any;             /**< [GB.GG.D2.TM] 0:match all. 1:match any*/
    uint8  tcp_flags;                       /**< [GB.GG.D2.TM] Tcp flag bitmap*/
    uint8  dscp;                            /**< [GB.GG.D2.TM] DSCP */
    uint8  dscp_mask;                       /**< [GB.GG.D2.TM] DSCP mask*/

    uint8  l4_protocol;                     /**< [GB.GG.D2.TM] Layer 4 protocol */
    uint8  l4_protocol_mask;                /**< [GB.GG.D2.TM] Layer 4 protocol mask, for gg if protocol is TCP/UDP/GRE, the mask must be 0xff*/
    uint8  igmp_type;                       /**< [GB.GG.D2.TM] IGMP type */
    uint8  igmp_type_mask;                  /**< [GB.GG.D2.TM] IGMP type mask*/

    uint8  icmp_type;                       /**< [GB.GG.D2.TM] ICMP type */
    uint8  icmp_type_mask;                  /**< [GB.GG.D2.TM] ICMP type mask*/
    uint8  icmp_code;                       /**< [GB.GG.D2.TM] ICMP code */
    uint8  icmp_code_mask;                  /**< [GB.GG.D2.TM] ICMP code mask*/

    uint8  ip_frag;                         /**< [GB.GG.D2.TM] Ip fragment, CTC_IP_FRAG_XXX */
    uint8  ip_hdr_error;                 /**< [GG.D2.TM] ip header error packet */
    uint8  rsv0[2];

    mac_addr_t mac_sa;                      /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;                 /**< [GB.GG.D2.TM] MAC-SA mask */
    mac_addr_t mac_da;                      /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;                 /**< [GB.GG.D2.TM] MAC-DA mask */

    uint16 cvlan;                           /**< [GB.GG.D2.TM] C-VLAN */
    uint16 cvlan_mask;                      /**< [GB.GG.D2.TM] C-VLAN mask*/
    uint16 svlan;                           /**< [GB.GG.D2.TM] S-VLAN */
    uint16 svlan_mask;                      /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  vrange_grpid;                    /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint16 cvlan_end;                       /**< [GG.D2.TM] orignal max cvlan id need to do vlan range acl*/
    uint16 svlan_end;                       /**< [GG.D2.TM] orignal max svlan id need to do vlan range acl*/

    uint8  ctag_cos;                        /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;                   /**< [GB.GG.D2.TM] C-tag CoS mask*/
    uint8  stag_cos;                        /**< [GB.GG.D2.TM] S-tag CoS */
    uint8  stag_cos_mask;                   /**< [GB.GG.D2.TM] S-tag CoS mask*/

    uint8  ctag_cfi;                        /**< [GB.GG.D2.TM] C-tag CFI */
    uint8  stag_cfi;                        /**< [GB.GG.D2.TM] S-tag CFI */
    uint8  l2_type;                         /**< [GB.D2.TM] Layer 2 type, Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;                    /**< [GB.D2.TM] Layer 2 type */

    uint8  l3_type;                         /**< [GG.D2.TM] Layer 3 type, Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;                    /**< [GG.D2.TM] Layer 3 type mask, when care Layer 3 type should set 0xF */

    uint16 eth_type;                        /**< [GB.GG.D2.TM] Ethernet type */
    uint16 eth_type_mask;                   /**< [GB.GG.D2.TM] Ethernet type mask*/

    uint16 arp_op_code;                     /**< [GB.GG.D2.TM] Arp op code*/
    uint16 arp_op_code_mask;                /**< [GB.GG.D2.TM] Arp op code mask*/

    uint32 sender_ip;                       /**< [GB.GG.D2.TM] Sender ip */
    uint32 sender_ip_mask;                  /**< [GB.GG.D2.TM] Sender ip mask */
    uint32 target_ip;                       /**< [GB.GG.D2.TM] Target ip */
    uint32 target_ip_mask;                  /**< [GB.GG.D2.TM] Target ip mask */

    uint8  stag_valid;                      /**< [GB.GG.D2.TM] Stag valid */
    uint8  ctag_valid;                      /**< [GB.GG.D2.TM] Ctag valid */
    uint8  ipv4_packet;                     /**< [GB] Ipv4 packet */
    uint8  arp_packet;                      /**< [GB] Arp packet */

    uint8  ecn;                             /**< [GG.D2.TM] ecn*/
    uint8  ecn_mask;                        /**< [GG.D2.TM] ecn mask*/

    uint16 arp_protocol_type;               /**< [GG.D2.TM] arp protocol type*/
    uint16 arp_protocol_type_mask;          /**< [GG.D2.TM] arp protocol type mask*/

    uint32 mpls_label0;                     /**< [GG.D2.TM] MPLS label 0, include: Label + Exp + S + TTL */
    uint32 mpls_label0_mask;                /**< [GG.D2.TM] MPLS label 0 mask*/
    uint32 mpls_label1;                     /**< [GG.D2.TM] MPLS label 1, include: Label + Exp + S + TTL */
    uint32 mpls_label1_mask;                /**< [GG.D2.TM] MPLS label 1 mask*/
    uint32 mpls_label2;                     /**< [GG.D2.TM] MPLS label 2, include: Label + Exp + S + TTL */
    uint32 mpls_label2_mask;                /**< [GG.D2.TM] MPLS label 2 mask*/

    uint8  mpls_label_num;                  /**< [GG.D2.TM] MPLS label number*/
    uint8  mpls_label_num_mask;             /**< [GG.D2.TM] MPLS label number mask*/

    uint8 ethoam_level;                     /**< [GB.GG.D2.TM] Ethernet oam level*/
    uint8 ethoam_level_mask;                /**< [GB.GG.D2.TM] Ethernet oam level mask*/


    uint8 ethoam_op_code;                   /**< [GB.GG.D2.TM] Ethernet oam op code*/
    uint8 ethoam_op_code_mask;              /**< [GB.GG.D2.TM] Ethernet oam op code mask*/







    uint8 gre_crks;                         /**< [GG] GRE C R K S 4 bits flags bitmap*/
    uint8 gre_crks_mask;                    /**< [GG] GRE C R K S 4 bits flags bitmap mask*/

    uint32 gre_key;                         /**< [GG.D2.TM] GRE key*/
    uint32 gre_key_mask;                    /**< [GG.D2.TM] GRE key mask*/

    uint32 metadata;                        /**< [GG.D2.TM] Metadata */
    uint32 metadata_mask;                   /**< [GG.D2.TM] Metadata mask*/

    uint32 vni;                             /**< [GG.D2.TM] share with l4 src port and dst port*/
    uint32 vni_mask;                        /**< [GG.D2.TM] share with l4 src port and dst port mask*/

    uint8 udf[CTC_ACL_UDF_BYTE_NUM];        /**< [GG] udf, bytes in packet combined */
    uint8 udf_mask[CTC_ACL_UDF_BYTE_NUM];   /**< [GG] udf mask*/

    uint8 udf_type;                         /**< [GG] udf type, refer ctc_parser_udf_type_t*/

    uint8 vlan_num;                         /**< [GG.D2.TM] vlan number */
    uint8 vlan_num_mask;                    /**< [GG.D2.TM] vlan number mask */

    ctc_field_port_t port;                  /**< [GB.GG.D2.TM] valid when group type CTC_ACL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */

};
typedef struct ctc_acl_ipv4_key_s  ctc_acl_ipv4_key_t;

/**
@brief  Acl Mpls Key Flag
*/
enum ctc_acl_mpls_key_flag_e
{
    CTC_ACL_MPLS_KEY_FLAG_MAC_DA        = 1U << 0,   /**< [GB.GG.D2.TM] Valid MAC-DA in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_MAC_SA        = 1U << 1,   /**< [GB.GG.D2.TM] Valid MAC-SA in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_CVLAN         = 1U << 3,   /**< [GB.GG.D2.TM] Valid C-VLAN in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_CTAG_COS      = 1U << 4,   /**< [GB.GG.D2.TM] Valid C-tag CoS in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_SVLAN         = 1U << 5,   /**< [GB.GG.D2.TM] Valid S-VLAN in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_STAG_COS      = 1U << 6,   /**< [GB.GG.D2.TM] Valid S-tag CoS in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL0   = 1U << 8,   /**< [GB.GG.D2.TM] Valid MPLS label 0 in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL1   = 1U << 9,   /**< [GB.GG.D2.TM] Valid MPLS label 1 in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL2   = 1U << 10,  /**< [GB.GG.D2.TM] Valid MPLS label 2 in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL3   = 1U << 11,  /**< [GB] Valid MPLS label 3 in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_ROUTED_PACKET = 1U << 12,  /**< [GB.GG.D2.TM] Valid Routed-packet in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_CTAG_CFI      = 1U << 13,  /**< [GB.D2.TM] Valid C-tag CoS in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_STAG_CFI      = 1U << 14,  /**< [GB.GG.D2.TM] Valid S-tag CoS in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_METADATA      = 1U << 15,  /**< [GG.D2.TM] Valid metadata in MPLS key */
    CTC_ACL_MPLS_KEY_FLAG_MPLS_LABEL_NUM = 1U << 16  /**< [GG.D2.TM] MPLS label number field*/
};
typedef enum ctc_acl_mpls_key_flag_e ctc_acl_mpls_key_flag_t;

/**
@brief Define ACl Mpls Key structure
*/
struct ctc_acl_mpls_key_s
{
    uint32 flag;                /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_MPLS_KEY_FLAG_XXX */

    mac_addr_t mac_da;          /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;     /**< [GB.GG.D2.TM] MAC-DA mask */
    mac_addr_t mac_sa;          /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;     /**< [GB.GG.D2.TM] MAC-SA mask */

    uint16 cvlan;               /**< [GB.GG.D2.TM] C-VLAN tag */
    uint16 cvlan_mask;          /**< [GB.GG.D2.TM] C-VLAN mask*/
    uint16 svlan;               /**< [GB.GG.D2.TM] S-VLAN tag */
    uint16 svlan_mask;          /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  vrange_grpid;        /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint16 cvlan_end;           /**< [GG.D2.TM] orignal max cvlan id need to do vlan range acl*/
    uint16 svlan_end;           /**< [GG.D2.TM] orignal max svlan id need to do vlan range acl*/

    uint8  ctag_cos;            /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;       /**< [GB.GG.D2.TM] C-tag CoS mask */
    uint8  stag_cos;            /**< [GB.GG.D2.TM] S-tag Cos */
    uint8  stag_cos_mask;       /**< [GB.GG.D2.TM] S-tag Cos mask */

    uint8  ctag_cfi;            /**< [GB.GG.D2.TM] C-tag Cfi */
    uint8  stag_cfi;            /**< [GB.GG.D2.TM] S-tag Cfi */
    uint8  routed_packet;       /**< [GB.GG.D2.TM] Routed packet */
    uint8  rsv0;


    uint32 mpls_label0;         /**< [GB.GG.D2.TM] MPLS label 0, include: Label + Exp + S + TTL */
    uint32 mpls_label0_mask;    /**< [GB.GG.D2.TM] MPLS label 0 mask*/
    uint32 mpls_label1;         /**< [GB.GG.D2.TM] MPLS label 1, include: Label + Exp + S + TTL */
    uint32 mpls_label1_mask;    /**< [GB.GG.D2.TM] MPLS label 1 mask*/
    uint32 mpls_label2;         /**< [GB.GG.D2.TM] MPLS label 2, include: Label + Exp + S + TTL */
    uint32 mpls_label2_mask;    /**< [GB.GG.D2.TM] MPLS label 2 mask*/
    uint32 mpls_label3;         /**< [GB] MPLS label 3, include: Label + Exp + S + TTL */
    uint32 mpls_label3_mask;    /**< [GB] MPLS label 3 mask*/

    uint8  mpls_label_num;      /**< [GG.D2.TM] MPLS label number*/
    uint8  mpls_label_num_mask; /**< [GG.D2.TM] MPLS label number mask*/

    uint32 metadata;            /**< [GG.D2.TM] Metadata */
    uint32 metadata_mask;       /**< [GG.D2.TM] Metadata mask*/

    ctc_field_port_t port;      /**< [GB.GG.D2.TM] valid when group type CTC_ACL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */

};
typedef struct ctc_acl_mpls_key_s  ctc_acl_mpls_key_t;

/**
@brief  Acl IPv6 Key Flag
*/
enum ctc_acl_ipv6_key_flag_e
{
    CTC_ACL_IPV6_KEY_FLAG_MAC_DA        = 1U << 0,  /**< [GB.GG.D2.TM] Valid MAC-DA in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_MAC_SA        = 1U << 1,  /**< [GB.GG.D2.TM] Valid MAC-SA in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_CVLAN         = 1U << 3,  /**< [GB.GG.D2.TM] Valid CVLAN in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_CTAG_COS      = 1U << 4,  /**< [GB.GG.D2.TM] Valid Ctag CoS in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_SVLAN         = 1U << 5,  /**< [GB.GG.D2.TM] Valid SVLAN in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_STAG_COS      = 1U << 6,  /**< [GB.GG.D2.TM] Valid Stag CoS in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_ETH_TYPE      = 1U << 7,  /**< [GB.GG] Valid Ether-type in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_L2_TYPE       = 1U << 8,  /**< [GB] Valid L2-type CoS in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_L3_TYPE       = 1U << 9,  /**< [GB.GG] Valid L3-type in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_IP_SA         = 1U << 10, /**< [GB.GG.D2.TM] Valid IPv6-SA in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_IP_DA         = 1U << 11, /**< [GB.GG.D2.TM] Valid IPv6-DA in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL   = 1U << 12, /**< [GB.GG.D2.TM] Valid L4-Protocol in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_DSCP          = 1U << 13, /**< [GB.GG.D2.TM] Valid DSCP in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_PRECEDENCE    = 1U << 14, /**< [GB.GG.D2.TM] Valid IP Precedence in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_IP_FRAG       = 1U << 15, /**< [GB.GG.D2.TM] Valid fragment in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_IP_OPTION     = 1U << 16, /**< [GB.GG.D2.TM] Valid IP option in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_ROUTED_PACKET = 1U << 17, /**< [GB.GG.D2.TM] Valid Routed-packet in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_FLOW_LABEL    = 1U << 19, /**< [GB.GG.D2.TM] Valid IPv6 flow label in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_CTAG_CFI      = 1U << 20, /**< [GB.GG.D2.TM] Valid C-tag Cfi in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_STAG_CFI      = 1U << 21, /**< [GB.GG.D2.TM] Valid S-tag Cfi in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_STAG_VALID    = 1U << 22, /**< [GB.GG.D2.TM] Valid Stag-valid in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID    = 1U << 23, /**< [GB.GG.D2.TM] Valid Ctag-valid in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_ECN           = 1U << 24, /**< [GG.D2.TM] Valid ECN in IPv6 key */
    CTC_ACL_IPV6_KEY_FLAG_METADATA      = 1U << 25, /**< [GG.D2.TM] Valid metadata in ipv6 key */
    CTC_ACL_IPV6_KEY_FLAG_UDF           = 1U << 26, /**< [GG.D2.TM] Valid udf in ipv6 key */
    CTC_ACL_IPV6_KEY_FLAG_VLAN_NUM      = 1U << 27, /**< [GG.D2.TM] Valid vlan number in ipv6 key */
    CTC_ACL_IPV6_KEY_FLAG_PKT_LEN_RANGE = 1U << 28, /**< [GG.D2.TM] Valid packet length range in ipv6 key */
    CTC_ACL_IPV6_KEY_FLAG_IP_HDR_ERROR  = 1U << 29  /**< [GG.D2.TM] Valid ip header error in ipv6 key */
};
typedef enum ctc_acl_ipv6_key_flag_e ctc_acl_ipv6_key_flag_t;


/**
@brief  Acl IPv6 Key Sub Flag
*/
enum ctc_acl_ipv6_key_sub_flag_e
{
    CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT  = 1U << 0, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP Valid L4 source port in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT  = 1U << 1, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP Valid L4 destination port in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_TCP_FLAGS    = 1U << 2, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP    Valid TCP flags in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_TYPE    = 1U << 3, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP   Valid ICMP type in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_CODE    = 1U << 4, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP   Valid ICMP code in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_KEY      = 1U << 5, /**< [GG.D2.TM] Depend on L4_PROTOCOL = GRE    Valid GRE key in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_VNI          = 1U << 6, /**< [GG.D2.TM] Depend on L4_PROTOCOL = UDP    Valid vni in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_NVGRE_KEY    = 1U << 7, /**< [GG.D2.TM] Depend on L4_PROTOCOL = NVGRE    Valid NVGRE key in IPv6 key */
    CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_CRKS     = 1U << 8   /**< [GG] Depend on L4_PROTOCOL = GRE, valid C R K S 4 bits in GRE header  in IPv6 key*/
};
typedef enum ctc_acl_ipv6_key_sub_flag_e ctc_acl_ipv6_key_sub_flag_t;


/**
@brief Define ACl IPv6 Key structure
*/
struct ctc_acl_ipv6_key_s
{
    uint32 flag;                 /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV6_KEY_FLAG_XXX */
    uint32 sub_flag;             /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV6_KEY_SUB_FLAG_XXX */

    ipv6_addr_t ip_sa;           /**< [GB.GG.D2.TM] IPv6-SA */
    ipv6_addr_t ip_sa_mask;      /**< [GB.GG.D2.TM] IPv6-SA mask */
    ipv6_addr_t ip_da;           /**< [GB.GG.D2.TM] IPv6-DA */
    ipv6_addr_t ip_da_mask;      /**< [GB.GG.D2.TM] IPv6-DA mask */

    uint16 l4_src_port_0;        /**< [GB.GG.D2.TM] Layer 4 source port.Min/Data */
    uint16 l4_src_port_1;        /**< [GB.GG.D2.TM] Layer 4 source port.Max/Mask */

    uint8  l4_src_port_use_mask; /**< [GB.GG.D2.TM] Use mask instead of range*/
    uint8  ip_option;            /**< [GB.GG.D2.TM] IP option */
    uint8  routed_packet;        /**< [GB.GG.D2.TM] Routed packet */
    uint8  l4_dst_port_use_mask; /**< [GB.GG.D2.TM] Use mask instead of range, dest_port is collision with l2_type and eth_type on GB*/

    uint16 l4_dst_port_0;        /**< [GB.GG.D2.TM] Layer 4 dest port */
    uint16 l4_dst_port_1;        /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint32 pkt_len_min;          /**< [GG.D2.TM] Packet length Min */
    uint32 pkt_len_max;          /**< [GG.D2.TM] Packet Length Max */

    uint8  ip_frag;              /**< [GB.GG.D2.TM] Ip fragment, CTC_IP_FRAG_XXX */
    uint8  tcp_flags_match_any;  /**< [GB.GG.D2.TM] 0:match all 1:match any*/
    uint8  tcp_flags;            /**< [GB.GG.D2.TM] Tcp flag bitmap*/
    uint8  ip_hdr_error;       /**< [GG.D2.TM] ip header error packet */

    uint32 flow_label;           /**< [GB.GG.D2.TM] Flow label */
    uint32 flow_label_mask;      /**< [GB.GG.D2.TM] Flow label */

    uint8  l4_protocol;          /**< [GB.GG.D2.TM] Layer 4 protocol */
    uint8  l4_protocol_mask;     /**< [GB.GG.D2.TM] Layer 4 protocol mask*/
    uint8  icmp_type;            /**< [GB.GG.D2.TM] ICMP type */
    uint8  icmp_type_mask;       /**< [GB.GG.D2.TM] ICMP type mask*/

    uint8  icmp_code;            /**< [GB.GG.D2.TM] ICMP code */
    uint8  icmp_code_mask;       /**< [GB.GG.D2.TM] ICMP code mask*/
    uint8  dscp;                 /**< [GB.GG.D2.TM] DSCP */
    uint8  dscp_mask;            /**< [GB.GG.D2.TM] DSCP mask */

    mac_addr_t mac_sa;           /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;      /**< [GB.GG.D2.TM] MAC-SA mask */
    mac_addr_t mac_da;           /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;      /**< [GB.GG.D2.TM] MAC-DA mask */

    uint16 eth_type;             /**< [GB.GG.D2.TM] Ethernet type */
    uint16 eth_type_mask;        /**< [GB.GG.D2.TM] Ethernet type mask */

    uint16 cvlan;                /**< [GB.GG.D2.TM] C-VLAN */
    uint16 cvlan_mask;           /**< [GB.GG.D2.TM] C-VLAN mask*/

    uint16 svlan;                /**< [GB.GG.D2.TM] S-VLAN */
    uint16 svlan_mask;           /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  vrange_grpid;         /**< [GG.D2.TM] vlan range group id: 0~63 used in vlan range check*/
    uint16 cvlan_end;            /**< [GG.D2.TM] orignal max cvlan id need to do vlan range acl*/
    uint16 svlan_end;            /**< [GG.D2.TM] orignal max svlan id need to do vlan range acl*/

    uint8  ctag_cos;             /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;        /**< [GB.GG.D2.TM] C-tag CoS mask */
    uint8  stag_cos;             /**< [GB.GG.D2.TM] S-tag CoS */
    uint8  stag_cos_mask;        /**< [GB.GG.D2.TM] S-tag Cos mask */

    uint8  ctag_cfi;             /**< [GB.GG.D2.TM] C-tag CFI */
    uint8  stag_cfi;             /**< [GB.GG.D2.TM] S-tag CFI */
    uint8  l2_type;              /**< [GB] Layer 2 type Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;         /**< [GB] Layer 2 type mask*/


    uint8  stag_valid;           /**< [GB.GG.D2.TM] Stag valid*/
    uint8  ctag_valid;           /**< [GB.GG.D2.TM] Ctag valid*/
    uint8  l3_type;              /**< [GB.GG.D2.TM] Layer 3 type Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;         /**< [GB.GG.D2.TM] Layer 3 type mask*/

    uint8  ecn;                  /**< [GG.D2.TM] ecn*/
    uint8  ecn_mask;             /**< [GG.D2.TM] ecn mask*/

    uint8 gre_crks;              /**< [GG] GRE C R K S 4 bits flags bitmap*/
    uint8 gre_crks_mask;         /**< [GG] GRE C R K S 4 bits flags bitmap mask*/

    uint32 gre_key;              /**< [GG.D2.TM] share with l4 src port and dst port*/
    uint32 gre_key_mask;         /**< [GG.D2.TM] share with l4 src port and dst port mask*/

    uint32 metadata;             /**< [GG.D2.TM] metadata */
    uint32 metadata_mask;        /**< [GG.D2.TM] metadata mask*/

    uint32 vni;                  /**< [GG.D2.TM] share with l4 src port and dst port*/
    uint32 vni_mask;             /**< [GG.D2.TM] share with l4 src port and dst port mask*/

    uint8 udf[CTC_ACL_UDF_BYTE_NUM];          /**< [GG] udf, bytes in packet combined */
    uint8 udf_mask[CTC_ACL_UDF_BYTE_NUM];     /**< [GG] udf mask*/

    uint8 udf_type;                           /**< [GG] udf type, refer ctc_parser_udf_type_t*/

    uint8 vlan_num;                           /**< [GG.D2.TM] vlan number */
    uint8 vlan_num_mask;                      /**< [GG.D2.TM] vlan number mask */

    ctc_field_port_t port;                    /**< [GB.GG.D2.TM] valid when group type CTC_ACL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */

};
typedef struct ctc_acl_ipv6_key_s ctc_acl_ipv6_key_t;

/**
@brief  Acl Pbr IPv4 Key Flag
*/
enum ctc_acl_pbr_ipv4_key_flag_e
{
    CTC_ACL_PBR_IPV4_KEY_FLAG_IP_SA         = 1U << 0, /**< [GB.GG.D2.TM] Valid IP-SA in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_FLAG_IP_DA         = 1U << 1, /**< [GB.GG.D2.TM] Valid IP-DA in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_FLAG_L4_PROTOCOL   = 1U << 2, /**< [GB.GG.D2.TM] Valid L4-Proto in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_FLAG_DSCP          = 1U << 3, /**< [GB.GG.D2.TM] Valid DSCP in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_FLAG_PRECEDENCE    = 1U << 4, /**< [GB.GG.D2.TM] Valid IP Precedence in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_FLAG_IP_FRAG       = 1U << 5, /**< [GB.GG.D2.TM] Valid fragment in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_FLAG_VRFID         = 1U << 6  /**< [GB.GG.D2.TM] Valid vrfid in IPv4 key */
};
typedef enum ctc_acl_pbr_ipv4_key_flag_e ctc_acl_pbr_ipv4_key_flag_t;

/**
@brief  Acl Pbr IPv4 Key Sub Flag
*/
enum ctc_acl_pbr_ipv4_key_sub_flag_e
{
    CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_L4_SRC_PORT     = 1U << 0, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP  Valid L4 source port in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_L4_DST_PORT     = 1U << 1, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP  Valid L4 destination port in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_TCP_FLAGS       = 1U << 2, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP Valid TCP flag in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_TYPE       = 1U << 3, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP Valid ICMP type in IPv4 key */
    CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_ICMP_CODE       = 1U << 4  /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP Valid ICMP code in IPv4 key */
};
typedef enum ctc_acl_pbr_ipv4_key_sub_flag_e ctc_acl_pbr_ipv4_key_sub_flag_t;


/**
@brief Define ACl Pbr IPv4 Key structure
*/
struct ctc_acl_pbr_ipv4_key_s
{
    uint32 flag;                        /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_PBR_IPV4_KEY_FLAG_XXX */
    uint32 sub_flag;                    /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_PBR_IPV4_KEY_SUB_FLAG_XXX */

    uint32 ip_sa;                       /**< [GB.GG.D2.TM] IP-SA */
    uint32 ip_sa_mask;                  /**< [GB.GG.D2.TM] IP-SA mask */
    uint32 ip_da;                       /**< [GB.GG.D2.TM] IP-DA */
    uint32 ip_da_mask;                  /**< [GB.GG.D2.TM] IP-DA mask */

    uint16 l4_src_port_0;               /**< [GB.GG.D2.TM] Layer 4 source port.Min/Data */
    uint16 l4_src_port_1;               /**< [GB.GG.D2.TM] Layer 4 source port.Max/Mask */

    uint8  l4_src_port_use_mask;        /**< [GB.GG.D2.TM] Use mask instead of range*/
    uint8  l4_dst_port_use_mask;        /**< [GB.GG.D2.TM] Use mask instead of range*/
    uint8  ip_frag;                     /**< [GB.GG.D2.TM] Ip fragment, CTC_IP_FRAG_XXX */
    uint8  rsv1;

    uint16 l4_dst_port_0;               /**< [GB.GG.D2.TM] Layer 4 dest port */
    uint16 l4_dst_port_1;               /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint8  tcp_flags_match_any;         /**< [GB.GG.D2.TM] 0:match all. 1:match any*/
    uint8  tcp_flags;                   /**< [GB.GG.D2.TM] Tcp flag bitmap*/
    uint8  dscp;                        /**< [GB.GG.D2.TM] DSCP */
    uint8  dscp_mask;                   /**< [GB.GG.D2.TM] DSCP mask*/

    uint8  l4_protocol;                 /**< [GB.GG.D2.TM] Layer 4 protocol */
    uint8  l4_protocol_mask;            /**< [GB.GG.D2.TM] Layer 4 protocol mask*/
    uint8  rsv2[2];

    uint8  icmp_type;                   /**< [GB.GG.D2.TM] ICMP type */
    uint8  icmp_type_mask;              /**< [GB.GG.D2.TM] ICMP type mask*/
    uint8  icmp_code;                   /**< [GB.GG.D2.TM] ICMP code */
    uint8  icmp_code_mask;              /**< [GB.GG.D2.TM] ICMP code mask*/

    uint16 vrfid;                       /**< [GB.GG.D2.TM] VRFID */
    uint16 vrfid_mask;                  /**< [GB.GG.D2.TM] VRFID mask*/

    ctc_field_port_t port;              /**< [GB.GG.D2.TM] valid when group type CTC_ACL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
};
typedef struct ctc_acl_pbr_ipv4_key_s  ctc_acl_pbr_ipv4_key_t;

/**
@brief  Acl Pbr IPv6 Key Flag
*/
enum ctc_acl_pbr_ipv6_key_flag_e
{
    CTC_ACL_PBR_IPV6_KEY_FLAG_MAC_DA        = 1U << 0,  /**< [GB] Valid MAC-DA in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_MAC_SA        = 1U << 1,  /**< [GB] Valid MAC-SA in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_CVLAN         = 1U << 2,  /**< [GB] Valid CVLAN in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_CTAG_COS      = 1U << 3,  /**< [GB] Valid Ctag CoS in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_SVLAN         = 1U << 4,  /**< [GB] Valid SVLAN in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_STAG_COS      = 1U << 5,  /**< [GB] Valid Stag CoS in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_ETH_TYPE      = 1U << 6,  /**< [GB] Valid Ether-type in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_L2_TYPE       = 1U << 7,  /**< [GB] Valid L2-type CoS in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_L3_TYPE       = 1U << 8,  /**< [GB] Valid L3-type in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_IP_SA         = 1U << 9,  /**< [GB.GG.D2.TM] Valid IPv6-SA in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_IP_DA         = 1U << 10, /**< [GB.GG.D2.TM] Valid IPv6-DA in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_L4_PROTOCOL   = 1U << 11, /**< [GB.GG.D2.TM] Valid L4-Protocol in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_DSCP          = 1U << 12, /**< [GB.GG.D2.TM] Valid DSCP in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_PRECEDENCE    = 1U << 13, /**< [GB.GG.D2.TM] Valid IP Precedence in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_IP_FRAG       = 1U << 14, /**< [GB.GG.D2.TM] Valid fragment in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_FLOW_LABEL    = 1U << 15, /**< [GB.GG.D2.TM] Valid IPv6 flow label in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_CTAG_CFI      = 1U << 16, /**< [GB] Valid C-tag Cfi in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_STAG_CFI      = 1U << 17, /**< [GB] Valid S-tag Cfi in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_FLAG_VRFID         = 1U << 18, /**< [GB.GG.D2.TM] Valid vrfid in IPv6 key */
};
typedef enum ctc_acl_pbr_ipv6_key_flag_e ctc_acl_pbr_ipv6_key_flag_t;

/**
@brief  Acl Pbr IPv6 Key Sub Flag
*/
enum ctc_acl_pbr_ipv6_key_sub_flag_e
{
    CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_L4_SRC_PORT   = 1U << 0,  /**< [GB.GG] Depend on L4_PROTOCOL = TCP|UDP Valid L4 source port in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_L4_DST_PORT   = 1U << 1,  /**< [GB.GG] Depend on L4_PROTOCOL = TCP|UDP Valid L4 destination port in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_TCP_FLAGS     = 1U << 2,  /**< [GB.GG] Depend on L4_PROTOCOL = TCP.    Valid TCP flags in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_TYPE     = 1U << 3,  /**< [GB.GG] Depend on L4_PROTOCOL = ICMP.   Valid ICMP type in IPv6 key */
    CTC_ACL_PBR_IPV6_KEY_SUB_FLAG_ICMP_CODE     = 1U << 4   /**< [GB.GG] Depend on L4_PROTOCOL = ICMP.   Valid ICMP code in IPv6 key */
};
typedef enum ctc_acl_pbr_ipv6_key_sub_flag_e ctc_acl_pbr_ipv6_key_sub_flag_t;


/**
@brief Define ACl Pbr IPv6 Key structure
*/
struct ctc_acl_pbr_ipv6_key_s
{
    uint32 flag;                        /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV6_KEY_FLAG_XXX */
    uint32 sub_flag;                    /**< [GB.GG.D2.TM] Bitmap of CTC_ACL_IPV6_KEY_SUB_FLAG_XXX */

    ipv6_addr_t ip_sa;                  /**< [GB.GG.D2.TM] IPv6-SA */
    ipv6_addr_t ip_sa_mask;             /**< [GB.GG.D2.TM] IPv6-SA mask */
    ipv6_addr_t ip_da;                  /**< [GB.GG.D2.TM] IPv6-DA */
    ipv6_addr_t ip_da_mask;             /**< [GB.GG.D2.TM] IPv6-DA mask */

    uint16 l4_src_port_0;               /**< [GB.GG.D2.TM] Layer 4 source port.Min/Data */
    uint16 l4_src_port_1;               /**< [GB.GG.D2.TM] Layer 4 source port.Max/Mask */

    uint8  l4_src_port_use_mask;        /**< [GB.GG.D2.TM] Use mask instead of range*/
    uint8  l4_dst_port_use_mask;        /**< [GB.GG.D2.TM] Use mask instead of range. dest_port is collision with l2_type and eth_type on GB.*/
    uint8  l3_type;                     /**< [GB] Layer 3 type. Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;                /**< [GB] Layer 3 type mask*/

    uint16 l4_dst_port_0;               /**< [GB.GG.D2.TM] Layer 4 dest port */
    uint16 l4_dst_port_1;               /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint8  ip_frag;                     /**< [GB.GG.D2.TM] Ip fragment, CTC_IP_FRAG_XXX */
    uint8  tcp_flags_match_any;         /**< [GB.GG.D2.TM] 0:match all. 1:match any*/
    uint8  tcp_flags;                   /**< [GB.GG.D2.TM] Tcp flag bitmap*/
    uint8  rsv0;

    uint32 flow_label;                  /**< [GB.GG.D2.TM] Flow label */
    uint32 flow_label_mask;             /**< [GB.GG.D2.TM] Flow label */

    uint8  l4_protocol;                 /**< [GB.GG.D2.TM] Layer 4 protocol */
    uint8  l4_protocol_mask;            /**< [GB.GG.D2.TM] Layer 4 protocol mask*/
    uint8  icmp_type;                   /**< [GB.GG.D2.TM] ICMP type */
    uint8  icmp_type_mask;              /**< [GB.GG.D2.TM] ICMP type mask*/

    uint8  icmp_code;                   /**< [GB.GG.D2.TM] ICMP code */
    uint8  icmp_code_mask;              /**< [GB.GG.D2.TM] ICMP code mask*/
    uint8  dscp;                        /**< [GB.GG.D2.TM] DSCP */
    uint8  dscp_mask;                   /**< [GB.GG.D2.TM] DSCP mask */

    mac_addr_t mac_sa;                  /**< [GB.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;             /**< [GB.D2.TM] MAC-SA mask */
    mac_addr_t mac_da;                  /**< [GB.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;             /**< [GB.D2.TM] MAC-DA mask */

    uint16 eth_type;                    /**< [GB] Ethernet type */
    uint16 eth_type_mask;               /**< [GB] Ethernet type mask */

    uint16 cvlan;                       /**< [GB] C-VLAN */
    uint16 cvlan_mask;                  /**< [GB] C-VLAN mask*/

    uint16 svlan;                       /**< [GB] S-VLAN */
    uint16 svlan_mask;                  /**< [GB] S-VLAN mask*/

    uint8  ctag_cos;                    /**< [GB] C-tag CoS */
    uint8  ctag_cos_mask;               /**< [GB] C-tag CoS mask */
    uint8  stag_cos;                    /**< [GB] S-tag CoS */
    uint8  stag_cos_mask;               /**< [GB] S-tag Cos mask */

    uint8  ctag_cfi;                    /**< [GB] C-tag CFI */
    uint8  stag_cfi;                    /**< [GB] S-tag CFI */
    uint8  l2_type;                     /**< [GB] Layer 2 type. Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;                /**< [GB] Layer 2 type mask*/


    uint16 vrfid;                       /**< [GB.GG.D2.TM] VRFID */
    uint16 vrfid_mask;                  /**< [GB.GG.D2.TM] VRFID mask*/

    ctc_field_port_t port;              /**< [GB.GG.D2.TM] valid when group type CTC_ACL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
};
typedef struct ctc_acl_pbr_ipv6_key_s ctc_acl_pbr_ipv6_key_t;

/**
@brief Define ACl Key structure
*/
struct ctc_acl_key_s
{
    ctc_acl_key_type_t type;                        /**< [GB.GG.D2.TM] ctc_acl_key_type_t */

    union
    {
        ctc_acl_mac_key_t  mac_key;                 /**< [GB.GG.D2.TM] ACL MAC key content */
        ctc_acl_mpls_key_t mpls_key;                /**< [GB.GG.D2.TM] ACL MPLS key content */
        ctc_acl_ipv4_key_t ipv4_key;                /**< [GB.GG.D2.TM] ACL IPv4 key content */
        ctc_acl_ipv6_key_t ipv6_key;                /**< [GB.GG.D2.TM] ACL IPv6 key content */
        ctc_acl_hash_mac_key_t  hash_mac_key;       /**< [GB.GG.D2.TM] ACL MAC Hash key content */
        ctc_acl_hash_ipv4_key_t hash_ipv4_key;      /**< [GB.GG.D2.TM] ACL IPv4 Hash key content */
        ctc_acl_hash_ipv6_key_t hash_ipv6_key;      /**< [GG.D2.TM] ACL IPv6 Hash key content */
        ctc_acl_hash_mpls_key_t hash_mpls_key;      /**< [GG.D2.TM] ACL MPLS Hash key content */
        ctc_acl_hash_l2_l3_key_t hash_l2_l3_key;    /**< [GG.D2.TM] ACL L2 L3 key content */
        ctc_acl_pbr_ipv4_key_t pbr_ipv4_key;        /**< [GB.GG.D2.TM] ACL PBR IPv4 key content */
        ctc_acl_pbr_ipv6_key_t pbr_ipv6_key;        /**< [GB.GG.D2.TM] ACL PBR IPv6 key content */
    } u;
};
typedef struct ctc_acl_key_s ctc_acl_key_t;

/**
@brief  Acl Action Flag
*/
enum ctc_acl_action_flag_e
{
    CTC_ACL_ACTION_FLAG_DISCARD                        = 1U << 0,  /**< [GB.GG.D2.TM] Discard the packet i:ingress e:egress*/
    CTC_ACL_ACTION_FLAG_DENY_BRIDGE                    = 1U << 1,  /**< [GB.GG.D2.TM] Deny bridging process */
    CTC_ACL_ACTION_FLAG_DENY_LEARNING                  = 1U << 2,  /**< [GB.GG.D2.TM] Deny learning process */
    CTC_ACL_ACTION_FLAG_DENY_ROUTE                     = 1U << 3,  /**< [GB.GG.D2.TM] Deny routing process */
    CTC_ACL_ACTION_FLAG_STATS                          = 1U << 6,  /**< [GB.GG.D2.TM] Statistic */
    CTC_ACL_ACTION_FLAG_PRIORITY_AND_COLOR             = 1U << 8,  /**< [GB.GG.D2.TM] Priority color */
    CTC_ACL_ACTION_FLAG_TRUST                          = 1U << 9,  /**< [GB.GG] QoS trust state */
    CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER             = 1U << 10, /**< [GB.GG.D2.TM] Micro Flow policer */
    CTC_ACL_ACTION_FLAG_RANDOM_LOG                     = 1U << 11, /**< [GB.GG.D2.TM] Log to any network port */
    CTC_ACL_ACTION_FLAG_COPY_TO_CPU                    = 1U << 12, /**< [GB.GG.D2.TM] Copy to cpu */
    CTC_ACL_ACTION_FLAG_REDIRECT                       = 1U << 13, /**< [GB.GG.D2.TM] Redirect */
    CTC_ACL_ACTION_FLAG_DSCP                           = 1U << 14, /**< [GB.GG.D2.TM] Dscp */
    CTC_ACL_ACTION_FLAG_COPY_TO_CPU_WITH_TIMESTAMP     = 1U << 15, /**< [GB] Copy to cpu with timestamp */
    CTC_ACL_ACTION_FLAG_QOS_DOMAIN                     = 1U << 16, /**< [GB.GG] Qos domain */
    CTC_ACL_ACTION_FLAG_MACRO_FLOW_POLICER             = 1U << 17, /**< [GB.GG.D2.TM] Macro Flow Policer */
    CTC_ACL_ACTION_FLAG_AGING                          = 1U << 18, /**< [GB] Aging */
    CTC_ACL_ACTION_FLAG_VLAN_EDIT                      = 1U << 19, /**< [GB.GG.D2.TM] Vlan edit */
    CTC_ACL_ACTION_FLAG_PRIORITY                       = 1U << 26, /**< [GG.D2.TM] Priority*/
    CTC_ACL_ACTION_FLAG_COLOR                          = 1U << 27, /**< [GG.D2.TM] Color id */
    CTC_ACL_ACTION_FLAG_DISABLE_IPFIX                  = 1U << 28, /**< [GG.D2.TM] Disable ipfix*/
    CTC_ACL_ACTION_FLAG_DENY_IPFIX_LEARNING            = 1U << 29, /**< [GG.D2.TM] Deny ipfix learning*/
    CTC_ACL_ACTION_FLAG_POSTCARD_EN                    = 1U << 30, /**< [GG] Enable postcard*/
    CTC_ACL_ACTION_FLAG_CANCEL_DISCARD                 = 1U << 31, /**< [GG.D2.TM] Cancel discard the packet */
    CTC_ACL_ACTION_FLAG_ASSIGN_OUTPUT_PORT             = 1LLU << 32, /**<[GG] DestId assigned by user */
    CTC_ACL_ACTION_FLAG_FORCE_ROUTE                    = 1LLU << 33,  /**< [GG] Force routing process,will cancel deny route,deny bridge */
    CTC_ACL_ACTION_FLAG_METADATA                       = 1LLU << 34, /**< [GG.D2.TM] Metadata */
    CTC_ACL_ACTION_FLAG_FID                            = 1LLU << 35, /**< [GG] fid */
    CTC_ACL_ACTION_FLAG_VRFID                          = 1LLU << 36,  /**< [GG] Vrf ID of the route */
    CTC_ACL_ACTION_FLAG_REDIRECT_WITH_RAW_PKT          = 1LLU << 37,  /**< [GG.D2.TM] Packet will do redirect with raw info */
    /*SYSTEM MODIFIED by xgu for SDK bug36147, system bug 34415, 2015-11-18 */
    CTC_ACL_ACTION_FLAG_FORCE_LEARNING                 = 1LLU << 38,  /**< [GG] Force Learning */

    /* following is pbr action flag */
    CTC_ACL_ACTION_FLAG_PBR_TTL_CHECK                  = 1U << 20, /**< [GB.GG] Set PBR ttl-check flag */
    CTC_ACL_ACTION_FLAG_PBR_ICMP_CHECK                 = 1U << 21, /**< [GB.GG] Set PBR icmp-check flag */
    CTC_ACL_ACTION_FLAG_PBR_CPU                        = 1U << 22, /**< [GB.GG.D2.TM] Set PBR copy-to-cpu flag */
    CTC_ACL_ACTION_FLAG_PBR_ECMP                       = 1U << 23, /**< [GB] Set PBR ecmp flag which means use ecmp nexthop directly */
    CTC_ACL_ACTION_FLAG_PBR_FWD                        = 1U << 24, /**< [GB.GG.D2.TM] Set PBR fwd flag */
    CTC_ACL_ACTION_FLAG_PBR_DENY                       = 1U << 25  /**< [GB.GG.D2.TM] Set PBR deny flag */
};
typedef enum ctc_acl_action_flag_e ctc_acl_action_flag_t;

/**
@brief  Define Log Percent
*/
enum ctc_log_percent_e
{
    CTC_LOG_PERCENT_POWER_NEGATIVE_15,     /**< [GB] 2 (-15) */
    CTC_LOG_PERCENT_POWER_NEGATIVE_14,     /**< [GB.GG.D2.TM] 2 (-14) */
    CTC_LOG_PERCENT_POWER_NEGATIVE_13,     /**< [GB.GG.D2.TM] 2 (-13) */
    CTC_LOG_PERCENT_POWER_NEGATIVE_12,     /**< [GB.GG.D2.TM] 2 (-12) */
    CTC_LOG_PERCENT_POWER_NEGATIVE_11,     /**< [GB.GG.D2.TM] 2 (-11) */
    CTC_LOG_PERCENT_POWER_NEGATIVE_10,     /**< [GB.GG.D2.TM] 2 (-10) */
    CTC_LOG_PERCENT_POWER_NEGATIVE_9,      /**< [GB.GG.D2.TM] 2 (-9)  */
    CTC_LOG_PERCENT_POWER_NEGATIVE_8,      /**< [GB.GG.D2.TM] 2 (-8)  */
    CTC_LOG_PERCENT_POWER_NEGATIVE_7,      /**< [GB.GG.D2.TM] 2 (-7)  */
    CTC_LOG_PERCENT_POWER_NEGATIVE_6,      /**< [GB.GG.D2.TM] 2 (-6)  */
    CTC_LOG_PERCENT_POWER_NEGATIVE_5,      /**< [GB.GG.D2.TM] 2 (-5) : */
    CTC_LOG_PERCENT_POWER_NEGATIVE_4,      /**< [GB.GG.D2.TM] 2 (-4) : 1/16*/
    CTC_LOG_PERCENT_POWER_NEGATIVE_3,      /**< [GB.GG.D2.TM] 2 (-3) : 1/8 */
    CTC_LOG_PERCENT_POWER_NEGATIVE_2,      /**< [GB.GG.D2.TM] 2 (-2) : 1/4   */
    CTC_LOG_PERCENT_POWER_NEGATIVE_1,      /**< [GB.GG.D2.TM] 2 (-1) : 1/2   */
    CTC_LOG_PERCENT_POWER_NEGATIVE_0,      /**< [GB.GG.D2.TM] 2 (0)  : 1/1  */
    CTC_LOG_PERCENT_MAX                    /**< [GB.GG.D2.TM] max number  */
} ;
typedef enum ctc_log_percent_e ctc_log_percent_t;

/**
@brief Define ACl Query mode
*/
enum ctc_acl_query_mode_e
{
    CTC_ACL_QUERY_MODE_GROUPID,      /**< [GB.GG.D2.TM] query by groupid*/
    CTC_ACL_QUERY_MODE_LKUP_LEVEL,   /**< [D2.TM] query by lkup level*/
    CTC_ACL_QUERY_MODE_MAX
};
typedef enum ctc_acl_query_mode_e ctc_acl_query_mode_t;

/**
@brief Define ACl Query structure
*/
struct ctc_acl_query_s
{
    uint8   query_mode; /**< [D2.TM] (in)     query mode, refer to ctc_acl_query_mode_t*/
    uint8   lkup_level; /**< [D2.TM] (in)     ACL lkup level*/
    uint8   dir;        /**< [D2.TM] (in)     direction*/
    uint32  group_id;   /**< [GB.GG.D2.TM] (in)     ACL group ID*/
    uint32  entry_size; /**< [GB.GG.D2.TM] (in)     The maximum number of entry IDs to return. If 0, the number of entries is returned*/
    uint32* entry_array;/**< [GB.GG.D2.TM] (in|out) A pointer to a memory buffer to hold the array of IDs*/
    uint32  entry_count;/**< [GB.GG.D2.TM] (out)    Number of entries returned or if entry_size is 0, the number of entries available*/
};
typedef struct ctc_acl_query_s ctc_acl_query_t;


/**
 @brief [GB] define acl vlan tag operation
*/

enum ctc_acl_vlan_tag_op_e
{
    CTC_ACL_VLAN_TAG_OP_NONE,        /**< [GB.GG.D2.TM] do nothing*/
    CTC_ACL_VLAN_TAG_OP_REP,         /**< [GB.GG.D2.TM] replace tag only for packet tagged*/
    CTC_ACL_VLAN_TAG_OP_ADD,         /**< [GB.GG.D2.TM] append a new tag even if packet already has tag*/
    CTC_ACL_VLAN_TAG_OP_DEL,         /**< [GB.GG.D2.TM] delete packet's tag only for packet tagged*/
    CTC_ACL_VLAN_TAG_OP_REP_OR_ADD,  /**< [GG.D2.TM] replace for tagged, add for no stag */
    CTC_ACL_VLAN_TAG_OP_MAX
};
typedef enum ctc_acl_vlan_tag_op_e ctc_acl_vlan_tag_op_t;



/**
 @brief [GB] define acl vid or cos or cfi operation
*/

enum ctc_acl_vlan_tag_sl_e
{
    CTC_ACL_VLAN_TAG_SL_NONE,               /**< [GB.GG.D2.TM] do nothing*/
    CTC_ACL_VLAN_TAG_SL_ALTERNATIVE  = 0x1, /**< [GB.GG.D2.TM] select the other tag's vid/cos, if the other tag not present, use default*/
    CTC_ACL_VLAN_TAG_SL_NEW          = 0x2, /**< [GB.GG.D2.TM] select user assigned vid/cos */
    CTC_ACL_VLAN_TAG_SL_MAP          = 0x3, /**< [D2.TM] select mapped scos from QoS table map*/
    CTC_ACL_VLAN_TAG_SL_MAX
};
typedef enum ctc_acl_vlan_tag_sl_e ctc_acl_vlan_tag_sl_t;



/**
 @brief [GB] define acl vlan edit operation
*/
struct ctc_acl_vlan_edit_s
{
    uint8  stag_op;         /**< [GB.GG.D2.TM] operation type of stag, see ctc_acl_vlan_tag_op_t  */
    uint8  svid_sl;         /**< [GB.GG.D2.TM] select type of svid, see ctc_acl_vlan_tag_sl_t  */
    uint8  scos_sl;         /**< [GB.GG.D2.TM] select type of scos, see ctc_acl_vlan_tag_sl_t  */
    uint8  scfi_sl;         /**< [GB.GG.D2.TM] select type of scfi, see ctc_acl_vlan_tag_sl_t  */
    uint16 svid_new;        /**< [GB.GG.D2.TM] new svid, valid only if svid_sl is CTC_ACL_VLAN_TAG_OP_REP/CTC_ACL_VLAN_TAG_OP_ADD*/
    uint8  scos_new;        /**< [GB.GG.D2.TM] new scos, valid only if scos_sl is CTC_ACL_VLAN_TAG_OP_REP/CTC_ACL_VLAN_TAG_OP_ADD*/
    uint8  scfi_new;        /**< [GB.GG.D2.TM] new scfi, valid only if scfi_sl is CTC_ACL_VLAN_TAG_OP_REP/CTC_ACL_VLAN_TAG_OP_ADD*/

    uint8  ctag_op;         /**< [GB.GG.D2.TM] operation type of ctag, see ctc_acl_vlan_tag_op_t */
    uint8  cvid_sl;         /**< [GB.GG.D2.TM] select type of cvid, see ctc_acl_vlan_tag_sl_t */
    uint8  ccos_sl;         /**< [GB.GG.D2.TM] select type of ccos, see ctc_acl_vlan_tag_sl_t */
    uint8  ccfi_sl;         /**< [GB.GG.D2.TM] select type of ccfi, see ctc_acl_vlan_tag_sl_t */
    uint16 cvid_new;        /**< [GB.GG.D2.TM] new cvid, valid only if cvid_sl is CTC_ACL_VLAN_TAG_OP_REP/CTC_ACL_VLAN_TAG_OP_ADD*/
    uint8  ccos_new;        /**< [GB.GG.D2.TM] new ccos, valid only if ccos_sl is CTC_ACL_VLAN_TAG_OP_REP/CTC_ACL_VLAN_TAG_OP_ADD*/
    uint8  ccfi_new;        /**< [GB.GG.D2.TM] new ccfi, valid only if ccfi_sl is CTC_ACL_VLAN_TAG_OP_REP/CTC_ACL_VLAN_TAG_OP_ADD*/
    uint8  tpid_index;      /**< [GB.GG.D2.TM] svlan tpid index*/
};
typedef struct ctc_acl_vlan_edit_s ctc_acl_vlan_edit_t;

/**
 @brief [GG] define acl start packet strip
*/
enum ctc_acl_start_packet_strip_e
{
    CTC_ACL_STRIP_NONE,          /**< [GG] do nothing*/
    CTC_ACL_STRIP_START_TO_L2,   /**< [GG] Remove the start-of-packet up to the L2 header. */
    CTC_ACL_STRIP_START_TO_L3,   /**< [GG] Remove the start-of-packet up to the L3 header. */
    CTC_ACL_STRIP_START_TO_L4,   /**< [GG] Remove the start-of-packet up to the L4 header. */
    CTC_ACL_STRIP_MAX
};
typedef enum ctc_acl_start_packet_strip_e ctc_acl_start_packet_strip_t;

/**
 @brief [GG] define acl packet strip
*/
struct ctc_acl_packet_strip_s
{
    uint8  start_packet_strip;  /**< [GG.D2.TM] strip start type. CTC_ACL_STRIP_START_TO_XX */
    uint8  packet_type;         /**< [GG.D2.TM] payload packet type. PKT_TYPE_XXX */
    uint8  strip_extra_len;     /**< [GG.D2.TM] strip extra length, unit: byte*/
};
typedef struct ctc_acl_packet_strip_s ctc_acl_packet_strip_t;

/**
 @brief define acl action structure
*/
struct ctc_acl_action_s
{
    uint64 flag;                /**< [GB.GG.D2.TM] Bitmap of ctc_acl_action_flag_t */

    uint32 nh_id;               /**< [GB.GG.D2.TM] Forward nexthop ID, shared by acl, qos and pbr*/
    uint32 stats_id;            /**< [GB.GG.D2.TM] Stats id*/
    uint16 micro_policer_id;    /**< [GB.GG.D2.TM] Micro flow policer ID */
    uint16 macro_policer_id;    /**< [GB.GG.D2.TM] Macro flow policer ID */
    uint8  trust;               /**< [GB.GG] QoS trust state, CTC_QOS_TRUST_XXX */
    uint8  color;               /**< [GB.GG.D2.TM] Color: green, yellow, or red :CTC_QOS_COLOR_XXX*/
    uint8  priority;            /**< [GB.GG.D2.TM] Priority: 0 - 15 */
    uint8  log_percent;         /**< [GB.GG.D2.TM] Logging percent, CTC_LOG_PERCENT_POWER_NEGATIVE_XX */
    uint8  log_session_id;      /**< [GB.GG.D2.TM] Logging session ID */
    uint8  qos_domain;          /**< [GB.GG] qos domain*/
    uint8  dscp;                /**< [GB.GG.D2.TM] dscp*/

    uint32  assign_port;        /**< [GG] using for CTC_ACL_ACTION_FLAG_ASSIGN_OUTPUT_PORT, means destination Port*/
    uint16  fid;                /**< [GG] fid */
    uint16  vrf_id;             /**< [GG] Vrf ID of the route */
    uint32  metadata;           /**< [GG.D2.TM] Metadata*/

    ctc_acl_packet_strip_t packet_strip; /**< [GG.D2.TM] strip packet for ingress acl*/

    ctc_acl_vlan_edit_t vlan_edit;     /**< [GB.GG.D2.TM] vlan edit, for D2 only support ingress acl */
};
typedef struct ctc_acl_action_s ctc_acl_action_t;

/**
@brief  acl entry structure
*/
struct ctc_acl_entry_s
{
    ctc_acl_key_t key;              /**< [GB.GG.D2.TM] acl key struct£¬ [D2.TM]only used when mode equal 0 for compatible mode*/
    ctc_acl_action_t action;        /**< [GB.GG.D2.TM] acl action struct*/

    uint8  key_type;                /**< [D2.TM] acl key type,refer to ctc_acl_key_type_t*/
    uint32 entry_id;                /**< [GB.GG.D2.TM] acl action id*/
    uint8  priority_valid;          /**< [GB.GG.D2.TM] acl entry priority valid. if not valid, use default priority*/
    uint32 priority;                /**< [GB.GG.D2.TM] acl entry priority.*/
    uint8  hash_field_sel_id;       /**< [D2.TM] Hash select ID,if set to 0 ,indicate all field are vaild */
    uint8  mode;                    /**< [D2.TM] 0, use sturct to install key and action; 1, use field to install key and action */
};
typedef struct ctc_acl_entry_s ctc_acl_entry_t;

/**
@brief  acl copy entry struct
*/
struct ctc_acl_copy_entry_s
{
    uint32 src_entry_id;  /**< [GB.GG.D2.TM] ACL entry ID to copy from */
    uint32 dst_entry_id;  /**< [GB.GG.D2.TM] New entry copied from src_entry_id */
    uint32 dst_group_id;  /**< [GB.GG.D2.TM] Group ID new entry belongs */
};
typedef struct ctc_acl_copy_entry_s ctc_acl_copy_entry_t;

/**
@brief  acl hash mac key flag
*/
enum ctc_acl_hash_mac_key_flag_e
{
    CTC_ACL_HASH_MAC_KEY_FLAG_MAC_DA           = 1U << 0, /**< [GB] MAC DA as ACL HASH MAC KEY member*/
    CTC_ACL_HASH_MAC_KEY_FLAG_ETH_TYPE         = 1U << 1, /**< [GB] ETHER TYPE as ACL HASH MAC KEY member*/
    CTC_ACL_HASH_MAC_KEY_FLAG_PHY_PORT         = 1U << 2, /**< [GB] PHY PORT as ACL HASH MAC KEY member*/
    CTC_ACL_HASH_MAC_KEY_FLAG_LOGIC_PORT       = 1U << 3, /**< [GB] LOGIC PORT as ACL HASH MAC KEY member*/
    CTC_ACL_HASH_MAC_KEY_FLAG_COS              = 1U << 4, /**< [GB] COS as ACL HASH MAC KEY member*/
    CTC_ACL_HASH_MAC_KEY_FLAG_VLAN_ID          = 1U << 5, /**< [GB] VLAN ID as ACL HASH MAC KEY member*/
    CTC_ACL_HASH_MAC_KEY_FLAG_MAX              = 1U << 6
};
typedef enum ctc_acl_hash_mac_key_flag_e ctc_acl_hash_mac_key_flag_t;


/**
@brief  acl hash IPv4 key flag
*/
enum ctc_acl_hash_ipv4_key_flag_e
{
    CTC_ACL_HASH_IPV4_KEY_FLAG_IP_DA            = 1U << 0, /**< [GB] IP DA as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_IP_SA            = 1U << 1, /**< [GB] IP SA as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_L4_SRC_PORT      = 1U << 2, /**< [GB] L4 source port as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_L4_DST_PORT      = 1U << 3, /**< [GB] L4 dest port as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_PHY_PORT         = 1U << 4, /**< [GB] Phy port as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_LOGIC_PORT       = 1U << 5, /**< [GB] Logic port as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_DSCP             = 1U << 6, /**< [GB] Dscp as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_L4_PROTOCOL      = 1U << 7, /**< [GB] L4 protocol as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_ARP_PACKET       = 1U << 8, /**< [GB] Arp packet as ACL IPV4 KEY member*/
    CTC_ACL_HASH_IPV4_KEY_FLAG_MAX              = 1U << 9
};
typedef enum ctc_acl_hash_ipv4_key_flag_e ctc_acl_hash_ipv4_key_flag_t;

/**
@brief  define acl hash mac field sel structure
*/
struct ctc_acl_hash_mac_field_sel_s
{
    uint8 mac_da     ;  /**< [GG.D2.TM] Select mac da as hash mac key field*/
    uint8 mac_sa     ;  /**< [GG.D2.TM] Select mac sa as hash mac key field*/
    uint8 eth_type   ;  /**< [GG.D2.TM] Select ether type as hash mac key field*/
    uint8 phy_port   ;  /**< [GG.D2.TM] Select phy port as hash mac key field*/
    uint8 logic_port ;  /**< [GG.D2.TM] Select logic port as hash mac key field*/
    uint8 metadata   ;  /**< [GG.D2.TM] Select metadata as hash mac key field*/
    uint8 cos        ;  /**< [GG.D2.TM] Select cos as hash mac key field*/
    uint8 cfi        ;  /**< [GG.D2.TM] Select cfi as hash mac key field*/
    uint8 vlan_id    ;  /**< [GG.D2.TM] Select vlan_id as hash mac key field*/
    uint8 tag_valid  ;  /**< [GG.D2.TM] Select tag valid as hash mac key field*/
};
typedef struct ctc_acl_hash_mac_field_sel_s ctc_acl_hash_mac_field_sel_t;

/**
@brief  define acl hash IPv4 field sel structure
*/
struct ctc_acl_hash_ipv4_field_sel_s
{
    uint8  ip_da       ;  /**< [GG.D2.TM] Select ip da as hash ipv4 key field*/
    uint8  ip_sa       ;  /**< [GG.D2.TM] Select ip sa as hash ipv4 key field*/
    uint8  l4_src_port ;  /**< [GG.D2.TM] Select l4 source port as hash ipv4 key field*/
    uint8  l4_dst_port ;  /**< [GG.D2.TM] Select l4 dest port as hash ipv4 key field*/
    uint8  phy_port    ;  /**< [GG.D2.TM] Select phy port as hash ipv4 key field*/
    uint8  logic_port  ;  /**< [GG.D2.TM] Select logic port as hash ipv4 key field*/
    uint8  metadata    ;  /**< [GG.D2.TM] Select metadata as hash ipv4 key field*/
    uint8  dscp        ;  /**< [GG.D2.TM] Select dscp as hash ipv4 key field*/
    uint8  ecn         ;  /**< [GG.D2.TM] Select ecn da as hash ipv4 key field*/
    uint8  l4_protocol ;  /**< [GG.D2.TM] Select l4 protocol as hash ipv4 key field*/
    uint8  icmp_type   ;  /**< [GG.D2.TM] Select icmp type as hash ipv4 key field*/
    uint8  icmp_code   ;  /**< [GG.D2.TM] Select icmp code as hash ipv4 key field*/
    uint8  vni         ;  /**< [GG.D2.TM] Select vxlan vni as hash ipv4 key field*/
    uint8  gre_key     ;  /**< [GG.D2.TM] Select gre or nvgre key as hash ipv4 key field*/
};
typedef struct ctc_acl_hash_ipv4_field_sel_s ctc_acl_hash_ipv4_field_sel_t;


/**
@brief  define acl hash mpls field sel structure
*/
struct ctc_acl_hash_mpls_field_sel_s
{
    uint8 phy_port          ;  /**< [GG.D2.TM] Select phy port as hash mpls key field*/
    uint8 logic_port        ;  /**< [GG.D2.TM] Select logic port as hash mpls key field*/
    uint8 metadata          ;  /**< [GG.D2.TM] Select metadata as hash mpls key field*/
    uint8 mpls_label_num    ;  /**< [GG.D2.TM] Select mpls label number as hash mpls key field*/
    uint8 mpls_label0_label ;  /**< [GG.D2.TM] Select mpls label0's label as hash mpls key field*/
    uint8 mpls_label0_exp   ;  /**< [GG.D2.TM] Select mpls label0's exp as hash mpls key field*/
    uint8 mpls_label0_s     ;  /**< [GG.D2.TM] Select mpls label0's sbit as hash mpls key field*/
    uint8 mpls_label0_ttl   ;  /**< [GG.D2.TM] Select mpls label0's ttl as hash mpls key field*/
    uint8 mpls_label1_label ;  /**< [GG.D2.TM] Select mpls label1's label as hash mpls key field*/
    uint8 mpls_label1_exp   ;  /**< [GG.D2.TM] Select mpls label1's exp as hash mpls key field*/
    uint8 mpls_label1_s     ;  /**< [GG.D2.TM] Select mpls label1's sbit as hash mpls key field*/
    uint8 mpls_label1_ttl   ;  /**< [GG.D2.TM] Select mpls label1's ttl as hash mpls key field*/
    uint8 mpls_label2_label ;  /**< [GG.D2.TM] Select mpls label2's label as hash mpls key field*/
    uint8 mpls_label2_exp   ;  /**< [GG.D2.TM] Select mpls label2's exp as hash mpls key field*/
    uint8 mpls_label2_s     ;  /**< [GG.D2.TM] Select mpls label2's sbit as hash mpls key field*/
    uint8 mpls_label2_ttl   ;  /**< [GG.D2.TM] Select mpls label2's ttl as hash mpls key field*/
};
typedef struct ctc_acl_hash_mpls_field_sel_s ctc_acl_hash_mpls_field_sel_t;

/**
@brief  define acl hash IPv6 field sel structure
*/
struct ctc_acl_hash_ipv6_field_sel_s
{
    uint8 ip_da       ;  /**< [GG.D2.TM] Select ip da as hash ipv6 key field*/
    uint8 ip_sa       ;  /**< [GG.D2.TM] Select ip sa as hash ipv6 key field*/
    uint8 l4_src_port ;  /**< [GG.D2.TM] Select l4 source port as hash ipv6 key field*/
    uint8 l4_dst_port ;  /**< [GG.D2.TM] Select l4 dest port as hash ipv6 key field*/
    uint8 phy_port    ;  /**< [GG.D2.TM] Select phy port as hash ipv6 key field*/
    uint8 logic_port  ;  /**< [GG.D2.TM] Select logic port as hash ipv6 key field*/
    uint8 metadata    ;  /**< [GG.D2.TM] Select metadata as hash ipv6 key field*/
    uint8 dscp        ;  /**< [GG.D2.TM] Select dscp as hash ipv6 key field*/
    uint8 l4_type     ;  /**< [GG.D2.TM] Select l4 type as hash ipv6 key field*/
    uint8 icmp_type   ;  /**< [GG.D2.TM] Select icmp type as hash ipv6 key field*/
    uint8 icmp_code   ;  /**< [GG.D2.TM] Select icmp code as hash ipv6 key field*/
    uint8  vni        ;  /**< [GG.D2.TM] Select vxlan vni as hash ipv6 key field*/
    uint8  gre_key    ;  /**< [GG.D2.TM] Select gre or nvgre key as hash ipv6 key field*/
};
typedef struct ctc_acl_hash_ipv6_field_sel_s ctc_acl_hash_ipv6_field_sel_t;


/**
@brief  define acl hash L2_L3 field sel structure
*/
struct ctc_acl_hash_l2_l3_field_sel_s
{
    uint8 phy_port            ;  /**< [GG.D2.TM] Select phy port as hash l2 l3 key field*/
    uint8 logic_port          ;  /**< [GG.D2.TM] Select logic port as hash l2 l3 key field*/
    uint8 metadata            ;  /**< [GG.D2.TM] Select metadata as hash l2 l3 key field*/
    uint8 mac_da              ;  /**< [GG.D2.TM] Select mac da as hash l2 l3 key field*/
    uint8 mac_sa              ;  /**< [GG.D2.TM] Select mac sa as hash l2 l3 key field*/
    uint8 stag_cos            ;  /**< [GG.D2.TM] Select stag cos as hash l2 l3 key field*/
    uint8 stag_cfi            ;  /**< [GG.D2.TM] Select stag cfi as hash l2 l3 key field*/
    uint8 stag_vlan           ;  /**< [GG.D2.TM] Select stag vlan as hash l2 l3 key field*/
    uint8 stag_valid          ;  /**< [GG.D2.TM] Select stag valid as hash l2 l3 key field*/
    uint8 ctag_cos            ;  /**< [GG.D2.TM] Select ctag cos as hash l2 l3 key field*/
    uint8 ctag_cfi            ;  /**< [GG.D2.TM] Select ctag cfi as hash l2 l3 key field*/
    uint8 ctag_vlan           ;  /**< [GG.D2.TM] Select ctag vlan as hash l2 l3 key field*/
    uint8 ctag_valid          ;  /**< [GG.D2.TM] Select ctag valid as hash l2 l3 key field*/
    uint8 ip_da              ;   /**< [GG.D2.TM] Select ip da as hash l2 l3 key field*/
    uint8 ip_sa              ;   /**< [GG.D2.TM] Select ip sa as hash l2 l3 key field*/
    uint8 dscp                ;  /**< [GG.D2.TM] Select dscp as hash l2 l3 key field*/
    uint8 ecn                 ;  /**< [GG.D2.TM] Select ecn as hash l2 l3 key field*/
    uint8 ttl                 ;  /**< [GG.D2.TM] Select ttl as hash l2 l3 key field*/
    uint8 eth_type            ;  /**< [GG.D2.TM] Select ether type as hash l2 l3 key field*/
    uint8 l3_type             ;  /**< [GG.D2.TM] Select l3 type as hash l2 l3 key field*/
    uint8 l4_type             ;  /**< [GG.D2.TM] Select l4 type as hash l2 l3 key field*/
    uint8 mpls_label_num      ;  /**< [GG.D2.TM] Select mpls label num as hash l2 l3 key field*/
    uint8 mpls_label0_label   ;  /**< [GG.D2.TM] Select mpls label0's label as hash l2 l3 key field*/
    uint8 mpls_label0_exp     ;  /**< [GG.D2.TM] Select mpls label0's exp as hash l2 l3 key field*/
    uint8 mpls_label0_s       ;  /**< [GG.D2.TM] Select mpls label0's sbit as hash l2 l3 key field*/
    uint8 mpls_label0_ttl     ;  /**< [GG.D2.TM] Select mpls label0's ttl as hash l2 l3 key field*/
    uint8 mpls_label1_label   ;  /**< [GG.D2.TM] Select mpls label1's label as hash l2 l3 key field*/
    uint8 mpls_label1_exp     ;  /**< [GG.D2.TM] Select mpls label1's exp as hash l2 l3 key field*/
    uint8 mpls_label1_s       ;  /**< [GG.D2.TM] Select mpls label1's sbit as hash l2 l3 key field*/
    uint8 mpls_label1_ttl     ;  /**< [GG.D2.TM] Select mpls label1's ttl as hash l2 l3 key field*/
    uint8 mpls_label2_label   ;  /**< [GG.D2.TM] Select mpls label2's label as hash l2 l3 key field*/
    uint8 mpls_label2_exp     ;  /**< [GG.D2.TM] Select mpls label2's exp as hash l2 l3 key field*/
    uint8 mpls_label2_s       ;  /**< [GG.D2.TM] Select mpls label2's sbit as hash l2 l3 key field*/
    uint8 mpls_label2_ttl     ;  /**< [GG.D2.TM] Select mpls label2's ttl as hash l2 l3 key field*/

    uint8 l4_src_port         ;  /**< [GG.D2.TM] Select l4 source port as hash l2 l3 key field*/
    uint8 l4_dst_port         ;  /**< [GG.D2.TM] Select l4 dest port as hash l2 l3 key field*/

    uint8  icmp_type          ;  /**< [GG.D2.TM] Select icmp type as hash l2 l3 key field*/
    uint8  icmp_code          ;  /**< [GG.D2.TM] Select icmp code as hash l2 l3 key field*/
    uint8  vni                ;  /**< [GG.D2.TM] Select vxlan vni as hash l2 l3 key field*/
    uint8  gre_key            ;  /**< [GG.D2.TM] Select gre or nvgre key as hash l2 l3 key field*/
};
typedef struct ctc_acl_hash_l2_l3_field_sel_s ctc_acl_hash_l2_l3_field_sel_t;



/**
@brief  define acl hash field sel structure
*/
struct ctc_acl_hash_field_sel_s
{
    uint8  hash_key_type;                       /**< [GG.D2.TM]refer ctc_acl_key_type_t */
    uint32 field_sel_id;                        /**< [GG.D2.TM] field select id*/

    union
    {
        ctc_acl_hash_mac_field_sel_t   mac;     /**< [GG.D2.TM] hash mac field select */
        ctc_acl_hash_mpls_field_sel_t  mpls;    /**< [GG.D2.TM] hash mpls field select*/
        ctc_acl_hash_ipv4_field_sel_t  ipv4;    /**< [GG.D2.TM] hash ipv4 field select*/
        ctc_acl_hash_ipv6_field_sel_t  ipv6;    /**< [GG.D2.TM] hash ipv6 field select*/
        ctc_acl_hash_l2_l3_field_sel_t l2_l3;   /**< [GG.D2.TM] hash l2 l3 field select*/
    }u;

};
typedef struct ctc_acl_hash_field_sel_s ctc_acl_hash_field_sel_t;


/**
@brief  acl global config structure
*/
struct ctc_acl_global_cfg_s
{
    uint8  merge_mac_ip;                 /**< [GB] If merged, l2 packets and ipv4 packets will all go ipv4-entry. In this case, mac-entry will never hit. */
    uint8  ingress_use_mapped_vlan;      /**< [GB] Ingress acl use mapped vlan instead of packet vlan  */

    uint8  trill_use_ipv6;               /**< [GB] trill packet use ipv6 key*/
    uint8  arp_use_ipv6;                 /**< [GB] arp packet use ipv6 key*/
    uint8  non_ipv4_mpls_use_ipv6;       /**< [GB] non ipv4 or mpls packet use ipv6 key*/
    uint8  hash_acl_en;                  /**< [GB] enable hash acl */
    uint32  priority_bitmap_of_stats;    /**< [GB] bitmap of priority to support stats. Only support 2 stats of 4 priority. Bigger than 2, the high priority will take effect. */

    uint8  ingress_port_service_acl_en;  /**< [GB.GG] ingress service acl enable on port*/
    uint8  ingress_vlan_service_acl_en;  /**< [GB.GG] ingress service acl enable on vlan*/
    uint8  egress_port_service_acl_en;   /**< [GB.GG] egress service acl enable on port*/
    uint8  egress_vlan_service_acl_en;   /**< [GB.GG] egress service acl enable on vlan*/

    uint32 hash_ipv4_key_flag;           /**< [GB] hash ipv4 key flag*/
    uint32 hash_mac_key_flag;            /**< [GB] hash mac key flag*/

    uint8  white_list_en;                /**< [GB] acl white list enable*/
/* SYSTEM MODIFY, Added by cuixl for SDK GG bug 27975, 2014-04-02 *//*SYSTEM MODIFY, commented by xgu, do not remove following code!! 2015-1-28*/
/* port acl lookup must sperated from vlan acl lookup in GG*/
    uint8  vlan_acl_num;              /**< [GG] specify vlan acl lookup number */
/* SYSTEM MODIFY END */

};
typedef struct ctc_acl_global_cfg_s ctc_acl_global_cfg_t;

/**
@brief  acl group type
*/
enum ctc_acl_group_type_e
{
    CTC_ACL_GROUP_TYPE_NONE,         /**< [GB.GG.D2.TM] default type, care port info by entry*/
    CTC_ACL_GROUP_TYPE_HASH,         /**< [GB.GG.D2.TM] Hash group */
    CTC_ACL_GROUP_TYPE_PORT_BITMAP,  /**< [GB.GG.D2.TM] Port bitmap */
    CTC_ACL_GROUP_TYPE_GLOBAL,       /**< [GB.GG.D2.TM] Global acl, mask ports and vlans */
    CTC_ACL_GROUP_TYPE_VLAN_CLASS,   /**< [GB.GG.D2.TM] A vlan class acl is against a class(group) of vlan*/
    CTC_ACL_GROUP_TYPE_PORT_CLASS,   /**< [GB.GG.D2.TM] A port class acl is against a class(group) of port*/
    CTC_ACL_GROUP_TYPE_SERVICE_ACL,  /**< [GB.GG.D2.TM] A service acl is against a service*/
    CTC_ACL_GROUP_TYPE_L3IF_CLASS,   /**< [D2.TM] A l3if class acl is against a class(group) of l3if*/
    CTC_ACL_GROUP_TYPE_PBR_CLASS,    /**< [GB.GG.D2.TM] A pbr class is against a class(group) of l3 source interface*/
    CTC_ACL_GROUP_TYPE_PORT,         /**< [GG.D2.TM] Port acl, care gport. */
    CTC_ACL_GROUP_TYPE_MAX
};
typedef enum ctc_acl_group_type_e ctc_acl_group_type_t;

enum ctc_acl_key_size_e
{
    CTC_ACL_KEY_SIZE_NONE,          /**< [D2.TM] None key size*/
    CTC_ACL_KEY_SIZE_160,           /**< [D2.TM] 160 bits key size*/
    CTC_ACL_KEY_SIZE_320,           /**< [D2.TM] 320 bits key size*/
    CTC_ACL_KEY_SIZE_640,           /**< [D2.TM] 640 bits key size*/
};
typedef enum ctc_acl_key_size_e ctc_acl_key_size_t;

/**
@brief  acl group info,  NOT For hash group
*/
struct ctc_acl_group_info_s
{
    uint8 type;                      /**< [GB.GG.D2.TM] install_group ignore this, ctc_acl_group_type_t */
    uint8 lchip;                     /**< [GB.GG.D2.TM] Local chip id, only for type= PORT_BITMAP, Other type ignore it*/
    uint8 priority;                  /**< [GB.GG.D2.TM] install_group ignore this, Group priority, Pbr ignore it*/
    uint8 key_size;                  /**< [D2.TM] All entrys in this group occupy the same key size otherwise decide according to key type,refer to ctc_acl_key_size_t*/
    ctc_direction_t dir;             /**< [GB.GG.D2.TM] diretion. Pbr ignore it*/

    union
    {
        ctc_port_bitmap_t port_bitmap;/**< [GB.GG.D2.TM] In GB, mpls key only support 52 bits, mac/ipv4/ipv6 key support 56 bits */
        uint16 port_class_id;         /**< [GB.GG.D2.TM] port class id, multiple ports can share same port_class_id */
        uint16 vlan_class_id;         /**< [GB.GG.D2.TM] vlan class id, multiple vlans can share same vlan_class_id */
        uint16 l3if_class_id;         /**< [D2.TM] l3if class id, multiple l3 interface can share same l3if_class_id */
        uint16 service_id;            /**< [GB.GG.D2.TM] service id  */
        uint8  pbr_class_id;          /**< [GB.GG.D2.TM] pbr class id */
        uint32 gport;                 /**< [GG.D2.TM] gport */
    } un;

};
typedef struct ctc_acl_group_info_s ctc_acl_group_info_t;




/**
@brief  acl cid pair flag
*/
enum ctc_acl_cid_pair_flag_e
{
    CTC_ACL_CID_PAIR_FLAG_FLEX            = 1U << 0,   /**< [D2.TM] CategoryId Pair use tcam, can use to solve hash conflict. */
    CTC_ACL_CID_PAIR_FLAG_SRC_CID         = 1U << 1,   /**< [D2.TM] src category id as cid pair member. */
    CTC_ACL_CID_PAIR_FLAG_DST_CID         = 1U << 2,   /**< [D2.TM] dst category id as cid pair member. */
    CTC_ACL_CID_APIR_FLAG_MAX
};
typedef enum ctc_acl_cid_pair_flag_e ctc_acl_cid_pair_flag_t;

/**
@brief  acl cid pair action flag
*/
enum ctc_acl_cid_pair_action_e
{
    CTC_ACL_CID_PAIR_ACTION_PERMIT,             /**< [D2.TM] Permit packet. */
    CTC_ACL_CID_PAIR_ACTION_DISCARD,            /**< [D2.TM] Discard packet. */
    CTC_ACL_CID_PAIR_ACTION_OVERWRITE_ACL,      /**< [D2.TM] Overwrite tcam acl. */
    CTC_ACL_CID_PAIR_ACTION_MAX
};
typedef enum ctc_acl_cid_pair_action_e ctc_acl_cid_pair_action_t;

/**
@brief  define acl cid pair structure
*/
struct ctc_acl_cid_pair_s
{
    uint32  flag;                       /**< [D2.TM] Category Id pair flag, ctc_acl_cid_pair_flag_t. */

    uint16  src_cid;                    /**< [D2.TM] src category id. */
    uint16  dst_cid;                    /**< [D2.TM] dst category id. */

    uint8   action_mode;                /**< [D2.TM] cid pair action mode, ctc_acl_cid_pair_action_t. */
    ctc_acl_property_t acl_prop;        /**< [D2.TM] overwrite acl property. */
};
typedef struct ctc_acl_cid_pair_s ctc_acl_cid_pair_t;

/**
@brief  define acl oam structure
*/
/**
@brief  acl oam type
*/
enum ctc_acl_oam_type_e
{
    CTC_ACL_OAM_TYPE_ETH_OAM = 1,    /**< [D2.TM] Oam type is Ether Oam. */
    CTC_ACL_OAM_TYPE_IP_BFD = 2,     /**< [D2.TM] Oam type is IP BFD. */
    CTC_ACL_OAM_TYPE_TWAMP = 11,     /**< [D2.TM] Oam type is TWAMP. */
    CTC_ACL_OAM_TYPE_FLEX = 12,      /**< [D2.TM] Oam type is FLEX. */
    CTC_ACL_OAM_TYPE_MAX
};
typedef enum ctc_acl_oam_type_e  ctc_acl_oam_type_t;

struct ctc_acl_oam_s
{
   uint8 dest_gchip;       /**< [D2.TM] Global Chip Id. */
   uint8 oam_type;         /**< [D2.TM] Oam Type, refer to ctc_acl_oam_type_t . */
   uint16 lmep_index;      /**< [D2.TM] Local Mep index . */

   uint8 mip_en;           /**< [D2.TM] Mip enable . */
   uint8 link_oam_en;      /**< [D2.TM] Link Oam or Section Oam . */
   uint8 packet_offset;    /**< [D2.TM] Only used for bfd */
   uint8 mac_da_check_en;  /**< [D2.TM] Mac da check enable*/

   uint8 is_self_address;  /**< [D2.TM] Indicate that mac address is switch's*/
   uint8 time_stamp_en;    /**< [D2.TM] Time stamp enable*/
   uint8 timestamp_format;   /**< [TM] Timestamp format (0: PTPv2, 1:NTP) */
   uint8 rsv;

};
typedef struct  ctc_acl_oam_s ctc_acl_oam_t;



/**
@brief  Only used for CTC_FIELD_KEY_AWARE_TUNNEL_INFO
*/
struct ctc_acl_tunnel_data_s
{
   uint8 type;                               /**< [D2.TM] 0:none; 1: Vxlan; 2: Gre; 3:Wlan*/
   uint8 rsv[3];
   uint32 vxlan_vni;                         /**< [D2.TM] Merge Key. Vxlan vni*/
   uint32 gre_key;                           /**< [D2.TM] Merge Key. Gre key*/
   mac_addr_t radio_mac;                     /**< [D2.TM] Merge Key. Wlan info: Radio Mac*/
   uint8 radio_id;                           /**< [D2.TM] Merge Key. Wlan info: Radio ID*/
   uint8 wlan_ctl_pkt;                       /**< [D2.TM] Merge Key. Wlan info: Wlan Ctl Pkt*/
};
typedef struct  ctc_acl_tunnel_data_s ctc_acl_tunnel_data_t;

enum ctc_acl_to_cpu_mode_e
{
    CTC_ACL_TO_CPU_MODE_TO_CPU_NOT_COVER,   /**< [D2.TM] Cover switch logic copy-to-CPU,if switch logic don't copy-to-CPU,the rules enable copy-to-CPU with CTC_PKT_CPU_REASON_ACL_MATCH */
    CTC_ACL_TO_CPU_MODE_TO_CPU_COVER,       /**< [D2.TM] Cover switch logic copy-to-CPU,re-assign cpu reason */
    CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU,      /**< [D2.TM] Cancel switch logic copy-to-CPU */
    CTC_ACL_TO_CPU_MODE_MAX
};
typedef enum ctc_acl_to_cpu_mode_e ctc_acl_to_cpu_mode_t;


struct ctc_acl_to_cpu_s
{
    uint16  mode;                          /**< [D2.TM] CTC_ACL_TO_CPU_MODE_XXX, refer to ctc_acl_to_cpu_mode_t */
    uint16  cpu_reason_id;                 /**< [D2.TM] just for CTC_ACL_TO_CPU_MODE_TO_CPU_COVER,only support normal exception or user-define cpu_reason */
};
typedef struct ctc_acl_to_cpu_s ctc_acl_to_cpu_t;


struct ctc_acl_lb_hash_s
{
    uint8   mode;                 /**< [D2.TM] 0:linkagg hash;1:ecmp hash; 2: linkagg & ecmp hash use same value */
    uint8  lb_hash;              /**< [D2.TM] LoadBalance Hash value  */
};
typedef struct ctc_acl_lb_hash_s ctc_acl_lb_hash_t;

struct ctc_acl_ipfix_s
{
    uint8  flow_cfg_id;              /**< [D2.TM] ipfix cfg profile id */
    uint8  field_sel_id;             /**< [D2.TM] ipfix hash field select id */
    uint8  hash_type;                /**< [D2.TM] ipfix hash key type, refer to ctc_ipfix_lkup_type_t */
    uint8  use_mapped_vlan;          /**< [D2.TM] ipfix use mapped vlan */

    uint8  use_cvlan;                /**< [D2.TM] ipfix use cvlan */
    uint8  lantency_en;              /**< [TM] enable ipfix measure lantency only for egress */
    uint8  rsv[2];
    uint32 policer_id;               /**< [TM] ipfix MFP policer id*/
};
typedef struct ctc_acl_ipfix_s ctc_acl_ipfix_t;

struct ctc_acl_udf_s
{
    uint32  udf_id;                       /**< [D2.TM] UDF ID*/
    uint8   udf[CTC_ACL_UDF_BYTE_NUM];    /**< [D2.TM] The offset of each UDF field*/
};
typedef struct ctc_acl_udf_s ctc_acl_udf_t;

enum ctc_acl_udf_offset_type_e
{
    CTC_ACL_UDF_TYPE_NONE,
    CTC_ACL_UDF_OFFSET_L2,  /**< [D2.TM] start of L2 header*/
    CTC_ACL_UDF_OFFSET_L3,  /**< [D2.TM] start of L3 header*/
    CTC_ACL_UDF_OFFSET_L4,  /**< [D2.TM] start of L4 header*/
    CTC_ACL_UDF_TYPE_NUM
};
typedef enum ctc_acl_udf_offset_type_e ctc_acl_udf_offset_type_t;

/**
@brief  acl entry structure
*/
struct ctc_acl_classify_udf_s
{
  uint32 udf_id;                                 /**< [D2.TM] UDF ID*/
  uint16 priority;                               /**< [D2.TM] entry priority of UDF entry*/
  uint8  query_type;                              /**< [D2.TM] 0-by UDF ID;1-By priority*/
  uint8  tcp_option_en;                          /**< [D2.TM] if set, indicate macth packet is TCP and TCP option enable */
  uint8  offset_type;                            /**< [D2.TM]  refer to ctc_acl_udf_offset_type_t*/
  uint8  offset_num;                             /**< [D2.TM] The number of UDF field*/
  uint8  offset_type_array[CTC_ACL_UDF_FIELD_NUM];                            /**< [TM]  refer to ctc_acl_udf_offset_type_t*/
  uint8  offset[CTC_ACL_UDF_FIELD_NUM];         /**< [D2.TM] The offset of each UDF field*/
  uint8  valid;                                 /**< [D2.TM] indicate the UDF entry have installed to HW and key is FULL, used to get udf entry*/
};
typedef struct ctc_acl_classify_udf_s ctc_acl_classify_udf_t;

struct ctc_acl_inter_cn_s
{
    uint8  inter_cn;           /**< [D2.TM] Internal cn, pls refer to ctc_qos_inter_cn_t */
    uint8  rsv[3];
};
typedef struct ctc_acl_inter_cn_s ctc_acl_inter_cn_t;

struct ctc_acl_table_map_s
{
    uint8  table_map_id;                   /**< [D2.TM] Table map id */
    uint8  table_map_type;                 /**< [D2.TM] Table mapping type, refer to ctc_qos_table_map_type_t */
    uint16 rsv;
};
typedef struct ctc_acl_table_map_s ctc_acl_table_map_t;

enum ctc_acl_field_action_type_e
{
    CTC_ACL_FIELD_ACTION_CP_TO_CPU,             /**< [D2.TM] Copy To Cpu;refer to ctc_acl_to_cpu_t */
    CTC_ACL_FIELD_ACTION_STATS,                 /**< [D2.TM] Packet Statistics; data0: Stats Id */
    CTC_ACL_FIELD_ACTION_RANDOM_LOG,            /**< [D2.TM] Random Log, the Destination is config by mirror; data0: Log Id; data1: ctc_log_percent_t */
    CTC_ACL_FIELD_ACTION_COLOR,                 /**< [D2.TM] Change Color; data0: New Color (ctc_qos_color_t)*/
    CTC_ACL_FIELD_ACTION_DSCP,                  /**< [D2.TM] Dscp(only support ingress direction); data0: New Dscp*/
    CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER,    /**< [D2.TM] Micro Flow Policer; data0: Micro Policer Id */
    CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER,    /**< [D2.TM] Macro Flow Policer; data0: Macro Policer Id */
    CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER,      /**< [D2.TM] HBWP Policer with cos index; data0: Policer Id; data1: cos index*/
    CTC_ACL_FIELD_ACTION_COPP,                  /**< [D2.TM] Control Plane Policing(CoPP); data0: Policer Id */
    CTC_ACL_FIELD_ACTION_SRC_CID,               /**< [D2.TM] Change Src category id; data0: New Src Category Id */
    CTC_ACL_FIELD_ACTION_TRUNCATED_LEN,         /**< [D2.TM] The length of packet after truncate; data0: Length */
    CTC_ACL_FIELD_ACTION_REDIRECT,              /**< [D2.TM] Redirect Packet; data0: Nexthop Id */
    CTC_ACL_FIELD_ACTION_REDIRECT_PORT,         /**< [D2.TM] Redirect Packet to Single Port; data0: Assign Port */
    CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT, /**< [D2.TM] Packet edit will be canceled for redirect packet*/
    CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT, /**<[D2.TM] Filter Routed packet for redirect operation */

    CTC_ACL_FIELD_ACTION_DISCARD,               /**< [D2.TM] Discard Packet; data0: color bitmap (ctc_qos_color_t); For CTC_ACL_KEY_INTERFACE,data0 must be CTC_QOS_COLOR_NONE, which means discarding red , green and yellow.*/
    CTC_ACL_FIELD_ACTION_CANCEL_DISCARD,        /**< [D2.TM] Cancel Discard; data0 : color bitmap (ctc_qos_color_t) */
    CTC_ACL_FIELD_ACTION_PRIORITY,              /**< [D2.TM] Change Priority; data0: color (ctc_qos_color_t); data1: New Priority */

    CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG,  /**< [D2.TM] Disable Elephant Flow Log to cpu */
    CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR,     /**< [D2.TM] Terminate Cid Header to Packet */
    CTC_ACL_FIELD_ACTION_CANCEL_NAT,            /**< [D2.TM] Do Not perform NAT on this matching packet */

    CTC_ACL_FIELD_ACTION_IPFIX,                 /**< [D2.TM] Enable Ipfix; ext_data: ctc_acl_ipfix_t */
    CTC_ACL_FIELD_ACTION_CANCEL_IPFIX,          /**< [D2.TM] Cancel Ipfix */
    CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING, /**< [D2.TM] Cancel Ipfix Learning */

    CTC_ACL_FIELD_ACTION_OAM,                   /**< [D2.TM] Enable OAM; ext_data: ctc_acl_oam_t */

    CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH,       /**< [D2] Replace ECMP/Linkagg Hash Value;  ext_data: ctc_acl_lb_hash_t */

    CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE,          /**< [D2.TM] Not encrypt packet on 802.1AE controlled ports */
    CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT,/**< [D2.TM] Unencrypted Packet not discard on 802.1AE controlled ports */

    CTC_ACL_FIELD_ACTION_METADATA,              /**< [D2.TM] Metedata; data0: Metadata, if data0 equal to 0, means reset metdata */
    CTC_ACL_FIELD_ACTION_INTER_CN,              /**< [D2.TM] Internal cn, indicate congestion class and state; ext_data: ctc_acl_inter_cn_t */

    CTC_ACL_FIELD_ACTION_VLAN_EDIT,             /**< [D2.TM] Vlan Edit; ext_data: ctc_acl_vlan_edit_t */
    CTC_ACL_FIELD_ACTION_STRIP_PACKET,          /**< [D2.TM] Used for Redirect Packet to Strip Packet Header ; ext_data: ctc_acl_packet_strip_t */

    CTC_ACL_FIELD_ACTION_CRITICAL_PKT,          /**< [D2.TM] Critical Packet; */
    CTC_ACL_FIELD_ACTION_LOGIC_PORT,            /**< [D2.TM] Logical port; */

    CTC_ACL_FIELD_ACTION_SPAN_FLOW,             /**< [D2.TM] Indicate is span packet */
    CTC_ACL_FIELD_ACTION_CANCEL_ALL,            /**< [D2.TM] Default Action:cancel all action; */
    CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP,         /**< [D2.TM] Qos table mapping; ext_data: ctc_acl_table_map_t */

     /*only support in acl hash*/
    CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN,    /**< [D2.TM] Cancel ACL Tcam Lookup; data0: ACL Priority */
    CTC_ACL_FIELD_ACTION_DENY_BRIDGE,           /**< [D2.TM] Deny Bridge */
    CTC_ACL_FIELD_ACTION_DENY_LEARNING,         /**< [D2.TM] Deny Learning */
    CTC_ACL_FIELD_ACTION_DENY_ROUTE,            /**< [D2.TM] Deny Route */
    CTC_ACL_FIELD_ACTION_DEST_CID,              /**< [D2.TM] Change Dest category id; data0: New Dest Category Id */

    CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE,       /**< [TM] Select hash offset profile id for ecmp or packet head, ext_data: ctc_lb_hash_offset_t*/
    CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE,        /**< [TM] Select hash offset profile id for linkagg, ext_data: ctc_lb_hash_offset_t */
    CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR,       /**< [TM] Disable ecmp rr mode, data0: disable use 1 and enable use 0 */
    CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR,    /**< [TM] Disable linkagg rr mode, data0: disable use 1 and enable use 0 */
    CTC_ACL_FIELD_ACTION_VXLAN_RSV_EDIT,          /**< [TM] Vxlan reserved data edit mode, data0: merge use 0, clear use 1 and replace use 2 */
    CTC_ACL_FIELD_ACTION_TIMESTAMP,                                /**< [TM] Packet timestamp process, ext_data: refer to ctc_acl_timestamp_t */
    CTC_ACL_FIELD_ACTION_REFLECTIVE_BRIDGE_EN,      /**< [TM] Enable input and output port is same when bridge, data0: 1-enable, 0-disable */
    CTC_ACL_FIELD_ACTION_PORT_ISOLATION_DIS,          /**< [TM] Disable port isolation, data0: 1-disable, 0-enable */

    CTC_ACL_FIELD_ACTION_NUM
};
typedef enum ctc_acl_field_action_type_e ctc_acl_field_action_type_t;

struct ctc_acl_field_action_s
{
    uint16 type;                    /**< [D2.TM] Action Field type, refer to ctc_acl_field_action_type_t */

    uint32 data0;                   /**< [D2.TM] Action Field data0 (uint32) */
    uint32 data1;                   /**< [D2.TM] Action Field data1 (uint32) */
    void*  ext_data;                /**< [D2.TM] Action Field extend data (void*) */
};
typedef struct ctc_acl_field_action_s ctc_acl_field_action_t;

struct ctc_acl_cid_pri_s
{
    uint8   dir;
    uint8   default_cid_pri;
    uint8   fwd_table_cid_pri;
	uint8   flow_table_cid_pri;
	uint8   global_cid_pri;
   	uint8   pkt_cid_pri;
	uint8   if_cid_pri;
	uint8   i2e_cid_pri;
};
typedef struct ctc_acl_cid_pri_s ctc_acl_cid_pri_t;

struct ctc_acl_league_s
{
       uint8  dir;               /**< [D2.TM]      Direction*/
       uint8  acl_priority;      /**< [D2.TM]      ACL priority*/
       uint8  auto_move_en;      /**< [D2.TM]      Automatically move entry forward and the user must must ensure that the attributes on the port/vlan/l3if are consistent*/
       uint16 lkup_level_bitmap; /**< [D2.TM]      Lookup level bitmap*/
       uint16 ext_tcam_bitmap;   /**< [TM]         Expand external tcam> */
};
typedef struct ctc_acl_league_s ctc_acl_league_t;

enum ctc_acl_reorder_mode_e
{
    CTC_ACL_REORDER_MODE_SCATTER,      /**< [D2.TM] reorder by scatter*/
    CTC_ACL_REORDER_MODE_DOWNTOUP,     /**< [D2.TM] reorder by down to up*/
    CTC_ACL_REORDER_MODE_UPTODOWN,     /**< [D2.TM] reorder by up to down*/
    CTC_ACL_REORDER_MODE_MAX
};
typedef enum ctc_acl_reorder_mode_e ctc_acl_reorder_mode_t;

struct ctc_acl_reorder_s
{
    uint8   mode;         /**< [D2.TM] (in)     reorder mode, refer to ctc_acl_reorder_mode_t*/
    uint8   dir;          /**< [D2.TM] (in)     Direction*/
    uint8   acl_priority; /**< [D2.TM] (in)     ACL priority*/
};
typedef struct ctc_acl_reorder_s ctc_acl_reorder_t;



#ifdef __cplusplus
}
#endif

#endif

