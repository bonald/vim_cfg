#include "ctc_api.h"
#include "hagt_inc.h"

#include "hagt.h"
#include "hagt_chip.h"
#include "hagt_stats.h"
#include "hagt_openflow.h"
#include "hagt_openflow_group.h"
#include "hagt_openflow_stats.h"


#define _____INTERNAL_APIS_____
/*************************************************************************************************
 * Name         : _hagt_openflow_group_get_stats
 * Purpose      : Hagt layer group get stats
 * Input        : key: HalMsgStatsKey_t*, p_rslt: glb_stats_result_t*
 * Output       : N/A
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
static int32
_hagt_openflow_group_get_stats(HalMsgStatsKey_t* key, glb_stats_result_t* p_rslt)
{
    p_rslt->basic.byte_count = 0;
    p_rslt->basic.packet_count = 0;
    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_group_clear_stats
 * Purpose      : Hagt layer group clear stats
 * Input        : key: HalMsgStatsKey_t*
 * Output       : N/A
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
static int32
_hagt_openflow_group_clear_stats(HalMsgStatsKey_t* key)
{
    return HAGT_E_NONE;
}

#define _____GROUP_INIT_____
/*************************************************************************************************
 * Name         : hagt_openflow_group_init
 * Purpose      : Hagt layer group init
 * Input        : N/A
 * Output       : N/A
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_group_init(void)
{
    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_group_stats_init
 * Purpose      : Hagt layer grouo stats init
 * Input        : N/A
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_group_stats_init(void)
{
    hagt_stast_pm_data_t pm_data;

    pm_data.get_stats_cb   = _hagt_openflow_group_get_stats;
    pm_data.clear_stats_cb = _hagt_openflow_group_clear_stats;

    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_OPENFLOW_GROUP, &pm_data);

    return HAGT_E_NONE;
}

