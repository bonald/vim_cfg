#if (FEATURE_MODE == 0)
/**
 @file sys_usw_l2_fdb.c

 @date 2009-10-19

 @version v2.0

The file implement   software simulation for MEP tx
*/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_oam.h"

#include "sys_usw_chip.h"
#include "sys_usw_ftm.h"

#include "sys_usw_oam_db.h"
#include "sys_usw_oam_cfm_db.h"
#include "sys_usw_oam_tp_y1731_db.h"
#include "sys_usw_oam_bfd_db.h"
#include "sys_usw_oam_cfm.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_oam_com.h"

#include "drv_api.h"


struct sys_oam_debug_type_s
{
    uint16 session_count;
    uint16 oam_type;
    uint8 lchip;
    uint8 resv;
};
typedef struct sys_oam_debug_type_s sys_oam_debug_type_t;


typedef int32 (*SYS_OAM_DEBUG_SHOW_FN_t)(uint8 lchip, sys_oam_chan_com_t*  p_sys_chan,sys_oam_lmep_t*  p_sys_lmep, sys_oam_rmep_t*  p_sys_rmep, sys_oam_debug_type_t* p_oam_debug);

extern sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM];

extern int32
_sys_usw_cfm_oam_get_bridge_mac(uint8 lchip, mac_addr_t* bridge_mac);


int32
sys_usw_oam_show_oam_property(uint8 lchip, ctc_oam_property_t* p_prop)
{
    ctc_oam_y1731_prop_t* p_eth_prop = NULL;
    p_eth_prop  = &p_prop->u.y1731;

    SYS_OAM_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(_sys_usw_cfm_get_property(lchip, p_prop));

    switch (p_eth_prop->cfg_type)
    {
    case CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN:
        if (CTC_BOTH_DIRECTION != p_eth_prop->dir)
        {
            SYS_OAM_DBG_DUMP("GPort  0x%04x %s OAM En %d\r\n", p_eth_prop->gport,
                             ((p_eth_prop->dir == CTC_INGRESS) ? "Ingress" : "Egress"), p_eth_prop->value);
        }
        else
        {
            p_eth_prop->dir = CTC_INGRESS;
            CTC_ERROR_RETURN(_sys_usw_cfm_get_property(lchip, p_prop));
            SYS_OAM_DBG_DUMP("GPort  0x%04x Ingress OAM En %d\r\n", p_eth_prop->gport, p_eth_prop->value);
            p_eth_prop->dir = CTC_EGRESS;
            CTC_ERROR_RETURN(_sys_usw_cfm_get_property(lchip, p_prop));
            SYS_OAM_DBG_DUMP("GPort  0x%04x Egress OAM En %d\r\n", p_eth_prop->gport, p_eth_prop->value);
        }

        break;

    case CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN:
        SYS_OAM_DBG_DUMP("GPort  0x%04x  Tunnel En %d\r\n", p_eth_prop->gport, p_eth_prop->value);
        break;

    case CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN:
        SYS_OAM_DBG_DUMP("GPort  0x%04x  LM En %d\r\n", p_eth_prop->gport, p_eth_prop->value);
        break;

    case CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN:
        SYS_OAM_DBG_DUMP("GPort  0x%04x  MIP EN level_bitmap 0x%02x\r\n", p_eth_prop->gport, p_eth_prop->value);
        break;

    case CTC_OAM_Y1731_CFG_TYPE_IF_STATUS:
        SYS_OAM_DBG_DUMP("GPort  0x%04x  interface status 0x%02x\r\n", p_eth_prop->gport, p_eth_prop->value);
        break;
    case CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU:
        SYS_OAM_DBG_DUMP("Big ccm interval    %u\r\n", p_eth_prop->value);
        break;
    case CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC:
        SYS_OAM_DBG_DUMP("LBM proc in asic    %s\r\n", p_eth_prop->value ? "N" : "Y");
        break;
    case CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC:
        SYS_OAM_DBG_DUMP("DM proc in asic     %s\r\n", p_eth_prop->value ? "N" : "Y");
        break;
    case CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC:
        SYS_OAM_DBG_DUMP("SLM proc in asic    %s\r\n", p_eth_prop->value ? "N" : "Y");
        break;
    case CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC:
        SYS_OAM_DBG_DUMP("LMM proc in asic    %s\r\n", p_eth_prop->value ? "N" : "Y");
        break;
    case CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA:
        SYS_OAM_DBG_DUMP("LBR use LBM mcada   %s\r\n", p_eth_prop->value ? "Y" : "N");
        break;
    case CTC_OAM_Y1731_CFG_TYPE_TP_ACH_CHAN_TYPE:
        SYS_OAM_DBG_DUMP("TP ach channel type 0x%02x\r\n", p_eth_prop->value);
        break;
    case CTC_OAM_Y1731_CFG_TYPE_RDI_MODE:
        SYS_OAM_DBG_DUMP("RDI mode            %u\r\n", p_eth_prop->value);
        break;
    case CTC_OAM_Y1731_CFG_TYPE_POAM_LM_STATS_EN:
        SYS_OAM_DBG_DUMP("POAM LM STATS EN    %s\r\n", p_eth_prop->value ? "Y" : "N");
        break;
    default:
        break;
    }

    return CTC_E_NONE;
}

#define __M_OAM_INTERNAL__
int32
sys_usw_oam_internal_property(uint8 lchip)
{
    SYS_OAM_INIT_CHECK(lchip);

    g_oam_master[lchip]->mep_index_alloc_by_sdk = 0;

    return CTC_E_NONE;
}

int32
sys_usw_oam_internal_maid_property(uint8 lchip, ctc_oam_maid_len_format_t maid_len)
{
    SYS_OAM_INIT_CHECK(lchip);

    if (maid_len == 0 || maid_len > 3)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid configuration of maid len format!!\n");
        return CTC_E_INVALID_CONFIG;
    }

    g_oam_master[lchip]->maid_len_format = maid_len;

    return CTC_E_NONE;
}

int32
sys_usw_clear_event_cache(uint8 lchip)
{
    uint32 entry_num = 0;

    SYS_OAM_INIT_CHECK(lchip);

    sys_usw_ftm_query_table_entry_num(lchip, DsEthMep_t, &entry_num);

    if (g_oam_master[lchip]->mep_defect_bitmap)
    {
        sal_memset((uint8*)g_oam_master[lchip]->mep_defect_bitmap, 0, entry_num*4);
    }

    return CTC_E_NONE;
}

int32
sys_usw_oam_tp_section_init(uint8 lchip, uint8 use_port)
{
    uint32 is_port  = 0;
    uint32 cmd      = 0;

    SYS_OAM_INIT_CHECK(lchip);

    is_port = (1 == use_port) ? 1: 0;
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_mplsSectionOamUsePort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &is_port));

    cmd = DRV_IOW(EpeAclQosCtl_t, EpeAclQosCtl_mplsSectionOamUsePort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &is_port));

    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_portbasedSectionOam_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &is_port));

    g_oam_master[lchip]->tp_section_oam_based_l3if = (1 == use_port) ? 0: 1;

    return CTC_E_NONE;

}

