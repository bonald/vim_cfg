/**
 @file ctc_scl.h

 @date 2013-02-21

 @version v2.0

 The file define api struct used in SCL.
*/

#ifndef _CTC_SCL_H_
#define _CTC_SCL_H_

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "common/include/ctc_const.h"
#include "common/include/ctc_field.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/


/****************************************************************
*
* Data Structures
*
****************************************************************/

enum  ctc_scl_global_e
{
    CTC_SCL_MIN_USER_ENTRY_ID = 0,                        /**< [GB.GG.D2.TM] Min user entry id */
    CTC_SCL_MAX_USER_ENTRY_ID = 0x7FFFFFFF,        /**< [GB.GG.D2.TM] Max user entry id */
    CTC_SCL_GROUP_ID_MIN_NORMAL = 0,                  /**< [GB.GG.D2.TM] Min normal group id */
    CTC_SCL_GROUP_ID_MAX_NORMAL = 0x7FFFFFFF,   /**< [GB.GG.D2.TM] Max normal group id */
    CTC_SCL_KEY_SIZE_SINGLE = 0,                            /**< [GG.D2.TM] Ipv4 tcam key use single size */
    CTC_SCL_KEY_SIZE_DOUBLE = 1                            /**< [GG.D2.TM] Ipv4 tcam key use double size */
};
typedef enum ctc_scl_global_e ctc_scl_global_t;

/* Below groups are created already. just use it . */
/** @brief  SCL hash group id */
enum ctc_scl_group_id_rsv_e
{
    CTC_SCL_GROUP_ID_HASH_PORT = 0xFFFF0001,                       /**< [GB.GG.D2.TM] Group id of hash port group */
    CTC_SCL_GROUP_ID_HASH_PORT_CVLAN = 0xFFFF0002,           /**< [GB.GG.D2.TM] Group id of hash port cvlan group */
    CTC_SCL_GROUP_ID_HASH_PORT_SVLAN = 0xFFFF0003,           /**< [GB.GG.D2.TM] Group id of hash port svlan group */
    CTC_SCL_GROUP_ID_HASH_PORT_2VLAN = 0xFFFF0004,           /**< [GB.GG.D2.TM] Group id of hash port double vlan group */
    CTC_SCL_GROUP_ID_HASH_PORT_CVLAN_COS = 0xFFFF0005,   /**< [GB.GG.D2.TM] Group id of hash port cvlan cos group */
    CTC_SCL_GROUP_ID_HASH_PORT_SVLAN_COS = 0xFFFF0006,   /**< [GB.GG.D2.TM] Group id of hash port svlan cos group */
    CTC_SCL_GROUP_ID_HASH_MAC = 0xFFFF0007,                       /**< [GB.GG.D2.TM] Group id of hash mac group */
    CTC_SCL_GROUP_ID_HASH_PORT_MAC = 0xFFFF0008,              /**< [GB.GG.D2.TM] Group id of hash port mac group */
    CTC_SCL_GROUP_ID_HASH_IPV4 = 0xFFFF0009,                       /**< [GB.GG.D2.TM] Group id of hash ipv4 group */
    CTC_SCL_GROUP_ID_HASH_PORT_IPV4 = 0xFFFF000A,              /**< [GB.GG.D2.TM] Group id of hash port ipv4 group */
    CTC_SCL_GROUP_ID_HASH_IPV6 = 0xFFFF000B,                       /**< [GB.GG.D2.TM] Group id of hash ipv6 group */
    CTC_SCL_GROUP_ID_HASH_L2 = 0xFFFF000C,                           /**< [GB.GG.D2.TM] Group id of hash layer 2 group */
    CTC_SCL_GROUP_ID_HASH_PORT_IPV6 = 0xFFFF000D,              /**< [GG.D2.TM] Group id of hash port ipv6 group */
    CTC_SCL_GROUP_ID_HASH_MAX = 0xFFFF000E,                                 /**< [GB.GG.D2.TM] Max group id of hash group */
    CTC_SCL_GROUP_ID_TCAM0 = CTC_SCL_GROUP_ID_HASH_MAX,       /**< [D2.TM] Group id of TCAM 0 */
    CTC_SCL_GROUP_ID_TCAM1 = 0xFFFF000F,                                   /**< [D2.TM] Group id of TCAM 1 */
    CTC_SCL_GROUP_ID_TCAM2 = 0xFFFF0010,       /**< [TM] Group id of TCAM 2 */
    CTC_SCL_GROUP_ID_TCAM3 = 0xFFFF0011,       /**< [TM] Group id of TCAM 3 */
    CTC_SCL_GROUP_ID_RSV_MAX                             /**< [GB.GG.D2.TM] Max group id of reserved group */
};
typedef enum ctc_scl_group_id_rsv_e ctc_scl_group_id_rsv_t;

/** @brief  SCL Key Type */
enum ctc_scl_key_type_e
{
    CTC_SCL_KEY_TCAM_VLAN,                               /**< [GB.GG] Tcam vlan key */
    CTC_SCL_KEY_TCAM_MAC,                                /**< [GB.GG.D2.TM] Tcam mac key */
    CTC_SCL_KEY_TCAM_IPV4,                               /**< [GB.GG.D2.TM] Tcam ipv4 key */
    CTC_SCL_KEY_TCAM_IPV4_SINGLE,                   /**< [D2.TM] Tcam ipv4 single key */
    CTC_SCL_KEY_TCAM_IPV6,                               /**< [GB.GG.D2.TM] Tcam ipv6 key */
    CTC_SCL_KEY_TCAM_IPV6_SINGLE,                   /**< [D2.TM] Tcam ipv6 single key */
    CTC_SCL_KEY_TCAM_UDF,                               /**< [TM] Tcam udf key */
    CTC_SCL_KEY_TCAM_UDF_EXT,                        /**< [TM] Tcam udf extend key */
    CTC_SCL_KEY_HASH_PORT,                              /**< [GB.GG.D2.TM] Hash port key */
    CTC_SCL_KEY_HASH_PORT_CVLAN,                    /**< [GB.GG.D2.TM] Hash port cvlan key */
    CTC_SCL_KEY_HASH_PORT_SVLAN,                    /**< [GB.GG.D2.TM] Hash port svlan key */
    CTC_SCL_KEY_HASH_PORT_2VLAN,                    /**< [GB.GG.D2.TM] Hash port double vlan key */
    CTC_SCL_KEY_HASH_PORT_CVLAN_COS,            /**< [GB.GG.D2.TM] Hash port  cvlan cos key */
    CTC_SCL_KEY_HASH_PORT_SVLAN_COS,            /**< [GB.GG.D2.TM] Hash port svlan cos key */
    CTC_SCL_KEY_HASH_MAC,                               /**< [GB.GG.D2.TM] Hash mac key */
    CTC_SCL_KEY_HASH_PORT_MAC,                      /**< [GB.GG.D2.TM] Hash port mac key */
    CTC_SCL_KEY_HASH_IPV4,                              /**< [GB.GG.D2.TM] Hash ipv4 key */
    CTC_SCL_KEY_HASH_PORT_IPV4,                     /**< [GB.GG.D2.TM] Hash port ipv4 key */
    CTC_SCL_KEY_HASH_IPV6,                              /**< [GB.GG.D2.TM] Hash ipv6 key */
    CTC_SCL_KEY_HASH_L2,                                 /**< [GB.GG.D2.TM] Hash layer 2 key */
    CTC_SCL_KEY_HASH_SVLAN,                         /**< [D2.TM] Hash svlan key */
    CTC_SCL_KEY_HASH_SVLAN_MAC,                     /**< [D2.TM] Hash svlan mac key */
    CTC_SCL_KEY_HASH_PORT_IPV6,                     /**< [GG.D2.TM] Hash port ipv6 key */
    CTC_SCL_KEY_HASH_PORT_CROSS,                     /**< [D2.TM] Hash port cross key */
    CTC_SCL_KEY_HASH_PORT_SVLAN_DSCP,                /**< [TM] Hash port svlan dscp key */
    CTC_SCL_KEY_HASH_PORT_VLAN_CROSS,                     /**< [D2] Hash port vlan cross key */
    CTC_SCL_KEY_NUM
};
typedef enum ctc_scl_key_type_e ctc_scl_key_type_t;

#define  HASH_KEY

enum  ctc_scl_gport_type_e
{
    CTC_SCL_GPROT_TYPE_PORT,                  /**< [GB.GG.D2.TM] global port */
    CTC_SCL_GPROT_TYPE_PORT_CLASS,       /**< [GB.GG.D2.TM] port class */
    CTC_SCL_GPROT_TYPE_LOGIC_PORT,       /**< [GG.D2.TM] logic port */
    CTC_SCL_GPROT_TYPE_NUM
};
typedef enum ctc_scl_gport_type_e ctc_scl_gport_type_t;

struct ctc_scl_hash_port_key_s
{
    uint32              gport;                            /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;                    /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               dir;                                 /**< [GB.GG.D2.TM] refer to ctc_direction_t */
};
typedef struct ctc_scl_hash_port_key_s ctc_scl_hash_port_key_t;

struct ctc_scl_hash_port_cvlan_key_s
{
    uint32              gport;                         /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;                 /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */

    uint8               dir;                             /**< [GB.GG.D2.TM] refer to ctc_direction_t  */
    uint16              cvlan;                        /**< [GB.GG.D2.TM] cvlan id */
    uint8               rsv0[2];
};
typedef struct ctc_scl_hash_port_cvlan_key_s ctc_scl_hash_port_cvlan_key_t;

struct ctc_scl_hash_port_svlan_key_s
{
    uint32              gport;                     /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;             /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */

    uint8               dir;                         /**< [GB.GG.D2.TM] refer to ctc_direction_t */
    uint16             svlan;                     /**< [GB.GG.D2.TM] svlan id */
    uint8               rsv0[2];
};
typedef struct ctc_scl_hash_port_svlan_key_s ctc_scl_hash_port_svlan_key_t;

struct ctc_scl_hash_port_2vlan_key_s
{
    uint32              gport;                   /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;           /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               dir;                       /**< [GB.GG.D2.TM] refer to ctc_direction_t */

    uint16              svlan;                  /**< [GB.GG.D2.TM] svlan id */
    uint16              cvlan;                  /**< [GB.GG.D2.TM] cvlan id */
};
typedef struct ctc_scl_hash_port_2vlan_key_s ctc_scl_hash_port_2vlan_key_t;

