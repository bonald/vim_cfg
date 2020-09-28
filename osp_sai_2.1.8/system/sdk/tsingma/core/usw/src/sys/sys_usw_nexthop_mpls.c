/**
 @file sys_usw_nexthop_mpls.c

 @date 2010-01-10

 @version v2.0

 The file contains all nexthop mpls related callback function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_qos_api.h"

#include "sys_usw_aps.h"
#include "sys_usw_l3if.h"
#include "sys_usw_register.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_ftm.h"
#include "drv_api.h"


#define SYS_NH_DSL3EDIT_MPLS4W_LABEL_NUMBER  2
#define SYS_NH_DSL3EDIT_MPLS8W_LABEL_NUMBER  4
#define SYS_NH_MARTINI_SEQ_TYPE_DISABLE      0
#define SYS_NH_MARTINI_SEQ_TYPE_PER_PW       1
#define SYS_NH_MARTINI_SEQ_TYPE_GLB0         2
#define SYS_NH_MARTINI_SEQ_TYPE_GLB1         3
#define SYS_NH_MPLS_LABEL_LOW2BITS_MASK      0xFFFFC
#define SYS_NH_MARTINI_SEQ_SHIFT             4
#define SYS_NH_MPLS_LABEL_MASK               0xFFFFF

#define SYS_NH_MPLS_LABLE_PW                 0
#define SYS_NH_MPLS_LABLE_LSP                1
#define SYS_NH_MPLS_LABLE_SPME               2

#define SYS_NH_MPLS_LSP_OP_ADD                1
#define SYS_NH_MPLS_LSP_OP_UPDATE          2

extern int32
_sys_usw_mpls_nh_get_oam_en(uint8 lchip, void* p_info, sys_nh_update_oam_info_t* p_oam_info,
            uint8 is_protect, uint8 is_tunnel);
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/


STATIC int32
_sys_usw_nh_mpls_encode_dsmpls(uint8 lchip, ctc_mpls_nh_label_param_t* p_label,
                                      sys_dsmpls_t*  p_dsmpls,
                                      uint32 stats_id,
                                      uint8 stats_type)
{
    uint32 ttl_index = 0;

    p_dsmpls->l3_rewrite_type = SYS_NH_L3EDIT_TYPE_MPLS_4W;
    p_dsmpls->ds_type = SYS_NH_DS_TYPE_L3EDIT;
    p_dsmpls->oam_en = 0;

    if (CTC_FLAG_ISSET(p_label->lable_flag, CTC_MPLS_NH_LABEL_MAP_TTL))
    {
        p_dsmpls->map_ttl = 1;
        p_dsmpls->map_ttl_mode = 1;     /* only for lsp label */
    }

    if (CTC_FLAG_ISSET(p_label->lable_flag, CTC_MPLS_NH_LABEL_IS_MCAST))
    {
        p_dsmpls->mcast_label = 1;
    }

    if (stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, stats_id, &(p_dsmpls->statsptr)));
    }

    CTC_ERROR_RETURN(sys_usw_lkup_ttl_index(lchip, p_label->ttl, &ttl_index));
    p_dsmpls->ttl_index = ttl_index;
    p_dsmpls->label  = (p_label->label & 0xFFFFF);   /*label*/

    switch (p_label->exp_type)
    {
        case CTC_NH_EXP_SELECT_ASSIGN:   /* user cfg exp  */
            p_dsmpls->derive_exp  = 0;
            p_dsmpls->exp = p_label->exp & 0x7;
            break;

        case CTC_NH_EXP_SELECT_MAP:  /*mapped exp */
            p_dsmpls->derive_exp = 1;
            p_dsmpls->exp = SYS_NH_MPLS_LABEL_EXP_SELECT_MAP;
            p_dsmpls->mpls_domain = p_label->exp_domain;
            break;

        case CTC_NH_EXP_SELECT_PACKET:   /*input packet exp  */

            p_dsmpls->derive_exp = 1;
            p_dsmpls->exp = SYS_NH_MPLS_LABEL_EXP_SELECT_PACKET;
            break;

        default:
            break;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_nh_mpls_encode_cw(uint8 lchip, ctc_mpls_nexthop_push_param_t* p_nh_param_push,
                                  sys_dsmpls_t*  p_dsmpls)
{

    if (!p_nh_param_push->martini_encap_valid)
    {
       return CTC_E_NONE;
    }

	/*GB use fixed CW*/
    if (0 == p_nh_param_push->martini_encap_type)
    {
        p_dsmpls->outer_editptr = 1; /*martini_encap_valid*/
        if (p_nh_param_push->nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN)
        { /*etree leaf*/
            p_dsmpls->outer_editptr_type = 1; /*use_src_leaf*/
        }
        else
        { /*normal CW*/
            p_dsmpls->outer_editptr_type = 0;
        }
    }
    else if (1 == p_nh_param_push->martini_encap_type)
    {
       p_dsmpls->outer_editptr = 1; /*martini_encap_valid*/
    }

    return CTC_E_NONE;
}



STATIC int32
_sys_usw_nh_mpls_build_pw(uint8 lchip, sys_nh_param_mpls_t* p_nh_param,
                                      sys_nh_param_dsnh_t* dsnh,
                                      sys_nh_db_mpls_tunnel_t* p_mpls_tunnel)
{
    sys_dsmpls_t  dsmpls;
    uint8 label_num                = 0;
    uint32 stats_id                = 0;
    uint8 next_is_spme             = 0;
    ctc_mpls_nexthop_push_param_t* p_nh_param_push = NULL;

    sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_nh_param_push  = p_nh_param->is_working? (&p_nh_param->p_mpls_nh_param->nh_para.nh_param_push) :
        (&p_nh_param->p_mpls_nh_param->nh_p_para.nh_p_param_push);

    label_num = p_nh_param_push->label_num;

    if (label_num > 1 || (label_num && p_mpls_tunnel && CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR)))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " pw can only support 1 label number, label number exceed \n");
		return CTC_E_NOT_SUPPORT;
    }

    if (label_num == 0)
    {
        return CTC_E_NONE;
    }

    if (p_nh_param_push->stats_valid)
    {
        stats_id = p_nh_param_push->stats_id;
    }

    next_is_spme = p_mpls_tunnel?CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME):0;
    CTC_ERROR_RETURN(
    _sys_usw_nh_mpls_encode_dsmpls(lchip, &p_nh_param_push->push_label[0], &dsmpls,
                                          stats_id, CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW));

    CTC_ERROR_RETURN(_sys_usw_nh_mpls_encode_cw(lchip, p_nh_param_push, &dsmpls));


    if (p_nh_param_push->nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN)
    { /*dscp use L2header's cos or L3header's dscp */
        dsmpls.src_dscp_type = 1;   /*if set to 1, L2header's cos*/
    }

    if (dsnh->hvpls)
    {
        dsmpls.logic_dest_port = (dsnh->logic_port & 0x3FFF);
    }


    /*1. Allocate new dsl3edit mpls entry*/
    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip,
                                                   SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1,
                                                   &dsnh->l3edit_ptr));

    if (next_is_spme)
    {
        dsmpls.next_editptr_valid = 1;
        dsmpls.outer_editptr = p_mpls_tunnel->lsp_offset[0];
        dsmpls.outer_editptr_type = 1;
    }

    if (p_nh_param->is_working)
    {
        dsmpls.discard = p_nh_param->oam_lock_en?1:0;
        dsmpls.discard_type = p_nh_param->oam_lock_en?1:0;
        dsmpls.mpls_oam_index = p_nh_param->oam_lm_en?p_nh_param->oam_mep_index:0;
    }
    else
    {
        dsmpls.discard = p_nh_param->p_oam_lock_en?1:0;
        dsmpls.discard_type = p_nh_param->p_oam_lock_en?1:0;
        dsmpls.mpls_oam_index = p_nh_param->p_oam_lm_en?p_nh_param->p_oam_mep_index:0;
    }

    /*2. Write HW table*/
    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                       SYS_NH_ENTRY_TYPE_L3EDIT_MPLS,
                                                       dsnh->l3edit_ptr, &dsmpls));
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_nh_mpls_build_dsl2edit(uint8 lchip, uint8  protection_spme,
                                       uint8 proecting_lsp,
                                       ctc_mpls_nexthop_tunnel_info_t* p_nh_param,
                                       sys_nh_db_mpls_tunnel_t* p_mpls_tunnel)
{
    sys_nh_db_dsl2editeth4w_t   dsl2edit_4w;
    sys_nh_db_dsl2editeth4w_t*  p_dsl2edit_4w  = &dsl2edit_4w;
    sys_nh_db_dsl2editeth8w_t   dsl2edit_8w;
    sys_nh_db_dsl2editeth8w_t*  p_dsl2edit_8w  = &dsl2edit_8w;
	sys_l3if_prop_t l3if_prop;
    uint32 offset = 0;
    mac_addr_t mac = {0};
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "protection_spme:%d, proecting_lsp:%d\n",
                   protection_spme, proecting_lsp);

    CTC_PTR_VALID_CHECK(p_mpls_tunnel);
    sal_memset(&l3if_prop, 0, sizeof(l3if_prop));

    if (!CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
    {
        if (!(CTC_IS_CPU_PORT(p_nh_param->oif.gport) || p_nh_param->oif.is_l2_port ||
                        SYS_IS_DROP_PORT(p_nh_param->oif.gport)) && !p_nh_param->loop_nhid)
        {
            CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_nh_param->oif.gport,
                                                                 p_nh_param->oif.vid, &l3if_prop));
        }
        else
        {
            /*SYSTEM MODIFIED by yangl for openflow 2019-08-27*/
            l3if_prop.l3if_id = 1021;
            l3if_prop.vlan_ptr = 5117; /*MAX_CTC_L3IF_ID -1 + 4096*/ 
        }
    }

    if (p_nh_param->loop_nhid)
    {
        DsL2Edit6WOuter_m dsl2edit6w;
        DsL2EditLoopback_m dsl2edit;
        sys_nh_info_dsnh_t nhinfo;

        sal_memset(&dsl2edit6w, 0, sizeof(dsl2edit6w));
        sal_memset(&dsl2edit, 0, sizeof(dsl2edit));
        sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
        CTC_ERROR_RETURN(_sys_usw_nh_get_nhinfo(lchip, p_nh_param->loop_nhid, &nhinfo));
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, &offset));
        CTC_ERROR_GOTO(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, offset, &dsl2edit6w), ret, error);
        SetDsL2EditLoopback(V, dsType_f, &dsl2edit,  SYS_NH_DS_TYPE_L2EDIT);
        SetDsL2EditLoopback(V, l2RewriteType_f,  &dsl2edit,  SYS_NH_L2EDIT_TYPE_LOOPBACK);
        SetDsL2EditLoopback(V, lbNextHopPtr_f, &dsl2edit,  (nhinfo.dsnh_offset&(~SYS_NH_DSNH_BY_PASS_FLAG)));
        SetDsL2EditLoopback(V, lbDestMap_f, &dsl2edit,  nhinfo.dest_map);
        if (offset % 2 == 0)
        {
            sal_memcpy((uint8*)&dsl2edit6w, &dsl2edit, sizeof(dsl2edit));
        }
        else
        {
            sal_memcpy((uint8*)&dsl2edit6w + sizeof(dsl2edit), &dsl2edit, sizeof(dsl2edit));
        }
        CTC_ERROR_GOTO(sys_usw_nh_set_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, offset/2, &dsl2edit6w), ret, error);
        p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme] = offset;
        p_mpls_tunnel->loop_nhid[proecting_lsp][protection_spme]        = p_nh_param->loop_nhid;
    }
    else if (p_nh_param->arp_id)
    {
        sys_nh_db_arp_t *p_arp = NULL;
        sys_nh_info_arp_param_t arp_parm;

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_nh_param->arp_id:%d\n", p_nh_param->arp_id);

        if (p_mpls_tunnel && (p_mpls_tunnel->arp_id[proecting_lsp][protection_spme] != p_nh_param->arp_id))
        {
            sal_memset(&arp_parm, 0, sizeof(sys_nh_info_arp_param_t));
            arp_parm.nh_entry_type  = SYS_NH_TYPE_MPLS;

            CTC_ERROR_RETURN(sys_usw_nh_bind_arp_cb(lchip, &arp_parm, p_nh_param->arp_id));
            p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_nh_param->arp_id));
            if (NULL == p_arp)
            {
                return CTC_E_NOT_EXIST;
            }

            p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme] = p_arp->offset;
            p_mpls_tunnel->arp_id[proecting_lsp][protection_spme]        = p_nh_param->arp_id;
        }

    }
    else if(0 == sal_memcmp(p_nh_param->mac_sa, mac, sizeof(mac_addr_t)))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_nh_param->mac:%s\n", sys_output_mac(p_nh_param->mac));
        sal_memset(&dsl2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));

        if (p_nh_param->oif.vid > 0 && p_nh_param->oif.vid <= CTC_MAX_VLAN_ID)
        {
            dsl2edit_4w.output_vid = p_nh_param->oif.vid;
            dsl2edit_4w.output_vlan_is_svlan = 1;
        }

        sal_memcpy(dsl2edit_4w.mac_da, p_nh_param->mac, sizeof(mac_addr_t));

        CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_dsl2edit_4w, &offset));

        p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme] = offset;
        p_mpls_tunnel->p_dsl2edit_4w[proecting_lsp][protection_spme] = p_dsl2edit_4w;
    }
    else
    {
        /*edit mac sa, must use l2edit8w, for openflow*/
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_nh_param->mac:%s\n", sys_output_mac(p_nh_param->mac));
        sal_memset(&dsl2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));

        if (p_nh_param->oif.vid > 0 && p_nh_param->oif.vid <= CTC_MAX_VLAN_ID)
        {
            dsl2edit_8w.output_vid = p_nh_param->oif.vid;
            dsl2edit_8w.output_vlan_is_svlan = 1;
        }

        sal_memcpy(dsl2edit_8w.mac_da, p_nh_param->mac, sizeof(mac_addr_t));
        sal_memcpy(dsl2edit_8w.mac_sa, p_nh_param->mac_sa, sizeof(mac_addr_t));
        dsl2edit_8w.update_mac_sa = 1;

        CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_dsl2edit_8w, &offset));

        CTC_SET_FLAG(p_mpls_tunnel->flag , SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W);
        p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme] = offset;
        p_mpls_tunnel->p_dsl2edit_4w[proecting_lsp][protection_spme] = (sys_nh_db_dsl2editeth4w_t*)p_dsl2edit_8w;
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit_offset[%d][%d]:%d\n",
                   proecting_lsp, protection_spme,
                   p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme]);

    return CTC_E_NONE;

error:
    if (p_nh_param->loop_nhid)
    {
        if (offset)
        {
            sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, offset);
        }
    }

    return ret;

}

