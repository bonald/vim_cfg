/**********************************************************
 * ctc_init.c
 * Date:
 **********************************************************/
/**********************************************************
 *
 * Header file
 *
 **********************************************************/
#include "sal.h"
#include "dal.h"
#include "ctc_debug.h"
#include "ctc_api.h"
#include "ctc_chip.h"
#include "ctc_packet.h"
#include "ctc_init.h"
#include "ctc_warmboot.h"
/**********************************************************
 *
 * Defines and macros
 *
 **********************************************************/


uint8 g_lchip_num = 0;


/**********************************************************
 *
 * Global and Declaration
 *
 **********************************************************/

extern ctc_init_cfg_t* g_init_cfg;
extern dal_op_t g_dal_op;
/**********************************************************
 *
 * Functions
 *
 **********************************************************/
int32 ctc_sdk_clear_global_cfg(void)
{
    uint8 tmp_lchip;
    if(!g_init_cfg)
    {
        return CTC_E_NONE;
    }

    #define FREE_GLOBAL_CFG(ptr) if(ptr) sal_free(ptr)

    FREE_GLOBAL_CFG(g_init_cfg->p_chip_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_datapath_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_nh_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_qos_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_vlan_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_l3if_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_parser_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_port_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_linkagg_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_pkt_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_l2_fdb_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_stats_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_acl_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_ipuc_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_mpls_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_learning_aging_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_oam_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_ptp_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_intr_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_stacking_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_dma_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_bpe_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_overlay_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->p_ipifx_cfg);
    FREE_GLOBAL_CFG(g_init_cfg->dal_cfg);

    for(tmp_lchip=0; tmp_lchip < CTC_MAX_LOCAL_CHIP_NUM; tmp_lchip++)
    {
        FREE_GLOBAL_CFG(g_init_cfg->phy_mapping_para[tmp_lchip]);
    }

    FREE_GLOBAL_CFG(g_init_cfg->ftm_info.key_info);
    FREE_GLOBAL_CFG(g_init_cfg->ftm_info.tbl_info);

    FREE_GLOBAL_CFG(g_init_cfg);

    return CTC_E_NONE;
}
int32
ctc_sdk_copy_global_cfg(ctc_init_cfg_t * p_cfg)
{
    uint8 tmp_lchip;

    if(g_init_cfg)
    {
        return CTC_E_NONE;
    }
    g_init_cfg = sal_malloc(sizeof(ctc_init_cfg_t));
    if(NULL == g_init_cfg)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(g_init_cfg, 0, sizeof(ctc_init_cfg_t));
    g_init_cfg->init_flag = p_cfg->init_flag;
    sal_memcpy(&g_init_cfg->ftm_info, &p_cfg->ftm_info, sizeof(ctc_ftm_profile_info_t));
    g_init_cfg->ftm_info.key_info = NULL;
    g_init_cfg->ftm_info.tbl_info = NULL;
    g_init_cfg->ftm_info.key_info = (ctc_ftm_key_info_t*)sal_malloc(sizeof(ctc_ftm_key_info_t)*CTC_FTM_KEY_TYPE_MAX);
    if(!g_init_cfg->ftm_info.key_info)
    {
        goto error_0;
    }
    g_init_cfg->ftm_info.tbl_info = (ctc_ftm_tbl_info_t*)sal_malloc(sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);
    if(!g_init_cfg->ftm_info.tbl_info)
    {
        goto error_0;
    }
    sal_memcpy(g_init_cfg->ftm_info.tbl_info, p_cfg->ftm_info.tbl_info, sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);
    sal_memcpy(g_init_cfg->ftm_info.key_info, p_cfg->ftm_info.key_info, sizeof(ctc_ftm_key_info_t)*CTC_FTM_TBL_TYPE_MAX);
    sal_memcpy(g_init_cfg->gchip, p_cfg->gchip, sizeof(p_cfg->gchip));
    g_init_cfg->local_chip_num = p_cfg->local_chip_num;
    g_init_cfg->rchain_en = p_cfg->rchain_en;
    g_init_cfg->rchain_gchip = p_cfg->rchain_gchip;

    #define MALLOC_AND_COPY_MEM(dest, source, field, type) \
        do{\
            if(source->field)\
            {\
                dest->field = (type*)sal_malloc(sizeof(type));\
                if(NULL == dest->field)\
                    goto error_0;\
                sal_memcpy(dest->field, source->field, sizeof(type));\
            }\
        }while(0)

    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_chip_cfg, ctc_chip_global_cfg_t);
    g_init_cfg->p_datapath_cfg = (ctc_datapath_global_cfg_t*)sal_malloc(CTC_MAX_LOCAL_CHIP_NUM*sizeof(ctc_datapath_global_cfg_t));
    if(NULL == g_init_cfg->p_datapath_cfg)
    {
        goto error_0;
    }
    sal_memcpy(g_init_cfg->p_datapath_cfg, p_cfg->p_datapath_cfg, CTC_MAX_LOCAL_CHIP_NUM*sizeof(ctc_datapath_global_cfg_t));
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_nh_cfg, ctc_nh_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_qos_cfg, ctc_qos_global_cfg_t);

    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_vlan_cfg, ctc_vlan_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_l3if_cfg, ctc_l3if_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_parser_cfg, ctc_parser_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_port_cfg, ctc_port_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_linkagg_cfg, ctc_linkagg_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_pkt_cfg, ctc_pkt_global_cfg_t);

    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_l2_fdb_cfg, ctc_l2_fdb_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_stats_cfg, ctc_stats_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_acl_cfg, ctc_acl_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_ipuc_cfg, ctc_ipuc_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_mpls_cfg, ctc_mpls_init_t);

    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_learning_aging_cfg, ctc_learn_aging_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_oam_cfg, ctc_oam_global_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_ptp_cfg, ctc_ptp_global_config_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_intr_cfg, ctc_intr_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_stacking_cfg, ctc_stacking_glb_cfg_t);


    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_dma_cfg, ctc_dma_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_bpe_cfg, ctc_bpe_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_overlay_cfg, ctc_overlay_tunnel_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, p_ipifx_cfg, ctc_ipfix_global_cfg_t);
    MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, dal_cfg, dal_op_t);

    for(tmp_lchip=0; tmp_lchip < CTC_MAX_LOCAL_CHIP_NUM; tmp_lchip++)
    {
        MALLOC_AND_COPY_MEM(g_init_cfg, p_cfg, phy_mapping_para[tmp_lchip], ctc_chip_phy_mapping_para_t);
    }

    return CTC_E_NONE;