struct ctc_scl_hash_port_cvlan_cos_key_s
{
    uint32              gport;                  /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;          /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               dir;                      /**< [GB.GG.D2.TM] refer to ctc_direction_t */

    uint16              cvlan;                  /**< [GB.GG.D2.TM] cvlan id */
    uint8               ctag_cos;             /**< [GB.GG.D2.TM] C-tag CoS */
    uint8               rsv0;
};
typedef struct ctc_scl_hash_port_cvlan_cos_key_s ctc_scl_hash_port_cvlan_cos_key_t;

struct ctc_scl_hash_port_svlan_cos_key_s
{
    uint32              gport;                    /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;            /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               dir;                        /**< [GB.GG.D2.TM] refer to ctc_direction_t */

    uint16              svlan;                    /**< [GB.GG.D2.TM] svlan id */
    uint8               stag_cos;               /**< [GB.GG.D2.TM] S-tag CoS */
    uint8               rsv0;
};
typedef struct ctc_scl_hash_port_svlan_cos_key_s ctc_scl_hash_port_svlan_cos_key_t;

struct ctc_scl_hash_mac_key_s
{
    mac_addr_t          mac;             /**< [GB.GG.D2.TM] macsa OR macda, based on mac_is_da */
    uint8               mac_is_da;       /**< [GB.GG.D2.TM] mac is macda OR macsa */
    uint8               rsv0;
};
typedef struct ctc_scl_hash_mac_key_s ctc_scl_hash_mac_key_t;

struct ctc_scl_hash_port_mac_key_s
{
    mac_addr_t          mac;                /**< [GB.GG.D2.TM] macsa OR macda, based on mac_is_da */
    uint8               mac_is_da;          /**< [GB.GG.D2.TM] mac is macda OR macsa */
    uint8               rsv0;

    uint32              gport;                 /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;         /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               rsv1;

};
typedef struct ctc_scl_hash_port_mac_key_s ctc_scl_hash_port_mac_key_t;

struct ctc_scl_hash_ipv4_key_s
{
    uint32              ip_sa;                  /**< [GB.GG.D2.TM] IP-SA */
};
typedef struct ctc_scl_hash_ipv4_key_s ctc_scl_hash_ipv4_key_t;

struct ctc_scl_hash_port_ipv4_key_s
{
    uint32              ip_sa;                  /**< [GB.GG.D2.TM] IP-SA */

    uint32              gport;                  /**< [GB.GG.D2.TM] global port value */
    uint8               gport_type;          /**< [GB.GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               rsv0;
};
typedef struct ctc_scl_hash_port_ipv4_key_s ctc_scl_hash_port_ipv4_key_t;

struct ctc_scl_hash_ipv6_key_s
{
    ipv6_addr_t         ip_sa;                  /**< [GB.GG.D2.TM] IP-SA */
};
typedef struct ctc_scl_hash_ipv6_key_s ctc_scl_hash_ipv6_key_t;

struct ctc_scl_hash_port_ipv6_key_s
{
    ipv6_addr_t         ip_sa;                  /**< [GG.D2.TM] IP-SA */

    uint32              gport;                    /**< [GG.D2.TM] global port value */
    uint8               gport_type;            /**< [GG.D2.TM] global port type: CTC_SCL_GPROT_TYPE_XXX */
    uint8               rsv0;
};
typedef struct ctc_scl_hash_port_ipv6_key_s ctc_scl_hash_port_ipv6_key_t;

struct ctc_scl_hash_l2_key_s
{
    uint32               field_sel_id;                  /**< [GG.D2.TM] Field Select Id */
    mac_addr_t          mac_da;                  /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t          mac_sa;                  /**< [GB.GG.D2.TM] MAC-SA */

    uint16              vlan_id;                 /**< [GB.GG.D2.TM] S-VLAN */
    uint32              gport;                   /**< [GB.GG.D2.TM] GG can only used as class id 1-0xFF*/
    uint16              eth_type;            /**< [GG.D2.TM] Ethernet type */
    uint8                cos;                     /**< [GB.GG.D2.TM] S-tag CoS */
    uint8                tag_valid;           /**< [GG.D2.TM] S-tag exist */
    uint8                cfi;                      /**< [GG.D2.TM] S-tag CFI */
    uint8                rsv[3];

};
typedef struct ctc_scl_hash_l2_key_s ctc_scl_hash_l2_key_t;

struct ctc_scl_hash_l2_field_sel_s
{
    uint8 mac_da;                   /**< [GG.D2.TM] L2 key field Select MAC-DA */
    uint8 mac_sa;                   /**< [GG.D2.TM] L2 key field Select MAC-SA */
    uint8 eth_type;                 /**< [GG.D2.TM] L2 key field Select Ethernet type */
    uint8 class_id;                  /**< [GG.D2.TM] L2 key field Select class id */
    uint8 cos;                         /**< [GG.D2.TM] L2 key field Select S-tag CoS */
    uint8 cfi;                          /**< [GG.D2.TM] L2 key field Select S-tag CFI */
    uint8 vlan_id;                   /**< [GG.D2.TM] L2 key field Select S-VLAN */
    uint8 tag_valid;                /**< [GG.D2.TM] L2 key field Select S-tag exist */
};
typedef struct ctc_scl_hash_l2_field_sel_s ctc_scl_hash_l2_field_sel_t;

struct ctc_scl_hash_field_sel_s
{
    uint8  key_type;            /**< [GG.D2.TM] Refer ctc_scl_key_type_t */
    uint32 field_sel_id;        /**< [GG.D2.TM] Field Select Id */

    union
    {
        ctc_scl_hash_l2_field_sel_t   l2;      /**< [GG.D2.TM] L2 key field select */
    }u;

};
typedef struct ctc_scl_hash_field_sel_s ctc_scl_hash_field_sel_t;


#define  TCAM_KEY




/** @brief  Vlan key field flag*/
enum ctc_scl_tcam_vlan_key_flag_e
{
    CTC_SCL_TCAM_VLAN_KEY_FLAG_CVLAN       = 1U << 0,         /**< [GB.GG] Valid C-VLAN in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_CTAG_COS    = 1U << 1,      /**< [GB.GG] Valid C-tag CoS in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_CTAG_CFI    = 1U << 2,       /**< [GB.GG] Valid C-tag CFI in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_SVLAN       = 1U << 3,         /**< [GB.GG] Valid S-VLAN in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_STAG_COS    = 1U << 4,      /**< [GB.GG] Valid S-tag CoS in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_STAG_CFI    = 1U << 5,       /**< [GB.GG] Valid S-tag CFI in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_CTAG_VALID  = 1U << 6,     /**< [GB.GG] Valid C-tag exist in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_STAG_VALID  = 1U << 7,     /**< [GB.GG] Valid S-tag exist in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_CUSTOMER_ID = 1U << 8,    /**< [GB.GG] Valid customer id in vlan key */
    CTC_SCL_TCAM_VLAN_KEY_FLAG_GPORT       = 1U << 9          /**< [GB] Valid gport in vlan key */
};
typedef enum ctc_scl_tcam_vlan_key_flag_e ctc_scl_tcam_vlan_key_flag_t;

/** @brief  Vlan key field */
struct ctc_scl_tcam_vlan_key_s
{
    uint32 flag;                            /**< [GB.GG] Bitmap of CTC_SCL_TCAM_MAC_KEY_FLAG_XXX */

    uint16 cvlan;                          /**< [GB.GG] C-VLAN */
    uint16 cvlan_mask;                /**< [GB.GG] C-VLAN mask*/
    uint16 svlan;                          /**< [GB.GG] S-VLAN */
    uint16 svlan_mask;                /**< [GB.GG] S-VLAN mask*/

    uint8  ctag_cos;                    /**< [GB.GG] C-tag CoS */
    uint8  ctag_cos_mask;           /**< [GB.GG] C-tag CoS mask*/
    uint8  ctag_cfi;                      /**< [GB.GG] C-tag CFI */
    uint8  ctag_valid;                  /**< [GB.GG] Ctag valid*/

    uint8  stag_cos;                    /**< [GB.GG] S-tag CoS */
    uint8  stag_cos_mask;           /**< [GB.GG] S-tag CoS mask*/
    uint8  stag_cfi;                      /**< [GB.GG] S-tag CFI */
    uint8  stag_valid;                  /**< [GB.GG] Stag valid*/
    uint32 customer_id;              /**< [GB.GG] Customer id, used for decap vpn label with iloop */
    uint32 customer_id_mask;    /**< [GB.GG] Customer id mask */
    uint32 gport;                        /**< [GB] Gport, used for decap vpn label with iloop */
    uint32 gport_mask;               /**< [GB] Gport mask */

    ctc_field_port_t port_data;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
    ctc_field_port_t port_mask;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
};
typedef struct ctc_scl_tcam_vlan_key_s ctc_scl_tcam_vlan_key_t;

/** @brief  Mac key field flag*/
enum ctc_scl_tcam_mac_key_flag_e
{
    CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_DA      = 1U << 0,         /**< [GB.GG.D2.TM] Valid MAC-DA in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_SA      = 1U << 1,         /**< [GB.GG.D2.TM] Valid MAC-SA in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_CVLAN       = 1U << 2,          /**< [GB.GG.D2.TM] Valid C-VLAN in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_COS    = 1U << 3,         /**< [GB.GG.D2.TM] Valid C-tag CoS in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_CFI    = 1U << 4,         /**< [GB.GG.D2.TM] Valid C-tag CFI in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_SVLAN       = 1U << 5,          /**< [GB.GG.D2.TM] Valid S-VLAN in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_COS    = 1U << 6,         /**< [GB.GG.D2.TM] Valid S-tag CoS in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_CFI    = 1U << 7,         /**< [GB.GG.D2.TM] Valid S-tag CFI in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_L2_TYPE     = 1U << 8,         /**< [GB.GG.D2.TM] Valid l2-type in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_L3_TYPE     = 1U << 9,         /**< [GB.GG.D2.TM] Valid l3-type in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_VALID  = 1U <<10,         /**< [GG.D2.TM] Valid C-tag exist in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_VALID  = 1U <<11,         /**< [GG.D2.TM] Valid S-tag exist in mac key */
    CTC_SCL_TCAM_MAC_KEY_FLAG_ETH_TYPE    = 1U <<12         /**< [GG.D2.TM] Valid Ethernet type in mac key */
};
typedef enum ctc_scl_tcam_mac_key_flag_e ctc_scl_tcam_mac_key_flag_t;

