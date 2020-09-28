/**
 @file sys_usw_nexthop_debug.c

 @date 2009-12-28

 @version v2.0

 The file contains all nexthop module core logic
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_linklist.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_l3if.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_aps.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_ftm.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
* Mater branch
*****************************************************************************/

#define SYS_NH_DEBUG_DUMP_HW_TABLE(tbl_str, FMT, ...)              \
    do                                                       \
    {                                                        \
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    "tbl_str "::");                                  \
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__);                      \
    } while (0)

#define SYS_NH_DEBUG_TYPE_HEAD(type) " ~~~~~ DUMP "type " Nexthop ~~~~~~\n"

#define SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE 41
#define SYS_NH_IP_TUNNEL_IPV4_PROTO_TYPE 4
#define SYS_NH_DEBUG_NH_MERGE_MODE       "N/A"

#define NHID_CONVERT_STR(id, str)  \
  ((id>=SYS_NH_INTERNAL_NHID_BASE)?(sal_sprintf(str, "%u%s", (id-SYS_NH_INTERNAL_NHID_BASE), "(I)")):  \
  (sal_sprintf(str, "%u", (id))))

extern void ctc_list_delete_list(struct ctc_linklist* list);
extern sys_usw_nh_api_master_t* p_usw_nh_api_master[CTC_MAX_LOCAL_CHIP_NUM];

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

#if 0
int32
sys_usw_nh_display_current_global_sram_info(uint8 lchip)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32* p_bmp;
    uint8  line_num = 16;
    uint8  line_loop = 0;
    int32 index, pos;
    bool have_used = FALSE;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (NULL == p_nh_master)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Nexthop Module have not been inited \n");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Systme max global nexthop offset is %u,global met offset is %d\n",
                       p_nh_master->max_glb_nh_offset, p_nh_master->max_glb_met_offset);
        p_bmp = p_nh_master->p_occupid_nh_offset_bmp;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Occupied global nexthop Offsets are: \n");
        line_loop = 0;

        for (index = 0; index < (p_nh_master->max_glb_nh_offset >> BITS_SHIFT_OF_WORD); index++)
        {
            for (pos = 0; pos < BITS_NUM_OF_WORD; pos++)
            {
                if (CTC_FLAG_ISSET(p_bmp[index], 1 << pos))
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d, ", (index << BITS_SHIFT_OF_WORD) | (pos));
                    have_used = TRUE;
                    line_loop++;
                }

                if (line_loop == line_num)
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
                    line_loop = 0;
                }
            }
        }

        if (!have_used)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " No offset in use");
        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

        p_bmp = p_nh_master->p_occupid_met_offset_bmp;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Occupied global met Offsets are: \n ");
        line_loop = 0;

        for (index = 0; index < (p_nh_master->max_glb_met_offset >> BITS_SHIFT_OF_WORD); index++)
        {
            for (pos = 0; pos < BITS_NUM_OF_WORD; pos++)
            {
                if (CTC_FLAG_ISSET(p_bmp[index], 1 << pos))
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d, ", (index << BITS_SHIFT_OF_WORD) | (pos));
                    have_used = TRUE;
                    line_loop++;
                }

                if (line_loop == line_num)
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
                    line_loop = 0;
                }
            }
        }
    }

    if (!have_used)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " No offset in use");
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}
#endif
int32
sys_usw_nh_dump_xvlan(uint8 lchip, uint32 nhid, uint8 detail)
{
    sys_nh_info_com_t* p_nhinfo = NULL;
    int32 ret;
    uint32 cmd;
    uint8 is_dsnh8w = 0;
    DsNextHop4W_m   dsnh4w;
    DsNextHop8W_m   dsnh8w;
    uint32 index = 0;
    uint8 svlan_tag_dis = 0;
    uint8 tag_mode = 0;
    uint8 svlan_valid = 0;
    uint8 svlan_tagged = 0;
    uint8 payload_operation = 0;
    uint16 output_svlan = 0;
    uint8 cvlan_tag_dis = 0;
    uint8 cvlan_valid = 0;
    uint16 output_cvlan = 0;
    uint8 cvlan_share = 0;
    uint8 loop = 0;
    uint8 loop_cnt = 0;
    uint32 dsnh_offset = 0;
    uint8 share_type = 0;

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo));
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (!detail)
    {
        return CTC_E_NONE;
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Vlan Edit \n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");

    dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;

    loop = 0;
    loop_cnt = 1;
    if (  CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
        && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
    {
        loop_cnt = 2;
    }

    while ((loop++) < loop_cnt)
    {
    if (  CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
        && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
        {
            if (loop == 1)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " ---------------------- :%s\n","W [W/P]");
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " ---------------------- :%s\n","P [W/P]");
                dsnh_offset = is_dsnh8w ? (dsnh_offset + 2) : (dsnh_offset + 1);
            }

        }

        is_dsnh8w = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);

        if ( (dsnh_offset >> SYS_NH_DSNH_INTERNAL_SHIFT) == SYS_NH_DSNH_INTERNAL_BASE)
        {
            cmd = DRV_IOR(EpeNextHopInternal_t, DRV_ENTRY_FLAG);
            index = (dsnh_offset&((1 << SYS_NH_DSNH_INTERNAL_SHIFT) - 1))/2;
        }
        else
        {
            cmd = DRV_IOR(DsNextHop8W_t, DRV_ENTRY_FLAG);
            index = (dsnh_offset % 2)?(dsnh_offset - 1):dsnh_offset;
        }

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsnh8w));

        if (!is_dsnh8w && (dsnh_offset % 2))
        {
            sal_memmove((uint8 *)&dsnh8w, (uint8 *)&dsnh8w + sizeof(dsnh4w), sizeof(dsnh4w));
        }


        share_type = GetDsNextHop8W(V, shareType_f,  &dsnh8w);
        svlan_tagged = GetDsNextHop8W(V, svlanTagged_f,  &dsnh8w);
        payload_operation = GetDsNextHop8W(V, payloadOperation_f,  &dsnh8w);

        if (is_dsnh8w || share_type == SYS_NH_SHARE_TYPE_VLANTAG)
        {
            svlan_tag_dis = GetDsNextHop8W(V, svlanTagDisable_f,  &dsnh8w);
            tag_mode      = GetDsNextHop8W(V, taggedMode_f,  &dsnh8w);
            svlan_valid   = GetDsNextHop8W(V, outputSvlanIdValid_f,  &dsnh8w);
            output_svlan  = GetDsNextHop8W(V, outputSvlanId_f,  &dsnh8w);

            cvlan_tag_dis = GetDsNextHop8W(V, cvlanTagDisable_f,  &dsnh8w);
            cvlan_share =  GetDsNextHop8W(V, ctagShareMode_f,  &dsnh8w);
            if (is_dsnh8w || (!cvlan_share))
            {
                cvlan_valid = GetDsNextHop8W(V, outputCvlanIdValid_f,  &dsnh8w);
                output_cvlan = GetDsNextHop8W(V, outputCvlanId_f,  &dsnh8w);
            }
        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :", "Svlan operation");
        if (!payload_operation)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "none");
        }
        else
        {
            if (svlan_tag_dis && cvlan_tag_dis && tag_mode)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "swap");
            }
            else if (svlan_tag_dis)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "none");
            }
            else if(!svlan_tagged)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "strip");
            }
            else if(tag_mode)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "add");
            }
            else if(svlan_valid)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "replace");
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "none");
            }
        }
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%d\n", "Svlan id", output_svlan);


        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :", "Cvlan operation");
        if (!payload_operation)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "none");
        }
        else
        {
            if (svlan_tag_dis && cvlan_tag_dis && tag_mode)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "swap");
            }
            else if (cvlan_tag_dis)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "none");
            }
            else if(tag_mode)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "add");
            }
            else if(cvlan_valid)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "replace");
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "none");
            }
        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%d\n", "Cvlan id", output_cvlan);
    }



    return CTC_E_NONE;

}

int32
sys_usw_nh_dump_iloop(uint8 lchip, uint32 nhid, uint8 detail)
{
    sys_nh_info_brguc_t* p_brguc_info;
    sys_nh_info_com_t* p_nhinfo = NULL;

    int32 ret;
    DsFwd_m dsfwd;
    uint32 cmd;
    uint32 nhptr = 0;
    uint32 destmap = 0;
    uint16 lport = 0;

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo));
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_ILOOP != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't BridgeUC nexthop for nexthopId %d\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_brguc_info = (sys_nh_info_brguc_t*)(p_nhinfo);

    cmd = DRV_IOR(DsFwd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_brguc_info->hdr.dsfwd_info.dsfwd_offset / 2, cmd, &dsfwd));

    nhptr = GetDsFwd(V, nextHopPtr_f, &dsfwd);
    destmap = GetDsFwd(V, destMap_f, &dsfwd);

    lport = ((destmap&0x100)|(((nhptr>>16)&0x1)<<7)|(nhptr&0xFF));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", nhid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "I-LOOP");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-4x\n", SYS_MAP_DRV_LPORT_TO_CTC_GPORT(0, lport));

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_rspan(uint8 lchip, uint32 nhid, uint8 detail)
{
    sys_nh_info_rspan_t* p_rspan_info;
    sys_nh_info_com_t* p_nhinfo = NULL;

    int32 ret;
    uint32 cmd;

    DsNextHop4W_m   dsnh;
    DsNextHop8W_m   dsnh8w;
    uint32 dsnh_offset = 0;


    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_RSPAN != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't RSPAN nexthop for nexthopId %d\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_rspan_info = (sys_nh_info_rspan_t*)(p_nhinfo);

    dsnh_offset = p_rspan_info->hdr.dsfwd_info.dsfwd_offset;

    if (((dsnh_offset >> 4) & 0x3FFF) != SYS_NH_DSNH_INTERNAL_BASE)
    {
        cmd = DRV_IOR(DsNextHop4W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset, cmd, &dsnh));
    }
    else
    {
        cmd = DRV_IOR(EpeNextHopInternal_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (dsnh_offset & 0x2) >> 1, cmd, &dsnh8w));
        sal_memcpy((uint8*)&dsnh, ((uint8*)&dsnh8w + sizeof(DsNextHop4W_m) * (dsnh_offset & 0x1)), sizeof(DsNextHop4W_m));
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", nhid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "RSPAN");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%u\n", GetDsNextHop4W(V, destVlanPtr_f, &dsnh));

    return CTC_E_NONE;
}
char *
_sys_usw_nh_dump_get_l2rewrite_desc(uint8 lchip, uint8 rewrite_type)
{
    switch(rewrite_type)
    {
    case SYS_NH_L2EDIT_TYPE_LOOPBACK:
        return "DsL2EditLoopback";
    case SYS_NH_L2EDIT_TYPE_ETH_4W:
        return "DsL2EditEth3W";
    case SYS_NH_L2EDIT_TYPE_ETH_8W:
        return "DsL2EditEth6W";
    case SYS_NH_L2EDIT_TYPE_MAC_SWAP:
        return "DsL2EditSwap";
    case SYS_NH_L2EDIT_TYPE_FLEX_8W:
        return "DsL2EditFlex";
    case SYS_NH_L2EDIT_TYPE_PBB_4W:
        return "DsL2EditPbb4W";
    case SYS_NH_L2EDIT_TYPE_PBB_8W:
        return "DsL2EditPbb8W";
    case SYS_NH_L2EDIT_TYPE_OF:
        return "DsL2EditOf";
    case SYS_NH_L2EDIT_TYPE_INNER_SWAP:
        return "DsL2EditInnerSwap";
    case SYS_NH_L2EDIT_TYPE_INNNER_DS_LITE:
        return "DsL2EditDsLite";
    case SYS_NH_L2EDIT_TYPE_INNNER_DS_LITE_8W:
        return "DsL2EditDsLite6w";
    default :
        return "Error";
    }
}


char *
_sys_usw_nh_dump_get_l3rewrite_desc(uint8 lchip, uint8 rewrite_type)
{

    switch(rewrite_type)
    {
    case SYS_NH_L3EDIT_TYPE_MPLS_4W:
        return "DsL3EditMpls3W";
    case SYS_NH_L3EDIT_TYPE_NAT_4W:
        return "DsL3EditNat3W";
    case SYS_NH_L3EDIT_TYPE_NAT_8W:
        return "DsL3EditNat6W";
    case SYS_NH_L3EDIT_TYPE_TUNNEL_V4:
        return "DsL3EditTunnelV4";
    case SYS_NH_L3EDIT_TYPE_TUNNEL_V6:
        return "DsL3EditTunnelV6";
    case SYS_NH_L3EDIT_TYPE_L3FLEX:
        return "DsL3EditFlex";
    case SYS_NH_L3EDIT_TYPE_OF8W:
        return "DsL3EditOf6W";
    case SYS_NH_L3EDIT_TYPE_OF16W:
        return "DsL3EditOf12W";
    case SYS_NH_L3EDIT_TYPE_LOOPBACK:
        return "DsL3EditLoopback";
    case SYS_NH_L3EDIT_TYPE_TRILL:
        return "DsL3EditTrill";
    case SYS_NH_L3EDIT_TYPE_MPLS_12W:
        return "DsL3EditMpls12W";
	default :
		return "Error";
    }

}