error_0:
    ctc_sdk_clear_global_cfg();
    return CTC_E_NO_MEMORY;
}
int32
ctc_sdk_init(void * p_global_cfg)
{
    int32  ret = CTC_E_NONE;
    uint8  do_init = 0;
    ctc_init_cfg_t * p_cfg = p_global_cfg;
    ctc_chip_device_info_t device_info;
    dal_op_t dal_cfg;
    char sdk_work_platform[2][32] = {"hardware platform", " software simulation platform"};
    uint8 i = 0;
    uint8 lchip = 0;

    CTC_PTR_VALID_CHECK(p_cfg);

    ctc_sdk_copy_global_cfg(p_cfg);
    if (p_cfg->local_chip_num > CTC_MAX_LOCAL_CHIP_NUM)
    {
        return CTC_E_INIT_FAIL;
    }

    if (p_cfg->rchain_en && !g_ctcs_api_en)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "route chain only support for ctcs mode \n");
        return CTC_E_INIT_FAIL;
    }

    for (lchip = 0; lchip < p_cfg->local_chip_num; lchip ++ )
    {
        if (p_cfg->dal_cfg)
        {
            sal_memcpy(&dal_cfg, p_cfg->dal_cfg, sizeof(dal_op_t));
        }
        else
        {
            sal_memcpy(&dal_cfg, &g_dal_op, sizeof(dal_op_t));
        }
        dal_cfg.lchip = lchip;
        ret = dal_op_init(&dal_cfg);
        if (ret != 0)
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dal_op_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /*install api  */
    ret = ctc_install_api();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_install_api failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* chip init*/
    ret = ctc_chip_init(p_cfg->local_chip_num);
    ret = ret ? ret : ctc_set_gchip_id(0, p_cfg->gchip[0]);
    CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "lchip:%d gchip:%d\n", 0, p_cfg->gchip[0]);

    if ((CTC_E_NONE == ret) && (p_cfg->local_chip_num > 1))
    {
        for(i = 1; i < p_cfg->local_chip_num; i++)
        {
            ctc_set_gchip_id(i, p_cfg->gchip[i]);
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "lchip:%d gchip:%d\n", i, p_cfg->gchip[i]);
        }
    }
    else if (ret < CTC_E_NONE)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_chip_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* dal init */