/** @brief  Mac key field */
struct ctc_scl_tcam_mac_key_s
{
    uint32 flag;                                   /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_TCAM_MAC_KEY_FLAG_XXX */

    mac_addr_t mac_da;                     /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;           /**< [GB.GG.D2.TM] MAC-DA mask */
    mac_addr_t mac_sa;                     /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;           /**< [GB.GG.D2.TM] MAC-SA mask */

    uint16 cvlan;                               /**< [GB.GG.D2.TM] C-VLAN */
    uint16 cvlan_mask;                     /**< [GB.GG.D2.TM] C-VLAN mask*/
    uint16 svlan;                               /**< [GB.GG.D2.TM] S-VLAN */
    uint16 svlan_mask;                     /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  ctag_cos;                          /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;                /**< [GB.GG.D2.TM] C-tag CoS mask*/
    uint8  ctag_cfi;                           /**< [GB.GG.D2.TM] C-tag CFI */
    uint8  ctag_valid;                       /**< [GG.D2.TM] C-tag exist */

    uint8  stag_cos;                         /**< [GB.GG.D2.TM] S-tag CoS */
    uint8  stag_cos_mask;               /**< [GB.GG.D2.TM] S-tag CoS mask*/
    uint8  stag_cfi;                          /**< [GB.GG.D2.TM] S-tag CFI */
    uint8  stag_valid;                      /**< [GG.D2.TM] S-tag exist */

    uint8  l2_type;                          /**< [GB.GG.D2.TM] Layer 2 type. Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;                /**< [GB.GG.D2.TM] Layer 2 type mask*/
    uint8  l3_type;                          /**< [GB] Layer 3 type. Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;                /**< [GB] Layer 3 type mask*/

    uint16 eth_type;                       /**< [GG.D2.TM] Ethernet type */
    uint16 eth_type_mask;             /**< [GG.D2.TM] Ethernet type mask*/

    ctc_field_port_t port_data;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
    ctc_field_port_t port_mask;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
} ;
typedef struct ctc_scl_tcam_mac_key_s ctc_scl_tcam_mac_key_t;

enum ctc_scl_tcam_ipv4_key_flag_e
{

    CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_DA        = 1U << 0,  /**< [GB.GG.D2.TM] Valid MAC-DA in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_SA        = 1U << 1,  /**< [GB.GG.D2.TM] Valid MAC-SA in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_CVLAN         = 1U << 2,  /**< [GB.GG.D2.TM] Valid C-VLAN in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_COS      = 1U << 3,  /**< [GB.GG.D2.TM] Valid C-tag CoS in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_CFI      = 1U << 4,  /**< [GB.GG.D2.TM] Valid C-tag CFI in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_SVLAN         = 1U << 5,  /**< [GB.GG.D2.TM] Valid S-VLAN in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_COS      = 1U << 6,  /**< [GB.GG.D2.TM] Valid S-tag CoS in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_CFI      = 1U << 7,  /**< [GB.GG.D2.TM] Valid S-tag CFI in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_L2_TYPE       = 1U << 8,  /**< [GB.GG.D2.TM] Valid l2-type in IPv4 key.*/
    CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE       = 1U << 9,  /**< [GB.GG.D2.TM] Valid l3-type in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_SA         = 1U << 10, /**< [GB.GG.D2.TM] Valid IP-SA in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_DA         = 1U << 11, /**< [GB.GG.D2.TM] Valid IP-DA in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_PROTOCOL   = 1U << 12, /**< [GB.GG.D2.TM] Valid L4-Proto in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_DSCP          = 1U << 13, /**< [GB.GG.D2.TM] Valid DSCP in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_PRECEDENCE    = 1U << 14, /**< [GB.GG.D2.TM] Valid IP Precedence in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_FRAG       = 1U << 15, /**< [GB.GG.D2.TM] Valid fragment in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_OPTION     = 1U << 16, /**< [GB.GG.D2.TM] Valid IP option in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_TYPE       = 1U << 17, /**< [GB.GG.D2.TM] Valid l4-type in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_ETH_TYPE      = 1U << 18, /**< [GB.GG.D2.TM] Valid Ether type in IPv4 Key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID    = 1U << 19, /**< [GG.D2.TM] Valid Stag-valid in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID    = 1U << 20, /**< [GG.D2.TM] Valid Ctag-valid in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_FLAG_ECN           = 1U << 21  /**< [GG.D2.TM] Valid ECN in IPv4 key */

};
typedef enum ctc_scl_tcam_ipv4_key_flag_e ctc_scl_tcam_ipv4_key_flag_t;

enum ctc_scl_tcam_ipv4_key_sub_flag_e
{
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT       = 1U << 0, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP. Valid L4 source port in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT       = 1U << 1, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP. Valid L4 destination port in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_TYPE         = 1U << 2, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP. Valid ICMP type in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_CODE         = 1U << 3, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP. Valid ICMP code in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_IGMP_TYPE         = 1U << 4, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = IGMP. Valid IGMP type in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_OP_CODE       = 1U << 5, /**< [GB.GG.D2.TM] (GB)Depend on ARP_PACKET. (GG)Depend on L3_TYPE. Valid Arp op code in Arp Key. */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP     = 1U << 6, /**< [GB.GG.D2.TM] (GB)Depend on ARP_PACKET. (GG)Depend on L3_TYPE. Valid Arp sender Ip in Arp Key. */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP     = 1U << 7, /**< [GB.GG.D2.TM] (GB)Depend on ARP_PACKET. (GG)Depend on L3_TYPE. Valid Arp target Ip in Arp Key. */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE = 1U << 8, /**< [GG.D2.TM] (GG)Depend on L3_TYPE. Valid Arp target Ip in Arp Key. */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_GRE_KEY           = 1U << 9,  /**< [GG.D2.TM] Depend on L4_PROTOCOL = GRE. Valid GRE key in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_NVGRE_KEY           = 1U << 10,  /**< [GG.D2.TM] Depend on L4_PROTOCOL = GRE. Valid NVGRE key in IPv4 key */
    CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_VNI           = 1U << 11  /**< [GG.D2.TM] Valid vni in IPv4 key for vxlan */
};
typedef enum ctc_scl_tcam_ipv4_key_sub_flag_e ctc_scl_tcam_ipv4_key_sub_flag_t;

struct ctc_scl_tcam_ipv4_key_s
{
    uint32 flag;                        /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_TCAM_IPV4_KEY_FLAG_XXX */
    uint32 sub_flag;                    /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_XXX */
    uint8  key_size;                    /**< [GG.D2.TM] CTC_SCL_KEY_SIZE_SINGLE or CTC_SCL_KEY_SIZE_DOUBLE */

    uint32 ip_sa;                       /**< [GB.GG.D2.TM] IP-SA */
    uint32 ip_sa_mask;                  /**< [GB.GG.D2.TM] IP-SA mask */
    uint32 ip_da;                       /**< [GB.GG.D2.TM] IP-DA */
    uint32 ip_da_mask;                  /**< [GB.GG.D2.TM] IP-DA mask */

    mac_addr_t mac_sa;                  /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;             /**< [GB.GG.D2.TM] MAC-SA mask */
    mac_addr_t mac_da;                  /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;             /**< [GB.GG.D2.TM] MAC-DA mask */


    uint16 cvlan;                       /**< [GB.GG.D2.TM] C-VLAN */
    uint16 cvlan_mask;                  /**< [GB.GG.D2.TM] C-VLAN mask*/
    uint16 svlan;                       /**< [GB.GG.D2.TM] S-VLAN */
    uint16 svlan_mask;                  /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  ctag_cos;                    /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;               /**< [GB.GG.D2.TM] C-tag CoS mask*/
    uint8  stag_cos;                    /**< [GB.GG.D2.TM] S-tag CoS */
    uint8  stag_cos_mask;               /**< [GB.GG.D2.TM] S-tag CoS mask*/

    uint8  ctag_cfi;                    /**< [GB.GG.D2.TM] C-tag CFI */
    uint8  stag_cfi;                    /**< [GB.GG.D2.TM] S-tag CFI */
    uint8  rsv0[2];

    uint16 eth_type;                    /**< [GB.GG.D2.TM] Ethernet type */
    uint16 eth_type_mask;               /**< [GB.GG.D2.TM] Ethernet type mask*/

    uint16 l4_src_port;                 /**< [GB.GG.D2.TM] Layer 4 source port.Min/Data */
    uint16 l4_src_port_mask;            /**< [GB.GG.D2.TM] Layer 4 source port.Max/Mask */
    uint16 l4_dst_port;                 /**< [GB.GG.D2.TM] Layer 4 dest port */
    uint16 l4_dst_port_mask;            /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint8  l2_type;                     /**< [GB.GG.D2.TM] Layer 2 type. Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;                /**< [GB.GG.D2.TM] Layer 2 type */
    uint8  l3_type;                     /**< [GB.GG.D2.TM] Layer 3 type. Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;                /**< [GB.GG.D2.TM] Layer 3 type */

    uint8  l4_type;                     /**< [GB] Layer 4 type. Refer to ctc_parser_l4_type_t */
    uint8  l4_type_mask;                /**< [GB] Layer 4 type */
    uint8  dscp;                        /**< [GB.GG.D2.TM] DSCP or Ip precedence */
    uint8  dscp_mask;                   /**< [GB.GG.D2.TM] DSCP or Ip precedence mask */

    uint8  ip_frag;                     /**< [GB.GG.D2.TM] Ip fragment, CTC_IP_FRAG_XXX */
    uint8  ip_option;                   /**< [GB.GG.D2.TM] Ip option */
    uint8  rsv1;


    uint8  l4_protocol;                 /**< [GB.GG.D2.TM] Layer 4 protocol */
    uint8  l4_protocol_mask;            /**< [GB.GG.D2.TM] Layer 4 protocol protocol*/
    uint8  igmp_type;                   /**< [GB.GG.D2.TM] IGMP type */
    uint8  igmp_type_mask;              /**< [GB.GG.D2.TM] IGMP type mask*/

    uint8  icmp_type;                   /**< [GB.GG.D2.TM] ICMP type */
    uint8  icmp_type_mask;              /**< [GB.GG.D2.TM] ICMP type mask*/
    uint8  icmp_code;                   /**< [GB.GG.D2.TM] ICMP code */
    uint8  icmp_code_mask;              /**< [GB.GG.D2.TM] ICMP code mask*/