STATIC int32
_sys_usw_nh_mpls_build_inner_dsl2edit(uint8 lchip,ctc_mpls_nexthop_push_param_t* p_nh_param_push,
                                                            sys_nh_param_dsnh_t* dsnh,
                                                            sys_nh_info_mpls_t* p_nhdb)
{
    sys_nh_db_dsl2editeth8w_t* p_dsl2edit6w = NULL;
    sys_nh_db_dsl2editeth8w_t dsl2edit6w;

    sal_memset(&dsl2edit6w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));

    sal_memcpy(dsl2edit6w.mac_da, p_nh_param_push->nh_com.mac, sizeof(mac_addr_t));
    sal_memcpy(dsl2edit6w.mac_sa, p_nh_param_push->nh_com.mac_sa, sizeof(mac_addr_t));
    dsl2edit6w.update_mac_sa = 1;

    if (CTC_VLAN_EGRESS_EDIT_INSERT_VLAN == p_nh_param_push->nh_com.vlan_info.svlan_edit_type)
    {
        dsl2edit6w.output_vid           = p_nh_param_push->nh_com.vlan_info.output_svid;
        dsl2edit6w.output_vlan_is_svlan = 1;
    }
    p_dsl2edit6w = &dsl2edit6w;
    CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_6w_inner(lchip, (sys_nh_db_dsl2editeth8w_t**)&p_dsl2edit6w, &dsnh->inner_l2edit_ptr));
    CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags , SYS_NH_INFO_FLAG_HAVE_L2EDI_6W);
    p_nhdb->p_dsl2edit_inner = p_dsl2edit6w;
    p_nhdb->inner_l2_ptr = dsnh->inner_l2edit_ptr;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_nh_push_multi_label(uint8 lchip, uint8 protect_lsp, ctc_mpls_nexthop_tunnel_info_t* p_tunnel_info,
                                           sys_nh_db_mpls_tunnel_t*  p_mpls_tunnel)
{
    sys_dsmpls_t  dsmpls;
    uint8 index = 0;
    uint32 offset = 0;
    uint8 is_sbit = 0;
    ctc_mpls_nh_label_param_t *p_label = NULL;

    sal_memset(&dsmpls , 0, sizeof(sys_dsmpls_t));

    offset = p_mpls_tunnel->lsp_offset[protect_lsp];
    if (offset == 0)
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip,
                                                        SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W, 1,
                                                        &offset));
    }

    dsmpls.next_editptr_valid = 1;
    dsmpls.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_MPLS_12W;
    dsmpls.label_num = (p_tunnel_info->label_num<=10)?p_tunnel_info->label_num:10;

    if (p_mpls_tunnel->sr[protect_lsp])
    {
        dsmpls.outer_editptr = p_mpls_tunnel->sr[protect_lsp][0].l2edit_offset;
        dsmpls.outer_editptr_type = 0;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Exsit SR Tunnel, l2edit_offset:%u\n",
                       dsmpls.outer_editptr);
    }
    else
    {
        dsmpls.outer_editptr = p_mpls_tunnel->l2edit_offset[protect_lsp][0];
        dsmpls.outer_editptr_type = 0;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Direct l2edit_offset:%u\n",
                       p_mpls_tunnel->l2edit_offset[protect_lsp][0]);
    }

    p_mpls_tunnel->lsp_offset[protect_lsp] = offset;

    for (index = 0; index < p_tunnel_info->label_num; index++)
    {
        if (index >= 10)
        {
            continue;
        }
        p_label = &(p_tunnel_info->tunnel_label[index]);
        is_sbit = index?0:1;
        dsmpls.label_full[index] = ((p_label->label<<12) | (p_label->exp<<9) | (is_sbit<<8) | (p_label->ttl));

    }

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W, offset, &dsmpls));

    CTC_SET_FLAG(p_mpls_tunnel->flag , SYS_NH_MPLS_TUNNEL_FLAG_IS_SR);

    return CTC_E_NONE;
}

STATIC int32
  _sys_usw_nh_mpls_build_sr_tunnel(uint8 lchip, uint8 protect_lsp, ctc_mpls_nexthop_tunnel_info_t* p_tunnel_info,
                                     sys_nh_db_mpls_tunnel_t* p_mpls_tunnel, uint8 is_add)
{
    int32 ret = CTC_E_NONE;
    sys_dsmpls_t  dsmpls;
    DsL2Edit6WOuter_m dsl2edit6w;
    DsL2EditLoopback_m dsl2edit;
    sys_nh_param_dsnh_t   dsnh_param;
    int8 sr_label_num = 0;
    uint8 loop_num = 0;
    uint8 have_lsp = 0;
    uint8 have_spme = 0;
    int16 i = 0;
    uint32 dest_map = 0;
    uint8 is_last_edit = 0;
    sys_l3if_prop_t                l3if_prop;
    sys_nh_mpls_sr_t* p_sr_tunnel = NULL;
    uint8 max_label = DRV_IS_DUET2(lchip)?2:10;
    uint8 entry_type = DRV_IS_DUET2(lchip)? SYS_NH_ENTRY_TYPE_L3EDIT_MPLS:SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sr_label_num = p_tunnel_info->label_num - max_label ;/*not include the first and the second lable, the two label processed by _sys_usw_nh_mpls_build_tunnel*/
    loop_num = DRV_IS_DUET2(lchip)?((sr_label_num + 2)/3):1;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " label_num = %d, sr_label_num = %d, loop_num:%d\n",
                   p_tunnel_info->label_num,   sr_label_num, loop_num);
    if (is_add)
    {
        p_mpls_tunnel->sr[protect_lsp] = (sys_nh_mpls_sr_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_mpls_sr_t) *loop_num);
        if (!p_mpls_tunnel->sr[protect_lsp])
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_mpls_tunnel->sr[protect_lsp], 0, sizeof(sys_nh_mpls_sr_t) *loop_num);
    }
    p_sr_tunnel = p_mpls_tunnel->sr[protect_lsp];
    if (!p_sr_tunnel)
    {
        return CTC_E_INVALID_PARAM;
    }
    /* get destination */
    sal_memset(&l3if_prop , 0, sizeof(sys_l3if_prop_t));
    if (!(CTC_IS_CPU_PORT(p_tunnel_info->oif.gport) || p_tunnel_info->oif.is_l2_port ||
            SYS_IS_DROP_PORT(p_tunnel_info->oif.gport)) && !p_tunnel_info->loop_nhid)
    {
        CTC_ERROR_GOTO(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_tunnel_info->oif.gport, p_tunnel_info->oif.vid, &l3if_prop), ret, error);
    }
    if (p_tunnel_info->arp_id)
    {
        sys_nh_db_arp_t*  p_arp = NULL;
        p_arp = sys_usw_nh_lkup_arp_id(lchip, p_tunnel_info->arp_id);
        if (!p_arp)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
            return CTC_E_NOT_EXIST;
        }
        if (p_arp->destmap_profile)
        {
            dest_map = SYS_ENCODE_ARP_DESTMAP(p_arp->destmap_profile);
        }
    }
    if (0 == dest_map)
    {
        dest_map = SYS_ENCODE_DESTMAP(CTC_MAP_GPORT_TO_GCHIP(p_tunnel_info->oif.gport), CTC_MAP_GPORT_TO_LPORT(p_tunnel_info->oif.gport));
    }

    for (i = loop_num - 1; i >= 0; i--)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "loop i = %d\n", i);
        is_last_edit = (i == loop_num - 1)? 1 : 0;
        have_lsp = 1;
        have_spme = 1;
        if (is_last_edit)/*the last edit*/
        {
            if (2 == (sr_label_num % 3))
            {
                have_spme = 0;
            }
            else if(1 == (sr_label_num % 3))
            {
                have_lsp = 0;
                have_spme = 0;
            }
        }

        if (!DRV_IS_DUET2(lchip))
        {
            have_lsp = 0;
            have_spme = 0;
        }

        /*spme, outer pipeline1*/
        if (have_spme)
        {
            if (is_add)
            {
                CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, &p_sr_tunnel[i].spme_offset), ret, error);
            }
            sal_memset(&dsmpls , 0, sizeof(sys_dsmpls_t));
            CTC_ERROR_GOTO(_sys_usw_nh_mpls_encode_dsmpls(lchip, &p_tunnel_info->tunnel_label[(i + 1)*3 + 1], &dsmpls, 0, 0), ret, error);
            dsmpls.outer_editptr = is_last_edit? p_mpls_tunnel->l2edit_offset[0][0] : p_sr_tunnel[i + 1].l2edit_offset;
            dsmpls.next_editptr_valid = 1;
            dsmpls.outer_editptr_type = 0;
            CTC_ERROR_GOTO(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, p_sr_tunnel[i].spme_offset, &dsmpls), ret, error);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spme offset = %d, outer_editptr = %d\n", p_sr_tunnel[i].spme_offset, dsmpls.outer_editptr);
        }
        /*lsp, outer pipeline0*/
        if (have_lsp)
        {
            if (is_add)
            {
                CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, &p_sr_tunnel[i].lsp_offset), ret, error);
            }
            sal_memset(&dsmpls , 0, sizeof(sys_dsmpls_t));
            CTC_ERROR_GOTO(_sys_usw_nh_mpls_encode_dsmpls(lchip, &p_tunnel_info->tunnel_label[(i + 1)*3], &dsmpls, 0, 0), ret, error);
            dsmpls.next_editptr_valid = 1;
            if (have_spme)
            {
                dsmpls.outer_editptr = p_sr_tunnel[i].spme_offset;
                dsmpls.outer_editptr_type = 1;
            }
            else
            {
                dsmpls.outer_editptr = is_last_edit? p_mpls_tunnel->l2edit_offset[0][0] : p_sr_tunnel[i + 1].l2edit_offset;
                dsmpls.outer_editptr_type = 0;
            }
            CTC_ERROR_GOTO(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, p_sr_tunnel[i].lsp_offset, &dsmpls), ret, error);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lsp offset = %d, outer_editptr = %d\n", p_sr_tunnel[i].lsp_offset, dsmpls.outer_editptr);
        }
        /*pw, inner*/
        if (is_add)
        {
            CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, entry_type, 1, &p_sr_tunnel[i].pw_offset), ret, error);
        }
        sal_memset(&dsmpls , 0, sizeof(sys_dsmpls_t));
        if (entry_type == SYS_NH_ENTRY_TYPE_L3EDIT_MPLS)
        {
            CTC_ERROR_GOTO(_sys_usw_nh_mpls_encode_dsmpls(lchip, &p_tunnel_info->tunnel_label[(i + 1)*3 - 1], &dsmpls, 0, 0), ret, error);
        }
        else
        {
            uint8 index = 0;
            ctc_mpls_nh_label_param_t *p_label = NULL;

            dsmpls.next_editptr_valid = 1;
            dsmpls.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_MPLS_12W;
            dsmpls.label_num = p_tunnel_info->label_num-10;

            dsmpls.outer_editptr = p_mpls_tunnel->l2edit_offset[0][0];
            dsmpls.outer_editptr_type = 0;

            for (index = 10; index < p_tunnel_info->label_num; index++)
            {
                p_label = &(p_tunnel_info->tunnel_label[index]);
                dsmpls.label_full[index-10] = ((p_label->label<<12) | (p_label->exp<<9) | (0<<8) | (p_label->ttl));

            }
        }
        CTC_ERROR_GOTO(sys_usw_nh_write_asic_table(lchip, entry_type, p_sr_tunnel[i].pw_offset, &dsmpls), ret, error);

        /*dsnexthop*/
        if (is_add)
        {
            CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1, &p_sr_tunnel[i].dsnh_offset), ret, error);
        }
        sal_memset(&dsnh_param , 0, sizeof(sys_nh_param_dsnh_t));
        dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_NONE;
        dsnh_param.dsnh_offset = p_sr_tunnel[i].dsnh_offset;
        if (have_lsp||have_spme)
        {
            dsnh_param.lspedit_ptr = p_sr_tunnel[i].lsp_offset;
        }
        else
        {
            dsnh_param.l2edit_ptr = is_last_edit? p_mpls_tunnel->l2edit_offset[0][0] : p_sr_tunnel[i + 1].l2edit_offset;
        }
        dsnh_param.dest_vlan_ptr = is_last_edit? l3if_prop.vlan_ptr : 0;
        dsnh_param.l3edit_ptr = p_sr_tunnel[i].pw_offset;
        CTC_ERROR_GOTO(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param), ret, error);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dsnexthop offset = %d, lspedit_ptr = %d, l2edit_ptr = %d\n",
                                                                 dsnh_param.dsnh_offset, dsnh_param.lspedit_ptr, dsnh_param.l2edit_ptr);
        /*l2 edit, outer pipelin2, note: this edit is the previous l2loopback which eloop to the current edits*/
        if (is_add)
        {
            CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, &p_sr_tunnel[i].l2edit_offset), ret, error);
        }
        sal_memset(&dsl2edit6w, 0, sizeof(dsl2edit6w));
        sal_memset(&dsl2edit, 0, sizeof(dsl2edit));
        CTC_ERROR_GOTO(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W,
                                                        p_sr_tunnel[i].l2edit_offset, &dsl2edit6w), ret, error);
        SetDsL2EditLoopback(V, dsType_f, &dsl2edit,  SYS_NH_DS_TYPE_L2EDIT);
        SetDsL2EditLoopback(V, l2RewriteType_f,  &dsl2edit,  SYS_NH_L2EDIT_TYPE_LOOPBACK);
        SetDsL2EditLoopback(V, lbNextHopPtr_f, &dsl2edit,  p_sr_tunnel[i].dsnh_offset);
        SetDsL2EditLoopback(V, lbDestMap_f, &dsl2edit,  dest_map);
        if (p_sr_tunnel[i].l2edit_offset % 2 == 0)
        {
            sal_memcpy((uint8*)&dsl2edit6w, &dsl2edit, sizeof(dsl2edit));
        }
        else
        {
            sal_memcpy((uint8*)&dsl2edit6w + sizeof(dsl2edit), &dsl2edit, sizeof(dsl2edit));
        }
        CTC_ERROR_GOTO(sys_usw_nh_set_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W,
                                                        p_sr_tunnel[i].l2edit_offset/2, &dsl2edit6w), ret, error);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "loopback offset = %d\n", p_sr_tunnel[i].l2edit_offset);
    }
    p_mpls_tunnel->sr_loop_num[protect_lsp] = loop_num;
    return CTC_E_NONE;