#ifdef _SAL_LINUX_UM
    ret = dal_set_device_access_type(DAL_PCIE_MM);

    if (ret < 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Failed to register dal callback\r\n");
        return ret;
    }
#endif

    /* ftm alloc */
    ret = ctc_ftm_mem_alloc(&(p_cfg->ftm_info));
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ftm_mem_alloc failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    for (lchip = 0; lchip < p_cfg->local_chip_num; lchip ++ )
    {
        ctc_chip_get_property(lchip, CTC_CHIP_PROP_DEVICE_INFO, (void*)&device_info);
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Lchip %d Chip name: %s, Device Id: %d, Version Id: %d \n", lchip, device_info.chip_name, device_info.device_id, device_info.version_id);
    }

    /* datapath init */
    for (lchip = 0; lchip < p_cfg->local_chip_num; lchip ++ )
    {
        ret = ctc_datapath_init(lchip, p_cfg->p_datapath_cfg);
        if (ret != 0)
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_datapath_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
    lchip = 0;

    if (p_cfg->phy_mapping_para[0] && (SDK_WORK_PLATFORM == 0))
    {
        /* must be done before init phy */
        for(i = 0; i < p_cfg->local_chip_num; i++)
        {
            ctc_chip_set_property(i, CTC_CHIP_PROP_PHY_MAPPING, p_cfg->phy_mapping_para[i]);
        }
    }
    CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SDK_WORK_PLATFORM:%s \r\n local_chip_num:%d \r\n",
                    sdk_work_platform[SDK_WORK_PLATFORM], p_cfg->local_chip_num);

    /* the following is resource module */
    /* chip global config */
    for (lchip = 0; lchip < p_cfg->local_chip_num; lchip++ )
    {
        CTC_PTR_VALID_CHECK(p_cfg->p_chip_cfg);
        p_cfg->p_chip_cfg->lchip = lchip;
        ret = ctc_set_chip_global_cfg(p_cfg->p_chip_cfg);
        if (ret != 0)
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_set_chip_global_cfg failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
    lchip = 0;

    /* initializate common table/register */
    ret = ctc_register_init(NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_register_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

     /* interrupt init */
    ret = ctc_interrupt_init(p_cfg->p_intr_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_interrupt_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* init dma */
    ret = ctc_dma_init(p_cfg->p_dma_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_dma_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* nexthop */
    ret = ctc_nexthop_init(p_cfg->p_nh_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_nexthop_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* l3if */
    ret = ctc_l3if_init(p_cfg->p_l3if_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_l3if_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stats init */
    ret = ctc_stats_init(p_cfg->p_stats_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_stats_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* qos */
    ret = ctc_qos_init(p_cfg->p_qos_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_qos_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* learning aging init */
    ret = ctc_learning_aging_init(p_cfg->p_learning_aging_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_learning_aging_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* SCL */
    ret = ctc_scl_init(NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_scl_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* VLAN */
    ret = ctc_vlan_init(p_cfg->p_vlan_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_vlan_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*parser*/
    ret = ctc_parser_init(p_cfg->p_parser_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_parser_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*    port  */
    ret = ctc_port_init(p_cfg->p_port_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_port_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_internal_port_init(NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_internal_port_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* Linkagg */
    ret = ctc_linkagg_init(p_cfg->p_linkagg_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_linkagg_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_pdu_init(NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_pdu_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_packet_init(p_cfg->p_pkt_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_packet_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*the following is feature module*/
    ret = ctc_mirror_init(NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_mirror_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* l2 init */
    ret = ctc_l2_fdb_init(p_cfg->p_l2_fdb_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_l2_fdb_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* stp init */
    ret = ctc_stp_init(NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_stp_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* acl init */
    ret = ctc_acl_init(p_cfg->p_acl_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_acl_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* ipuc init */
    ret = ctc_ipuc_init(p_cfg->p_ipuc_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ipuc_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* mpls init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_MPLS, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_mpls_init(p_cfg->p_mpls_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_mpls_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* ipmc init */
    ret = ctc_ipmc_init(NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ipmc_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* aps init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_APS, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_aps_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_aps_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* security init */
    ret = ctc_security_init(NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_security_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* oam init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_OAM, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_oam_init(p_cfg->p_oam_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_oam_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /**ptp init*/
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_PTP, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_ptp_init(p_cfg->p_ptp_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ptp_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        }
    }


    /* SyncE init*/
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_SYNCE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_sync_ether_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_sync_ether_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* stacking init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_STACKING, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_stacking_init(p_cfg->p_stacking_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_stacking_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* bpe init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_BPE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_bpe_init(p_cfg->p_bpe_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_bpe_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    CTC_INIT_DO_INIT(CTC_INIT_MODULE_IPFIX, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_ipfix_init(p_cfg->p_ipifx_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ipfix_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    CTC_INIT_DO_INIT(CTC_INIT_MODULE_MONITOR, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_monitor_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_monitor_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* Overlay init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_OVERLAY, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_overlay_tunnel_init(p_cfg->p_overlay_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_overlay_tunnel_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* Efd init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_EFD, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_efd_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_efd_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* Trill init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_TRILL, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_trill_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_trill_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* Fcoe init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_FCOE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_fcoe_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_fcoe_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    if (ret == CTC_E_NOT_SUPPORT)
    {
        ret =  CTC_E_NONE;
    }

#if (FEATURE_MODE == 0)
    /* Wlan init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_WLAN, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_wlan_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wlan_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* dot1ae init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_DOT1AE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_dot1ae_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_dot1ae_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* NPM init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_NPM, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctc_npm_init(NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_npm_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
#endif


    /* warm boot ,must be the last */
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        ret = ctc_wb_init_done(lchip);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init_done:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }

        ret = ctc_dma_init(p_cfg->p_dma_cfg);
        if (ret != CTC_E_NONE)
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_dma_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));

            return ret;
        }
    }

    if (ret == CTC_E_NOT_SUPPORT)
    {
        ret =  CTC_E_NONE;
    }

    return ret;
}

int32
ctc_sdk_deinit(void)
{
    int32 ret = 0;
    uint8 lchip = 0;

    /* chip deinit must be put at the beginning*/
    ret = ctc_chip_deinit();
    if (ret < CTC_E_NONE)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_chip_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* l2 deinit */
    ret = ctc_l2_fdb_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_l2_fdb_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* isr deinit */
    ret = ctc_interrupt_deinit();
    if (ret < 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"interrupt deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__,  ctc_get_error_desc(ret));
        return ret;
    }

    /* dma deinit */
    ret = ctc_dma_deinit();
    if (ret < 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"dma deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

#if (FEATURE_MODE == 0)
    /* wlan deinit */
    ret = ctc_wlan_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wlan_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* npm deinit */
    ret = ctc_npm_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_npm_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* dot1ae deinit */
    ret = ctc_dot1ae_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wlan_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
#endif

    /* fcoe deinit */
    ret = ctc_fcoe_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_fcoe_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* trill deinit */
    ret = ctc_trill_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_trill_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* efd deinit */
    ret = ctc_efd_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_efd_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* overlay tunnel deinit */
    ret = ctc_overlay_tunnel_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_overlay_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* monitor deinit */
    ret = ctc_monitor_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_monitor_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ipfix deinit */
    ret = ctc_ipfix_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ipfix_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* bpe deinit */
    ret = ctc_bpe_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_bpe_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stacking deinit */
    ret = ctc_stacking_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_stacking_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* sync ether deinit */
    ret = ctc_sync_ether_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_sync_ether_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ptp deinit */
    ret = ctc_ptp_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ptp_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
    }

    /* oam deinit */
    ret = ctc_oam_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_oam_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* security deinit */
    ret = ctc_security_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_security_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* aps deinit */
    ret = ctc_aps_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_aps_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* learning aging deinit */
    ret = ctc_learning_aging_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_learning_aging_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ipmc deinit */
    ret = ctc_ipmc_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ipmc_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* mpls deinit */
    ret = ctc_mpls_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_mpls_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ipuc deinit */
    ret = ctc_ipuc_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ipuc_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* acl deinit */
    ret = ctc_acl_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_acl_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stats deinit */
    ret = ctc_stats_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_stats_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stp deinit */
    ret = ctc_stp_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_stp_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /*mirror deinit */
    ret = ctc_mirror_deinit();
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_mirror_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* packet deinit*/
    ret = ctc_packet_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_packet_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* pdu deinit*/
    ret = ctc_pdu_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_pdu_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* linkagg deinit*/
    ret = ctc_linkagg_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_linkagg_deinit failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* internal port deinit */
    ret = ctc_internal_port_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_internal_port_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* port deinit */
    ret = ctc_port_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_port_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* parser deinit */
    ret = ctc_parser_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_parser_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* l3if deinit */
    ret = ctc_l3if_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_l3if_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* vlan deinit */
    ret = ctc_vlan_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_vlan_deinit failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* scl deinit */
    ret = ctc_scl_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_scl_deinit failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* qos deinit */
    ret = ctc_qos_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_qos_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* nexthop deinit */
    ret = ctc_nexthop_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_nexthop_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* register deinit */
    ret = ctc_register_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctc_register_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* datapath deinit */
    ret = ctc_datapath_deinit();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_datapath_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ftm free */
    ret = ctc_ftm_mem_free();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_ftm_free failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* uninstall api  */
    ret = ctc_uninstall_api();
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_uninstall_api failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* dal deinit */
    for (lchip = 0; lchip < g_lchip_num; lchip++ )
    {
        ret = dal_op_deinit(lchip);
        if (ret != 0)
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dal_op_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    return ret;
}

int32
ctc_sdk_change_mem_profile( ctc_ftm_change_profile_t* p_profile)
{
    int32  ret = 0;
    ctc_ftm_key_info_t* p_tmp_key_info;
    ctc_ftm_tbl_info_t* p_tmp_tbl_info;
    ctc_wb_api_t wb_api;
    bool val = FALSE;
    CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_NET_RX_EN, (void*)&val));
    CTC_ERROR_GOTO(ctc_ftm_mem_change(p_profile), ret, end_0);

    /*copy new ftm profile to g_init_cfg*/
    p_tmp_tbl_info = g_init_cfg->ftm_info.tbl_info;
    p_tmp_key_info = g_init_cfg->ftm_info.key_info;
    sal_memcpy(&g_init_cfg->ftm_info, p_profile->new_profile, sizeof(ctc_ftm_profile_info_t));
    g_init_cfg->ftm_info.key_info = p_tmp_key_info;
    g_init_cfg->ftm_info.tbl_info = p_tmp_tbl_info;
    sal_memcpy(p_tmp_key_info, p_profile->new_profile->key_info, p_profile->new_profile->key_info_size*sizeof(ctc_ftm_key_info_t));
    sal_memcpy(p_tmp_tbl_info, p_profile->new_profile->tbl_info, p_profile->new_profile->tbl_info_size*sizeof(ctc_ftm_tbl_info_t));

    switch(p_profile->change_mode)
    {
        case CTC_FTM_MEM_CHANGE_REBOOT:/*only deinit/init*/
            CTC_ERROR_GOTO(ctc_sdk_deinit(), ret, end_0);
            CTC_ERROR_GOTO(ctc_sdk_init(g_init_cfg), ret, end_0);
            break;
        case CTC_FTM_MEM_CHANGE_RECOVER:/*recover data by warmboot db*/
            CTC_ERROR_GOTO(ctc_wb_sync(0), ret, end_0);
            CTC_ERROR_GOTO(ctc_wb_sync_done(0, 0), ret, end_0);
            CTC_ERROR_GOTO(ctc_sdk_deinit(), ret, end_0);

            sal_memset(&wb_api, 0xFF, sizeof(wb_api));
            wb_api.reloading = 1;
            CTC_ERROR_GOTO(ctc_wb_init(0, &wb_api), ret, end_0);
            CTC_ERROR_GOTO(ctc_sdk_init(g_init_cfg), ret, end_0);
            CTC_ERROR_GOTO(ctc_wb_init_done(0), ret, end_0);
            break;
        default:
            break;
    }
end_0:
    val = TRUE;
    ctc_global_ctl_set(CTC_GLOBAL_NET_RX_EN, (void*)&val);
    return ret;
}