    uint32 sender_ip;                   /**< [GG.D2.TM] Sender ip */
    uint32 sender_ip_mask;              /**< [GG.D2.TM] Sender ip mask */
    uint32 target_ip;                   /**< [GG.D2.TM] Target ip */
    uint32 target_ip_mask;              /**< [GG.D2.TM] Target ip mask */

    uint16 arp_op_code;                 /**< [GG.D2.TM] Arp op code*/
    uint16 arp_op_code_mask;            /**< [GG.D2.TM] Arp op code mask*/

    uint8  ecn;                         /**< [GG.D2.TM] ECN */
    uint8  ecn_mask;                    /**< [GG.D2.TM] ECN mask */

    uint8  stag_valid;                  /**< [GG.D2.TM] S-tag exist */
    uint8  ctag_valid;                  /**< [GG.D2.TM] C-tag exist */

    uint16 arp_protocol_type;           /**< [GG.D2.TM] Arp protocol type */
    uint16 arp_protocol_type_mask;      /**< [GG.D2.TM] Arp protocol type mask */














    uint32 gre_key;                     /**< [GG.D2.TM] GRE key */
    uint32 gre_key_mask;                /**< [GG.D2.TM] GRE key mask */

    uint32 vni;                     /**< [GG.D2.TM] vni value */
    uint32 vni_mask;                /**< [GG.D2.TM] vni value mask */

    ctc_field_port_t port_data;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
    ctc_field_port_t port_mask;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
} ;
typedef struct ctc_scl_tcam_ipv4_key_s ctc_scl_tcam_ipv4_key_t;


enum ctc_scl_tcam_ipv6_key_flag_e
{
    CTC_SCL_TCAM_IPV6_KEY_FLAG_MAC_DA        = 1U << 0,  /**< [GB.GG.D2.TM] Valid MAC-DA in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_MAC_SA        = 1U << 1,  /**< [GB.GG.D2.TM] Valid MAC-SA in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_CVLAN         = 1U << 2,  /**< [GB.GG.D2.TM] Valid CVLAN in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_COS      = 1U << 3,  /**< [GB.GG.D2.TM] Valid Ctag CoS in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_CFI      = 1U << 4,  /**< [GB.GG.D2.TM] Valid C-tag Cfi in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_SVLAN         = 1U << 5,  /**< [GB.GG.D2.TM] Valid SVLAN in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_COS      = 1U << 6,  /**< [GB.GG.D2.TM] Valid Stag CoS in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_CFI      = 1U << 7,  /**< [GB.GG.D2.TM] Valid S-tag Cfi in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_ETH_TYPE      = 1U << 8,  /**< [GB.GG.D2.TM] Valid Ether-type in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_L2_TYPE       = 1U << 9,  /**< [GB.GG.D2.TM] Valid L2-type CoS in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE       = 1U << 10, /**< [GB.GG.D2.TM] Valid L3-type in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_TYPE       = 1U << 11, /**< [GB.GG.D2.TM] Valid L4-type in IPv6 key*/
    CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_SA         = 1U << 12, /**< [GB.GG.D2.TM] Valid IPv6-SA in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_DA         = 1U << 13, /**< [GB.GG.D2.TM] Valid IPv6-DA in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_PROTOCOL   = 1U << 14, /**< [GB.GG.D2.TM] Valid L4-Protocol in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_DSCP          = 1U << 15, /**< [GB.GG.D2.TM] Valid DSCP in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_PRECEDENCE    = 1U << 16, /**< [GB.GG.D2.TM] Valid IP Precedence in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_FRAG       = 1U << 17, /**< [GB.GG.D2.TM] Valid fragment in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_OPTION     = 1U << 18, /**< [GB.GG.D2.TM] Valid IP option in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_FLOW_LABEL    = 1U << 19, /**< [GB.GG.D2.TM] Valid IPv6 flow label in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_VALID    = 1U << 20, /**< [GG.D2.TM] Valid Stag-valid in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_VALID    = 1U << 21, /**< [GG.D2.TM] Valid Ctag-valid in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_FLAG_ECN           = 1U << 22  /**< [GG.D2.TM] */
};
typedef enum ctc_scl_tcam_ipv6_key_flag_e ctc_scl_tcam_ipv6_key_flag_t;

enum ctc_scl_tcam_ipv6_key_sub_flag_e
{
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_SRC_PORT   = 1U << 0, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP Valid L4 source port in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_DST_PORT   = 1U << 1, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = TCP|UDP Valid L4 destination port in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_ICMP_TYPE     = 1U << 2, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP.   Valid ICMP type in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_ICMP_CODE     = 1U << 3, /**< [GB.GG.D2.TM] Depend on L4_PROTOCOL = ICMP.   Valid ICMP code in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_GRE_KEY       = 1U << 4,  /**< [GG.D2.TM] Depend on L4_PROTOCOL = GRE.  Valid GRE key in IPv6 key*/
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_NVGRE_KEY           = 1U << 5,  /**< [GG.D2.TM] Depend on L4_PROTOCOL = GRE. Valid NVGRE key in IPv6 key */
    CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_VNI           = 1U << 6  /**< [GG.D2.TM] Valid vni key in IPv6 key for vxlan */
};
typedef enum ctc_scl_tcam_ipv6_key_sub_flag_e ctc_scl_tcam_ipv6_key_sub_flag_t;

struct ctc_scl_tcam_ipv6_key_s
{
    uint32 flag;                        /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_TCAM_IPV6_KEY_FLAG_XXX */
    uint32 sub_flag;                    /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_XXX */

    ipv6_addr_t ip_sa;                  /**< [GB.GG.D2.TM] IPv6-SA */
    ipv6_addr_t ip_sa_mask;             /**< [GB.GG.D2.TM] IPv6-SA mask */
    ipv6_addr_t ip_da;                  /**< [GB.GG.D2.TM] IPv6-DA */
    ipv6_addr_t ip_da_mask;             /**< [GB.GG.D2.TM] IPv6-DA mask */

    mac_addr_t mac_sa;                  /**< [GB.GG.D2.TM] MAC-SA */
    mac_addr_t mac_sa_mask;             /**< [GB.GG.D2.TM] MAC-SA mask */
    mac_addr_t mac_da;                  /**< [GB.GG.D2.TM] MAC-DA */
    mac_addr_t mac_da_mask;             /**< [GB.GG.D2.TM] MAC-DA mask */

    uint16 l4_src_port;                 /**< [GB.GG.D2.TM] Layer 4 source port.Min/Data */
    uint16 l4_src_port_mask;            /**< [GB.GG.D2.TM] Layer 4 source port.Max/Mask */
    uint16 l4_dst_port;                 /**< [GB.GG.D2.TM] Layer 4 dest port */
    uint16 l4_dst_port_mask;            /**< [GB.GG.D2.TM] Layer 4 dest port */

    uint16 cvlan;                       /**< [GB.GG.D2.TM] C-VLAN */
    uint16 cvlan_mask;                  /**< [GB.GG.D2.TM] C-VLAN mask*/
    uint16 svlan;                       /**< [GB.GG.D2.TM] S-VLAN */
    uint16 svlan_mask;                  /**< [GB.GG.D2.TM] S-VLAN mask*/

    uint8  ctag_cos;                    /**< [GB.GG.D2.TM] C-tag CoS */
    uint8  ctag_cos_mask;               /**< [GB.GG.D2.TM] C-tag CoS mask */
    uint8  stag_cos;                    /**< [GB.GG.D2.TM] S-tag CoS */
    uint8  stag_cos_mask;               /**< [GB.GG.D2.TM] S-tag Cos mask */

    uint8  ctag_cfi;                    /**< [GB.GG.D2.TM] C-tag CFI */
    uint8  stag_cfi;                    /**< [GB.GG.D2.TM] S-tag CFI */
    uint8  l2_type;                     /**< [GB.GG.D2.TM] Layer 2 type. Refer to ctc_parser_l2_type_t */
    uint8  l2_type_mask;                /**< [GB.GG.D2.TM] Layer 2 type mask*/

    uint8  l3_type;                     /**< [GB.GG.D2.TM] Layer 3 type. Refer to ctc_parser_l3_type_t */
    uint8  l3_type_mask;                /**< [GB.GG.D2.TM] Layer 3 type mask*/
    uint8  l4_type;                     /**< [GB.GG.D2.TM] Layer 4 type. Refer to ctc_parser_l4_type_t */
    uint8  l4_type_mask;                /**< [GB.GG.D2.TM] Layer 4 type mask*/

    uint8  ip_option;                   /**< [GB.GG.D2.TM] IP option */
    uint8  ip_frag;                     /**< [GB.GG.D2.TM] Ip fragment, CTC_IP_FRAG_XXX */
    uint8  rsv0[2];

    uint8  l4_protocol;                 /**< [GB.GG.D2.TM] Layer 4 protocol */
    uint8  l4_protocol_mask;            /**< [GB.GG.D2.TM] Layer 4 protocol mask*/
    uint8  dscp;                        /**< [GB.GG.D2.TM] DSCP */
    uint8  dscp_mask;                   /**< [GB.GG.D2.TM] DSCP mask */

    uint8  icmp_type;                   /**< [GB.GG.D2.TM] ICMP type */
    uint8  icmp_type_mask;              /**< [GB.GG.D2.TM] ICMP type mask*/

    uint8  icmp_code;                   /**< [GB.GG.D2.TM] ICMP code */
    uint8  icmp_code_mask;              /**< [GB.GG.D2.TM] ICMP code mask*/

    uint16 eth_type;                    /**< [GB.GG.D2.TM] Ethernet type */
    uint16 eth_type_mask;               /**< [GB.GG.D2.TM] Ethernet type mask */

    uint32 flow_label;                  /**< [GB.GG.D2.TM] Flow label */
    uint32 flow_label_mask;             /**< [GB.GG.D2.TM] Flow label */

    uint8  ecn;                         /**< [GG.D2.TM] ECN */
    uint8  ecn_mask;                    /**< [GG.D2.TM] ECN mask */

    uint8  stag_valid;                  /**< [GG.D2.TM] S-tag exist */
    uint8  ctag_valid;                  /**< [GG.D2.TM] C-tag exist */

    uint32 gre_key;                     /**< [GG.D2.TM] GRE key */
    uint32 gre_key_mask;                /**< [GG.D2.TM] GRE key mask */

    uint32 vni;                     /**< [GG.D2.TM] VNI id*/
    uint32 vni_mask;                /**< [GG.D2.TM] VNI id mask*/