error:
    if (is_add)
    {
        for (i = loop_num - 1; i >= 0; i--)
        {
            if (p_sr_tunnel[i].l2edit_offset)
            {
                sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_sr_tunnel[i].l2edit_offset);
            }
            if (p_sr_tunnel[i].spme_offset)
            {
                sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, p_sr_tunnel[i].spme_offset);
            }
            if (p_sr_tunnel[i].lsp_offset)
            {
                sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_sr_tunnel[i].lsp_offset);
            }
            sys_usw_nh_offset_free(lchip, entry_type, 1, p_sr_tunnel[i].pw_offset);
            sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1, p_sr_tunnel[i].dsnh_offset);
        }
        mem_free(p_mpls_tunnel->sr[protect_lsp]);
    }
    return ret;
}
STATIC int32
_sys_usw_nh_mpls_build_tunnel(uint8 lchip, uint8  protection_spme,
                                     uint8 proecting_lsp,
                                     uint8 label_type,
                                     ctc_mpls_nexthop_tunnel_info_t* p_tunnel_info,
                                     sys_nh_db_mpls_tunnel_t* p_mpls_tunnel)
{
    sys_dsmpls_t  dsmpls;
    uint8 block_size               = 0;
    uint8 table_type               = 0;
    uint32 offset                  = 0;
    uint32 stats_id                = 0;
    int32 ret                      = CTC_E_NONE;
    ctc_mpls_nh_label_param_t* p_label = 0;
    /*sys_dsmpls_t*  p_dsmpls;*/
    uint8 use_spool = 0;
    sys_nh_update_oam_info_t oam_info;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " proecting_lsp:%d,protection_spme:%d, label_type:%d\n",
                  proecting_lsp,   protection_spme,label_type);


    block_size = 1;
    if (label_type == SYS_NH_MPLS_LABLE_LSP)
    {
        if(CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS)
            &&  protection_spme)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "2 Level spme protecting path!!\n");
            return CTC_E_NONE;
        }
        p_label = &(p_tunnel_info->tunnel_label[0]);
        if (p_tunnel_info->stats_valid)
        {
            stats_id = p_tunnel_info->stats_id;
        }
        if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
        {
            table_type = SYS_NH_ENTRY_TYPE_L3EDIT_SPME;
        }
        else
        {
            table_type = SYS_NH_ENTRY_TYPE_L3EDIT_MPLS;
        }
        offset = p_mpls_tunnel->lsp_offset[proecting_lsp];
    }
    else
    {
        p_label = &(p_tunnel_info->tunnel_label[1]);
        table_type = SYS_NH_ENTRY_TYPE_L3EDIT_SPME;
        if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
        {
            block_size = 2;
            use_spool = 0;
        }
        if(protection_spme)
        {
            if (p_mpls_tunnel->spme_offset[proecting_lsp][0] == 0)
            {
                return CTC_E_INVALID_PARAM;
            }
            offset = p_mpls_tunnel->spme_offset[proecting_lsp][0] + 1;
        }
        else
        {
            offset = p_mpls_tunnel->spme_offset[proecting_lsp][0];
        }
    }

    if ((offset == 0) && (!use_spool))
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip,
                                                        table_type, block_size,
                                                        &offset));
    }
    sal_memset(&dsmpls , 0, sizeof(sys_dsmpls_t));

    ret = _sys_usw_nh_mpls_encode_dsmpls(lchip, p_label, &dsmpls,
                                          stats_id, CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP);

    /*SPME label*/
    if (label_type == SYS_NH_MPLS_LABLE_SPME)
    {
        if (p_mpls_tunnel->sr[proecting_lsp])
        {
            dsmpls.outer_editptr = p_mpls_tunnel->sr[proecting_lsp][0].l2edit_offset;
        }
        else
        {
            dsmpls.outer_editptr = p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme];
        }
        dsmpls.next_editptr_valid = 1;
        dsmpls.outer_editptr_type = 0;

        p_mpls_tunnel->flag  |= SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME;
        p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme] = offset;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spme_offset[%d][%d]:%u outer_editptr:%u\n",
                       proecting_lsp, protection_spme,
                       p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme],dsmpls.outer_editptr);
    }

    /*LSP label*/
    if (label_type == SYS_NH_MPLS_LABLE_LSP)
    {
        dsmpls.next_editptr_valid = 1;

        if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME)&&p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme])
        {
            dsmpls.outer_editptr = p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme];
            dsmpls.outer_editptr_type = 1;
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ExsitSpme, spme_offset[%d][%d]:%u\n",
                           proecting_lsp, protection_spme,
                           p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme]);
        }
        else
        {
            dsmpls.outer_editptr = p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme];
            dsmpls.outer_editptr_type = 0;
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Direct l2edit_offset[%d][%d]:%u\n",
                           proecting_lsp, protection_spme,
                           p_mpls_tunnel->l2edit_offset[proecting_lsp][protection_spme]);
        }

        p_mpls_tunnel->lsp_offset[proecting_lsp] = offset;

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lsp_offset[%d][%d]: %u\n",
                       proecting_lsp, protection_spme,offset);

        /*lsp+spme 2layer aps do not support update oam*/
        if (!CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
        {
            sal_memset(&oam_info, 0, sizeof(oam_info));
            _sys_usw_mpls_nh_get_oam_en(lchip, p_mpls_tunnel, &oam_info, proecting_lsp, 1);
            if (p_tunnel_info->tunnel_label[0].label == oam_info.mpls_label)
            {
                dsmpls.discard = oam_info.lock_en?1:0;
                dsmpls.discard_type= oam_info.lock_en?1:0;
                dsmpls.mpls_oam_index = oam_info.oam_mep_index;
            }
        }

    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "label %d\n", dsmpls.label);

    if (use_spool)
    {
    #if 0
        p_dsmpls = &dsmpls;
        CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_dsmpls, table_type, &offset));
        if ((label_type == SYS_NH_MPLS_LABLE_SPME) ||
            ((label_type == SYS_NH_MPLS_LABLE_LSP)&&CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME)))
        {
            p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme] = offset;
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spme_offset[%d]:%u\n",
                           proecting_lsp, p_mpls_tunnel->spme_offset[proecting_lsp][protection_spme]);
        }
        else
        {
            p_mpls_tunnel->lsp_offset[proecting_lsp] = offset;
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lsp_offset[%d]:%u\n",
                           proecting_lsp, p_mpls_tunnel->lsp_offset[proecting_lsp]);
        }
        p_mpls_tunnel->p_l3_edit = p_dsmpls;
    #endif
    }
    else
    {
        ret = ret?ret:(sys_usw_nh_write_asic_table(lchip, table_type, offset, &dsmpls));
    }

    return ret;

}



