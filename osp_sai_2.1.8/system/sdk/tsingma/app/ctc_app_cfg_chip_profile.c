/**
 @file ctc_read_chip_profile.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-7-5

 @version v2.0

 This file contains chip profile related function.
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "api/include/ctc_api.h"
#include "ctc_app_cfg_chip_profile.h"

#ifdef CHIP_AGENT
#include "chip_agent.h"
#ifdef GREATBELT
#include "greatbelt/include/drv_chip_agent.h"
#elif defined (GOLDENGATE)
#include "goldengate/include/drv_chip_agent.h"
#elif defined (DUET2)
#include "duet2/include/drv_chip_agent.h"
#endif
#endif

#include "ctc_app.h"
#include "ctc_app_isr.h"
#include "ctc_app_cfg_parse.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

#define WHITE_SPACE(C) ((C) == '\t' || (C) == ' ')
#define EMPTY_LINE(C)     ((C) == '\0' || (C) == '\r' || (C) == '\n')
#define NUMBER_CHAR(C) \
    ((C) == '0' || (C) == '1' || (C) == '1' || (C) == '2' || (C) == '3' || (C) == '4' \
     || (C) == '5' || (C) == '6' || (C) == '7' || (C) == '8' || (C) == '9')


/**
 @brief Define queue type
*/
enum ctc_queue_type
{
    CTC_NETWORK_EGRESS_QUEUE,
    CTC_STATCI_INT_PORT_QUEUE,
    CTC_FABRIC_QUEUE,
    CTC_INTERNAL_PORT_QUEUE,
    CTC_SERVICE_INGRESS_QUEUE,
    CTC_SERVICE_EGRESS_QUEUE,
    CTC_MAX_QUEUE_TYPE
};
typedef enum ctc_queue_type ctc_queue_type_e;

/****************************************************************************
 *
 * Global and Declaration
 *
 *****************************************************************************/
/*static ctc_queue_type_e s_queue_type;*/

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

STATIC int32
_do_parser_ipuc(ctc_app_parse_file_t* p_file, ctc_ipuc_global_cfg_t* p_ipuc_info)
{
    uint32 val = 0;
    uint8 entry_num = 0;

    if (!ctc_app_parse_file(p_file, "IPUC_TCAM_PREFIX_8", NULL, &val, &entry_num))
    {
        p_ipuc_info->use_hash8= (uint8)val;
    }

    return CTC_E_NONE;
}