int32
sys_usw_nh_dump_table_offset(uint8 lchip, uint32 nhid, uint8 bind_type)
{
    sys_nh_info_com_t* p_nhinfo = NULL;
    int ret = 0;
    uint32 dsnh_offset = 0;
    uint8 is_dsnh8w = 0;
    uint8 is_rsv_dsnh = 0;
    uint8 loop = 0;
    uint8 loop_cnt = 0;
    uint8 loop2 = 0;
    uint8 loop2_cnt = 0;
    uint32 cmd ;
    DsNextHop4W_m   dsnh4w;
    DsNextHop8W_m   dsnh8w;
    uint32 outer_edit_ptr = 0;
    uint32 inner_edit_ptr = 0;
    uint32 inner_edit_ptr_type = 0 ;
    uint32 outer_edit_ptr_type = 0 ;
    uint32 outer_edit_location = 0 ;
    uint8 share_type = 0;
    DsL3Edit3W_m   l3edit;
    DsL2Edit3W_m  l2edit;
    uint8 l2_rewrite_type = SYS_NH_L2EDIT_TYPE_NONE;
    uint8 l3_rewrite_type;
    uint8 dsnh_aps_en = 0;
    uint16 aps_bridge_id = 0;
    uint32 next_ptr_valid = 0;
    uint32 spme_en = 0;
    uint32 tmp_outer_edit_ptr_type = 0;
    uint32 tmp_outer_edit_location = 0;
    sys_nh_info_mcast_t *p_mcast_db = NULL;
    uint32 payload_op = 0;
    DsL2Edit6WOuter_m  l2edit_out;

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (ret != CTC_E_NONE)
    {
        return ret;
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Table offset \n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");

    if (SYS_NH_TYPE_ECMP == p_nhinfo->hdr.nh_entry_type)
    {
        sys_nh_info_ecmp_t *p_ecmp_db = (sys_nh_info_ecmp_t *)p_nhinfo;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u \n", "DsEcmpGroup", p_ecmp_db->ecmp_group_id);
        if (CTC_NH_ECMP_TYPE_XERSPAN == p_ecmp_db->type)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u \n", "DsNexthop8w", p_ecmp_db->hdr.dsfwd_info.dsnh_offset);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        }

        return CTC_E_NONE;
    }

    if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
        && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%s\n", "DsFwd", SYS_NH_DEBUG_NH_MERGE_MODE);
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u [3w]\n", "DsFwd", p_nhinfo->hdr.dsfwd_info.dsfwd_offset);
    }

    /*Get dsfwd cb information*/
    if ((bind_type == SYS_NH_TYPE_IPUC) ||  (bind_type == SYS_NH_TYPE_MPLS) || \
        (bind_type == SYS_NH_TYPE_IP_TUNNEL) ||  (bind_type == SYS_NH_TYPE_TRILL) || (bind_type == SYS_NH_TYPE_MISC))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  --%-19s :%s\n", "Bind", "Yes");
    }

    loop = 0;
    loop_cnt = 1;
    if (  CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
        && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
    {
        loop_cnt = 2;
    }

    dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
    dsnh_offset &= 0x1FFFF;
    is_dsnh8w = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
    if ( (dsnh_offset >> SYS_NH_DSNH_INTERNAL_SHIFT) == SYS_NH_DSNH_INTERNAL_BASE)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u \n", "EpeNextHopInternal", (dsnh_offset & 0xF)/2);
        is_rsv_dsnh = 1;
        loop_cnt = 0;
    }

    if (SYS_NH_TYPE_MCAST == p_nhinfo->hdr.nh_entry_type)
    {
        loop_cnt = 0;
        p_mcast_db = (sys_nh_info_mcast_t *)p_nhinfo;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u [3w]\n", "DsMetEntry3w", p_mcast_db->basic_met_offset);
    }
    if (SYS_NH_TYPE_ILOOP == p_nhinfo->hdr.nh_entry_type||
        SYS_NH_TYPE_ELOOP == p_nhinfo->hdr.nh_entry_type||
        SYS_NH_TYPE_DROP == p_nhinfo->hdr.nh_entry_type||
        SYS_NH_TYPE_TOCPU == p_nhinfo->hdr.nh_entry_type)
    {
        return CTC_E_NONE;
    }


    while ((loop++) < loop_cnt)
    {
    if (  CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
        && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
        {
            if (loop == 1)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n ---------------------- :%s\n","W [W/P]");
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n ---------------------- :%s\n","P [W/P]");
                dsnh_offset = is_dsnh8w ? (dsnh_offset + 2) : (dsnh_offset + 1);
            }

        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u [%s]\n", (is_dsnh8w ? "DsNexthop8w" : "DsNexthop4w"), dsnh_offset, (is_dsnh8w ? "8w" : "4w"));

        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
        {
            continue;
        }

        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE))
        {
            continue;
        }


        if (is_dsnh8w && !is_rsv_dsnh )
        {
            cmd = DRV_IOR(DsNextHop8W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset, cmd, &dsnh8w));

            share_type = GetDsNextHop8W(V, shareType_f,  &dsnh8w);
            outer_edit_location =  GetDsNextHop8W(V, outerEditLocation_f, &dsnh8w);
            outer_edit_ptr_type = GetDsNextHop8W(V, outerEditPtrType_f, &dsnh8w);
            outer_edit_ptr =  GetDsNextHop8W(V, outerEditPtr_f, &dsnh8w);
            inner_edit_ptr_type = GetDsNextHop8W(V, innerEditPtrType_f, &dsnh8w);
            inner_edit_ptr =  GetDsNextHop8W(V, innerEditPtr_f, &dsnh8w);
            dsnh_aps_en = GetDsNextHop8W(V, apsBridgeEn_f, &dsnh8w);
        }
        else if(!is_dsnh8w && !is_rsv_dsnh)
        {
            cmd = DRV_IOR(DsNextHop4W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset, cmd, &dsnh4w));
            dsnh_aps_en = GetDsNextHop8W(V, apsBridgeEn_f, &dsnh4w);
            share_type = GetDsNextHop4W(V, shareType_f,  &dsnh4w);

            payload_op = GetDsNextHop4W(V, payloadOperation_f,  &dsnh4w);
            if (SYS_NH_OP_ROUTE_COMPACT == payload_op)
            {

            }
            else if (share_type == SYS_NH_SHARE_TYPE_L23EDIT)
            {
                outer_edit_location =  GetDsNextHop4W(V, outerEditLocation_f, &dsnh4w);
                outer_edit_ptr_type = GetDsNextHop4W(V, outerEditPtrType_f, &dsnh4w);
                outer_edit_ptr =  GetDsNextHop4W(V, outerEditPtr_f, &dsnh4w);
                inner_edit_ptr_type = GetDsNextHop4W(V, innerEditPtrType_f,  &dsnh4w);
                inner_edit_ptr =  GetDsNextHop4W(V, innerEditPtr_f, &dsnh4w);
            }
            else if(share_type == SYS_NH_SHARE_TYPE_L2EDIT_VLAN)
            {
                outer_edit_location =  GetDsNextHop4W(V, u1_g4_outerEditLocation_f, &dsnh4w);
                outer_edit_ptr_type = GetDsNextHop4W(V, u1_g4_outerEditPtrType_f, &dsnh4w);
                outer_edit_ptr =  GetDsNextHop4W(V, u1_g4_outerEditPtr_f, &dsnh4w);
                inner_edit_ptr = 0;
            }
        }

        /*
        inner_edit:ptr_type
        0- L2Edit
        1- L3Edit

        outer_edit: location / type
        0 / 1   L3Edit (Pipe0) -- > ( L3Edit (Pipe1), opt)-- > L2Edit(Pipe2)
        0 / 0   L2Edit (Pipe0) -- > L3Edit(Pipe1)
        1 / 0   L2Edit (Pipe2)
        1 / 1   L3Edit (Pipe1) -- > L2Edit(Pipe2)
        */
        if (inner_edit_ptr != 0)
        {
            if (inner_edit_ptr_type)
            {
                cmd = DRV_IOR(DsL3EditNat3W_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, inner_edit_ptr, cmd, &l3edit));
                l3_rewrite_type =  GetDsL3EditNat3W(V, l3RewriteType_f, &l3edit);
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u\n", _sys_usw_nh_dump_get_l3rewrite_desc(lchip, l3_rewrite_type), inner_edit_ptr );
            }
            else
            {
                cmd = DRV_IOR(DsL2EditEth3W_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, inner_edit_ptr, cmd, &l2edit));

                l2_rewrite_type = GetDsL2EditEth3W(V, l2RewriteType_f, &l2edit);
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u\n", _sys_usw_nh_dump_get_l2rewrite_desc(lchip, l2_rewrite_type), inner_edit_ptr );
            }
        }

        loop2 = 0;
        loop2_cnt = dsnh_aps_en?2:1;
        aps_bridge_id = outer_edit_ptr;
        tmp_outer_edit_ptr_type = outer_edit_ptr_type;
        tmp_outer_edit_location = outer_edit_location;

        while ((loop2++) < loop2_cnt)
        {
            if (dsnh_aps_en)
            {
                if (loop2 == 1)
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n -W [W/P]\n");
                    cmd = DRV_IOR(DsApsBridge_t, DsApsBridge_workingOuterEditPtr_f);
                }
                else
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n -P [W/P]\n");
                    cmd = DRV_IOR(DsApsBridge_t, DsApsBridge_protectingOuterEditPtr_f);
                }
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, aps_bridge_id, cmd, &outer_edit_ptr));

                cmd = DRV_IOR(DsApsBridge_t, DsApsBridge_spmeApsEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, aps_bridge_id, cmd, &spme_en));

            }

            outer_edit_ptr_type = tmp_outer_edit_ptr_type;
            outer_edit_location = tmp_outer_edit_location;

            if (outer_edit_ptr != 0)
            {
                if (1 == outer_edit_ptr_type && 0 == outer_edit_location)
                {   /*L3 Edit*/
                    cmd = DRV_IOR(DsL3EditNat3W_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, outer_edit_ptr, cmd, &l3edit));
                    l3_rewrite_type =  GetDsL3EditNat3W(V, l3RewriteType_f, &l3edit);
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u\n", _sys_usw_nh_dump_get_l3rewrite_desc(lchip, l3_rewrite_type), outer_edit_ptr );


                    outer_edit_ptr_type =  GetDsL3EditNat3W(V, outerEditPtrType_f, &l3edit);
                    outer_edit_ptr =  GetDsL3EditNat3W(V, outerEditPtr_f, &l3edit);
                    next_ptr_valid = GetDsL3EditNat3W(V, nextEditPtrValid_f, &l3edit);
                }
                else if(0 == outer_edit_ptr_type && 0 == outer_edit_location)
                {   /*L2 Edit*/
                    cmd = DRV_IOR(DsL2EditEth3W_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, outer_edit_ptr, cmd, &l2edit));
                    l2_rewrite_type = GetDsL2EditEth3W(V, l2RewriteType_f, &l2edit);
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u\n", _sys_usw_nh_dump_get_l2rewrite_desc(lchip, l2_rewrite_type), outer_edit_ptr );
                }
                else
                {
                    next_ptr_valid = 1;
                }

                /*SPME*/
                if (next_ptr_valid && 1 == outer_edit_ptr_type)
                {
                    DsL3Edit6W3rd_m l3edit6w;

                    outer_edit_ptr = spme_en?(outer_edit_ptr + 1):outer_edit_ptr;
                    cmd = DRV_IOR(DsL3Edit6W3rd_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, outer_edit_ptr/2, cmd, &l3edit6w));
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u\n", "DsL3Edit6W3rd", outer_edit_ptr/2);

                    if (outer_edit_ptr % 2 == 0)
                    {
                        sal_memcpy(&l3edit, (uint8*)&l3edit6w,  sizeof(l3edit));
                    }
                    else
                    {
                        sal_memcpy( &l3edit, (uint8*)&l3edit6w + sizeof(l3edit), sizeof(l3edit));
                    }

                    outer_edit_ptr_type =  GetDsL3Edit3W3rd(V, outerEditPtrType_f, &l3edit);
                    outer_edit_ptr =  GetDsL3Edit3W3rd(V, outerEditPtr_f, &l3edit);
                    next_ptr_valid = GetDsL3Edit3W3rd(V, nextEditPtrValid_f, &l3edit);
                }

                /*OUT Edit*/
                if (next_ptr_valid && 0 == outer_edit_ptr_type)
                {
                    /*L2 Edit outer*/
                    cmd = DRV_IOR(DsL2Edit6WOuter_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, outer_edit_ptr/2, cmd, &l2edit_out));

                    l2_rewrite_type = GetDsL2Edit6WOuter(V, l2RewriteType_f, &l2edit_out);

                    if ((l2_rewrite_type == SYS_NH_L2EDIT_TYPE_ETH_8W) || (l2_rewrite_type == SYS_NH_L2EDIT_TYPE_FLEX_8W) ||
                       (l2_rewrite_type == SYS_NH_L2EDIT_TYPE_MAC_SWAP))
                    {
                        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u [%s]\n", "DsL2Edit6WOuter", outer_edit_ptr/2,  "6w");
                    }
                    else
                    {
                        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-20s :%u [%s]\n", "DsL2Edit3WOuter", outer_edit_ptr,  "3w");
                    }
                }


            }
        }
    }


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_brguc(uint8 lchip, uint32 nhid, uint8 detail)
{
    sys_nh_info_brguc_t* p_brguc_info = NULL;
    sys_nh_info_com_t* p_nhinfo = NULL;
	ctc_aps_bridge_group_t  aps_group ;
    int32 ret;

     char* sub_type[] = {"None", "Basic", "Bypass", "VLAN edit", "APS VLAN edit"};
     char nh_str[50] = {0};

     ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_BRGUC != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't BridgeUC nexthop for nexthopId %d\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_brguc_info = (sys_nh_info_brguc_t*)(p_nhinfo);

    CTC_MAX_VALUE_CHECK(p_brguc_info->nh_sub_type, (SYS_NH_PARAM_BRGUC_SUB_TYPE_MAX - 1));
    NHID_CONVERT_STR(nhid, nh_str);

    if (CTC_FLAG_ISSET(p_brguc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
    {
        sys_usw_aps_get_ports(lchip, p_brguc_info->dest_gport, &aps_group);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-17s%-16s 0x%.4x%6s %s\n",nh_str,
                "Bridge Unicast", sub_type[p_brguc_info->nh_sub_type], aps_group.working_gport, "", aps_group.protect_en?"N":"Y");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-17s%-16s 0x%.4x%6s %s\n", "", "", "",
                       aps_group.protection_gport, "", aps_group.protect_en?"Y":"N");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-17s%-16s 0x%.4x%6s %s\n", nh_str,
            "Bridge Unicast", sub_type[p_brguc_info->nh_sub_type], p_brguc_info->dest_gport, "", "Y");
	}

    if (p_brguc_info->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT ||
        p_brguc_info->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT)
    {
        sys_usw_nh_dump_xvlan(lchip, nhid, detail);
    }

    return CTC_E_NONE;
}