    ctc_field_port_t port_data;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
    ctc_field_port_t port_mask;              /**< [GG.D2.TM] valid when group type CTC_SCL_GROUP_TYPE_NONE, refer to struct ctc_field_port_t */
} ;
typedef struct ctc_scl_tcam_ipv6_key_s ctc_scl_tcam_ipv6_key_t;


struct ctc_scl_key_s
{
    ctc_scl_key_type_t type;                 /**< [GB.GG.D2.TM] CTC_SCL_KEY_XXX */
    uint8 rsv;

    union
    {
        ctc_scl_tcam_vlan_key_t             tcam_vlan_key;            /**<[GB.GG.D2.TM] SCL VLAN key content */
        ctc_scl_tcam_mac_key_t              tcam_mac_key;             /**<[GB.GG.D2.TM] SCL MAC  key content */
        ctc_scl_tcam_ipv4_key_t             tcam_ipv4_key;            /**<[GB.GG.D2.TM] SCL IPv4 key content */
        ctc_scl_tcam_ipv6_key_t             tcam_ipv6_key;            /**<[GB.GG.D2.TM] SCL IPv6 key content */

        ctc_scl_hash_port_key_t             hash_port_key;            /**<[GB.GG.D2.TM] SCL port hash key content */
        ctc_scl_hash_port_cvlan_key_t       hash_port_cvlan_key;      /**<[GB.GG.D2.TM] SCL port c-vlan hash key content */
        ctc_scl_hash_port_svlan_key_t       hash_port_svlan_key;      /**<[GB.GG.D2.TM] SCL port s-vlan hash key content */
        ctc_scl_hash_port_2vlan_key_t       hash_port_2vlan_key;      /**<[GB.GG.D2.TM] SCL port double vlan hash key content */
        ctc_scl_hash_port_cvlan_cos_key_t   hash_port_cvlan_cos_key;  /**<[GB.GG.D2.TM] SCL port c-vlan cos hash key content */
        ctc_scl_hash_port_svlan_cos_key_t   hash_port_svlan_cos_key;  /**<[GB.GG.D2.TM] SCL port s-vlan cos hash key content */
        ctc_scl_hash_port_mac_key_t         hash_port_mac_key;        /**<[GB.GG.D2.TM] SCL port mac hash key content */
        ctc_scl_hash_port_ipv4_key_t        hash_port_ipv4_key;       /**<[GB.GG.D2.TM] SCL port ipv4 hash key content */
        ctc_scl_hash_mac_key_t              hash_mac_key;             /**<[GB.GG.D2.TM] SCL mac hash key content */
        ctc_scl_hash_ipv4_key_t             hash_ipv4_key;            /**<[GB.GG.D2.TM] SCL ipv4 hash key content */
        ctc_scl_hash_ipv6_key_t             hash_ipv6_key;            /**<[GB.GG.D2.TM] SCL ipv6 hash key content */
        ctc_scl_hash_port_ipv6_key_t        hash_port_ipv6_key;       /**<[GG.D2.TM] SCL port ipv6 hash key content */
        ctc_scl_hash_l2_key_t               hash_l2_key;              /**<[GB.GG.D2.TM] SCL layer 2 hash key content */
    } u;
};
typedef struct ctc_scl_key_s ctc_scl_key_t;

enum ctc_scl_action_type_e
{
    CTC_SCL_ACTION_INGRESS,     /**<[GB.GG.D2.TM] Ingress SCL action */
    CTC_SCL_ACTION_EGRESS,      /**<[GB.GG.D2.TM] Engress SCL action */
    CTC_SCL_ACTION_FLOW,        /**<[GG.D2.TM] SCL flow action*/
    CTC_SCL_ACTION_NUM
};
typedef enum ctc_scl_action_type_e ctc_scl_action_type_t;


#define ACTION
enum ctc_scl_vlan_domain_e
{
    CTC_SCL_VLAN_DOMAIN_SVLAN,    /**< [GB.GG.D2.TM] svlan domain */
    CTC_SCL_VLAN_DOMAIN_CVLAN,    /**< [GB.GG.D2.TM] cvlan domain */
    CTC_SCL_VLAN_DOMAIN_UNCHANGE,  /**< [GB.GG.D2.TM] keep unchange */
    CTC_SCL_VLAN_DOMAIN_MAX
};
typedef enum ctc_scl_vlan_domain_e ctc_scl_vlan_domain_t;


/**
 @brief [GB.GG] define acl vlan edit operation
*/
struct ctc_scl_vlan_edit_s
{
    uint8  stag_op;         /**< [GB.GG.D2.TM] operation type of stag, see CTC_VLAN_TAG_OP_XXX  */
    uint8  svid_sl;         /**< [GB.GG.D2.TM] select type of svid, see CTC_VLAN_TAG_SL_XXX  */
    uint8  scos_sl;         /**< [GB.GG.D2.TM] select type of scos, see CTC_VLAN_TAG_SL_XXX  */
    uint8  scfi_sl;         /**< [GB.GG.D2.TM] select type of scfi, see CTC_VLAN_TAG_SL_XXX  */
    uint16 svid_new;        /**< [GB.GG.D2.TM] new svid, valid only if svid_sl is CTC_VLAN_TAG_OP_REP|CTC_VLAN_TAG_OP_REP_OR_ADD|CTC_VLAN_TAG_OP_ADD*/
    uint8  scos_new;        /**< [GB.GG.D2.TM] new scos, valid only if scos_sl is CTC_VLAN_TAG_OP_REP|CTC_VLAN_TAG_OP_REP_OR_ADD|CTC_VLAN_TAG_OP_ADD*/
    uint8  scfi_new;        /**< [GB.GG.D2.TM] new scfi, valid only if scfi_sl is CTC_VLAN_TAG_OP_REP|CTC_VLAN_TAG_OP_REP_OR_ADD|CTC_VLAN_TAG_OP_ADD*/

    uint8  ctag_op;         /**< [GB.GG.D2.TM] operation type of ctag, see CTC_VLAN_TAG_OP_XXX */
    uint8  cvid_sl;         /**< [GB.GG.D2.TM] select type of cvid, see CTC_VLAN_TAG_SL_XXX */
    uint8  ccos_sl;         /**< [GB.GG.D2.TM] select type of ccos, see CTC_VLAN_TAG_SL_XXX */
    uint8  ccfi_sl;         /**< [GB.GG.D2.TM] select type of ccfi, see CTC_VLAN_TAG_SL_XXX */
    uint16 cvid_new;        /**< [GB.GG.D2.TM] new cvid, valid only if cvid_sl is CTC_VLAN_TAG_OP_REP|CTC_VLAN_TAG_OP_REP_OR_ADD|CTC_VLAN_TAG_OP_ADD*/
    uint8  ccos_new;        /**< [GB.GG.D2.TM] new ccos, valid only if ccos_sl is CTC_VLAN_TAG_OP_REP|CTC_VLAN_TAG_OP_REP_OR_ADD|CTC_VLAN_TAG_OP_ADD*/
    uint8  ccfi_new;        /**< [GB.GG.D2.TM] new ccfi, valid only if ccfi_sl is CTC_VLAN_TAG_OP_REP|CTC_VLAN_TAG_OP_REP_OR_ADD|CTC_VLAN_TAG_OP_ADD*/

    uint8  vlan_domain;     /**< [GB.GG.D2.TM] refer ctc_scl_vlan_domain_t. [ingress] */
    uint8  tpid_index;      /**< [GB.GG.D2.TM] svlan tpid index*/

};
typedef struct ctc_scl_vlan_edit_s ctc_scl_vlan_edit_t;


struct ctc_scl_query_s
{
    uint32  group_id;   /* [GB.GG.D2.TM] [in]     SCL group ID*/
    uint32  entry_size; /* [GB.GG.D2.TM] [in]     The maximum number of entry IDs to return. If 0, the number of entries is returned*/
    uint32* entry_array;/* [GB.GG.D2.TM] [in|out] A pointer to a memory buffer to hold the array of IDs*/
    uint32  entry_count;/* [GB.GG.D2.TM] [out]    Number of entries returned or. if entry_size  is 0, the number of entries available.*/
};
typedef struct ctc_scl_query_s ctc_scl_query_t;

enum ctc_scl_igs_action_flag_e
{
    CTC_SCL_IGS_ACTION_FLAG_DISCARD              = 1U << 0,  /**< [GB.GG.D2.TM] Discard the packet */
    CTC_SCL_IGS_ACTION_FLAG_STATS                = 1U << 1,  /**< [GB.GG.D2.TM] Statistic */
    CTC_SCL_IGS_ACTION_FLAG_PRIORITY_AND_COLOR   = 1U << 2,  /**< [GB.GG.D2.TM] Priority color */
    CTC_SCL_IGS_ACTION_FLAG_COPY_TO_CPU          = 1U << 3,  /**< [GB.GG.D2.TM] Copy to cpu */
    CTC_SCL_IGS_ACTION_FLAG_REDIRECT             = 1U << 4,  /**< [GB.GG.D2.TM] Redirect */
    CTC_SCL_IGS_ACTION_FLAG_AGING                = 1U << 5,  /**< [GB] Aging */
    CTC_SCL_IGS_ACTION_FLAG_FID                  = 1U << 6,  /**< [GB.GG.D2.TM] FID */
    CTC_SCL_IGS_ACTION_FLAG_VLAN_EDIT            = 1U << 7,  /**< [GB.GG.D2.TM] Vlan edit */
    CTC_SCL_IGS_ACTION_FLAG_LOGIC_PORT           = 1U << 8,  /**< [GB.GG.D2.TM] Logic port */
    CTC_SCL_IGS_ACTION_FLAG_BINDING_EN           = 1U << 9,  /**< [GB.GG.D2.TM] Enable binding */
    CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR         = 1U << 10, /**< [GB.GG.D2.TM] User vlan ptr */
    CTC_SCL_IGS_ACTION_FLAG_VPLS                 = 1U << 11, /**< [GB] VPLS */
    CTC_SCL_IGS_ACTION_FLAG_APS                  = 1U << 12, /**< [GB.GG.D2.TM] APS */
    CTC_SCL_IGS_ACTION_FLAG_ETREE_LEAF           = 1U << 13, /**< [GB.GG.D2.TM] Etree leaf */
    CTC_SCL_IGS_ACTION_FLAG_SERVICE_ID           = 1U << 14, /**< [GB.D2.TM] Service ID */
    CTC_SCL_IGS_ACTION_FLAG_IGMP_SNOOPING        = 1U << 15, /**< [GB] IGMP snooping */
    CTC_SCL_IGS_ACTION_FLAG_STP_ID               = 1U << 16, /**< [GG.D2.TM] STP ID */
    CTC_SCL_IGS_ACTION_FLAG_VPLS_OAM             = 1U << 17, /**< [GG.D2.TM] indecate VPLS OAM if set in L2VPN OAM, otherwise VPWS*/
    CTC_SCL_IGS_ACTION_FLAG_PRIORITY             = 1U << 18, /**< [GG.D2.TM] Priority */
    CTC_SCL_IGS_ACTION_FLAG_COLOR                = 1U << 19, /**< [GG.D2.TM] Color */
    CTC_SCL_IGS_ACTION_FLAG_DSCP                 = 1U << 20, /**< [GG.D2.TM] Dscp */
    CTC_SCL_IGS_ACTION_FLAG_L2VPN_OAM            = 1U << 21, /**< [GG.D2.TM] indecate L2VPN OAM, need config l2vpn_oam_id*/
    CTC_SCL_IGS_ACTION_FLAG_VRFID                = 1U << 22,  /**< [GB.GG.D2.TM] Vrf ID of the route*/
    /* SYSTEM MODIFIED xuwj fix bug start */     
    CTC_SCL_IGS_ACTION_FLAG_DENY_BRIDGE           = 1U << 23,
    /* SYSTEM MODIFIED xuwj fix bug end */  
};
typedef enum ctc_scl_igs_action_flag_e ctc_scl_igs_action_flag_t;