STATIC int32
_do_parser(ctc_app_parse_file_t* p_file, ctc_init_chip_info_t* p_chip_info)
{
    uint32 val = 0;
    uint8 entry_num = 0;
    uint8 val_array[CTC_MAX_LOCAL_CHIP_NUM];

    sal_memset(val_array, 0, sizeof(val_array));

    entry_num = 1;
    if (!ctc_app_parse_file(p_file, "Local chip_num", NULL, &val, &entry_num))
    {
        if (val < 1 || val > CTC_MAX_LOCAL_CHIP_NUM)
        {
            return CTC_E_INVALID_PARAM;
        }
        else
        {
            p_chip_info->local_chip_num = (uint8)val;
        }
    }

    entry_num = CTC_MAX_LOCAL_CHIP_NUM;
    if (!ctc_app_parse_file(p_file, "Local chip", LETTER_NUMBER_MIX, &val_array, &entry_num))
    {
        sal_memcpy(p_chip_info->gchip, val_array, CTC_MAX_LOCAL_CHIP_NUM);
    }


    if (!ctc_app_parse_file(p_file, "Port_phy_mapping", NULL, &val, &entry_num))
    {
        p_chip_info->port_phy_mapping_en = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "Interrupt_mode", NULL, &val, &entry_num))
    {
        p_chip_info->interrupt_mode = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "IRQ", NULL, &val, &entry_num))
    {
        p_chip_info->irq = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "FTM Profile", NULL, &val, &entry_num))
    {
        p_chip_info->profile_type = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "Nexthop Edit Mode", NULL, &val, &entry_num))
    {
        p_chip_info->nh_dedit_mode = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "External Nexthop Number", NULL, &val, &entry_num))
    {
        p_chip_info->ext_nexthop_num = val;
    }

    if(!ctc_app_parse_file(p_file, "MPLS Tunnel Number", NULL, &val, &entry_num))
    {
        p_chip_info->mpls_tunnel_num = (uint16)val;
    }

    if(!ctc_app_parse_file(p_file, "MPLS ENTRY FOR SPACE 0", NULL, &val, &entry_num))
    {
        p_chip_info->mpls_entry_num[0] = val;
    }

    if(!ctc_app_parse_file(p_file, "MPLS ENTRY FOR SPACE 1", NULL, &val, &entry_num))
    {
        p_chip_info->mpls_entry_num[1]  = val;
    }

    if(!ctc_app_parse_file(p_file, "MPLS ENTRY FOR SPACE 2", NULL, &val, &entry_num))
    {
        p_chip_info->mpls_entry_num[2]  = val;
    }

    if(!ctc_app_parse_file(p_file, "MPLS ENTRY FOR SPACE 3", NULL, &val, &entry_num))
    {
        p_chip_info->mpls_entry_num[3] = val;
    }

    if(!ctc_app_parse_file(p_file, "FDB Hw Learning", NULL, &val, &entry_num))
    {
        p_chip_info->fdb_hw_learning_mode = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "Logic Port Num", NULL, &val, &entry_num))
    {
        p_chip_info->logic_port_num = val;
    }

    if(!ctc_app_parse_file(p_file, "STP MODE", NULL, &val, &entry_num))
    {
        p_chip_info->stp_mode = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "MAX_FID_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->max_fid_num = val;
    }


    if(!ctc_app_parse_file(p_file, "STATS_QUEUE_DEQ_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_queue_deq_en = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "STATS_QUEUE_DROP_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_queue_drop_en = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "STATS_FLOW_POLICER_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_flow_policer_en = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "STATS_VLAN_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_vlan_en = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "STATS_PORT_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_port_en = (val)?1:0;
    }

    p_chip_info->stats_ecmp_en = 1;
    if(!ctc_app_parse_file(p_file, "STATS_ECMP_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_ecmp_en = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "STATS_VRF_EN", NULL, &val, &entry_num))
    {
        p_chip_info->stats_vrf_en = (val)?1:0;
    }

    if(!ctc_app_parse_file(p_file, "STATS_POLICER_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->stats_policer_num = (uint16)val;
    }

    if(!ctc_app_parse_file(p_file, "CUT_THROUGH_SPEED", NULL, &val, &entry_num))
    {
        p_chip_info->cut_through_speed = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "BFD_MEP_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->bfd_mep_num = (uint16)val;
    }

    if(!ctc_app_parse_file(p_file, "BFD_VCCV_WITH_IP_EN", NULL, &val, &entry_num))
    {
        p_chip_info->bfd_vccv_with_ip_en = (val)?1:0;
    }

    if (!ctc_app_parse_file(p_file, "BPE_BR_PORT_EXTENDER_EN", NULL, &val, &entry_num))
    {
        p_chip_info->bpe_br_pe_en = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "BPE_BR_UC_MAX_ECID", NULL, &val, &entry_num))
    {
        p_chip_info->bpe_br_uc_max_ecid = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "BPE_BR_MC_MAX_ECID", NULL, &val, &entry_num))
    {
        p_chip_info->bpe_br_mc_max_ecid = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "BPE_BR_PORT_BASE", NULL, &val, &entry_num))
    {
        p_chip_info->bpe_br_port_base = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "CPU_NETWORK_PORT_EN", NULL, &val, &entry_num))
    {
        p_chip_info->cpu_port_en = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "CPU_NETWORK_PORT_ID", NULL, &val, &entry_num))
    {
        p_chip_info->cpu_port = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "ECC_RECOVER_EN", NULL, &val, &entry_num))
    {
        p_chip_info->ecc_recover_en = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "TCAM_SCAN_EN", NULL, &val, &entry_num))
    {
        p_chip_info->tcam_scan_en = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_POLICER_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->policer_num = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_PORT_QUEUE_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->queue_num_per_network_port = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_PORT_EXT_QUEUE_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->queue_num_per_internal_port = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_CPU_QUEUE_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->queue_num_for_cpu_reason = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_CPU_QUEUE_SHAPE_PROFILE_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->cpu_que_shp_profile_num = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_INGRESS_VLAN_POLICER_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->ingress_vlan_policer_num = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_EGRESS_VLAN_POLICER_NUM", NULL, &val, &entry_num))
    {
        p_chip_info->egress_vlan_policer_num = (uint16)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_POLICER_MERGE_MODE", NULL, &val, &entry_num))
    {
        p_chip_info->policer_merge_mode = (uint8)val;
    }

    if (!ctc_app_parse_file(p_file, "QOS_SERVICE_QUEUE_MODE", NULL, &val, &entry_num))
    {
        p_chip_info->service_queue_mode = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "FABRIC MODE", NULL, &val, &entry_num))
    {
        p_chip_info->fabric_mode = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "STACKING VERSION", NULL, &val, &entry_num))
    {
        p_chip_info->stacking_version = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "TRUNK_MODE", NULL, &val, &entry_num))
    {
        p_chip_info->stacking_mode = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "TRIE_SORT_EN", NULL, &val, &entry_num))
    {
        p_chip_info->trie_sort_en= (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "LAG_GB_GG_INTERCONNECT_EN", NULL, &val, &entry_num))
    {
        p_chip_info->lag_gb_gg_interconnect_en = (uint8)val;
    }

    if(!ctc_app_parse_file(p_file, "CUT_THROUGH_BITMAP", NULL, &val, &entry_num))
    {
        p_chip_info->cut_through_bitmap = (uint32)val;
    }

    if(!ctc_app_parse_file(p_file, "ALPM_CPU_MASK", NULL, &val, &entry_num))
    {
        p_chip_info->alpm_affinity_mask = (uint64)val;
    }

    if(!ctc_app_parse_file(p_file, "NORMAL_CPU_MASK", NULL, &val, &entry_num))
    {
        p_chip_info->normal_affinity_mask = (uint64)val;
    }

    return CTC_E_NONE;
}