#define __M_OAM_ISR__
STATIC int32
_sys_usw_oam_isr_get_defect_name(uint8 lchip, ctc_oam_event_entry_t* mep_info)
{

    uint32 mep_defect_bitmap_cmp    = 0;
    uint32 mep_defect_bitmap_temp   = 0;
    uint32 mep_index = 0;
    uint8  index     = 0;
    SYS_LCHIP_CHECK_ACTIVE(lchip);

    if (!mep_info->is_remote)
    {
        mep_index = mep_info->lmep_index;
        SYS_OAM_DBG_DUMP("  mep_index        : 0x%-4x\r\n",  mep_index);

        mep_defect_bitmap_temp = ((mep_info->event_bmp & CTC_OAM_DEFECT_UNEXPECTED_LEVEL)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_MISMERGE)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_UNEXPECTED_MEP)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_RDI_TX)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_DOWN)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_INIT)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_UP)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_CSF)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_SF));
    }
    else
    {
        mep_index = mep_info->rmep_index;
        SYS_OAM_DBG_DUMP("  rmep_index       : 0x%-4x\r\n",  mep_index);
        mep_defect_bitmap_temp = ((mep_info->event_bmp & CTC_OAM_DEFECT_DLOC)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_UNEXPECTED_PERIOD)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_MAC_STATUS_CHANGE)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_SRC_MAC_MISMATCH)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_RDI_RX)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_CSF)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_DOWN)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_INIT)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_UP)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT)
                                  | (mep_info->event_bmp & CTC_OAM_DEFECT_SF));
    }

    if (NULL == g_oam_master[lchip]->mep_defect_bitmap)
    {
        return CTC_E_INVALID_PTR;
    }

    mep_defect_bitmap_cmp = mep_defect_bitmap_temp ^ (g_oam_master[lchip]->mep_defect_bitmap[mep_index]);

    if (0 == mep_defect_bitmap_cmp)
    {
        SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "Defect no name");
    }

    for (index = 0; index < 32; index++)
    {
        if (CTC_IS_BIT_SET(mep_defect_bitmap_cmp, index))
        {
            switch (1 << index)
            {
            case CTC_OAM_DEFECT_EVENT_BFD_DOWN:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_BFD_DOWN))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_DOWN);
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_INIT);
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_UP);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "BFD state down");
                }
                break;

            case CTC_OAM_DEFECT_EVENT_BFD_INIT:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_BFD_INIT))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_INIT);
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_DOWN);
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_UP);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "BFD state init");
                }
                break;

            case CTC_OAM_DEFECT_EVENT_BFD_UP:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_BFD_UP))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_UP);
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_INIT);
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_DOWN);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "BFD state up");
                }
                break;

            case CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "1st packet set");
                }
                else
                {

                }

                break;

            case CTC_OAM_DEFECT_CSF:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_CSF))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_CSF);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "CSFDefect set");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_CSF);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "CSFDefect: clear");
                }

                break;

            case CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "BFD MIS CONNECT");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "BFD MIS CONNECT clear");
                }

                break;

            case CTC_OAM_DEFECT_UNEXPECTED_LEVEL:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_UNEXPECTED_LEVEL))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_UNEXPECTED_LEVEL);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "xconCCMdefect: low ccm");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_UNEXPECTED_LEVEL);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "xconCCMdefect: low ccm clear");
                }

                break;

            case CTC_OAM_DEFECT_MISMERGE:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_MISMERGE))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_MISMERGE);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "xconCCMdefect: ma id mismatch");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_MISMERGE);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "xconCCMdefect: ma id mismatch clear");
                }

                break;

            case CTC_OAM_DEFECT_UNEXPECTED_PERIOD:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_UNEXPECTED_PERIOD))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_UNEXPECTED_PERIOD);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "errorCCMdefect: ccm interval mismatch");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_UNEXPECTED_PERIOD);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "errorCCMdefect: ccm interval mismatch clear");
                }

                break;

            case CTC_OAM_DEFECT_DLOC:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_DLOC))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_DLOC);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SomeRMEPCCMDefect: dloc defect");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_DLOC);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SomeRMEPCCMDefect: dloc defect clear");
                }

                break;

            case CTC_OAM_DEFECT_UNEXPECTED_MEP:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_UNEXPECTED_MEP))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_UNEXPECTED_MEP);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "errorCCMdefect: rmep not found");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_UNEXPECTED_MEP);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "errorCCMdefect: rmep not found clear");
                }

                break;

            case CTC_OAM_DEFECT_EVENT_RDI_RX:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_RDI_RX))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_RDI_RX);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SomeRDIdefect: rdi defect");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_RDI_RX);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SomeRDIdefect: rdi defect clear");
                }

                break;

            case CTC_OAM_DEFECT_MAC_STATUS_CHANGE:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_MAC_STATUS_CHANGE))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_MAC_STATUS_CHANGE);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SomeMACstatusDefect");
                }

                break;

            case CTC_OAM_DEFECT_SRC_MAC_MISMATCH:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_SRC_MAC_MISMATCH))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_SRC_MAC_MISMATCH);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "RMEP SRC MAC Mismatch");
                }

                break;

            case CTC_OAM_DEFECT_EVENT_RDI_TX:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_EVENT_RDI_TX))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_RDI_TX);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "TX RDI set");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_EVENT_RDI_TX);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "TX RDI clear");
                }

                break;

            case CTC_OAM_DEFECT_SF:
                if (CTC_FLAG_ISSET(mep_defect_bitmap_temp, CTC_OAM_DEFECT_SF))
                {
                    CTC_SET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_SF);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SF defect");
                }
                else
                {
                    CTC_UNSET_FLAG(g_oam_master[lchip]->mep_defect_bitmap[mep_index], CTC_OAM_DEFECT_SF);
                    SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "SF defect clear");
                }

                break;
            default:
                SYS_OAM_DBG_DUMP("  defect name: %s\r\n", "Defect no name");
                break;
            }
        }
    }

    return CTC_E_NONE;
}


 int32
sys_usw_isr_oam_process_isr(uint8 gchip, void* p_data)
{
    uint16 mep_num       = 0;
    ctc_oam_event_t* p_event = NULL;
    uint8 lchip = 0;

    CTC_PTR_VALID_CHECK(p_data);
    SYS_OAM_INIT_CHECK(lchip);
    SYS_LCHIP_CHECK_ACTIVE(lchip);

    sys_usw_get_local_chip_id(gchip, &lchip);

    p_event = (ctc_oam_event_t*)p_data;

    for (mep_num = 0; mep_num < p_event->valid_entry_num; mep_num++)
    {
        _sys_usw_oam_isr_get_defect_name(lchip, &p_event->oam_event_entry[mep_num]);

        if (p_event->oam_event_entry[mep_num].is_remote)
        {
            p_event->oam_event_entry[mep_num].is_remote = 0;
            _sys_usw_oam_isr_get_defect_name(lchip, &p_event->oam_event_entry[mep_num]);
        }
    }

    return CTC_E_NONE;
}