STATIC int32
sys_usw_nh_dump_ipuc_brief(uint8 lchip, uint32 nhid, sys_nh_info_ipuc_t* p_ipuc_info, uint32* p_l2edit_ptr)
{
    sys_l3if_prop_t l3if_prop;
    int32 ret = CTC_E_NONE;
    ctc_aps_bridge_group_t aps_group;
    sys_nh_db_arp_t *p_arp = NULL;
    char nh_str[50] = {0};

    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));

    NHID_CONVERT_STR(nhid, nh_str);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", nh_str);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "IP Unicast");

    if (p_ipuc_info->arp_id)
    {
       p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_ipuc_info->arp_id));
       if (NULL == p_arp)
       {
           return CTC_E_NOT_EXIST;
       }
    }

    if (CTC_FLAG_ISSET(p_ipuc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s%-9s%-10s%-16s%-8s\n", "-", "-", "-", "-", "-");
        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_ipuc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
    {
        sys_usw_aps_get_ports(lchip, p_ipuc_info->gport, &aps_group);
    }
    else
    {
        aps_group.working_gport = p_ipuc_info->gport;
        aps_group.protect_en = 0;
    }

    if (CTC_IS_CPU_PORT(p_ipuc_info->gport) || (p_arp && CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU)))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s          ", "CPU");
    }
    else if (p_arp && CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s          ", "Drop");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%04X       ", aps_group.working_gport);
    }

    /* working path */
    l3if_prop.l3if_id = p_ipuc_info->l3ifid;
    ret = sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);
    if (CTC_E_NONE != ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s",  "-");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9u", l3if_prop.vlan_id);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u", l3if_prop.l3if_id);
    }

    if(p_ipuc_info->arp_id)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16d", p_ipuc_info->arp_id);
    }
    else
    {
        if(p_ipuc_info->p_dsl2edit)
        {
            if (p_ipuc_info->l2edit_8w)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s", sys_output_mac(((sys_nh_db_dsl2editeth8w_t*)(p_ipuc_info->p_dsl2edit))->mac_da));
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s", sys_output_mac(p_ipuc_info->p_dsl2edit->mac_da));
            }
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s", sys_output_mac(p_ipuc_info->mac_da));
        }
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s\n", aps_group.protect_en?"N":"Y");

    /*Protection path*/
    if (CTC_FLAG_ISSET(p_ipuc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%04X       ", aps_group.protection_gport);

        l3if_prop.l3if_id = p_ipuc_info->protection_path->l3ifid;
        ret = sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);

        if (CTC_E_NONE != ret)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9u", l3if_prop.vlan_id);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u", l3if_prop.l3if_id);
        }

        if (p_ipuc_info->protection_path->arp_id)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16d", p_ipuc_info->protection_path->arp_id);
        }
        else
        {
            if(p_ipuc_info->protection_path->p_dsl2edit)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s", sys_output_mac(p_ipuc_info->protection_path->p_dsl2edit->mac_da));
        }
        else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s", sys_output_mac(p_ipuc_info->protection_path->mac_da));
            }
        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s\n", aps_group.protect_en?"Y":"N");
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_ipuc(uint8 lchip, uint32 nhid, uint8 detail, uint8* p_bind_type)
{
    sys_nh_info_ipuc_t* p_ipuc_info = NULL;
    sys_nh_info_com_t* p_nh_com_info = NULL;
    sys_nh_info_dsnh_t nh_dsnh_info;
    sys_usw_nh_master_t* p_nh_master = NULL;
    int32 ret = CTC_E_NONE;
    uint32 l2_edit_ptr = 0;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    ret = sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nh_com_info);

    sal_memset(&nh_dsnh_info, 0, sizeof(sys_nh_info_dsnh_t));
    ret = (ret < 0) ? ret : _sys_usw_nh_get_nhinfo(lchip, nhid, &nh_dsnh_info);

    if (ret  !=CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_IPUC != p_nh_com_info->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't IPUC nexthop for nexthopId %d\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_ipuc_info = (sys_nh_info_ipuc_t*)(p_nh_com_info);
    if (p_ipuc_info->updateAd)
    {
        *p_bind_type = SYS_NH_TYPE_IPUC;
    }

    CTC_ERROR_RETURN(sys_usw_nh_dump_ipuc_brief(lchip, nhid, p_ipuc_info, &l2_edit_ptr));

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_ecmp(uint8 lchip, uint32 nhid, uint8 detail)
{
    sys_nh_info_ecmp_t* p_ecmp_info = NULL;
    sys_nh_info_com_t* p_nhinfo = NULL;
    int32 ret;
    uint8 loop = 0;

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (ret  !=CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_ECMP != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't ECMP nexthop for nexthopId %d\n", nhid);
        return CTC_E_INVALID_PARAM;
    }


    p_ecmp_info = (sys_nh_info_ecmp_t*)(p_nhinfo);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8dECMP\n", nhid);

    if (detail)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n%-6s%s\n","No.","MEMBER_NHID");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------\n");

        for (loop = 0; loop < p_ecmp_info->ecmp_cnt; loop++)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d%u\n", loop, p_ecmp_info->nh_array[loop]);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_ip_mcast_l3if_info(uint8 lchip, uint32 gport, uint16 vlan_id)
{
    int32 ret = CTC_E_NONE;
    sys_l3if_prop_t l3if_prop;

    sal_memset(&l3if_prop,0,sizeof(sys_l3if_prop_t));
    ret = sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, gport, vlan_id, &l3if_prop);
    if (CTC_E_NONE == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9u%-8u%s\n", l3if_prop.vlan_id, l3if_prop.l3if_id, "-");
        return CTC_E_NONE;
    }

    return CTC_E_NONE;
}

void
sys_usw_nh_dump_mcast_member_info(uint8 lchip, sys_nh_mcast_meminfo_t* p_meminfo_com, uint32* p_memcnt, uint32 gport,uint8 aps_en)
{
    uint16  loop = 0;

    if ((SYS_NH_PARAM_BRGMC_MEM_LOCAL == p_meminfo_com->dsmet.member_type)
       || (SYS_NH_PARAM_BRGMC_MEM_RAPS == p_meminfo_com->dsmet.member_type)
       || (SYS_NH_PARAM_MCAST_MEM_MIRROR_WITH_NH == p_meminfo_com->dsmet.member_type))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", *p_memcnt);

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s0x%.4x%-4s%-9s%-8s%s\n", "L2 Mcast",gport,"", "-", "-", "-");
    }

    else if(SYS_NH_PARAM_BRGMC_MEM_LOCAL_WITH_APS_BRIDGE == p_meminfo_com->dsmet.member_type)
    {

        gport = p_meminfo_com->dsmet.ucastid;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", *p_memcnt);

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s0x%.4x%-4s%-9s%-8s%u\n", "L2 Mcast", gport, "", "APS", "-", p_meminfo_com->dsmet.ref_nhid);
    }
    else if (SYS_NH_PARAM_MCAST_MEM_LOCAL_LOGIC_REP == p_meminfo_com->dsmet.member_type)
    {
        for (loop = 0; loop <= p_meminfo_com->dsmet.replicate_num ; loop ++)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d", (*p_memcnt)++);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-14s", "L2 Mcast");
            if (p_meminfo_com->dsmet.replicate_num == 0)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s%-9s%-8s%u\n", "-", "-", "-", p_meminfo_com->dsmet.ref_nhid);
            }
            else
            {
                if (CTC_FLAG_ISSET(p_meminfo_com->dsmet.flag, SYS_NH_DSMET_FLAG_USE_DSNH8W))
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s%-9s%-8s%d\n", "-", "-", "-", p_meminfo_com->dsmet.vid_list[loop*2]);
                }
                else
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s%-9s%-8s%d\n", "-", "-", "-", p_meminfo_com->dsmet.vid_list[loop]);
                }
            }
        }
    }
    else if (SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP == p_meminfo_com->dsmet.member_type)
    {
        for (loop = 0; loop < (p_meminfo_com->dsmet.replicate_num + 1); loop++)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", (*p_memcnt)++);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s0x%.4x%-4s", "IP Mcast", gport, "");
            if (p_meminfo_com->dsmet.vid_list)
            {
                sys_usw_nh_dump_ip_mcast_l3if_info(lchip, gport, p_meminfo_com->dsmet.vid_list[loop]);
            }
            else
            {
                sys_usw_nh_dump_ip_mcast_l3if_info(lchip, gport, p_meminfo_com->dsmet.vid);
            }
        }
        /*This parameter has plused extra a number in the final loop,so it should be subtracted one number*/
        (*p_memcnt)--;
    }
    else if (SYS_NH_PARAM_IPMC_MEM_LOCAL == p_meminfo_com->dsmet.member_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", *p_memcnt);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s0x%.4x%-4s", "IP Mcast", gport, "");
        sys_usw_nh_dump_ip_mcast_l3if_info(lchip, gport, p_meminfo_com->dsmet.vid);
    }
    else if (SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH_DESTID == p_meminfo_com->dsmet.member_type
        ||SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH == p_meminfo_com->dsmet.member_type)
    {
        uint32 cmd = 0;
        uint32 dest_map = 0;
        uint8  arp_cpu = 0;

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", *p_memcnt);
        if (CTC_FLAG_ISSET(p_meminfo_com->dsmet.flag, SYS_NH_DSMET_FLAG_DESTMAP_PROFILE))
        {
            cmd = DRV_IOR(DsDestMapProfileMc_t, DsDestMapProfileMc_destMap_f);
            DRV_IOCTL(lchip, (p_meminfo_com->dsmet.ucastid&0x1ff), cmd, &dest_map);
            gport = dest_map&0x1fff;
            arp_cpu = (dest_map == 0xffff)?1:0;
        }
        if (SYS_DESTMAP_IS_CPU(p_meminfo_com->dsmet.ucastid) || (sys_usw_chip_is_eth_cpu_port(lchip, p_meminfo_com->dsmet.ucastid)) || arp_cpu)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s%s   %-4s%-9s%-8s", "L2 Mcast", "CPU", "", "-", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s0x%.4x%-4s%-9s%-8s", "L2 Mcast", gport, "", "-", "-");
        }
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%u\n", p_meminfo_com->dsmet.ref_nhid);
    }
}

void
sys_usw_nh_dump_mcast_bitmap_info(uint8 lchip, sys_nh_mcast_meminfo_t* p_meminfo_com, uint8 gchip, uint32* p_memcnt)
{
    uint8 loop = 0;
    uint16 gport = 0;

    for (loop = 0; ((loop < 32) && p_meminfo_com->dsmet.pbm[0]); loop++)
    {
        if (CTC_IS_BIT_SET(p_meminfo_com->dsmet.pbm[0], loop))
        {
            if (CTC_FLAG_ISSET(p_meminfo_com->dsmet.flag, SYS_NH_DSMET_FLAG_IS_LINKAGG))
            {
                gport = CTC_MAP_TID_TO_GPORT(loop);
            }
            else
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, (loop + p_meminfo_com->dsmet.port_type*SYS_NH_MCAST_MEMBER_BITMAP_SIZE));
            }
            (*p_memcnt)++;
            sys_usw_nh_dump_mcast_member_info(lchip, p_meminfo_com, p_memcnt, gport,0);
        }
    }

    for (loop = 0; ((loop < 32) && p_meminfo_com->dsmet.pbm[1]); loop++)
    {
        if (CTC_IS_BIT_SET(p_meminfo_com->dsmet.pbm[1], loop))
        {
            if (CTC_FLAG_ISSET(p_meminfo_com->dsmet.flag, SYS_NH_DSMET_FLAG_IS_LINKAGG))
            {
                gport = CTC_MAP_TID_TO_GPORT(loop + 32);
            }
            else
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, (loop + 32 + p_meminfo_com->dsmet.port_type*SYS_NH_MCAST_MEMBER_BITMAP_SIZE));
            }
            (*p_memcnt)++;
            sys_usw_nh_dump_mcast_member_info(lchip, p_meminfo_com, p_memcnt, gport,0);
        }
    }

    for (loop = 0; ((loop < 32) && p_meminfo_com->dsmet.pbm[2]); loop++)
    {
        if (CTC_IS_BIT_SET(p_meminfo_com->dsmet.pbm[2], loop))
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, (loop + 256 + p_meminfo_com->dsmet.port_type*SYS_NH_MCAST_MEMBER_BITMAP_SIZE));
            (*p_memcnt)++;
            sys_usw_nh_dump_mcast_member_info(lchip, p_meminfo_com, p_memcnt, gport,0);
        }
    }

    for (loop = 0; ((loop < 32) && p_meminfo_com->dsmet.pbm[3]); loop++)
    {
        if (CTC_IS_BIT_SET(p_meminfo_com->dsmet.pbm[3], loop))
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, (loop + 32 + 256 + p_meminfo_com->dsmet.port_type*SYS_NH_MCAST_MEMBER_BITMAP_SIZE));
            (*p_memcnt)++;
            sys_usw_nh_dump_mcast_member_info(lchip, p_meminfo_com, p_memcnt, gport,0);
        }
    }
}

int32
sys_usw_nh_dump_mcast_brief(uint8 lchip, uint32 nhid, sys_nh_info_mcast_t*  p_mcast_info, bool detail)
{
    sys_nh_mcast_meminfo_t* p_meminfo_com;
    ctc_list_pointer_node_t* p_pos;
    uint32 memcnt = 0;
    uint8 gchip = 0;
    uint16 gport = 0;
    uint8 have_mcast_profile = 0;
    char  nh_str[30] = {0};

    NHID_CONVERT_STR(nhid, nh_str);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", nh_str);

    if (CTC_FLAG_ISSET(p_mcast_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MCAST_PROFILE))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "Mcast-Profile");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "Mcast");
    }

    if (CTC_FLAG_ISSET(p_mcast_info->mcast_flag, SYS_NH_INFO_MCAST_FLAG_STK_MEM_PROFILE) ||
        CTC_FLAG_ISSET(p_mcast_info->mcast_flag, SYS_NH_INFO_MCAST_FLAG_MEM_PROFILE))
    {
        have_mcast_profile = 1;
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15u", p_mcast_info->basic_met_offset);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s\n",
        (CTC_FLAG_ISSET(p_mcast_info->mcast_flag, SYS_NH_INFO_MCAST_FLAG_LOGIC_REP_EN)?"Enable":"Disable"));