STATIC int32
_do_parser_module_init(ctc_app_parse_file_t* p_file, ctc_init_chip_info_t* p_chip_info)
{
    uint32 val = 0;
    uint8 entry_num = 0;

    CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_FCOE - 1);

    val = 0;
    if (!ctc_app_parse_file(p_file, "MPLS_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_MPLS);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_MPLS);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "APS_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_APS);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_APS);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "OAM_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_OAM);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_OAM);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "PTP_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_PTP);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_PTP);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "SYNCE_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_SYNCE);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_SYNCE);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "STACKING_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_STACKING);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_STACKING);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "BPE_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_BPE);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_BPE);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "IPFIX_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_IPFIX);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_IPFIX);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "MONITOR_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_MONITOR);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_MONITOR);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "OVERLAY_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_OVERLAY);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_OVERLAY);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "EFD_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_EFD);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_EFD);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "FCOE_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_FCOE);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_FCOE);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "TRILL_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_TRILL);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_TRILL);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "WLAN_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_WLAN);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_WLAN);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "NPM_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_NPM);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_NPM);
        }
    }

    val = 0;
    if (!ctc_app_parse_file(p_file, "DOT1AE_SUPPORT", NULL, &val, &entry_num))
    {
        if(val)
        {
            CTC_SET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_DOT1AE);
        }
        else
        {
            CTC_UNSET_FLAG(p_chip_info->init_flag, CTC_INIT_MODULE_DOT1AE);
        }
    }

    return CTC_E_NONE;
}