#define __M_OAM_SHOW__
int32
sys_usw_oam_show_oam_status(uint8 lchip)
{
    mac_addr_t share_mac;
    sys_oam_master_t* p_oam_master = NULL;

    uint32 mep_entry_num        = 0;
    uint32 ma_entry_num         = 0;
    uint32 ma_name_entry_num    = 0;
    uint32 lm_entry_num         = 0;
    uint32 v6addr_entry_num     = 0;

    uint32 session_y1731        = 0;
    uint32 session_ethernet     = 0;
    uint32 session_tp_y1731     = 0;

    uint32 session_bfd          = 0;
    uint32 session_bfd_tp       = 0;

    static char* maid_str[] = {"Auto", "16 bytes", "32 bytes", "48 bytes"};

    SYS_OAM_INIT_CHECK(lchip);

    p_oam_master = g_oam_master[lchip];


    _sys_usw_cfm_oam_get_bridge_mac(lchip, &share_mac);


    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsEthMep_t,   &mep_entry_num));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsMa_t,       &ma_entry_num));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsMaName_t,   &ma_name_entry_num));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsOamLmStats_t,  &lm_entry_num));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsBfdV6Addr_t,  &v6addr_entry_num));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------Work Mode------------------------------------\n");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %s\n","MEP Index alloc mode", (p_oam_master->mep_index_alloc_by_sdk ? "SDK":"SYSTEM"));
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %.4x.%.4x.%.4x\n", "Share MAC", sal_ntohs(*(unsigned short*)&share_mac[0]),
                                                sal_ntohs(*(unsigned short*)&share_mac[2]), sal_ntohs(*(unsigned short*)&share_mac[4]));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %s\n","MAID length mode", maid_str[p_oam_master->maid_len_format]);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s:\n %-32s %s\n", "Build-in Process OAM PDU", " ", "CCM/LBM/DMM/LMM/TST/SLM/BFD/DLM/DM/DLMDM");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s:\n %-32s %s\n", "Assistant CPU Process OAM PDU", " ", "LTM/LTR/LBR/DMR/1DM/SLR/LCK/SCC/MCC");

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n-------------------------------OAM Resource ---------------------------------\n");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","OAM MEP ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",mep_entry_num);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_oam_master->oam_reource_num[SYS_OAM_TBL_MEP]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","OAM MA ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",ma_entry_num);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_oam_master->oam_reource_num[SYS_OAM_TBL_MA]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","OAM MANAME ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",ma_name_entry_num);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_oam_master->oam_reource_num[SYS_OAM_TBL_MANAME]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","OAM LM ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",lm_entry_num);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_oam_master->oam_reource_num[SYS_OAM_TBL_LM]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","OAM Lookup Key ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",mep_entry_num);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_oam_master->oam_reource_num[SYS_OAM_TBL_LOOKUP_KEY]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","OAM BFD IPV6ADDR ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",v6addr_entry_num);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", p_oam_master->oam_reource_num[SYS_OAM_TBL_BFDV6ADDR]);


    session_ethernet     = p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_ETH_P2P]
                            + p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_ETH_P2MP];
    session_tp_y1731     = p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_TP_SECTION]
                            + p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_TP_MPLS];

    session_y1731        = session_ethernet + session_tp_y1731;


    session_bfd_tp       = p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_TP_SECTION]
                            + p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_TP_MPLS];

    session_bfd          = session_bfd_tp + p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_IPv4]
                            + p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_IPv6]
                            + p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_MPLS]
                            + p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_MICRO];

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n-----------------------------Created OAM(session)----------------------------\n");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Y1731", session_y1731);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Ethernet OAM", session_ethernet);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","---P2P", p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_ETH_P2P]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","---P2MP", p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_ETH_P2MP]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--MPLS-TP Y1731 OAM", session_tp_y1731);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","---Section OAM", p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_TP_SECTION]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","---LSP/PW OAM", p_oam_master->oam_session_num[SYS_OAM_SESSION_Y1731_TP_MPLS]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","BFD", session_bfd);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--IPV4 BFD", p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_IPv4]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--IPV6 BFD", p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_IPv6]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--MPLS BFD", p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_MPLS]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Micro BFD", p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_MICRO]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--MPLS-TP BFD", session_bfd_tp);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","---Section BFD", p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_TP_SECTION]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","---LSP/PW BFD", p_oam_master->oam_session_num[SYS_OAM_SESSION_BFD_TP_MPLS]);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--MEP ON CPU", p_oam_master->oam_session_num[SYS_OAM_SESSION_MEP_ON_CPU]);

    return CTC_E_NONE;
}


int32
sys_usw_oam_show_oam_defect_status(uint8 lchip)
{
    uint32 max_defect = 0;
    uint32 defect_type = 0;
    uint8 shift = 0;

    sys_oam_defect_info_t defect_info;

    SYS_OAM_INIT_CHECK(lchip);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s%6s%6s\n", "DefectType", "   RDI", "   Event");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------\n");


    max_defect = CTC_OAM_DEFECT_ALL;
    while(max_defect)
    {
        defect_type = (1 << shift);
        sal_memset(&defect_info, 0, sizeof(sys_oam_defect_info_t));
        sys_usw_oam_get_defect_type_config(lchip, defect_type, &defect_info);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s%6s%6s\n", defect_info.defect_name,
                                        (defect_info.to_rdi ? "Y" : "N"), (defect_info.to_event? "Y" : "N"));

        max_defect = (max_defect >> 1);
        shift++;
    }
    return CTC_E_NONE;
}


STATIC char*
_sys_usw_oam_get_cpu_reseaon_str(uint8 lchip, uint32 id)
{
    switch(id)
    {
        case CTC_OAM_EXCP_INVALID_OAM_PDU:
            return "Individual OAM PDU";
            break;

        case CTC_OAM_EXCP_HIGH_VER_OAM_TO_CPU:
            return "Higher version OAM PDU";
            break;

        case CTC_OAM_EXCP_OPTION_CCM_TLV:
            return "CCM has optional TLV";
            break;

        case CTC_OAM_EXCP_APS_PDU_TO_CPU:
            return "APS PDU";
            break;

        case CTC_OAM_EXCP_DM_TO_CPU:
            return "DM PDU";
            break;

        case CTC_OAM_EXCP_PBT_MM_DEFECT_OAM_PDU:
            return "PBT mmDefect OAM PDU";
            break;

        case CTC_OAM_EXCP_LM_TO_CPU:
            return "LM PDU";
            break;

        case CTC_OAM_EXCP_ETH_TST_TO_CPU:
            return "Test PDU";
            break;

        case CTC_OAM_EXCP_BIG_INTERVAL_OR_SW_MEP_TO_CPU:
            return "Big CCM/BFD interval to CPU or MEP configured in software";
            break;

        case CTC_OAM_EXCP_MPLS_TP_DLM_TO_CPU:
            return "MPLS-TP DLM PDU";
            break;

        case CTC_OAM_EXCP_MPLS_TP_DM_DLMDM_TO_CPU:
            return "MPLS-TP DM/DLMDM PDU";
            break;

        case CTC_OAM_EXCP_CSF_TO_CPU:
            return "Y1731 CSF PDU";
            break;

        case CTC_OAM_EXCP_MCC_PDU_TO_CPU:
            return "Y1731 MCC PDU";
            break;

        case CTC_OAM_EXCP_EQUAL_LTM_LTR_TO_CPU:
            return "LTM/LTR PDU";
            break;

        case CTC_OAM_EXCP_LEARNING_BFD_TO_CPU:
            return "Learning BFD to CPU";
            break;

        case CTC_OAM_EXCP_BFD_TIMER_NEGOTIATION:
            return "BFD timer negotiation";
            break;

        case CTC_OAM_EXCP_SCC_PDU_TO_CPU:
            return "Y1731 SCC PDU";
            break;

        case CTC_OAM_EXCP_ALL_DEFECT:
            return "All defect to CPU";
            break;

        case CTC_OAM_EXCP_LBM:
            return "Ether LBM/LBR and MPLS-TP LBR PDU";
            break;

        case CTC_OAM_EXCP_TP_LBM:
            return "MPLS-TP LBM PDU";
            break;

        case CTC_OAM_EXCP_TP_CSF:
            return "MPLS-TP CSF PDU";
            break;

        case CTC_OAM_EXCP_ETH_SC_TO_CPU:
            return "ETH SC PDU";
            break;

        case CTC_OAM_EXCP_ETH_LL_TO_CPU:
            return "ETH LL PDU";
            break;

        case CTC_OAM_EXCP_TP_FM:
            return "MPLS-TP FM PDU";
            break;

        case CTC_OAM_EXCP_SM:
            return "Ether SLM/SLR PDU";
            break;

        case CTC_OAM_EXCP_TP_BFD_CV:
            return "TP BFD CV PDU";
            break;

        case CTC_OAM_EXCP_UNKNOWN_PDU:
            return "Unknown PDU";
            break;

        case CTC_OAM_EXCP_LEARNING_CCM_TO_CPU:
            return "Learning CCM to CPU";
            break;

        case CTC_OAM_EXCP_BFD_DISC_MISMATCH:
            return "BFD discreaminator mismatch PDU";
            break;

        case CTC_OAM_EXCP_ALL_TO_CPU:
            return "ALL OAM PDU TO CPU";
            break;

        case CTC_OAM_EXCP_MACDA_CHK_FAIL:
            return "MACDA CHK FAIL";
            break;

        case CTC_OAM_EXCP_TWAMP_TO_CPU:
            return "TWAMP PDU";
            break;

    }
    return "-";
}