if (detail)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nMulticast Member:\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-13s%-10s%-9s%-8s%s\n",
                                         "No.", "MEMBER_TYPE", "DESTPORT", "VLAN", "L3IFID", "MEMBER_NHID");

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    CTC_LIST_POINTER_LOOP(p_pos, &(p_mcast_info->p_mem_list))
    {
        p_meminfo_com = _ctc_container_of(p_pos, sys_nh_mcast_meminfo_t, list_head);

           if (!p_meminfo_com->dsmet.logic_port
                && CTC_FLAG_ISSET(p_meminfo_com->dsmet.flag, SYS_NH_DSMET_FLAG_USE_PBM))
            {
                sys_usw_nh_dump_mcast_bitmap_info(lchip, p_meminfo_com, gchip, &memcnt);
            }
            else
            {
                if (SYS_NH_MET_DROP_UCAST_ID == p_meminfo_com->dsmet.ucastid)
                {
                    continue;
                }
                memcnt++;
                if (CTC_FLAG_ISSET(p_meminfo_com->dsmet.flag, SYS_NH_DSMET_FLAG_IS_LINKAGG))
                {
                    gport = CTC_MAP_TID_TO_GPORT(p_meminfo_com->dsmet.ucastid);
                }
                else
                {
                    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, (p_meminfo_com->dsmet.ucastid & 0x1FF));
                }
                sys_usw_nh_dump_mcast_member_info(lchip, p_meminfo_com, &memcnt, gport, 0);
            }
      }

    if (have_mcast_profile)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", (memcnt+1));
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s%-10u%-9s%-8s%s\n", "Profile",p_mcast_info->profile_nh_id, "-", "-", "-");
    }
}
    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_mcast(uint8 lchip, uint32 nhid, bool detail)
{
    sys_nh_info_mcast_t* p_mcast_info = NULL;

    sys_nh_info_com_t* p_nhinfo = NULL;
    int32 ret;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (ret  !=CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_MCAST != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't BridgeMC nexthop for nexthopId %u\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_mcast_info = (sys_nh_info_mcast_t*)(p_nhinfo);

    CTC_ERROR_RETURN(sys_usw_nh_dump_mcast_brief(lchip, nhid, p_mcast_info, detail));
    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_mpls(uint8 lchip, uint32 nhid, uint8 detail, uint8* p_bind_type)
{
    sys_nh_info_mpls_t* p_mpls_info;
    sys_nh_info_com_t* p_nhinfo = NULL;
    int32  ret;
    uint32 cmd = 0;
    uint32 vc_label = 0;
    uint16 tunnel_id = 0;
    char   vc_label_str[10] = {0};
    char   aps_group_str[10] = {0};
    DsL3EditMpls3W_m  dsl3edit;
    sys_l3if_prop_t l3if_prop;
    ctc_aps_bridge_group_t aps_group;
    sys_nh_db_arp_t* p_arp =NULL;
    char nh_str[50] = {0};

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (ret  !=CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_MPLS != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't MPLS nexthop for nexthopId %u\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_mpls_info = (sys_nh_info_mpls_t*)(p_nhinfo);

    if (p_mpls_info->updateAd)
    {
        *p_bind_type = SYS_NH_TYPE_MPLS;
    }

    if (p_mpls_info->arp_id)
    {
       p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_mpls_info->arp_id));
       if (NULL == p_arp)
       {
           return CTC_E_NOT_EXIST;
       }
    }

    NHID_CONVERT_STR(nhid, nh_str);

    sal_sprintf(aps_group_str, "%s", "-");
    sal_sprintf(vc_label_str, "%s", "-");
    sys_usw_aps_get_ports(lchip, p_mpls_info->aps_group_id, &aps_group);


    /*MPLS POP Nexthop*/
    if (CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH))
    {

        if (CTC_IS_CPU_PORT(p_mpls_info->gport) || (p_arp && CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU)))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-15s%4s%7s", nh_str, "MPLS-POP", "CPU", "");
        }
        else if (p_arp && CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-15s%4s%7s", nh_str, "MPLS-POP", "Drop", "");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-15s0x%.4x%7s", nh_str, "MPLS-POP", p_mpls_info->gport, "");
        }

        l3if_prop.l3if_id = p_mpls_info->l3ifid;
        ret = sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);
        if (CTC_E_NONE != ret)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s%-10s", "-", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9u%-10u", l3if_prop.vlan_id, l3if_prop.l3if_id);
        }

        if(p_mpls_info->arp_id)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d\n", p_mpls_info->arp_id);
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", sys_output_mac(p_mpls_info->p_dsl2edit->mac_da));
        }


         return CTC_E_NONE;
     }

    /*MPLS PUSH Nexthop*/
    if (CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
    {
        tunnel_id = p_mpls_info->working_path.p_mpls_tunnel?p_mpls_info->working_path.p_mpls_tunnel->tunnel_id:0;
        cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_mpls_info->working_path.dsl3edit_offset, cmd, &dsl3edit));
        vc_label = GetDsL3EditMpls3W(V, label_f, &dsl3edit);
        sal_sprintf(vc_label_str, "%u", vc_label);
        if (CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
            && !CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
        {
            sal_sprintf(aps_group_str, "%u", p_mpls_info->aps_group_id);
        }
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s%-9s%-14u%-13s%-14s%s\n",
                       nh_str, "MPLS", tunnel_id, vc_label_str, aps_group_str, aps_group.protect_en ? "N" : "Y");

        if (CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
            && !CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
        {
            if (p_mpls_info->protection_path)
            {
                cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_mpls_info->protection_path->dsl3edit_offset, cmd, &dsl3edit));

                vc_label = GetDsL3EditMpls3W(V, label_f, &dsl3edit);
                sal_sprintf(vc_label_str, "%u", vc_label);
                if (NULL != p_mpls_info->protection_path->p_mpls_tunnel)
                {
                    tunnel_id = p_mpls_info->protection_path->p_mpls_tunnel->tunnel_id;
                }

                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s%-9s%-14u%-13s%-14s%s\n",
                               "", "", tunnel_id, vc_label_str, aps_group_str, aps_group.protect_en ? "Y" : "N");

            }
        }
    }
    else
    {
        if (NULL != p_mpls_info->working_path.p_mpls_tunnel)
        {
            tunnel_id = p_mpls_info->working_path.p_mpls_tunnel->tunnel_id;
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s%-9s%-14u%-13s%-14s%s\n",
                           nh_str, "MPLS", tunnel_id, vc_label_str, "-", "-");
        }
    }

    sys_usw_nh_dump_xvlan(lchip, nhid, detail);


    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_get_tunnel_info(uint8 lchip, sys_nh_info_ip_tunnel_t* p_nhinfo, char* tunnel_type,
                                       char* ipsa, char * ipda, char * gre_key)
{

    uint32 l3edit_offset = p_nhinfo->dsl3edit_offset;
    DsL3EditTunnelV4_m  dsl3edit_v4;
    DsL3EditTunnelV6_m  dsl3edit_v6;
    DsL2EditEth6W_m   dsl2edit_6w;
    DsL3EditNat3W_m  dsl3edit_nat;
    DsL3EditNat6W_m  dsl3edit_6w;
    uint32 cmd = 0;
    uint8 flag = p_nhinfo->flag;
    uint32 tempip = 0;
    DsL3TunnelV4IpSa_m v4_sa;
    DsL3TunnelV6IpSa_m v6_sa;
    uint32 gre_info = 0;
    ipv6_addr_t hw_ipv6_address;
    ipv6_addr_t sw_ipv6_address;
    uint8 ip_protocol_type = 0;
    uint8 tunnel_6to4_da = 0;
    uint8 tunnel_6to4_sa = 0;
    uint8 inner_header_valid = 0;
    uint8 inner_header_type = 0;
    uint8 ipsa_index = 0;
    uint8 isatp_tunnel = 0;
    uint8 tunnel_6to4_ipv6_prefix_length = 0;
    uint8 tunnel_6to4_ipv4_prefix_length = 0;
    uint8 tunnel_6to4_sa_ipv6_prefix_length = 0;
    uint8 tunnel_6to4_sa_ipv4_prefix_length = 0;
    uint8 nvgre_header_mode = 0;
    uint8 udp_tunnel_type = 0;
    uint8 is_nvgre_encaps = 0;
    uint8 is_geneve_encaps = 0;
    uint32 ip_da = 0;
    uint32 ip_sa = 0;
    uint32 tunnel_ip_da = 0;
    uint32 tunnel_ip_sa = 0;
    uint8  ipsa_valid = 0;
    char   str[35];
    char   format[10];

    sal_memset(&dsl2edit_6w, 0, sizeof(DsL2EditEth6W_m));
    sal_memcpy(tunnel_type, "-", sal_strlen("-"));
    sal_memcpy(ipsa, "-", sal_strlen("-"));
    sal_memcpy(ipda, "-", sal_strlen("-"));
    sal_memcpy(gre_key, "-", sal_strlen("-"));

    flag = flag & (SYS_NH_IP_TUNNEL_FLAG_IN_V4 | SYS_NH_IP_TUNNEL_FLAG_IN_V6
           | SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4 | SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6
           | SYS_NH_IP_TUNNEL_FLAG_NAT_V4);

    switch (flag)
    {
        case SYS_NH_IP_TUNNEL_FLAG_IN_V4:
        {
            cmd = DRV_IOR(DsL3EditTunnelV4_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3edit_offset, cmd, &dsl3edit_v4));

            ip_protocol_type = GetDsL3EditTunnelV4(V, ipProtocolType_f, &dsl3edit_v4);
            tunnel_6to4_da = GetDsL3EditTunnelV4(V, tunnel6To4Da_f, &dsl3edit_v4);
            gre_info = GetDsL3EditTunnelV4(V, greKeyLow_f, &dsl3edit_v4);
            gre_info |= GetDsL3EditTunnelV4(V, greKeyHigh_f, &dsl3edit_v4) << 16;
            inner_header_valid = GetDsL3EditTunnelV4(V, innerHeaderValid_f, &dsl3edit_v4);
            inner_header_type = GetDsL3EditTunnelV4(V, innerHeaderType_f, &dsl3edit_v4);
            ipsa_index = GetDsL3EditTunnelV4(V, ipSaIndex_f, &dsl3edit_v4);
             /*TDDO isatp_tunnel = GetDsL3EditTunnelV4(V, isatpTunnel_f, &dsl3edit_v4);*/
            tunnel_6to4_sa = GetDsL3EditTunnelV4(V, tunnel6To4Sa_f, &dsl3edit_v4);
            tunnel_6to4_ipv6_prefix_length = GetDsL3EditTunnelV4(V, tunnel6To4Ipv6PrefixLength_f, &dsl3edit_v4);
            tunnel_6to4_ipv4_prefix_length = GetDsL3EditTunnelV4(V, tunnel6To4Ipv4PrefixLength_f, &dsl3edit_v4);
            tunnel_6to4_sa_ipv6_prefix_length = GetDsL3EditTunnelV4(V, tunnel6To4SaIpv6PrefixLength_f, &dsl3edit_v4);
            tunnel_6to4_sa_ipv4_prefix_length = GetDsL3EditTunnelV4(V, tunnel6To4SaIpv4PrefixLength_f, &dsl3edit_v4);
            nvgre_header_mode = GetDsL3EditTunnelV4(V, nvgreHeaderMode_f, &dsl3edit_v4);
            udp_tunnel_type = GetDsL3EditTunnelV4(V, udpTunnelType_f, &dsl3edit_v4);
            is_nvgre_encaps = GetDsL3EditTunnelV4(V, isNvgreEncaps_f, &dsl3edit_v4);
            is_geneve_encaps = GetDsL3EditTunnelV4(V, isGeneveTunnel_f, &dsl3edit_v4);
            tunnel_ip_da = GetDsL3EditTunnelV4(V, ipDa_f, &dsl3edit_v4);
            tunnel_ip_sa = GetDsL3EditTunnelV4(V, ipSaLow_f, &dsl3edit_v4) & 0xFFFF;
            tunnel_ip_sa |= GetDsL3EditTunnelV4(V, ipSaHigh_f, &dsl3edit_v4) << 16;
            ipsa_valid = GetDsL3EditTunnelV4(V, ipSaValid_f, &dsl3edit_v4);

            if ((17 == ip_protocol_type) || (SYS_NH_IP_TUNNEL_IPV4_PROTO_TYPE == ip_protocol_type))
            {
                ip_da = tunnel_ip_da;
                ip_sa = tunnel_ip_sa;
                /* CTC_TUNNEL_TYPE_IPV4_IN4 */
                /* RFC 2003/2893: IPv4-in-IPv4 tunnel. */
                sal_memcpy(tunnel_type, "IPv4-in-IPv4", sal_strlen("IPv4-in-IPv4"));
            }
            else if (SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE == ip_protocol_type)
            {
                if (1 == isatp_tunnel)
                {
                    ip_da = 0;
                    ip_sa = tunnel_ip_sa;
                    /* CTC_TUNNEL_TYPE_ISATAP */
                    sal_memcpy(tunnel_type, "ISATAP", sal_strlen("ISATAP"));
                }
                else if (((0 != tunnel_6to4_ipv6_prefix_length) || (0 != tunnel_6to4_ipv4_prefix_length)
                         || (0 != tunnel_6to4_sa_ipv6_prefix_length) || (0 != tunnel_6to4_sa_ipv4_prefix_length))
                         || ((1 == tunnel_6to4_sa) && (1 == tunnel_6to4_da) && (0 != tunnel_ip_da)))
                {
                    if (tunnel_6to4_sa)
                    {
                        ip_sa = GetDsL3EditTunnelV4(V, ipSa6To4Low_f, &dsl3edit_v4) & 0xFFFF;
                        ip_sa |= GetDsL3EditTunnelV4(V, ipSa6To4High_f, &dsl3edit_v4) << 16;
                    }
                    else
                    {
                        ip_sa = tunnel_ip_sa;
                    }

                    ip_da = GetDsL3EditTunnelV4(V, ipDa_f, &dsl3edit_v4);

                    /* CTC_TUNNEL_TYPE_6RD */
                    sal_memcpy(tunnel_type, "6RD", sal_strlen("6RD"));
                }
                else if (1 == tunnel_6to4_da)
                {
                    ip_da = 0;
                    if (0 == tunnel_6to4_sa)
                    {
                        ip_sa = tunnel_ip_sa;
                    }
                    else
                    {
                        ip_sa = 0;
                    }
                    /* CTC_TUNNEL_TYPE_6TO4 */
                    sal_memcpy(tunnel_type, "6TO4", sal_strlen("6TO4"));
                }
                else
                {
                    ip_da = tunnel_ip_da;
                    ip_sa = tunnel_ip_sa;
                    /* CTC_TUNNEL_TYPE_IPV6_IN4 */
                    /* RFC 2003/2893: IPv6-in-IPv4 tunnel. */
                    sal_memcpy(tunnel_type, "IPv6-in-IPv4", sal_strlen("IPv6-in-IPv4"));
                }
            }
            else if (1 == inner_header_valid)
            {
                if ((1 == inner_header_type) && (1 == is_nvgre_encaps) && (1 == nvgre_header_mode))
                {
                    ip_da = tunnel_ip_da;
                    ip_sa = tunnel_ip_sa;
                    /* CTC_TUNNEL_TYPE_NVGRE_IN4 */
                    sal_memcpy(tunnel_type, "NVGRE-in-IPv4", sal_strlen("NVGRE-in-IPv4"));
                }
                else if ((1 == inner_header_type) && (0 == is_nvgre_encaps) && (0 == ipsa_valid))
                {
                    /* CTC_TUNNEL_TYPE_GRE_IN4 */
                    /* RFC 1701/2784/2890: GRE-in-IPv4  tunnel. */
                    sal_memcpy(tunnel_type, "GRE-in-IPv4", sal_strlen("GRE-in-IPv4"));
                    sal_sprintf(gre_key, "%s", CTC_DEBUG_HEX_FORMAT(str, format, gre_info, 8, U));
                    ip_da = tunnel_ip_da;
                    cmd = DRV_IOR(DsL3TunnelV4IpSa_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipsa_index, cmd, &v4_sa));
                    ip_sa = GetDsL3TunnelV4IpSa(V, ipSa_f, &v4_sa);
                }
                else if ((2 == inner_header_type) && (1 == udp_tunnel_type) && (1 == ipsa_valid))
                {
                    ip_da = tunnel_ip_da;
                    ip_sa = tunnel_ip_sa;
                    if (is_geneve_encaps)
                    {
                        /* CTC_TUNNEL_TYPE_GENEVE_IN4 */
                        sal_memcpy(tunnel_type, "GENEVE-in-IPv4", sal_strlen("GENEVE-in-IPv4"));
                    }
                    else
                    {
                        /* CTC_TUNNEL_TYPE_VXLAN_IN4 */
                        sal_memcpy(tunnel_type, "VXLAN-in-IPv4", sal_strlen("VXLAN-in-IPv4"));
                    }
                }
                else if (0 == inner_header_type)
                {
                    /* CTC_TUNNEL_TYPE_GRE_IN4 */
                    /* RFC 1701/2784/2890: GRE-in-IPv4  tunnel. */
                    sal_memcpy(tunnel_type, "GRE-in-IPv4", sal_strlen("GRE-in-IPv4"));
                    ip_da = tunnel_ip_da;
                    cmd = DRV_IOR(DsL3TunnelV4IpSa_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipsa_index, cmd, &v4_sa));
                    ip_sa = GetDsL3TunnelV4IpSa(V, ipSa_f, &v4_sa);
                }
            }
            else if (1 == isatp_tunnel)
            {
                /* CTC_TUNNEL_TYPE_ISATAP */
                /* RFC 5214/5579 ,IPv6-in-IPv4 tunnel,ipv4 da copy ipv6 da(last 32bit) */
                sal_memcpy(tunnel_type, "ISATAP", sal_strlen("ISATAP"));
            }
            else
            {
                sal_memcpy(tunnel_type, "-", sal_strlen("-"));
            }

            tempip = sal_ntohl(ip_sa);
            sal_inet_ntop(AF_INET, &tempip, ipsa, CTC_IPV6_ADDR_STR_LEN);

            tempip = sal_ntohl(ip_da);
            sal_inet_ntop(AF_INET, &tempip, ipda, CTC_IPV6_ADDR_STR_LEN);
            break;
        }

        case SYS_NH_IP_TUNNEL_FLAG_IN_V6:
        {
            cmd = DRV_IOR(DsL3EditTunnelV6_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3edit_offset, cmd, &dsl3edit_v6));

            ip_protocol_type = GetDsL3EditTunnelV6(V, ipProtocolType_f, &dsl3edit_v6);
            inner_header_valid = GetDsL3EditTunnelV6(V, innerHeaderValid_f, &dsl3edit_v6);
            inner_header_type = GetDsL3EditTunnelV6(V, innerHeaderType_f, &dsl3edit_v6);
            ipsa_index = GetDsL3EditTunnelV6(V, ipSaIndex_f, &dsl3edit_v6);
            nvgre_header_mode = GetDsL3EditTunnelV6(V, nvgreHeaderMode_f, &dsl3edit_v6);
            is_nvgre_encaps = GetDsL3EditTunnelV6(V, isNvgreEncaps_f, &dsl3edit_v6);
            is_geneve_encaps =  GetDsL3EditTunnelV6(V, isGeneveTunnel_f, &dsl3edit_v6);

            if ((17 == ip_protocol_type) || (SYS_NH_IP_TUNNEL_IPV4_PROTO_TYPE == ip_protocol_type))
            {
                /* CTC_TUNNEL_TYPE_IPV4_IN6 */
                /* RFC 2003/2893: IPv4-in-IPv6 tunnel. */
                sal_memcpy(tunnel_type, "IPv4-in-IPv6", sal_strlen("IPv4-in-IPv6"));
            }
            else if (SYS_NH_IP_TUNNEL_IPV6_PROTO_TYPE == ip_protocol_type)
            {
                /* CTC_TUNNEL_TYPE_IPV6_IN6 */
                /* RFC 2003/2893: IPv6-in-IPv6 tunnel. */
                sal_memcpy(tunnel_type, "IPv6-in-IPv6", sal_strlen("IPv6-in-IPv6"));
            }
            else if ((1 == nvgre_header_mode) && (1 == is_nvgre_encaps))
            {
                sal_memcpy(tunnel_type, "NVGRE-in-IPv6", sal_strlen("NVGRE-in-IPv6"));
            }
            else if ((1 == inner_header_valid) && (2 == inner_header_type))
            {
                if (is_geneve_encaps)
                {
                    sal_memcpy(tunnel_type, "GENEVE-in-IPv6", sal_strlen("GENEVE-in-IPv6"));
                }
                else
                {
                    sal_memcpy(tunnel_type, "VXLAN-in-IPv6", sal_strlen("VXLAN-in-IPv6"));
                }
            }
            else if ((1 == inner_header_valid) && ((1 == inner_header_type) || (0 == inner_header_type)))
            {
                /* CTC_TUNNEL_TYPE_GRE_IN6 */
                /* RFC 1701/2784/2890: GRE-in-IPv6  tunnel. */
                sal_memcpy(tunnel_type, "GRE-in-IPv6", sal_strlen("GRE-in-IPv6"));
                if (0x1 == inner_header_type)
                {
                    gre_info = GetDsL3EditTunnelV6(V, greKeyLow_f, &dsl3edit_v6);
                    gre_info |= GetDsL3EditTunnelV6(V, greKeyHigh_f, &dsl3edit_v6) << 16;
                    sal_sprintf(gre_key, "%s", CTC_DEBUG_HEX_FORMAT(str, format, gre_info, 8, U));
                }
            }
            else
            {
                sal_memcpy(tunnel_type, "-", sal_strlen("-"));
            }

            sal_memset(&hw_ipv6_address, 0, sizeof(ipv6_addr_t));
            sal_memset(&sw_ipv6_address, 0, sizeof(ipv6_addr_t));

            GetDsL3EditTunnelV6(A, ipDa_f, &dsl3edit_v6, hw_ipv6_address);

            sw_ipv6_address[0] = sal_ntohl(hw_ipv6_address[3]);
            sw_ipv6_address[1] = sal_ntohl(hw_ipv6_address[2]);
            sw_ipv6_address[2] = sal_ntohl(hw_ipv6_address[1]);
            sw_ipv6_address[3] = sal_ntohl(hw_ipv6_address[0]);

            sal_inet_ntop(AF_INET6, sw_ipv6_address, ipda, CTC_IPV6_ADDR_STR_LEN);

            cmd = DRV_IOR(DsL3TunnelV6IpSa_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipsa_index, cmd, &v6_sa));

            sal_memset(&hw_ipv6_address, 0, sizeof(ipv6_addr_t));
            sal_memset(&sw_ipv6_address, 0, sizeof(ipv6_addr_t));

            GetDsL3TunnelV6IpSa(A, ipSa_f, &v6_sa, hw_ipv6_address);

            sw_ipv6_address[0] = sal_ntohl(hw_ipv6_address[3]);
            sw_ipv6_address[1] = sal_ntohl(hw_ipv6_address[2]);
            sw_ipv6_address[2] = sal_ntohl(hw_ipv6_address[1]);
            sw_ipv6_address[3] = sal_ntohl(hw_ipv6_address[0]);

            sal_inet_ntop(AF_INET6, sw_ipv6_address, ipsa, CTC_IPV6_ADDR_STR_LEN);
            break;
        }

        case SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4:
        {
            sal_memcpy(tunnel_type, "IVI_6TO4", sal_strlen("IVI_6TO4"));
            break;
        }

        case SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6:
        {
            sal_memcpy(tunnel_type, "IVI_4TO6", sal_strlen("IVI_4TO6"));
            cmd = DRV_IOR(DsL3EditNat6W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3edit_offset, cmd, &dsl3edit_6w));

            sal_memset(&hw_ipv6_address, 0, sizeof(ipv6_addr_t));
            sal_memset(&sw_ipv6_address, 0, sizeof(ipv6_addr_t));

            GetDsL3EditNat6W(A, ipDa_f, &dsl3edit_6w, hw_ipv6_address);

            sw_ipv6_address[0] = sal_ntohl(hw_ipv6_address[3]);
            sw_ipv6_address[1] = sal_ntohl(hw_ipv6_address[2]);
            sw_ipv6_address[2] = sal_ntohl(hw_ipv6_address[1]);
            sw_ipv6_address[3] = sal_ntohl(hw_ipv6_address[0]);

            sal_inet_ntop(AF_INET6, sw_ipv6_address, ipda, CTC_IPV6_ADDR_STR_LEN);
            break;
        }

        case SYS_NH_IP_TUNNEL_FLAG_NAT_V4:
        {
            sal_memcpy(tunnel_type,"NAT",sal_strlen("NAT"));
            cmd = DRV_IOR(DsL3EditNat3W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3edit_offset, cmd, &dsl3edit_nat));
            GetDsL3EditNat3W(A, ipDa_f, &dsl3edit_nat, &ip_da);
            tempip = sal_ntohl(ip_da);
            sal_inet_ntop(AF_INET, &tempip, ipda, CTC_IPV6_ADDR_STR_LEN);
            break;
        }
    }

    if (0 == sal_strcmp(ipda, "0.0.0.0"))
    {
        sal_sprintf(ipda, "%s", "-");
    }

    if (0 == sal_strcmp(ipsa, "0.0.0.0"))
    {
        sal_sprintf(ipsa, "%s", "-");
    }

    if (0 == sal_strcmp(ipda, "::"))
    {
        sal_sprintf(ipda, "%s", "-");
    }

    if (0 == sal_strcmp(ipsa, "::"))
    {
        sal_sprintf(ipsa, "%s", "-");
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_ip_tunnel_brief(uint8 lchip, uint32 nhid,sys_nh_info_ip_tunnel_t* p_nhinfo)
{
    uint32 cmd;

    int32 ret = CTC_E_NONE;
    sys_l3if_prop_t l3if_prop;

    char tunnel_type[32] = {0};
    char ipda[200] = {0};
    char ipsa[200] = {0};
    char gre_key[40] = {0};
    uint16 lport = 0;
    DsL2Edit3WOuter_m dsl2edit3w;
    DsL2Edit6WOuter_m dsl2edit6w;
    hw_mac_addr_t hw_mac;
    mac_addr_t user_mac;

    char str[35];
    char format[10];
    uint32 offset = 0;
    char  nh_str[30] = {0};

    sys_usw_nh_dump_get_tunnel_info(lchip, p_nhinfo, tunnel_type, ipsa, ipda, gre_key);
    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "%-8d%-15s%-15s%-11s%-8s%-8s%-17s%-18s%-18s%s\n",
                       nhid, "IP-Tunnel", "-", "-", "-", "-", "-", "-", "-", "-");
        return CTC_E_NONE;
    }


    NHID_CONVERT_STR(nhid, nh_str);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", nh_str);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "IP-Tunnel");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", tunnel_type);

    CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_IP_TUNNEL, &lport))

    if( CTC_FLAG_ISSET(p_nhinfo->flag, SYS_NH_IP_TUNNEL_REROUTE))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s", "RE-ROUTE");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "-");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "-");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s", CTC_DEBUG_HEX_FORMAT(str, format, p_nhinfo->gport, 4, U));

        l3if_prop.l3if_id = p_nhinfo->l3ifid;
        ret = sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);
        if (p_nhinfo->ecmp_if_id)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "-");
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", p_nhinfo->ecmp_if_id);
        }
        else if (CTC_E_NONE != ret)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "-");
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", l3if_prop.vlan_id);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", l3if_prop.l3if_id);
        }
    }

    if(p_nhinfo->arp_id)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-17d", p_nhinfo->arp_id);
    }
    else
    {
        sal_memset(&user_mac, 0, sizeof(user_mac));

        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            sal_memset(&hw_mac, 0, sizeof(hw_mac));
            sal_memset(&dsl2edit3w, 0, sizeof(DsL2Edit3WOuter_m));
            sal_memset(&dsl2edit6w, 0, sizeof(DsL2Edit6WOuter_m));

            offset = p_nhinfo->outer_l2_edit_offset;

            cmd = DRV_IOR(DsL2Edit6WOuter_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, offset / 2, cmd, &dsl2edit6w));

            if (0 == offset % 2)
            {
                sal_memcpy(&dsl2edit3w, &dsl2edit6w, sizeof(dsl2edit3w));
            }
            else
            {
                sal_memcpy(&dsl2edit3w, (uint8*)&dsl2edit6w + sizeof(dsl2edit3w), sizeof(dsl2edit3w));
            }

            GetDsL2EditEth3W(A, macDa_f, &dsl2edit3w, hw_mac);
            SYS_USW_SET_USER_MAC(user_mac, hw_mac);
        }

        if ((0 == user_mac[0]) && (0 == user_mac[1])
            && (0 == user_mac[2]) && (0 == user_mac[3])
            && (0 == user_mac[4]) && (0 == user_mac[5]))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-17s", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-17s", sys_output_mac(user_mac));
        }
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-36s", ipsa);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-2s", gre_key);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%82s%-18s"," ", ipda);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_ip_tunnel(uint8 lchip, uint32 nhid, uint8 detail, uint8* p_bind_type)
{
    sys_nh_info_ip_tunnel_t* p_ip_tunnel_info;
    sys_nh_info_com_t* p_nhinfo = NULL;

    int32 ret;


    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (CTC_E_NOT_EXIST == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  IP tunnel nexthop %u not exist\n", nhid);
        return CTC_E_NONE;
    }
    else if (CTC_E_INVALID_PARAM == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Invalid nexthopId %u\n", nhid);
        return CTC_E_NONE;
    }
    else
    {
        CTC_ERROR_RETURN(ret);
    }

    if (SYS_NH_TYPE_IP_TUNNEL != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't IP tunnel nexthop for nexthopId %u\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)(p_nhinfo);

    if (p_ip_tunnel_info->updateAd)
    {
        *p_bind_type = SYS_NH_TYPE_IP_TUNNEL;
    }
    CTC_ERROR_RETURN(sys_usw_nh_dump_ip_tunnel_brief(lchip, nhid,p_ip_tunnel_info));

    return CTC_E_NONE;
}

int32
sys_usw_nh_wlan_tunnel_dump_get_tunnel_info(uint8 lchip, sys_nh_info_wlan_tunnel_t* p_nhinfo, char* tunnel_type,
                                       char* ipsa, char* ipda, char* src_port, char* dest_port)
{
    uint32 l3edit_offset = p_nhinfo->dsl3edit_offset;
    DsL3EditTunnelV4_m  dsl3edit_v4;
    DsL3EditTunnelV6_m  dsl3edit_v6;
    uint32 cmd = 0;
    uint8 flag = p_nhinfo->flag;
    uint32 tempip = 0;
    DsL3TunnelV4IpSa_m v4_sa;
    DsL3TunnelV6IpSa_m v6_sa;
    ipv6_addr_t hw_ipv6_address;
    ipv6_addr_t sw_ipv6_address;
    uint8 ipsa_index = 0;
    uint32 ip_da = 0;
    uint32 ip_sa = 0;
    uint16 l4_src_port = 0;
    uint16 l4_dest_port = 0;
    char   str[35];
    char   format[10];

    sal_memcpy(tunnel_type, "-", sal_strlen("-"));
    sal_memcpy(ipsa, "-", sal_strlen("-"));
    sal_memcpy(ipda, "-", sal_strlen("-"));
    sal_memcpy(src_port, "-", sal_strlen("-"));
    sal_memcpy(dest_port, "-", sal_strlen("-"));

    switch (flag)
    {
        case SYS_NH_WLAN_TUNNEL_FLAG_IN_V4:
        {
            cmd = DRV_IOR(DsL3EditTunnelV4_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3edit_offset, cmd, &dsl3edit_v4));

            ipsa_index = GetDsL3EditTunnelV4(V, ipSaIndex_f, &dsl3edit_v4);
            ip_da = GetDsL3EditTunnelV4(V, ipDa_f, &dsl3edit_v4);
            cmd = DRV_IOR(DsL3TunnelV4IpSa_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipsa_index, cmd, &v4_sa));
            ip_sa = GetDsL3TunnelV4IpSa(V, ipSa_f, &v4_sa);
            sal_memcpy(tunnel_type, "TUNNEL-IPv4", sal_strlen("TUNNEL-IPv4"));
            l4_src_port = GetDsL3EditTunnelV4(V, udpDestPort_f, &dsl3edit_v4);
            l4_dest_port = GetDsL3EditTunnelV4(V, udpSrcPort_f, &dsl3edit_v4);
            sal_sprintf(src_port, "%s", CTC_DEBUG_HEX_FORMAT(str, format, l4_src_port, 4, U));
            sal_sprintf(dest_port, "%s", CTC_DEBUG_HEX_FORMAT(str, format, l4_dest_port, 4, U));
            tempip = sal_ntohl(ip_sa);
            sal_inet_ntop(AF_INET, &tempip, ipsa, CTC_IPV6_ADDR_STR_LEN);

            tempip = sal_ntohl(ip_da);
            sal_inet_ntop(AF_INET, &tempip, ipda, CTC_IPV6_ADDR_STR_LEN);
            break;
        }

        case SYS_NH_WLAN_TUNNEL_FLAG_IN_V6:
        {
            cmd = DRV_IOR(DsL3EditTunnelV6_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3edit_offset, cmd, &dsl3edit_v6));

            sal_memcpy(tunnel_type, "TUNNEL-IPv6", sal_strlen("TUNNEL-IPv6"));
            l4_src_port = GetDsL3EditTunnelV6(V, udpDestPort_f, &dsl3edit_v6);
            l4_dest_port = GetDsL3EditTunnelV6(V, udpSrcPort_f, &dsl3edit_v6);
            sal_sprintf(src_port, "%s", CTC_DEBUG_HEX_FORMAT(str, format, l4_src_port, 4, U));
            sal_sprintf(dest_port, "%s", CTC_DEBUG_HEX_FORMAT(str, format, l4_dest_port, 4, U));
            ipsa_index = GetDsL3EditTunnelV6(V, ipSaIndex_f, &dsl3edit_v6);
            sal_memset(&hw_ipv6_address, 0, sizeof(ipv6_addr_t));
            sal_memset(&sw_ipv6_address, 0, sizeof(ipv6_addr_t));

            GetDsL3EditTunnelV6(A, ipDa_f, &dsl3edit_v6, hw_ipv6_address);

            sw_ipv6_address[0] = sal_ntohl(hw_ipv6_address[3]);
            sw_ipv6_address[1] = sal_ntohl(hw_ipv6_address[2]);
            sw_ipv6_address[2] = sal_ntohl(hw_ipv6_address[1]);
            sw_ipv6_address[3] = sal_ntohl(hw_ipv6_address[0]);

            sal_inet_ntop(AF_INET6, sw_ipv6_address, ipda, CTC_IPV6_ADDR_STR_LEN);

            cmd = DRV_IOR(DsL3TunnelV6IpSa_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ipsa_index, cmd, &v6_sa));

            sal_memset(&hw_ipv6_address, 0, sizeof(ipv6_addr_t));
            sal_memset(&sw_ipv6_address, 0, sizeof(ipv6_addr_t));

            GetDsL3TunnelV6IpSa(A, ipSa_f, &v6_sa, hw_ipv6_address);

            sw_ipv6_address[0] = sal_ntohl(hw_ipv6_address[3]);
            sw_ipv6_address[1] = sal_ntohl(hw_ipv6_address[2]);
            sw_ipv6_address[2] = sal_ntohl(hw_ipv6_address[1]);
            sw_ipv6_address[3] = sal_ntohl(hw_ipv6_address[0]);

            sal_inet_ntop(AF_INET6, sw_ipv6_address, ipsa, CTC_IPV6_ADDR_STR_LEN);
            break;
        }
    }

    if (0 == sal_strcmp(ipda, "0.0.0.0"))
    {
        sal_sprintf(ipda, "%s", "-");
    }

    if (0 == sal_strcmp(ipsa, "0.0.0.0"))
    {
        sal_sprintf(ipsa, "%s", "-");
    }

    if (0 == sal_strcmp(ipda, "::"))
    {
        sal_sprintf(ipda, "%s", "-");
    }

    if (0 == sal_strcmp(ipsa, "::"))
    {
        sal_sprintf(ipsa, "%s", "-");
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_wlan_tunnel_brief(uint8 lchip, uint32 nhid,sys_nh_info_wlan_tunnel_t* p_nhinfo)
{

    char tunnel_type[32] = {0};
    char ipda[200] = {0};
    char ipsa[200] = {0};
    char src_port[40] = {0};
    char dest_port[40] = {0};
    char  nh_str[30] = {0};


    sys_usw_nh_wlan_tunnel_dump_get_tunnel_info(lchip, p_nhinfo, tunnel_type, ipsa, ipda, src_port, dest_port);
    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "%-8d%-15s%-15s%-11s%-8s%-8s%-17s%-18s%-18s%s\n",
                       nhid, "WLAN-Tunnel", "-", "-", "-", "-", "-", "-", "-", "-");
        return CTC_E_NONE;
    }

    NHID_CONVERT_STR(nhid, nh_str);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d", nhid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "WLAN-Tunnel");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", tunnel_type);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s", dest_port);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s", src_port);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-36s", ipsa);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s", "-");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%60s%-18s"," ", ipda);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_wlan_tunnel(uint8 lchip, uint32 nhid, uint8 detail)
{
    sys_nh_info_wlan_tunnel_t* p_wlan_tunnel_info;
    sys_nh_info_com_t* p_nhinfo = NULL;

    int32 ret;


    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (CTC_E_NOT_EXIST == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Wlan tunnel nexthop %u not exist\n", nhid);
        return CTC_E_NONE;
    }
    else if (CTC_E_INVALID_PARAM == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Invalid nexthopId %u\n", nhid);
        return CTC_E_NONE;
    }
    else
    {
        CTC_ERROR_RETURN(ret);
    }

    if (SYS_NH_TYPE_WLAN_TUNNEL != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't Wlan tunnel nexthop for nexthopId %u\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_wlan_tunnel_info = (sys_nh_info_wlan_tunnel_t*)(p_nhinfo);
    CTC_ERROR_RETURN(sys_usw_nh_dump_wlan_tunnel_brief(lchip, nhid, p_wlan_tunnel_info));

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_trill_brief(uint8 lchip, uint32 nhid,sys_nh_info_trill_t* p_nhinfo)
{
    char  nh_str[30] = {0};

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                      "%-8d%-10s%-12s%-20s%-18s%-18s\n",
                       nhid, "TRILL", "-", "-", "-", "-");
        return CTC_E_NONE;
    }

    NHID_CONVERT_STR(nhid, nh_str);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d", nhid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "TRILL");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%.4x      ", p_nhinfo->gport);

    if (p_nhinfo->p_dsl2edit_4w)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s", sys_output_mac(p_nhinfo->p_dsl2edit_4w->mac_da));
    }
    else
    {
         SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s", "-");
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18d", p_nhinfo->egress_nickname);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18d", p_nhinfo->ingress_nickname);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}