int32
ctc_app_get_chip_profile(uint8* fname,
    ctc_init_cfg_t * p_init_config,
    ctc_init_chip_info_t* p_chip_info)
{
#ifdef _SAL_LINUX_UM
    struct stat stat_buf;
#endif
    char    filepath[128];
    int32   ret;
    ctc_app_parse_file_t file;

    ret = CTC_E_NONE;

    /* check whether has this file at /mnt/flash/  */
    sal_memset(filepath, 0, sizeof(filepath));
    sal_strcpy(filepath, (char*)fname);
#ifdef _SAL_LINUX_UM
    if (!stat(filepath, &stat_buf))
    {
        /* use file at  local*/
    }
    else
    {
        /* use file at flash */
        sal_memset(filepath, 0, sizeof(filepath));
        sal_sprintf(filepath, "/mnt/flash/%s", fname);
    }
#endif

    sal_memset(&file, 0, sizeof(ctc_app_parse_file_t));

    ret = ctc_app_parse_open_file((const char*)fname, &file);
    if (ret != CTC_E_NONE)
    {
        goto SET_DEF_CONFIG;
    }

    CTC_ERROR_RETURN(_do_parser_module_init(&file, p_chip_info));

    CTC_ERROR_RETURN(_do_parser(&file, p_chip_info));

    CTC_ERROR_RETURN(_do_parser_ipuc(&file, p_init_config->p_ipuc_cfg));

    CTC_ERROR_RETURN(ctc_app_parse_close_file(&file));

    return ret;

SET_DEF_CONFIG:
    /*set chip profile default */
    p_chip_info->local_chip_num = 1;
    p_chip_info->gchip[0] = 0;
    p_chip_info->port_phy_mapping_en = 0;
    p_chip_info->interrupt_mode = 0;
    p_chip_info->irq           = 0;
    p_chip_info->profile_type = 0;
    p_chip_info->nh_dedit_mode = 0;
    p_chip_info->ext_nexthop_num = 16384;
    p_chip_info->mpls_tunnel_num = 1024;
    p_chip_info->mpls_entry_num[0] = 1024;
    p_chip_info->mpls_entry_num[1] = 1024;
    p_chip_info->mpls_entry_num[2] = 1024;
    p_chip_info->mpls_entry_num[3] = 1024;
    p_chip_info->fdb_hw_learning_mode = 0;
    p_chip_info->logic_port_num = 1024;
    p_chip_info->stp_mode = 0;
    p_chip_info->stats_queue_deq_en = 1;
    p_chip_info->stats_queue_drop_en = 1;
    p_chip_info->stats_flow_policer_en = 1;
    p_chip_info->stats_vlan_en = 1;
    p_chip_info->stats_port_en = 0;
    p_chip_info->stats_ecmp_en = 1;
    p_chip_info->stats_vrf_en = 1;
    p_chip_info->stats_policer_num = 512;
    p_chip_info->cut_through_speed = 0;
    p_chip_info->bfd_mep_num = 2002;
    p_chip_info->bfd_vccv_with_ip_en = 1;
    p_chip_info->cpu_port_en = 0;
    p_chip_info->fabric_mode = 0;
    p_chip_info->stacking_version = 0;
    p_chip_info->stacking_mode = 0;
    p_chip_info->trie_sort_en  = 0;
    p_chip_info->lag_gb_gg_interconnect_en = 0;
    p_chip_info->ingress_vlan_policer_num = 0;
    p_chip_info->egress_vlan_policer_num = 0;
    p_chip_info->policer_merge_mode = 0;

    return CTC_E_NONE;
}