int32
sys_usw_oam_show_oam_cpu_reason(uint8 lchip)
{

    uint8 id = 0;
    char* p_str = NULL;

    SYS_OAM_INIT_CHECK(lchip);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-24s\n", "Id", "Reason");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------\n");
    for(id = 0; id < 32; id++)
    {
       p_str = _sys_usw_oam_get_cpu_reseaon_str(lchip, id);
       SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d%-24s\n", id, p_str);
    }

    return CTC_E_NONE;
}


/*
L£ºSection/Link OAM   W£ºVPWS     V£ºVPLS
D: Dm En              M£ºMEP En   T£ºCC Tx En
S£ºSingleHop
I£ºSBFD-Initiator  R£ºSBFD-Reflector
*/

STATIC int32
_sys_usw_oam_get_mep_flag(uint8 lchip, ctc_oam_key_t *p_oam_key, ctc_oam_mep_info_t *mep_info, char *flag)
{

    if(CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
    {
        sal_strcat(flag, "L");
    }

    if(CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_L2VPN))
    {
        if(CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_VPLS))
        {
            sal_strcat(flag, "V");
        }
        else
        {
            sal_strcat(flag, "W");
        }
    }

    if((CTC_OAM_MEP_TYPE_ETH_1AG == mep_info->mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_info->mep_type)
        ||(CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_info->mep_type))
    {
        if(mep_info->lmep.y1731_lmep.active)
        {
            sal_strcat(flag, "M");
        }

        if(mep_info->lmep.y1731_lmep.ccm_enable)
        {
            sal_strcat(flag, "T");
        }

        if(mep_info->lmep.y1731_lmep.dm_enable)
        {
            sal_strcat(flag, "D");
        }
    }
    else
    {
        if(mep_info->lmep.bfd_lmep.mep_en)
        {
            sal_strcat(flag, "M");
        }

        if(mep_info->lmep.bfd_lmep.cc_enable)
        {
            sal_strcat(flag, "T");
        }

        if ((mep_info->lmep.bfd_lmep.single_hop)
            &&((CTC_OAM_MEP_TYPE_IP_BFD == mep_info->mep_type) || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_info->mep_type)))
        {
            sal_strcat(flag, "S");
        }
    }

    if(mep_info->is_rmep)
    {

    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_oam_get_defect_report(uint8 lchip, uint32 mep_index, uint8* b_report)
{

    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 idx = 0;
    uint32 bit = 0;

    idx = mep_index/32;
    bit = mep_index%32;

    cmd = DRV_IOR(DsOamDefectStatus_t, DsOamDefectStatus_defectStatus_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &field_val));

    if(CTC_IS_BIT_SET(field_val, bit))
    {
        *b_report = 1;
    }
    else
    {
        *b_report = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_oam_show_oam_mep_detail(uint8 lchip, ctc_oam_key_t *p_oam_key)
{
    sys_oam_chan_com_t*     p_sys_chan  = NULL;
    sys_oam_lmep_t*   p_sys_lmep  = NULL;
    sys_oam_rmep_t*   p_sys_rmep  = NULL;
    int32 ret = CTC_E_NONE;
    uint8 b_report = 0;

    static uint32 r_index[128]     = {0};
    static uint32 r_key_index[128] = {0};
    uint8  r_count          = 0;
    uint32 cnt              = 0;

    ctc_slist_t* p_rmep_list = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;

    static ctc_oam_mep_info_t mep_info;
    static char *bfd_state[4] = {"Admin Down","Down", "Init", "Up"};

    char str_port[64]   = {0};
    char str_space[64]  = {0};
    char str_label[64]  = {0};
    char flag[64]       = {0};
    char csf_type[64]   = {0};
    uint8 is_tp_oam     = 0;

    SYS_OAM_INIT_CHECK(lchip);

	sal_memset(r_index,0,sizeof(r_index));
	sal_memset(r_key_index,0,sizeof(r_key_index));
    sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == p_oam_key->mep_type)
         ||(CTC_OAM_MEP_TYPE_ETH_Y1731 == p_oam_key->mep_type))
    {
        p_sys_chan = (sys_oam_chan_com_t*)_sys_usw_cfm_chan_lkup(lchip, p_oam_key);
        if (NULL == p_sys_chan)
        {
            ret = CTC_E_NOT_EXIST;
            goto RETURN;
        }
        /*3. lookup lmep and check exist*/
        p_sys_lmep = (sys_oam_lmep_t*)_sys_usw_cfm_lmep_lkup(lchip, p_oam_key->u.eth.md_level, (sys_oam_chan_eth_t*)p_sys_chan);
        if (NULL == p_sys_lmep)
        {
            ret = CTC_E_NOT_EXIST;
            goto RETURN;
        }
        if (p_sys_lmep->mep_on_cpu)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%15s %s\n", "", "MEP on CPU");
            goto RETURN;
        }

        if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            goto Print_key_0;
        }


        sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
        mep_info.mep_index = p_sys_lmep->lmep_index;
        _sys_usw_oam_get_mep_info(lchip, &mep_info);

        _sys_usw_oam_get_mep_flag(lchip, p_oam_key, &mep_info, flag);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n%s\n", "Flag    L: Section/Link OAM   W: VPWS     V: VPLS",
                                                          "        D: Dm En              M: MEP En   T: CC Tx En    S: SingleHop");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------------------------\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
            "Port", "Vlan", "Cvlan", "VpnId", "MdLvl", "Up", "Interval", "Defect", "Flag");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------------------------\n");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x%-10u%-10u%-10u%-10d%-10s%-10d%-10s%-10s\n",
            p_oam_key->u.eth.gport,
            p_oam_key->u.eth.vlan_id,
            p_oam_key->u.eth.cvlan_id,
            p_oam_key->u.eth.l2vpn_oam_id,
            p_oam_key->u.eth.md_level,
            (mep_info.lmep.y1731_lmep.is_up_mep ? "Y" : "N"),
            mep_info.lmep.y1731_lmep.ccm_interval,
            ((mep_info.lmep.y1731_lmep.d_meg_lvl | mep_info.lmep.y1731_lmep.d_mismerge | mep_info.lmep.y1731_lmep.d_unexp_mep)? " Y" : " N"),
             flag);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Status\n");

        _sys_usw_oam_get_defect_report(lchip, p_sys_lmep->lmep_index, &b_report);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-14s%-12s%-10s%-8s%-14s\n",
            "LMepId", " dUnExpMep", " dMisMerge", " dMegLvl", " TxRDI", " ReportDefect");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u%-14s%-12s%-10s%-8s%-10s\n",
            mep_info.lmep.y1731_lmep.mep_id,
            (mep_info.lmep.y1731_lmep.d_unexp_mep ? " Y" :" N"),
            (mep_info.lmep.y1731_lmep.d_mismerge ? " Y" :" N"),
            (mep_info.lmep.y1731_lmep.d_meg_lvl ? " Y" :" N"),
            (mep_info.lmep.y1731_lmep.present_rdi ? " Y" :" N"),
            (b_report ? " Y": " N"));

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-14s%-12s%-10s%-8s%-14s%-10s\n",
            "RMepId", " dUnExpPeriod", " SrcMacMis", " dLoc", " RxRDI", " ReportDefect", "1stPkt");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

        p_rmep_list = p_sys_lmep->rmep_list;
        r_count     = 0;
        CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode)
        {
            p_sys_rmep = _ctc_container_of(ctc_slistnode, sys_oam_rmep_t, head);
            sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
            mep_info.mep_index =  p_sys_rmep->rmep_index;
            _sys_usw_oam_get_mep_info(lchip, &mep_info);
            _sys_usw_oam_get_defect_report(lchip, p_sys_rmep->rmep_index, &b_report);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u%-14s%-12s%-10s%-8s%-14s%-10s\n",
                mep_info.rmep.y1731_rmep.rmep_id,
                (mep_info.rmep.y1731_rmep.d_unexp_period ? " Y" :" N"),
                (mep_info.rmep.y1731_rmep.ma_sa_mismatch ? " Y" :" N"),
                (mep_info.rmep.y1731_rmep.d_loc     ? " Y" :" N"),
                (mep_info.rmep.y1731_rmep.last_rdi  ? " Y" :" N"),
                (b_report ? " Y": " N"),
                (mep_info.rmep.y1731_rmep.first_pkt_rx ? " Y" :" N"));

            r_index[r_count]     = p_sys_rmep->rmep_index;
            r_key_index[r_count] = p_sys_rmep->key_index;
            r_count++;
            if(r_count >= 128)
            {
                break;
            }
        }
