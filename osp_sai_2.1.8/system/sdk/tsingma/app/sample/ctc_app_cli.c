/**
 @file ctc_app_packet_cli.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-25

 @version v2.0

 This file define packet sample CLI functions

*/

#include "api/include/ctc_api.h"
#include "ctc_cli.h"
#include "ctc_app_packet.h"
#include "ctc_cli_common.h"
#include "ctc_app.h"
#include "ctc_app_warmboot.h"
extern int32 ctc_app_sdk_deinit();
extern int32 ctc_app_sdk_init(ctc_init_cfg_t* p_init_config);
extern int32
ctc_app_isr_init(void);

extern int32 ctc_app_isr_init(void);
extern int32 ctcs_app_isr_init(uint8 lchip);
extern int32 ctc_app_read_ftm_profile(const int8* file_name,
                     ctc_ftm_profile_info_t* profile_info);
extern ctc_init_cfg_t* g_init_cfg;
CTC_CLI(ctc_cli_app_packet_show,
        ctc_cli_app_packet_show_cmd,
        "show app packet stats",
        CTC_CLI_SHOW_STR,
        CTC_CLI_APP_M_STR,
        "Packet",
        "Stats")
{
    ctc_app_packet_sample_show();
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_packet_print_rx,
        ctc_cli_app_packet_print_rx_cmd,
        "app packet cpu-rx-print (on|off)",
        CTC_CLI_APP_M_STR,
        "Packet",
        "Print CPU RX packet",
        "On",
        "Off")
{
    uint32 enable = FALSE;
    int32 ret = 0;

    if (0 == sal_strcmp(argv[0], "on"))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    ret = ctc_app_packet_sample_set_rx_print_en(enable);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_packet_create_socket,
        ctc_cli_app_packet_create_socket_cmd,
        "app packet create eth-socket ifid IFID",
        CTC_CLI_APP_M_STR,
        "Packet",
        "Create",
        "Socket",
        "Eth interface",
        "Interface Id")
{
    uint16 if_id = 0;
    int32 ret = 0;

    CTC_CLI_GET_UINT16_RANGE("if-id", if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    ret = ctc_app_packet_sample_eth_open_raw_socket(if_id);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_packet_destroy_socket,
        ctc_cli_app_packet_destroy_socket_cmd,
        "app packet destroy eth-socket",
        CTC_CLI_APP_M_STR,
        "Packet",
        "Destroy",
        "Socket")
{
    int32 ret = 0;
    ret = ctc_app_packet_sample_eth_close_raw_socket();
     if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

#ifdef _SAL_LINUX_UM
extern int32 ctc_app_packet_netif_rx(char* eth_name, uint8 rebind);
extern int32 ctc_app_packet_netif_tx(char* eth_name, uint8 rebind, uint8 *tx_pkt, uint16 pkt_len);
CTC_CLI(ctc_cli_app_packet_netif_rx_tx,
        ctc_cli_app_packet_netif_rx_tx_cmd,
        "app packet netif (rx|tx) NAME (rebind|) (lchip LCHIP|)",
        CTC_CLI_APP_M_STR,
        "Packet",
        "Netif",
        "Receive",
        "Send",
        "Netif Name",
        "Rebind new socket",
        "lchip",
        "LCHIP")
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 is_rx = 0;
    uint8 is_rebind = 0;
    char name[32];

    static uint8 pkt_buf_uc[CTC_PKT_MTU] = {
        0x00, 0x1F, 0xFE, 0xAF, 0xAD, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x81, 0x00, 0x00, 0x0A,
        0x08, 0x00, 0x45, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x11, 0xB8, 0xAF, 0x01, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x35, 0x00, 0x2A, 0xFC, 0x26, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    index = CTC_CLI_GET_SPECIFIC_INDEX("rx", 0);
    if (0xFF != index)
    {
        is_rx = 1;
    }
    sal_memcpy(name, argv[1], CTC_PKT_MAX_NETIF_NAME_LEN);

    index = CTC_CLI_GET_ARGC_INDEX("rebind");
    if (0xFF != index)
    {
        is_rebind = 1;
    }

    if (is_rx)
    {
        ret = ctc_app_packet_netif_rx(name, is_rebind);
    }
    else
    {
        ret = ctc_app_packet_netif_tx(name, is_rebind, pkt_buf_uc, 80);
    }
    if (ret < 0)
    {
        ctc_cli_out("netif rx error, ret = %d\n", ret);
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
#endif

CTC_CLI(ctc_cli_app_debug_on,
        ctc_cli_app_debug_on_cmd,
        "debug app app (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "App",
        CTC_CLI_APP_M_STR,
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    typeenum = APP_SMP;

    ctc_debug_set_flag("app", "app", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_debug_off,
        ctc_cli_app_debug_off_cmd,
        "no debug app app",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "App",
        CTC_CLI_APP_M_STR)
{
    uint32 typeenum = APP_SMP;
    uint8 level = 0;

    ctc_debug_set_flag("app", "app", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

extern int32 ctc_app_get_config(uint8 lchip, ctc_init_cfg_t * p_init_config, ctc_init_cfg_t * user_init_config);
CTC_CLI(ctc_cli_app_module_init,
        ctc_cli_app_module_init_cmd,
        "app sdk init (wb-reloading | lchip CHIPID |)",
        CTC_CLI_APP_M_STR,
        "SDK",
        "Init",
        "Warmboot reloading status",
        "Lchip",
        "Lchip ID")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_init_cfg_t* init_config = NULL;
    ctc_init_cfg_t* user_init_config = NULL;
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
    ctc_ftm_key_info_t* ftm_key_info = NULL;
    ctc_ftm_tbl_info_t* ftm_tbl_info = NULL;
    ctc_ptp_global_config_t* p_ptp_cfg;
    ctc_wb_api_t wb_api;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    if(lchip >= g_ctc_app_master.lchip_num)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Invalid chip id:%s@%d \n",  __FUNCTION__, __LINE__);
        return CTC_E_INVALID_CHIP_ID;
    }

    sal_memset(&wb_api, 0, sizeof(ctc_wb_api_t));
    ftm_key_info = (ctc_ftm_key_info_t*)sal_malloc(sizeof(ctc_ftm_key_info_t)*CTC_FTM_KEY_TYPE_MAX*3);
    ftm_tbl_info = (ctc_ftm_tbl_info_t*)sal_malloc(sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);
    init_config = (ctc_init_cfg_t*)sal_malloc( sizeof(ctc_init_cfg_t));
    mpls_cfg = (ctc_mpls_init_t*)sal_malloc( sizeof(ctc_mpls_init_t));
    datapath_cfg = (ctc_datapath_global_cfg_t*)sal_malloc(sizeof(ctc_datapath_global_cfg_t)*CTC_MAX_LOCAL_CHIP_NUM);
    dma_cfg = (ctc_dma_global_cfg_t*)sal_malloc(sizeof(ctc_dma_global_cfg_t));
    qos_cfg = (ctc_qos_global_cfg_t*)sal_malloc(sizeof(ctc_qos_global_cfg_t));
    stacking_cfg = (ctc_stacking_glb_cfg_t*)sal_malloc(sizeof(ctc_stacking_glb_cfg_t));
    p_ptp_cfg = (ctc_ptp_global_config_t*)sal_malloc(sizeof(ctc_ptp_global_config_t));
    if ((NULL == ftm_key_info) || (NULL == ftm_tbl_info) || (NULL == init_config)
        || (NULL == mpls_cfg) || (NULL == datapath_cfg)
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
    sal_memset(ftm_key_info, 0, 3*CTC_FTM_KEY_TYPE_MAX * sizeof(ctc_ftm_key_info_t));
    sal_memset(ftm_tbl_info, 0, CTC_FTM_TBL_TYPE_MAX * sizeof(ctc_ftm_tbl_info_t));
    sal_memset(&bpe_cfg,0,sizeof(ctc_bpe_global_cfg_t));
    sal_memset(&chip_cfg,0,sizeof(ctc_chip_global_cfg_t));
    sal_memset(datapath_cfg,0,CTC_MAX_LOCAL_CHIP_NUM*sizeof(ctc_datapath_global_cfg_t));
    sal_memset(qos_cfg,0,sizeof(ctc_qos_global_cfg_t));
    sal_memset(&ipuc_cfg,0,sizeof(ctc_ipuc_global_cfg_t));
    sal_memset(&stats_cfg,0,sizeof(ctc_stats_global_cfg_t));
    sal_memset(stacking_cfg,0,sizeof(ctc_stacking_glb_cfg_t));
    sal_memset(p_ptp_cfg, 0, sizeof(ctc_ptp_global_config_t));

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
    init_config->p_ptp_cfg = p_ptp_cfg;

    index = CTC_CLI_GET_ARGC_INDEX("wb-reloading");
    if (index != 0xFF)
    {
        g_ctc_app_master.wb_enable = 1;
        g_ctc_app_master.wb_reloading = 1;
    }


    /* init mem module */
    ret = mem_mgr_init();
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mem_mgr_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
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
    ret =  ctc_wb_init(lchip, &wb_api);
    if (ret != 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        goto EXIT;
    }

    ret = ctc_app_get_config(lchip, init_config, user_init_config);
    if (ret)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (g_ctc_app_master.ctcs_api_en)
    {
        ret = ctcs_sdk_init(lchip, init_config);
    }
    else
    {
        ret = ctc_sdk_init(init_config);
    }
    if (ret)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (g_ctc_app_master.ctcs_api_en)
    {
        ret = ctcs_app_isr_init(lchip);
    }
    else
    {
        ret = ctc_app_isr_init();
    }
    if (ret)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
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
    if(p_ptp_cfg)
    {
        sal_free(p_ptp_cfg);
    }

    if(NULL != init_config)
    {
        uint8 lchip_idx = 0;
        for(lchip_idx=0; lchip_idx < g_ctc_app_master.lchip_num; lchip_idx++)
        {
            init_config->phy_mapping_para[lchip_idx] ? sal_free(init_config->phy_mapping_para[lchip_idx]):0;
        }

        sal_free(init_config);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_module_deinit,
        ctc_cli_app_module_deinit_cmd,
        "app sdk deinit (lchip CHIPID|)",
        CTC_CLI_APP_M_STR,
        "SDK",
        "Deinit")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    if(lchip >= g_ctc_app_master.lchip_num)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Invalid chip id:%s@%d \n",  __FUNCTION__, __LINE__);
        return CTC_E_INVALID_CHIP_ID;
    }

    if (g_ctc_app_master.ctcs_api_en)
    {
        ret = ctcs_sdk_deinit(lchip);
    }
    else
    {
        ret = ctc_app_sdk_deinit();
    }

    ctc_wb_deinit(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% deinit fail\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_change_mem_profile,
    ctc_cli_app_change_mem_profile_cmd,
    "app ftm change-profile FILENAME (recover-en |)(lchip LCHIP|)",
    CTC_CLI_APP_M_STR,
    "Ftm",
    "Change memory profile",
    "File name",
    "Recover enable",
    "Lchip",
    "Lchip ID")
{

    uint8 lchip = 0;
    int32 ret = 0;
    uint8 index = 0;

    ctc_ftm_profile_info_t new_profile;
    ctc_ftm_change_profile_t profile;

    sal_memset(&new_profile, 0, sizeof(new_profile));
    sal_memset(&profile, 0, sizeof(profile));
    new_profile.key_info = (ctc_ftm_key_info_t*)sal_malloc(sizeof(ctc_ftm_key_info_t)*CTC_FTM_KEY_TYPE_MAX);
    new_profile.tbl_info = (ctc_ftm_tbl_info_t*)sal_malloc(sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);

    sal_memset(new_profile.key_info, 0, sizeof(ctc_ftm_key_info_t)*CTC_FTM_KEY_TYPE_MAX);
    sal_memset(new_profile.tbl_info, 0, sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);
    ret = ctc_app_read_ftm_profile((const int8*)argv[0], &new_profile);
    if(ret == CTC_E_NONE)
    {
        new_profile.profile_type = CTC_FTM_PROFILE_USER_DEFINE;
    }
    if(NULL == g_init_cfg)
    {
        ctc_cli_out("unexpect\n");
        ret =  CLI_ERROR;
        goto end_0;
    }

    profile.new_profile = &new_profile;
    profile.old_profile = &g_init_cfg->ftm_info;
    if(g_init_cfg->ftm_info.profile_type != CTC_FTM_PROFILE_USER_DEFINE)
    {
        ret =  CTC_E_NOT_SUPPORT;
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        goto end_0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("recover-en");
    if (index != 0xFF)
    {
        profile.recover_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if (g_ctc_app_master.ctcs_api_en)
    {
        ret = ctcs_sdk_change_mem_profile(lchip, &profile);
    }
    else
    {
        ret = ctc_sdk_change_mem_profile(&profile);
    }
    if (ret)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        ret =  CLI_ERROR;
        goto end_0;
    }

    if(profile.change_mode > 1)
    {
        if (g_ctc_app_master.ctcs_api_en)
        {
            ret = ctcs_app_isr_init(lchip);
        }
        else
        {
            ret = ctc_app_isr_init();
        }
        if (ret)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

end_0:
    if(new_profile.key_info)
    {
        sal_free(new_profile.key_info);
    }

    if(new_profile.tbl_info)
    {
        sal_free(new_profile.tbl_info);
    }
    return ret;
}

int32
ctc_app_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_app_packet_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_packet_print_rx_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_packet_create_socket_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_packet_destroy_socket_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_debug_off_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_app_module_init_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_app_module_deinit_cmd);
    install_element(CTC_APP_MODE, &ctc_cli_app_change_mem_profile_cmd);
#ifdef _SAL_LINUX_UM
    install_element(CTC_APP_MODE, &ctc_cli_app_packet_netif_rx_tx_cmd);
#endif

    return CLI_SUCCESS;
}