int32
sys_usw_nh_dump_trill(uint8 lchip, uint32 nhid, uint8 detail, uint8* p_bind_type)
{
    sys_nh_info_trill_t* p_trill_info;
    sys_nh_info_com_t* p_nhinfo = NULL;

    int32 ret;


    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (CTC_E_NOT_EXIST == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  TRILL nexthop %u not exist\n", nhid);
        return CTC_E_NONE;
    }
    else if (CTC_E_INVALID_PARAM == ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Invalid nexthopId %u\n", nhid);
        return CTC_E_NONE;
    }
    else
    {
        CTC_ERROR_RETURN(ret);
    }

    if (SYS_NH_TYPE_TRILL != p_nhinfo->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't TRILL nexthop for nexthopId %u\n", nhid);
        return CTC_E_INVALID_PARAM;
    }

    p_trill_info = (sys_nh_info_trill_t*)(p_nhinfo);
    if (p_trill_info->updateAd)
    {
        *p_bind_type = SYS_NH_TYPE_TRILL;
    }
    CTC_ERROR_RETURN(sys_usw_nh_dump_trill_brief(lchip, nhid,p_trill_info));

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_mpls_tunnel(uint8 lchip, uint16 tunnel_id, uint8 detail)
{
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel;
    int32 ret = 0;
    DsL2Edit6WOuter_m dsl2edit;
    DsL3EditMpls3W_m dsl3editmpls;
    ctc_aps_bridge_group_t  aps_group;
    ctc_aps_bridge_group_t  aps_group2;
    DsApsBridge_m ds_aps_bridge;
    hw_mac_addr_t hw_mac;
    mac_addr_t sw_mac;
    sys_l3if_prop_t l3if_prop;
    uint8 lsp_loop_end = 0, spme_loop_end = 0;
    uint8 protect_lsp = 0,protect_spme = 0;
    uint32 offset = 0;
    uint32 lsp_label = 0, spme_label = 0;
    uint8 level1_aps_protect_en = 0, level2_aps_protect_en =0;
    uint8  exist_spme = 0;
    uint8 l2edit_type = 0;
    uint32  gport[SYS_NH_APS_M][SYS_NH_APS_M];
    uint16  l3if_id[SYS_NH_APS_M][SYS_NH_APS_M];
    uint8   active[SYS_NH_APS_M][SYS_NH_APS_M];
    char p_port[16]={0};
    uint8 gchip = 0;

    sys_usw_get_gchip_id(lchip, &gchip);
    sal_memset(&dsl2edit, 0, sizeof(DsL2Edit6WOuter_m));
    sal_memset(&dsl3editmpls, 0, sizeof(DsL3EditMpls3W_m));
    sal_memset(&ds_aps_bridge, 0, sizeof(DsApsBridge_m));

    SYS_NH_INIT_CHECK;

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel));

    if (!p_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% MPLS Tunnel is not exist \n");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-14s%-11s%-13s%-9s%-14s%-17s%-8s%-9s%s\n",
                   "TUNNEL_ID", "REF_CN", "DESTPORT", "VLAN", "L3IFID", "MACDA/ARPID", "LSP", "SPME", "ACTIVE");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------------\n");


    if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
    {
        sys_usw_aps_get_ports(lchip, p_mpls_tunnel->gport, &aps_group);
        level1_aps_protect_en = aps_group.protect_en;
        if (aps_group.next_w_aps_en)
        {
            sys_usw_aps_get_ports(lchip, aps_group.next_aps.w_group_id, &aps_group2);
            gport[0][0] = aps_group2.working_gport;
            gport[0][1] = aps_group2.protection_gport;
            l3if_id[0][0] = aps_group2.w_l3if_id;
            l3if_id[0][1] = aps_group2.p_l3if_id;
            if(level1_aps_protect_en == 0)
            {
                level2_aps_protect_en = aps_group2.protect_en;
            }
        }
        else
        {
            gport[0][0] = aps_group.working_gport;
            l3if_id[0][0] = aps_group.w_l3if_id;
        }

        if (aps_group.next_p_aps_en)
        {
            sys_usw_aps_get_ports(lchip, aps_group.next_aps.p_group_id, &aps_group2);
            gport[1][0] = aps_group2.working_gport;
            gport[1][1] = aps_group2.protection_gport;
            l3if_id[1][0] = aps_group2.w_l3if_id;
            l3if_id[1][1] = aps_group2.p_l3if_id;
            if(level1_aps_protect_en)
            {
                level2_aps_protect_en = aps_group2.protect_en;
            }
        }
        else
        {
            gport[1][0] = aps_group.protection_gport;
            l3if_id[1][0] = aps_group.p_l3if_id;
        }
    }
    else
    {

      sys_nh_db_arp_t *p_arp ;
      if(p_mpls_tunnel->arp_id[0][0])
      {
           p_arp= sys_usw_nh_lkup_arp_id(lchip, p_mpls_tunnel->arp_id[0][0]);
           if (NULL == p_arp)
           {
               return CTC_E_NOT_EXIST;
           }
           gport[0][0] = ((CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))?CTC_GPORT_RCPU(gchip):p_arp->gport);
           l3if_id[0][0] = p_arp->l3if_id;
      }
      else
      {
          gport[0][0] = p_mpls_tunnel->gport;
          l3if_id[0][0] = p_mpls_tunnel->l3ifid;
      }
    }

    if(level1_aps_protect_en == 0)
    {
        active[0][0] = !level2_aps_protect_en;
        active[0][1] = level2_aps_protect_en;
        active[1][0] = 0;
        active[1][1] = 0;

    }
    else
    {
        active[0][0] = 0;
        active[0][1] = 0;
        active[1][0] = !level2_aps_protect_en;
        active[1][1] = level2_aps_protect_en;
    }

    if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
    {
        protect_lsp = CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_PROTECTION);
        lsp_loop_end = protect_lsp + 1;
        spme_loop_end =2;
    }
    else
    {
        protect_lsp = 0;
        lsp_loop_end = CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS) ? 2 : 1;
        spme_loop_end = 1;
    }

    for (protect_lsp = 0; protect_lsp < lsp_loop_end; protect_lsp++)
    {
       for (protect_spme = 0; protect_spme < spme_loop_end; protect_spme++)
       {
        offset = p_mpls_tunnel->lsp_offset[protect_lsp];
        CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, offset, &dsl3editmpls));
        lsp_label = GetDsL3EditMpls3WLsp(V, label_f, &dsl3editmpls);

        /* SPME */
        if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME))
        {
            offset = p_mpls_tunnel->spme_offset[protect_lsp][protect_spme];
        }
        else if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
        {
            offset = p_mpls_tunnel->lsp_offset[protect_lsp];
        }
        else
        {
            offset = 0;
        }

        if (offset)
        {
            DsL3Edit6W3rd_m l3edit6w;
            sal_memset(&l3edit6w, 0, sizeof(l3edit6w));
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME,
                                                              offset, &l3edit6w));
            if (offset % 2 == 0)
            {
                sal_memcpy(&dsl3editmpls, (uint8*)&l3edit6w,  sizeof(dsl3editmpls));
            }
            else
            {
                sal_memcpy( &dsl3editmpls, (uint8*)&l3edit6w + sizeof(dsl3editmpls), sizeof(dsl3editmpls));
            }

            spme_label = GetDsL3EditMpls3WLsp(V, label_f, &dsl3editmpls);
            exist_spme = 1;
        }
        else
        {
            exist_spme = 0;
        }

        /* L2Edit */
        offset = p_mpls_tunnel->l2edit_offset[protect_lsp][protect_spme];
        CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W,
                                                          offset, &dsl2edit));
        l2edit_type = GetDsL2Edit6WOuter(V, l2RewriteType_f, &dsl2edit);

        if (l2edit_type == 3)
        {
            GetDsL2Edit6WOuter(A, macDa_f, &dsl2edit, hw_mac);
        }
        else
        {
            DsL2Edit3WOuter_m dsl2edit_3w;

            if ((offset%2) == 0)
            {
                sal_memcpy(&dsl2edit_3w, (uint8*)&dsl2edit,  sizeof(dsl2edit_3w));
            }
            else
            {
                sal_memcpy( &dsl2edit_3w, (uint8*)&dsl2edit + sizeof(dsl2edit_3w), sizeof(dsl2edit_3w));
            }
            GetDsL2EditEth3W(A, macDa_f, &dsl2edit_3w, hw_mac);
        }
        SYS_USW_SET_USER_MAC(sw_mac, hw_mac);

        /* dump mpls tunnel in brief */
        if ((protect_lsp == 0)  && (protect_spme == 0 ))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-14u%-11u", tunnel_id, p_mpls_tunnel->ref_cnt);
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-14s%-11s", "", "");
        }
        /* work path */
        sal_sprintf(p_port, "0x%04x", gport[protect_lsp][protect_spme]);

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", p_mpls_tunnel->loop_nhid[protect_lsp][protect_spme]?"-":p_port);

        l3if_prop.l3if_id = l3if_id[protect_lsp][protect_spme];
        ret =  sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);
        if (CTC_E_NONE != ret)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "-");
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-14s", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u",  l3if_prop.vlan_id);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-14u", l3if_prop.l3if_id);
        }

        if(p_mpls_tunnel->arp_id[protect_lsp][protect_spme])
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-17d", p_mpls_tunnel->arp_id[protect_lsp][protect_spme]);
        }
        else
        {
            if ((0 == sw_mac[0]) && (0 == sw_mac[1]) && (0 == sw_mac[2])
                && (0 == sw_mac[3]) && (0 == sw_mac[4]) && (0 == sw_mac[5]))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-17s", "-");
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-17s", sys_output_mac(sw_mac));
            }
        }

        if (!p_mpls_tunnel->label_num)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s ", "-");
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7u ", lsp_label);
        }

        if (exist_spme)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9u", spme_label);
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s", "-");
        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5s\n", active[protect_lsp][protect_spme] ? "Y" : "N");
      }
    }

    if (TRUE == detail)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------------\n");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Detail Table offset \n");

        for (protect_lsp = 0; protect_lsp < lsp_loop_end; protect_lsp++)
        {
          for (protect_spme = 0; protect_spme < spme_loop_end; protect_spme++)
          {
             if (!CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME)&&p_mpls_tunnel->label_num)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-%-20s :%-4u [%s]\n", CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR)?"DsL3EditMpls12W":"DsL3EditMpls3W",  \
                    p_mpls_tunnel->lsp_offset[protect_lsp],protect_lsp ? "Lsp(P)":"Lsp");
            }

            /* SPME */
            if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME))
            {
                offset = p_mpls_tunnel->spme_offset[protect_lsp][protect_spme];
            }
            else if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
            {
                offset = p_mpls_tunnel->lsp_offset[protect_lsp];
            }
            else
            {
                offset = 0;
            }

            if (offset)
            {
              SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-%-20s :%-4u [%s]\n", "DsL3Edit3W3rd",offset,protect_spme ? "SPME(P)":"SPME");
            }

            offset = p_mpls_tunnel->l2edit_offset[protect_lsp][protect_spme];
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, offset, &dsl2edit));
            l2edit_type = GetDsL2Edit6WOuter(V, l2RewriteType_f, &dsl2edit);

            if (l2edit_type == SYS_NH_L2EDIT_TYPE_ETH_8W)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-%-20s :%-4u\n","DsL2Edit6WOuter", p_mpls_tunnel->l2edit_offset[protect_lsp][protect_spme]/2);
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-%-20s :%-4u\n","DsL2Edit3WOuter", p_mpls_tunnel->l2edit_offset[protect_lsp][protect_spme]);
            }
            }
        }
    }
    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_misc(uint8 lchip, uint32 nhid, uint8 detail, uint8* p_bind_type)
{
    sys_nh_info_misc_t* p_nhinfo_misc;
    sys_nh_info_com_t* p_nhinfo_com = NULL;
    char  nh_str[30] = {0};
    int32 ret;

    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo_com));
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (SYS_NH_TYPE_MISC != p_nhinfo_com->hdr.nh_entry_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% Isn't misc nexthop for nexthopId %u\n", nhid);
        return CTC_E_INVALID_PARAM;
    }
    NHID_CONVERT_STR(nhid, nh_str);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8u", nhid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s", "MISC");
    p_nhinfo_misc = (sys_nh_info_misc_t*)(p_nhinfo_com);
    if (CTC_IS_CPU_PORT(p_nhinfo_misc->gport))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "CPU");
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-.4x\n", p_nhinfo_misc->gport);
    }

    if (p_nhinfo_misc->updateAd)
    {
        *p_bind_type = SYS_NH_TYPE_MISC;
    }
    return CTC_E_NONE;
}