Print_key_0:
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Table offset \n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsMa", p_sys_lmep->ma_index);
        if (((sys_oam_chan_eth_t*)p_sys_chan)->key.com.key_exit)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamEthHashKey", ((sys_oam_chan_eth_t*)p_sys_chan)->key.com.key_index);
        }
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEthMep", p_sys_lmep->lmep_index);

        for(cnt = 0; cnt < r_count; cnt++)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamRmepHashKey", r_key_index[cnt]);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEthRmep", r_index[cnt]);
        }

        if (p_sys_lmep->p_sys_maid)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u~%u \n", "DsMaName", p_sys_lmep->p_sys_maid->maid_index, p_sys_lmep->p_sys_maid->maid_index + p_sys_lmep->p_sys_maid->maid_entry_num - 1);
        }

    }
    else if(CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == p_oam_key->mep_type)
    {
        p_sys_chan = (sys_oam_chan_com_t*)_sys_usw_tp_chan_lkup(lchip, p_oam_key);
        if (NULL == p_sys_chan)
        {
            ret = CTC_E_NOT_EXIST;
            goto RETURN;
        }
        /*3. lookup lmep and check exist*/
        p_sys_lmep = (sys_oam_lmep_t*)_sys_usw_tp_y1731_lmep_lkup(lchip, (sys_oam_chan_tp_t*)p_sys_chan);
        if (NULL == p_sys_lmep)
        {
            ret = CTC_E_NOT_EXIST;
            goto RETURN;
        }

        if (p_sys_lmep->mep_on_cpu)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%15s %s\n", "", "MEP on CPU");
            goto RETURN;
        }

        if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            goto Print_key_1;
        }

        sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
        mep_info.mep_index =  p_sys_lmep->lmep_index;
        _sys_usw_oam_get_mep_info(lchip, &mep_info);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n%s\n", "Flag    L: Section/Link OAM   W: VPWS     V: VPLS",
                                                          "        D: Dm En              M: MEP En   T: CC Tx En    S: SingleHop");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");
        _sys_usw_oam_get_mep_flag(lchip, p_oam_key, &mep_info, flag);
        sal_sprintf(str_port, "0x%x", p_oam_key->u.tp.gport_or_l3if_id);
        sal_sprintf(str_space, "%u", p_oam_key->u.tp.mpls_spaceid);
        sal_sprintf(str_label, "%u", p_oam_key->u.tp.label);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
            "Port/IFID", "Space", "Label", "NHID", "Mepid", "Interval", "Defect", "Flag");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10u%-10d%-10d%-10s%-10s\n",
            (CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM)? str_port : "-"),
            (CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM)? "-": str_space),
            (CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM)? "-": str_label),
            ((sys_oam_lmep_t*)p_sys_lmep)->nhid,
            mep_info.lmep.y1731_lmep.mep_id,
            mep_info.lmep.y1731_lmep.ccm_interval,
            ((mep_info.lmep.y1731_lmep.d_meg_lvl | mep_info.lmep.y1731_lmep.d_mismerge | mep_info.lmep.y1731_lmep.d_unexp_mep)? " Y" : " N"),
             flag);

        _sys_usw_oam_get_defect_report(lchip, p_sys_lmep->lmep_index, &b_report);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Status\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-14s%-12s%-10s%-8s%-14s\n",
            "LMepId", " dUnExpMep", " dMisMerge", " dMegLvl", " TxRDI", " ReportDefect");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u%-14s%-12s%-10s%-8s%-14s\n",
            mep_info.lmep.y1731_lmep.mep_id,
            (mep_info.lmep.y1731_lmep.d_unexp_mep ? " Y" :" N"),
            (mep_info.lmep.y1731_lmep.d_mismerge ? " Y" :" N"),
            (mep_info.lmep.y1731_lmep.d_meg_lvl ? " Y" :" N"),
            (mep_info.lmep.y1731_lmep.present_rdi ? " Y" :" N"),
            (b_report ? " Y": " N"));


        p_rmep_list = p_sys_lmep->rmep_list;
        r_count     = 0;
        CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode)
        {
            p_sys_rmep = _ctc_container_of(ctc_slistnode, sys_oam_rmep_t, head);
            sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
            mep_info.mep_index =  p_sys_rmep->rmep_index;
            _sys_usw_oam_get_mep_info(lchip, &mep_info);
            r_index[r_count]     = p_sys_rmep->rmep_index;
            r_count++;
        }

        if (p_sys_rmep)
        {
            _sys_usw_oam_get_defect_report(lchip, (p_sys_rmep->rmep_index), &b_report);

            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-14s%-12s%-10s%-8s%-14s%-10s\n",
                            "RMepId", " dUnExpPeriod", " SrcMacMis", " dLoc", " RxRDI", " ReportDefect", "1stPkt");
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u%-14s%-12s%-10s%-8s%-14s%-10s\n",
                            mep_info.rmep.y1731_rmep.rmep_id,
                            (mep_info.rmep.y1731_rmep.d_unexp_period ? " Y" : " N"),
                            " -",
                            (mep_info.rmep.y1731_rmep.d_loc     ? " Y" : " N"),
                            (mep_info.rmep.y1731_rmep.last_rdi  ? " Y" : " N"),
                            (b_report ? " Y" : " N"),
                            (mep_info.rmep.y1731_rmep.first_pkt_rx ? " Y" : " N"));
        }
