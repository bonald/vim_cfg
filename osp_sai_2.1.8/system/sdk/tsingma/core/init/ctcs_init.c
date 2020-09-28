/**********************************************************
 * ctcs_init.c
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
#include "ctcs_api.h"
#include "ctc_chip.h"
#include "ctc_packet.h"
#include "ctc_init.h"
#include "ctc_warmboot.h"
/**********************************************************
 *
 * Defines and macros
 *
 **********************************************************/



/**********************************************************
 *
 * Global and Declaration
 *
 **********************************************************/

uint32 g_rchain_en[2] = {0};
ctc_init_cfg_t* g_init_cfg = NULL;
extern dal_op_t g_dal_op;

/**********************************************************
 *
 * Functions
 *
 **********************************************************/
extern int32 ctc_sdk_copy_global_cfg(ctc_init_cfg_t * p_cfg);
extern int32 ctc_sdk_clear_global_cfg(void);
int32
ctcs_sdk_init(uint8 lchip, void * p_global_cfg)
{
    int32  ret = CTC_E_NONE;
    uint8 do_init = 0;
    ctc_init_cfg_t * p_cfg = p_global_cfg;
    ctc_chip_device_info_t device_info;
    dal_op_t dal_cfg;

    char sdk_work_platform[2][32] = {"hardware platform", " software simulation platform"};

    CTC_PTR_VALID_CHECK(p_cfg);
    ctc_sdk_copy_global_cfg(p_cfg);
    if (p_cfg->rchain_en)
    {
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_PORT);
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_CHIP);
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_ACL);
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_QOS);
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_STACKING);
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_FTM);
        CTC_BMP_SET(g_rchain_en, CTC_FEATURE_REGISTER);
        if (lchip)
        {
            p_cfg->init_flag = 0;
            CTC_SET_FLAG(p_cfg->init_flag, CTC_INIT_MODULE_STACKING);
            CTC_SET_FLAG(p_cfg->init_flag, CTC_INIT_MODULE_APS);
            if (p_cfg->p_nh_cfg)
            {
                /*enable flex edit mode*/
                p_cfg->p_nh_cfg->nh_edit_mode = 2;
            }

            if (p_cfg->p_stacking_cfg)
            {
                /*enable stacking fabric mode*/
                p_cfg->p_stacking_cfg->fabric_mode = 1;
            }
        }
        else
        {
            if (p_cfg->p_stacking_cfg)
            {
                p_cfg->p_stacking_cfg->mcast_mode = 1;
            }
        }

        if (p_cfg->p_chip_cfg)
        {
            p_cfg->p_chip_cfg->rchain_en = 1;
            p_cfg->p_chip_cfg->rchain_gchip = p_cfg->rchain_gchip;
        }
    }

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

    /*install api  */
    ret = ctcs_install_api(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_install_api failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* chip init*/
    ret = ctcs_chip_init(lchip, p_cfg->local_chip_num);
    ret = ret ? ret : ctcs_set_gchip_id(lchip, p_cfg->gchip[lchip]);
    CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "lchip:%d gchip:%d\n", lchip, p_cfg->gchip[lchip]);
    if (ret < CTC_E_NONE)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_chip_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
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
    ret = ctcs_ftm_mem_alloc(lchip, &(p_cfg->ftm_info));
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ftm_alloc failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ctcs_chip_get_property(lchip, CTC_CHIP_PROP_DEVICE_INFO, (void*)&device_info);
    CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Chip name: %s, Device Id: %d, Version Id: %d \n", device_info.chip_name, device_info.device_id, device_info.version_id);

    /* datapath init */
    ret = ctcs_datapath_init(lchip, p_cfg->p_datapath_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_datapath_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    if (p_cfg->phy_mapping_para[lchip] && (SDK_WORK_PLATFORM == 0))
    {
        /* must be done before init phy */
        ctcs_chip_set_property(lchip, CTC_CHIP_PROP_PHY_MAPPING, p_cfg->phy_mapping_para[lchip]);
    }

    CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SDK_WORK_PLATFORM:%s \r\n local_chip_num:%d \r\n",
                    sdk_work_platform[SDK_WORK_PLATFORM], p_cfg->local_chip_num);

    /* the following is resource module */
    /* chip global config */
    ret = ctcs_set_chip_global_cfg(lchip, p_cfg->p_chip_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_set_chip_global_cfg failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* initializate common table/register */
    ret = ctcs_register_init(lchip, NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_register_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* interrupt init */
    ret = ctcs_interrupt_init(lchip, p_cfg->p_intr_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_interrupt_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* init dma */
    ret = ctcs_dma_init(lchip, p_cfg->p_dma_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_dma_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* nexthop */
    ret = ctcs_nexthop_init(lchip, p_cfg->p_nh_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_nexthop_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* l3if */
    ret = ctcs_l3if_init(lchip, p_cfg->p_l3if_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_l3if_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stats init */
    ret = ctcs_stats_init(lchip, p_cfg->p_stats_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_stats_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* qos */
    ret = ctcs_qos_init(lchip, p_cfg->p_qos_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_qos_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* learning aging init */
    ret = ctcs_learning_aging_init(lchip, p_cfg->p_learning_aging_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_learning_aging_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* SCL */
    ret = ctcs_scl_init(lchip, NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_scl_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* VLAN */
    ret = ctcs_vlan_init(lchip, p_cfg->p_vlan_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_vlan_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*parser*/
    ret = ctcs_parser_init(lchip, p_cfg->p_parser_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_parser_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*    port  */
    ret = ctcs_port_init(lchip, p_cfg->p_port_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_port_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctcs_internal_port_init(lchip, NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_internal_port_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* Linkagg */
    ret = ctcs_linkagg_init(lchip, p_cfg->p_linkagg_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_linkagg_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctcs_pdu_init(lchip, NULL);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_pdu_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctcs_packet_init(lchip, p_cfg->p_pkt_cfg);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_pdu_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*the following is feature module*/
    /*mirror*/
    ret = ctcs_mirror_init(lchip, NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_mirror_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* l2 init */
    ret = ctcs_l2_fdb_init(lchip, p_cfg->p_l2_fdb_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_l2_fdb_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* stp init */
    ret = ctcs_stp_init(lchip, NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_stp_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* acl init */
    ret = ctcs_acl_init(lchip, p_cfg->p_acl_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_acl_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* ipuc init */
    ret = ctcs_ipuc_init(lchip, p_cfg->p_ipuc_cfg);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ipuc_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* mpls init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_MPLS, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_mpls_init(lchip, p_cfg->p_mpls_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_mpls_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* ipmc init */
    ret = ctcs_ipmc_init(lchip, NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ipmc_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    /* aps init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_APS, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_aps_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_aps_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* security init */
    ret = ctcs_security_init(lchip, NULL);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_security_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* oam init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_OAM, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_oam_init(lchip, p_cfg->p_oam_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_oam_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /**ptp init*/
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_PTP, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_ptp_init(lchip, p_cfg->p_ptp_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ptp_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        }
    }


    /* SyncE init*/
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_SYNCE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_sync_ether_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_sync_ether_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* stacking init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_STACKING, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_stacking_init(lchip, p_cfg->p_stacking_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_stacking_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* bpe init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_BPE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_bpe_init(lchip, p_cfg->p_bpe_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_bpe_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* Ipfix init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_IPFIX, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_ipfix_init(lchip, p_cfg->p_ipifx_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ipfix_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* Monitor init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_MONITOR, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_monitor_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_monitor_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* Overlay init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_OVERLAY, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_overlay_tunnel_init(lchip, p_cfg->p_overlay_cfg);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_overlay_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }


    /* Efd init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_EFD, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_efd_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_efd_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* Trill init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_TRILL, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_trill_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_trill_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* Fcoe init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_FCOE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_fcoe_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_fcoe_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
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
        ret = ctcs_wlan_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_wlan_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* dot1ae init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_DOT1AE, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_dot1ae_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_dot1ae_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }

    /* Npm init */
    CTC_INIT_DO_INIT(CTC_INIT_MODULE_NPM, do_init, p_cfg->init_flag);
    if (do_init)
    {
        ret = ctcs_npm_init(lchip, NULL);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_npm_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
#endif


    /* warm boot ,must be the last */
    if (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        ret = ctc_wb_init_done(lchip);
        if (CTC_INIT_ERR_RET(ret))
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init_done:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }

        ret = ctcs_dma_init(lchip, p_cfg->p_dma_cfg);
        if (ret != CTC_E_NONE)
        {
            CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_dma_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));

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
ctcs_sdk_deinit(uint8 lchip)
{
    int32 ret = 0;

    /* chip deinit must be put at the beginning*/
    ret = ctcs_chip_deinit(lchip);
    if (ret < CTC_E_NONE)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_chip_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctcs_l2_fdb_deinit(lchip);
    if (ret < 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_l2_fdb_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* isr deinit */
    ret = ctcs_interrupt_deinit(lchip);
    if (ret < 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"interrupt deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* dma deinit */
    ret = ctcs_dma_deinit(lchip);
    if (ret < 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"dma deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

#if (FEATURE_MODE == 0)
    /* wlan deinit */
    ret = ctcs_wlan_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_wlan_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* npm deinit */
    ret = ctcs_npm_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_npm_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* dot1ae deinit */
    ret = ctcs_dot1ae_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_wlan_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
#endif
    /* fcoe deinit */
    ret = ctcs_fcoe_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_fcoe_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* trill deinit */
    ret = ctcs_trill_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_trill_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* efd deinit */
    ret = ctcs_efd_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_efd_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* overlay tunnel deinit */
    ret = ctcs_overlay_tunnel_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_overlay_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* monitor deinit */
    ret = ctcs_monitor_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_monitor_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ipfix deinit */
    ret = ctcs_ipfix_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ipfix_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* bpe deinit */
    ret = ctcs_bpe_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_bpe_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stacking deinit */
    ret = ctcs_stacking_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_stacking_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* sync ether deinit */
    ret = ctcs_sync_ether_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_sync_ether_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ptp deinit */
    ret = ctcs_ptp_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ptp_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
    }

    /* oam deinit */
    ret = ctcs_oam_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_oam_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* security deinit */
    ret = ctcs_security_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_security_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* aps deinit */
    ret = ctcs_aps_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_aps_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* learning aging deinit */
    ret = ctcs_learning_aging_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_learning_aging_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ipmc deinit */
    ret = ctcs_ipmc_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ipmc_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* mpls deinit */
    ret = ctcs_mpls_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_mpls_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ipuc deinit */
    ret = ctcs_ipuc_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ipuc_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* acl deinit */
    ret = ctcs_acl_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_acl_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stats deinit */
    ret = ctcs_stats_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_stats_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stp deinit */
    ret = ctcs_stp_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_stp_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*mirror deinit */
    ret = ctcs_mirror_deinit(lchip);
    if (CTC_INIT_ERR_RET(ret))
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_mirror_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* packet deinit*/
    ret = ctcs_packet_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_packet_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* pdu deinit*/
    ret = ctcs_pdu_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_pdu_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* linkagg deinit*/
    ret = ctcs_linkagg_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_linkagg_deinit failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* internal port deinit */
    ret = ctcs_internal_port_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_internal_port_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* port deinit */
    ret = ctcs_port_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_port_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* parser deinit */
    ret = ctcs_parser_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_parser_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* l3if deinit */
    ret = ctcs_l3if_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_l3if_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* vlan deinit */
    ret = ctcs_vlan_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_vlan_deinit failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* scl deinit */
    ret = ctcs_scl_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_scl_deinit failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* qos deinit */
    ret = ctcs_qos_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_qos_deinit failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* nexthop deinit */
    ret = ctcs_nexthop_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_nexthop_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* register deinit */
    ret = ctcs_register_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"ctcs_register_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* datapath deinit */
    ret = ctcs_datapath_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_datapath_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* ftm free */
    ret = ctcs_ftm_mem_free(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_ftm_free failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* uninstall api  */
    ret = ctcs_uninstall_api(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctcs_uninstall_api failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* dal deinit */
    ret = dal_op_deinit(lchip);
    if (ret != 0)
    {
        CTC_INIT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dal_op_deinit failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    return ret;
}
int32
ctcs_sdk_change_mem_profile(uint8 lchip, ctc_ftm_change_profile_t* p_profile)
{
    int32  ret = 0;
    ctc_ftm_key_info_t* p_tmp_key_info;
    ctc_ftm_tbl_info_t* p_tmp_tbl_info;
    ctc_wb_api_t wb_api;
    bool val = FALSE;

    CTC_ERROR_RETURN(ctcs_global_ctl_set(lchip, CTC_GLOBAL_NET_RX_EN, (void*)&val));
    CTC_ERROR_GOTO(ctcs_ftm_mem_change(lchip, p_profile), ret, end_0);

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
            CTC_ERROR_GOTO(ctcs_sdk_deinit(lchip), ret, end_0);
            CTC_ERROR_GOTO(ctcs_sdk_init(lchip, g_init_cfg), ret, end_0);
            break;
        case CTC_FTM_MEM_CHANGE_RECOVER:/*recover data by warmboot db*/
            CTC_ERROR_GOTO(ctc_wb_sync(lchip), ret, end_0);
            CTC_ERROR_GOTO(ctc_wb_sync_done(lchip, 0), ret, end_0);
            CTC_ERROR_GOTO(ctcs_sdk_deinit(lchip), ret, end_0);

            sal_memset(&wb_api, 0xFF, sizeof(wb_api));
            wb_api.reloading = 1;
            CTC_ERROR_GOTO(ctc_wb_init(lchip, &wb_api), ret, end_0);
            CTC_ERROR_GOTO(ctcs_sdk_init(lchip, g_init_cfg), ret, end_0);
            CTC_ERROR_GOTO(ctc_wb_init_done(lchip), ret, end_0);
            break;
        default:
            break;
    }
end_0:
    val = TRUE;
    ctcs_global_ctl_set(lchip, CTC_GLOBAL_NET_RX_EN, (void*)&val);
    return ret;
}