void
sys_usw_nh_dump_head_by_type(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nh_type)
{
    int32 ret;
    sys_nh_info_com_t* p_nh_com_info = NULL;

    if ((nh_type != SYS_NH_TYPE_NULL) && (nh_type != SYS_NH_TYPE_DROP) && (nh_type != SYS_NH_TYPE_TOCPU))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nI: Internal Nexthop, NHID is offset based on 0x80000000\n");
    }
    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info));
    if (ret != CTC_E_NONE)
    {
        return;
    }

    switch (nh_type)
    {
    case SYS_NH_TYPE_NULL:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nNHID is %u, Invalid Nexthop\n", nhid);
        break;

    case SYS_NH_TYPE_MCAST:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s%-15s%-15s\n", "NHID", "TYPE", "GROUP_ID", "LOGIC_REP");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
        break;

    case SYS_NH_TYPE_BRGUC:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-17s%-16s%-10s%-8s\n", "NHID", "TYPE", "SUB_TYPE", "DESTPORT", "ACTIVE");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------\n");
        break;

    case SYS_NH_TYPE_IPUC:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s%-13s%-9s%-10s%-16s%-8s\n", "NHID", "TYPE", "DESTPORT", "VLAN", "L3IFID", "MACDA/ARPID","ACTIVE");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------\n");
        break;

    case SYS_NH_TYPE_MPLS:
        if (!CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH))
        {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-9s%-9s%-14s%-13s%-14s%s\n", "NHID", "TYPE", "TUNNEL_ID", "VC_LABEL", "APS_GROUP", "ACTIVE");
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------\n");

        }
        else
        {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s%-13s%-9s%-10s%s\n", "NHID", "TYPE", "DESTPORT", "VLAN", "L3IFID", "MACDA/ARPID");
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------\n");
        }

        break;

    case SYS_NH_TYPE_ECMP:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s\n", "NHID", "TYPE");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------\n");
        break;

    case SYS_NH_TYPE_ILOOP:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s%s\n", "NHID", "TYPE", "LPBK_LPORT");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "\n%-8s%-15s%-15s%-11s%-8s%-8s%-17s%-18s%-18s%s\n",
                       "NHID", "TYPE", "TUNNEL_TYPE", "DESTPORT", "VLAN", "L3IFID", "MACDA/ARPID", "IPSA/IPDA", " ", "GRE_KEY");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "--------------------------------------------------------------------------------------------------------------------------------\n");
        break;

    case SYS_NH_TYPE_TRILL:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "\n%-8s%-10s%-12s%-20s%-18s%-18s\n",
                       "NHID", "TYPE", "DESTPORT", "MACDA", "EGR-NICKNAME", "IGR-NICKNAME");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "--------------------------------------------------------------------------------\n");
        break;

    case SYS_NH_TYPE_RSPAN:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s%s\n", "NHID", "TYPE", "VLAN");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------\n");
        break;

    case SYS_NH_TYPE_MISC:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-8s%-15s%-8s\n", "NHID", "TYPE","DESTPORT");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");
        break;

    case SYS_NH_TYPE_WLAN_TUNNEL:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "\n%-8s%-15s%-15s%-11s%-11s%-18s%-18s%s\n",
                       "NHID", "TYPE", "TUNNEL_TYPE", "DESTPORT", "SRCPORT", "IPSA/IPDA", " ", "IS_DOT11");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                       "--------------------------------------------------------------------------------------------------------------------------------\n");
        break;

    default:
        return;
    }

    return;
}