Print_key_1:
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Table offset \n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsMa", p_sys_lmep->ma_index);

        if(CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamMplsSectionHashKey", ((sys_oam_chan_tp_t*)p_sys_chan)->key.com.key_index);
        }
        else
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamMplsLabelHashKey", ((sys_oam_chan_tp_t*)p_sys_chan)->key.com.key_index);
        }
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEthMep", p_sys_lmep->lmep_index);

        for(cnt = 0; cnt < r_count; cnt++)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEthRmep", r_index[cnt]);
        }

        if (p_sys_lmep->p_sys_maid)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u~%u \n", "DsMaName", p_sys_lmep->p_sys_maid->maid_index, p_sys_lmep->p_sys_maid->maid_index + p_sys_lmep->p_sys_maid->maid_entry_num - 1);
        }

    }
    else if((CTC_OAM_MEP_TYPE_IP_BFD == p_oam_key->mep_type)
            ||(CTC_OAM_MEP_TYPE_MPLS_BFD == p_oam_key->mep_type)
            ||(CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_oam_key->mep_type)
            ||(CTC_OAM_MEP_TYPE_MICRO_BFD == p_oam_key->mep_type))
    {

        p_sys_chan = (sys_oam_chan_com_t*)_sys_usw_bfd_chan_lkup(lchip, p_oam_key);
        if (NULL == p_sys_chan)
        {
            ret = CTC_E_NOT_EXIST;
            goto RETURN;
        }
        /*3. lookup lmep and check exist*/
        p_sys_lmep = (sys_oam_lmep_t*)_sys_usw_bfd_lmep_lkup(lchip, p_sys_chan);
        if (NULL == p_sys_lmep)
        {
            ret = CTC_E_NOT_EXIST;
            goto RETURN;
        }

        if (p_sys_lmep->mep_on_cpu)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%15s %s\n", "", "MEP on CPU");
            goto RETURN;
        }

        if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            goto Print_key_2;
        }

        sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
        mep_info.mep_index =  p_sys_lmep->lmep_index;
        _sys_usw_oam_get_mep_info(lchip, &mep_info);


        p_rmep_list = p_sys_lmep->rmep_list;
        r_count     = 0;
        CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode)
        {
            p_sys_rmep = _ctc_container_of(ctc_slistnode, sys_oam_rmep_t, head);
            sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
            mep_info.mep_index =  p_sys_rmep->rmep_index;
            _sys_usw_oam_get_mep_info(lchip, &mep_info);
            r_index[r_count]     = p_sys_rmep->rmep_index;
            r_count++;
        }

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n%s\n%s\n", "Flag    L: Section/Link OAM   W: VPWS     V: VPLS",
                                                          "        D: Dm En              M: MEP En   T: CC Tx En    S: SingleHop",
                                                          "        I: SBFD-Initiator     R: SBFD-Reflector  B: Binding-en        ");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------\n");
        _sys_usw_oam_get_mep_flag(lchip, p_oam_key, &mep_info, flag);

        if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR))
        {
            sal_strcat(flag, "I");
        }
        else if(CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            sal_strcat(flag, "R");
        }
        if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_BINDING_EN))
        {
            sal_strcat(flag, "B");
        }

        sal_sprintf(str_port, "0x%x", p_oam_key->u.tp.gport_or_l3if_id);
        sal_sprintf(str_space, "%s", "-");
        sal_sprintf(str_label, "%s", "-");
        if ((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_oam_key->mep_type)
                 &&(!CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM)))
        {
            sal_sprintf(str_space, "%u", p_oam_key->u.tp.mpls_spaceid);
            sal_sprintf(str_label, "%u", p_oam_key->u.tp.label);
        }
        else if (CTC_OAM_MEP_TYPE_MPLS_BFD == p_oam_key->mep_type)
        {
            sal_sprintf(str_space, "%u", p_sys_lmep->spaceid);
            sal_sprintf(str_label, "%u", p_sys_lmep->mpls_in_label);
        }

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s%-10s%-7s%-9s%-9s%-10s%-10s%-10s%-10s%-10s\n",
            "Discr", "Port/IFID","Space", "Label", "NHID", "State", "RxIntv", "TxIntv", "Flag", "CsfType");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------\n");

        is_tp_oam = (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_oam_key->mep_type);
        if (is_tp_oam)
        {
            switch (mep_info.lmep.bfd_lmep.rx_csf_type)
            {
                case 1:
                    sal_strcpy(csf_type,"FDI");
                    break;
                case 2:
                    sal_strcpy(csf_type,"RDI");
                    break;
                case 0:
                    sal_strcpy(csf_type,"-");     /*tp-bfd csf 0 is DCI*/
                    break;
                case 7:
                    sal_strcpy(csf_type,"LOS");
                    break;
                default:
                    sal_strcpy(csf_type,"-");
                    break;
            }
        }

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11u%-10s%-7s%-9s%-9u%-10s%-10u%-10u%-10s%-10s\n",
            mep_info.lmep.bfd_lmep.local_discr,
            ( is_tp_oam ? (CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM)? str_port : "-") : "-"),
            (p_sys_lmep->mpls_in_label) ? str_space : "-",
            (p_sys_lmep->mpls_in_label) ? str_label : "-",
            ((sys_oam_lmep_t*)p_sys_lmep)->nhid,
            bfd_state[mep_info.lmep.bfd_lmep.loacl_state],
            mep_info.rmep.bfd_rmep.actual_rx_interval,
            mep_info.lmep.bfd_lmep.actual_tx_interval,
             flag,
             (mep_info.lmep.bfd_lmep.csf_en)? csf_type:"-");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Status\n");
        _sys_usw_oam_get_defect_report(lchip, p_sys_lmep->lmep_index, &b_report);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s%-8s%-12s%-12s%-10s%-7s%-14s\n",
            "MyDisc", "State", "DesTxIntv", "ActTxIntv", "DetMult", "Diag", "ReportDefect");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------\n");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11u%-8s%-12u%-12u%-10u%-7u%-14s\n",
            mep_info.lmep.bfd_lmep.local_discr,
            bfd_state[mep_info.lmep.bfd_lmep.loacl_state],
            mep_info.lmep.bfd_lmep.desired_min_tx_interval,
            mep_info.lmep.bfd_lmep.actual_tx_interval,
            mep_info.lmep.bfd_lmep.local_detect_mult,
            mep_info.lmep.bfd_lmep.local_diag,
            (b_report ? " Y": " N"));

        if (p_sys_rmep)
        {
            _sys_usw_oam_get_defect_report(lchip, p_sys_rmep->rmep_index, &b_report);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s%-8s%-12s%-12s%-10s%-7s%-14s%-8s\n",
                            "YourDisc", "State", "ReqRxIntv", "ActRxIntv", "DetMult", "Diag", "ReportDefect", "1stPkt");
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------\n");

            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11u%-8s%-12u%-12u%-10u%-7u%-14s%-8s\n",
                            mep_info.rmep.bfd_rmep.remote_discr,
                            bfd_state[mep_info.rmep.bfd_rmep.remote_state],
                            mep_info.rmep.bfd_rmep.required_min_rx_interval,
                            mep_info.rmep.bfd_rmep.actual_rx_interval,
                            mep_info.rmep.bfd_rmep.remote_detect_mult,
                            mep_info.rmep.bfd_rmep.remote_diag,
                            (b_report ? " Y" : " N"),
                            (mep_info.rmep.bfd_rmep.first_pkt_rx ? " Y" : " N"));
        }