enum ctc_scl_igs_action_sub_flag_e
{
    CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_ACL_EN      = 1U << 1,   /**< [GB.GG.D2.TM] Depend on FLAG_SERVICE_ID. */
    CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_QUEUE_EN    = 1U << 2,   /**< [GB.D2.TM] Depend on FLAG_SERVICE_ID. need binding logic port*/
    CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_POLICER_EN  = 1U << 3    /**< [GB.GG.D2.TM] Depend on FLAG_SERVICE_ID. */
};
typedef enum ctc_scl_igs_action_sub_flag_e ctc_scl_igs_action_sub_flag_t;

enum ctc_scl_bind_type_e
{
    CTC_SCL_BIND_TYPE_PORT,          /**< [GB.GG.D2.TM] Port bind */
    CTC_SCL_BIND_TYPE_VLAN,          /**< [GB.GG.D2.TM] Vlan bind */
    CTC_SCL_BIND_TYPE_IPV4SA,        /**< [GB.GG.D2.TM] Ipv4-SA bind */
    CTC_SCL_BIND_TYPE_IPV4SA_VLAN,   /**< [GB.GG.D2.TM] Ipv4-SA and Vlan bind */
    CTC_SCL_BIND_TYPE_MACSA,         /**< [GB.GG.D2.TM] MAC-SA bind */
    CTC_SCL_BIND_TYPE_MAX            /**< [GB.GG.D2.TM] Max bind type */
};
typedef enum ctc_scl_bind_type_e ctc_scl_bind_type_t;

struct ctc_scl_bind_s
{
    ctc_scl_bind_type_t type;        /**< [GB.GG.D2.TM] refer ctc_scl_bind_type_t */
    mac_addr_t          mac_sa;      /**< [GB.GG.D2.TM] MAC-SA */
    uint32              ipv4_sa;     /**< [GB.GG.D2.TM]  Ipv4-SA */
    uint32              gport;       /**< [GB.GG.D2.TM] Global port */
    uint16              vlan_id;     /**< [GB.GG.D2.TM] VLAN ID*/
};
typedef struct ctc_scl_bind_s ctc_scl_bind_t;

struct ctc_scl_aps_s
{
    uint16   protected_vlan;         /**< [GB.GG.D2.TM] Protected vlan */
    uint16   aps_select_group_id;    /**< [GB.GG.D2.TM] APS select group id*/
    uint8    is_working_path;        /**< [GB.GG.D2.TM] is working path */
    uint8    protected_vlan_valid;   /**< [GB.GG.D2.TM] Protected vlan valid */
};
typedef struct ctc_scl_aps_s ctc_scl_aps_t;

struct ctc_scl_vpls_s
{
    uint8 mac_limit_en;             /**< [GB] Enable mac limit */
    uint8 learning_en;              /**< [GB.D2.TM] Enable learning */
};
typedef struct ctc_scl_vpls_s ctc_scl_vpls_t;

struct ctc_scl_logic_port_s
{
    uint16 logic_port;                      /**< [GB.GG.D2.TM] Logic port */
    uint8  logic_port_type;                 /**< [GB.GG.D2.TM] If set, indicates logic port horizon split. Used for VPLS. */
};
typedef struct ctc_scl_logic_port_s ctc_scl_logic_port_t;

struct ctc_scl_igs_action_s
{
    uint32                      flag;          /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_IGS_ACTION_FLAG_XXX */
    uint32                      sub_flag;      /**< [GB.GG.D2.TM] Bitmap of CTC_SCL_IGS_ACTION_SUB_FLAG_XXX */

    uint16                      fid;           /**< [GB.GG.D2.TM] Can be used as VPLS vsiid.*/
    uint16                      service_id;    /**< [GB.D2.TM] Service ID */
    uint32                      nh_id;         /**< [GB.GG.D2.TM] Forward nexthop ID, shared by scl, qos and pbr*/
    uint8                       color;         /**< [GB.GG.D2.TM] Color: green, yellow, or red :CTC_QOS_COLOR_XXX*/
    uint8                       priority;      /**< [GB.GG.D2.TM] Priority: 0 - 63, for D2, priority: 0 - 15 */
    uint16                      user_vlanptr;  /**< [GB.GG.D2.TM] User vlan ptr */
    uint32                      stats_id;      /**< [GB.GG.D2.TM] Stats id */
    uint8                       stp_id;        /**< [GG.D2.TM] STP id: 1 - 127 */
    uint8                       dscp;          /**< [GG.D2.TM] DSCP */
    uint16                      l2vpn_oam_id;  /**< [GG.D2.TM] vpws oam id or vpls fid */
    uint16                      policer_id;    /**< [GB.GG.D2.TM] Policer ID */
    uint16                      vrf_id;        /**< [GB.GG.D2.TM] Vrf ID of the route */
    ctc_scl_bind_t              bind;          /**< [GB.GG.D2.TM] refer ctc_scl_bind_t */
    ctc_scl_aps_t               aps;           /**< [GB.GG.D2.TM] refer ctc_scl_aps_t */
    ctc_scl_vpls_t              vpls;          /**< [GB.GG.D2.TM] refer ctc_scl_vpls_t */
    ctc_scl_logic_port_t        logic_port;    /**< [GB.GG.D2.TM] refer ctc_scl_logic_port_t */
    ctc_scl_vlan_edit_t         vlan_edit;     /**< [GB.GG.D2.TM] refer ctc_scl_vlan_edit_t */
};
typedef struct ctc_scl_igs_action_s ctc_scl_igs_action_t;

/*
 @brief scl blocking packet blocked type
*/
enum ctc_scl_blocking_pkt_type_e
{
    CTC_SCL_BLOCKING_UNKNOW_UCAST        = 0x0001,        /**< [GB.D2.TM] Unknown ucast type packet will be blocked*/
    CTC_SCL_BLOCKING_UNKNOW_MCAST        = 0x0002,        /**< [GB.D2.TM] Unknown mcast type packet will be blocked*/
    CTC_SCL_BLOCKING_KNOW_UCAST          = 0x0004,        /**< [GB.D2.TM] Known ucast type packet will be blocked*/
    CTC_SCL_BLOCKING_KNOW_MCAST          = 0x0008,        /**< [GB.D2.TM] Known mcast type packet will be blocked*/
    CTC_SCL_BLOCKING_BCAST               = 0x0010         /**< [GB.D2.TM] Bcast type packet will be blocked*/
};
typedef enum ctc_scl_blocking_pkt_type_e ctc_scl_blocking_pkt_type_t;


enum ctc_scl_egs_action_flag_e
{
    CTC_SCL_EGS_ACTION_FLAG_DISCARD    = 1U << 0, /**< [GB.GG.D2.TM] Discard the packet */
    CTC_SCL_EGS_ACTION_FLAG_STATS      = 1U << 1, /**< [GB.GG.D2.TM] Statistic */
    CTC_SCL_EGS_ACTION_FLAG_AGING      = 1U << 2, /**< [GB] Aging */
    CTC_SCL_EGS_ACTION_FLAG_VLAN_EDIT  = 1U << 3,  /**< [GB.GG.D2.TM] Vlan edit */
    CTC_SCL_EGS_ACTION_FLAG_PRIORITY_AND_COLOR   = 1U << 4   /**< [GB] Priority color */
};
typedef enum ctc_scl_egs_action_flag_e ctc_scl_egs_action_flag_t;

struct ctc_scl_egs_action_s
{
    uint32                          flag;           /**< [GB.GG.D2.TM] refer CTC_SCl_EGS_ACTION_XXX*/
    uint32                          stats_id;       /**< [GB.GG.D2.TM] Stats id */
    ctc_scl_vlan_edit_t             vlan_edit;      /**< [GB.GG.D2.TM] Refer ctc_scl_vlan_edit_t */
    uint8                           color;          /**< [GB] Color: green, yellow, or red :CTC_QOS_COLOR_XXX*/
    uint8                           priority;       /**< [GB] Priority: 0 - 63 */
    uint8                           block_pkt_type; /**< [GB.D2.TM] Block packet type*/
};
typedef struct ctc_scl_egs_action_s ctc_scl_egs_action_t;

enum ctc_scl_offset_base_type_e
{
    CTC_SCL_OFFSET_BASE_TYPE_L2,           /**< [GG.D2.TM] Start offset type is layer2 */
    CTC_SCL_OFFSET_BASE_TYPE_L3,           /**< [GG.D2.TM] Start offset type is layer3 */
    CTC_SCL_OFFSET_BASE_TYPE_L4,           /**< [GG.D2.TM] Start offset type is layer4 */
    CTC_SCL_OFFSET_BASE_TYPE_MAX
};
typedef enum ctc_scl_offset_base_type_e ctc_scl_offset_base_type_t;

struct ctc_scl_force_decap_s
{
    uint8  force_decap_en;          /**< [GG.D2.TM] Enable force decap */
    uint8  offset_base_type;        /**< [GG.D2.TM] Start offset type; refer to ctc_scl_offset_base_type_t */
    uint8  ext_offset;              /**< [GG.D2.TM] Extend offset */
    uint8  payload_type;            /**< [GG.D2.TM] Packet payload type; refer to packet_type_t*/
    uint8  use_outer_ttl;           /**< [TM] Update outer ttl */
};
typedef struct ctc_scl_force_decap_s ctc_scl_force_decap_t;