STATIC int32
_sys_usw_nh_mpls_build_dsnh(uint8 lchip, sys_nh_param_mpls_t* p_nh_param,
                                   uint32 dsnh_offset,
                                   bool working_path,
                                   sys_nh_info_mpls_t* p_nhdb)
{
    ctc_mpls_nexthop_pop_param_t* p_nh_param_pop = NULL;         /**< mpls (asp) pop nexthop */
    ctc_mpls_nexthop_push_param_t* p_nh_param_push = NULL;      /**< mpls push (asp) nexthop */
    ctc_mpls_nh_op_t               nh_opcode = 0;                /**< MPLS nexthop operation code*/
    sys_l3if_prop_t                l3if_prop;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
	sys_nh_param_dsnh_t   dsnh_param;
    uint8 next_is_spme = 0;
    mac_addr_t mac = {0};
    ctc_mpls_nexthop_param_t* p_nh_mpls_param = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));
    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));

    p_nh_mpls_param = p_nh_param->p_mpls_nh_param;
    p_nh_param->is_working = working_path;

    /* L2Edit & L3Edit  */
    if (p_nh_mpls_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE)
    {
        p_nh_param_push  = working_path ? (&p_nh_mpls_param->nh_para.nh_param_push) :
        (&p_nh_mpls_param->nh_p_para.nh_p_param_push);

        CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, p_nh_param_push->tunnel_id, &p_mpls_tunnel));

        if (p_nh_param_push->nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN
            && (p_nh_mpls_param->logic_port ||
            CTC_FLAG_ISSET(p_nh_mpls_param->flag, CTC_MPLS_NH_IS_HVPLS)))
        {
            CTC_MAX_VALUE_CHECK(p_nh_mpls_param->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
            p_nhdb->dest_logic_port = p_nh_mpls_param->logic_port;
            dsnh_param.logic_port = p_nh_mpls_param->logic_port;
            dsnh_param.hvpls = CTC_FLAG_ISSET(p_nh_mpls_param->flag, CTC_MPLS_NH_IS_HVPLS);

            if (p_nh_mpls_param->service_id && p_nh_mpls_param->logic_port)
            {
                /*Call QoS for binding service with logic Port*/
                CTC_ERROR_RETURN(sys_usw_qos_bind_service_logic_dstport(lchip, p_nh_mpls_param->service_id, p_nh_mpls_param->logic_port));
                p_nhdb->service_id = p_nh_mpls_param->service_id;
            }

            CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_PORT);
            if (!CTC_FLAG_ISSET(p_nh_mpls_param->flag, CTC_MPLS_NH_IS_HVPLS))
            {
                CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN);
            }
        }

        if (!p_mpls_tunnel && !p_nh_param_push->loop_nhid)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Tunnel label not exist\n");
			return CTC_E_NOT_EXIST;

        }

        if (p_nh_mpls_param->aps_en && (p_nh_param_push->label_num != 1))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Need at least one mpls label\n");
			return CTC_E_INVALID_CONFIG;

        }
        dsnh_param.p_vlan_info = &p_nh_param_push->nh_com.vlan_info;
        nh_opcode              = p_nh_param_push->nh_com.opcode;

        if (!p_nh_param_push->loop_nhid && !CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
        {
            l3if_prop.l3if_id      = p_mpls_tunnel->l3ifid;

            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_mpls_tunnel->l3ifid:%d\n", p_mpls_tunnel->l3ifid);

            if (l3if_prop.l3if_id)
            {
                CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop));
                dsnh_param.dest_vlan_ptr  = l3if_prop.vlan_ptr;
            }
        }


    }
    else if (p_nh_mpls_param->nh_prop == CTC_MPLS_NH_POP_TYPE)
    {
        if (!working_path || p_nh_mpls_param->aps_en)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        }

        p_nh_param_pop  = &p_nh_mpls_param->nh_para.nh_param_pop;
        nh_opcode       = p_nh_param_pop->nh_com.opcode;

        if (!(CTC_IS_CPU_PORT(p_nh_param_pop->nh_com.oif.gport) || p_nh_param_pop->nh_com.oif.is_l2_port || SYS_IS_DROP_PORT(p_nh_param_pop->nh_com.oif.gport)))
        {
            CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_nh_param_pop->nh_com.oif.gport, p_nh_param_pop->nh_com.oif.vid, &l3if_prop));
        }
        else
        {
            /*SYSTEM MODIFIED by yangl for openflow 2019-08-27*/
            l3if_prop.l3if_id = 1021;
            l3if_prop.vlan_ptr = 5117; /*MAX_CTC_L3IF_ID -1 + 4096*/
        }

        dsnh_param.dest_vlan_ptr  = l3if_prop.vlan_ptr;
    }

    switch (nh_opcode)
    {
    case CTC_MPLS_NH_PUSH_OP_NONE:
        dsnh_param.dsnh_type  = SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_NONE;
        break;

    case CTC_MPLS_NH_PUSH_OP_ROUTE:
        dsnh_param.dsnh_type  = SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_ROUTE;
        break;

    case  CTC_MPLS_NH_PUSH_OP_L2VPN:
        dsnh_param.dsnh_type  = SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_L2VPN;
        break;

    case CTC_MPLS_NH_PHP:
        dsnh_param.dsnh_type  = SYS_NH_PARAM_DSNH_TYPE_MPLS_PHP;
        break;

    default:
        break;
    }

    if (p_nh_mpls_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE)
    {
        sys_aps_bind_nh_param_t bind_param;
        mac_addr_t mac_zero = {0};
        uint8 do_inner_l2_edit = 0;

        do_inner_l2_edit = sal_memcmp(p_nh_param_push->nh_com.mac, mac_zero, sizeof(mac_addr_t)) ||
            sal_memcmp(p_nh_param_push->nh_com.mac_sa, mac_zero, sizeof(mac_addr_t));
        if (p_nh_param_push->loop_nhid)
        {
            if (do_inner_l2_edit && p_nh_param_push->label_num)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "loop and do inner l2 edit cannot support!\n");
                return CTC_E_INVALID_CONFIG;
            }

            /*add l2edit to loop nh*/
            CTC_ERROR_RETURN(sys_usw_nh_add_loopback_l2edit(lchip, p_nh_param_push->loop_nhid, TRUE,
                    &dsnh_param.l2edit_ptr));
            if(working_path)
            {
                p_nhdb->working_path.loop_nhid = p_nh_param_push->loop_nhid;
            }
            else if(p_nhdb->protection_path)
            {
                p_nhdb->protection_path->loop_nhid = p_nh_param_push->loop_nhid;
            }
            dsnh_param.loop_edit = 1;
        }
        else if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
        {
            next_is_spme = 1;
        }
        else
        {
            if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
            {
                dsnh_param.lspedit_ptr = p_mpls_tunnel->gport;
                CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP);
                CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);

                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "APS(ID)p_mpls_tunnel->gport:%d\n", p_mpls_tunnel->gport);
            }
            else
            {
                dsnh_param.lspedit_ptr = p_mpls_tunnel->lsp_offset[0];
                dsnh_param.l2edit_ptr = p_mpls_tunnel->l2edit_offset[0][0];

                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_mpls_tunnel->lsp_offset:%d\n", p_mpls_tunnel->lsp_offset[0]);
            }
        }

        if (p_nh_param_push->martini_encap_valid
            && (1 == p_nh_param_push->martini_encap_type))
        {
            CTC_ERROR_RETURN(sys_usw_nh_mpls_add_cw(lchip, p_nh_param_push->seq_num_index, &p_nhdb->cw_index, TRUE));
            dsnh_param.cw_index = p_nhdb->cw_index;
        }

        /*inner l2 edit*/
        if (do_inner_l2_edit)
        {
            if (p_nh_param_push->label_num && !next_is_spme && !p_nh_param_push->loop_nhid)
            {
                /*1. If mpls nexthop have pw label(using outer edit pipe0) and inner l2 edit(inned edit), mpls tunnel must using is_spme */
                /*2. For loop nexthop have no mpls tunnel, inner l2 edit using inner edit and pw label using outer edit pipe0 */
                return CTC_E_INVALID_CONFIG;
            }

            CTC_ERROR_RETURN(_sys_usw_nh_mpls_build_inner_dsl2edit(lchip, p_nh_param_push, &dsnh_param, p_nhdb));
        }

        /* _2chips */
        CTC_ERROR_RETURN(_sys_usw_nh_mpls_build_pw(lchip, p_nh_param, &dsnh_param, p_mpls_tunnel));

        if (next_is_spme)
        {
            if (p_nh_param_push->label_num)
            {
                /*
                    1. innner l2 edit
                    2.l3edit
                */
            }
            else
            {
                /*
                    lsp ptr, position jump mode
                */
                dsnh_param.lspedit_ptr = p_mpls_tunnel->lsp_offset[0];
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_mpls_tunnel->lsp_offset:%d\n", p_mpls_tunnel->lsp_offset[0]);
            }
            dsnh_param.spme_mode = 1;
        }


        if (dsnh_param.l3edit_ptr != 0)
        {
            CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
        }

        if (working_path)
        {
            p_nhdb->working_path.dsl3edit_offset  = dsnh_param.l3edit_ptr;
        }
        else
        {
            p_nhdb->protection_path->dsl3edit_offset = dsnh_param.l3edit_ptr;
        }

        /*update aps bridge*/
        sal_memset(&bind_param, 0, sizeof(bind_param));
        bind_param.working_path = working_path;

        if (p_nh_param_push->loop_nhid)
        {
            /*do nothing*/
        }
        else if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
        {
            bind_param.dest_id = p_mpls_tunnel->gport; /*aps group id*/
            bind_param.next_aps_en = 1;

        }
        else
        {
            sys_nh_db_arp_t*  p_arp;
            if (p_mpls_tunnel->arp_id[0][0])
            {
                p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_mpls_tunnel->arp_id[0][0]));
                if (NULL == p_arp)
                {
                    return CTC_E_NOT_EXIST;
                }
                bind_param.dest_id = p_arp->destmap_profile;
			    bind_param.l3if_id  = p_arp->l3if_id;
                bind_param.destmap_profile = 1;
            }
            else
            {
                bind_param.dest_id = p_mpls_tunnel->gport;
			    bind_param.l3if_id  = p_mpls_tunnel->l3ifid;
            }

        }
        if (p_nh_param->p_mpls_nh_param->aps_en)
        {
            bind_param.nh_id = p_nh_param->hdr.nhid ;
            bind_param.different_nh = 1;
            CTC_ERROR_RETURN(sys_usw_aps_bind_nexthop(lchip, p_nh_mpls_param->aps_bridge_group_id, &bind_param));
        }
    }
    else if (p_nh_mpls_param->nh_prop == CTC_MPLS_NH_POP_TYPE)
    {

        if(p_nh_param_pop->arp_id)
        {
            sys_nh_db_arp_t *p_arp = NULL;
            sys_nh_info_arp_param_t arp_parm;

            sal_memset(&arp_parm, 0, sizeof(sys_nh_info_arp_param_t));
            arp_parm.nh_entry_type  = SYS_NH_TYPE_MPLS;
            p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_nh_param_pop->arp_id));
            if (NULL == p_arp)
            {
                return CTC_E_NOT_EXIST;
            }
            CTC_ERROR_RETURN(sys_usw_nh_bind_arp_cb(lchip, &arp_parm, p_nh_param_pop->arp_id));
            dsnh_param.l2edit_ptr   = p_arp->offset;
            p_nhdb->arp_id          =  p_nh_param_pop->arp_id;
        }
        else
        {
            /*SYSTEM MODIFIED by yangl for openflow 2019-08-27*/
            if (0 == sal_memcmp(p_nh_param_pop->nh_com.mac_sa, mac, sizeof(mac_addr_t)))
            {
                sys_nh_db_dsl2editeth4w_t   dsl2edit_4w;
                sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w  = &dsl2edit_4w;

                sal_memset(&dsl2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));

                sal_memcpy(dsl2edit_4w.mac_da, p_nh_param_pop->nh_com.mac, sizeof(mac_addr_t));
                if (p_nh_param_pop->nh_com.oif.vid != 0)
                {
                    dsl2edit_4w.output_vid = p_nh_param_pop->nh_com.oif.vid;
                    dsl2edit_4w.output_vlan_is_svlan = 1;
                }

                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_dsl2edit_4w, &dsnh_param.l2edit_ptr));
                p_nhdb->p_dsl2edit = p_dsl2edit_4w;
                p_nhdb->outer_l2_ptr = dsnh_param.l2edit_ptr;
                CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
            }
            else
            {
                sys_nh_db_dsl2editeth8w_t   dsl2edit_8w;
                sys_nh_db_dsl2editeth8w_t*  p_dsl2edit_8w  = &dsl2edit_8w;
                
                sal_memset(&dsl2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                if (p_nh_param_pop->nh_com.oif.vid != 0)
                {
                    dsl2edit_8w.output_vid = p_nh_param_pop->nh_com.oif.vid;
                    dsl2edit_8w.output_vlan_is_svlan = 1;
                }
                
                sal_memcpy(dsl2edit_8w.mac_da, p_nh_param_pop->nh_com.mac, sizeof(mac_addr_t));
                sal_memcpy(dsl2edit_8w.mac_sa, p_nh_param_pop->nh_com.mac_sa, sizeof(mac_addr_t));
                dsl2edit_8w.update_mac_sa = 1;

                if (CTC_FLAG_ISSET(p_nh_param_pop->nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_L2EDIT_OUTPUT_VID))
                {
                    dsl2edit_8w.output_vid = p_nh_param_pop->nh_com.oif.vid;
                }

                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_dsl2edit_8w, &dsnh_param.l2edit_ptr));

                p_nhdb->p_dsl2edit = (sys_nh_db_dsl2editeth4w_t*)p_dsl2edit_8w;
                p_nhdb->outer_l2_ptr = dsnh_param.l2edit_ptr;
                CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
            }
            /*yangl end*/
            
 
        }

        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH);
        p_nhdb->gport  = p_nh_param_pop->nh_com.oif.gport;
        p_nhdb->l3ifid = l3if_prop.l3if_id;

        if (p_nh_param_pop->ttl_mode == CTC_MPLS_TUNNEL_MODE_PIPE)
        { /*ttl use IP header*/
            CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_USE_TTL_FROM_PKT);
            CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_USE_MAPPED_DSCP);
        }
        else if (p_nh_param_pop->ttl_mode == CTC_MPLS_TUNNEL_MODE_UNIFORM)
        { /*ttl use mpls header*/
            CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_USE_MAPPED_DSCP);
        }
        else  /*short pipe*/
        { /*ttl & dscp use IP header*/
            CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_PHP_SHORT_PIPE);
        }

    }


    if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        dsnh_param.dsnh_offset = working_path ? p_nhdb->hdr.dsfwd_info.dsnh_offset :\
                                                p_nhdb->hdr.dsfwd_info.dsnh_offset + 2;
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param));
    }
    else
    {
        dsnh_param.dsnh_offset = working_path ? p_nhdb->hdr.dsfwd_info.dsnh_offset : \
                                               p_nhdb->hdr.dsfwd_info.dsnh_offset + 1;
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param));
    }


    if (p_nh_mpls_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE && p_mpls_tunnel)
    {
        if (working_path)
        {
            p_nhdb->working_path.p_mpls_tunnel = p_mpls_tunnel;
            p_nhdb->gport  = p_mpls_tunnel->gport;
            p_nhdb->l3ifid = p_mpls_tunnel->l3ifid;
            p_nhdb->arp_id = p_mpls_tunnel->arp_id[0][0];
        }
        else
        {
            p_nhdb->protection_path->p_mpls_tunnel = p_mpls_tunnel;
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_nh_update_tunnel_ref_info(uint8 lchip, sys_nh_info_mpls_t* p_nhinfo, uint16 tunnel_id, uint8 is_add)
{
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    sys_nh_ref_list_node_t* p_curr = NULL;
    sys_nh_ref_list_node_t* p_nh_ref_list_node = NULL;
    sys_nh_ref_list_node_t* p_prev = NULL;
    uint8 find_flag = 0;

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel));
    if (!p_mpls_tunnel)
    {
        return CTC_E_NONE;
    }

    p_curr = p_mpls_tunnel->p_ref_nh_list;

    if (is_add)
    {
        while (p_curr)
        {
            if ((sys_nh_info_mpls_t*)p_curr->p_ref_nhinfo == p_nhinfo)
            {
                find_flag = 1;
                break;
            }

            p_curr = p_curr->p_next;
        }

        if (find_flag == 0)
        {
            p_nh_ref_list_node = (sys_nh_ref_list_node_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_ref_list_node_t));
            if (p_nh_ref_list_node == NULL)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    			return CTC_E_NO_MEMORY;
            }
            p_nh_ref_list_node->p_ref_nhinfo = (sys_nh_info_com_t* )p_nhinfo;
            p_nh_ref_list_node->p_next = p_mpls_tunnel->p_ref_nh_list;

            p_mpls_tunnel->p_ref_nh_list = p_nh_ref_list_node;
        }
        p_mpls_tunnel->ref_cnt++;
    }
    else
    {
        while (p_curr)
        {
            if ((sys_nh_info_mpls_t*)p_curr->p_ref_nhinfo == p_nhinfo)
            {
                if (NULL == p_prev)
                /*Remove first node*/
                {
                   p_mpls_tunnel->p_ref_nh_list = p_curr->p_next;
                }
                else
                {
                    p_prev->p_next = p_curr->p_next;
                }

                mem_free(p_curr);
                break;
            }

            p_prev = p_curr;
            p_curr = p_curr->p_next;
        }
        p_mpls_tunnel->ref_cnt--;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_free_mpls_nh_resource(uint8 lchip, sys_nh_info_mpls_t* p_nhinfo,sys_nh_param_mpls_t *p_nh_param,
    uint8 free_tunnel,uint8 is_del, sys_nh_info_mpls_t* p_new_info)
{

    sys_aps_bind_nh_param_t  bind_nh;
    sal_memset(&bind_nh,0,sizeof(bind_nh));

    bind_nh.nh_id = p_nh_param->hdr.nhid;

    /*1.unbind aps resource*/
     if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
            && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH)
            && p_nhinfo->protection_path
            && (is_del || !p_nh_param->p_mpls_nh_param->aps_en || ( p_nh_param->p_mpls_nh_param->aps_bridge_group_id != p_nhinfo->gport)))
     {

        sys_usw_aps_unbind_nexthop(lchip,p_nhinfo->gport,&bind_nh);
        bind_nh.working_path = 1;
       sys_usw_aps_unbind_nexthop(lchip,p_nhinfo->gport,&bind_nh);

     }
    /*2. Delete this mpls nexthop resource */
    if (p_nhinfo->cw_index != 0)
    {
        CTC_ERROR_RETURN(sys_usw_nh_mpls_add_cw(lchip, 0, &p_nhinfo->cw_index, FALSE));
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
    {
        uint32 offset = 0;
        sys_dsmpls_t dsmpls;

        offset = p_nhinfo->working_path.dsl3edit_offset;
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, offset));
        sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));
        CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS,
                                                            offset, &dsmpls));

        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN)
            && !CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH)
            && p_nhinfo->protection_path)
        {

            offset = p_nhinfo->protection_path->dsl3edit_offset;
            CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, offset));
            sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));
            CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS,
                                                                offset, &dsmpls));
        }
    }

    if  (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH) && p_nhinfo->arp_id)
    {
        sys_usw_nh_unbind_arp_cb(lchip, p_nhinfo->arp_id, 0);
        p_nhinfo->arp_id = 0;
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT)
        && CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH))
    {
        sys_usw_nh_remove_route_l2edit_outer(lchip, p_nhinfo->p_dsl2edit);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W))
    {
        sys_usw_nh_remove_l2edit_6w_inner(lchip, p_nhinfo->p_dsl2edit_inner);
        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W);
    }

    if (p_nhinfo->working_path.loop_nhid)
    {
        sys_usw_nh_remove_loopback_l2edit(lchip, p_nhinfo->working_path.loop_nhid, TRUE);
    }
    if (p_nhinfo->protection_path && p_nhinfo->protection_path->loop_nhid)
    {
        sys_usw_nh_remove_loopback_l2edit(lchip, p_nhinfo->protection_path->loop_nhid, TRUE);
    }

   CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);

   if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT)
    && CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH))
   {
       CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
       CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MPLS_POP_NH);
   }

   if (free_tunnel)
   {
       /*p_new_info only used for mpls nexthop update to free old nexthop info, for delete and other resource rollback, p_new_info should be NULL*/
       sys_nh_info_mpls_t* p_info = NULL;
       if (p_nhinfo->working_path.p_mpls_tunnel && p_nhinfo->working_path.p_mpls_tunnel->ref_cnt)
       {
            p_info = p_new_info?p_new_info:p_nhinfo;
            if (p_new_info && (p_nh_param->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
                (p_nhinfo->working_path.p_mpls_tunnel->tunnel_id == p_nh_param->p_mpls_nh_param->nh_para.nh_param_push.tunnel_id))
            {
                /*mpls nexthop used tunnel id have not changed, just update ref cnt*/
                p_info = NULL;
            }
            _sys_usw_nh_update_tunnel_ref_info(lchip, p_info, p_nhinfo->working_path.p_mpls_tunnel->tunnel_id, 0);
       }

       if (p_nhinfo->protection_path)
       {
           sys_nh_info_mpls_t* p_info = NULL;
           if (p_nhinfo->protection_path->p_mpls_tunnel && p_nhinfo->protection_path->p_mpls_tunnel->ref_cnt)
           {
               p_info = p_new_info?p_new_info:p_nhinfo;
               if (p_new_info && (p_nh_param->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
                    (p_nhinfo->protection_path->p_mpls_tunnel->tunnel_id == p_nh_param->p_mpls_nh_param->nh_p_para.nh_p_param_push.tunnel_id))
               {
                    /*mpls nexthop used tunnel id have not changed, just update ref cnt*/
                    p_info = NULL;
               }
              _sys_usw_nh_update_tunnel_ref_info(lchip, p_info, p_nhinfo->protection_path->p_mpls_tunnel->tunnel_id, 0);
           }

           mem_free(p_nhinfo->protection_path);
       }
   }

   if (p_nhinfo->service_id &&
       p_nhinfo->dest_logic_port)
   {
       /*Call QoS for unbinding service with logic Port*/
       CTC_ERROR_RETURN(sys_usw_qos_unbind_service_logic_dstport(lchip, p_nhinfo->service_id,  p_nhinfo->dest_logic_port));
   }

    return CTC_E_NONE;
}

/* Used to update mpls nexthop when tunnel arp update */
STATIC int32
_sys_usw_nh_mpls_update_arp_cb(uint8 lchip, sys_nh_info_mpls_t* p_nhinfo, sys_nh_db_mpls_tunnel_t* p_tunnel)
{
    sys_l3if_prop_t l3if_prop;
    uint32 nexthop_ptr = 0;
    uint32 tbl_type = 0;
    DsNextHop8W_m dsnh8w;
    sys_nh_db_arp_t* p_arp = NULL;
    uint16 arp_id = p_tunnel->arp_id[0][0];

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mpls nexthop id:%u, arp id:%u, mpls tunnel:%u\n", p_nhinfo->nhid, arp_id, p_tunnel->tunnel_id);

    if (!p_nhinfo || !arp_id)
    {
        return CTC_E_NONE;
    }

    if (!p_nhinfo->working_path.p_mpls_tunnel)
    {
        /*tunnel not exist*/
        return CTC_E_NONE;
    }

    /*2. get arp id info*/
    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
    if (NULL == p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    tbl_type = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?SYS_NH_ENTRY_TYPE_NEXTHOP_8W:SYS_NH_ENTRY_TYPE_NEXTHOP_4W;
    sal_memset(&l3if_prop, 0, sizeof(l3if_prop));

    /*Update destvlan ptr from dsnexthop*/
    if (!p_nhinfo->protection_path || (p_nhinfo->working_path.p_mpls_tunnel == p_tunnel))
    {
        l3if_prop.l3if_id = p_nhinfo->working_path.p_mpls_tunnel->l3ifid;
        if (l3if_prop.l3if_id)
        {
            CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop));
        }

        nexthop_ptr = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, tbl_type, nexthop_ptr, &dsnh8w));
        SetDsNextHop4W(V, destVlanPtr_f,        &dsnh8w,   l3if_prop.vlan_ptr);

        if (!p_tunnel->label_num && !GetDsNextHop4W(V, shareType_f, &dsnh8w))
        {
            /*outer edit ptr come from dsnexthop directly, need update dsnexthop*/
            SetDsNextHop4W(V, outerEditPtr_f,  &dsnh8w,  p_arp->offset);
        }
        CTC_ERROR_RETURN(sys_usw_nh_set_asic_table(lchip, tbl_type, nexthop_ptr, &dsnh8w));
    }

    /*3. check mpls nexthop use aps or not*/
    if (!p_nhinfo->protection_path)
    {
        p_nhinfo->arp_id = arp_id;

        /*Update destmap profile ID*/
        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
        {
            uint32 dsfwd_offset = 0;
            DsFwd_m dsfwd;
            DsFwdHalf_m dsfwd_half;

            dsfwd_offset = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, dsfwd_offset, &dsfwd));
            if (GetDsFwd(V, isHalf_f, &dsfwd))
            {
                if (dsfwd_offset % 2 == 0)
                {
                    GetDsFwdDualHalf(A, g_0_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
                }
                else
                {
                    GetDsFwdDualHalf(A, g_1_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
                }
                SetDsFwdHalf(V, destMap_f, &dsfwd_half,   SYS_ENCODE_ARP_DESTMAP(p_arp->destmap_profile));

                if (dsfwd_offset % 2 == 0)
                {
                    SetDsFwdDualHalf(A, g_0_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
                }
                else
                {
                    SetDsFwdDualHalf(A, g_1_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
                }
            }
            else
            {
                SetDsFwd(V, destMap_f, &dsfwd, SYS_ENCODE_ARP_DESTMAP(p_arp->destmap_profile));
            }

            CTC_ERROR_RETURN(sys_usw_nh_set_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, dsfwd_offset/2, &dsfwd));

        }
        else
        {
            /*using cb to update fwd table info*/
        	if (NULL != p_nhinfo->updateAd)
            {
                sys_nh_info_dsnh_t dsnh_info;
                sal_memset(&dsnh_info, 0, sizeof(sys_nh_info_dsnh_t));
                _sys_usw_nh_get_nhinfo(lchip, p_nhinfo->nhid, &dsnh_info);
                CTC_ERROR_RETURN(p_nhinfo->updateAd(lchip, p_nhinfo->data, &dsnh_info));
            }
        }
    }
    else
    {
        sys_aps_bind_nh_param_t bind_param;

        sal_memset(&bind_param, 0, sizeof(bind_param));
        if (p_nhinfo->working_path.p_mpls_tunnel == p_tunnel)
        {
            bind_param.working_path = 1;
            bind_param.dest_id = p_arp->destmap_profile;
    	    bind_param.l3if_id  = p_arp->l3if_id;
            bind_param.nh_id = p_nhinfo->nhid ;
            bind_param.different_nh = 1;
            bind_param.destmap_profile = 1;
            CTC_ERROR_RETURN(sys_usw_aps_bind_nexthop(lchip,p_nhinfo->aps_group_id, &bind_param));
        }

        if (p_nhinfo->protection_path->p_mpls_tunnel == p_tunnel)
        {
            l3if_prop.l3if_id = p_nhinfo->protection_path->p_mpls_tunnel->l3ifid;
            if (l3if_prop.l3if_id)
            {
                CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop));
            }

            nexthop_ptr = p_nhinfo->hdr.dsfwd_info.dsnh_offset+((tbl_type==SYS_NH_ENTRY_TYPE_NEXTHOP_8W)?2:1);
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, tbl_type, nexthop_ptr, &dsnh8w));
            SetDsNextHop4W(V, destVlanPtr_f,        &dsnh8w,   l3if_prop.vlan_ptr);
            if (!p_tunnel->label_num && !GetDsNextHop4W(V, shareType_f, &dsnh8w))
            {
                /*outer edit ptr come from dsnexthop directly, need update dsnexthop*/
                SetDsNextHop4W(V, outerEditPtr_f,  &dsnh8w,  p_arp->offset);
            }
            CTC_ERROR_RETURN(sys_usw_nh_set_asic_table(lchip, tbl_type, nexthop_ptr, &dsnh8w));

            bind_param.working_path = 0;
            bind_param.dest_id = p_arp->destmap_profile;
    	    bind_param.l3if_id  = p_arp->l3if_id;
            bind_param.nh_id = p_nhinfo->nhid ;
            bind_param.different_nh = 1;
            bind_param.destmap_profile = 1;
            CTC_ERROR_RETURN(sys_usw_aps_bind_nexthop(lchip,p_nhinfo->aps_group_id, &bind_param));
        }
    }

	/*update oam mep*/
    _sys_usw_nh_update_oam_mep(lchip, p_nhinfo->p_ref_oam_list, arp_id);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_mpls_add_dsfwd(uint8 lchip, sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_dsfwd_t dsfwd_param;
    sys_nh_info_mpls_t* p_nhdb = (sys_nh_info_mpls_t*)(p_com_db);
    int ret = 0;

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    dsfwd_param.is_mcast = FALSE;
    if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        dsfwd_param.nexthop_ext = 1;
    }

    if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
    {
        sys_usw_get_gchip_id(lchip, &dsfwd_param.dest_chipid);
        dsfwd_param.dest_id = p_nhdb->aps_group_id & 0x3FF;
        dsfwd_param.aps_type = CTC_APS_BRIDGE;
    }
    else if (p_nhdb->arp_id)
    {
        sys_nh_db_arp_t *p_arp = NULL;
        p_arp = (sys_usw_nh_lkup_arp_id(lchip, p_nhdb->arp_id));
        if (NULL == p_arp)
        {
            return CTC_E_NOT_EXIST;
        }

        if ((p_arp) && CTC_FLAG_ISSET(p_arp->flag,SYS_NH_ARP_PORT_VALID) && p_arp->destmap_profile)
        {
            dsfwd_param.dest_id = p_arp->destmap_profile;
            dsfwd_param.use_destmap_profile = 1;
        }
    }
    else
    {
        dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nhdb->gport);
        dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nhdb->gport);
    }

    if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LEN_ADJUST_EN))
    {
        sys_usw_lkup_adjust_len_index(lchip, p_nhdb->hdr.adjust_len, &dsfwd_param.adjust_len_idx);
    }

    /*Build DsFwd Table*/
    if (!CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        ret = sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                             &(p_nhdb->hdr.dsfwd_info.dsfwd_offset));
    }

    dsfwd_param.dsfwd_offset = p_nhdb->hdr.dsfwd_info.dsfwd_offset;
    dsfwd_param.dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
    dsfwd_param.drop_pkt = CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
    dsfwd_param.is_egress_edit =  CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsFwd :: DestChipId = %d, DestId = %d"
                                         "DsNexthop Offset = %d, DsNexthopExt = %d\n",
                                          dsfwd_param.dest_chipid, dsfwd_param.dest_id,
                                          dsfwd_param.dsnh_offset, dsfwd_param.nexthop_ext);
    /*Write table*/
    ret = ret ? ret : sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param);

    if (ret == CTC_E_NONE)
    {
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    }
    else
    {
        sys_usw_nh_offset_free(lchip,  SYS_NH_ENTRY_TYPE_FWD, 1, p_nhdb->hdr.dsfwd_info.dsfwd_offset);

    }

    return ret;
}