Print_key_2:
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Table offset \n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsMa", p_sys_lmep->ma_index);

        if(CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_oam_key->mep_type)
        {
            if(CTC_FLAG_ISSET(p_oam_key->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamMplsSectionHashKey", ((sys_oam_chan_tp_t*)p_sys_chan)->key.com.key_index);
            }
            else
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamMplsLabelHashKey", ((sys_oam_chan_tp_t*)p_sys_chan)->key.com.key_index);
            }
        }
        else
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsEgressXcOamBfdHashKey", ((sys_oam_chan_bfd_t*)p_sys_chan)->key.com.key_index);
        }

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsBfdMep", p_sys_lmep->lmep_index);

        for(cnt = 0; cnt < r_count; cnt++)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsBfdRmep", r_index[cnt]);
        }

        if (p_sys_lmep->p_sys_maid)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u~%u \n", "DsMaName", p_sys_lmep->p_sys_maid->maid_index, p_sys_lmep->p_sys_maid->maid_index + p_sys_lmep->p_sys_maid->maid_entry_num - 1);
        }

    }

RETURN:

    return ret ;
}


STATIC int32
_sys_usw_show_oam_info_by_cfm(uint8 lchip, sys_oam_chan_com_t*  p_sys_chan,
                                                    sys_oam_lmep_t*  p_sys_lmep,
                                                    sys_oam_rmep_t*  p_sys_rmep,
                                                    sys_oam_debug_type_t* p_oam_debug)
{
    uint8 mep_type = 0;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    ctc_oam_mep_info_t mep_info;
    char flag[64] = {0};
    ctc_oam_key_t oam_key;


    mep_type = p_sys_chan->mep_type;
    if((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
        ||(CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
    {
        p_sys_chan_eth = (sys_oam_chan_eth_t*)p_sys_chan;

        /*Get mep info*/
        sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
        mep_info.mep_index =  p_sys_lmep->lmep_index;
        if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");
            return CTC_E_INVALID_CHIP_ID;
        }
        _sys_usw_oam_get_mep_info(lchip, &mep_info);

        /*build key*/
        sal_memset(&oam_key, 0, sizeof(ctc_oam_key_t));
        oam_key.mep_type = mep_type;
        if(p_sys_chan_eth->key.link_oam)
        {
            CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }
        if(p_sys_chan_eth->key.l2vpn_oam_id)
        {
            CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_L2VPN);
            if(p_sys_chan_eth->key.is_vpws)
            {

            }
            else
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_VPLS);
            }
        }

        _sys_usw_oam_get_mep_flag(lchip, &oam_key, &mep_info, flag);

        /*dump*/
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x%-10u%-10u%-10u%-10d%-10s%-10d%-10s%-10s\n",
            p_sys_chan_eth->key.gport,
            p_sys_chan_eth->key.vlan_id,
            p_sys_chan_eth->key.cvlan_id,
            p_sys_chan_eth->key.l2vpn_oam_id,
            ((sys_oam_lmep_t*)p_sys_lmep)->md_level,
            ((mep_info.lmep.y1731_lmep.is_up_mep) ? "Y" : "N"),
            mep_info.lmep.y1731_lmep.ccm_interval,
            ((mep_info.lmep.y1731_lmep.d_meg_lvl | mep_info.lmep.y1731_lmep.d_mismerge | mep_info.lmep.y1731_lmep.d_unexp_mep)? " Y" : " N"),
             flag);

        p_oam_debug->session_count ++;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_show_oam_info_by_y1731(uint8 lchip, sys_oam_chan_com_t*  p_sys_chan,
                                                    sys_oam_lmep_t*  p_sys_lmep,
                                                    sys_oam_rmep_t*  p_sys_rmep,
                                                    sys_oam_debug_type_t* p_oam_debug)
{
    uint8 mep_type = 0;
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    ctc_oam_mep_info_t mep_info;
    char str_port[64]   = {0};
    char str_space[64]  = {0};
    char str_label[64]  = {0};
    char flag[64]       = {0};
    ctc_oam_key_t oam_key;

    mep_type = p_sys_chan->mep_type;
    if(CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type)
    {
        p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_chan;

        /*Get mep info*/
        sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
        mep_info.mep_index =  p_sys_lmep->lmep_index;
        if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");
            return CTC_E_INVALID_CHIP_ID;
        }
        _sys_usw_oam_get_mep_info(lchip, &mep_info);

        /*build key*/
        sal_memset(&oam_key, 0, sizeof(ctc_oam_key_t));
        oam_key.mep_type = mep_type;
        if(p_sys_chan_tp->key.section_oam)
        {
            CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        }
        _sys_usw_oam_get_mep_flag(lchip, &oam_key, &mep_info, flag);

        /*dump*/
        sal_sprintf(str_port, "0x%x", p_sys_chan_tp->key.gport_l3if_id);
        sal_sprintf(str_space, "%u", p_sys_chan_tp->key.spaceid);
        sal_sprintf(str_label, "%u", p_sys_chan_tp->key.label);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10u%-10d%-10d%-10s%-10s\n",
            (p_sys_chan_tp->key.section_oam ? str_port : "-"),
            (p_sys_chan_tp->key.section_oam ? "-": str_space),
            (p_sys_chan_tp->key.section_oam ? "-": str_label),
            ((sys_oam_lmep_t*)p_sys_lmep)->nhid,
            mep_info.lmep.y1731_lmep.mep_id,
            mep_info.lmep.y1731_lmep.ccm_interval,
            ((mep_info.lmep.y1731_lmep.d_meg_lvl | mep_info.lmep.y1731_lmep.d_mismerge | mep_info.lmep.y1731_lmep.d_unexp_mep)? " Y" : " N"),
             flag);

        p_oam_debug->session_count ++;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_show_oam_info_by_bfd(uint8 lchip, sys_oam_chan_com_t*  p_sys_chan,
                                                    sys_oam_lmep_t*  p_sys_lmep,
                                                    sys_oam_rmep_t*  p_sys_rmep,
                                                    sys_oam_debug_type_t* p_oam_debug)
{
    uint8 mep_type = 0;
    sys_oam_chan_tp_t *p_sys_chan_tp    = NULL;
    ctc_oam_mep_info_t mep_info;
    uint8 is_tp_oam = 0;

    char str_port[64]   = {0};
    char str_space[64]  = {0};
    char str_label[64]  = {0};
    char flag[64]       = {0};
    char *bfd_state[4] = {"Admin Down","Down", "Init", "Up"};
    ctc_oam_key_t oam_key;

    mep_type = p_sys_chan->mep_type;
    if((CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        if(CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        {
            p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_chan;

            sal_sprintf(str_port, "0x%x", p_sys_chan_tp->key.gport_l3if_id);
            sal_sprintf(str_space, "%u", p_sys_chan_tp->key.spaceid);
            sal_sprintf(str_label, "%u", p_sys_chan_tp->key.label);
            is_tp_oam = 1;
        }
        else
        {
        }

        /*Get mep info*/
        sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));
        if (p_sys_rmep)
        {
            mep_info.mep_index =  p_sys_rmep->rmep_index;
        }
        else
        {
            mep_info.mep_index =  p_sys_lmep->lmep_index;
        }
        if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");
            return CTC_E_INVALID_CHIP_ID;

        }
        _sys_usw_oam_get_mep_info(lchip, &mep_info);

        /*build key*/
        sal_memset(&oam_key, 0, sizeof(ctc_oam_key_t));
        oam_key.mep_type = mep_type;
        if(is_tp_oam)
        {
            if(p_sys_chan_tp->key.section_oam)
            {
                CTC_SET_FLAG(oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
            }
        }
        _sys_usw_oam_get_mep_flag(lchip, &oam_key, &mep_info, flag);

        if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR))
        {
            sal_strcat(flag, "I");
        }
        else if(CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            sal_strcat(flag, "R");
        }

        /*dump*/
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11u%-10s%-7s%-9s%-9u%-10s%-10u%-10u%-10s\n",
            mep_info.lmep.bfd_lmep.local_discr,
            (is_tp_oam ? (p_sys_chan_tp->key.section_oam ? str_port : "-") : "-"),
            (is_tp_oam ? (p_sys_chan_tp->key.section_oam ? "-": str_space) : "-"),
            (is_tp_oam ? (p_sys_chan_tp->key.section_oam ? "-": str_label) : "-"),
            ((sys_oam_lmep_t*)p_sys_lmep)->nhid,
            bfd_state[mep_info.lmep.bfd_lmep.loacl_state],
            mep_info.lmep.bfd_lmep.actual_tx_interval,
            p_sys_rmep?mep_info.rmep.bfd_rmep.actual_rx_interval:0,
             flag);

        p_oam_debug->session_count ++;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_show_oam_info(sys_oam_chan_com_t*  p_sys_chan, void* p_data)
{
    uint8 oam_type = 0;
    ctc_slistnode_t* ctc_slistnode_l = NULL;
    ctc_slistnode_t* ctc_slistnode_r = NULL;
    ctc_slist_t* p_lmep_list = NULL;
    ctc_slist_t* p_rmep_list = NULL;

    sys_oam_lmep_t* p_sys_lmep_com  = NULL;
    sys_oam_rmep_t* p_sys_rmep_com  = NULL;
    sys_oam_debug_type_t* p_oam_debug   = NULL;

    SYS_OAM_DEBUG_SHOW_FN_t show_oam_fn[] =
    {
        _sys_usw_show_oam_info_by_cfm,
        _sys_usw_show_oam_info_by_y1731,
        _sys_usw_show_oam_info_by_bfd
    };

    p_oam_debug = p_data;
    oam_type = p_oam_debug->oam_type;

    p_lmep_list = p_sys_chan->lmep_list;

    CTC_SLIST_LOOP(p_lmep_list, ctc_slistnode_l)
    {
        p_sys_lmep_com = _ctc_container_of(ctc_slistnode_l, sys_oam_lmep_t, head);

        p_rmep_list = p_sys_lmep_com->rmep_list;
        CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode_r)
        {
            p_sys_rmep_com = _ctc_container_of(ctc_slistnode_r, sys_oam_rmep_t, head);

        }
        show_oam_fn[oam_type - 1](p_oam_debug->lchip, p_sys_chan, p_sys_lmep_com, p_sys_rmep_com, p_oam_debug);
    }

    return CTC_E_NONE;
}