enum ctc_scl_flow_action_flag_e
{
    CTC_SCL_FLOW_ACTION_FLAG_DISCARD                            = 1U << 0,  /**< [GG.D2.TM] Discard the packet */
    CTC_SCL_FLOW_ACTION_FLAG_STATS                                = 1U << 1,  /**< [GG.D2.TM] Statistic */
    CTC_SCL_FLOW_ACTION_FLAG_TRUST                                = 1U << 2,  /**< [GG.D2.TM] QoS trust state */
    CTC_SCL_FLOW_ACTION_FLAG_COPY_TO_CPU                    = 1U << 3,  /**< [GG.D2.TM] Copy to cpu */
    CTC_SCL_FLOW_ACTION_FLAG_REDIRECT                           = 1U << 4,  /**< [GG.D2.TM] Redirect */
    CTC_SCL_FLOW_ACTION_FLAG_MICRO_FLOW_POLICER      = 1U << 5, /**< [GG.D2.TM] Micro Flow policer */
    CTC_SCL_FLOW_ACTION_FLAG_MACRO_FLOW_POLICER     = 1U << 6, /**< [GG.D2.TM] Macro Flow Policer */
    CTC_SCL_FLOW_ACTION_FLAG_METADATA                         = 1U << 7, /**< [GG.D2.TM] Metadata */
    CTC_SCL_FLOW_ACTION_FLAG_POSTCARD_EN                    = 1U << 8,  /**< [GG.D2.TM] Enable postcard*/
    CTC_SCL_FLOW_ACTION_FLAG_PRIORITY                            = 1U << 9, /**< [GG.D2.TM] Priority */
    CTC_SCL_FLOW_ACTION_FLAG_COLOR                                 = 1U << 10,  /**< [GG.D2.TM] Color */
    CTC_SCL_FLOW_ACTION_FLAG_DENY_BRIDGE                     = 1U << 11,  /**< [GG.D2.TM] Deny bridging process */
    CTC_SCL_FLOW_ACTION_FLAG_DENY_LEARNING                 = 1U << 12,  /**< [GG.D2.TM] Deny learning process */
    CTC_SCL_FLOW_ACTION_FLAG_DENY_ROUTE                       = 1U << 13,  /**< [GG.D2.TM] Deny routing process */
    CTC_SCL_FLOW_ACTION_FLAG_FORCE_BRIDGE                    = 1U << 14,  /**< [GG.D2.TM] Force bridging process */
    CTC_SCL_FLOW_ACTION_FLAG_FORCE_LEARNING                = 1U << 15,  /**< [GG.D2.TM] Force learning process */
    CTC_SCL_FLOW_ACTION_FLAG_FORCE_ROUTE                      = 1U << 16,  /**< [GG.D2.TM] Force routing process */
    CTC_SCL_FLOW_ACTION_FLAG_ACL_FLOW_TCAM_EN           = 1U << 17,  /**< [GG.D2.TM] Enable acl flow tcam lookup */
    CTC_SCL_FLOW_ACTION_FLAG_ACL_FLOW_HASH_EN           = 1U << 18,  /**< [GG.D2.TM] Enable acl flow hash lookup */
    CTC_SCL_FLOW_ACTION_FLAG_OVERWRITE_ACL_FLOW_TCAM   = 1U << 19,  /**< [GG.D2.TM] Overwrite acl flow tcam lookup */
    CTC_SCL_FLOW_ACTION_FLAG_OVERWRITE_ACL_FLOW_HASH   = 1U << 20,  /**< [GG.D2.TM] Overwrite acl flow hash lookup */
    CTC_SCL_FLOW_ACTION_FLAG_OVERWRITE_PORT_IPFIX           = 1U << 21,  /**< [GG.D2.TM] Overwrite port ipfix hash lookup */
    CTC_SCL_FLOW_ACTION_FLAG_FID                                             = 1U << 22, /**< [GG.D2.TM] fid */
    CTC_SCL_FLOW_ACTION_FLAG_VRFID                                         = 1U << 23,  /**< [GG.D2.TM] Vrf ID of the route */
    CTC_SCL_FLOW_ACTION_FLAG_FORCE_DECAP                            = 1U << 24,  /**< [GG.D2.TM] Enable force decap for tunnel */
    /* SYSTEM MODIFIED xuwj support tap feature */
    CTC_SCL_FLOW_ACTION_FLAG_INNERLOOKUP                    = 1U << 25,
    CTC_SCL_FLOW_ACTION_FLAG_STRIP_PACKET                   = 1U << 26,
    CTC_SCL_FLOW_ACTION_FLAG_ACLQOS_USE_OUTER_LOOKUP        = 1U << 27,
    /* SYSTEM MODIFIED xuwj support tap feature end */
};
typedef enum ctc_scl_flow_action_flag_e ctc_scl_flow_action_flag_t;

struct ctc_scl_flow_action_acl_s
{
    uint8                        acl_hash_lkup_type;               /**< [GG.D2.TM] Acl hash lookup type, CTC_ACL_HASH_LKUP_TYPE_XXX */
    uint8                        acl_hash_field_sel_id;            /**< [GG.D2.TM] Acl hash lookup field select */
    uint8                        acl_tcam_lkup_type;               /**< [GG.D2.TM] Acl tcam lookup type, CTC_ACL_TCAM_LKUP_TYPE_XXX */
    uint16                       acl_tcam_lkup_class_id;           /**< [GG.D2.TM] Acl tcam lookup class id */
    uint8                        rsv[3];
};
typedef struct ctc_scl_flow_action_acl_s ctc_scl_flow_action_acl_t;

struct ctc_scl_flow_action_s
{
    uint32                      flag;          /**< [GG.D2.TM] Bitmap of CTC_SCL_FLOW_ACTION_FLAG_XXX */
    uint32                      metadata;            /**< [GG.D2.TM] Metadata*/
    uint32                      nh_id;         /**< [GG.D2.TM] Forward nexthop ID, shared by scl, qos and pbr*/
    uint8                        color;         /**< [GG.D2.TM] Color: green, yellow, or red :CTC_QOS_COLOR_XXX*/
    uint8                        priority;      /**< [GG.D2.TM] Priority: 0 - 15 */
    uint8                        trust;               /**< [GG.D2.TM] QoS trust state, CTC_QOS_TRUST_XXX */
    uint8                        rsv;
    uint16                     micro_policer_id;    /**< [GG.D2.TM] Micro flow policer ID */
    uint16                     macro_policer_id;    /**< [GG.D2.TM] Macro flow policer ID */
    uint32                     stats_id;      /**< [GG.D2.TM] Stats id */
    uint8                       ipfix_lkup_type;     /**< [GG.D2.TM] Refer to ctc_ipfix_lkup_type_t */
    uint8                       ipfix_field_sel_id;   /**< [GG.D2.TM] Field select id , used associate with ctc_ipfix_key_type_t */
    uint16                     fid;                       /**<[GG.D2.TM] fid */
    uint16                     vrf_id;                  /**< [GG.D2.TM] Vrf ID of the route */
    uint8                        rsv0[2];

    ctc_scl_flow_action_acl_t     acl;      /**< [GG.D2.TM] Action to acl */
    ctc_scl_force_decap_t force_decap;      /**< [GG.D2.TM] Force decap tunnel packet */
};
typedef struct ctc_scl_flow_action_s ctc_scl_flow_action_t;


struct ctc_scl_action_s
{
    ctc_scl_action_type_t           type;           /**< [GB.GG.D2.TM] Refer ctc_scl_action_type_t */
    union
    {
        ctc_scl_igs_action_t        igs_action;     /**< [GB.GG.D2.TM] Refer ctc_scl_igs_action_t */
        ctc_scl_egs_action_t        egs_action;     /**< [GB.GG.D2.TM] Refer ctc_scl_egs_action_t */
        ctc_scl_flow_action_t       flow_action;     /**< [GG.D2.TM] Refer ctc_scl_flow_action_t */
    }u;

};
typedef struct ctc_scl_action_s ctc_scl_action_t;

/** @brief  scl entry struct */
struct ctc_scl_entry_s
{
    ctc_scl_key_t key;              /**< [GB.GG.D2.TM] scl key struct, [D2.TM]only used when mode equal 0 for compatible mode */
    ctc_scl_action_t action;        /**< [GB.GG.D2.TM] scl action struct, [D2.TM]only used when mode equal 0 for compatible mode */
    uint8 key_type;                 /**< [D2.TM] ctc_scl_key_type_t,CTC_SCL_KEY_XXX */
    uint8 action_type;              /**< [D2.TM] ctc_scl_action_type_t,CTC_SCL_KEY_XXX */
    uint8 resolve_conflict;         /**< [D2.TM] resolve conflict, install to tcam*/

    uint32 entry_id;                /**< [GB.GG.D2.TM] scl entry id. */
    uint8  priority_valid;          /**< [GB.GG.D2.TM] scl entry prioirty valid. if not valid, use default priority */
    uint32 priority;                /**< [GB.GG.D2.TM] scl entry priority. 0-0xffffffff */
    uint8  mode;                    /**< [D2.TM] 0, use sturct to install key and action; 1, use field to install key and action */
    uint8  hash_field_sel_id;       /**< [D2.TM] hash select id, only used for scl flow l2 hash key */
};
typedef struct ctc_scl_entry_s ctc_scl_entry_t;

/** @brief  scl copy entry struct */
struct ctc_scl_copy_entry_s
{
    uint32 src_entry_id;  /**< [GB.GG.D2.TM] SCL entry ID to copy from */
    uint32 dst_entry_id;  /**< [GB.GG.D2.TM] New entry copied from src_entry */
    uint32 dst_group_id;  /**< [GB.GG.D2.TM] Group ID new entry belongs */
};
typedef struct ctc_scl_copy_entry_s ctc_scl_copy_entry_t;

