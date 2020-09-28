#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_captured_info.h"
#include "ctc_usw_dkit_path.h"

#if 0
#include "ctc_usw_dkit_bus_info.h"
#endif


extern ctc_dkit_master_t* g_usw_dkit_master[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

 int32
ctc_usw_dkit_path_process(void* p_para)
{
    ctc_dkit_path_para_t* p_path_para = (ctc_dkit_path_para_t*)p_para;
    sal_time_t tv;
    uint8 lchip = 0;

    DKITS_PTR_VALID_CHECK(p_para);
    lchip = p_path_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);
    DKITS_PTR_VALID_CHECK(g_usw_dkit_master[lchip]);

    /*get systime*/
    sal_time(&tv);
    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT("TIME: %s", sal_ctime(&tv));
    CTC_DKIT_PRINT("============================================================\n");
    if ((p_path_para->captured) && (p_path_para->on_line))
    {
        ctc_usw_dkit_captured_path_process((void*)p_path_para);
    }
    else
    {
#if 0
        ctc_usw_dkit_bus_path_process(lchip, &(g_usw_dkit_master[lchip]->path_stats), p_path_para->detail);
#endif
    }
    CTC_DKIT_PRINT("============================================================\n");

    return CLI_SUCCESS;
}


