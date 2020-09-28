#if (FEATURE_MODE == 0)
/**
 @file ctc_wlan.h

 @author  Copyright(C) 2005-2016 Centec Networks Inc.  All rights reserved.

 @date 2016-01-23

 @version v2.0

   This file contains wlan tunnel related data structure, enum, macro and proto.
*/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup wlan WLAN
 @{
*/

#ifndef _CTC_WLAN_H
#define _CTC_WLAN_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common/include/ctc_const.h"
#include "common/include/ctc_common.h"

#define WLAN_WLAN_CONTROL_L4_PORT            5246
#define WLAN_WLAN_DATA_L4_PORT               5247

/**
 @brief  Define wlan default client action
*/

enum ctc_wlan_default_client_action_type_e
{
    CTC_WLAN_DEFAULT_CLIENT_ACTION_DISCARD,               /**< [D2.TM] packet will be dropped when match default client. (init config)*/
    CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN,     /**< [D2.TM] packet will be mapped to a pre-configed vlan. (init config)*/
    CTC_WLAN_DEFAULT_CLIENT_ACTION_MAX
};
typedef enum ctc_wlan_default_client_action_type_e ctc_wlan_default_client_action_type_t;

/**
 @brief  Define wlan network topo configure. Only POP_POA mode is support now. User doesn't need to care.
*/
enum ctc_wlan_topo_type_e
{
    CTC_WLAN_TOPO_POP_POA,      /**< [D2.TM] POP_POA*/
    CTC_WLAN_TOPO_HUB_SPOK,     /**< [D2.TM] HUB_SPOK*/
    CTC_WLAN_TOPO_FULL_MASH,    /**< [D2.TM] FULL_MASH*/
    CTC_WLAN_TOPO_MAX
};
typedef enum ctc_wlan_topo_type_e ctc_wlan_topo_type_t;
/**
 @brief  Define wlan global config
*/
struct ctc_wlan_global_cfg_s
{
    uint8 control_pkt_decrypt_en;            /**< [D2.TM] control encrypt packet will decypt by chip or cpu. default by chip. */
    uint16 dtls_version;                     /**< [D2.TM] dtls version. 0xFEFF:DTLS1.0;  0xFEFD:DTLS1.2*/
    uint8 fc_swap_enable;                    /**< [D2.TM] 802.11 frame control,default swap is enable */
    uint16 udp_dest_port0;                   /**< [D2.TM] wlan control dest port0, default is 5246 */
    uint16 udp_dest_port1;                   /**< [D2.TM] wlan control dest port1, default is 5246 */
    uint16 default_client_vlan_id;           /**< [D2.TM] default local client mapping vlan id */
    uint8  default_client_action;            /**< [D2.TM] default local client action, default is discard.refer to ctc_wlan_default_client_action_type_t ,remote default client action is always discard*/
    uint8 rsv[3];
};
typedef struct ctc_wlan_global_cfg_s ctc_wlan_global_cfg_t;

/**
 @brief  Define wlan tunnel mode
*/
enum ctc_wlan_tunnel_mode_e
{
    CTC_WLAN_TUNNEL_MODE_NETWORK,      /**< [D2.TM] The Tunnel mode of this key is ipsa+ipda+l4_port */
    CTC_WLAN_TUNNEL_MODE_BSSID,        /**< [D2.TM] The Tunnel mode of this key is ipsa+ipda+l4_port+bssid */
    CTC_WLAN_TUNNEL_MODE_BSSID_RADIO, /**< [D2.TM] The Tunnel mode of this key is ipsa+ipda+l4_port+bssid+radio_id */
    CTC_WLAN_MAX_TUNNEL_MODE
};
typedef enum ctc_wlan_tunnel_mode_e ctc_wlan_tunnel_mode_t;

/**
 @brief  Define wlan tunnel type
*/
enum ctc_wlan_tunnel_type_e
{
    CTC_WLAN_TUNNEL_TYPE_WTP_2_AC,        /**< [D2.TM] Wtp to ac tunnel type */
    CTC_WLAN_TUNNEL_TYPE_AC_2_AC,         /**< [D2.TM] Ac to ac tunnel type */
    CTC_WLAN_MAX_TUNNEL_TYPE
};
typedef enum ctc_wlan_tunnel_type_e ctc_wlan_tunnel_type_t;

/**
 @brief  Define wlan tunnel flag
*/
enum ctc_wlan_tunnel_flag_e
{
    CTC_WLAN_TUNNEL_FLAG_IPV6                           = 0x0001, /**< [D2.TM] ipv6 wlan tunnel*/
    CTC_WLAN_TUNNEL_FLAG_DECRYPT_ENABLE                 = 0x0002, /**< [D2.TM] enable wlan decrypt */
    CTC_WLAN_TUNNEL_FLAG_REASSEMBLE_ENABLE              = 0x0004, /**< [D2.TM] enable wlan reassemble */
    CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT          = 0x0008, /**< [D2.TM] install to tcam, for resolve hash conflict */
    CTC_WLAN_TUNNEL_FLAG_TTL_CHECK                      = 0x0010, /**< [D2.TM] wlan tunnel do TTL check for out header */
    CTC_WLAN_TUNNEL_FLAG_QOS_MAP                        = 0x0020, /**< [D2.TM] Qos map */
    CTC_WLAN_TUNNEL_FLAG_SPLIT_MAC                      = 0x0040, /**< [D2.TM] wlan tunnel split mac mode: inner packet is 802.11; default: not set, local mac mode, inner packet is 802.3 */
    CTC_WLAN_TUNNEL_FLAG_SERVICE_ACL_EN                 = 0x0080,  /**< [D2.TM] wlan tunnel enable service acl qos*/
    CTC_WLAN_TUNNEL_FLAG_ACL_EN                         = 0x0100,  /**< [D2.TM] tunnel acl enable */
    CTC_WLAN_TUNNEL_FLAG_STATS_EN                       = 0x0200  /**< [D2.TM] wlan tunnel enable stats qos*/

};
typedef enum ctc_wlan_tunnel_flag_e ctc_wlan_tunnel_flag_t;

/**
 @brief  Define wlan tunnel info
*/
struct ctc_wlan_tunnel_s
{
    uint8 mode;                 /**< [D2.TM] wlan tunnel mode, refer to ctc_wlan_tunnel_mode_t */
	uint8 type;                 /**< [D2.TM] wlan tunnel type, refer to ctc_wlan_tunnel_type_t */
    uint16 flag;                /**< [D2.TM] refer to ctc_wlan_tunnel_flag_t */
   union
    {
        ip_addr_t ipv4;         /**< [D2.TM] IPv4 source address */
        ipv6_addr_t ipv6;       /**< [D2.TM] IPv6 source address */
    } ipsa;

    union
    {
        ip_addr_t ipv4;         /**< [D2.TM] IPv4 dest address */
        ipv6_addr_t ipv6;       /**< [D2.TM] IPv6 dest address */
    } ipda;
    uint16 l4_port;             /**< [D2.TM] udp dest port*/
    mac_addr_t bssid;           /**< [D2.TM] bssid mac */
    uint8 radio_id;             /**< [D2.TM] radio id */

    uint8  decrypt_id;           /**< [D2.TM] decrypt key index, value:0-63 */
    uint16 default_vlan_id;     /**< [D2.TM] default vlan id for the tunnel */
    uint16 logic_port;          /**< [D2.TM] logic port, 0 is invalid */
    uint16 policer_id;          /**< [D2.TM] if set 0, indicate invalid*/
	ctc_scl_qos_map_t  qos_map;  /**< [D2.TM] qos map*/
	uint8 acl_lkup_num;  /**< [D2.TM] acl look up number*/
    ctc_acl_property_t acl_prop[CTC_MAX_ACL_LKUP_NUM];/**< [D2.TM] overwrite acl */
};
typedef struct ctc_wlan_tunnel_s ctc_wlan_tunnel_t;

/**
 @brief  Define wlan client flag
*/
enum ctc_wlan_client_flag_e
{
    CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY                    = 0x0001, /**< [D2.TM] client check key type is mcast ???? */
    CTC_WLAN_CLIENT_FLAG_STATS_EN                       = 0x0002, /**< [D2.TM] stats id enable */
    CTC_WLAN_CLIENT_FLAG_MAPPING_VLAN                   = 0x0004, /**< [D2.TM] use for classify */
    CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT          = 0x0008, /**< [D2.TM] install to tcam, for resolve hash conflict */
    CTC_WLAN_CLIENT_FLAG_QOS_MAP                        = 0x0010, /**< [D2.TM] Qos map */
    CTC_WLAN_CLIENT_FLAG_ACL_EN                         = 0x0020  /**< [D2.TM] client acl enable */
};
typedef enum ctc_wlan_client_flag_e ctc_wlan_client_flag_t;



/**
 @brief  Define wlan client info
*/
struct ctc_wlan_client_s
{
    /*key*/
    mac_addr_t sta;                 /**< [D2.TM] the client mac addr, default is mac sa;when is_local is 0 and tunnel_type ac2ac, the sta must be mac da */
	uint8   is_local;                /**< [D2.TM] the sta is local or form other AC  */
    uint8   tunnel_type;             /**< [D2.TM] tunnel type, refer to ctc_wlan_tunnel_type_t only valid when is_local is 0*/
	uint16  src_vlan_id;             /**< [D2.TM] the src_vlan_id, only valid when the flag set multicast enable*/
    uint32  flag;                    /**< [D2.TM] client flag, refer to struct ctc_wlan_client_flag_t */
    /*ad*/
    uint8   roam_status;              /**<[D2.TM] 0:no roam, 1:POA, 2:POP */
    uint32  nh_id;                   /**< [D2.TM] nexthop id only valid when is_local is 0 */
    uint16  cid;                     /**< [D2.TM] category id, 0 indicate invalid*/
    uint16  policer_id;              /**< [D2.TM] if set 0,  indicate invalid*/
    uint16  stats_id;                /**< [D2.TM] Stats id for the client */
    uint16  vlan_id;                 /**< [D2.TM] if CTC_WLAN_CLIENT_FLAG_MAPPING_VLAN Set,indicate remapping STA's VLAN .*/

	uint16  vrfid;                  /**< [D2.TM] indicate routed packet's VRFID,default is 0 */
	uint16  logic_port;             /**< [D2.TM] logic port, 0 is invalid */

	uint8   acl_lkup_num;           /**< [D2.TM] acl look up number */
    ctc_acl_property_t acl[CTC_MAX_ACL_LKUP_NUM]; /**< [D2.TM] overwrite acl */
	ctc_scl_qos_map_t  qos_map;  /**< [D2.TM] qos map */
};
typedef struct ctc_wlan_client_s ctc_wlan_client_t;

#define WLAN_AES_KEY_LENGTH    16
#define WLAN_SHA_KEY_LENGTH    20

/**
 @brief  Define wlan crypt info
*/
struct ctc_wlan_crypt_info_s
{
    uint8 aes_key[WLAN_AES_KEY_LENGTH];     /**< [D2.TM] aes key */
    uint8 sha_key[WLAN_SHA_KEY_LENGTH];     /**< [D2.TM] sha key */
    uint64 seq_num;                         /**< [D2.TM] seq num */
    uint16 epoch;                           /**< [D2.TM] epoch value */
    uint8  valid;                           /**< [D2.TM] when type is encrypt, the field is invalid */
    uint8  rsv;
};
typedef struct ctc_wlan_crypt_info_s ctc_wlan_crypt_info_t;

#define CTC_WLAN_CRYPT_TYPE_ENCRYPT         0
#define CTC_WLAN_CRYPT_TYPE_DECRYPT         1

/**
 @brief  Define wlan crypt
*/
struct ctc_wlan_crypt_s
{
    uint8 crypt_id;                 /**< [D2.TM] crypt id,type is encrypt, range :0-127; decrypt range:0-63 */
    uint8 type;                     /**< [D2.TM] crypt type, 0:encrypt;1:decrypt*/
    uint8 key_id;                   /**< [D2.TM] only for decrypt*/
    uint8 rsv;

    ctc_wlan_crypt_info_t key;    /**< [D2.TM] when type equal decrypt, may be need two keys.Type equal encrypt, just need one key*/
};
typedef struct ctc_wlan_crypt_s ctc_wlan_crypt_t;

/**@} end of @defgroup  wlan WLAN */
#ifdef __cplusplus
}
#endif

#endif

#endif