/** @brief  scl group type */
enum ctc_scl_group_type_e
{
    CTC_SCL_GROUP_TYPE_NONE,         /**< [GG.D2.TM]Do not care about group type. */
    CTC_SCL_GROUP_TYPE_GLOBAL,       /**< [GB.GG.D2.TM] Global scl, mask ports. */
    CTC_SCL_GROUP_TYPE_PORT,         /**< [GB.GG.D2.TM] Port scl, care gport. (GB)only for vlan key. */
    CTC_SCL_GROUP_TYPE_PORT_CLASS,   /**< [GB.GG.D2.TM] A port class scl is against a class(group) of port*/
    CTC_SCL_GROUP_TYPE_LOGIC_PORT,   /**< [GB.GG.D2.TM] Logic Port scl, care logic_port. */
    CTC_SCL_GROUP_TYPE_HASH,         /**< [GB.GG.D2.TM] Hash group. */
    CTC_SCL_GROUP_TYPE_MAX
};
typedef enum ctc_scl_group_type_e ctc_scl_group_type_t;

/** @brief  scl group info */
struct ctc_scl_group_info_s
{
    uint8 type;                      /**< [GB.GG.D2.TM] CTC_SCL_GROUP_TYPE_XXX*/
    uint8 lchip;                     /**< [GB.GG.D2.TM] Local chip id. only for ctc_scl_get_group_info.*/
    uint8 priority;                  /**< [GG.D2.TM] install_group ignore this. Group priority 0-1. */
    union
    {
        uint16 port_class_id;        /**< [GB.GG.D2.TM] port class id. multiple ports can share same un.un.port_class_id. for GB <1~61>. */
        uint32 gport;       /**< [GB.GG.D2.TM] global_port, include linkagg */
        uint16 logic_port;      /**< [GB.GG.D2.TM] Logic port */
    }un;
};
typedef struct ctc_scl_group_info_s ctc_scl_group_info_t;


enum ctc_scl_qos_map_flag_e
{
    CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID       = 1U << 0,  /**< [D2.TM] Priority valid*/
    CTC_SCL_QOS_MAP_FLAG_DSCP_VALID           = 1U << 1,  /**< [D2.TM] Dscp valid*/
    CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID      = 1U << 2,  /**< [D2.TM] Trust COS valid*/
};
typedef enum ctc_scl_qos_map_flag_e ctc_scl_qos_map_flag_t;

/** @brief  scl group info */
struct ctc_scl_qos_map_s
{
    uint32  flag;                   /**< [D2.TM] QoS map flag, refer to ctc_scl_qos_map_flag_t*/
    uint8   priority;               /**< [D2.TM] New Priority of All Color */
    uint8   color;                  /**< [D2.TM] New Color*/
    uint8   dscp;                   /**< [D2.TM] New DSCP */
    uint8   trust_dscp;             /**< [D2.TM] Trust DSCP */
    uint8   trust_cos;              /**< [D2.TM] Trust COS; 0: trust stag cos, 1: trust ctag cos */
    uint8   cos_domain;             /**< [D2.TM] QoS COS domain */
    uint8   dscp_domain;            /**< [D2.TM] QoS DSCP domain */
};
typedef struct ctc_scl_qos_map_s ctc_scl_qos_map_t;

enum ctc_scl_field_action_type_e
{

    CTC_SCL_FIELD_ACTION_TYPE_PENDING ,                 /**< [D2.TM] Action change to pending for installed entry, used for Update all action */

    /*Ingress Egress Flow share*/
    CTC_SCL_FIELD_ACTION_TYPE_DISCARD ,                 /**< [D2.TM] DisCard Packet of All Color. */
    CTC_SCL_FIELD_ACTION_TYPE_STATS ,                   /**< [D2.TM] Packet Statistics; data0: Stats Id. */
    CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT,               /**< [D2.TM] Vlan Edit;ext_data: ctc_scl_vlan_edit_t. */
    CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL,              /**< [D2.TM] Reset to default action */

    /*Ingress Flow share*/
    CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU ,             /**< [D2.TM] Copy To Cpu. */
    CTC_SCL_FIELD_ACTION_TYPE_REDIRECT ,                /**< [D2.TM] Redirect Packet with NHID; data0: Nexthop Id. */

    CTC_SCL_FIELD_ACTION_TYPE_FID ,                     /**< [D2.TM] Fid; data0: fid. */
    CTC_SCL_FIELD_ACTION_TYPE_VRFID ,                   /**< [D2.TM] Vrf id; data0: Vrfid. */
    CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID ,              /**< [D2.TM] policer id; data0: Policer Id. data1: Service Policer En */
    CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER,         /**< [D2.TM] HBWP Policer with cos index; data0: Policer Id; data1: cos index*/
    CTC_SCL_FIELD_ACTION_TYPE_SRC_CID ,                 /**< [D2.TM] Source Category id; data0: source cid value */
    CTC_SCL_FIELD_ACTION_TYPE_DST_CID ,                 /**< [D2.TM] Destination Category id; data0: dest cid value */
    CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW,                /**< [D2.TM] The packet is span Flow,and it won't be mirrored again. */
    CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT,            /**< [D2.TM] Redirect Packet to single port*/
    CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP,                  /**< [D2.TM] Qos map, ext_data: refer to ctc_scl_qos_map_t */

    /*only valid for Ingress Action*/
    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT ,              /**< [D2.TM] Logic Port; ext_data: ctc_scl_logic_port_t. */
    CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR ,            /**< [D2.TM] User VlanPtr; data0: User VlanPtr */
    CTC_SCL_FIELD_ACTION_TYPE_APS  ,                    /**< [D2.TM] Aps Group; ext_data: ctc_scl_aps_t. */
    CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF,               /**< [D2.TM] VPLS Etree Leaf. */
    CTC_SCL_FIELD_ACTION_TYPE_STP_ID ,                  /**< [D2.TM] Stp Id; data0: Stp Id. */
    CTC_SCL_FIELD_ACTION_TYPE_PTP_CLOCK_ID,             /**< [D2.TM] Ptp Clock Id; data0: ptp clock id. */
    CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID,               /**< [D2.TM] Service Id; data0: service id. */
    CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN,           /**< [D2.TM] Service acl enable. */
    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN,   /**< [D2.TM] Logic port security enable. */
    CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN,               /**< [D2.TM] Enable binding; ext_data: ctc_scl_bind_t */
    CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT_DISCARD_EN,     /**< [D2.TM] Enable mac limit discard */
    CTC_SCL_FIELD_ACTION_TYPE_OAM,                      /**< [D2.TM] Enable OAM, data0: vpls oam using 0 and vpws oam using 1, data1: l2vpn_oam_id (only effective for vpws)*/
    CTC_SCL_FIELD_ACTION_TYPE_ROUTER_MAC_MATCH,         /**< [TM] Set Router MAC Match, data0: enable use1 and disable use 0 */
    CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT,                        /**< [TM] Logic port mac limit discard or to cpu, ext_data: ctc_maclimit_action_t */
    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_TYPE,          /**< [TM] Set logic port type */

    /*only valid for SclFlow Action*/
    CTC_SCL_FIELD_ACTION_TYPE_METADATA ,                /**< [D2.TM] Metadata; data0: meta data. */
    CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE ,             /**< [D2.TM] Deny Bridge. */
    CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING ,           /**< [D2.TM] Deny Learning. */
    CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE ,              /**< [D2.TM] Deny Route. */
    CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE ,            /**< [D2.TM] Force Bridge. */
    CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING ,          /**< [D2.TM] Force Learning. */
    CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE ,             /**< [D2.TM] Forece Route. */
    CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN ,             /**< [D2.TM] Enable acl flow tcam lookup; ext_data: ctc_acl_property_t. */
    CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN ,             /**< [D2.TM] Enable acl flow hash lookup; ext_data: ctc_acl_property_t. */
    CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD , /**< [D2.TM] Acl user outer info. data0: outer info */
    CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP,              /**< [D2.TM] Force decap */
    CTC_SCL_FIELD_ACTION_TYPE_SNOOPING_PARSER,          /**< [TM] Snooping parser without decap, ext_data: ctc_scl_snooping_parser_t */
    CTC_SCL_FIELD_ACTION_TYPE_LB_HASH_SELECT_ID,        /**< [TM] Load balance hash select id, refer to ctc_lb_hash_config_t, data0: sel_id<0-7>, 0 means disable */

    /*only valid for egress action */
    CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_UCAST ,      /**< [D2.TM] Drop unknown ucast packet */
    CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_MCAST ,      /**< [D2.TM] Drop unknown mcast packet */
    CTC_SCL_FIELD_ACTION_TYPE_DROP_BCAST ,              /**< [D2.TM] Drop bcast packet */
    CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_UCAST ,        /**< [D2.TM] Drop known ucast packet */
    CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_MCAST ,        /**< [D2.TM] Drop known mcast packet */
    CTC_SCL_FIELD_ACTION_TYPE_NUM
};
typedef enum ctc_scl_field_action_type_e ctc_scl_field_action_type_t;



struct ctc_scl_field_action_s
{
    uint32 type;                            /**< [D2.TM] Action Field type, CTC_SCL_ACTION_FIELD_TYPE_XXX. */

    uint32 data0;                           /**< [D2.TM] Action Field data0 (uint32). */
    uint32 data1;                           /**< [D2.TM] Action Field data1 (uint32). */
    void*  ext_data;                        /**< [D2.TM] Action Field data (void*). */

};
typedef struct ctc_scl_field_action_s ctc_scl_field_action_t;


struct ctc_scl_default_action_s
{
    uint8  mode;                            /**< [D2.TM] scl default ad mode, mode 0 use struct to set default ad and mode 1 use field to set default ad */
    uint8  dir;                             /**< [D2.TM] default action type */
    uint8  scl_id;                         /**< [TM] scl id, use 1 to set hash 1 default action */
    uint32 gport;                           /**< [GB.GG.D2.TM] Global port */
    ctc_scl_action_t action;                /**< [GB.GG.D2.TM] scl action struct */
    ctc_scl_field_action_t* field_action;   /**< [D2.TM] scl field action */
};
typedef struct ctc_scl_default_action_s ctc_scl_default_action_t;

struct ctc_scl_snooping_parser_s
{
    uint8  enable;                  /**< [TM] Enable snooping parser */
    uint8  start_offset_type;       /**< [TM] Start offset type; refer to ctc_scl_offset_base_type_t */
    uint8  ext_offset;              /**< [TM] Extend offset */
    uint8  payload_type;            /**< [TM] Packet payload type; refer to packet_type_t*/
    uint8  use_inner_hash_en;       /**< [TM] If set, do inner hash for unknown packet */
};
typedef struct ctc_scl_snooping_parser_s ctc_scl_snooping_parser_t;

#ifdef __cplusplus
}
#endif

#endif

