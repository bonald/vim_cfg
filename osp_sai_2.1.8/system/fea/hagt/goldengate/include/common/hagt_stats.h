#ifndef __HAGT_STATS_H__
#define __HAGT_STATS_H__
#include "glb_stats_define.h"

typedef int32 (*HAGT_STATS_GET_STATS_CB)(HalMsgStatsKey_t*, glb_stats_result_t*);
typedef int32 (*HAGT_STATS_CLEAR_STATS_CB)(HalMsgStatsKey_t*);
struct hagt_stast_pm_data_s
{
    HAGT_STATS_GET_STATS_CB get_stats_cb;
    HAGT_STATS_CLEAR_STATS_CB clear_stats_cb;
};
typedef struct hagt_stast_pm_data_s hagt_stast_pm_data_t;

extern int32
hagt_stats_regrister_pm_data(glb_stats_type_t type, hagt_stast_pm_data_t* p_pm_data);

extern int32
hagt_stats_init(hagt_master_t* pst_hagt_master);

#endif /*__HAGT_STATS_H__*/
