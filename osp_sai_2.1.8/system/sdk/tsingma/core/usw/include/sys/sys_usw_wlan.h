#if (FEATURE_MODE == 0)
/**
 @file sys_usw_overlay_tunnel.h

 @date 2013-10-25

 @version v2.0

*/

#ifndef _SYS_USW_WLAN_H
#define _SYS_USW_WLAN_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"
#include "ctc_scl.h"
#include "ctc_wlan.h"
#include "ctc_debug.h"
#include "ctc_hash.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define SYS_WLAN_TUNNEL_ENTRY_BLOCK_SIZE    16

#define SYS_WLAN_DBG_OUT(level, FMT, ...)\
    do\
    {\
        CTC_DEBUG_OUT(wlan, wlan, WLAN_SYS, level, FMT, ##__VA_ARGS__);\
    } while (0)

#define SYS_WLAN_TUNNEL_SW_ENTRY_INFO_LEN      36

struct sys_wlan_tunnel_sw_entry_s
{
    ctc_slistnode_t    head;                /* keep head top!! */
    union
    {
        ip_addr_t ipv4;         /**< [D2] IPv4 source address */
        ipv6_addr_t ipv6;       /**< [D2] IPv6 source address */
    } ipsa;
    union
    {
        ip_addr_t ipv4;         /**< [D2] IPv4 dest address */
        ipv6_addr_t ipv6;       /**< [D2] IPv6 dest address */
    } ipda;
    uint16 l4_port;             /**< [D2] udp dest port */
    uint8 is_ipv6;              /**< [D2] ipv4 or ipv6 */
    uint8 rsv;
    uint16 tunnel_id;
    uint16 network_count;       /**< [D2] network tunnel count */
    uint16 bssid_count;         /**< [D2] bssid tunnel count */
    uint16 bssid_rid_count;     /**< [D2] bssid rid tunnel count */
};
typedef struct sys_wlan_tunnel_sw_entry_s sys_wlan_tunnel_sw_entry_t;

enum sys_wlan_group_sub_type_e
{
    SYS_WLAN_GROUP_SUB_TYPE_HASH0_TUNNEL,
    SYS_WLAN_GROUP_SUB_TYPE_HASH1_TUNNEL,
    SYS_WLAN_GROUP_SUB_TYPE_TCAM0_TUNNEL,
    SYS_WLAN_GROUP_SUB_TYPE_TCAM1_TUNNEL,

    SYS_WLAN_GROUP_SUB_TYPE_HASH0_CLIENT_LOCAL,
    SYS_WLAN_GROUP_SUB_TYPE_HASH0_CLIENT_REMOTE,
    SYS_WLAN_GROUP_SUB_TYPE_HASH1_CLIENT_LOCAL,
    SYS_WLAN_GROUP_SUB_TYPE_HASH1_CLIENT_REMOTE,
    SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_LOCAL,
    SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_REMOTE,
    SYS_WLAN_GROUP_SUB_TYPE_TCAM1_CLIENT_LOCAL,
    SYS_WLAN_GROUP_SUB_TYPE_TCAM1_CLIENT_REMOTE,
    SYS_WLAN_GROUP_SUB_TYPE_NUM
};

struct sys_wlan_master_s
{
    sal_mutex_t* mutex;
    uint8 roam_type;
    uint8 opf_type_wlan_tunnel_id;
    ctc_hash_t* tunnel_entry;
    uint32 decrypt_fwd_offset;
    uint32 decap_nh_offset;
    uint32 l2_trans_edit_offset;
    uint32 default_clinet_entry_id;
    uint8  default_client_action;
    uint16 default_client_vlan_id;
    uint32 scl_hash0_tunnel_gid;
    uint32 scl_hash1_tunnel_gid;
    uint32 scl_tcam0_tunnel_gid;
    uint32 scl_tcam1_tunnel_gid;
    uint32 scl_hash0_client_local_gid;
    uint32 scl_hash0_client_remote_gid;
    uint32 scl_hash1_client_local_gid;
    uint32 scl_hash1_client_remote_gid;
    uint32 scl_tcam0_client_local_gid;
    uint32 scl_tcam0_client_remote_gid;
    uint32 scl_tcam1_client_local_gid;
    uint32 scl_tcam1_client_remote_gid;
};
typedef struct sys_wlan_master_s sys_wlan_master_t;

/****************************************************************************
*
* Function
*
*****************************************************************************/
extern int32
sys_usw_wlan_init(uint8 lchip, ctc_wlan_global_cfg_t* p_global_t);

extern int32
sys_usw_wlan_deinit(uint8 lchip);

extern int32
sys_usw_wlan_add_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param);

extern int32
sys_usw_wlan_remove_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param);

extern int32
sys_usw_wlan_update_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param);

extern int32
sys_usw_wlan_add_client(uint8 lchip, ctc_wlan_client_t* p_client_param);

extern int32
sys_usw_wlan_remove_client(uint8 lchip, ctc_wlan_client_t* p_client_param);

extern int32
sys_usw_wlan_update_client(uint8 lchip, ctc_wlan_client_t* p_client_param);

extern int32
sys_usw_wlan_set_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param);

extern int32
sys_usw_wlan_get_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param);

extern int32
sys_usw_wlan_set_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param);

extern int32
sys_usw_wlan_get_global_cfg(uint8 lchip,  ctc_wlan_global_cfg_t* p_glb_param);

int32
sys_usw_wlan_show_status(uint8 lchip, uint8 type, uint8 param);

#ifdef __cplusplus
}
#endif

#endif

#endif

