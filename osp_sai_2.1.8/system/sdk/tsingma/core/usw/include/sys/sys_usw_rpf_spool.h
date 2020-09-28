#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_usw_rpf_spool.h

 @date 2012-09-18

 @version v2.0

IPUC and IPMC share rpf profile,but ipuc or ipmc has at least
256 (ipmc is 255) profile, total up to 511.

*/
#ifndef _SYS_USW_RPF_SPOOL_H
#define _SYS_USW_RPF_SPOOL_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "drv_api.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "ctc_hash.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
enum sys_rpf_usage_type_e
{
    SYS_RPF_USAGE_TYPE_IPUC,
    SYS_RPF_USAGE_TYPE_IPMC,
    SYS_RPF_USAGE_TYPE_DEFAULT,
    SYS_RPF_USAGE_TYPE_NUM
};
typedef enum sys_rpf_usage_type_e sys_rpf_usage_type_t;

#define SYS_RPF_DBG_OUT(level, FMT, ...)                              \
    {                                                                       \
        CTC_DEBUG_OUT(ip, rpf, RPF_SYS, level, FMT, ##__VA_ARGS__);  \
    }

#define SYS_RPF_PROFILE_START_INDEX       1
#define SYS_USW_MAX_RPF_IF_NUM           16

enum sys_rpf_chk_mode_e
{
    SYS_RPF_CHK_MODE_PROFILE = 0,  /*must be 0*/
    SYS_RPF_CHK_MODE_IFID,
    SYS_RPF_CHK_MODE_NUM
};
typedef enum sys_rpf_chk_mode_e sys_rpf_chk_mode_t;

enum sys_rpf_strick_type_e
{
    SYS_RPF_STRICK_TYPE_RPFID_MATCH,
    SYS_RPF_STRICK_TYPE_PORT_MATCH,
    SYS_RPF_STRICK_TYPE_RPFID_AND_PORT_MATCH,
    SYS_RPF_STRICK_TYPE_NUM
};
typedef enum sys_rpf_strick_type_e sys_rpf_strick_type_t;

struct sys_rpf_profile_s
{
    DsRpf_m  rpf_intf;
    uint8    is_ipmc;
    uint8    rsv;
    uint16   profile_id;
};
typedef struct sys_rpf_profile_s sys_rpf_profile_t;

struct sys_rpf_info_s
{
    uint8   is_ipmc;
    uint8   mode;
    uint8   rpf_intf_cnt;
    uint16  rpf_intf[SYS_USW_MAX_RPF_IF_NUM];
    uint16  rpf_id;      /*[out param]*/
};
typedef struct sys_rpf_info_s sys_rpf_info_t;


/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

extern int32
sys_usw_rpf_init(uint8 lchip);

extern int32
sys_usw_rpf_deinit(uint8 lchip);

extern int32
sys_usw_rpf_remove_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info);
extern int32
sys_usw_rpf_update_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info);
extern int32
sys_usw_rpf_add_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info);

extern int32
sys_usw_rpf_restore_profile(uint8 lchip, uint32 profile_id);

#ifdef __cplusplus
}
#endif

#endif

#endif