/*type: 0-mdio, 1-phy address*/
STATIC int32
_ctc_app_get_phy_map(uint8* fname, uint8 type, uint32* result)
{
    int32 ret;
    ctc_app_parse_file_t file;
    uint8 entry_num = 0;


    CTC_PTR_VALID_CHECK(fname);

    ret = ctc_app_parse_open_file((const char*)fname, &file);
    if(ret != CTC_E_NONE)
    {
        return ret;
    }

    entry_num = CTC_MAX_PHY_PORT;

    if (0 == type)
    {
        ret = ctc_app_parse_file(&file, "PHY_MAPPING_ITEM", "PHY_ADDR", result, &entry_num);
    }
    else if (1 == type)
    {
        ret = ctc_app_parse_file(&file, "PHY_MAPPING_ITEM", "MDIO_BUS", result, &entry_num);
    }
    else
    {
        ret = ctc_app_parse_file(&file, "PHY_MAPPING_ITEM", "API_PORT", result, &entry_num);
    }

    ctc_app_parse_close_file(&file);

    return CTC_E_NONE;
}

int32
ctc_app_set_phy_mapping(uint8* fname, ctc_init_cfg_t * p_init_config, ctc_init_chip_info_t* p_chip_info)
{
    uint16 index = 0;
    uint32* tmp_gport = NULL;
    uint32* tmp_phy = NULL;
    uint32* tmp_mdio = NULL;
    uint8 gchip = 0;
    uint16 lport = 0;
    uint8 lchip = 0;
    int32 ret = CTC_E_NONE;

    /* init */

    tmp_gport = (uint32*)sal_malloc(sizeof(uint32)*MAX_PORT_NUM_PER_CHIP*p_chip_info->local_chip_num);
    tmp_phy = (uint32*)sal_malloc(sizeof(uint32)*MAX_PORT_NUM_PER_CHIP*p_chip_info->local_chip_num);
    tmp_mdio = (uint32*)sal_malloc(sizeof(uint32)*MAX_PORT_NUM_PER_CHIP*p_chip_info->local_chip_num);
    if ((NULL == tmp_gport) || (NULL == tmp_phy) || (NULL == tmp_mdio))
    {
        ret = CTC_E_NO_MEMORY;
        goto error;
    }
    sal_memset(tmp_gport, CTC_MAX_UINT8_VALUE, sizeof(uint32)*MAX_PORT_NUM_PER_CHIP*p_chip_info->local_chip_num);
    sal_memset(tmp_phy, CTC_MAX_UINT8_VALUE, sizeof(uint32)*MAX_PORT_NUM_PER_CHIP*p_chip_info->local_chip_num);
    sal_memset(tmp_mdio, CTC_MAX_UINT8_VALUE, sizeof(uint32)*MAX_PORT_NUM_PER_CHIP*p_chip_info->local_chip_num);

    for (index = 0; index < p_chip_info->local_chip_num; ++ index)
    {
        p_init_config->phy_mapping_para[index] =
                (ctc_chip_phy_mapping_para_t*)sal_malloc(sizeof(ctc_chip_phy_mapping_para_t));
        if (NULL == p_init_config->phy_mapping_para[index])
        {
            for (index = 0; index < p_chip_info->local_chip_num; ++ index)
            {
                if (p_init_config->phy_mapping_para[index])
                {
                    sal_free(p_init_config->phy_mapping_para[index]);
                    p_init_config->phy_mapping_para[index] = NULL;
                }
            }
            ret = CTC_E_NO_MEMORY;
            goto error;
        }
        sal_memset(p_init_config->phy_mapping_para[index], CTC_MAX_UINT8_VALUE, sizeof(ctc_chip_phy_mapping_para_t));
    }

    /*Get port phy mdio mapping*/
    if (p_chip_info->port_phy_mapping_en)
    {
        ret = _ctc_app_get_phy_map(fname, 0, tmp_phy);
        if (CTC_E_NONE != ret)
        {
            goto error;
        }
        ret = _ctc_app_get_phy_map(fname, 1, tmp_mdio);
        if (CTC_E_NONE != ret)
        {
            goto error;
        }
        ret = _ctc_app_get_phy_map(fname, 2, tmp_gport);
        if (CTC_E_NONE != ret)
        {
            goto error;
        }

        /*Get port phy address mapping*/

        for (index = 0; index < MAX_PORT_NUM_PER_CHIP * p_chip_info->local_chip_num; index++)
        {
            if (0xFF != tmp_gport[index])
            {
                gchip = CTC_MAP_GPORT_TO_GCHIP(tmp_gport[index]);
                lport = CTC_MAP_GPORT_TO_LPORT(tmp_gport[index]);

                for (lchip = 0; lchip < p_chip_info->local_chip_num; ++ lchip)
                {
                    if (gchip == p_chip_info->gchip[lchip])
                    {
                        break;
                    }
                }

                if (lchip < p_chip_info->local_chip_num)
                {
                    p_init_config->phy_mapping_para[lchip]->port_mdio_mapping_tbl[lport] = tmp_mdio[index];
                    p_init_config->phy_mapping_para[lchip]->port_phy_mapping_tbl[lport] = tmp_phy[index];
                }
            }
        }
    }
error:
    if (tmp_gport)
    {
        sal_free(tmp_gport);
    }
    if (tmp_phy)
    {
        sal_free(tmp_phy);
    }

    if (tmp_mdio)
    {
        sal_free(tmp_mdio);
    }
    return ret;
}