int32
sys_usw_oam_show_oam_mep(uint8 lchip, uint8 oam_type)
{
    sys_oam_debug_type_t oam_eth;
    sys_oam_debug_type_t oam_tp_y1731;
    sys_oam_debug_type_t oam_bfd;

    sal_memset(&oam_eth,        0 ,sizeof(sys_oam_debug_type_t));
    sal_memset(&oam_tp_y1731,   0 ,sizeof(sys_oam_debug_type_t));
    sal_memset(&oam_bfd,        0 ,sizeof(sys_oam_debug_type_t));

    SYS_OAM_INIT_CHECK(lchip);
    oam_eth.lchip = lchip;
    oam_tp_y1731.lchip = lchip;
    oam_bfd.lchip = lchip;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n%s\n%s\n", "Flag    L: Section/Link OAM   W: VPWS     V: VPLS",
                                                      "        D: Dm En              M: MEP En   T: CC Tx En    S: SingleHop",
                                                      "        I: SBFD-Initiator     R: SBFD-Reflector                      ");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");
    if((SYS_OAM_DBG_ALL == oam_type) || (SYS_OAM_DBG_ETH_OAM == oam_type))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "Ethernet");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
            "Port", "Vlan", "Cvlan", "VpnId", "MdLvl", "Up", "Interval", "Defect", "Flag");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

        oam_eth.oam_type = SYS_OAM_DBG_ETH_OAM;
        ctc_hash_traverse_through(g_oam_master[lchip]->chan_hash,
                        (hash_traversal_fn)_sys_usw_show_oam_info,
                        &oam_eth);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }

    if((SYS_OAM_DBG_ALL == oam_type) || (SYS_OAM_DBG_TP_Y1731_OAM == oam_type))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "TP Y1731");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
            "Port/IFID", "Space", "Label", "NHID", "Mepid", "Interval", "Defect", "Flag");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

        oam_tp_y1731.oam_type = SYS_OAM_DBG_TP_Y1731_OAM;
        ctc_hash_traverse_through(g_oam_master[lchip]->chan_hash,
                            (hash_traversal_fn)_sys_usw_show_oam_info,
                            &oam_tp_y1731);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }

    if((SYS_OAM_DBG_ALL == oam_type) || (SYS_OAM_DBG_BFD == oam_type))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "BFD");

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-7s%-9s%-9s%-10s%-10s%-10s%-10s\n",
            "Discr", "Port/IFID", "Space", "Label", "NHID", "State", "TxIntv", "RxIntv", "Flag");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");

        oam_bfd.oam_type = SYS_OAM_DBG_BFD;
        ctc_hash_traverse_through(g_oam_master[lchip]->chan_hash,
                            (hash_traversal_fn)_sys_usw_show_oam_info,
                            &oam_bfd);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nTotal OAM Session number \n");
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------\n");
    if((SYS_OAM_DBG_ALL == oam_type) || (SYS_OAM_DBG_ETH_OAM == oam_type))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "Ethernet", oam_eth.session_count);
    }
    if((SYS_OAM_DBG_ALL == oam_type) || (SYS_OAM_DBG_TP_Y1731_OAM == oam_type))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "TP Y1731", oam_tp_y1731.session_count);
    }
    if((SYS_OAM_DBG_ALL == oam_type) || (SYS_OAM_DBG_BFD == oam_type))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "BFD", oam_bfd.session_count);
    }

    return CTC_E_NONE;
}



#endif