int32
sys_usw_nh_create_mpls_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_mpls_t* p_nh_param;
    sys_nh_info_mpls_t* p_nhdb;
    int32 ret = 0;
    uint8 index = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);

    CTC_EQUAL_CHECK(SYS_NH_TYPE_MPLS, p_com_nh_para->hdr.nh_param_type);
    p_nh_param = (sys_nh_param_mpls_t*)(p_com_nh_para);
    p_nhdb = (sys_nh_info_mpls_t*)(p_com_db);
    p_nhdb->nhid = p_nh_param->hdr.nhid;

    CTC_PTR_VALID_CHECK(p_nh_param->p_mpls_nh_param);

    if (p_nh_param->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE)
    {
        for (index = 0; index < CTC_MPLS_NH_MAX_PUSH_LABEL_NUM; index++)
        {
            SYS_NH_CHECK_MPLS_LABVEL_VALUE(p_nh_param->p_mpls_nh_param->nh_para.nh_param_push.push_label[index].label);
            SYS_NH_CHECK_MPLS_EXP_VALUE(p_nh_param->p_mpls_nh_param->nh_para.nh_param_push.push_label[index].exp);
            SYS_NH_CHECK_MPLS_EXP_TYPE_VALUE(p_nh_param->p_mpls_nh_param->nh_para.nh_param_push.push_label[index].exp_type);
            CTC_MAX_VALUE_CHECK(p_nh_param->p_mpls_nh_param->nh_para.nh_param_push.push_label[index].exp_domain, 15);
        }

        if (p_nh_param->p_mpls_nh_param->aps_en)
        {
            for (index = 0; index < CTC_MPLS_NH_MAX_PUSH_LABEL_NUM; index++)
            {
                SYS_NH_CHECK_MPLS_LABVEL_VALUE(p_nh_param->p_mpls_nh_param->nh_p_para.nh_p_param_push.push_label[index].label);
                SYS_NH_CHECK_MPLS_EXP_VALUE(p_nh_param->p_mpls_nh_param->nh_p_para.nh_p_param_push.push_label[index].exp);
                SYS_NH_CHECK_MPLS_EXP_TYPE_VALUE(p_nh_param->p_mpls_nh_param->nh_p_para.nh_p_param_push.push_label[index].exp_type);
                CTC_MAX_VALUE_CHECK(p_nh_param->p_mpls_nh_param->nh_p_para.nh_p_param_push.push_label[index].exp_domain, 15);
            }
        }
    }


    if (p_nh_param->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_POP_TYPE)
    {
        uint16 arp_id = 0;
        ctc_mpls_nexthop_pop_param_t* p_nh_pop = NULL;

        p_nh_pop = &p_nh_param->p_mpls_nh_param->nh_para.nh_param_pop;
        arp_id = p_nh_pop->arp_id;
        CTC_ERROR_RETURN(sys_usw_nh_get_arp_oif(lchip, arp_id, &p_nh_pop->nh_com.oif, (uint8*)p_nh_pop->nh_com.mac, NULL));
    }

    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_MPLS;

    /*Create unresolved ipuc nh*/
    if (CTC_FLAG_ISSET(p_nh_param->p_mpls_nh_param->flag, CTC_MPLS_NH_IS_UNRSV))
    {
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
        return CTC_E_NONE;
    }


    if (p_nh_param->p_mpls_nh_param->aps_en)
    {
        bool protect_en = FALSE;
        CTC_ERROR_RETURN(sys_usw_aps_get_protection(lchip, p_nh_param->p_mpls_nh_param->aps_bridge_group_id, &protect_en));

        /*working path -- PW layer*/
        ret = _sys_usw_nh_mpls_build_dsnh(lchip, p_nh_param,  p_nh_param->p_mpls_nh_param->dsnh_offset, TRUE, p_nhdb);
        if (ret != CTC_E_NONE)
        {
            goto error_proc;
        }

        /*protection path -- PW layer*/
        p_nhdb->protection_path  = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_info_mpls_edit_info_t));
        if (!p_nhdb->protection_path)
        {
            ret = CTC_E_NO_MEMORY;
            goto error_proc;
        }

        sal_memset(p_nhdb->protection_path, 0, sizeof(sys_nh_info_mpls_edit_info_t));

        ret = _sys_usw_nh_mpls_build_dsnh(lchip, p_nh_param, p_nh_param->p_mpls_nh_param->dsnh_offset, FALSE, p_nhdb);
        if (ret != CTC_E_NONE)
        {
            goto error_proc;
        }

        p_nhdb->aps_group_id = p_nh_param->p_mpls_nh_param->aps_bridge_group_id;
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);
        CTC_UNSET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH);
        ret = ret ? ret : sys_usw_aps_set_share_nh(lchip, p_nhdb->aps_group_id, FALSE);

        if (ret != CTC_E_NONE)
        {
            goto error_proc;
        }
    }
    else
    {
        CTC_ERROR_GOTO(_sys_usw_nh_mpls_build_dsnh(lchip, p_nh_param,
                p_nh_param->p_mpls_nh_param->dsnh_offset, TRUE, p_nhdb),ret,error_proc);

        if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
        {
            p_nhdb->aps_group_id = p_nhdb->gport;
            CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH);
           CTC_ERROR_GOTO( sys_usw_aps_set_share_nh(lchip, p_nhdb->aps_group_id, TRUE),ret,error_proc);
        }

    }
    if (p_nh_param->p_mpls_nh_param->adjust_length != 0)
    {
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LEN_ADJUST_EN);
        p_nhdb->hdr.adjust_len = p_nh_param->p_mpls_nh_param->adjust_length ;
    }
    /*3.Create Dwfwd*/
    if(p_nh_param->hdr.have_dsfwd)
    {
        CTC_ERROR_GOTO(_sys_usw_nh_mpls_add_dsfwd(lchip, p_com_db),ret,error_proc);
    }

    /*update mpls tunnel ref_cnt*/
    if(p_nhdb->working_path.p_mpls_tunnel && (p_nh_param->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE))
    {
       _sys_usw_nh_update_tunnel_ref_info(lchip, p_nhdb, p_nhdb->working_path.p_mpls_tunnel->tunnel_id, 1);
    }
    if(p_nhdb->protection_path && p_nhdb->protection_path->p_mpls_tunnel &&
        (p_nh_param->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE))
    {
       _sys_usw_nh_update_tunnel_ref_info(lchip, p_nhdb, p_nhdb->protection_path->p_mpls_tunnel->tunnel_id, 1);
    }

    return CTC_E_NONE;

error_proc:
     _sys_usw_nh_free_mpls_nh_resource(lchip, p_nhdb,p_nh_param, 0,1, NULL);

    return ret;
}

/**
 @brief Callback function of delete unicast ip nexthop

 @param[in] p_com_db, pointer used to store nexthop data

 @return CTC_E_XXX
 */