#if defined CHIP_AGENT && (1 == SDK_WORK_PLATFORM)
STATIC int32
_get_interage(uint8* string, uint32* integer)
{
    uint8* ch = NULL;
    uint32 val = 0;

    ch = (uint8*)sal_strstr((char*)string, "=");

    if (NULL == ch)
    {
        return -1;
    }
    else
    {
        ch++;
    }

    while (sal_isspace(*ch))
    {
        ch++;
    }

    sal_sscanf((char*)ch, "%d", &val);
    *integer = val;

    return 0;
}

STATIC int32
_get_string(uint8* string, uint8* str)
{
    uint8* ch = NULL;

    ch = (uint8*)sal_strstr((char*)string, "=");

    if (NULL == ch)
    {
        return -1;
    }
    else
    {
        ch++;
    }

    while (sal_isspace(*ch))
    {
        ch++;
    }

    sal_strcpy((char*)str, (char*)ch);

    return 0;
}

STATIC int32
_chip_agent_cfg_string_atrim(char* output, const char* input)
{
    char* p = NULL;

    /*trim left space*/
    while (*input != '\0')
    {
        if (WHITE_SPACE(*input))
        {
            ++input;
        }
        else
        {
            break;
        }
    }

    sal_strcpy(output, input);
    /*trim right space*/
    p = output + sal_strlen(output) - 1;

    while (p >= output)
    {
        /*skip empty line*/
        if (WHITE_SPACE(*p) || ('\r' == (*p)) || ('\n' == (*p)))
        {
            --p;
        }
        else
        {
            break;
        }
    }

    *(++p) = '\0';

    return 0;
}

