#ifndef __HSRV_STATS_H__
#define __HSRV_STATS_H__
#include "glb_stats_define.h"
#include "ctclib_opf.h"

struct hsrv_stats_info_s
{
    glb_stats_type_t type;
    bool   is_time_out;
    uint32 agent_id_requested;
    
    uint32 agent_id_received;

    glb_stats_result_t stats_rslt;
};
typedef struct hsrv_stats_info_s hsrv_stats_info_t;

struct hsrv_stats_master_s
{
    hsrv_stats_info_t stats_info[GLB_STATS_TYPE_MAX];
    ctclib_opf_t* pst_stats_id;
};
typedef struct hsrv_stats_master_s hsrv_stats_master_t;

extern int32
hsrv_stats_get_stats(glb_stats_type_t type, uint32 agent_id, HalMsgStatsKey_t* key);
extern int32
hsrv_stats_clear_stats(glb_stats_type_t type, uint32 agent_id, HalMsgStatsKey_t* key, int32 key_num);
extern int32
hsrv_stats_module_start();

#endif /*__HSRV_STATS_H__*/