int32
sys_usw_nh_delete_mpls_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{
    sys_nh_info_mpls_t* p_nhinfo;
    sys_nh_ref_list_node_t* p_ref_node, * p_tmp_node;
    sys_nh_param_dsfwd_t dsfwd_param;
    sys_nh_param_mpls_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_MPLS, p_data->hdr.nh_entry_type);
    p_nhinfo = (sys_nh_info_mpls_t*)(p_data);

    CTC_ERROR_RETURN(sys_usw_nh_update_ecmp_member(lchip, *p_nhid, SYS_NH_CHANGE_TYPE_NH_DELETE));

    /*1. Notify all reference nh, ipuc will be deleted*/
    p_ref_node = p_nhinfo->hdr.p_ref_nh_list;

    while (p_ref_node)
    {
        /*Remove from db*/
        p_tmp_node = p_ref_node;
        p_ref_node = p_ref_node->p_next;
        mem_free(p_tmp_node);
    }

    p_nhinfo->hdr.p_ref_nh_list = NULL;

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        /*2. Delete this mpls nexthop*/
        sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
        dsfwd_param.dsfwd_offset = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
        dsfwd_param.drop_pkt = 1;
        /*Write table*/
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));
        /*Free DsFwd offset*/
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
        SYS_NH_ENTRY_TYPE_FWD, 1, p_nhinfo->hdr.dsfwd_info.dsfwd_offset));
    }

    sal_memset(&nh_param, 0, sizeof(nh_param));
    nh_param.hdr.nhid = *p_nhid;
    CTC_ERROR_RETURN(_sys_usw_nh_free_mpls_nh_resource(lchip, p_nhinfo,&nh_param, 1,1, NULL));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_mpls_fwd_to_spec(uint8 lchip, sys_nh_param_mpls_t* p_nhpara,
                                           sys_nh_info_mpls_t* p_nhinfo)
{

    sys_nh_param_special_t nh_para_spec;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&nh_para_spec, 0, sizeof(sys_nh_param_special_t));

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        nh_para_spec.hdr.have_dsfwd= TRUE;
        nh_para_spec.hdr.nh_param_type = SYS_NH_TYPE_UNROV;
        nh_para_spec.hdr.is_internal_nh = TRUE;

        /*1.2 update dsfwd to unrov nh's dsfwd, write dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_create_special_cb(lchip,
                             (sys_nh_param_com_t*)(&nh_para_spec), (sys_nh_info_com_t*)(p_nhinfo)));
    }

    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);

    CTC_ERROR_RETURN(_sys_usw_nh_free_mpls_nh_resource(lchip, p_nhinfo,p_nhpara, 1,1, NULL));
    _sys_usw_nh_free_offset_by_nhinfo(lchip, SYS_NH_TYPE_MPLS, (sys_nh_info_com_t*)p_nhinfo);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_nh_alloc_dsnh_offset(uint8 lchip, sys_nh_param_mpls_t* p_nhpara,
                                          sys_nh_info_mpls_t* p_nhinfo)
{
    uint32    dsnh_offset = 0;
    ctc_vlan_egress_edit_info_t* p_vlan_info = NULL;

    p_vlan_info = &p_nhpara->p_mpls_nh_param->nh_para.nh_param_push.nh_com.vlan_info;

    dsnh_offset = p_nhpara->p_mpls_nh_param->dsnh_offset;
    if ((p_nhpara->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
        (p_nhpara->p_mpls_nh_param->nh_para.nh_param_push.nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN)
        && (p_vlan_info != NULL)
        && ((CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID))
             ||((CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
             ||(CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID)))))
    {
        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
    }
    else
    {
        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
    }

    if (p_nhpara->p_mpls_nh_param->aps_en)
    {
        p_nhinfo->hdr.dsnh_entry_num  = 2;
    }
    else
    {
        p_nhinfo->hdr.dsnh_entry_num  = 1;
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, p_nhinfo->hdr.dsnh_entry_num, &dsnh_offset));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, p_nhinfo->hdr.dsnh_entry_num, &dsnh_offset));

    }

    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH);

    p_nhinfo->hdr.dsfwd_info.dsnh_offset = dsnh_offset;


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_nh_free_dsnh_offset(uint8 lchip, sys_nh_info_mpls_t* p_nhinfo)
{
    uint32    dsnh_offset = 0;
    dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, p_nhinfo->hdr.dsnh_entry_num, dsnh_offset));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, p_nhinfo->hdr.dsnh_entry_num, dsnh_offset));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mpls_nh_get_oam_en(uint8 lchip, void* p_info, sys_nh_update_oam_info_t* p_oam_info,
            uint8 is_protect, uint8 is_tunnel)
{
    uint32 l3_offset = 0;
    uint32 cmd = 0;
    DsL3EditMpls3W_m ds_edit_mpls;
    sys_nh_info_mpls_t* p_nhinfo = NULL;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;

    if (!is_tunnel)
    {
        p_nhinfo = (sys_nh_info_mpls_t*)p_info;
        /*pw nexthop*/
        if (is_protect && p_nhinfo->protection_path)
        {
            l3_offset = p_nhinfo->protection_path->dsl3edit_offset;
        }
        else
        {
            l3_offset = p_nhinfo->working_path.dsl3edit_offset;
        }
    }
    else
    {
        p_mpls_tunnel =  (sys_nh_db_mpls_tunnel_t*)p_info;

        if (!is_protect)
        {
           l3_offset = p_mpls_tunnel->lsp_offset[0];
       	}
	    else
	   	{
		   l3_offset = p_mpls_tunnel->lsp_offset[1];
		}
    }

    if (!l3_offset)
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, l3_offset, cmd, &ds_edit_mpls));
    p_oam_info->lm_en = GetDsL3EditMpls3W(V, mplsOamIndex_f, &ds_edit_mpls)?1:0;
    p_oam_info->oam_mep_index = GetDsL3EditMpls3W(V, mplsOamIndex_f, &ds_edit_mpls);
    p_oam_info->lock_en = GetDsL3EditMpls3W(V, discard_f, &ds_edit_mpls) && GetDsL3EditMpls3W(V, discardType_f, &ds_edit_mpls);
    p_oam_info->mpls_label = GetDsL3EditMpls3W(V, label_f, &ds_edit_mpls);


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_mpls_fwd_attr(uint8 lchip, sys_nh_param_mpls_t* p_nhpara, sys_nh_info_mpls_t* p_nhinfo)
{
    int32 ret = CTC_E_NONE;
	sys_nh_info_mpls_t nhinfo_tmp;
    sys_nh_info_mpls_edit_info_t protection_path;  /*when pw aps enbale*/

    bool b_old_pw_aps = FALSE;
    bool b_new_pw_aps = FALSE;
    bool b_pw_aps_switch = FALSE;

    uint8 is_update = (p_nhpara->hdr.change_type != SYS_NH_CHANGE_TYPE_NULL);
    sys_nh_info_mpls_t old_nh_info;
    uint32 dsnh_offset = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    DsNextHop8W_m data_w;
    DsNextHop8W_m data_p;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memcpy(&nhinfo_tmp, p_nhinfo, sizeof(sys_nh_info_mpls_t));
    sal_memcpy(&nhinfo_tmp.hdr.dsfwd_info, &p_nhinfo->hdr.dsfwd_info, sizeof(sys_nh_info_dsfwd_t));



    b_old_pw_aps = (p_nhinfo->protection_path)? TRUE : FALSE;
    b_new_pw_aps = (p_nhpara->p_mpls_nh_param->aps_en)? TRUE : FALSE;
    b_pw_aps_switch = (b_old_pw_aps != b_new_pw_aps)? TRUE : FALSE;


    if (b_pw_aps_switch
        &&(!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH))
        && (SYS_NH_EDIT_MODE() == SYS_NH_IGS_CHIP_EDIT_MODE))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(b_old_pw_aps)
    {
       sal_memcpy(&protection_path, p_nhinfo->protection_path, sizeof(sys_nh_info_mpls_edit_info_t));

       nhinfo_tmp.protection_path = &protection_path;
    }

     if(is_update)
     {
         sys_nh_update_oam_info_t oam_info;
         sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
         _sys_usw_mpls_nh_get_oam_en(lchip, p_nhinfo, &oam_info, 0, 0);

        if (p_nhpara->p_mpls_nh_param->nh_para.nh_param_push.push_label[0].label == oam_info.mpls_label)
        {
            p_nhpara->oam_lm_en = oam_info.lm_en;
            p_nhpara->oam_lock_en = oam_info.lock_en;
            p_nhpara->oam_mep_index = oam_info.oam_mep_index;
        }

        if (b_old_pw_aps && b_new_pw_aps)
        {
             sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
             _sys_usw_mpls_nh_get_oam_en(lchip, p_nhinfo, &oam_info, 1, 0);

            if (p_nhpara->p_mpls_nh_param->nh_p_para.nh_p_param_push.push_label[0].label == oam_info.mpls_label)
            {
                p_nhpara->p_oam_lm_en = oam_info.lm_en;
                p_nhpara->p_oam_lock_en = oam_info.lock_en;
                p_nhpara->p_oam_mep_index = oam_info.oam_mep_index;
            }
        }

         /*store nh old data, for recover*/
        tbl_id = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?DsNextHop8W_t:DsNextHop4W_t;
        sal_memset(&data_w, 0, sizeof(DsNextHop8W_m));
        dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, dsnh_offset, cmd, &data_w);

        if (p_nhinfo->protection_path)
        {
            dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset + (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?2:1);
            sal_memset(&data_p, 0, sizeof(DsNextHop8W_m));
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, dsnh_offset, cmd, &data_p);
        }
         sal_memcpy(&old_nh_info, p_nhinfo, sizeof(sys_nh_info_mpls_t));
         sal_memset(p_nhinfo, 0, sizeof(sys_nh_info_mpls_t));
         sys_usw_nh_copy_nh_entry_flags(lchip, &old_nh_info.hdr, &p_nhinfo->hdr);
         p_nhinfo->updateAd = old_nh_info.updateAd;
         p_nhinfo->data = old_nh_info.data;
     }


    if (b_pw_aps_switch &&
        (SYS_NH_EDIT_MODE() == SYS_NH_IGS_CHIP_EDIT_MODE))
    {
        CTC_ERROR_GOTO(_sys_usw_mpls_nh_alloc_dsnh_offset(lchip, p_nhpara, p_nhinfo), ret, error_proc);
    }

    CTC_ERROR_GOTO(sys_usw_nh_create_mpls_cb(lchip, (sys_nh_param_com_t*)p_nhpara, (sys_nh_info_com_t*)p_nhinfo), ret, error_proc);
    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
    /*for update free resource here*/
    if(is_update)
    {
        _sys_usw_nh_free_mpls_nh_resource(lchip, &old_nh_info,p_nhpara,1,0, p_nhinfo);
        if (b_pw_aps_switch && (SYS_NH_EDIT_MODE() == SYS_NH_IGS_CHIP_EDIT_MODE))
        {
            /*free old nexthop resource*/
            _sys_usw_mpls_nh_free_dsnh_offset(lchip, &old_nh_info);
        }
    }
   return CTC_E_NONE;

error_proc:
     if (is_update)
     {
         sal_memcpy(p_nhinfo, &old_nh_info,  sizeof(sys_nh_info_mpls_t));
         tbl_id = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?DsNextHop8W_t:DsNextHop4W_t;
        dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, dsnh_offset, cmd, &data_w);

        if (p_nhinfo->protection_path)
        {
            dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset + (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?2:1);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, dsnh_offset, cmd, &data_p);
        }
     }
    return ret;

}


/**
 @brief Callback function used to update ipuc nexthop

 @param[in] p_nh_ptr, pointer of ipuc nexthop DB

 @param[in] p_para, member information

 @return CTC_E_XXX
 */
int32
sys_usw_nh_update_mpls_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                sys_nh_param_com_t* p_para)
{
    sys_nh_info_mpls_t* p_nh_info;
    sys_nh_param_mpls_t* p_nh_para;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_para);
    CTC_PTR_VALID_CHECK(p_nh_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_MPLS, p_para->hdr.nh_param_type);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_MPLS, p_nh_db->hdr.nh_entry_type);
    p_nh_info = (sys_nh_info_mpls_t*)(p_nh_db);
    p_nh_para = (sys_nh_param_mpls_t*)(p_para);

    switch (p_nh_para->hdr.change_type)
    {
    case SYS_NH_CHANGE_TYPE_FWD_TO_UNROV:
        if (p_nh_info->arp_id)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Arp nexthop cannot update to unrov \n");
			return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_RETURN(_sys_usw_nh_update_mpls_fwd_to_spec(lchip, p_nh_para, p_nh_info));
		break;

    case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
        p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

        CTC_ERROR_RETURN(_sys_usw_nh_update_mpls_fwd_attr(lchip, p_nh_para, p_nh_info));
	    break;

    case SYS_NH_CHANGE_TYPE_UNROV_TO_FWD:
        if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Current nexthop isnt unresolved nexthop \n");
			return CTC_E_INVALID_CONFIG;

        }
        p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);
        CTC_ERROR_RETURN(_sys_usw_nh_update_mpls_fwd_attr(lchip, p_nh_para, p_nh_info));

        break;
   case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
        {
  		  if (CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
            {
                return CTC_E_NONE;
            }
            CTC_ERROR_RETURN(_sys_usw_nh_mpls_add_dsfwd(lchip,  p_nh_db));
        }
    	break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    if (p_nh_info->hdr.p_ref_nh_list)
    {
       int32 ret = 0;
       ret = (sys_usw_nh_update_ecmp_member(lchip, p_nh_para->hdr.nhid, p_nh_para->hdr.change_type));
       if (ret)
       {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop update for ecmp fail!, ret:%d \n", ret);
       }
    }

    if (CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED))
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_loopback_l2edit(lchip, p_nh_para->hdr.nhid));
    }

    return CTC_E_NONE;
}

STATIC int32
sys_usw_mpls_nh_tunnel_label_free_resource(uint8 lchip, bool is_2level_p_path,
                                                  sys_nh_db_mpls_tunnel_t* p_mpls_tunnel)
{
    uint8 lsp_loop_end             = 0;
    uint8 smpe_cnt                 = 0;
    uint8 protect_lsp               = 0;
    uint8 protect_spme              = 0;
    uint32 offset                  = 0;
    sys_dsmpls_t dsmpls;
    uint8 table_type               = 0;
    uint16 update_aps_id             = 0 ;
    int16 i = 0;

    sys_aps_bind_nh_param_t  bind_nh;
    sal_memset(&bind_nh,0,sizeof(bind_nh));
    bind_nh.mpls_tunnel = 1;
    bind_nh.nh_id = p_mpls_tunnel->tunnel_id;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
    {
        protect_lsp = is_2level_p_path;
        lsp_loop_end = protect_lsp + 1;
    }
    else
    {
        protect_lsp = 0;
        lsp_loop_end = CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS) ? 2 : 1;
    }

    for (protect_lsp = 0; protect_lsp < lsp_loop_end; protect_lsp++ )
    {
        /*Free LSP*/
        offset =  p_mpls_tunnel->lsp_offset[protect_lsp];
        if (offset)
        {
            if(CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
            {
                table_type = SYS_NH_ENTRY_TYPE_L3EDIT_SPME;
            }
            else if(CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR) && !DRV_IS_DUET2(lchip))
            {
                table_type = SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W;
            }
            else
            {
                table_type = SYS_NH_ENTRY_TYPE_L3EDIT_MPLS;
            }

            if (p_mpls_tunnel->p_l3_edit)
            {
                sys_usw_nh_remove_l3edit_tunnel(lchip, p_mpls_tunnel->p_l3_edit, table_type);
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, table_type, 1, offset));
                p_mpls_tunnel->lsp_offset[protect_lsp] = 0;
                sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));
                CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, table_type, offset, &dsmpls));
            }
        }
        if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
        {
            sys_usw_aps_get_next_group(lchip, p_mpls_tunnel->gport, &update_aps_id, protect_lsp);
            smpe_cnt = 2;
        }
        else
        {
            update_aps_id = p_mpls_tunnel->gport;
            smpe_cnt = 1;

        }
        for (protect_spme = 0; protect_spme < smpe_cnt; protect_spme++ )
        {
            /*Free SPME*/
            offset = p_mpls_tunnel->spme_offset[protect_lsp][protect_spme];
            if (offset && (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME) ||
                CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME)))
            {
                if (p_mpls_tunnel->p_l3_edit)
                {
                    table_type = SYS_NH_ENTRY_TYPE_L3EDIT_SPME;
                    sys_usw_nh_remove_l3edit_tunnel(lchip, p_mpls_tunnel->p_l3_edit, table_type);
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, offset));
                    p_mpls_tunnel->spme_offset[protect_lsp][protect_spme] = 0;
                    sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));
                    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME,
                                                                        offset, &dsmpls));
                }
            }

            /*free l2edti*/
            if (p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protect_spme] && (!p_mpls_tunnel->arp_id[protect_lsp][protect_spme]))
            {
                if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W))
                {
                    sys_usw_nh_remove_route_l2edit_8w_outer(lchip, (sys_nh_db_dsl2editeth8w_t*)(p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protect_spme]));
                    p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protect_spme] = NULL;
                    CTC_UNSET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W);
                }
                else
                {
                    sys_usw_nh_remove_route_l2edit_outer(lchip, p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protect_spme]);
                    p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protect_spme] = NULL;
                }
            }
            /*free arp*/
            if (p_mpls_tunnel->arp_id[protect_lsp][protect_spme])
            {
                sys_usw_nh_unbind_arp_cb(lchip, p_mpls_tunnel->arp_id[protect_lsp][protect_spme], 0);
                p_mpls_tunnel->arp_id[protect_lsp][protect_spme] = 0;
            }

            /*free loop-nhid*/
            if (p_mpls_tunnel->loop_nhid[protect_lsp][protect_spme])
            {
                sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_mpls_tunnel->l2edit_offset[protect_lsp][protect_spme]);
                p_mpls_tunnel->loop_nhid[protect_lsp][protect_spme] = 0;
            }
            bind_nh.working_path = (smpe_cnt == 2)? protect_spme :protect_lsp;

            if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
            {
                sys_usw_aps_unbind_nexthop(lchip,update_aps_id,&bind_nh);
            }
        }

	    if (p_mpls_tunnel->sr[protect_lsp])
        {
            sys_nh_mpls_sr_t* p_tunnel_sr = NULL;
            p_tunnel_sr = p_mpls_tunnel->sr[protect_lsp];
            for (i = p_mpls_tunnel->sr_loop_num[protect_lsp]- 1; i >= 0; i--)
            {
                if (p_tunnel_sr[i].l2edit_offset)
                {
                    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_tunnel_sr[i].l2edit_offset);
                }
                if (p_tunnel_sr[i].spme_offset)
                {
                    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, p_tunnel_sr[i].spme_offset);
                }
                if (p_tunnel_sr[i].lsp_offset)
                {
                    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_tunnel_sr[i].lsp_offset);
                }

                if (DRV_IS_DUET2(lchip))
                {
                    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_tunnel_sr[i].pw_offset);
                }
                else
                {
                    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W, 1, p_tunnel_sr[i].pw_offset);
                }
                sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1, p_tunnel_sr[i].dsnh_offset);
            }
            mem_free(p_mpls_tunnel->sr[protect_lsp]);
        }

    }

    return CTC_E_NONE;
}

int32
sys_usw_mpls_nh_update_aps(uint8 lchip, uint8 protection_spme,uint8 protect_lsp,
                                  ctc_mpls_nexthop_tunnel_param_t* p_nh_param,
                                  sys_nh_db_mpls_tunnel_t* p_mpls_tunnel)
{
    uint16 update_aps_id = 0;
    uint8 update_is_protect_path = 0;
    uint16 edit_ptr = 0;
    uint16 arp_id = 0;
    sys_aps_bind_nh_param_t bind_param;
    uint32 loop_nhid = 0;

    sys_l3if_prop_t l3if_prop;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    sal_memset(&bind_param,0,sizeof(bind_param));
    sal_memset(&l3if_prop, 0, sizeof(l3if_prop));


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN))
    {
      CTC_ERROR_RETURN(
            sys_usw_aps_get_next_group(lchip, p_nh_param->aps_bridge_group_id, &update_aps_id, !protect_lsp));
      edit_ptr = p_mpls_tunnel->lsp_offset[protect_lsp];
      /* 2-level spme & lsp aps, lsp edit ptr only need 2 pointer:
         working lsp & protection lsp outedit pointer */

        if(!protect_lsp)
        {
            p_mpls_tunnel->flag  |= SYS_NH_MPLS_TUNNEL_FLAG_WORKING;
        }
        else
        {
            p_mpls_tunnel->flag  |= SYS_NH_MPLS_TUNNEL_FLAG_PROTECTION;
        }
        bind_param.spme_en = 1;
        arp_id =  p_mpls_tunnel->arp_id[protect_lsp][protection_spme];
        loop_nhid = p_mpls_tunnel->loop_nhid[protect_lsp][protection_spme];

        update_is_protect_path = protection_spme;

       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update Lsp protect_en:%d, Next apsId:%d,protect_en:%d editPtr:%d\n",
                      protect_lsp, update_aps_id,protection_spme, edit_ptr);

    }
    else if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN))
    {
        update_aps_id = p_nh_param->aps_bridge_group_id;
        arp_id   = p_mpls_tunnel->arp_id[protect_lsp][0];
        loop_nhid = p_mpls_tunnel->loop_nhid[protect_lsp][0];
        update_is_protect_path = protect_lsp;
        edit_ptr = p_mpls_tunnel->lsp_offset[protect_lsp];
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update Lsp protect_en:%d, apsId:%d, editPtr:%d\n",
                         protect_lsp, update_aps_id, edit_ptr);
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Not need update aps!!!!!!\n");
    }

    bind_param.working_path = !update_is_protect_path;
    bind_param.edit_ptr_valid =1;
    bind_param.edit_ptr = edit_ptr;
    bind_param.edit_ptr_type = 1;
    bind_param.edit_ptr_location = 0;

    if ((!arp_id) && !loop_nhid)
    {
        ctc_mpls_nexthop_tunnel_info_t  *p_tunnel_param ;
        p_tunnel_param = update_is_protect_path ?(&p_nh_param->nh_p_param):(&p_nh_param->nh_param);

        /*SYSTEM MODIFIED by yangl for tm g8131*/
        sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_tunnel_param->oif.gport, p_tunnel_param->oif.vid, &l3if_prop);
        bind_param.l3if_id  = l3if_prop.l3if_id;
        bind_param.dest_id = p_tunnel_param->oif.gport;
    }
    else if (!loop_nhid)
    {
       sys_nh_db_arp_t*  p_arp = NULL;
        p_arp = (sys_usw_nh_lkup_arp_id(lchip,arp_id ));
        if(p_arp)
        {
           bind_param.destmap_profile = 1;
           bind_param.dest_id = p_arp->destmap_profile;
           bind_param.l3if_id  = p_arp->l3if_id;
        }
    }
    bind_param.mpls_tunnel = 1;
    bind_param.nh_id = p_mpls_tunnel->tunnel_id ;
    CTC_ERROR_RETURN(sys_usw_aps_bind_nexthop(lchip, update_aps_id, &bind_param));
  return CTC_E_NONE;

}