void
sys_usw_nh_dump_type(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nh_type, uint8 detail)
{
    uint8 bind_type = SYS_NH_TYPE_NULL;

    switch (nh_type)
    {
    case SYS_NH_TYPE_MCAST:
        sys_usw_nh_dump_mcast(lchip, nhid, detail);
        break;

    case SYS_NH_TYPE_BRGUC:
        sys_usw_nh_dump_brguc(lchip, nhid, detail);
        break;

    case SYS_NH_TYPE_IPUC:
        sys_usw_nh_dump_ipuc(lchip, nhid, detail, &bind_type);
        break;

    case SYS_NH_TYPE_MPLS:
        sys_usw_nh_dump_mpls(lchip, nhid, detail, &bind_type);
        break;

    case SYS_NH_TYPE_ECMP:
        sys_usw_nh_dump_ecmp(lchip, nhid, detail);
        break;

    case SYS_NH_TYPE_DROP:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NHID is %d, Drop Nexthop\n", nhid);
        break;

    case SYS_NH_TYPE_TOCPU:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NHID is %d, ToCPU Nexthop\n", nhid);
        break;

    case SYS_NH_TYPE_UNROV:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NHID is %d, Unresolved Nexthop\n", nhid);
        break;

    case SYS_NH_TYPE_ILOOP:
        sys_usw_nh_dump_iloop(lchip, nhid, detail);
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        sys_usw_nh_dump_ip_tunnel(lchip, nhid, detail, &bind_type);
        break;
    case SYS_NH_TYPE_TRILL:
        sys_usw_nh_dump_trill(lchip, nhid, detail, &bind_type);
        break;

    case SYS_NH_TYPE_ELOOP:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NHID is %d, E-Loop Nexthop\n", nhid);
        break;

    case SYS_NH_TYPE_RSPAN:
        sys_usw_nh_dump_rspan(lchip, nhid, detail);
        break;

    case SYS_NH_TYPE_MISC:
        sys_usw_nh_dump_misc(lchip, nhid, detail, &bind_type);
        break;
    case SYS_NH_TYPE_WLAN_TUNNEL:
        sys_usw_nh_dump_wlan_tunnel(lchip, nhid, detail);
        break;
    default:
        return;
    }
	if(detail )
	{
	   sys_usw_nh_dump_table_offset(lchip, nhid, bind_type);
	}

}


