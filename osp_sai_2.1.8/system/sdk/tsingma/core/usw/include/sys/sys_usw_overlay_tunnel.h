#if (FEATURE_MODE == 0)
/**
 @file sys_usw_overlay_tunnel.h

 @date 2013-10-25

 @version v2.0

*/

#ifndef _SYS_USW_OVERLAY_TUNNEL_H
#define _SYS_USW_OVERLAY_TUNNEL_H
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
#include "ctc_overlay_tunnel.h"
#include "ctc_debug.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define SYS_OVERLAT_TUNNEL_MAX_IP_INDEX       4

enum sys_overlay_tunnel_packet_type_e
{
    SYS_OVERLAY_TUNNEL_PACKET_TYPE_UC,
    SYS_OVERLAY_TUNNEL_PACKET_TYPE_MC,
    SYS_OVERLAY_TUNNEL_PACKET_TYPE_MAX
};
typedef enum sys_overlay_tunnel_packet_type_e sys_overlay_tunnel_packet_type_t;

struct sys_overlay_tunnel_master_s
{
    uint32 ipda_index_cnt[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    sal_mutex_t* mutex;
    uint8 could_sec_en;
};
typedef struct sys_overlay_tunnel_master_s sys_overlay_tunnel_master_t;

#define SYS_OVERLAY_TUNNEL_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(overlay_tunnel, overlay_tunnel, OVERLAY_TUNNEL_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)

/****************************************************************************
*
* Function
*
*****************************************************************************/
extern int32
sys_usw_overlay_tunnel_init(uint8 lchip, void* o_param);

extern int32
sys_usw_overlay_tunnel_set_fid (uint8 lchip,  uint32 vn_id, uint16 fid );

extern int32
sys_usw_overlay_tunnel_get_fid (uint8 lchip,  uint32 vn_id, uint16* fid );

extern int32
sys_usw_overlay_tunnel_get_vn_id(uint8 lchip, uint16 fid, uint32* p_vn_id);

extern int32
sys_usw_overlay_tunnel_add_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param);

extern int32
sys_usw_overlay_tunnel_remove_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param);

extern int32
sys_usw_overlay_tunnel_update_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param);

extern int32
sys_usw_overlay_tunnel_deinit(uint8 lchip);

extern int32
sys_usw_overlay_tunnel_show_status(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif
