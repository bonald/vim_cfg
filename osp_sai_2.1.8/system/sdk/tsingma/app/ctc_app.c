#include "api/include/ctc_api.h"
#include "api/include/ctcs_api.h"
#include "ctc_app_cfg.h"
#include "app_usr.h"
#include "ctc_app_isr.h"
#include "ctc_app.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_app_warmboot.h"
#include "ctc_app_packet.h"

/*the datapath is apply to centec's demo borad*/
#ifdef SDK_IN_USERMODE
#include <sys/mman.h>
#endif

extern int32 g_error_on;
extern int32 ctc_master_cli(int32 ctc_shell_mode);
extern int32 ctc_cli_read(int32 ctc_shell_mode);
extern int32 ctc_app_cli_init(void);
ctc_app_master_t g_ctc_app_master = {0};

int32
ctc_app_get_lchip_id(uint8 gchip, uint8* lchip)
{
    uint8 lchip_id = 0;

    if (NULL == lchip)
    {
        return -1;
    }

    for (lchip_id = 0; lchip_id < g_ctc_app_master.lchip_num; lchip_id++)
    {
        if (gchip == g_ctc_app_master.gchip_id[lchip_id])
        {
            *lchip = lchip_id;
            return 0;
        }
    }

    return -1;
}

int32
ctc_app_sample_init(void)
{
    int32 ret = 0;
    uint8 lchip = 0;

    if (0 == g_ctc_app_master.ctcs_api_en)
    {
        /*call ctc api functions */
        ret = ctc_app_isr_init();
        if (ret != 0)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_isr_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }
    }
    else
    {
        /*call ctcs api functions for per chip operation*/
        for (lchip = 0; lchip < g_ctc_app_master.lchip_num; lchip++)
        {
            ret = ctcs_app_isr_init(lchip);
            if (ret != 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_app_isr_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
                return ret;
            }
        }
    }

    ret = ctc_app_packet_eth_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_packet_eth_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }


	for (lchip = 0; lchip < g_ctc_app_master.lchip_num; lchip++)
    {
       ret = ctc_app_index_init(g_ctc_app_master.gchip_id[lchip]);
       if (ret != 0)
       {
           CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_index_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
           return ret;
       }
 	}


    return ret;
}

int32
ctc_app_sdk_init(ctc_init_cfg_t* p_init_config)
{
    int32 ret = 0;
    ctc_init_cfg_t* init_config = NULL;
    ctc_pkt_global_cfg_t pkt_cfg;
    ctc_nh_global_cfg_t    nh_global_cfg;
    ctc_oam_global_t  oam_global;
    ctc_l2_fdb_global_cfg_t l2_fdb_global_cfg;
    ctc_mpls_init_t* mpls_cfg = NULL;
    ctc_intr_global_cfg_t intr_cfg;
    ctc_dma_global_cfg_t* dma_cfg = NULL;
    ctc_bpe_global_cfg_t bpe_cfg;
    ctc_chip_global_cfg_t   chip_cfg;
    ctc_datapath_global_cfg_t* datapath_cfg = NULL;
    ctc_qos_global_cfg_t*      qos_cfg  =NULL;
    ctc_ipuc_global_cfg_t      ipuc_cfg;
    ctc_stats_global_cfg_t stats_cfg;
    ctc_stacking_glb_cfg_t*      stacking_cfg  =NULL;
    ctc_ptp_global_config_t*     ptp_cfg = NULL;
    uint8 lchip = 0;
    ctc_ftm_key_info_t* ftm_key_info = NULL;
    ctc_ftm_tbl_info_t* ftm_tbl_info = NULL;
    ctc_wb_api_t wb_api;

    sal_memset(&wb_api, 0, sizeof(ctc_wb_api_t));
    ftm_key_info = (ctc_ftm_key_info_t*)sal_malloc(sizeof(ctc_ftm_key_info_t)*CTC_FTM_KEY_TYPE_MAX);
    ftm_tbl_info = (ctc_ftm_tbl_info_t*)sal_malloc(sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);
    init_config = (ctc_init_cfg_t*)sal_malloc( sizeof(ctc_init_cfg_t));
    mpls_cfg = (ctc_mpls_init_t*)sal_malloc( sizeof(ctc_mpls_init_t));
    datapath_cfg = (ctc_datapath_global_cfg_t*)sal_malloc(sizeof(ctc_datapath_global_cfg_t)*CTC_MAX_LOCAL_CHIP_NUM);
    dma_cfg = (ctc_dma_global_cfg_t*)sal_malloc(sizeof(ctc_dma_global_cfg_t));
    qos_cfg = (ctc_qos_global_cfg_t*)sal_malloc(sizeof(ctc_qos_global_cfg_t));
    stacking_cfg = (ctc_stacking_glb_cfg_t*)sal_malloc(sizeof(ctc_stacking_glb_cfg_t));
    ptp_cfg = (ctc_ptp_global_config_t*)sal_malloc(sizeof(ctc_ptp_global_config_t));
    if ((NULL == ftm_key_info) || (NULL == ftm_tbl_info) || (NULL == init_config)
        || (NULL == mpls_cfg) || (NULL == datapath_cfg) || (NULL == ptp_cfg)
        || (NULL == dma_cfg) || (NULL == qos_cfg) || (NULL == stacking_cfg))
    {
        ret = CTC_E_NO_MEMORY;
        goto EXIT;
    }

    sal_memset(init_config,0,sizeof(ctc_init_cfg_t));
    sal_memset(&pkt_cfg, 0,  sizeof(ctc_pkt_global_cfg_t));
    sal_memset(&nh_global_cfg,0,sizeof(ctc_nh_global_cfg_t));
    sal_memset(&oam_global,0,sizeof(ctc_oam_global_t));
    sal_memset(&l2_fdb_global_cfg,0,sizeof(ctc_l2_fdb_global_cfg_t));
    sal_memset(mpls_cfg,0,sizeof(ctc_mpls_init_t));
    sal_memset(&intr_cfg,0,sizeof(ctc_intr_global_cfg_t));
    sal_memset(dma_cfg,0,sizeof(ctc_dma_global_cfg_t));
    sal_memset(ftm_key_info, 0, CTC_FTM_KEY_TYPE_MAX * sizeof(ctc_ftm_key_info_t));
    sal_memset(ftm_tbl_info, 0, CTC_FTM_TBL_TYPE_MAX * sizeof(ctc_ftm_tbl_info_t));
    sal_memset(&bpe_cfg,0,sizeof(ctc_bpe_global_cfg_t));
    sal_memset(&chip_cfg,0,sizeof(ctc_chip_global_cfg_t));
    sal_memset(datapath_cfg,0,sizeof(ctc_datapath_global_cfg_t)*CTC_MAX_LOCAL_CHIP_NUM);
    sal_memset(qos_cfg,0,sizeof(ctc_qos_global_cfg_t));
    sal_memset(&ipuc_cfg,0,sizeof(ctc_ipuc_global_cfg_t));
    sal_memset(&stats_cfg,0,sizeof(ctc_stats_global_cfg_t));
    sal_memset(stacking_cfg,0,sizeof(ctc_stacking_glb_cfg_t));
    sal_memset(ptp_cfg,0,sizeof(ctc_ptp_global_config_t));

    /* Config module init parameter, if set init_config.p_MODULE_cfg = NULL, using SDK default configration */
    init_config->p_pkt_cfg = &pkt_cfg;
    init_config->p_nh_cfg    = &nh_global_cfg;
    init_config->p_oam_cfg   = &oam_global;
    init_config->p_l2_fdb_cfg= &l2_fdb_global_cfg;
    init_config->p_mpls_cfg  = mpls_cfg;
    init_config->p_intr_cfg  = &intr_cfg;
    init_config->p_dma_cfg = dma_cfg;
    init_config->ftm_info.key_info = ftm_key_info;
    init_config->ftm_info.tbl_info = ftm_tbl_info;
    init_config->p_bpe_cfg = &bpe_cfg;
    init_config->p_chip_cfg = &chip_cfg;
    init_config->p_datapath_cfg = datapath_cfg;
    init_config->p_qos_cfg = qos_cfg;
    init_config->p_ipuc_cfg = &ipuc_cfg;
    init_config->p_stats_cfg = &stats_cfg;
    init_config->p_stacking_cfg = stacking_cfg;
    init_config->p_ptp_cfg = ptp_cfg;

    g_error_on = 1;

    /* init mem module */
    ret = mem_mgr_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mem_mgr_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        goto EXIT;
    }

    /* init debug module */
    ret = ctc_debug_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_debug_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        goto EXIT;
    }

    /* init sal module */
    ret = sal_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"sal_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        goto EXIT;
    }



#ifdef SDK_IN_USERMODE
    wb_api.enable = g_ctc_app_master.wb_enable;
    wb_api.reloading = g_ctc_app_master.wb_reloading;
    wb_api.init = &ctc_app_wb_init;
    wb_api.init_done = &ctc_app_wb_init_done;
    wb_api.sync = &ctc_app_wb_sync;
    wb_api.sync_done = &ctc_app_wb_sync_done;
    wb_api.add_entry = &ctc_app_wb_add_entry;
    wb_api.query_entry = &ctc_app_wb_query_entry;
#endif

    /* init sdk */
    if (0 == g_ctc_app_master.ctcs_api_en)
    {
        ret =  ctc_wb_init(lchip, &wb_api);
        if (ret != 0)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            goto EXIT;
        }

        /* get config info */
        ret = ctc_app_get_config(lchip, init_config, p_init_config);
        if (ret != 0)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_get_config failed:%s@%d \n",  __FUNCTION__, __LINE__);
            goto EXIT;
        }

        g_ctc_app_master.lchip_num = init_config->local_chip_num;
        sal_memcpy(g_ctc_app_master.gchip_id, init_config->gchip, sizeof(uint8)*CTC_MAX_LOCAL_CHIP_NUM);

        /*use ctc api functions */
        ret = ctc_sdk_init(init_config);
        if (ret != 0)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_sdk_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            goto EXIT;
        }
    }
    else
    {
        /*use ctcs api functions for per chip operation*/
        for (lchip = 0; lchip < CTC_MAX_LOCAL_CHIP_NUM; lchip++)
        {
            ret =  ctc_wb_init(lchip, &wb_api);
            if (ret != 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
                goto EXIT;
            }

            /* get config info */
            ret = ctc_app_get_config(lchip, init_config,p_init_config);
            if (ret != 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_get_config failed:%s@%d \n",  __FUNCTION__, __LINE__);
                goto EXIT;
            }
            if ((g_ctc_app_master.ctcs_api_en > 1) && (init_config->local_chip_num > 1))
            { /*rchain mode*/
                init_config->rchain_en = 1;
                init_config->rchain_gchip = 126;
            }

            if (lchip >= init_config->local_chip_num)
            {
                break;
            }

            g_ctc_app_master.lchip_num = init_config->local_chip_num;
            sal_memcpy(g_ctc_app_master.gchip_id, init_config->gchip, sizeof(uint8)*CTC_MAX_LOCAL_CHIP_NUM);

            ret = ctcs_sdk_init(lchip, init_config);
            if (ret != 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_sdk_init(lchip:%d) failed:%s@%d \n", lchip,  __FUNCTION__, __LINE__);
                goto EXIT;
            }
        }
    }

EXIT:
    if (ftm_key_info)
    {
        sal_free(ftm_key_info);
    }

    if (ftm_tbl_info)
    {
        sal_free(ftm_tbl_info);
    }

    if (mpls_cfg)
    {
        sal_free(mpls_cfg);
    }

    if (datapath_cfg)
    {
        sal_free(datapath_cfg);
    }

    if(dma_cfg)
    {
        sal_free(dma_cfg);
    }
    if(qos_cfg)
    {
        sal_free(qos_cfg);
    }

    if(stacking_cfg)
    {
        sal_free(stacking_cfg);
    }

    if(ptp_cfg)
    {
        sal_free(ptp_cfg);
    }

    if(NULL != init_config)
    {
        for (lchip = 0; lchip < g_ctc_app_master.lchip_num; lchip++)
        {
            if (init_config->phy_mapping_para[lchip])
            {
                sal_free(init_config->phy_mapping_para[lchip]);
            }
        }

        sal_free(init_config);
    }

    g_error_on = 0;

    return ret;
}

int32
ctc_app_sdk_deinit()
{
    int32 ret = 0;
    uint8 lchip = 0;

    if (0 == g_ctc_app_master.ctcs_api_en)
    {
        ret = ctc_sdk_deinit();
    }
    else
    {
        for (lchip = 0; lchip < CTC_MAX_LOCAL_CHIP_NUM; lchip++)
        {
            ret = ctcs_sdk_deinit(lchip);
        }
    }
    g_ctc_app_master.wb_reloading = 0;
    return ret;
}

/**
 @brief The sample code for initialize SDK

 @param[in] ctc_shell_mode  mode 0, get commands from screen.
                            mode 1, C/S mode, client and server communicate by socket/netlink. The shell server start up with SDK, and the client is ctc_shell.
                            mode 2, user define mode, input commands by ctc_vti_cmd_input() and output result to user by ctc_vti_cmd_output_register().

 @param[in] ctcs_api_en   ctc api mode or ctcs api mode

 @param[in] p_init_config  init parameter, if set NULL, using parameter from app code else using user defined parameter

 @remark  Refer to SDK PG and APP for detail description

 @return CTC_E_XXX

*/

int32
userinit(uint8 ctc_shell_mode, uint8 ctcs_api_en, void* p_init_config)
{
    int32 ret = 0;
    ctc_init_cfg_t* user_init_config = (ctc_init_cfg_t*)p_init_config;
    g_ctc_app_master.ctcs_api_en = ctcs_api_en;

#if defined CHIP_AGENT && defined _SAL_LINUX_UM
    /*for GG platform*/
    extern int32 ctc_chip_agent_init();
    ret = ctc_chip_agent_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_chip_agent_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }
#endif

    ret = ctc_app_sdk_init(user_init_config);
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_sdk_init failed, error code is %d \n", ret);
        return ret;
    }

    /* cli used for sdk start_up, for example serdes ffe typical cfg */
    ret = ctc_app_usr_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "app_usr_init failed, error code is %d \n", ret);
        return ret;
    }
    ret = ctc_app_sample_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_app_sample_init failed, error code is %d \n", ret);
        return ret;
    }

#if defined CHIP_AGENT && defined _SAL_LINUX_UM && (1 == SDK_WORK_PLATFORM)
    /* should init Chip Agent Mode after feature init finished, for GB platform */
    extern int32 chip_agent_init_mode();
    ret = chip_agent_init_mode();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"chip_agent_init_mode failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }
#endif

    ctc_master_cli(ctc_shell_mode);

#ifdef _SAL_LINUX_UM
    ctc_app_cli_init();
#endif

    if (ctc_shell_mode < 2)
    {
        ctc_cli_read(ctc_shell_mode);
    }
    return 0;
}
CTC_EXPORT_SYMBOL(userinit);