void
sys_usw_nh_dump_by_type(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nh_type, uint8 detail)
{
    sys_usw_nh_dump_head_by_type(lchip, nhid, nh_type);
    sys_usw_nh_dump_type(lchip, nhid, nh_type, detail);

    return;
}

int32
sys_usw_nh_dump(uint8 lchip, uint32 nhid, bool detail)
{

    sys_nh_info_com_t* p_nhinfo = NULL;

    SYS_NH_INIT_CHECK;

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid,  &p_nhinfo));
    if (p_nhinfo)
    {
        sys_usw_nh_dump_by_type(lchip, nhid, p_nhinfo->hdr.nh_entry_type, detail);
    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_dump_mcast_group(uint8 lchip, uint32 group_id, uint8 detail)
{
    uint32 nhid = 0;

    SYS_NH_INIT_CHECK;

    CTC_ERROR_RETURN(sys_usw_nh_get_mcast_nh(lchip, group_id, &nhid));
    CTC_ERROR_RETURN(sys_usw_nh_dump(lchip, nhid, detail));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_nhid_compare(sys_nh_info_com_t* p0, sys_nh_info_com_t* p1)
{
    return((p0->hdr.nh_id >= p1->hdr.nh_id)?1:(-1));
}

STATIC int32
_sys_usw_nh_dump_info(void* node_data, void* user_data)
{
    sys_nh_info_com_t* p_nhinfo = NULL;
    sys_traverse_t* p_dump_traverse = NULL;
    sys_usw_nh_type_t nh_type;
    ctc_linklist_t* p_tmp_list = NULL;
    uint8 dump_type = 0;

    p_dump_traverse = (sys_traverse_t*)user_data;
    p_nhinfo=(sys_nh_info_com_t*)node_data;
    nh_type = p_dump_traverse->value1;
    p_tmp_list = (ctc_linklist_t*)p_dump_traverse->data;
    dump_type = p_dump_traverse->value2;

    if ((SYS_NH_TYPE_MAX != nh_type) && (nh_type != p_nhinfo->hdr.nh_entry_type))
    {
        return CTC_E_NONE;
    }

    if (((dump_type == 1) && (p_nhinfo->hdr.nh_id < SYS_NH_INTERNAL_NHID_BASE)) ||
       ((dump_type == 2) && (p_nhinfo->hdr.nh_id >= SYS_NH_INTERNAL_NHID_BASE)))
    {
        return CTC_E_NONE;
    }

    ctc_listnode_add_sort(p_tmp_list, p_nhinfo);

    return CTC_E_NONE;
}

/*dump_type:0-All, 1-Internal Onlt, 2-Externel Only*/
int32
sys_usw_nh_dump_all(uint8 lchip, sys_usw_nh_type_t nh_type, uint8 dump_type)
{
    uint32 start_type = 0;
    uint32 end_type = 0;
    uint32 loop = 0;
    sys_traverse_t dump_traverse;
    ctc_linklist_t* p_tmp_list = NULL;
    sys_nh_info_com_t* p_nhinfo = NULL;
    struct ctc_listnode* node;

    LCHIP_CHECK(lchip);
    SYS_NH_INIT_CHECK;

    p_tmp_list = ctc_list_create((ctc_list_cmp_cb_t) _sys_usw_nh_nhid_compare, NULL);

    if(!p_tmp_list)
    {
        return CTC_E_NO_MEMORY;
    }

    start_type = (SYS_NH_TYPE_MAX == nh_type) ? SYS_NH_TYPE_NULL : nh_type;
    end_type   = (SYS_NH_TYPE_MAX == nh_type) ? (SYS_NH_TYPE_MAX - 1) : nh_type;

    for (loop = start_type; loop <= end_type; loop++)
    {
        sal_memset(&dump_traverse, 0, sizeof(dump_traverse));
        dump_traverse.data = p_tmp_list;
        dump_traverse.value1 = loop;
        dump_traverse.value2 = dump_type;

        ctc_hash_traverse(g_usw_nh_master[lchip]->nhid_hash, (hash_traversal_fn)_sys_usw_nh_dump_info, &dump_traverse);

        if (p_tmp_list->count)
        {
            p_nhinfo = (sys_nh_info_com_t*)p_tmp_list->head->data;
            sys_usw_nh_dump_head_by_type(lchip, p_nhinfo->hdr.nh_id,loop);
        }

        CTC_LIST_LOOP(p_tmp_list, p_nhinfo, node)
        {
            sys_usw_nh_dump_type(lchip, p_nhinfo->hdr.nh_id, p_nhinfo->hdr.nh_entry_type, FALSE);
        }

        if (p_tmp_list->count)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        }

        ctc_list_delete_all_node(p_tmp_list);

    }


    ctc_list_free(p_tmp_list);

    return CTC_E_NONE;
}

extern int32
sys_usw_nh_dump_arp(uint8 lchip, uint16 arp_id, bool detail)
{
    sys_nh_db_arp_t* p_arp = NULL;
    uint8 gchip = 0;


    sys_usw_get_gchip_id(lchip, &gchip);
    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));

    if (!p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%% ARP Id is not exist \n");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
	    return CTC_E_NOT_EXIST;
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ARP_ID     ACTION      MAC_DA         VLAN         PORT\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d %-10s ",
                   arp_id,     (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP) ? "Discard" :
                            ((CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU)) ? "TO CPU" : "FWD")));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%04x.%04x.%04x  ",
                            (p_arp->mac_da[0] << 8 | p_arp->mac_da[1]), (p_arp->mac_da[2] << 8 | p_arp->mac_da[3]),(p_arp->mac_da[4] << 8 | p_arp->mac_da[5]));
    if(CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_VLAN_VALID))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12d",
                   p_arp->output_vid);
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s",
                       "-");
    }


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%.4x \n",  ((CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))?CTC_GPORT_RCPU(gchip):p_arp->gport));

    if (TRUE == detail)
    {
        if (p_arp->offset || p_arp->in_l2_offset|| p_arp->destmap_profile || p_arp->ref_cnt)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Table offset \n");
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------\n");
        }

        if (p_arp->offset)
        {
            uint32 cmd = 0;
            uint8  l2_rewrite_type = 0;
            DsL2Edit6WOuter_m l2edit;

            cmd = DRV_IOR(DsL2Edit6WOuter_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_arp->offset/2, cmd, &l2edit));

            l2_rewrite_type = GetDsL2Edit6WOuter(V, l2RewriteType_f, &l2edit);

            if (l2_rewrite_type == SYS_NH_L2EDIT_TYPE_ETH_8W)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsL2Edit6WOuter", p_arp->offset);
            }
            else
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsL2Edit3WOuter", p_arp->offset);
            }
        }

        if (p_arp->in_l2_offset)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsL2EditEth3W", p_arp->in_l2_offset);
        }

        if (p_arp->destmap_profile)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "DsDestMapProfileUc", p_arp->destmap_profile);
        }

        if (p_arp->ref_cnt)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "Ref Count", p_arp->ref_cnt);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_dump_resource_usage(uint8 lchip)
{
    uint32 used_cnt = 0;
    uint32 entry_enum, glb_entry_num;
    uint8 dyn_tbl_idx ;
    sys_usw_nh_master_t* p_nh_master = NULL;


    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------Work Mode----------------------------\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %s\n","Packet Edit mode", p_nh_master->pkt_nh_edit_mode ?"Egress Chip":"Ingess Chip");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %d\n", "Max ecmp member",p_nh_master->max_ecmp);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %d\n", "ECMP group count",p_nh_master->cur_ecmp_cnt);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: %s\n", "IPMC support logic replication",p_nh_master->ipmc_logic_replication ? "Yes":"No");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-32s: 0x%x\n", "Internal Nexthop Base", (p_nh_master->internal_nexthop_base<<4));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n------------------ Nexthop Resource ---------------------------\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","External Nexthop ");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",p_nh_master->max_external_nhid - 1);/*nhid 0: reserved*/
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used count", (p_nh_master->nhid_hash->count-p_nh_master->internal_nh_used_num));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n","Internal Nexthop");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count", SYS_NH_INTERNAL_NHID_MAX_SIZE);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used  count ",p_nh_master->internal_nh_used_num);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s:\n","MPLS Tunnel");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Total count ",p_nh_master->max_tunnel_id);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Used  count ",p_nh_master->tunnel_id_vec->used_cnt);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n------------------Created Nexthop ---------------------------\n");
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Mcast Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_MCAST]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","BRGUC Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_BRGUC]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","IPUC Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_IPUC]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","MPLS Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_MPLS]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","ECMP Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_ECMP]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Rspan Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_RSPAN]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Ip Tunnel Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_IP_TUNNEL]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","TRILL Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_TRILL]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Wlan Tunnel Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_WLAN_TUNNEL]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Misc Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_MISC]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","ToCpu Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_TOCPU]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","unRov Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_UNROV]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","ILoop Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_ILOOP]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","ELoop Nexthop", p_nh_master->nhid_used_cnt[SYS_NH_TYPE_ELOOP]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","Rsv Drop Nexthop", 1);


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n----------------------Nexthop Table from Profile----------------\n");
    sys_usw_ftm_get_dynamic_table_info(lchip, DsFwd_t,  &dyn_tbl_idx, &entry_enum, &glb_entry_num);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %-8u in DYN%d_SRAM\n", "DsFwd", entry_enum, dyn_tbl_idx);
    sys_usw_ftm_get_dynamic_table_info(lchip, DsMetEntry3W_t,  &dyn_tbl_idx,  &entry_enum, &glb_entry_num);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %-8u in DYN%d_SRAM \n", "DsMet", entry_enum, dyn_tbl_idx);
    sys_usw_ftm_get_dynamic_table_info(lchip, DsNextHop4W_t,  &dyn_tbl_idx, &entry_enum, &glb_entry_num);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %-8u in DYN%d_SRAM \n", "DsNexthop", entry_enum, dyn_tbl_idx);
    sys_usw_ftm_get_dynamic_table_info(lchip, DsL2EditEth3W_t,  &dyn_tbl_idx, &entry_enum, &glb_entry_num);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %-8u in DYN%d_SRAM\n", "DsEdit", entry_enum, dyn_tbl_idx);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %-8u in STATIC_SRAM\n", "DsL3Edit3W_SPME", 1024);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %-8u in STATIC_SRAM\n", "DsL2EditEth_Outer", 1024);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n--------------------[lchip :%d]Nexthop Table Usage -------------------------\n",lchip);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","DsFwd",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_FWD]);

    used_cnt = p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_MET]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_MET_6W]
             + p_nh_master->max_glb_met_offset;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s: %u\n", "DsMet", used_cnt);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u \n", "--Global DsMet", p_nh_master->max_glb_met_offset);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--DsMet3w",  p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_MET]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n", "--DsMet6w", p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_MET_6W]);

    used_cnt = p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_NEXTHOP_4W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_NEXTHOP_8W]
             + p_nh_master->max_glb_nh_offset;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s: %u\n","DsNexthop",used_cnt);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--Global DsNexthop",p_nh_master->max_glb_nh_offset);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsNexthop4w",
        p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_NEXTHOP_4W] );
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsNexthop8w",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_NEXTHOP_8W]);



    used_cnt = p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_LPBK]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_OF_6W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_PBB_4W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_PBB_8W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_SWAP]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP];

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s: %u\n","DsL2Edit",used_cnt);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL2EditEth3W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL2EditEth6W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL2EditFlex8W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL2EditLoopback",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_LPBK]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL2EditPbb3W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_PBB_4W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL2EditPbb6W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_PBB_8W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL2EditSwap",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_SWAP]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL2EditInnerSwap",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP]);

    used_cnt = p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_FLEX]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_MPLS]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_OF_6W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA];
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s: %u\n","DsL3Edit",used_cnt);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL3EditFlex",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_FLEX]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL3EditMpls",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_MPLS]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL3EditMpls12W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL3TunnelV4IpSa",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL3TunnelV6IpSa",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL3EditTunnelV4",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL3EditTunnelV6",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL3EditNat3W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL3EditNat6W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n","--DsL3EditOF6W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_OF_6W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL3EditOF12W",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u [3w]\n","--DsL3EditTrill",p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TRILL]);


    used_cnt = p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_SPME];
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-24s:%u\n","DsL3Edit3W_SPME",used_cnt);

    used_cnt = p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W]
             + p_nh_master->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W];
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s:%u\n","DsL2EditEth_Outer",used_cnt);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s:%u\n","ARP",sys_usw_nh_get_arp_num(lchip));

    return CTC_E_NONE;
}