/*
ctc_mpls_nexthop_tunnel_param_t :
   if tunnel label have two layer label,taking LSP label & SPME label as a case;
         and if tunnel label have only one layer lable,taking LSP label as a case

    if CTC_NH_MPLS_APS_EN & CTC_NH_MPLS_APS_2_LAYER_EN are set, aps_bridge_group_id is 2-level(Lsp) protection group id
                                    SPME label(w)  (nh_param)
              LSP label(w)------>                                (CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION isn't set)
                                    SPME label(p)   (nh_p_param)


                                    SPME label(w)   (nh_param)
              LSP label(P)------>                                (CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION is set)
                                    SPME label(p)   (nh_p_param)

    if CTC_NH_MPLS_APS_EN only is set, there are two case:
     1)aps_bridge_group_id is 2-level(Lsp) protection group id.

              LSP label(w)------> SPME label  (-->nh_param)
              LSP label(P)------> SPME label  (-->nh_p_param)

     2)aps_bridge_group_id is 2-level(Lsp) protection group id.
              LSP label(w) (-->nh_param)
              LSP label(P) (-->nh_p_param)
*/
int32
sys_usw_mpls_nh_update_tunnel_label_fwd_attr(uint8 lchip, uint8 is_add,
                                           uint16 tunnel_id,
                                           ctc_mpls_nexthop_tunnel_param_t* p_nh_param)
{
    ctc_mpls_nexthop_tunnel_info_t* p_tunnel_info = NULL;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    int32     ret = 0;
    uint8    loop = 0, loop_cnt = 0;
    sys_l3if_prop_t l3if_prop;
    uint8 protect_lsp = 0,protection_spme;
    bool is_2level_p_path = 0;
    bool free_memory = 0;
    uint16 arp_id = 0;
    uint8 index = 0;
    uint8 arp_update = 0;
    uint16 old_arp_id[SYS_NH_APS_M][SYS_NH_APS_M];


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* parameter */
    CTC_PTR_VALID_CHECK(p_nh_param);
    if(p_nh_param->nh_param.label_num > CTC_MPLS_NH_MAX_TUNNEL_LABEL_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }
    for (index = 0; index < CTC_MPLS_NH_MAX_TUNNEL_LABEL_NUM; index++)
    {
        SYS_NH_CHECK_MPLS_LABVEL_VALUE(p_nh_param->nh_param.tunnel_label[index].label);
        SYS_NH_CHECK_MPLS_EXP_VALUE(p_nh_param->nh_param.tunnel_label[index].exp);
        SYS_NH_CHECK_MPLS_EXP_TYPE_VALUE(p_nh_param->nh_param.tunnel_label[index].exp_type);
        CTC_MAX_VALUE_CHECK(p_nh_param->nh_param.tunnel_label[index].exp_domain, 15);
    }

    if (p_nh_param->nh_param.is_sr)
    {
        if (p_nh_param->nh_param.is_spme)
        {
            return CTC_E_INVALID_PARAM;
        }

        for (index = 0; index < p_nh_param->nh_param.label_num; index++)
        {
            if ((p_nh_param->nh_param.tunnel_label[index].exp_type != CTC_NH_EXP_SELECT_ASSIGN)
                || (CTC_FLAG_ISSET(p_nh_param->nh_param.tunnel_label[index].lable_flag, CTC_MPLS_NH_LABEL_MAP_TTL)))
            {
                return CTC_E_INVALID_PARAM;
            }
        }
    }
    else if (p_nh_param->nh_param.label_num > 2)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel));
    if (!is_add && !p_mpls_tunnel)
    {
        return CTC_E_NOT_EXIST;
    }

    sal_memset(&l3if_prop, 0, sizeof(l3if_prop));
    if (!is_add)
    {
        /*check working path arp id change*/
        if (!p_nh_param->aps_flag || !CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN))
        {
            /*tunnel no aps or tunnel 1 level aps working path*/
            arp_id = p_mpls_tunnel->arp_id[0][0];
        }
        else
        {
            /*tunnel 2 level aps */
            arp_id = (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION)?p_mpls_tunnel->arp_id[1][0]:p_mpls_tunnel->arp_id[0][0]);
        }

        if ((p_nh_param->nh_param.arp_id && !arp_id) || (!p_nh_param->nh_param.arp_id && arp_id))
        {
            return CTC_E_INVALID_CONFIG;
        }

        arp_id = p_nh_param->nh_param.arp_id?p_nh_param->nh_param.arp_id:p_mpls_tunnel->arp_id[0][0];
        sal_memcpy(old_arp_id, p_mpls_tunnel->arp_id, sizeof(p_mpls_tunnel->arp_id));
        if (p_nh_param->nh_param.arp_id && (p_nh_param->nh_param.arp_id != p_mpls_tunnel->arp_id[0][0]))
        {
            arp_update = 1;
        }
    }
    else
    {
        /*new add, get arp from parameter*/
        arp_id = p_nh_param->nh_param.arp_id;
    }

    /*get oif from arp*/
    CTC_ERROR_RETURN(sys_usw_nh_get_arp_oif(lchip, arp_id, &p_nh_param->nh_param.oif, (uint8*)p_nh_param->nh_param.mac, NULL));
    SYS_GLOBAL_PORT_CHECK( p_nh_param->nh_param.oif.gport);
    CTC_MAX_VALUE_CHECK( p_nh_param->nh_param.oif.vid, CTC_MAX_VLAN_ID);

    /*for sr*/
    if (p_nh_param->nh_param.label_num > 2)
    {
        for (index = 0; index < p_nh_param->nh_param.label_num; index++)
        {
            if (p_nh_param->nh_param.tunnel_label[index].exp_type != CTC_NH_EXP_SELECT_ASSIGN ||
                CTC_FLAG_ISSET(p_nh_param->nh_param.tunnel_label[index].lable_flag, CTC_MPLS_NH_LABEL_MAP_TTL) ||
                p_nh_param->nh_param.is_spme || p_nh_param->nh_param.stats_valid)
            {
                return CTC_E_INVALID_CONFIG;
            }
        }
    }

    if(p_nh_param->aps_flag)
    {
        if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN) && p_mpls_tunnel)
        {
            /*tunnel 2 level aps protect*/
            arp_id = CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION)?p_mpls_tunnel->arp_id[1][1]:p_mpls_tunnel->arp_id[0][1];

        }
        else if (p_mpls_tunnel)
        {
            /*check lsp protect arp id changed or not*/
            arp_id = p_mpls_tunnel->arp_id[1][0];
        }

        if (!is_add && ((p_nh_param->nh_p_param.arp_id && !arp_id) ||(!p_nh_param->nh_p_param.arp_id && arp_id)))
        {
            return CTC_E_INVALID_CONFIG;
        }

        if (p_nh_param->nh_p_param.arp_id && (p_nh_param->nh_p_param.arp_id != arp_id))
        {
            arp_update = 1;
        }

        for (index = 0; index < p_nh_param->nh_p_param.label_num; index++)
        {
            SYS_NH_CHECK_MPLS_LABVEL_VALUE(p_nh_param->nh_p_param.tunnel_label[index].label);
            SYS_NH_CHECK_MPLS_EXP_VALUE(p_nh_param->nh_p_param.tunnel_label[index].exp);
            SYS_NH_CHECK_MPLS_EXP_TYPE_VALUE(p_nh_param->nh_p_param.tunnel_label[index].exp_type);
            CTC_MAX_VALUE_CHECK(p_nh_param->nh_p_param.tunnel_label[index].exp_domain, 15);
        }
        CTC_ERROR_RETURN(sys_usw_nh_get_arp_oif(lchip, p_nh_param->nh_p_param.arp_id,
            &p_nh_param->nh_p_param.oif, (uint8*)p_nh_param->nh_p_param.mac, NULL));
        SYS_GLOBAL_PORT_CHECK( p_nh_param->nh_p_param.oif.gport);
        CTC_MAX_VALUE_CHECK( p_nh_param->nh_p_param.oif.vid, CTC_MAX_VLAN_ID);
    }

    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN))
    {
        SYS_APS_GROUP_ID_CHECK(p_nh_param->aps_bridge_group_id);

        if (p_nh_param->nh_p_param.stats_id)
        {
            uint32 stats_ptr = 0;
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_nh_param->nh_p_param.stats_id, &stats_ptr));
        }
    }


    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN)
        && (p_nh_param->nh_param.label_num == 0))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Need at least one mpls label\n");
        return CTC_E_INVALID_CONFIG;

    }

    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN)
        && (p_nh_param->nh_p_param.label_num == 0))
    {
      sal_memcpy(&p_nh_param->nh_p_param,&p_nh_param->nh_param,sizeof(ctc_mpls_nexthop_tunnel_info_t));
    }

    /*check stats id*/
    if (p_nh_param->nh_param.stats_id)
    {
        uint32 stats_ptr = 0;
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_nh_param->nh_param.stats_id, &stats_ptr));
    }

    if (!CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN))
    {
        if (!(CTC_IS_CPU_PORT(p_nh_param->nh_param.oif.gport) || p_nh_param->nh_param.oif.is_l2_port ||
                SYS_IS_DROP_PORT(p_nh_param->nh_param.oif.gport)) && !p_nh_param->nh_param.loop_nhid)
        {
            CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_nh_param->nh_param.oif.gport, p_nh_param->nh_param.oif.vid, &l3if_prop));
        }
        else
        {
            /*SYSTEM MODIFIED by yangl for openflow 2019-08-27*/
            l3if_prop.l3if_id = 1021;
            l3if_prop.vlan_ptr = 5117; /*MAX_CTC_L3IF_ID -1 + 4096*/
        }
    }

    if (is_add)
    {
        if (p_mpls_tunnel)
        {
            if (!CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mpls tunnel already exist \n");
                return CTC_E_EXIST;
            }
            else if ((CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_WORKING) == !CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION))
                || (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_PROTECTION) == CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION)))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
                return CTC_E_EXIST;

            }
        }
        else
        {
            p_mpls_tunnel  = (sys_nh_db_mpls_tunnel_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_db_mpls_tunnel_t));
            if (!p_mpls_tunnel)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                return CTC_E_NO_MEMORY;

            }

            sal_memset(p_mpls_tunnel, 0, sizeof(sys_nh_db_mpls_tunnel_t));
            free_memory = 1;

        }
    }
    else
    {
         /*p_mpls_tunnel->flag = 0;*/
    }

    p_mpls_tunnel->tunnel_id = tunnel_id;
    if (p_nh_param->nh_param.is_sr)
    {
        CTC_SET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR);
    }

    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN))
    {
        CTC_SET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS);
    }
    else
    {
        CTC_UNSET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS);
    }

    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN))
    {
        CTC_SET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS);
        is_2level_p_path = (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION));
    }
    else
    {
        CTC_UNSET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS);
    }

    if (p_nh_param->nh_param.is_spme)
    {
        p_mpls_tunnel->flag  |= SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME;
    }

    p_tunnel_info = &p_nh_param->nh_param;
    loop_cnt = (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN)) ? 2 : 1;
    loop = 0;

    while (loop < loop_cnt)
    {
         if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN) )
         {
             if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION))
             {
                 protect_lsp = 1;
             }

             if (!CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN) && loop)
             {
                 protect_lsp = 1;
             }
         }

         if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN))
         {
             protection_spme = loop;
         }
         else
         {
             protection_spme = 0;
         }

         /*1. Build l2edit(ARP), output mac ===========*/
        if(!is_add)
        {
            /*free l2edti*/
            if (p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protection_spme] && (!p_mpls_tunnel->arp_id[protect_lsp][protection_spme]))
            {
                if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W))
                {
                    sys_usw_nh_remove_route_l2edit_8w_outer(lchip, (sys_nh_db_dsl2editeth8w_t*)(p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protection_spme]));
                    p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protection_spme] = NULL;
                    CTC_UNSET_FLAG(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W);
                }
                else
                {
                    if (p_mpls_tunnel->loop_nhid[protect_lsp][protection_spme])
                    {
                        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_mpls_tunnel->l2edit_offset[protect_lsp][protection_spme]);
                        p_mpls_tunnel->loop_nhid[protect_lsp][protection_spme] = 0;
                    }
                    else
                    {
                        sys_usw_nh_remove_route_l2edit_outer(lchip, p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protection_spme]);
                        p_mpls_tunnel->p_dsl2edit_4w[protect_lsp][protection_spme] = NULL;
                    }
                }
            }
        }
        CTC_ERROR_GOTO(_sys_usw_nh_mpls_build_dsl2edit(lchip, protection_spme,
                                                               protect_lsp,
                                                               p_tunnel_info,
                                                               p_mpls_tunnel),
                                                    ret, error_proc);
        if ((p_tunnel_info->label_num > 2 && DRV_IS_DUET2(lchip)) ||
            (p_tunnel_info->label_num > 10 && !DRV_IS_DUET2(lchip)))
        {
            /*sr lable need loop*/
            CTC_ERROR_GOTO(_sys_usw_nh_mpls_build_sr_tunnel(lchip, protect_lsp, p_tunnel_info, p_mpls_tunnel, is_add), ret, error_proc);
        }

        if (p_tunnel_info->is_sr && !DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_GOTO(_sys_usw_mpls_nh_push_multi_label(lchip, protect_lsp, p_tunnel_info, p_mpls_tunnel), ret, error_proc);
        }
        else
        {
            /*2. Build spme ===========*/
            if (p_tunnel_info->label_num >= 2)
            {
                CTC_ERROR_GOTO(_sys_usw_nh_mpls_build_tunnel(lchip, protection_spme,
                                                                 protect_lsp,
                                                                 SYS_NH_MPLS_LABLE_SPME,
                                                                 p_tunnel_info,
                                                                 p_mpls_tunnel),
                                                   ret, error_proc);
            }
	        else if (p_mpls_tunnel->spme_offset[protect_lsp][protection_spme] != 0)
            {
                sys_dsmpls_t dsmpls;
                uint32 offset = 0;

                sal_memset(&dsmpls , 0, sizeof(sys_dsmpls_t));
                offset = p_mpls_tunnel->spme_offset[protect_lsp][protection_spme];
                CTC_ERROR_GOTO(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, offset), ret, error_proc);
                p_mpls_tunnel->spme_offset[protect_lsp][protection_spme] = 0;
                sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));
                CTC_ERROR_GOTO(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME,
                                                                    offset, &dsmpls), ret, error_proc);
            }

            /*3. Build lsp ===========*/
            if (p_tunnel_info->label_num >= 1)
            {
                CTC_ERROR_GOTO(_sys_usw_nh_mpls_build_tunnel(lchip, protection_spme,
                         protect_lsp,
                         SYS_NH_MPLS_LABLE_LSP,
                         p_tunnel_info,
                         p_mpls_tunnel),
                     ret, error_proc);
            }
        }

         /*3. update aps ===========*/
         if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN) )
         {
             CTC_ERROR_GOTO(sys_usw_mpls_nh_update_aps(lchip,protection_spme, protect_lsp,
                                                              p_nh_param,
                                                              p_mpls_tunnel),
                                                  ret, error_proc);
         }
         p_tunnel_info = &p_nh_param->nh_p_param;
         loop++;
    };


    if (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN))
    {
        p_mpls_tunnel->gport = p_nh_param->aps_bridge_group_id;
    }
    else
    {
        sys_nh_db_arp_t*  p_arp = NULL;
        uint8 gchip = 0;

        if (p_nh_param->nh_param.arp_id)
        {
            /*not aps, if arp exist, get gport for to-cpu arp*/
            p_arp = sys_usw_nh_lkup_arp_id(lchip, p_nh_param->nh_param.arp_id);
            if (!p_arp)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
    			return CTC_E_NOT_EXIST;
            }
            sys_usw_get_gchip_id(lchip, &gchip);
            p_mpls_tunnel->gport = ((CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))?CTC_GPORT_RCPU(gchip):p_nh_param->nh_param.oif.gport);
        }
        else
        {
            p_mpls_tunnel->gport = p_nh_param->nh_param.oif.gport;
        }
        p_mpls_tunnel->l3ifid = l3if_prop.l3if_id;
    }

    p_mpls_tunnel->label_num = p_nh_param->nh_param.label_num;

    CTC_ERROR_GOTO((sys_usw_nh_add_mpls_tunnel(lchip, tunnel_id, p_mpls_tunnel)),
                   ret, error_proc);


    if (arp_update)
    {
        sys_nh_info_mpls_t* p_nh_info = NULL;
        sys_nh_ref_list_node_t* p_curr = NULL;
        uint8 lsp_index = 0;

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Tunnel Arp update !\n");

        /*1.unbind arp */
        for (lsp_index = 0; lsp_index < 2; lsp_index++)
        {
            if ((old_arp_id[lsp_index][0] != p_mpls_tunnel->arp_id[lsp_index][0]) && old_arp_id[lsp_index][0])
            {
                sys_usw_nh_unbind_arp_cb(lchip, old_arp_id[lsp_index][0], 0);
            }

            if ((old_arp_id[lsp_index][1] != p_mpls_tunnel->arp_id[lsp_index][1]) && old_arp_id[lsp_index][1])
            {
                sys_usw_nh_unbind_arp_cb(lchip, old_arp_id[lsp_index][1], 0);
            }
        }

        /* 2. update mpls nexthop */
        p_curr = p_mpls_tunnel->p_ref_nh_list;
        while (p_curr)
        {
            p_nh_info = (sys_nh_info_mpls_t*)p_curr->p_ref_nhinfo;
            if (!p_nh_param->aps_flag)
            {
                _sys_usw_nh_mpls_update_arp_cb(lchip, p_nh_info, p_mpls_tunnel);
            }
            else
            {
                p_nh_info->arp_id = p_mpls_tunnel->arp_id[0][0];
            }
            p_curr = p_curr->p_next;
        }
    }

    return CTC_E_NONE;

  error_proc:
      sys_usw_mpls_nh_tunnel_label_free_resource(lchip, is_2level_p_path, p_mpls_tunnel);
      if (free_memory)
      {
          sys_usw_nh_remove_mpls_tunnel(lchip, p_mpls_tunnel->tunnel_id);
          mem_free(p_mpls_tunnel);
      }
      return ret;

  }