STATIC int32
_chip_agent_cfg_parser_line(char* line, chip_agent_t* p_chip_agent)
{
    char gw_ip[64];
    char server_ip[64];
    struct sal_sockaddr_in addr;
    uint32 port = 0;
    int32 ret = 0;

    sal_memset(gw_ip, 0, sizeof(gw_ip));
    sal_memset(server_ip, 0, sizeof(server_ip));
    sal_memset(&addr, 0, sizeof(addr));

    if (0 == sal_strncmp("[Mode]", line, sal_strlen("[Mode]")))
    {
        if (NULL != sal_strstr(line, "client"))
        {
            p_chip_agent->mode = CHIP_AGT_MODE_CLIENT;
        }
        else if (NULL != sal_strstr(line, "server"))
        {
            p_chip_agent->mode = CHIP_AGT_MODE_SERVER;
        }
    }
    else if (0 == sal_strncmp("[Server IP]", line, sal_strlen("[Server IP]")))
    {
        ret = _get_string((uint8 *)line, (uint8 *)server_ip);

        if (ret < 0)
        {
            return -1;
        }
        else
        {
            ret = sal_inet_pton(AF_INET, server_ip, &addr.sin_addr);

            if (ret != 0)
            {
                if((p_chip_agent->cfg.server_count) < CHIP_AGT_MAX_SERVER)
                {
                    sal_strncpy(p_chip_agent->cfg.addr[p_chip_agent->cfg.server_count++], server_ip, 20);
                }
            }
        }
    }
    else if (0 == sal_strncmp("[TCP Port]", line, sal_strlen("[TCP Port]")))
    {
        p_chip_agent->cfg.tcp_port = 0;
        ret = _get_interage((uint8 *)line, &port);

        if (ret < 0)
        {
            return -1;
        }
        else
        {
            p_chip_agent->cfg.tcp_port = port;
        }
    }
    else if (0 == sal_strncmp("[CPU Mirror Port]", line, sal_strlen("[CPU Mirror Port]")))
    {
        p_chip_agent->cfg.mirror_port = 0;
        ret = _get_interage((uint8 *)line, &port);

        if (ret < 0)
        {
            return -1;
        }
        else
        {
            p_chip_agent->cfg.mirror_port = port;
        }
    }
    else if (0 == sal_strncmp("[Gateway IP]", line, sal_strlen("[Gateway IP]")))
    {
        ret = _get_string((uint8 *)line, (uint8 *)gw_ip);

        if (ret < 0)
        {
            return -1;
        }
        else
        {
            ret = sal_inet_pton(AF_INET, gw_ip, &addr.sin_addr);

            if (ret != 0)
            {
                sal_strncpy(p_chip_agent->cfg.gw, gw_ip, 20);
            }
        }
    }
    else if (0 == sal_strncmp("[Situation]", line, sal_strlen("[Situation]")))
    {
        if (NULL != sal_strstr(line, "standalone"))
        {
            p_chip_agent->cfg.standalone = TRUE;
        }
        else
        {
            p_chip_agent->cfg.standalone = FALSE;
        }
    }
    else if (0 == sal_strncmp("[Engine]", line, sal_strlen("[Engine]")))
    {
        if (NULL != sal_strstr(line, "asic"))
        {
            p_chip_agent->cfg.is_asic = TRUE;
        }
        else
        {
            p_chip_agent->cfg.is_asic = FALSE;
        }
    }

    return 0;
}

STATIC int32
_chip_agent_cfg_read(const char* file_name, chip_agent_t* p_agent)
{
    char string[64];
    char line[64];
    sal_file_t local_fp = NULL;

    local_fp = sal_fopen(file_name, "r");

    if (NULL == local_fp)
    {
        p_agent->mode = CHIP_AGT_MODE_NONE;
        return 0;
    }
    sal_memset(string, 0, sizeof(string));
    /*parse profile*/
    while (sal_fgets(string, 64, local_fp))
    {
        sal_memset(line, 0, sizeof(line));

        /*comment line*/
        if ('#' == string[0])
        {
            continue;
        }

        if (EMPTY_LINE(string[0]))
        {
            continue;
        }

        _chip_agent_cfg_string_atrim(line, string);
        _chip_agent_cfg_parser_line(line, p_agent);
    }

    /*close file*/
    sal_fclose(local_fp);
    local_fp = NULL;

    return 0;
}

int32
ctc_app_chip_agent_init(const char* file_name, chip_agent_t* p_agent)
{
    char file_path[32];

    sal_memset(file_path, 0, sizeof(file_path));

#if (SDK_WORK_PLATFORM == 1)  /*simulation */
    sal_strcat(file_path, "./");
#else
    /* config file always on flash for board */
    sal_strcat(file_path, "/mnt/flash/");
#endif
    sal_strcat(file_path, file_name);

    /* If your system cannot read file system, you can hardcode to client mode */
#if 1
    _chip_agent_cfg_read(file_path, p_agent);
#else
    g_chip_agent.mode = CHIP_AGT_MODE_CLIENT;
    sal_strncpy(g_chip_agent.cfg.addr, "10.10.29.100", 20);
    g_chip_agent.cfg.tcp_port = CHIP_AGT_SOCKET_PORT;
#endif

    return 0;
}
#endif