int32
sys_usw_mpls_nh_update_tunnel_mac_fwd_attr(uint8 lchip, bool is_add,
                                           uint16 tunnel_id,
                                           ctc_mpls_nexthop_tunnel_param_t* p_nh_param)
{
    ctc_mpls_nexthop_tunnel_info_t* p_tunnel_info = &p_nh_param->nh_param;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    int32     ret = 0;
    sys_l3if_prop_t l3if_prop;
    bool is_2level_p_path = 0;
    bool free_memory = 0;
    uint8 do_nh_edit = 1;
    uint16 arp_id = 0;
    uint8 arp_update = 0;
    uint16 old_arp_id = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel));
    if (!is_add && !p_mpls_tunnel)
    {
        return CTC_E_NOT_EXIST;
    }

    sal_memset(&l3if_prop, 0, sizeof(l3if_prop));
    if (p_mpls_tunnel)
    {
        if ((p_nh_param->nh_param.arp_id && !p_mpls_tunnel->arp_id[0][0]) ||
            (!p_nh_param->nh_param.arp_id && p_mpls_tunnel->arp_id[0][0]))
        {
            return CTC_E_INVALID_CONFIG;
        }

        arp_id = p_nh_param->nh_param.arp_id?p_nh_param->nh_param.arp_id:p_mpls_tunnel->arp_id[0][0];
        if (p_nh_param->nh_param.arp_id && (p_nh_param->nh_param.arp_id != p_mpls_tunnel->arp_id[0][0]))
        {
            arp_update = 1;
            old_arp_id = p_mpls_tunnel->arp_id[0][0];
        }
    }
    else
    {
        /*new add, get arp from parameter*/
        arp_id = p_nh_param->nh_param.arp_id;
    }

    /*get oif from arp*/
    CTC_ERROR_RETURN(sys_usw_nh_get_arp_oif(lchip, arp_id, &p_nh_param->nh_param.oif, (uint8*)p_nh_param->nh_param.mac, NULL));
    SYS_GLOBAL_PORT_CHECK(p_tunnel_info->oif.gport);
    CTC_MAX_VALUE_CHECK(p_tunnel_info->oif.vid, CTC_MAX_VLAN_ID);

    /* check exp */
    if (!(CTC_IS_CPU_PORT(p_nh_param->nh_param.oif.gport) || p_nh_param->nh_param.oif.is_l2_port ||
            SYS_IS_DROP_PORT(p_nh_param->nh_param.oif.gport)) && !p_nh_param->nh_param.loop_nhid)
    {
        CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_nh_param->nh_param.oif.gport, p_nh_param->nh_param.oif.vid, &l3if_prop));
    }
    else
    {
            /*SYSTEM MODIFIED by yangl for openflow 2019-08-27*/
            l3if_prop.l3if_id = 1021;
            l3if_prop.vlan_ptr = 5117; /*MAX_CTC_L3IF_ID -1 + 4096*/
    }

    if (is_add)
    {
        if (p_mpls_tunnel)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;


        }
        else
        {
            p_mpls_tunnel  = (sys_nh_db_mpls_tunnel_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_db_mpls_tunnel_t));

            if (!p_mpls_tunnel)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

            }

            sal_memset(p_mpls_tunnel, 0, sizeof(sys_nh_db_mpls_tunnel_t));
            free_memory = 1;

        }
        p_mpls_tunnel->tunnel_id = tunnel_id;
    }

    if (do_nh_edit)
    {
        /*1. Build l2edit(ARP), output mac ===========*/
        if (!is_add)
        {
            /*free l2edti*/
            if (p_mpls_tunnel->p_dsl2edit_4w[0][0] && (!p_mpls_tunnel->arp_id[0][0]))
            {
                sys_usw_nh_remove_route_l2edit_outer(lchip, p_mpls_tunnel->p_dsl2edit_4w[0][0]);
            }
        }

        CTC_ERROR_GOTO(_sys_usw_nh_mpls_build_dsl2edit(lchip, 0, 0,
                                                              p_tunnel_info,
                                                              p_mpls_tunnel),
                                                          ret,
                                                          error_proc);
    }

    p_mpls_tunnel->gport = p_nh_param->nh_param.oif.gport;
    p_mpls_tunnel->l3ifid = l3if_prop.l3if_id;

    CTC_ERROR_GOTO((sys_usw_nh_add_mpls_tunnel(lchip, tunnel_id, p_mpls_tunnel)),
                   ret, error_proc);

    if (arp_update)
    {
        sys_nh_info_mpls_t* p_nh_info = NULL;
        sys_nh_ref_list_node_t* p_curr = NULL;
        p_curr = p_mpls_tunnel->p_ref_nh_list;
        sys_usw_nh_unbind_arp_cb(lchip, old_arp_id, 0);
        while (p_curr)
        {
            p_nh_info = (sys_nh_info_mpls_t*)p_curr->p_ref_nhinfo;
            _sys_usw_nh_mpls_update_arp_cb(lchip, p_nh_info, p_mpls_tunnel);
            p_curr = p_curr->p_next;
        }
    }
    return CTC_E_NONE;

  error_proc:
      sys_usw_mpls_nh_tunnel_label_free_resource(lchip, is_2level_p_path, p_mpls_tunnel);
      if (free_memory)
      {
          sys_usw_nh_remove_mpls_tunnel(lchip, p_mpls_tunnel->tunnel_id);
          mem_free(p_mpls_tunnel);
      }
      return ret;

}

int32
sys_usw_mpls_nh_create_tunnel_label(uint8 lchip, uint16 tunnel_id,
                                           ctc_mpls_nexthop_tunnel_param_t* p_nh_param)
{
    if ((p_nh_param->nh_param.label_num == 0)
        && (!CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN) ||
    (CTC_FLAG_ISSET(p_nh_param->aps_flag, CTC_NH_MPLS_APS_EN) && (p_nh_param->nh_p_param.label_num == 0))))
    {
        CTC_ERROR_RETURN(sys_usw_mpls_nh_update_tunnel_mac_fwd_attr(lchip, TRUE, tunnel_id, p_nh_param));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_mpls_nh_update_tunnel_label_fwd_attr(lchip, TRUE, tunnel_id, p_nh_param));
    }

    return CTC_E_NONE;
}

int32
sys_usw_mpls_nh_remove_tunnel_label(uint8 lchip, uint16 tunnel_id)
{

    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel;
    sys_nh_ref_list_node_t* p_ref_node;
    sys_nh_ref_list_node_t* p_tmp_node;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel));

    if (p_mpls_tunnel && p_mpls_tunnel->ref_cnt != 0)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Other nexthop are using already\n");
			return CTC_E_IN_USE;

    }

    if (!p_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
    {
        CTC_ERROR_RETURN(sys_usw_mpls_nh_tunnel_label_free_resource(lchip, 1, p_mpls_tunnel));
    }

    CTC_ERROR_RETURN(sys_usw_mpls_nh_tunnel_label_free_resource(lchip,  0, p_mpls_tunnel));

    CTC_ERROR_RETURN(sys_usw_nh_remove_mpls_tunnel(lchip, p_mpls_tunnel->tunnel_id));

    p_ref_node = p_mpls_tunnel->p_ref_nh_list;
    while (p_ref_node)
    {
        /*Remove from db*/
        p_tmp_node = p_ref_node;
        p_ref_node = p_ref_node->p_next;
        mem_free(p_tmp_node);
    }

    mem_free(p_mpls_tunnel);

    return CTC_E_NONE;
}


int32
sys_usw_mpls_nh_update_tunnel_label(uint8 lchip, uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_tunnel_param)
{
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    sys_nh_db_mpls_tunnel_t mpls_tunnel;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"tunnel_id:%d, label_num:%d aps_en:%d\n", tunnel_id, p_tunnel_param->nh_param.label_num,
        (CTC_FLAG_ISSET(p_tunnel_param->aps_flag, CTC_NH_MPLS_APS_EN)?1:0));

    sal_memset(&mpls_tunnel, 0, sizeof(sys_nh_db_mpls_tunnel_t));

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel));

    if (!p_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mpls tunnel not exist \n");
		return CTC_E_NOT_EXIST;
    }

    if ((CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS) !=
            CTC_FLAG_ISSET(p_tunnel_param->aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN))
        || (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS) !=
            CTC_FLAG_ISSET(p_tunnel_param->aps_flag, CTC_NH_MPLS_APS_EN))
        || (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS)
            && p_tunnel_param->aps_bridge_group_id != p_mpls_tunnel->gport)
        || ((0 == p_tunnel_param->nh_param.label_num) && (0 != p_mpls_tunnel->label_num))
        || ((0 != p_tunnel_param->nh_param.label_num) && (0 == p_mpls_tunnel->label_num))
        || p_tunnel_param->nh_param.is_sr != CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME) !=
            p_tunnel_param->nh_param.is_spme)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mpls tunnel SPME conflict \n");
        return CTC_E_INVALID_PARAM;
    }

	if((p_tunnel_param->nh_param.label_num == 0)
        && (!CTC_FLAG_ISSET(p_tunnel_param->aps_flag, CTC_NH_MPLS_APS_EN)
          ||(CTC_FLAG_ISSET(p_tunnel_param->aps_flag, CTC_NH_MPLS_APS_EN)
               && (p_tunnel_param->nh_p_param.label_num == 0))))
    {

		CTC_ERROR_RETURN(sys_usw_mpls_nh_update_tunnel_mac_fwd_attr(lchip, FALSE,
                                                                           tunnel_id,
                                                                           p_tunnel_param));
    }
    else
    {
		CTC_ERROR_RETURN(sys_usw_mpls_nh_update_tunnel_label_fwd_attr(lchip, FALSE,
                                                                           tunnel_id,
                                                                           p_tunnel_param));
	}

      return CTC_E_NONE;

  }


int32
sys_usw_mpls_nh_swap_tunnel(uint8 lchip, uint16 old_tunnel_id, uint16 new_tunnel_id)
{
    sys_nh_db_mpls_tunnel_t* p_old_mpls_tunnel;
    sys_nh_db_mpls_tunnel_t* p_new_mpls_tunnel;
    sys_nh_db_mpls_tunnel_t tmp_mpls_tunnel;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, old_tunnel_id, &p_old_mpls_tunnel));
    if (!p_old_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }

    CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, new_tunnel_id, &p_new_mpls_tunnel));
    if (!p_new_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }

    sal_memset(&tmp_mpls_tunnel, 0, sizeof(sys_nh_db_mpls_tunnel_t));

     /*swap tunnel soft-table*/
    sal_memcpy(&tmp_mpls_tunnel,p_old_mpls_tunnel, sizeof(sys_nh_db_mpls_tunnel_t));

    sal_memcpy(p_old_mpls_tunnel,p_new_mpls_tunnel, sizeof(sys_nh_db_mpls_tunnel_t));
    p_old_mpls_tunnel->tunnel_id = old_tunnel_id;

    sal_memcpy(p_new_mpls_tunnel,&tmp_mpls_tunnel, sizeof(sys_nh_db_mpls_tunnel_t));
    p_new_mpls_tunnel->tunnel_id = new_tunnel_id;

    p_new_mpls_tunnel->ref_cnt = p_old_mpls_tunnel->ref_cnt;
    p_old_mpls_tunnel->ref_cnt = tmp_mpls_tunnel.ref_cnt;


    p_new_mpls_tunnel->stats_ptr = p_old_mpls_tunnel->stats_ptr;
    p_old_mpls_tunnel->stats_ptr = tmp_mpls_tunnel.stats_ptr;
    p_new_mpls_tunnel->p_stats_ptr = p_old_mpls_tunnel->p_stats_ptr;
    p_old_mpls_tunnel->p_stats_ptr = tmp_mpls_tunnel.p_stats_ptr;


    return CTC_E_NONE;
}


