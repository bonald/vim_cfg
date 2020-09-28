/**
 @file sys_usw_nexthop.c

 @date 2009-09-16

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
#include "ctc_common.h"
#include "ctc_linklist.h"

#include "ctc_packet.h"
#include "ctc_warmboot.h"

#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_qos_api.h"

#include "sys_usw_vlan.h"
#include "sys_usw_l3if.h"
#include "sys_usw_ftm.h"
#include "sys_usw_aps.h"
#include "sys_usw_register.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_wb_nh.h"
#include "sys_usw_wb_common.h"

#include "drv_api.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

#define SWAP32(x) \
    ((uint32)( \
         (((uint32)(x) & (uint32)0x000000ffUL) << 24) | \
         (((uint32)(x) & (uint32)0x0000ff00UL) << 8) | \
         (((uint32)(x) & (uint32)0x00ff0000UL) >> 8) | \
         (((uint32)(x) & (uint32)0xff000000UL) >> 24)))

#define DS_ARRAY_T(size) struct { uint32 array[size/4];}

extern sys_usw_nh_api_master_t* p_usw_nh_api_master[CTC_MAX_LOCAL_CHIP_NUM];
extern int32 _sys_usw_cpu_reason_get_info(uint8 lchip, uint16 reason_id, uint32 *destmap);
extern int32 _sys_usw_get_sub_queue_id_by_cpu_reason(uint8 lchip, uint16 reason_id, uint8* sub_queue_id);
extern int32 _sys_usw_nh_update_tunnel_ref_info(uint8 lchip, sys_nh_info_mpls_t* p_nhinfo, uint16 tunnel_id, uint8 is_add);
extern int32 _sys_usw_nh_wb_traverse_sync_mcast(void *data,uint32 vec_index, void *user_data);
extern int32 _sys_usw_nh_wb_restore_mcast(uint8 lchip,  ctc_wb_query_t *p_wb_query);
/**********************************/
enum opf_nh_index
{
    OPF_NH_DYN1_SRAM, /*dynamic sram(may be DsFwd) */
    OPF_NH_DYN2_SRAM, /*dynamic sram(may be DsNexthop, DsL2Edit, DsL3Edit, etc) */
    OPF_NH_DYN3_SRAM, /*dynamic sram(may be DsMet) */
    OPF_NH_DYN4_SRAM, /*dynamic sram(may be DsNexthop, DsL2Edit, DsL3Edit, etc) */
    OPF_NH_NHID_INTERNAL,  /*internal NHID for DsNexthop*/
    OPF_NH_OUTER_L2EDIT,  /*dsl2outeredit for arp*/
    OPF_NH_OUTER_SPME,  /*dsl2outeredit for spme*/
    OPF_NH_L2EDIT_VLAN_PROFILE,  /*vlan action profile for dsl2edit*/
    OPF_NH_DESTMAP_PROFILE, /*arp destmap profile*/
    OPF_NH_DYN_MAX
};

#define SYS_USW_INVALID_STATS_PTR 0
#define SYS_USW_NH_DROP_DESTMAP          0xFFFE


#define SYS_NH_EXTERNAL_NHID_DEFAUL_NUM 0x80000000
#define SYS_NH_TUNNEL_ID_DEFAUL_NUM     1024

#define SYS_NH_ARP_ID_DEFAUL_NUM        1024

#define SYS_NH_MCAST_LOGIC_MAX_CNT       32

#define SYS_USW_NH_DSNH_SET_DEFAULT_COS_ACTION(_dsnh)    \
    do {                                                      \
       _dsnh.replace_ctag_cos = 0;\
       _dsnh.copy_ctag_cos= 0;    \
       _dsnh.derive_stag_cos= 1;\
       _dsnh.stag_cfi= 0;\
    } while (0)

#define SYS_USW_NH_DSNH_ASSIGN_MACDA(_dsnh, _dsnh_param)         \
    do {                                                               \
         sal_memcpy(_dsnh.mac_da, _dsnh_param.macDa, sizeof(mac_addr_t)); \
    } while (0)

#define SYS_USW_NH_DSNH_BUILD_L2EDITPTR_L3EDITPTR(__dsnh,     \
                                                        __dsnh_l2editptr, __dsnh_l3editptr)     \
    do {                                                            \
    } while (0)

#define SYS_USW_NH_DSNH8W_BUILD_L2EDITPTR_L3EDITPTR(__dsnh,   \
                                                          __dsnh_l2editptr, __dsnh_l3editptr)     \
    do {                                                            \
    } while (0)

#define SYS_USW_NH_DSNH_BUILD_FLAG(__dsnh,  flag)  \
    do {                                                                    \
        if (CTC_FLAG_ISSET(flag, SYS_NH_PARAM_FLAG_USE_TTL_FROM_PKT)) \
        { \
            __dsnh.use_packet_ttl = 1;\
        } \
        if (CTC_FLAG_ISSET(flag, SYS_NH_PARAM_FLAG_USE_MAPPED_STAG_COS)) \
        { \
            __dsnh.stag_cfi = 1; \
        } \
        if (CTC_FLAG_ISSET(flag, SYS_NH_PARAM_FLAG_USE_MAPPED_CTAG_COS)) \
        { \
            __dsnh.replace_ctag_cos = 1; \
        } \
        if (CTC_FLAG_ISSET(flag, SYS_NH_PARAM_FLAG_USE_MAPPED_DSCP)) \
        { \
            __dsnh.stag_cos |= 1 << 1; \
        } \
    } while (0)

#define SYS_USW_NH_MAX_ECMP_CHECK(max_ecmp)  \
    do {    \
        if ((max_ecmp%4 != 0) || (max_ecmp > 0 && max_ecmp < 16))    \
        {   \
            return CTC_E_INVALID_PARAM; \
        }   \
        else if (max_ecmp > SYS_USW_MAX_ECPN)   \
        {   \
            return CTC_E_INVALID_PARAM;    \
        }   \
    } while (0)

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

sys_usw_nh_master_t* g_usw_nh_master[CTC_MAX_LOCAL_CHIP_NUM] = { NULL };



/****************************************************************************
 *
* Function
*
*****************************************************************************/
int32
_sys_usw_nh_api_get_nhinfo_by_nhid(uint8 lchip, uint32 nhid, sys_usw_nh_master_t* p_nh_master,
                                         sys_nh_info_com_t** pp_nhinfo);


int32
_sys_usw_nh_alloc_offset_by_nhtype(uint8 lchip, sys_nh_param_com_t* p_nh_com_para, sys_nh_info_com_t* p_nhinfo);



int32
_sys_usw_nh_free_offset_by_nhinfo(uint8 lchip, sys_usw_nh_type_t nh_type, sys_nh_info_com_t* p_nhinfo);

extern int32
_sys_usw_nh_ipuc_update_dsnh_cb(uint8 lchip, uint32 nh_offset, void* p_arp_db);

#define __________db__________


/**
 @brief This function is used to get nexthop module master data
 */
int32
_sys_usw_nh_get_nh_master(uint8 lchip, sys_usw_nh_master_t** p_out_nh_master)
{
    LCHIP_CHECK(lchip);
    if (NULL == g_usw_nh_master[lchip])
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexhexthop not init \n");
			return CTC_E_NOT_INIT;

    }

    *p_out_nh_master = g_usw_nh_master[lchip];
    return CTC_E_NONE;

}

int32
sys_usw_nh_get_nh_master(uint8 lchip, sys_usw_nh_master_t** p_nh_master)
{
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, p_nh_master));
    return CTC_E_NONE;
}


int32
sys_usw_nh_get_nh_resource(uint8 lchip, uint32 type, uint32* used_count)
{
    uint32 index_end = 0;
    uint32 index_start = 0;
    uint32 index = 0;
    uint32 num = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    switch(type)
    {
        case SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W:
            index_start = SYS_NH_ENTRY_TYPE_L2EDIT_FROM;
            index_end = SYS_NH_ENTRY_TYPE_L2EDIT_TO;
            break;
        case SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W:
            index_start = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W;
            index_end = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W;
            break;
        case SYS_NH_ENTRY_TYPE_L3EDIT_FLEX:
            index_start = SYS_NH_ENTRY_TYPE_L3EDIT_FROM;
            index_end = SYS_NH_ENTRY_TYPE_L3EDIT_TO;
            break;
        case SYS_NH_ENTRY_TYPE_MET:
            index_start = SYS_NH_ENTRY_TYPE_MET;
            index_end = SYS_NH_ENTRY_TYPE_MET_6W;
            break;
        case SYS_NH_ENTRY_TYPE_NEXTHOP_4W:
            index_start = SYS_NH_ENTRY_TYPE_NEXTHOP_4W;
            index_end = SYS_NH_ENTRY_TYPE_NEXTHOP_8W;
            break;
         default:
            index_start = type;
            index_end = type;
    }

    for(index = index_start; index <= index_end; index++)
    {
        if((type != SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W) && (index == SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W || index == SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W))
        {
            continue;
        }
        num += p_nh_master->nhtbl_used_cnt[index];
    }

    if(type == SYS_NH_ENTRY_TYPE_MET)
    {
        num += p_nh_master->max_glb_met_offset;
    }
    else if(type == SYS_NH_ENTRY_TYPE_NEXTHOP_4W)
    {
        num += p_nh_master->max_glb_nh_offset;
    }

    *used_count = num;

    return CTC_E_NONE;
}

/**
 @brief This function is used to get nexthop module master data
 */
bool
_sys_usw_nh_is_ipmc_logic_rep_enable(uint8 lchip)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    return p_nh_master->ipmc_logic_replication ? TRUE : FALSE;
}




int32
_sys_usw_nh_set_ipmc_logic_replication(uint8 lchip, uint8 enable)
{

	sys_usw_nh_master_t* p_nh_master = NULL;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    p_nh_master->ipmc_logic_replication  = (enable) ? 1 : 0;

    return CTC_E_NONE;
}

int32
_sys_usw_nh_write_entry_dsl2editeth4w(uint8 lchip, uint32 offset,
                    sys_dsl2edit_t* p_ds_l2_edit_4w , uint8 type)
{


    if ((SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W == type) ||
        (SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP == type) ||
        (SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W == type) ||
        (SYS_NH_ENTRY_TYPE_L2EDIT_LPBK == type))
    {

        CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                            type,
                                                            offset,
                                                            p_ds_l2_edit_4w));
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_nh_write_entry_dsl2editeth8w(uint8 lchip, uint32 offset, sys_nh_db_dsl2editeth8w_t* p_ds_l2_edit_8w, uint8 type)
{

    sys_dsl2edit_t dsl2edit;

    sal_memset(&dsl2edit, 0, sizeof(sys_dsl2edit_t));

    if(SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W == type)
    {
        dsl2edit.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
        dsl2edit.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_8W;
        dsl2edit.offset = offset;
        dsl2edit.update_mac_sa = p_ds_l2_edit_8w->update_mac_sa;
        dsl2edit.strip_inner_vlan = p_ds_l2_edit_8w->strip_inner_vlan;
        dsl2edit.is_vlan_valid = (p_ds_l2_edit_8w->output_vid != 0);
        /* SYSTEM MODIFIED by yangl for OPENFLOW 20190826 */
        if (CTC_MAX_VLAN_ID == p_ds_l2_edit_8w->output_vid)
        {
            dsl2edit.is_vlan_valid = 0;
        }
        dsl2edit.packet_type = p_ds_l2_edit_8w->packet_type;
        dsl2edit.output_vlan_id= p_ds_l2_edit_8w->output_vid;
        dsl2edit.is_6w = 1;
        dsl2edit.dynamic = 1;
        dsl2edit.is_dot11 = p_ds_l2_edit_8w->is_dot11;
        dsl2edit.derive_mac_from_label = p_ds_l2_edit_8w->derive_mac_from_label;
        dsl2edit.derive_mcast_mac_for_mpls = p_ds_l2_edit_8w->derive_mcast_mac_for_mpls;
        dsl2edit.derive_mcast_mac_for_trill = p_ds_l2_edit_8w->derive_mcast_mac_for_trill;
        dsl2edit.map_cos = p_ds_l2_edit_8w->map_cos;
        dsl2edit.dot11_sub_type = p_ds_l2_edit_8w->dot11_sub_type;
        dsl2edit.derive_mcast_mac_for_ip = p_ds_l2_edit_8w->derive_mcast_mac_for_ip;
        dsl2edit.cos_domain = p_ds_l2_edit_8w->cos_domain;
        sal_memcpy(dsl2edit.mac_da, p_ds_l2_edit_8w->mac_da, sizeof(mac_addr_t));
        sal_memcpy(dsl2edit.mac_sa, p_ds_l2_edit_8w->mac_sa, sizeof(mac_addr_t));

        CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                            SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W,
                                                            offset,
                                                            &dsl2edit));
    }
    else if(SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W == type)
    {
        dsl2edit.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
        dsl2edit.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_8W;
        dsl2edit.offset = offset;
        dsl2edit.is_vlan_valid = (p_ds_l2_edit_8w->output_vid != 0);
        /* SYSTEM MODIFIED by yangl for OPENFLOW 20190826 */
        if (CTC_MAX_VLAN_ID == p_ds_l2_edit_8w->output_vid)
        {
            dsl2edit.is_vlan_valid = 0;
        }
        /* yangl end */
        dsl2edit.packet_type = p_ds_l2_edit_8w->packet_type;
        dsl2edit.output_vlan_id= p_ds_l2_edit_8w->output_vid;
        dsl2edit.ether_type = p_ds_l2_edit_8w->ether_type;
        dsl2edit.is_6w = 1;
        sal_memcpy(dsl2edit.mac_da, p_ds_l2_edit_8w->mac_da, sizeof(mac_addr_t));
        sal_memcpy(dsl2edit.mac_sa, p_ds_l2_edit_8w->mac_sa, sizeof(mac_addr_t));
        dsl2edit.update_mac_sa = p_ds_l2_edit_8w->update_mac_sa;
        CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                            SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W,
                                                            offset,
                                                            &dsl2edit));
    }
    else if(SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W == type)
    {
        dsl2edit.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
        dsl2edit.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_FLEX_8W;
        dsl2edit.offset = offset;
        /* The output_vlan_id and output_vid represent the gem_vlan*/
        dsl2edit.output_vlan_id = p_ds_l2_edit_8w->output_vid;
        /* The output_vlan_is_svlan and dynamic represent the logic_dest_port*/
        dsl2edit.ether_type = p_ds_l2_edit_8w->output_vlan_is_svlan<<8 | p_ds_l2_edit_8w->dynamic;
        CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                    SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W,
                                                    offset,
                                                    &dsl2edit));
    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_get_arp_oif(uint8 lchip, uint16 arp_id, ctc_nh_oif_info_t *p_oif, uint8* p_mac, uint8* is_drop)
{
    sys_nh_db_arp_t*  p_arp = NULL;
    if (0 != arp_id)
    {
        p_arp = sys_usw_nh_lkup_arp_id(lchip, arp_id);
        if (!p_arp)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
			return CTC_E_NOT_EXIST;

        }

        if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_VLAN_VALID))
        {
            p_oif->vid = p_arp->output_vid;
        }

        if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID))
        {
            p_oif->gport = p_arp->gport;
        }

        if (is_drop)
        {
            *is_drop = CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP)?1:0;
        }

        if (p_mac)
        {
            sal_memcpy(p_mac, p_arp->mac_da, sizeof(mac_addr_t));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_write_entry_arp(uint8 lchip, sys_nh_db_arp_t* p_ds_arp)
{
    uint32 cmd                     = 0;
    uint32 lport                   = 0;
    uint8 dest_gchip               = 0;
    uint32 dest_map                = 0;
    uint8 tbl_type = 0;
    uint32 offset = 0;
    uint8 is_drop = CTC_FLAG_ISSET(p_ds_arp->flag, SYS_NH_ARP_FLAG_DROP)?1:0;
    sys_dsl2edit_t   dsl2edit_4w;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Arp_Id:%u, DestMapProfile:%u\n", p_ds_arp->arp_id, p_ds_arp->destmap_profile);


    if (CTC_FLAG_ISSET(p_ds_arp->flag, SYS_NH_ARP_ECMP_IF))
    {
        hw_mac_addr_t mac_da;
        DsEgressPortMac_m        dsportmac;

        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_ds_arp->gport, lchip, lport);
        SYS_USW_SET_HW_MAC(mac_da,  p_ds_arp->mac_da);
        SetDsEgressPortMac(A, portMac_f, &dsportmac, mac_da);
        cmd = DRV_IOW(DsEgressPortMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dsportmac));

        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_ds_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))
    {
        /*Exception to cpu, special for arp to cpu reason*/
        dest_map = 0xFFFF;

    }
    else if(CTC_FLAG_ISSET(p_ds_arp->flag, SYS_NH_ARP_FLAG_DROP) && p_ds_arp->destmap_profile)
    {
        lport = SYS_RSV_PORT_DROP_ID;
        sys_usw_get_gchip_id(lchip, &dest_gchip);

        if (CTC_FLAG_ISSET(p_ds_arp->flag, SYS_NH_ARP_PORT_VALID))
        {
            p_ds_arp->gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(dest_gchip, lport);
            dest_map = SYS_ENCODE_DESTMAP(dest_gchip, SYS_RSV_PORT_DROP_ID);
        }
    }
    else
    {
        /*Set the output port*/
        if (!is_drop)
        {
            lport = CTC_MAP_GPORT_TO_LPORT(p_ds_arp->gport);
            dest_gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_ds_arp->gport);
            dest_map = SYS_ENCODE_DESTMAP(dest_gchip, lport);
        }
    }

    sal_memset(&dsl2edit_4w, 0, sizeof(dsl2edit_4w));
    sal_memcpy(dsl2edit_4w.mac_da, p_ds_arp->mac_da, sizeof(mac_addr_t));

    dsl2edit_4w.ds_type  = is_drop?SYS_NH_DS_TYPE_DISCARD:SYS_NH_DS_TYPE_L2EDIT;
    dsl2edit_4w.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_4W;
    dsl2edit_4w.is_vlan_valid = (p_ds_arp->output_vid != 0);
    dsl2edit_4w.output_vlan_id= p_ds_arp->output_vid;

    /*write outer static l2edit*/
    if (p_ds_arp->offset)
    {
        offset = p_ds_arp->offset;
        dsl2edit_4w.offset = offset;
        tbl_type = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W;
        CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth4w(lchip, offset, &dsl2edit_4w, tbl_type));
    }

    /*write inner dynamic l2edit*/
    if (p_ds_arp->in_l2_offset)
    {
        /*write l2edit*/
        offset = p_ds_arp->in_l2_offset;
        dsl2edit_4w.offset = offset;
        tbl_type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W;
        dsl2edit_4w.dynamic = 1;

            CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth4w(lchip, offset, &dsl2edit_4w, tbl_type));

    }


    if (p_ds_arp->destmap_profile)
    {
        cmd = DRV_IOW(DsDestMapProfileUc_t, DsDestMapProfileUc_destMap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_ds_arp->destmap_profile, cmd, &dest_map));

        cmd = DRV_IOW(DsDestMapProfileMc_t, DsDestMapProfileMc_destMap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_ds_arp->destmap_profile, cmd, &dest_map));

    }

    return CTC_E_NONE;

}

#define NH_L2EDIT_OUTER
int32
sys_usw_nh_add_route_l2edit_outer(uint8 lchip, sys_nh_db_dsl2editeth4w_t** p_dsl2edit_4w, uint32* p_offset)  /*DsL2Edit*/
{
    sys_nh_db_dsl2editeth4w_t* p_new_dsl2edit_4w = NULL;
    uint32 offset = 0;
    int32 ret = CTC_E_NONE;
    sys_nh_db_edit_t* p_db_out = NULL;
    sys_nh_db_edit_t l2edit_db;
    sys_usw_nh_master_t* p_master = NULL;
    sys_dsl2edit_t l2edit_param;
    uint8 mac_0[6] = {0};

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lkup dsl2edit_4w node, mac:%s!!\n",
                   sys_output_mac((*p_dsl2edit_4w)->mac_da));

    if ((*p_dsl2edit_4w)->is_ecmp_if)
    {
        sys_usw_nh_get_emcp_if_l2edit_offset(lchip, &offset);
        *p_offset = offset;
        return CTC_E_NONE;
    }

    sal_memset(&l2edit_param, 0, sizeof(sys_dsl2edit_t));
    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W;
    l2edit_db.offset = *p_offset;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    sal_memcpy(&l2edit_db.data, *p_dsl2edit_4w, sizeof(sys_nh_db_dsl2editeth4w_t));
    CTC_ERROR_RETURN(ctc_spool_add(p_master->p_edit_spool, &l2edit_db, NULL, &p_db_out));

    p_new_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&(l2edit_db.data);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", p_db_out->offset);

    l2edit_param.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
    l2edit_param.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_4W;
    l2edit_param.is_vlan_valid = (p_new_dsl2edit_4w->output_vid != 0);
    l2edit_param.output_vlan_id= p_new_dsl2edit_4w->output_vid;
    l2edit_param.offset = p_db_out->offset;
    l2edit_param.fpma = p_new_dsl2edit_4w->fpma;
    if (!sal_memcmp(p_new_dsl2edit_4w->mac_da, mac_0, sizeof(mac_addr_t)))
    {
        l2edit_param.derive_mcast_mac_for_ip = 1;
    }
    sal_memcpy(l2edit_param.mac_da, p_new_dsl2edit_4w->mac_da, sizeof(mac_addr_t));
    ret = _sys_usw_nh_write_entry_dsl2editeth4w(lchip, p_db_out->offset, &l2edit_param,SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W);
    if (ret != CTC_E_NONE)
    {
        goto error1;
    }

    *p_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&(p_db_out->data);
    *p_offset = p_db_out->offset;

    return CTC_E_NONE;

error1:
    ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL);
    return ret;
}

int32
sys_usw_nh_remove_route_l2edit_outer(uint8 lchip, sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_edit_t l2edit_db;

    if (p_dsl2edit_4w->is_ecmp_if)
    {
        return CTC_E_NONE;
    }

    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W;
    sal_memcpy(&l2edit_db.data, p_dsl2edit_4w, sizeof(sys_nh_db_dsl2editeth4w_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL));

    return CTC_E_NONE;
}

int32
sys_usw_nh_add_route_l2edit_8w_outer(uint8 lchip, sys_nh_db_dsl2editeth8w_t** p_dsl2edit_8w, uint32* p_offset)  /*DsL2Edit*/
{
    sys_nh_db_dsl2editeth8w_t* p_new_dsl2edit_8w = NULL;
    uint32 offset = 0;
    int32 ret = CTC_E_NONE;
    sys_nh_db_edit_t l2edit_db;
    sys_nh_db_edit_t* p_db_out = NULL;
    sys_usw_nh_master_t* p_master = NULL;
    uint8 mac_0[6] = {0};

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lkup dsl2edit_8w node, mac da:%s , mac sa:%s!!\n",
                   sys_output_mac((*p_dsl2edit_8w)->mac_da), sys_output_mac((*p_dsl2edit_8w)->mac_sa));

    if ((*p_dsl2edit_8w)->is_ecmp_if)
    {
        sys_usw_nh_get_emcp_if_l2edit_offset(lchip, &offset);
        *p_offset = offset;
        return CTC_E_NONE;
    }

    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W;
    l2edit_db.offset = *p_offset;
    sal_memcpy(&l2edit_db.data, *p_dsl2edit_8w, sizeof(sys_nh_db_dsl2editeth8w_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_add(p_master->p_edit_spool, &l2edit_db, NULL, &p_db_out));

    p_new_dsl2edit_8w = (sys_nh_db_dsl2editeth8w_t*)&(l2edit_db.data);

    if (!sal_memcmp(p_new_dsl2edit_8w->mac_da, mac_0, sizeof(mac_addr_t)))
    {
        p_new_dsl2edit_8w->derive_mcast_mac_for_ip = 1;
    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", p_db_out->offset);
    ret = _sys_usw_nh_write_entry_dsl2editeth8w(lchip, p_db_out->offset, p_new_dsl2edit_8w, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W);
    if (ret != CTC_E_NONE)
    {
        goto error1;
    }

    *p_offset = p_db_out->offset;
    *p_dsl2edit_8w = (sys_nh_db_dsl2editeth8w_t*)&(p_db_out->data);
    return CTC_E_NONE;

error1:
    ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL);
    return ret;
}

int32
sys_usw_nh_remove_route_l2edit_8w_outer(uint8 lchip, sys_nh_db_dsl2editeth8w_t* p_dsl2edit_8w)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_edit_t l2edit_db;

    if (p_dsl2edit_8w->is_ecmp_if)
    {
        return CTC_E_NONE;
    }

    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W;
    sal_memcpy(&l2edit_db.data, p_dsl2edit_8w, sizeof(sys_nh_db_dsl2editeth8w_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL));

    return CTC_E_NONE;
}

int32
sys_usw_nh_add_loopback_l2edit(uint8 lchip, uint32 nhid, uint8 is_l2edit, uint32* p_edit_ptr)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    sys_nh_info_com_t* p_nh_com_info;
    int32 ret = CTC_E_NONE;
    uint32 new_offset = 0;
    sys_dsl2edit_t l2edit_param;
    sys_nh_info_dsnh_t nhinfo;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&l2edit_param, 0, sizeof(sys_dsl2edit_t));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));

    CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
    {
        p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
        if (p_eloop_node->nhid == nhid)
        {
            *p_edit_ptr = p_eloop_node->edit_ptr;
            p_eloop_node->ref_cnt++;
            return CTC_E_NONE;
        }
    }

    p_eloop_node =  mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_eloop_node_t));
    if (NULL == p_eloop_node)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_eloop_node, 0, sizeof(sys_nh_eloop_node_t));
    p_eloop_node->nhid = nhid;

    CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, &new_offset), ret, Error0);
    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info), ret, Error1);
    if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        l2edit_param.ds_type  = SYS_NH_DS_TYPE_DISCARD;
    }
    else
    {
        l2edit_param.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
    }

    CTC_ERROR_GOTO(_sys_usw_nh_get_nhinfo(lchip, nhid, &nhinfo),ret, Error1);
    l2edit_param.l2_rewrite_type = SYS_NH_L2EDIT_TYPE_LOOPBACK;
    l2edit_param.dest_map = nhinfo.dest_map;
    l2edit_param.nexthop_ptr = (nhinfo.dsnh_offset&(~SYS_NH_DSNH_BY_PASS_FLAG));
    l2edit_param.nexthop_ext = nhinfo.nexthop_ext;

    CTC_ERROR_GOTO(_sys_usw_nh_write_entry_dsl2editeth4w(lchip, new_offset, &l2edit_param, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK),ret, Error1);

    p_eloop_node->edit_ptr = new_offset;
    p_eloop_node->ref_cnt++;
    ctc_slist_add_tail(g_usw_nh_master[lchip]->eloop_list, &(p_eloop_node->head));
    *p_edit_ptr = new_offset;

    CTC_SET_FLAG(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED);

    return CTC_E_NONE;

Error1:
    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, new_offset);
Error0:
    mem_free(p_eloop_node);

    return ret;
}

int32
sys_usw_nh_remove_loopback_l2edit(uint8 lchip, uint32 nhid, uint8 is_l2edit)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    ctc_slistnode_t* ctc_slistnode_new = NULL;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    sys_nh_info_com_t* p_nh_com_info;

    CTC_SLIST_LOOP_DEL(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode, ctc_slistnode_new)
    {
        p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
        if (p_eloop_node->nhid == nhid)
        {
            p_eloop_node->ref_cnt--;
            if (0 == p_eloop_node->ref_cnt)
            {
                sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, p_eloop_node->edit_ptr);
                ctc_slist_delete_node(g_usw_nh_master[lchip]->eloop_list, &(p_eloop_node->head));
                mem_free(p_eloop_node);

                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info));
                CTC_UNSET_FLAG(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED);
            }
            return CTC_E_NONE;
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_loopback_l2edit(uint8 lchip, uint32 nhid)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    sys_nh_info_com_t* p_nh_com_info;
    sys_dsl2edit_t l2edit_param;
    sys_nh_info_dsnh_t nhinfo;

    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
    {
        p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
        if (p_eloop_node->nhid == nhid)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info));
            if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
            {
                l2edit_param.ds_type = SYS_NH_DS_TYPE_DISCARD;
            }
            else
            {
                l2edit_param.ds_type = SYS_NH_DS_TYPE_L2EDIT;
            }

            CTC_ERROR_RETURN(_sys_usw_nh_get_nhinfo(lchip, nhid, &nhinfo));

            l2edit_param.l2_rewrite_type = SYS_NH_L2EDIT_TYPE_LOOPBACK;
            l2edit_param.dest_map = nhinfo.dest_map;
            l2edit_param.nexthop_ptr = (nhinfo.dsnh_offset&(~SYS_NH_DSNH_BY_PASS_FLAG));
            l2edit_param.nexthop_ext = nhinfo.nexthop_ext;

            CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth4w(lchip, p_eloop_node->edit_ptr, &l2edit_param, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK));
            return CTC_E_NONE;
        }
    }

    return CTC_E_NONE;
}


#define NH_L2EDIT_INNER
int32
sys_usw_nh_add_l2edit_6w_inner(uint8 lchip, sys_nh_db_dsl2editeth8w_t** p_dsl2edit_8w, uint32* p_offset)  /*DsL2Edit*/
{
    sys_nh_db_dsl2editeth8w_t* p_new_dsl2edit_8w = NULL;
    int32 ret = CTC_E_NONE;
    sys_nh_db_edit_t l2edit_db;
    sys_nh_db_edit_t* p_db_out = NULL;
    sys_usw_nh_master_t* p_master = NULL;

    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W;
    l2edit_db.offset = *p_offset;
    sal_memcpy(&l2edit_db.data, *p_dsl2edit_8w, sizeof(sys_nh_db_dsl2editeth8w_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    CTC_ERROR_RETURN(ctc_spool_add(p_master->p_edit_spool, &l2edit_db, NULL, &p_db_out));

    p_new_dsl2edit_8w = (sys_nh_db_dsl2editeth8w_t*)&l2edit_db.data;
    sal_memcpy(p_new_dsl2edit_8w, (*p_dsl2edit_8w), sizeof(sys_nh_db_dsl2editeth8w_t));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", p_db_out->offset);

    ret = _sys_usw_nh_write_entry_dsl2editeth8w(lchip, p_db_out->offset, p_new_dsl2edit_8w, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W);
    if (ret != CTC_E_NONE)
    {
        goto error;
    }

    *p_dsl2edit_8w = (sys_nh_db_dsl2editeth8w_t*)&(p_db_out->data);

    *p_offset = p_db_out->offset;

    return CTC_E_NONE;

error:
    ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL);
    return ret;
}

int32
sys_usw_nh_remove_l2edit_6w_inner(uint8 lchip, sys_nh_db_dsl2editeth8w_t* p_dsl2edit_8w)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_edit_t l2edit_db;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W;
    sal_memcpy(&l2edit_db.data, p_dsl2edit_8w, sizeof(sys_nh_db_dsl2editeth8w_t));

    CTC_ERROR_RETURN(ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL));

    return CTC_E_NONE;
}

int32
sys_usw_nh_add_l2edit_3w_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t** p_dsl2edit_4w, uint8 type, uint32* p_offset)  /*DsL2Edit*/
{
    sys_nh_db_dsl2editeth4w_t* p_new_dsl2edit_4w = NULL;
    uint32 offset = 0;
    int32 ret = CTC_E_NONE;
    sys_nh_db_edit_t l2edit_db;
    sys_nh_db_edit_t* p_db_out = NULL;
    sys_usw_nh_master_t* p_master = NULL;
    sys_dsl2edit_t l2edit_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lkup dsl2edit_4w node, mac:%s!!\n",
                   sys_output_mac((*p_dsl2edit_4w)->mac_da));

    if ((*p_dsl2edit_4w)->is_ecmp_if)
    {
        sys_usw_nh_get_emcp_if_l2edit_offset(lchip, &offset);
        *p_offset = offset;
        return CTC_E_NONE;
    }

    sal_memset(&l2edit_param, 0, sizeof(sys_dsl2edit_t));
    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = type;
    l2edit_db.offset = *p_offset;
    sal_memcpy(&l2edit_db.data, *p_dsl2edit_4w, sizeof(sys_nh_db_dsl2editeth4w_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_add(p_master->p_edit_spool, &l2edit_db, NULL, &p_db_out));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", p_db_out->offset);

    p_new_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&l2edit_db.data;

    l2edit_param.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
    l2edit_param.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_4W;
    l2edit_param.offset = p_db_out->offset;
    l2edit_param.dynamic = 1;
    l2edit_param.is_vlan_valid = (p_new_dsl2edit_4w->output_vid != 0);
    l2edit_param.output_vlan_id = p_new_dsl2edit_4w->output_vid;
    l2edit_param.is_6w = 0;
    l2edit_param.is_dot11 = p_new_dsl2edit_4w->is_dot11;
    l2edit_param.derive_mac_from_label = p_new_dsl2edit_4w->derive_mac_from_label;
    l2edit_param.derive_mcast_mac_for_mpls = p_new_dsl2edit_4w->derive_mcast_mac_for_mpls;
    l2edit_param.derive_mcast_mac_for_trill = p_new_dsl2edit_4w->derive_mcast_mac_for_trill;
    l2edit_param.map_cos = p_new_dsl2edit_4w->map_cos;
    l2edit_param.dot11_sub_type = p_new_dsl2edit_4w->dot11_sub_type;
    l2edit_param.derive_mcast_mac_for_ip = p_new_dsl2edit_4w->derive_mcast_mac_for_ip;
    sal_memcpy(l2edit_param.mac_da, p_new_dsl2edit_4w->mac_da, sizeof(mac_addr_t));

    ret = _sys_usw_nh_write_entry_dsl2editeth4w(lchip, p_db_out->offset, &l2edit_param, type);
    if (ret != CTC_E_NONE)
    {
        goto error1;
    }

    *p_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&(p_db_out->data);
    *p_offset = p_db_out->offset;

    return CTC_E_NONE;

error1:
    ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL);
    return ret;
}


int32
sys_usw_nh_remove_l2edit_3w_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w, uint8 l2_edit_type)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_edit_t l2edit_db;

    CTC_PTR_VALID_CHECK(p_dsl2edit_4w);
    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = l2_edit_type;
    sal_memcpy(&l2edit_db.data, p_dsl2edit_4w, sizeof(sys_nh_db_dsl2editeth4w_t));
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL));

    return CTC_E_NONE;
}


int32
sys_usw_nh_add_swap_l2edit_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t** p_dsl2edit_4w, uint32* p_offset)  /*DsL2Edit*/
{
    sys_nh_db_dsl2editeth4w_t* p_new_dsl2edit_4w = NULL;
    int32 ret = CTC_E_NONE;
    sys_nh_db_edit_t l2edit_db;
    sys_nh_db_edit_t* p_db_out = NULL;
    sys_usw_nh_master_t* p_master = NULL;
    sys_dsl2edit_t dsl2edit_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lkup dsl2edit_4w node, mac:%s!!\n",
                   sys_output_mac((*p_dsl2edit_4w)->mac_da));


    sal_memset(&dsl2edit_param, 0, sizeof(sys_dsl2edit_t));
    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP;
    l2edit_db.offset = *p_offset;
    sal_memcpy(&l2edit_db.data, *p_dsl2edit_4w, sizeof(sys_nh_db_dsl2editeth4w_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_add(p_master->p_edit_spool, &l2edit_db, NULL, &p_db_out));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", p_db_out->offset);

    p_new_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&(l2edit_db.data);

    dsl2edit_param.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
    dsl2edit_param.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_INNER_SWAP;
    dsl2edit_param.offset = p_db_out->offset;
    dsl2edit_param.strip_inner_vlan = p_new_dsl2edit_4w->strip_inner_vlan;
    sal_memcpy(dsl2edit_param.mac_da, p_new_dsl2edit_4w->mac_da, sizeof(mac_addr_t));

    ret = _sys_usw_nh_write_entry_dsl2editeth4w(lchip, p_db_out->offset, &dsl2edit_param,SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP);
    if (ret != CTC_E_NONE)
    {
        goto error1;
    }

    *p_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&(p_db_out->data);
    *p_offset = p_db_out->offset;
    return CTC_E_NONE;

error1:
    ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL);
    return ret;
}

int32
sys_usw_nh_remove_swap_l2edit_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_edit_t l2edit_db;

    sal_memset(&l2edit_db, 0, sizeof(sys_nh_db_edit_t));
    l2edit_db.edit_type = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP;
    sal_memcpy(&l2edit_db.data, p_dsl2edit_4w, sizeof(sys_nh_db_dsl2editeth4w_t));
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_remove(p_master->p_edit_spool, &l2edit_db, NULL));

    return CTC_E_NONE;
}

#define NH_L3EDIT

STATIC int32
_sys_usw_nh_get_l3edit_tunnel_size(uint8 lchip, uint8 edit_type, uint8* size)
{
    if (edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4)
    {
        *size = sizeof(sys_dsl3edit_tunnelv4_t);
    }
    else if(edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6)
    {
        *size = sizeof(sys_dsl3edit_tunnelv6_t);
    }
    else if(edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W)
    {
        *size = sizeof(sys_dsl3edit_nat4w_t);
    }
    else if(edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W)
    {
        *size = sizeof(sys_dsl3edit_nat8w_t);
    }
    else if((edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_SPME)||(edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_MPLS))
    {
        *size = sizeof(sys_dsmpls_t);
    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "l3 edit tunnel do not support edit_type:%d !!\n", edit_type);
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
sys_usw_nh_add_l3edit_tunnel(uint8 lchip, void** p_dsl3edit, uint8 edit_type, uint32* p_offset)  /*DsL3Edit*/
{
    int32 ret = CTC_E_NONE;
    sys_nh_db_edit_t edit_db;
    sys_nh_db_edit_t* p_db_out = NULL;
    void* p_new_edit = NULL;
    sys_usw_nh_master_t* p_master = NULL;
    uint8 type_size = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&edit_db, 0, sizeof(sys_nh_db_edit_t));

    CTC_ERROR_RETURN(_sys_usw_nh_get_l3edit_tunnel_size(lchip, edit_type, &type_size))

    sal_memcpy(&edit_db.data, (uint8*)(*p_dsl3edit), type_size);

    edit_db.edit_type = edit_type;
    edit_db.offset = *p_offset;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_add(p_master->p_edit_spool, &edit_db, NULL, &p_db_out));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3edit offset=%d\n", p_db_out->offset);

    p_new_edit = (void*)&(edit_db.data);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "edit offset=%d\n", p_db_out->offset);

    ret = sys_usw_nh_write_asic_table(lchip, edit_type, p_db_out->offset, p_new_edit);
    if (ret != CTC_E_NONE)
    {
        goto error1;
    }

    *p_dsl3edit = (void*)&(p_db_out->data);
    *p_offset = p_db_out->offset;

    return CTC_E_NONE;

error1:
    ctc_spool_remove(p_master->p_edit_spool, &edit_db, NULL);
    return ret;
}

int32
sys_usw_nh_remove_l3edit_tunnel(uint8 lchip, void* p_dsl3edit, uint8 edit_type)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_edit_t edit_db;
    uint8 type_size = 0;

    sal_memset(&edit_db, 0, sizeof(sys_nh_db_edit_t));
    edit_db.edit_type = edit_type;

    CTC_ERROR_RETURN(_sys_usw_nh_get_l3edit_tunnel_size(lchip, edit_type, &type_size))
    sal_memcpy(&edit_db.data, (uint8*)(p_dsl3edit), type_size);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    CTC_ERROR_RETURN(ctc_spool_remove(p_master->p_edit_spool, &edit_db, NULL));

    return CTC_E_NONE;
}

#define NH_MPLS_TUNNEL
int32
sys_usw_nh_lkup_mpls_tunnel(uint8 lchip, uint16 tunnel_id, sys_nh_db_mpls_tunnel_t** pp_mpls_tunnel)
{

    sys_usw_nh_master_t* p_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (tunnel_id >= p_master->max_tunnel_id)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Exceed max tunnel id, tunnel id:%u \n", tunnel_id);
        return CTC_E_INVALID_PARAM;
    }

    *pp_mpls_tunnel = ctc_vector_get(p_master->tunnel_id_vec, tunnel_id);

    return CTC_E_NONE;

}

/**
 @brief Insert node into AVL tree
 */
int32
sys_usw_nh_add_mpls_tunnel(uint8 lchip, uint16 tunnel_id, sys_nh_db_mpls_tunnel_t* p_mpls_tunnel)
{
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (FALSE == ctc_vector_add(p_master->tunnel_id_vec, tunnel_id, p_mpls_tunnel))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    return ret;
}

int32
sys_usw_nh_remove_mpls_tunnel(uint8 lchip, uint16 tunnel_id)
{

    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (FALSE == ctc_vector_del(p_master->tunnel_id_vec, tunnel_id))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    return ret;
}

int32
sys_usw_nh_check_mpls_tunnel_exp(uint8 lchip, uint16 tunnel_id, uint8* exp_type)
{
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    DsL3EditMpls3W_m dsmpls;
    sys_dsmpls_t sys_mpls;

    uint8 table_type = 0;
    uint8 i = 0;
    uint8 j = 0;

    sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel);
    if (NULL == p_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Tunnel label not exist\n");
			return CTC_E_NOT_EXIST;

    }

    /*check lsp*/
    table_type = SYS_NH_ENTRY_TYPE_L3EDIT_MPLS;

    for (i = 0; i < 2; i++)
    {

        if (p_mpls_tunnel->lsp_offset[i])
        {
            sal_memset(&dsmpls, 0, sizeof(DsL3EditMpls3W_m));
            sal_memset(&sys_mpls, 0, sizeof(sys_dsmpls_t));
            sys_usw_nh_get_asic_table(lchip,
                                     table_type, p_mpls_tunnel->lsp_offset[i], &dsmpls);


            sys_mpls.derive_exp = GetDsL3EditMpls3W(V, deriveLabel_f,    &dsmpls);
            sys_mpls.exp = GetDsL3EditMpls3W(V, uMplsPhb_gRaw_labelTc_f,          &dsmpls);

            if((0 == sys_mpls.exp)
                && (1 == sys_mpls.derive_exp))
            {
                *exp_type = 1;
                return CTC_E_NONE;
            }
        }
    }

    /*check spme*/
    table_type = SYS_NH_ENTRY_TYPE_L3EDIT_SPME;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if(p_mpls_tunnel->spme_offset[i][j])
            {
                DsL3Edit6W3rd_m l3edit6w;

                sal_memset(&l3edit6w, 0, sizeof(DsL3Edit6W3rd_m));
                sal_memset(&sys_mpls, 0, sizeof(sys_dsmpls_t));
                sys_usw_nh_get_asic_table(lchip, table_type, p_mpls_tunnel->spme_offset[i][j], &l3edit6w);

                sys_mpls.derive_exp = GetDsL3EditMpls3W(V, deriveLabel_f,    &l3edit6w);
                sys_mpls.exp = GetDsL3EditMpls3W(V, uMplsPhb_gRaw_labelTc_f,          &l3edit6w);

                if((0 == sys_mpls.exp)
                    && (1 == sys_mpls.derive_exp))
                {
                    *exp_type = 1;
                    return CTC_E_NONE;
                }
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_check_mpls_tunnel_is_spme(uint8 lchip, uint16 tunnel_id, uint8* spme_mode)
{
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;

    sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel);
    if (NULL == p_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Tunnel label not exist\n");
			return CTC_E_NOT_EXIST;

    }
    if (CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
    {
        *spme_mode = 1;
    }
    else
    {
        *spme_mode = 0;
    }
    return CTC_E_NONE;
}
#define ARP_ID

uint32
sys_usw_nh_get_arp_num(uint8 lchip)
{
    sys_usw_nh_master_t* p_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "\n%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));
    return p_master->arp_id_hash->count;
}

sys_nh_db_arp_t*
sys_usw_nh_lkup_arp_id(uint8 lchip, uint16 arp_id)
{
    sys_usw_nh_master_t* p_master = NULL;
    sys_nh_db_arp_t arp_db;

    sal_memset(&arp_db, 0, sizeof(sys_nh_db_arp_t));

    _sys_usw_nh_get_nh_master(lchip, &p_master);

    arp_db.arp_id = arp_id;

    return ctc_hash_lookup(p_master->arp_id_hash, &arp_db);
}


int32
sys_usw_nh_add_arp_id(uint8 lchip, sys_nh_db_arp_t* p_arp)
{
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "\n%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (FALSE == ctc_hash_insert(p_master->arp_id_hash, p_arp))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    return ret;
}

int32
sys_usw_nh_remove_arp_id(uint8 lchip, sys_nh_db_arp_t* p_arp)
{
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "\n%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (NULL == ctc_hash_remove(p_master->arp_id_hash, p_arp))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Not exist \n");
			return CTC_E_NOT_EXIST;

    }

    return ret;
}

#define NH_MISC

int32
_sys_usw_nh_set_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num, bool is_set)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32* p_bmp, curr_offset;
    int32 i;

    if (0 == entry_num)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if ((start_offset + entry_num - 1) > p_nh_master->max_glb_met_offset)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (NULL == (p_bmp = p_nh_master->p_occupid_met_offset_bmp))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
		return CTC_E_NOT_INIT;

    }

    for (i = 0; i < entry_num; i++)
    {
        curr_offset = start_offset + i;
        if (is_set)
        {
            CTC_SET_FLAG(p_bmp[(curr_offset >> BITS_SHIFT_OF_WORD)],
                         (1 << (curr_offset & BITS_MASK_OF_WORD)));
        }
        else
        {
            CTC_UNSET_FLAG(p_bmp[(curr_offset >> BITS_SHIFT_OF_WORD)],
                           (1 << (curr_offset & BITS_MASK_OF_WORD)));
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_check_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num,
                                      bool should_not_inuse)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32* p_bmp, curr_offset;
    uint32 i;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (NULL == (p_bmp = p_nh_master->p_occupid_met_offset_bmp))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
		return CTC_E_NOT_INIT;
    }

    if (0 == entry_num)
    {
        return CTC_E_NONE;
    }

    for (i = 0; i < entry_num; i++)
    {
        curr_offset = start_offset + i;
        if (should_not_inuse && CTC_FLAG_ISSET(p_bmp[((curr_offset) >> BITS_SHIFT_OF_WORD)],
                                               (1 << (curr_offset & BITS_MASK_OF_WORD))))
        {
			return CTC_E_IN_USE;

        }

        if ((!should_not_inuse) && (!CTC_FLAG_ISSET(p_bmp[((curr_offset) >> BITS_SHIFT_OF_WORD)],
                                                    (1 << (curr_offset & BITS_MASK_OF_WORD)))))
        {
			return CTC_E_IN_USE;

        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_get_max_ecmp(uint8 lchip, uint16* max_ecmp)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    *max_ecmp = p_nh_master->max_ecmp;

    return CTC_E_NONE;
}

int32
sys_usw_nh_set_max_ecmp(uint8 lchip, uint16 max_ecmp)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /* the max ecmp num should be times of 4 */
    SYS_USW_NH_MAX_ECMP_CHECK(max_ecmp);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (p_nh_master->cur_ecmp_cnt != 0)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] ECMP already in use max path can not be reset \n");
			return CTC_E_IN_USE;

    }

    p_nh_master->max_ecmp = max_ecmp;
    p_nh_master->max_ecmp_group_num = (0 == max_ecmp) ? MCHIP_CAP(SYS_CAP_NH_ECMP_GROUP_ID_NUM) : MCHIP_CAP(SYS_CAP_NH_ECMP_MEMBER_NUM) / max_ecmp;

    return CTC_E_NONE;
}
#if 0
int32
sys_usw_nh_get_dsfwd_mode(uint8 lchip, uint8* merge_dsfwd)
{

    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    *merge_dsfwd = p_nh_master->no_dsfwd;
    return CTC_E_NONE;
}
#endif
int32
sys_usw_nh_set_dsfwd_mode(uint8 lchip, uint8 merge_dsfwd)
{

    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

     p_nh_master->no_dsfwd =  merge_dsfwd;
    return CTC_E_NONE;
}


/**
 @brief This function is used to get l3ifid

 @param[in] nhid, nexthop id

 @param[out] p_l3ifid, l3 interface id

 @return CTC_E_XXX
 */
int32
sys_usw_nh_get_l3ifid(uint8 lchip, uint32 nhid, uint16* p_l3ifid)
{
    sys_nh_info_com_t* p_nhinfo;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_nh_info_ipuc_t* p_ipuc_info;
    uint16 arp_id = 0;

    CTC_PTR_VALID_CHECK(p_l3ifid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (nhid == SYS_NH_RESOLVED_NHID_FOR_DROP ||
        nhid == SYS_NH_RESOLVED_NHID_FOR_TOCPU)
    {
        *p_l3ifid = SYS_L3IF_INVALID_L3IF_ID;
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    CTC_ERROR_RETURN(
        _sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nhinfo));

    switch (p_nhinfo->hdr.nh_entry_type)
    {
    case SYS_NH_TYPE_IPUC:
        p_ipuc_info = (sys_nh_info_ipuc_t*)(p_nhinfo);
        *p_l3ifid = p_ipuc_info->l3ifid;
        arp_id = p_ipuc_info->arp_id;
        break;

    case SYS_NH_TYPE_MPLS:
        {
            sys_nh_info_mpls_t* p_mpls_info;
            p_mpls_info = (sys_nh_info_mpls_t*)(p_nhinfo);
            *p_l3ifid = p_mpls_info->l3ifid;
            arp_id = p_mpls_info->arp_id;
            break;
        }

    case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_ip_tunnel_info;
            p_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)(p_nhinfo);
            *p_l3ifid = p_ip_tunnel_info->l3ifid;
            break;
        }

    case SYS_NH_TYPE_MISC:
        {

            sys_nh_info_misc_t* p_misc_info;
            p_misc_info = (sys_nh_info_misc_t*)(p_nhinfo);
            *p_l3ifid = p_misc_info->l3ifid;
            break;
        }
    default:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
		return CTC_E_INVALID_CONFIG;

    }

    if (arp_id)
    {
        sys_nh_db_arp_t *p_arp = NULL;
        p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
        if (NULL == p_arp)
        {
            return CTC_E_NOT_EXIST;
        }

        if (p_arp && !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP) && !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU)
            && !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_ECMP_IF))
        {
            /*update l3if id*/
            *p_l3ifid = p_arp->l3if_id;
        }


    }

    return CTC_E_NONE;
}
#if 0
int32
sys_usw_nh_set_ip_use_l2edit(uint8 lchip, uint8 enable)
{

	sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    p_nh_master->ip_use_l2edit = (enable) ? 1 : 0;

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_ip_use_l2edit(uint8 lchip, uint8* enable)
{

	sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    *enable = p_nh_master->ip_use_l2edit;
    return CTC_E_NONE;
}
#endif
int32
sys_usw_nh_set_reflective_brg_en(uint8 lchip, uint8 enable)
{
	sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    p_nh_master->reflective_brg_en = enable ?1:0;
    return CTC_E_NONE;
}

int32
sys_usw_nh_get_reflective_brg_en(uint8 lchip, uint8 *enable)
{
	sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    *enable = p_nh_master->reflective_brg_en;
    return CTC_E_NONE;
}


int32
_sys_usw_nh_get_mcast_nh(uint8 lchip, uint32 group_id, uint32* nhid)
{
    sys_nh_info_mcast_t* p_mcast_db = NULL;
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    p_mcast_db =  ctc_vector_get(p_nh_master->mcast_group_vec, group_id);
    if (NULL == p_mcast_db)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
		return CTC_E_NOT_EXIST;
    }

    *nhid = p_mcast_db->hdr.nh_id;

    return CTC_E_NONE;
}

int32
sys_usw_nh_add_mcast_db(uint8 lchip, uint32 group_id, void* data)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (FALSE == ctc_vector_add(p_nh_master->mcast_group_vec, group_id, data))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_del_mcast_db(uint8 lchip, uint32 group_id)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (FALSE == ctc_vector_del(p_nh_master->mcast_group_vec, group_id))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    return CTC_E_NONE;
}

int32
_sys_usw_misc_nh_create(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nh_param_type)
{
    sys_nh_param_special_t nh_para;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    switch (nh_param_type)
    {
    case SYS_NH_TYPE_DROP:
    case SYS_NH_TYPE_TOCPU:
    case SYS_NH_TYPE_UNROV:
        break;

    default:
        CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
    }

    sal_memset(&nh_para, 0, sizeof(sys_nh_param_special_t));
    nh_para.hdr.nhid = nhid;
    nh_para.hdr.nh_param_type = nh_param_type;
    nh_para.hdr.is_internal_nh = FALSE;
	nh_para.hdr.have_dsfwd = TRUE;

    CTC_ERROR_RETURN(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_para)));
    return CTC_E_NONE;
}

int32
_sys_usw_nh_get_mcast_member(void* array_data, uint32 group_id, void* p_data)
{
    sys_nh_mcast_traverse_t* p_mcast_traverse = NULL;
    uint32 nhid = 0;
    sys_nh_info_com_t* p_com_db = NULL;
    sys_nh_info_mcast_t* p_mcast_db = NULL;
    sys_usw_nh_master_t* p_nh_master = NULL;
    ctc_nh_info_t nh_info;
    ctc_mcast_nh_param_member_t nh_member;
    uint8 lchip = 0;

    p_mcast_traverse = (sys_nh_mcast_traverse_t*)p_data;
    lchip = p_mcast_traverse->lchip;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    CTC_ERROR_RETURN(_sys_usw_nh_get_mcast_nh(lchip, group_id, &nhid));
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    CTC_ERROR_RETURN(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_com_db));
    p_mcast_db = (sys_nh_info_mcast_t*)p_com_db;
    SYS_NH_UNLOCK;
    sal_memset(&nh_info, 0, sizeof(ctc_nh_info_t));
    nh_info.buffer = &nh_member;
    nh_info.buffer_len = 1;
    do
    {
        sal_memset(&nh_member, 0, sizeof(ctc_mcast_nh_param_member_t));
        sys_usw_nh_get_mcast_member_info(lchip, p_mcast_db, &nh_info);
        p_mcast_traverse->fn(p_mcast_db->basic_met_offset, &nh_member, p_mcast_traverse->user_data);
        nh_info.start_index = nh_info.next_query_index;
    }while(!nh_info.is_end);
    SYS_NH_LOCK;

    return CTC_E_NONE;
}

int32
_sys_usw_nh_traverse_mcast(uint8 lchip, ctc_nh_mcast_traverse_fn fn, ctc_nh_mcast_traverse_t* p_data)
{
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_nh_master = NULL;
    vector_traversal_fn2 fn2 = NULL;
    sys_nh_mcast_traverse_t mcast_traverse;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    fn2 = _sys_usw_nh_get_mcast_member;
    sal_memset(&mcast_traverse, 0, sizeof(sys_nh_mcast_traverse_t));
    mcast_traverse.user_data = p_data->user_data;
    mcast_traverse.fn = fn;
    mcast_traverse.lchip = p_data->lchip_id;

    ctc_vector_traverse2(p_nh_master->mcast_group_vec, 0, fn2, &mcast_traverse);
    return ret;
}
#define ___ARP___

STATIC int32
_sys_usw_nh_arp_free_resource(uint8 lchip, sys_nh_db_arp_t* p_arp)
{

    if (p_arp->offset)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_arp->offset);
    }
    if (p_arp->in_l2_offset)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, p_arp->in_l2_offset);
    }

    if (p_arp->destmap_profile)
    {
        uint32 offset = p_arp->destmap_profile;
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE, 1, offset);
    }


    sys_usw_nh_remove_arp_id(lchip, p_arp);
    mem_free(p_arp);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_build_arp_info(uint8 lchip, sys_nh_db_arp_t* p_arp, ctc_nh_nexthop_mac_param_t* p_param)
{

    uint8 gchip = 0;
    /*db*/
    CTC_SET_FLAG(p_arp->flag, SYS_NH_ARP_PORT_VALID);
    sys_usw_get_gchip_id(lchip, &gchip);

    if (CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP))
    {
        CTC_SET_FLAG(p_arp->flag, SYS_NH_ARP_ECMP_IF);
    }
    else
    {
        CTC_UNSET_FLAG(p_arp->flag, SYS_NH_ARP_ECMP_IF);
    }

    if (CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_DISCARD))
    {
        CTC_SET_FLAG(p_arp->flag, SYS_NH_ARP_FLAG_DROP);
        p_param->gport = CTC_MAP_LPORT_TO_GPORT(gchip,SYS_RSV_PORT_DROP_ID);
    }
    else
    {
        CTC_UNSET_FLAG(p_arp->flag, SYS_NH_ARP_FLAG_DROP);
    }

    if (CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_REDIRECT_TO_CPU))
    {
        CTC_SET_FLAG(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU);
    }
    else
    {
        CTC_UNSET_FLAG(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU);
    }

    p_arp->gport = p_param->gport;
    sal_memcpy(p_arp->mac_da, p_param->mac, sizeof(mac_addr_t));

    if (CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_VLAN_VALID))
    {
        CTC_SET_FLAG(p_arp->flag, SYS_NH_ARP_VLAN_VALID);
        p_arp->output_vid = p_param->vlan_id;
        p_arp->output_vlan_is_svlan = 1;
    }
    else
    {
        CTC_UNSET_FLAG(p_arp->flag, SYS_NH_ARP_VLAN_VALID);
    }

    if(p_arp->l3if_type == CTC_L3IF_TYPE_PHY_IF && p_param->vlan_id)
    {
       CTC_SET_FLAG(p_arp->flag, SYS_NH_ARP_IPUC_L2_EDIT);
    }
    else
    {
       CTC_UNSET_FLAG(p_arp->flag, SYS_NH_ARP_IPUC_L2_EDIT);
    }

    return CTC_E_NONE;
}


/**
 @brief Add Next Hop Router Mac
*/
int32
_sys_usw_nh_add_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param)
{
    int32 ret = CTC_E_NONE;
    sys_nh_db_arp_t*  p_arp = NULL;
     sys_l3if_prop_t l3if;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_param);

    sal_memset(&l3if,0,sizeof(sys_l3if_prop_t));
    /*check flag*/
    if ( (CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_DISCARD)
            + CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_REDIRECT_TO_CPU)
            + CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP)) > 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (arp_id < 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_VLAN_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_param->vlan_id);
    }

    if (!CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP))
    {
        ret = sys_usw_l3if_get_l3if_info_with_port_and_vlan( lchip,  p_param->gport, p_param->vlan_id, &l3if);
        if (ret == CTC_E_NOT_EXIST)
        {
            if (!CTC_FLAG_ISSET(p_param->flag, CTC_NH_NEXTHOP_MAC_DISCARD))
            {
                return ret;
            }
            l3if.l3if_type = 0xff;  /*invalid l3if type*/
        }
    }

    /*alloc*/
    p_arp = sys_usw_nh_lkup_arp_id(lchip, arp_id);
    if (NULL == p_arp)
    {
        p_arp  = (sys_nh_db_arp_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_db_arp_t));
        if (NULL == p_arp)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;

        }
        sal_memset(p_arp, 0, sizeof(sys_nh_db_arp_t));

        /*alloc*/
        p_arp->arp_id = arp_id;
        p_arp->l3if_type = l3if.l3if_type;
        p_arp->l3if_id  = l3if.l3if_id;
        ret = _sys_usw_nh_build_arp_info(lchip, p_arp, p_param);

        if (CTC_E_NONE != ret)
        {
           goto error0;
        }

        ret = sys_usw_nh_add_arp_id(lchip, p_arp);
        if (CTC_E_NONE != ret)
        {
           goto error0;
        }

        ret = sys_usw_nh_write_entry_arp(lchip, p_arp);
        if (ret != CTC_E_NONE)
        {
            goto error1;
        }
        return CTC_E_NONE;

    }
    else
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

    }

error1:
    sys_usw_nh_remove_arp_id(lchip, p_arp);

error0:
    _sys_usw_nh_arp_free_resource(lchip, p_arp);
    return ret;

}

/**
 @brief Remove Next Hop Router Mac
*/
int32
_sys_usw_nh_remove_nexthop_mac(uint8 lchip, uint16 arp_id)
{
    sys_nh_db_arp_t*  p_arp = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (arp_id < 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
    if (!p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    if(p_arp->updateNhp || p_arp->ref_cnt)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Host nexthop already in use. \n");
			return CTC_E_IN_USE;

    }

    if (CTC_FLAG_ISSET(p_arp->flag , SYS_NH_ARP_ECMP_IF))
    {
        sal_memset(p_arp->mac_da, 0, sizeof(mac_addr_t));
        sys_usw_nh_write_entry_arp(lchip, p_arp);
    }

    _sys_usw_nh_arp_free_resource(lchip, p_arp);
    return CTC_E_NONE;

}

/**
 @brief Update Next Hop Router Mac
*/
int32
_sys_usw_nh_update_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param)
{
    sys_nh_db_arp_t*  p_arp = NULL;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_new_param);

    if ( (CTC_FLAG_ISSET(p_new_param->flag, CTC_NH_NEXTHOP_MAC_DISCARD)
            + CTC_FLAG_ISSET(p_new_param->flag, CTC_NH_NEXTHOP_MAC_REDIRECT_TO_CPU)
            + CTC_FLAG_ISSET(p_new_param->flag, CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP)) > 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (arp_id < 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_new_param->flag, CTC_NH_NEXTHOP_MAC_VLAN_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_new_param->vlan_id);
    }

    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
    if (NULL == p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Arp Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }
    else
    {
       if (!CTC_FLAG_ISSET(p_new_param->flag, CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP))
       {
            sys_l3if_prop_t l3if;
            sal_memset(&l3if, 0, sizeof(sys_l3if_prop_t));
            ret = sys_usw_l3if_get_l3if_info_with_port_and_vlan( lchip,  p_new_param->gport, p_new_param->vlan_id, &l3if);
            if (ret == CTC_E_NOT_EXIST)
            {
                if (!CTC_FLAG_ISSET(p_new_param->flag, CTC_NH_NEXTHOP_MAC_DISCARD))
                {
                    return ret;
                }
                l3if.l3if_type = 0xff;  /*invalid l3if type*/
            }

          if ((p_arp->l3if_id != l3if.l3if_id ||p_arp->l3if_type != l3if.l3if_type) && p_arp->ref_cnt)
          {
              SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Update L3 interface failed old l3if_id :%d new l3if_id:%d ,ref cnt:%u\n",
                            p_arp->l3if_id, l3if.l3if_id,p_arp->ref_cnt);
             return CTC_E_INVALID_PARAM;
          }
          p_arp->l3if_type = l3if.l3if_type;
          p_arp->l3if_id  = l3if.l3if_id;
       }

       CTC_ERROR_RETURN(_sys_usw_nh_build_arp_info(lchip, p_arp, p_new_param));
    }

    CTC_ERROR_RETURN(sys_usw_nh_write_entry_arp(lchip, p_arp));

    return CTC_E_NONE;

}
int32
sys_usw_nh_unbind_arp_cb(uint8 lchip, uint16 arp_id, uint8 use_cb)
{
   sys_nh_db_arp_t*  p_arp = NULL;
    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
    if (NULL == p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }
    if (use_cb)
    {
        p_arp->updateNhp = NULL;
    }
    p_arp->ref_cnt = (p_arp->ref_cnt)?(p_arp->ref_cnt-1):0;

    if (!p_arp->ref_cnt)
    {
        if ((p_arp->destmap_profile) && CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID))
        {
            sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE, 1,  p_arp->destmap_profile);
            p_arp->destmap_profile= 0;
        }
        if (p_arp->in_l2_offset)
        {
            sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, p_arp->in_l2_offset);
            p_arp->in_l2_offset = 0;
        }
        if (p_arp->offset)
        {
            sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_arp->offset);
            p_arp->offset = 0;
        }
    }

    return CTC_E_NONE;
}
int32
sys_usw_nh_bind_arp_cb(uint8 lchip, sys_nh_info_arp_param_t* p_arp_parm, uint16 arp_id)
{

    sys_nh_db_arp_t*  p_arp = NULL;
    int32 ret = CTC_E_NONE;
    uint8 use_arp_profile = 0;
    uint8 use_inner_l2_edit = 0;
    uint8 use_outer_l2_edit = 0;
    uint8 binding_cb = 0;

    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
    if (NULL == p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    p_arp->nh_entry_type = p_arp_parm->nh_entry_type;
    if (SYS_NH_TYPE_IPUC == p_arp_parm->nh_entry_type)
    {
        if((p_arp->updateNhp)&&(!p_arp_parm->is_aps))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Host nexthop already in use. \n");
            return CTC_E_IN_USE;
        }
        if(p_arp_parm->is_aps)
        {
            use_arp_profile = 1;
            use_inner_l2_edit = 1;
        }
        if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_IPUC_L2_EDIT))
        {
            use_inner_l2_edit = 1;
        }
        if ((NULL == p_arp->updateNhp)&&(!p_arp_parm->is_aps))
        {
            p_arp->nh_id = p_arp_parm->nh_id;
            p_arp->updateNhp = p_arp_parm->updateNhp;
            binding_cb = 1;
        }
    }
    else
    {
        use_arp_profile = 1;
        use_outer_l2_edit = 1;
    }

    if (use_arp_profile && (0 == p_arp->destmap_profile) && CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID))
    {
        use_arp_profile = 2;
        CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE, 1,  &p_arp->destmap_profile), ret, err);
    }
    if (use_inner_l2_edit && (0 == p_arp->in_l2_offset))
    {
        use_inner_l2_edit = 2;
        CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, &p_arp->in_l2_offset), ret, err);
    }
    if (use_outer_l2_edit && (0 == p_arp->offset))
    {
        use_outer_l2_edit = 2;
        CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, &p_arp->offset), ret, err);
    }
    CTC_ERROR_GOTO(sys_usw_nh_write_entry_arp(lchip, p_arp), ret, err);
    p_arp->ref_cnt++;
    return CTC_E_NONE;
err:
    if (2 == use_outer_l2_edit)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_arp->offset);
        p_arp->offset = 0;
    }
    if (2 == use_inner_l2_edit)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, p_arp->in_l2_offset);
        p_arp->in_l2_offset = 0;
    }
    if (2 == use_arp_profile)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE, 1,  p_arp->destmap_profile);
        p_arp->destmap_profile = 0;
    }
    if(binding_cb)
    {
        p_arp->nh_id = 0;
        p_arp->updateNhp = NULL;
    }

    return ret;
}

#define ___UDF___

int32
_sys_usw_nh_add_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit)
{
    uint8 index = 0;
    EpePktProcFlexTunnelCtl_m flex_ctl;
    uint32 cmd = 0;
    uint32 l4_type = 0;
    uint32 field_id = 0;
    uint8 field_step = 0;
    uint8 profile_id = p_edit->profile_id;
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32 dest_hw[4] = {0};

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "profile_id = %u, udf_offset_type = %u, udf_edit_length = %u\n",
        p_edit->profile_id, p_edit->offset_type, p_edit->edit_length);
    for (index = 0; index < CTC_NH_UDF_MAX_EDIT_NUM; index++)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "UDF edit index = %u , type = %u, offset = %u, length = 0x%x\n",
        index, p_edit->edit[index].type, p_edit->edit[index].offset, p_edit->edit[index].length);
    }

    /*parameter check*/
    if (!profile_id || profile_id > 8)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_MAX_VALUE_CHECK(p_edit->offset_type,CTC_NH_UDF_OFFSET_TYPE_RAW);

    if (!p_edit->edit_length || p_edit->edit_length > 16 || (p_edit->edit_length&0x1))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_edit->hdr_option_length >= 16)
    {
        return CTC_E_INVALID_PARAM;
    }

    for (index = 0; index < CTC_NH_UDF_MAX_EDIT_NUM; index++)
    {
        CTC_MAX_VALUE_CHECK(p_edit->edit[index].type, 3);
        if (p_edit->edit[index].type)
        {
            if (p_edit->edit[index].offset & 0x1)
            {
                return CTC_E_INVALID_PARAM;
            }

            if (p_edit->edit[index].length > 16 || !p_edit->edit[index].length)
            {
                return CTC_E_INVALID_PARAM;
            }

            if (p_edit->edit[index].offset +  p_edit->edit[index].length > (p_edit->edit_length*8))
            {
                return CTC_E_INVALID_PARAM;
            }

        }
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    if (CTC_IS_BIT_SET(p_nh_master->udf_profile_bitmap, profile_id))
    {
        return CTC_E_EXIST;
    }

    if (p_edit->offset_type==CTC_NH_UDF_OFFSET_TYPE_UDP)
    {
        l4_type = 1;
    }
    else if (p_edit->offset_type==CTC_NH_UDF_OFFSET_TYPE_GRE)
    {
        l4_type = 2;
    }
    else if (p_edit->offset_type==CTC_NH_UDF_OFFSET_TYPE_GRE_WITH_KEY)
    {
        l4_type = 3;
    }

    field_step = (EpePktProcFlexTunnelCtl_flexArray_1_existIpHeader_f - EpePktProcFlexTunnelCtl_flexArray_0_existIpHeader_f);

    cmd = DRV_IOR(EpePktProcFlexTunnelCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flex_ctl));
    field_id = EpePktProcFlexTunnelCtl_flexArray_0_existIpHeader_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, ((p_edit->offset_type==CTC_NH_UDF_OFFSET_TYPE_RAW)?0:1));

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_flexHdrLen_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, (p_edit->edit_length>>1));

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_l4HdrType_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, l4_type);

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_lenFieldDeltaValue_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, p_edit->hdr_option_length);

    for (index = 0; index < 4; index++)
    {
        /*- p_edit->udf_edit_data[index] = SWAP32(p_edit->udf_edit_data[index]);*/
    }

    sys_usw_dword_reverse_copy(dest_hw, p_edit->edit_data, (p_edit->edit_length+3)/4);

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_flexHdrTemplate_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_A(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, &dest_hw);

    for (index = 0; index < CTC_NH_UDF_MAX_EDIT_NUM; index++)
    {
        uint16 mask = 0;
        if (!p_edit->edit[index].type)
        {
            continue;
        }

        field_id = EpePktProcFlexTunnelCtl_flexArray_0_replaceTypeField1_f + index + (profile_id-1)*field_step;
        DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, p_edit->edit[index].type);
        field_id = EpePktProcFlexTunnelCtl_flexArray_0_replacePosField1_f + index + (profile_id-1)*field_step;
        DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, ((p_edit->edit_length*8-16-p_edit->edit[index].offset)>>1));
        field_id = EpePktProcFlexTunnelCtl_flexArray_0_flexHdrMaskField1_f + index + (profile_id-1)*field_step;

        mask = (1 << (p_edit->edit[index].length)) - 1;
        mask = ~mask;
        DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, mask);
    }
    cmd = DRV_IOW(EpePktProcFlexTunnelCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flex_ctl));

    CTC_BIT_SET(p_nh_master->udf_profile_bitmap, profile_id);
    p_nh_master->udf_ether_type[profile_id-1] = p_edit->ether_type;

    return CTC_E_NONE;
}

int32
_sys_usw_nh_remove_udf_profile(uint8 lchip, uint8 profile_id)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32 cmd = 0;
    EpePktProcFlexTunnelCtl_m flex_ctl;
    uint32 field_id = 0;
    uint8 field_step = 0;
    uint8 index = 0;

    if (!profile_id || profile_id > 8)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (p_nh_master->udf_profile_ref_cnt[profile_id-1])
    {
        return CTC_E_IN_USE;
    }

    cmd = DRV_IOR(EpePktProcFlexTunnelCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flex_ctl));

    field_step = (EpePktProcFlexTunnelCtl_flexArray_1_existIpHeader_f - EpePktProcFlexTunnelCtl_flexArray_0_existIpHeader_f);
    field_id = EpePktProcFlexTunnelCtl_flexArray_0_existIpHeader_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_flexHdrLen_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_l4HdrType_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);

    field_id = EpePktProcFlexTunnelCtl_flexArray_0_lenFieldDeltaValue_f + (profile_id-1)*field_step;
    DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);

    for (index = 0; index < CTC_NH_UDF_MAX_EDIT_NUM; index++)
    {
        field_id = EpePktProcFlexTunnelCtl_flexArray_0_replaceTypeField1_f + index + (profile_id-1)*field_step;
        DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);
        field_id = EpePktProcFlexTunnelCtl_flexArray_0_replacePosField1_f + index + (profile_id-1)*field_step;
        DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);
        field_id = EpePktProcFlexTunnelCtl_flexArray_0_flexHdrMaskField1_f + index + (profile_id-1)*field_step;
        DRV_SET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl, 0);
    }
    cmd = DRV_IOW(EpePktProcFlexTunnelCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flex_ctl));

    CTC_BIT_UNSET(p_nh_master->udf_profile_bitmap, profile_id);

    return CTC_E_NONE;
}

int32
_sys_usw_nh_get_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32 cmd = 0;
    EpePktProcFlexTunnelCtl_m flex_ctl;
    uint8 field_step = 0;
    uint32 value = 0;
    uint32 field_id = 0;
    uint8 profile_id = 0;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    CTC_MAX_VALUE_CHECK(p_edit->profile_id, 8);
    if (!CTC_IS_BIT_SET(p_nh_master->udf_profile_bitmap, p_edit->profile_id))
    {
        return CTC_E_NOT_EXIST;
    }

    profile_id = p_edit->profile_id-1;

    cmd = DRV_IOR(EpePktProcFlexTunnelCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flex_ctl));
    field_step = (EpePktProcFlexTunnelCtl_flexArray_1_existIpHeader_f - EpePktProcFlexTunnelCtl_flexArray_0_existIpHeader_f);
    field_id = EpePktProcFlexTunnelCtl_flexArray_0_existIpHeader_f + profile_id*field_step;

    value = DRV_GET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl);
    if (value)
    {
        field_id = EpePktProcFlexTunnelCtl_flexArray_0_l4HdrType_f + profile_id*field_step;
        value = DRV_GET_FIELD_V(lchip, EpePktProcFlexTunnelCtl_t, field_id, &flex_ctl);
        if (value == 1)
        {
            p_edit->offset_type = CTC_NH_UDF_OFFSET_TYPE_UDP;
        }
        else if (value == 2)
        {
            p_edit->offset_type = CTC_NH_UDF_OFFSET_TYPE_GRE;
        }
        else if (value == 3)
        {
            p_edit->offset_type = CTC_NH_UDF_OFFSET_TYPE_GRE_WITH_KEY;
        }
        else
        {
            p_edit->offset_type = CTC_NH_UDF_OFFSET_TYPE_IP;
        }

    }
    else
    {
        p_edit->offset_type = CTC_NH_UDF_OFFSET_TYPE_RAW;
    }

    return CTC_E_NONE;
}
#define _________api___________
/**
 @brief This function is to create normal bridge nexthop in different types

 @param[in] gport, global port id

 @param[in] nh_type, nexthop type

 @return CTC_E_XXX
 */
int32
_sys_usw_brguc_nh_create_by_type(uint8 lchip, uint32 gport, sys_nh_param_brguc_sub_type_t nh_type)
{
    sys_nh_param_brguc_t brguc_nh;
    sys_hbnh_brguc_node_info_t* p_brguc_node;
    sys_hbnh_brguc_node_info_t brguc_node_tmp;
    bool is_add = FALSE;
	int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = 0x%x , nh_type = %d\n", gport, nh_type);


    sal_memset(&brguc_nh, 0, sizeof(sys_nh_param_brguc_t));
    brguc_nh.hdr.nh_param_type = SYS_NH_TYPE_BRGUC;
    brguc_nh.hdr.nhid = SYS_NH_INVALID_NHID;
    brguc_nh.hdr.is_internal_nh = TRUE;
    brguc_nh.nh_sub_type = nh_type;
    brguc_nh.gport = gport;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "CtcGport = 0x%x ,DrvGport:0x%x, nh_type = %d, index:%d\n",
                                          gport, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport), nh_type, GPORT_TO_INDEX(gport));

    brguc_node_tmp.gport = gport;
    p_brguc_node = ctc_hash_lookup(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, &brguc_node_tmp);
    if (NULL == p_brguc_node)
    {
        p_brguc_node = (sys_hbnh_brguc_node_info_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_hbnh_brguc_node_info_t));
        if (NULL == p_brguc_node)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        sal_memset(p_brguc_node, 0, sizeof(sys_hbnh_brguc_node_info_t));
        p_brguc_node->gport = gport;
        p_brguc_node->nhid_brguc  = SYS_NH_INVALID_NHID;
        p_brguc_node->nhid_bypass  = SYS_NH_INVALID_NHID;
        is_add = TRUE;
    }

    if ((nh_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC)
        && (p_brguc_node->nhid_brguc != SYS_NH_INVALID_NHID))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
		return CTC_E_EXIST;
    }

    if ((nh_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS)
        && (p_brguc_node->nhid_bypass != SYS_NH_INVALID_NHID))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
		return CTC_E_EXIST;
    }

    ret = sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&brguc_nh));
	if(ret != CTC_E_NONE)
    {
        mem_free(p_brguc_node);
		return ret;
    }
    if (nh_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC)
    {
        p_brguc_node->nhid_brguc = brguc_nh.hdr.nhid;
    }

    if (nh_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS)
    {
        p_brguc_node->nhid_bypass = brguc_nh.hdr.nhid;
    }

    if (is_add)
    {
        if (FALSE == ctc_hash_insert(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, p_brguc_node))
        {
            if (p_brguc_node->nhid_brguc != SYS_NH_INVALID_NHID)
            {
                sys_usw_nh_api_delete(lchip, p_brguc_node->nhid_brguc, SYS_NH_TYPE_BRGUC);
            }

            if (p_brguc_node->nhid_bypass != SYS_NH_INVALID_NHID)
            {
                sys_usw_nh_api_delete(lchip, p_brguc_node->nhid_bypass, SYS_NH_TYPE_BRGUC);
            }

            mem_free(p_brguc_node);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_port(uint8 lchip, uint32 nhid, uint8* aps_en, uint32* gport)
{
    sys_nh_info_com_t* p_nhinfo;
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint8 gchip = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    CTC_ERROR_RETURN(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nhinfo));

    *aps_en = 0;

    switch (p_nhinfo->hdr.nh_entry_type)
    {
    case SYS_NH_TYPE_BRGUC:
        {
            sys_nh_info_brguc_t* p_brguc_info;
            p_brguc_info = (sys_nh_info_brguc_t*)p_nhinfo;
            if ((p_brguc_info->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC || p_brguc_info->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT
                 || p_brguc_info->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS))
            {
                *gport = p_brguc_info->dest_gport;
            }
            else if (p_brguc_info->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT)
            {
                *gport = p_brguc_info->dest_gport;
                *aps_en = 1;

            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        break;

    case SYS_NH_TYPE_IPUC:
        {
            sys_nh_info_ipuc_t* p_ipuc_info;
            p_ipuc_info = (sys_nh_info_ipuc_t*)(p_nhinfo);
            if (CTC_FLAG_ISSET(p_ipuc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
            {
                *gport = CTC_MAX_UINT16_VALUE;
            }
            else
            {
                *gport = p_ipuc_info->gport;
            }

            *aps_en = FALSE;
        }
        break;

    case SYS_NH_TYPE_MPLS:
        {
            sys_nh_info_mpls_t* p_mpls_info;
            p_mpls_info = (sys_nh_info_mpls_t*)(p_nhinfo);

            if (CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
            {
                *gport = CTC_MAX_UINT16_VALUE;
                *aps_en = 0;
            }
            else if (CTC_FLAG_ISSET(p_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
            {
                *gport = p_mpls_info->aps_group_id;
                *aps_en = 1;
            }
            else
            {
                *gport = p_mpls_info->gport;
                *aps_en = 0;
            }
        }
        break;

    case SYS_NH_TYPE_MCAST:
        {
            sys_nh_info_mcast_t* p_mcast_info;
            p_mcast_info = (sys_nh_info_mcast_t*)(p_nhinfo);

            if (CTC_FLAG_ISSET(p_mcast_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MCAST_PROFILE))
            {
                /*mcast prifile member gort have no mean*/
                sys_usw_get_gchip_id(lchip, &gchip);
                *gport = CTC_MAP_LPORT_TO_GPORT(gchip, 0);
                *aps_en = 0;
            }
            else
            {
                *gport = CTC_MAX_UINT16_VALUE;
                *aps_en = 0;
            }
        }
        break;

    case SYS_NH_TYPE_MISC:
        {
            sys_nh_info_misc_t* p_nh_info;
            p_nh_info = (sys_nh_info_misc_t*)(p_nhinfo);
            *gport = p_nh_info->gport;
        }
        break;

    case  SYS_NH_TYPE_DROP:
        {
            *gport = CTC_MAX_UINT16_VALUE;
            *aps_en = 0;
        }
        break;

    case  SYS_NH_TYPE_TOCPU:
        {
            sys_usw_get_gchip_id(lchip, &gchip);
            *gport = CTC_GPORT_RCPU(gchip);
            *aps_en = 0;
        }
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_ip_tunnel_info;
            p_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)(p_nhinfo);
            if (CTC_FLAG_ISSET(p_ip_tunnel_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
            {
                *gport = CTC_MAX_UINT16_VALUE;
            }
            else
            {
                *gport = p_ip_tunnel_info->gport;
            }

            *aps_en = FALSE;
        }
        break;
    case SYS_NH_TYPE_TRILL:
        {
            sys_nh_info_trill_t* p_trill_info;
            p_trill_info = (sys_nh_info_trill_t*)(p_nhinfo);
            if (CTC_FLAG_ISSET(p_trill_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
            {
                *gport = CTC_MAX_UINT16_VALUE;
            }
            else
            {
                *gport = p_trill_info->gport;
            }
            *aps_en = FALSE;
        }
        break;
    case SYS_NH_TYPE_ILOOP:
        {
            sys_nh_info_special_t* p_nh_info;
            p_nh_info = (sys_nh_info_special_t*)(p_nhinfo);
            *gport = p_nh_info->dest_gport;
        }
        break;

    case SYS_NH_TYPE_WLAN_TUNNEL:
        {
            sys_nh_info_wlan_tunnel_t* p_nh_info;
            p_nh_info = (sys_nh_info_wlan_tunnel_t*)(p_nhinfo);
            *gport = p_nh_info->gport;
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

bool
sys_usw_nh_is_cpu_nhid(uint8 lchip, uint32 nhid)
{
    uint8 is_cpu_nh = 0;
    int32 ret = 0;
    sys_nh_info_com_t* p_nhinfo = NULL;
    sys_nh_info_misc_t* p_nh_misc_info = NULL;

    if (nhid == SYS_NH_RESOLVED_NHID_FOR_TOCPU)
    {
        is_cpu_nh = 1;
    }

    ret = sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo);
    if (ret < 0)
    {
        return FALSE;
    }

    if ((p_nhinfo) && (p_nhinfo->hdr.nh_entry_type == SYS_NH_TYPE_MISC))
    {
        p_nh_misc_info = (sys_nh_info_misc_t*)p_nhinfo;
        if (p_nh_misc_info->misc_nh_type == CTC_MISC_NH_TYPE_TO_CPU)
        {
            is_cpu_nh = 1;
        }
    }
    return (is_cpu_nh ? TRUE : FALSE);
}

/**
 @brief This function is used to allocate a new internal nexthop id
 */
STATIC int32
_sys_usw_nh_new_nh_id(uint8 lchip, uint32* p_nhid)
{
    sys_usw_opf_t opf;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_nhid);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = OPF_NH_NHID_INTERNAL;
    opf.multiple  = 1;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, p_nhid));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "New allocated nhid = 0x%x\n", *p_nhid);

    return CTC_E_NONE;
}

int32
_sys_usw_nh_info_free(uint8 lchip, uint32 nhid)
{
    sys_usw_opf_t opf;
    sys_nh_info_com_t* p_nhinfo = NULL;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_nh_info_com_t nh_temp;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Need freed nhid = %d\n", nhid);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&nh_temp, 0, sizeof(nh_temp));

    if (nhid >= p_nh_master->max_external_nhid)
    {
        opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
        opf.pool_index = OPF_NH_NHID_INTERNAL;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, nhid));
        p_nh_master->internal_nh_used_num = p_nh_master->internal_nh_used_num?(p_nh_master->internal_nh_used_num-1):0;
    }

    nh_temp.hdr.nh_id = nhid;
    p_nhinfo = (sys_nh_info_com_t*)ctc_hash_remove(p_nh_master->nhid_hash, &nh_temp);
    if (p_nhinfo)
    {
        mem_free(p_nhinfo);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_nh_get_nhinfo(uint8 lchip, uint32 nhid, sys_nh_info_dsnh_t* p_nhinfo)
{

    sys_nh_info_com_t* p_nh_com_info;
    sys_l3if_ecmp_if_t ecmp_if = {0};
    uint8   gchip  = 0;
    uint16   arp_id = 0;
    uint32 dest_id = 0;
    uint8  dest_chipid = 0;
    sys_nh_db_arp_t *p_arp = NULL;
    int32 ret = 0;
    uint8 sub_queue_id = 0;

    if (!p_nhinfo)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info), ret, error);

    if ((p_nh_com_info->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_MCAST_PROFILE))
    {
        /*mcast profile do not used by fwd */
        ret = CTC_E_INVALID_PARAM;
        goto error;
    }

    sys_usw_get_gchip_id(lchip, &gchip);

    p_nhinfo->nh_entry_type = p_nh_com_info->hdr.nh_entry_type;
    p_nhinfo->aps_en        = CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);
    p_nhinfo->drop_pkt      = CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
    p_nhinfo->dsnh_offset   = p_nh_com_info->hdr.dsfwd_info.dsnh_offset;
    p_nhinfo->nexthop_ext   = CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
    p_nhinfo->dsfwd_valid   = CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (p_nh_com_info->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD);

    if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE) &&
        !p_nhinfo->dsfwd_valid)
    {
        p_nhinfo->dsnh_offset   = p_nh_com_info->hdr.dsfwd_info.dsnh_offset|SYS_NH_DSNH_BY_PASS_FLAG;
    }

    if(CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LEN_ADJUST_EN))
    {
        p_nhinfo->adjust_len = p_nh_com_info->hdr.adjust_len;
    }
    p_nhinfo->merge_dsfwd = 0;
    if(!p_nhinfo->dsfwd_valid)
    {
        /*Use DsFwd Mode*/
        if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_ECMP_IF))
        {
            p_nhinfo->merge_dsfwd = 1;
        }

    }
    else
    {
        p_nhinfo->dsfwd_offset = p_nh_com_info->hdr.dsfwd_info.dsfwd_offset;
    }

    switch (p_nh_com_info->hdr.nh_entry_type)
    {
    case SYS_NH_TYPE_MCAST:
        {
            sys_nh_info_mcast_t * p_nh_mcast_info = (sys_nh_info_mcast_t*)p_nh_com_info;
            p_nhinfo->is_mcast = 1;
            p_nhinfo->merge_dsfwd = 1;

            p_nhinfo->gport = ((p_nh_mcast_info->basic_met_offset)&0xffff);
            dest_id = ((p_nh_mcast_info->basic_met_offset)&0xffff);
        }
        break;
    case SYS_NH_TYPE_BRGUC:
        {
            sys_nh_info_brguc_t* p_nh_brguc_info = (sys_nh_info_brguc_t*)p_nh_com_info;

            if (!p_nhinfo->aps_en)
            {
                p_nhinfo->gport       = p_nh_brguc_info->dest_gport;
                dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_brguc_info->dest_gport);
                dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_brguc_info->dest_gport);
            }
            else
            {
                p_nhinfo->gport       = p_nh_brguc_info->dest_gport;
                dest_chipid           = gchip;
                dest_id               = p_nh_brguc_info->dest_gport;
                p_nhinfo->individual_nexthop =
                    CTC_FLAG_ISSET(p_nh_brguc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH)? 0 : 1;
            }

            if (!p_nhinfo->dsfwd_valid)
            {
                p_nhinfo->merge_dsfwd  = 1; /*L2Uc Nexthop don't need update,so it can use merge DsFwd mode*/
            }

            if (p_nh_brguc_info->nh_sub_type < SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT)
            {
                /*indicate l2uc*/
                p_nhinfo->re_route = 1;
            }
        }
        break;

    case SYS_NH_TYPE_MPLS:
        {
            sys_nh_info_mpls_t* p_nh_mpls_info = (sys_nh_info_mpls_t*)p_nh_com_info;

            if (!p_nhinfo->aps_en)
            {
                p_nhinfo->gport       = p_nh_mpls_info->gport;
                arp_id = p_nh_mpls_info->arp_id;
                dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_mpls_info->gport);
                dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_mpls_info->gport);

            }
            else
            {
                dest_chipid           = gchip;
                dest_id               = p_nh_mpls_info->aps_group_id;
                p_nhinfo->gport       = p_nh_mpls_info->aps_group_id;
                p_nhinfo->individual_nexthop = (p_nh_mpls_info->protection_path)? 1 : 0;
            }

            /* 1. For PW Nexthop  */
            /* PW have APS, LSP no APS, mep_on_tunnel = 1, APS group in MEP is PW APS group */
            /* PW have APS, LSP have APS, mep_on_tunnel = 0, APS group in MEP is LSP APS group */
            /* PW no APS, LSP have APS, mep_on_tunnel = 0, APS group in MEP is LSP APS group */
            /* 2. For LSP Nexthop*/
            /* LSP have APS, SPME have or no APS, mep_on_tunnel = 1, APS group in MEP is LSP APS group*/
            /* LSP no APS, SPME have APS, mep_on_tunnel = 0, APS group in MEP is SPME APS group*/
            p_nhinfo->oam_aps_en = p_nhinfo->aps_en;
            if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))/*PW nexthop*/
            {
                if (p_nh_mpls_info->protection_path)/* PW have APS*/
                {
                    p_nhinfo->mep_on_tunnel = 1;
                    p_nhinfo->oam_aps_group_id =  p_nh_mpls_info->aps_group_id;
                    /* Working LSP have APS*/
                    if ((!p_nhinfo->protection_path) && p_nh_mpls_info->working_path.p_mpls_tunnel &&
                        CTC_FLAG_ISSET(p_nh_mpls_info->working_path.p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
                    {
                        p_nhinfo->oam_aps_group_id  =  p_nh_mpls_info->working_path.p_mpls_tunnel->gport;
                        p_nhinfo->mep_on_tunnel = 0;
                    }
                    /* Protection LSP have APS*/
                    else if (p_nh_mpls_info->protection_path &&
                        p_nh_mpls_info->protection_path->p_mpls_tunnel &&
                    CTC_FLAG_ISSET(p_nh_mpls_info->protection_path->p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
                    {
                        p_nhinfo->oam_aps_group_id =  p_nh_mpls_info->protection_path->p_mpls_tunnel->gport;
                        p_nhinfo->mep_on_tunnel = 0;
                    }
                    if (p_nhinfo->protection_path)
                    {
                        p_nhinfo->dsnh_offset = p_nhinfo->dsnh_offset + 1*(1 + p_nhinfo->nexthop_ext);
                    }
                }
                else /* PW no APS, LSP may be have APS*/
                {
                    p_nhinfo->mep_on_tunnel = 0;
                    p_nhinfo->oam_aps_group_id = (p_nh_mpls_info->working_path.p_mpls_tunnel)?
                            p_nh_mpls_info->working_path.p_mpls_tunnel->gport : 0;    /* gport is LSP APS group actually*/
                }
            }
            else /*LSP nexthop*/
            {
                if ((NULL == p_nh_mpls_info->protection_path)/* LSP no APS and SPME have APS*/
                    && p_nh_mpls_info->working_path.p_mpls_tunnel
                    && CTC_FLAG_ISSET(p_nh_mpls_info->working_path.p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME)
                    && CTC_FLAG_ISSET(p_nh_mpls_info->working_path.p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS)
                    && !CTC_FLAG_ISSET(p_nh_mpls_info->working_path.p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
                {
                    p_nhinfo->mep_on_tunnel = 0;
                    p_nhinfo->oam_aps_group_id  =  p_nh_mpls_info->working_path.p_mpls_tunnel->gport;
                }
                else
                {
                    p_nhinfo->mep_on_tunnel = p_nhinfo->aps_en;
                    p_nhinfo->oam_aps_group_id =  p_nh_mpls_info->aps_group_id;
                }
            }

            p_nhinfo->bind_data = p_nh_mpls_info->data;
            p_nhinfo->updateAd = p_nh_mpls_info->updateAd;
            p_nhinfo->tunnel_id = p_nh_mpls_info->working_path.p_mpls_tunnel?p_nh_mpls_info->working_path.p_mpls_tunnel->tunnel_id:0;

        }
        break;
    case SYS_NH_TYPE_ECMP:
        {
            sys_nh_info_ecmp_t* p_nh_ecmp_info = (sys_nh_info_ecmp_t*)p_nh_com_info;
            p_nhinfo->ecmp_valid = TRUE;
            p_nhinfo->ecmp_group_id = p_nh_ecmp_info->ecmp_group_id;
            p_nhinfo->ecmp_num = p_nh_ecmp_info->ecmp_cnt;
            p_nhinfo->ecmp_cnt = p_nh_ecmp_info->ecmp_cnt;
            p_nhinfo->valid_cnt = p_nh_ecmp_info->valid_cnt;
            p_nhinfo->merge_dsfwd  = 0;
            sal_memcpy(&p_nhinfo->nh_array[0], &p_nh_ecmp_info->nh_array[0], p_nh_ecmp_info->ecmp_cnt * sizeof(uint32));
       }
       break;
    case SYS_NH_TYPE_IPUC:
        {
            sys_nh_info_ipuc_t* p_nh_ipuc_info = (sys_nh_info_ipuc_t*)p_nh_com_info;
            p_nhinfo->have_l2edit = CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
            p_nhinfo->bind_data = p_nh_ipuc_info->data;
            p_nhinfo->updateAd = p_nh_ipuc_info->updateAd;
            if (!p_nhinfo->aps_en)
            {
                p_nhinfo->gport       = p_nh_ipuc_info->gport;
                arp_id = p_nh_ipuc_info->arp_id;
                dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_ipuc_info->gport);
                dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_ipuc_info->gport);
            }
            else
            {
                p_nhinfo->gport       = p_nh_ipuc_info->gport;
                dest_chipid           = gchip;
                dest_id               = p_nh_ipuc_info->gport;
            }
            p_nhinfo->merge_dsfwd = CTC_FLAG_ISSET(p_nh_ipuc_info->flag,SYS_NH_IPUC_FLAG_MERGE_DSFWD)?1:0;
        }
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_nh_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)p_nh_com_info;

            p_nhinfo->gport       = p_nh_ip_tunnel_info->gport;
            arp_id = p_nh_ip_tunnel_info->arp_id;
            p_nhinfo->is_ivi = CTC_FLAG_ISSET(p_nh_ip_tunnel_info->flag, SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4) ||
                               CTC_FLAG_ISSET(p_nh_ip_tunnel_info->flag, SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6);
            p_nhinfo->re_route = CTC_FLAG_ISSET(p_nh_ip_tunnel_info->flag, SYS_NH_IP_TUNNEL_REROUTE);
            p_nhinfo->bind_data = p_nh_ip_tunnel_info->data;
            p_nhinfo->cloud_sec_en = p_nh_ip_tunnel_info->dot1ae_en?1:0;
            p_nhinfo->updateAd = p_nh_ip_tunnel_info->updateAd;

            if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_ECMP_IF))
            {
                p_nhinfo->ecmp_valid = FALSE;
                p_nhinfo->is_ecmp_intf = TRUE;
                p_nhinfo->merge_dsfwd = 1;
                sys_usw_l3if_get_ecmp_if_info(lchip, p_nh_ip_tunnel_info->ecmp_if_id, &ecmp_if);
                p_nhinfo->ecmp_group_id = ecmp_if.hw_group_id;
                dest_id = p_nhinfo->ecmp_group_id;
            }
            else
            {
                dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_ip_tunnel_info->gport);
                dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_ip_tunnel_info->gport);
            }

        }
        break;

    case SYS_NH_TYPE_TRILL:
        {
            sys_nh_info_trill_t* p_nh_trill_info = (sys_nh_info_trill_t*)p_nh_com_info;
            p_nhinfo->gport       = p_nh_trill_info->gport;
            p_nhinfo->bind_data = p_nh_trill_info->data;
            p_nhinfo->updateAd = p_nh_trill_info->updateAd;
            dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_trill_info->gport);
            dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_trill_info->gport);
        }
        break;

    case SYS_NH_TYPE_DROP:
        {
            p_nhinfo->dsfwd_valid = TRUE;
            p_nhinfo->drop_pkt = TRUE;
            dest_chipid           = gchip;
            dest_id               = SYS_RSV_PORT_DROP_ID;
        }
    break;

    case SYS_NH_TYPE_TOCPU:
        {
            p_nhinfo->gport = CTC_GPORT_RCPU(gchip);
            p_nhinfo->dsfwd_valid = TRUE;

            /*Notice: the function may be called in Qos module, to avoid dead lock, using internal function instead*/
            CTC_ERROR_GOTO(_sys_usw_cpu_reason_get_info(lchip, CTC_PKT_CPU_REASON_FWD_CPU, &p_nhinfo->dest_map), ret, error);
        }
        break;
    case SYS_NH_TYPE_MISC:
        {
            sys_nh_info_misc_t* p_nh_misc_info = (sys_nh_info_misc_t*)p_nh_com_info;
            p_nhinfo->gport       = p_nh_misc_info->gport;
            dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_misc_info->gport);
            dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_misc_info->gport);
            p_nhinfo->bind_data = p_nh_misc_info->data;
            p_nhinfo->updateAd = p_nh_misc_info->updateAd;
        }
        break;

    case SYS_NH_TYPE_ILOOP:
        {
            sys_nh_info_special_t* p_nh_iloop_info = (sys_nh_info_special_t *)p_nh_com_info;
            p_nhinfo->dsfwd_valid   = CTC_FLAG_ISSET(p_nh_iloop_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
            p_nhinfo->nexthop_ext   = CTC_FLAG_ISSET(p_nh_iloop_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
            p_nhinfo->aps_en        = CTC_FLAG_ISSET(p_nh_iloop_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);
            dest_chipid             = gchip;
            dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_iloop_info->dest_gport);
        }
        break;

    case SYS_NH_TYPE_RSPAN:
        break;
    case SYS_NH_TYPE_WLAN_TUNNEL:
        {
            sys_nh_info_wlan_tunnel_t* p_nh_wlan_tunnel_info = (sys_nh_info_wlan_tunnel_t*)p_nh_com_info;

            p_nhinfo->gport       = p_nh_wlan_tunnel_info->gport;
            dest_chipid           = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_wlan_tunnel_info->gport);
            dest_id               = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_wlan_tunnel_info->gport);
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    if (arp_id)
    {
        p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
        if (NULL == p_arp)
        {
            return CTC_E_NOT_EXIST;
        }
        p_nhinfo->arp_id = arp_id;

        p_nhinfo->drop_pkt = CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP);

        if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID) &&
            !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP) &&
            !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))
        {
            p_nhinfo->gport = p_arp->gport;
        }

        if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))
        {
            p_nhinfo->to_cpu_en    = CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU);
            if (!p_arp->destmap_profile)
            {
                p_nhinfo->dsnh_offset  =   CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_ARP_MISS, 0);
            }
        }

        /*If Using destmap profile, mapping profile id to destid for encoding destmap usage*/
        if (p_arp->destmap_profile)
        {
            p_nhinfo->use_destmap_profile = 1;
        }
    }

    /*fwd to cpu nexthop without arp, using CTC_PKT_CPU_REASON_FWD_CPU cpu reason*/
    if(CTC_IS_CPU_PORT(p_nhinfo->gport) && !arp_id)
    {
        p_nhinfo->dsnh_offset = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_FWD_CPU, 0);
    }

    /*encoding destmap*/
    if (arp_id && p_arp->destmap_profile)
    {
        p_nhinfo->dest_map = SYS_ENCODE_ARP_DESTMAP(p_arp->destmap_profile);
    }
    else if(CTC_IS_CPU_PORT(p_nhinfo->gport))
    {
        CTC_ERROR_RETURN(_sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_FWD_CPU, &sub_queue_id));
        p_nhinfo->dest_map = SYS_ENCODE_EXCP_DESTMAP(dest_chipid, sub_queue_id);
    }
    else if (p_nhinfo->drop_pkt)
    {
        p_nhinfo->dest_map = SYS_ENCODE_DESTMAP(dest_chipid, SYS_RSV_PORT_DROP_ID);
        p_nhinfo->dsnh_offset = 0x3FFFF;
    }
    else if(p_nhinfo->is_ecmp_intf)
    {
        p_nhinfo->dest_map = SYS_ENCODE_ECMP_DESTMAP(p_nhinfo->ecmp_group_id);
    }
    else if (p_nhinfo->to_cpu_en)
    {
        uint8 sub_queue_id = 0;
        uint32 temp_dest = 0;
        /*ARP miss to cpu*/
        CTC_ERROR_RETURN(_sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_ARP_MISS, &sub_queue_id));
        CTC_ERROR_RETURN(_sys_usw_cpu_reason_get_info(lchip, CTC_PKT_CPU_REASON_ARP_MISS, &temp_dest));
        p_nhinfo->dest_map = SYS_ENCODE_EXCP_DESTMAP(SYS_DECODE_DESTMAP_GCHIP(temp_dest), sub_queue_id);
    }
    else if( p_nhinfo->is_mcast)
    {
        p_nhinfo->dest_map =SYS_ENCODE_MCAST_IPE_DESTMAP(dest_id);
    }
    else if(p_nhinfo->aps_en)
    {
        p_nhinfo->dest_map = SYS_ENCODE_APS_DESTMAP(dest_chipid, dest_id);
    }
    else
    {
        p_nhinfo->dest_map = SYS_ENCODE_DESTMAP(dest_chipid, dest_id);
    }

    return CTC_E_NONE;
error:
    return ret;
}

int32
_sys_usw_nh_api_get_nhinfo_by_nhid(uint8 lchip, uint32 nhid, sys_usw_nh_master_t* p_nh_master,
                                         sys_nh_info_com_t** pp_nhinfo)
{
    sys_nh_info_com_t* p_nh_info;
    sys_nh_info_com_t nh_temp;

    sal_memset(&nh_temp, 0, sizeof(nh_temp));
    nh_temp.hdr.nh_id = nhid;
    p_nh_info = ctc_hash_lookup(p_nh_master->nhid_hash, &nh_temp);

    if (NULL == p_nh_info)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop %d not exist \n", nhid);
	    return CTC_E_NOT_EXIST;
    }

    *pp_nhinfo = p_nh_info;

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_nhinfo_by_nhid(uint8 lchip, uint32 nhid, sys_nh_info_com_t** pp_nhinfo)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_nh_info_com_t* p_nhinfo = NULL;
    uint16 arp_id = 0;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(pp_nhinfo);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    ret = (_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nhinfo));
    if (ret)
    {
        return ret;
    }

    switch(p_nhinfo->hdr.nh_entry_type)
    {
        case SYS_NH_TYPE_MPLS:
        {
            sys_nh_info_mpls_t* p_nh_mpls_info = (sys_nh_info_mpls_t*)p_nhinfo;
            arp_id = p_nh_mpls_info->arp_id;

            if (arp_id)
            {
                sys_nh_db_arp_t *p_arp = NULL;
                p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
                if (!p_arp)
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Arp %d not exist \n", arp_id);
                    return CTC_E_NOT_EXIST;
                }

                p_nh_mpls_info->gport = p_arp->gport;

                if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID) &&
                    !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP) &&
                    !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))
                {
                    /*update l3if id*/
                   p_nh_mpls_info->l3ifid = p_arp->l3if_id;
                }
            }
            break;
        }
        case SYS_NH_TYPE_IPUC:
        {
            sys_nh_info_ipuc_t* p_nh_ipuc_info = (sys_nh_info_ipuc_t*)p_nhinfo;
            uint8 loop = 2;
            uint8 aps_en = 0;

            aps_en = CTC_FLAG_ISSET(p_nh_ipuc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);

            for (loop = 0; loop < 2; loop++)
            {
                if ((loop == 1) && !p_nh_ipuc_info->protection_path && !aps_en)
                {
                    break;
                }

                arp_id = loop?p_nh_ipuc_info->protection_path->arp_id:p_nh_ipuc_info->arp_id;
                if (arp_id)
                {
                    sys_nh_db_arp_t *p_arp = NULL;
                    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
                    if (!p_arp)
                    {
                        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Arp %d not exist \n", arp_id);
                        return CTC_E_NOT_EXIST;
                    }

                    /*p_nh_ipuc_info->gport means aps group id, cannot update to gport*/
                    if (!aps_en)
                    {
                        p_nh_ipuc_info->gport = p_arp->gport;
                    }

                    if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID) &&
                        !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP) &&
                        !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))
                    {
                       /*update l3if id*/
                        if (!loop)
                        {
                           p_nh_ipuc_info->l3ifid = p_arp->l3if_id;
                        }
                        else
                        {
                            p_nh_ipuc_info->protection_path->l3ifid = p_arp->l3if_id;
                        }
                    }
                }
            }

            break;
        }
        case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_nh_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)p_nhinfo;
            arp_id = p_nh_ip_tunnel_info->arp_id;
            if (arp_id)
            {
                sys_nh_db_arp_t *p_arp = NULL;
                p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
                if (!p_arp)
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Arp %d not exist \n", arp_id);
                    return CTC_E_NOT_EXIST;
                }

                p_nh_ip_tunnel_info->gport = p_arp->gport;

                if (CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_PORT_VALID) &&
                    !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_DROP) &&
                    !CTC_FLAG_ISSET(p_arp->flag, SYS_NH_ARP_FLAG_REDIRECT_TO_CPU))
                {
                    /*update l3if id*/
                    p_nh_ip_tunnel_info->l3ifid = p_arp->l3if_id;
                }
            }
            break;
        }
        case SYS_NH_TYPE_TRILL:
        {
            /*TBD*/
            break;
        }

        case SYS_NH_TYPE_WLAN_TUNNEL:
        {
            break;
        }
        default:
            break;
    }

    *pp_nhinfo = p_nhinfo;

    return CTC_E_NONE;
}

/**
 @brief Get nh db entry size
 */
STATIC int32
_sys_usw_nh_get_nhentry_size_by_nhtype(uint8 lchip,
    sys_usw_nh_type_t nh_type, uint32* p_size)
{

    switch (nh_type)
    {
    case SYS_NH_TYPE_BRGUC:
        *p_size = sizeof(sys_nh_info_brguc_t);
        break;

    case SYS_NH_TYPE_MCAST:
        *p_size = sizeof(sys_nh_info_mcast_t);
        break;

    case SYS_NH_TYPE_IPUC:
        *p_size = sizeof(sys_nh_info_ipuc_t);
        break;

    case SYS_NH_TYPE_ECMP:
        *p_size = sizeof(sys_nh_info_ecmp_t);
        break;

    case SYS_NH_TYPE_MPLS:
        *p_size = sizeof(sys_nh_info_mpls_t);
        break;

    case SYS_NH_TYPE_DROP:
    case SYS_NH_TYPE_TOCPU:
    case SYS_NH_TYPE_UNROV:
    case SYS_NH_TYPE_ILOOP:
        *p_size = sizeof(sys_nh_info_special_t);
        break;

    case SYS_NH_TYPE_RSPAN:
        *p_size = sizeof(sys_nh_info_rspan_t);
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        *p_size = sizeof(sys_nh_info_ip_tunnel_t);
        break;

    case SYS_NH_TYPE_TRILL:
        *p_size = sizeof(sys_nh_info_trill_t);
        break;

    case SYS_NH_TYPE_WLAN_TUNNEL:
        *p_size = sizeof(sys_nh_info_wlan_tunnel_t);
        break;

    case SYS_NH_TYPE_MISC:
        *p_size = sizeof(sys_nh_info_misc_t);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_api_create(uint8 lchip, sys_nh_param_com_t* p_nh_com_para)
{

    sys_usw_nh_type_t nh_type;
    p_sys_nh_create_cb_t nh_sem_map_cb;
    sys_nh_info_com_t* p_nhinfo = NULL;
    int32 ret = CTC_E_NONE;
    uint32 tmp_nh_id = 0;
    uint32 db_entry_size = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*1. Sanity check and init*/
    if (NULL == p_nh_com_para)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    if (!p_nh_com_para->hdr.is_internal_nh
        && (p_nh_com_para->hdr.nh_param_type != SYS_NH_TYPE_DROP)
        && (p_nh_com_para->hdr.nh_param_type != SYS_NH_TYPE_TOCPU))
    {
        if (p_nh_com_para->hdr.nhid < SYS_NH_RESOLVED_NHID_MAX ||
            p_nh_com_para->hdr.nhid >= (p_nh_master->max_external_nhid))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    /*Check If this nexthop is exist*/
    if (!(p_nh_com_para->hdr.is_internal_nh))
    {
        sys_nh_info_com_t nh_temp;

        sal_memset(&nh_temp, 0, sizeof(nh_temp));
        nh_temp.hdr.nh_id = p_nh_com_para->hdr.nhid;
        if (ctc_hash_lookup(p_nh_master->nhid_hash, &nh_temp))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop  exist \n");
    		return CTC_E_EXIST;
        }
    }

    nh_type = p_nh_com_para->hdr.nh_param_type;
    ret = _sys_usw_nh_get_nhentry_size_by_nhtype(lchip, nh_type, &db_entry_size);
    if (ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
		return CTC_E_INVALID_CONFIG;
    }

    p_nhinfo = mem_malloc(MEM_NEXTHOP_MODULE, db_entry_size);
    if (NULL == p_nhinfo)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    else
    {
        sal_memset(p_nhinfo, 0, db_entry_size);
    }

    ret = _sys_usw_nh_alloc_offset_by_nhtype(lchip, p_nh_com_para, p_nhinfo);
    if (ret)
    {

        mem_free(p_nhinfo);
        return ret;
    }

    p_nhinfo->hdr.nh_id = p_nh_com_para->hdr.nhid;

    /*2. Semantic mapping Callback*/
    nh_sem_map_cb = p_nh_master->callbacks_nh_create[nh_type];
    ret = (* nh_sem_map_cb)(lchip, p_nh_com_para, p_nhinfo);
    if (ret)
    {
        _sys_usw_nh_free_offset_by_nhinfo(lchip, nh_type, p_nhinfo);

        mem_free(p_nhinfo);
        return ret;
    }

    /*3. Store nh infor into nh_array*/
    if (p_nh_com_para->hdr.is_internal_nh)
    {
        /*alloc a new nhid*/
        ret = _sys_usw_nh_new_nh_id(lchip, &tmp_nh_id);
        if (ret)
        {
            _sys_usw_nh_free_offset_by_nhinfo(lchip, nh_type, p_nhinfo);
            mem_free(p_nhinfo);
            return ret;
        }
        p_nh_com_para->hdr.nhid = tmp_nh_id; /*Write back the nhid*/
        p_nhinfo->hdr.nh_id = tmp_nh_id;
    }

    if (FALSE == ctc_hash_insert(p_nh_master->nhid_hash, p_nhinfo))
    {
        _sys_usw_nh_free_offset_by_nhinfo(lchip, nh_type, p_nhinfo);
        _sys_usw_nh_info_free(lchip, p_nh_com_para->hdr.nhid);
        mem_free(p_nhinfo);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;

    }

    if (ret == CTC_E_NONE && nh_type == SYS_NH_TYPE_ECMP)
    {
        p_nh_master->cur_ecmp_cnt++;
    }
    p_nh_master->nhid_used_cnt[nh_type]++;

    if (p_nh_com_para->hdr.is_internal_nh)
    {
        p_nh_master->internal_nh_used_num++;
    }

    return ret;
}

int32
sys_usw_nh_api_delete(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nhid_type)
{
    p_sys_nh_delete_cb_t nh_del_cb;
    sys_usw_nh_type_t nh_type;
    sys_nh_info_com_t* p_nh_info = NULL;
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    CTC_ERROR_RETURN(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nh_info));

    if ((nhid_type != p_nh_info->hdr.nh_entry_type))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type doesnt match \n");
		return CTC_E_INVALID_CONFIG;
    }

    nh_type = p_nh_info->hdr.nh_entry_type;

    nh_del_cb = p_nh_master->callbacks_nh_delete[nh_type];

    ret = (* nh_del_cb)(lchip, p_nh_info, & nhid);

    ret = ret ? ret : _sys_usw_nh_free_offset_by_nhinfo(lchip, nh_type, p_nh_info);

    ret = ret ? ret : _sys_usw_nh_info_free(lchip, nhid);

    if (ret == CTC_E_NONE && nh_type == SYS_NH_TYPE_ECMP)
    {
        p_nh_master->cur_ecmp_cnt = (p_nh_master->cur_ecmp_cnt > 0) ? (p_nh_master->cur_ecmp_cnt - 1) : p_nh_master->cur_ecmp_cnt;
    }
    if(p_nh_master->nhid_used_cnt[nh_type])
    {
      p_nh_master->nhid_used_cnt[nh_type]--;
    }
    return ret;
}


int32
sys_usw_nh_api_update(uint8 lchip, uint32 nhid, sys_nh_param_com_t* p_nh_com_para)
{
    p_sys_nh_update_cb_t nh_update_cb;
    sys_usw_nh_type_t nh_type;
    sys_nh_info_com_t* p_nh_info;
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    nh_type = p_nh_com_para->hdr.nh_param_type;

    CTC_ERROR_RETURN(
        _sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nh_info));

    if ((nh_type != p_nh_info->hdr.nh_entry_type))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Nexthop Type Not Match, nh_type:%d \n", nh_type);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type doesnt match \n");
		return CTC_E_INVALID_CONFIG;
    }
    if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        if(p_nh_com_para->hdr.change_type == SYS_NH_CHANGE_TYPE_FWD_TO_UNROV)
        {
            return CTC_E_NONE;
        }

        if(p_nh_com_para->hdr.change_type == SYS_NH_CHANGE_TYPE_UNROV_TO_FWD)
        {
            CTC_ERROR_RETURN(_sys_usw_nh_alloc_offset_by_nhtype(lchip, p_nh_com_para, p_nh_info));
        }
    }
    else
    {
        if(p_nh_com_para->hdr.change_type == SYS_NH_CHANGE_TYPE_UNROV_TO_FWD)
        {
           p_nh_com_para->hdr.change_type = SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR;
        }
    }

    nh_update_cb = p_nh_master->callbacks_nh_update[nh_type];
    if(nh_update_cb)
    {
       ret = (* nh_update_cb)(lchip, p_nh_info, p_nh_com_para);
    }

    if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        ret = ret?ret:sys_usw_nh_merge_dsfwd_disable(lchip, nhid);
    }

    return ret;
}

int32
sys_usw_nh_update_ad(uint8 lchip, uint32 nhid, uint8 update_type, uint8 is_arp)
{
    sys_nh_info_com_t* p_nh_info;
    int32 ret = CTC_E_NONE;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_nh_db_arp_t* p_arp = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (is_arp)
    {
        p_arp = (sys_usw_nh_lkup_arp_id(lchip, nhid));
        if (NULL == p_arp)
        {
            return CTC_E_NOT_EXIST;
        }

        if (p_arp->updateNhp)
        {
            CTC_ERROR_RETURN(p_arp->updateNhp(lchip, p_arp));
        }

        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nh_info));
	if (update_type != SYS_NH_CHANGE_TYPE_ADD_DYNTBL)
    {
        sys_nh_info_dsnh_t        dsnh_info;
        uint8 gchip_id = 0;
        sal_memset(&dsnh_info, 0, sizeof(sys_nh_info_dsnh_t));
        _sys_usw_nh_get_nhinfo((sys_usw_chip_get_rchain_en()?0:lchip), nhid, &dsnh_info);
        sys_usw_get_gchip_id(lchip, &gchip_id);
        if (update_type == SYS_NH_CHANGE_TYPE_NH_DELETE)
        {
            dsnh_info.dest_map = SYS_ENCODE_DESTMAP(gchip_id, SYS_RSV_PORT_DROP_ID);
            dsnh_info.gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, SYS_RSV_PORT_DROP_ID);
        }
        if (dsnh_info.updateAd)
        {
            ret = (dsnh_info.updateAd(lchip, dsnh_info.bind_data, &dsnh_info));
        }
    }

    return ret;
}

#define _________offset__________

int32
_sys_usw_nh_get_dsnh_offset_by_nhid(uint8 lchip, uint32 nhid, uint32 *p_dsnh_offset, uint8* p_use_dsnh8w)
{
    sys_nh_info_com_t* p_nhinfo;
    sys_usw_nh_master_t* p_nh_master = NULL;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    CTC_ERROR_RETURN(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nhinfo));


    *p_dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;  /* modified */


    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        *p_use_dsnh8w = TRUE;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_get_resolved_offset(uint8 lchip, sys_usw_nh_res_offset_type_t type, uint32* p_offset)
{
    sys_nh_offset_attr_t* p_res_offset;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_offset);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    /*Always get chip 0's offset, all chip should be same,
    maybe we don't need to store this value per-chip*/
    p_res_offset = &(p_nh_master->sys_nh_resolved_offset[type]);
    *p_offset = p_res_offset->offset_base;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Type = %d, resolved offset = 0x%x\n", type, *p_offset);

    return CTC_E_NONE;
}

/**
 @brief This function is used to alloc dynamic offset

 @return CTC_E_XXX
 */
int32
sys_usw_nh_offset_alloc(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                              uint32 entry_num, uint32* p_offset)
{
    sys_usw_opf_t opf;
    uint32 entry_size;
	sys_usw_nh_master_t* p_nh_master = NULL;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    *p_offset = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_offset);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_nh_master->nh_table_info_array[entry_type].opf_pool_type + OPF_NH_DYN1_SRAM;
    entry_size = p_nh_master->nh_table_info_array[entry_type].entry_size;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.multiple = entry_size;
    entry_size = entry_size * entry_num;
    opf.reverse = (p_nh_master->nh_table_info_array[entry_type].alloc_dir == 1) ? 1 : 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, entry_size, p_offset));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lchip = %d, entry_type = %d, entry_number = %d, offset = 0x%x\n",
                   lchip, entry_type, entry_num, *p_offset);
    p_nh_master->nhtbl_used_cnt[entry_type] += entry_size;

    return CTC_E_NONE;
}

int32
sys_usw_nh_offset_alloc_with_multiple(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                                            uint32 entry_num, uint16 multi, uint32* p_offset)
{
    sys_usw_opf_t opf;
    uint32 entry_size;
	sys_usw_nh_master_t* p_nh_master = NULL;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_offset);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    entry_size = p_nh_master->nh_table_info_array[entry_type].entry_size;
    opf.pool_index = p_nh_master->nh_table_info_array[entry_type].opf_pool_type + OPF_NH_DYN1_SRAM;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.multiple = multi;
    opf.reverse = (p_nh_master->nh_table_info_array[entry_type].alloc_dir == 1) ? 1 : 0;
    entry_size = entry_size * entry_num;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, entry_size, p_offset));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lchip = %d, entry_type = %d, entry_number = %d, multi = %d, offset = 0x%x\n",
                   lchip, entry_type, entry_num, multi, *p_offset);
    p_nh_master->nhtbl_used_cnt[entry_type] += entry_size;
    return CTC_E_NONE;
}


int32
sys_usw_nh_offset_alloc_from_position(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                                            uint32 entry_num, uint32 start_offset)
{
    sys_usw_opf_t opf;
    uint32 entry_size;
	sys_usw_nh_master_t* p_nh_master = NULL;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_nh_master->nh_table_info_array[entry_type].opf_pool_type + OPF_NH_DYN1_SRAM;
    entry_size = p_nh_master->nh_table_info_array[entry_type].entry_size;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.multiple = entry_size;
    entry_size = entry_size * entry_num;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf,  entry_size, start_offset));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lchip = %d, entry_type = %d, entry_number = %d, offset = 0x%x\n",
                   lchip, entry_type, entry_num, start_offset);

    p_nh_master->nhtbl_used_cnt[entry_type] += entry_size;
    return CTC_E_NONE;
}


/**
 @brief This function is used to free dynamic offset

 */
int32
sys_usw_nh_offset_free(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                             uint32 entry_num, uint32 offset)
{

    sys_usw_opf_t opf;
    uint32 entry_size;
	sys_usw_nh_master_t* p_nh_master = NULL;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (offset == 0)
    {/*offset 0 is reserved*/
        return CTC_E_NONE;
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lchip = %d, entry_type = %d, entry_number = %d, offset = %d\n",
                   lchip, entry_type, entry_num, offset);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = p_nh_master->nh_table_info_array[entry_type].opf_pool_type + OPF_NH_DYN1_SRAM;
    entry_size = p_nh_master->nh_table_info_array[entry_type].entry_size;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    entry_size = entry_size * entry_num;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, entry_size, offset));

    if(p_nh_master->nhtbl_used_cnt[entry_type] >= entry_size)
    {
       p_nh_master->nhtbl_used_cnt[entry_type] -= entry_size;
    }
    return CTC_E_NONE;

}

int32
sys_usw_nh_swap_nexthop_offset(uint8 lchip, uint32 nhid, uint32 nhid2)
{
    uint32 dsnh_offset = 0;
    uint32 dsnh_offset2 = 0;
    DsNextHop4W_m dsnh;
    DsNextHop4W_m dsnh2;
    uint32 cmd = 0;
    sys_nh_info_com_t*p_nhinfo = NULL;
    sys_nh_info_com_t*p_nhinfo2 = NULL;

    if (nhid == nhid2)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo));
    if (NULL == p_nhinfo)
    {
        return CTC_E_NH_INVALID_NHID;
    }

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid2, &p_nhinfo2));
    if (NULL == p_nhinfo2)
    {
        return CTC_E_NH_INVALID_NHID;
    }

    dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
    dsnh_offset2 = p_nhinfo2->hdr.dsfwd_info.dsnh_offset;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Swap nhid%d, offset:%d, nhid2:%d, offset2:%d \n", nhid, dsnh_offset, nhid2, dsnh_offset2);

    cmd = DRV_IOR(DsNextHop4W_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset, cmd, &dsnh));
    cmd = DRV_IOR(DsNextHop4W_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset2, cmd, &dsnh2));

    cmd = DRV_IOW(DsNextHop4W_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset, cmd, &dsnh2));
    cmd = DRV_IOW(DsFwd_t, DsFwd_nextHopPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_nhinfo->hdr.dsfwd_info.dsfwd_offset, cmd, &dsnh_offset2));
    p_nhinfo->hdr.dsfwd_info.dsnh_offset = dsnh_offset2;

    cmd = DRV_IOW(DsNextHop4W_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsnh_offset2, cmd, &dsnh));
    cmd = DRV_IOW(DsFwd_t, DsFwd_nextHopPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_nhinfo2->hdr.dsfwd_info.dsfwd_offset, cmd, &dsnh_offset));
    p_nhinfo2->hdr.dsfwd_info.dsnh_offset = dsnh_offset;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_alloc_logic_replicate_offset(uint8 lchip, sys_nh_info_com_t* p_nhinfo, uint32 dest_gport, uint32 group_id)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    sys_nh_repli_offset_node_t *p_offset_node = NULL;
    bool found = FALSE;
    int32 ret = 0;
    uint8 type = 0;
    uint8 multi = 0;
    uint32 offset = 0;
    sys_usw_nh_master_t* p_usw_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_usw_nh_master));

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        type = SYS_NH_ENTRY_TYPE_NEXTHOP_8W;
        multi = 2;
    }
    else
    {
        type = SYS_NH_ENTRY_TYPE_NEXTHOP_4W;
        multi = 1;
    }


    CTC_SLIST_LOOP(p_usw_nh_master->repli_offset_list, ctc_slistnode)
    {
        p_offset_node = _ctc_container_of(ctc_slistnode, sys_nh_repli_offset_node_t, head);
        if (p_offset_node->bitmap != 0xFFFFFFFF &&
            p_offset_node->dest_gport == dest_gport &&
            p_offset_node->group_id == group_id)
        {
            found = TRUE;
            break;
        }
    }

    if (TRUE == found)
    {
        uint8 bit = 0;

        for (bit = 0; bit < 32; bit++)
        {
            if (!CTC_IS_BIT_SET(p_offset_node->bitmap, bit))
            {
                CTC_BIT_SET(p_offset_node->bitmap , bit);
                offset = bit*multi;
                p_nhinfo->hdr.dsfwd_info.dsnh_offset = p_offset_node->start_offset + offset;
                p_nhinfo->hdr.p_data = (void*)p_offset_node;

                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update alloc_logic_replicate_offset :%d, gport:0x%x, group-id:%d\n",
                               p_nhinfo->hdr.dsfwd_info.dsnh_offset, dest_gport, group_id);

                return CTC_E_NONE;
            }
        }
    }

    p_offset_node =  mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_repli_offset_node_t));
    if (NULL == p_offset_node)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_offset_node, 0, sizeof(sys_nh_repli_offset_node_t));

    offset = SYS_NH_MCAST_LOGIC_MAX_CNT*multi;
    ret = sys_usw_nh_offset_alloc_with_multiple(lchip, type,
                                                      SYS_NH_MCAST_LOGIC_MAX_CNT,
                                                      offset,
                                                      &p_offset_node->start_offset);
    if (ret < 0)
    {
        mem_free(p_offset_node);
        return ret;
    }

    CTC_BIT_SET(p_offset_node->bitmap , 0);
    p_offset_node->dest_gport = dest_gport;
    p_offset_node->group_id = group_id;

    p_nhinfo->hdr.dsfwd_info.dsnh_offset = p_offset_node->start_offset;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "New alloc_logic_replicate_offset :%d, gport:0x%x, group-id:%d\n",
                   p_nhinfo->hdr.dsfwd_info.dsnh_offset, dest_gport, group_id);

	ctc_slist_add_tail(p_usw_nh_master->repli_offset_list, &(p_offset_node->head));

    p_nhinfo->hdr.p_data = (void*)p_offset_node;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_nh_free_logic_replicate_offset(uint8 lchip, sys_nh_info_com_t* p_nhinfo)
{
    sys_nh_repli_offset_node_t *p_offset_node = NULL;
    uint8 type = 0;
    uint8 multi = 0;
    uint8 bit = 0;
    sys_usw_nh_master_t* p_usw_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_usw_nh_master));

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        type = SYS_NH_ENTRY_TYPE_NEXTHOP_8W;
        multi = 2;
    }
    else
    {
        type = SYS_NH_ENTRY_TYPE_NEXTHOP_4W;
        multi = 1;
    }


    p_offset_node = (sys_nh_repli_offset_node_t *)p_nhinfo->hdr.p_data;

    if (NULL == p_offset_node)
    {
        return CTC_E_NOT_EXIST;
    }


    bit =  (p_nhinfo->hdr.dsfwd_info.dsnh_offset - p_offset_node->start_offset) / multi;

    CTC_BIT_UNSET(p_offset_node->bitmap , bit);


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free_logic_replicate_offset :%d, gport:0x%x\n", p_nhinfo->hdr.dsfwd_info.dsnh_offset, p_offset_node->dest_gport);

    if (p_offset_node->bitmap == 0)
    {

        ctc_slist_delete_node(p_usw_nh_master->repli_offset_list, &(p_offset_node->head));

        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, type,
                                                      SYS_NH_MCAST_LOGIC_MAX_CNT,
                                                      p_offset_node->start_offset));
        mem_free(p_offset_node);
        p_nhinfo->hdr.p_data = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free_offset node!!!!!\n");
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_alloc_offset_by_nhtype(uint8 lchip, sys_nh_param_com_t* p_nh_com_para, sys_nh_info_com_t* p_nhinfo)
{
    sys_usw_nh_master_t* p_master;

    uint32 dsnh_offset = 0;
    uint8 use_rsv_dsnh = 0;
     /*uint8 remote_chip = 0;*/
    uint8 dsnh_entry_num = 0;
    uint32 port_ext_mc_en = 0;
    uint8 exp_type = 0;
    uint8 tunnel_is_spme = 0;
    uint8 pkt_nh_edit_mode  = SYS_NH_IGS_CHIP_EDIT_MODE;
    uint8 dest_gchip = 0;
    uint8 use_dsnh8w = 0;
    uint32 dest_gport = 0;
    uint8 xgpon_en = 0;
    uint32 cmd = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " entry_type = %d\n", p_nh_com_para->hdr.nh_param_type);

    sys_usw_get_gchip_id(lchip, &dest_gchip);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    switch (p_nh_com_para->hdr.nh_param_type)
    {
    case SYS_NH_TYPE_BRGUC:
        {
            sys_nh_param_brguc_t* p_nh_param = (sys_nh_param_brguc_t*)p_nh_com_para;
            sys_nh_info_brguc_t* p_brg_nhinfo = (sys_nh_info_brguc_t*)p_nhinfo;
            dsnh_offset = p_nh_param->dsnh_offset;
            p_brg_nhinfo->nh_sub_type = p_nh_param->nh_sub_type;
            if (p_nh_param->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT)
            {
                if (CTC_FLAG_ISSET(p_nh_param->p_vlan_edit_info->flag, CTC_VLAN_NH_MTU_CHECK_EN))
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mtu check is not supported \n");
                    return CTC_E_NOT_SUPPORT;
                }

                if ((0 == p_nh_param->p_vlan_edit_info->flag)
                    &&(0 == p_nh_param->p_vlan_edit_info->edit_flag)
                    &&(0 == p_nh_param->p_vlan_edit_info->loop_nhid)
                    && (p_nh_param->p_vlan_edit_info->svlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE
                        && p_nh_param->p_vlan_edit_info->cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE)
                    && (!p_nh_param->p_vlan_edit_nh_param->logic_port)
                    && (!p_nh_param->p_vlan_edit_nh_param->logic_port_check))
                {
                    CTC_ERROR_RETURN(_sys_usw_nh_get_resolved_offset(lchip,
                                         SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, &dsnh_offset));
                    p_nhinfo->hdr.dsnh_entry_num  = 1;
                    use_rsv_dsnh = 1;
                }
                else
                {
                    p_nhinfo->hdr.dsnh_entry_num  = 1;
                    use_dsnh8w = p_nh_param->p_vlan_edit_nh_param
                        && (p_nh_param->p_vlan_edit_nh_param->logic_port ||p_nh_param->p_vlan_edit_nh_param->logic_port_check)
                        && (p_nh_param->p_vlan_edit_info->cvlan_edit_type != CTC_VLAN_EGRESS_EDIT_NONE)
                        && (p_nh_param->p_vlan_edit_info->cvlan_edit_type != CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE);

                    if (use_dsnh8w ||
                        p_nh_param->p_vlan_edit_info->cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_STRIP_VLAN
                        || (p_nh_param->p_vlan_edit_info->loop_nhid
                        && p_nh_param->p_vlan_edit_info->svlan_edit_type != CTC_VLAN_EGRESS_EDIT_NONE
                        && p_nh_param->p_vlan_edit_info->cvlan_edit_type != CTC_VLAN_EGRESS_EDIT_NONE)
                        || (p_nh_param->p_vlan_edit_nh_param && p_nh_param->p_vlan_edit_nh_param->cid && !g_usw_nh_master[lchip]->cid_use_4w)
                        || (CTC_FLAG_ISSET(p_nh_param->p_vlan_edit_info->flag, CTC_VLAN_NH_STATS_EN) && g_usw_nh_master[lchip]->cid_use_4w))
                    {
                        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                    }

                    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
                    if (xgpon_en && DRV_IS_DUET2(lchip))
                    {
                        if (CTC_FLAG_ISSET(p_nh_param->p_vlan_edit_info->flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG))
                        {
                            dest_gport = p_nh_param->gport;
                            CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_REPLI_EN);
                            CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN);
                        }

                        if (DRV_IS_DUET2(lchip) && CTC_FLAG_ISSET(p_nh_param->p_vlan_edit_info->flag, CTC_VLAN_NH_LENGTH_ADJUST_EN))
                        {
                            p_nh_param->p_vlan_edit_info->stats_id = 0;
                            use_rsv_dsnh = 1;
                            dsnh_offset = p_master->xlate_nh_offset + p_nh_param->p_vlan_edit_info->output_svid;
                        }
                    }
                }
                pkt_nh_edit_mode  = (sys_usw_chip_get_rchain_en()&&lchip)?SYS_NH_EGS_CHIP_EDIT_MODE: SYS_NH_IGS_CHIP_EDIT_MODE;
                dest_gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_param->gport);
            }
            else if (p_nh_param->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT)
            {
                bool share_nh = FALSE;
                CTC_ERROR_RETURN(sys_usw_aps_get_share_nh(lchip, p_nh_param->gport, &share_nh));

                if (share_nh)
                {
                    p_nhinfo->hdr.dsnh_entry_num  = 1;
                    if (p_nh_param->p_vlan_edit_info->svlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE
                        && p_nh_param->p_vlan_edit_info->cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE)
                    {
                        CTC_ERROR_RETURN(_sys_usw_nh_get_resolved_offset(lchip,
                                             SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, &dsnh_offset));
                        use_rsv_dsnh = 1;
                    }

                    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH);
                }
                else
                {
                    p_nhinfo->hdr.dsnh_entry_num  = 2;
                }
                pkt_nh_edit_mode  = (sys_usw_chip_get_rchain_en()&&lchip)?SYS_NH_EGS_CHIP_EDIT_MODE: SYS_NH_IGS_CHIP_EDIT_MODE;
            }
            else if (p_nh_param->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC)
            {
                CTC_ERROR_RETURN(_sys_usw_nh_get_resolved_offset(lchip,
                                     SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, &dsnh_offset));
                p_nhinfo->hdr.dsnh_entry_num  = 1;
                use_rsv_dsnh = 1;
            }
            else if (p_nh_param->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS)
            {
                CTC_ERROR_RETURN(_sys_usw_nh_get_resolved_offset(lchip,
                                     SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, &dsnh_offset));

                CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);

                p_nhinfo->hdr.dsnh_entry_num  = 1;
                use_rsv_dsnh = 1;
            }
        }
        break;

    case SYS_NH_TYPE_IPUC:
        {
            sys_nh_param_ipuc_t* p_nh_param = (sys_nh_param_ipuc_t*)p_nh_com_para;
            ctc_ip_nh_param_t* p_ctc_nh_param = (ctc_ip_nh_param_t*)p_nh_param->p_ipuc_param;

            dsnh_offset = p_ctc_nh_param->dsnh_offset;
            if(p_ctc_nh_param->aps_en)
            {
                p_nhinfo->hdr.dsnh_entry_num  = 2;
            }
            else
            {
                p_nhinfo->hdr.dsnh_entry_num  = 1;
                dest_gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_ctc_nh_param->oif.gport);
            }
            if (CTC_FLAG_ISSET(p_ctc_nh_param->flag, CTC_IP_NH_FLAG_UNROV))
            {
                p_nhinfo->hdr.dsnh_entry_num = 0;
            }

            if (p_master->pkt_nh_edit_mode == 1)
            {
                pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE;
            }
        }
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_param_ip_tunnel_t* p_nh_param = (sys_nh_param_ip_tunnel_t*)p_nh_com_para;

            cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_portExtenderMcastEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &port_ext_mc_en));

            if (CTC_FLAG_ISSET(p_nh_param->p_ip_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI)
                ||  (SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_nh_param->p_ip_nh_param) && port_ext_mc_en)
                || (p_nh_param->p_ip_nh_param->tunnel_info.logic_port)
                ||(p_nh_param->p_ip_nh_param->tunnel_info.inner_packet_type ==PKT_TYPE_IPV4 )
                ||(p_nh_param->p_ip_nh_param->tunnel_info.inner_packet_type ==PKT_TYPE_IPV6 )
                ||CTC_FLAG_ISSET(p_nh_param->p_ip_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR)
                ||(0 != p_nh_param->p_ip_nh_param->tunnel_info.vn_id)
                ||CTC_FLAG_ISSET(p_nh_param->p_ip_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_LOGIC_PORT_CHECK)
                || p_nh_param->p_ip_nh_param->tunnel_info.udf_profile_id)
            {
                CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
            }
            dsnh_offset = p_nh_param->p_ip_nh_param->dsnh_offset;
            p_nhinfo->hdr.dsnh_entry_num  = 1;
            pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE;
            if (!CTC_FLAG_ISSET(p_nh_param->p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR))
            {
                dest_gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_param->p_ip_nh_param->oif.gport);
            }

            if (CTC_FLAG_ISSET(p_nh_param->p_ip_nh_param->flag, CTC_IP_NH_FLAG_UNROV))
            {
                p_nhinfo->hdr.dsnh_entry_num = 0;
            }
        }
        break;

    case SYS_NH_TYPE_WLAN_TUNNEL:
        {
            sys_nh_param_wlan_tunnel_t* p_nh_param = (sys_nh_param_wlan_tunnel_t*)p_nh_com_para;
            CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
            dsnh_offset = p_nh_param->p_wlan_nh_param->dsnh_offset;
            p_nhinfo->hdr.dsnh_entry_num  = 1;
            pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE;
        }
        break;

    case SYS_NH_TYPE_TRILL:
        {
            sys_nh_param_trill_t* p_nh_param = (sys_nh_param_trill_t*)p_nh_com_para;
            dsnh_offset = p_nh_param->p_trill_nh_param->dsnh_offset;
            p_nhinfo->hdr.dsnh_entry_num  = 1;
            pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE;
            dest_gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_param->p_trill_nh_param->oif.gport);

            if (CTC_FLAG_ISSET(p_nh_param->p_trill_nh_param->flag, CTC_NH_TRILL_IS_UNRSV))
            {
                p_nhinfo->hdr.dsnh_entry_num = 0;
            }
        }
        break;

    case SYS_NH_TYPE_MISC:
        {
            sys_nh_param_misc_t* p_nh_param = (sys_nh_param_misc_t*)p_nh_com_para;

            if(CTC_MISC_NH_TYPE_TO_CPU == p_nh_param->p_misc_param->type)
            {
                p_nhinfo->hdr.dsnh_entry_num  = 0;
            }
            else if ( CTC_MISC_NH_TYPE_LEAF_SPINE == p_nh_param->p_misc_param->type)
            {
                p_nhinfo->hdr.dsnh_entry_num  = 0;
                CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);
            }
            else
            {
                dsnh_offset = p_nh_param->p_misc_param->dsnh_offset;
                p_nhinfo->hdr.dsnh_entry_num  = 1;
                if ((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == p_nh_param->p_misc_param->type) ||
                    (CTC_MISC_NH_TYPE_OVER_L2 == p_nh_param->p_misc_param->type) ||
                    (p_nh_param->p_misc_param->cid && !g_usw_nh_master[lchip]->cid_use_4w) ||
                    (p_nh_param->p_misc_param->stats_id && g_usw_nh_master[lchip]->cid_use_4w) ||
                    p_nh_param->p_misc_param->misc_param.flex_edit.user_vlanptr)
                {
                    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                }
                 pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE;
                 dest_gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_param->p_misc_param->gport);
            }
        }
        break;

    case SYS_NH_TYPE_RSPAN:
        {
            sys_nh_param_rspan_t* p_nh_para = (sys_nh_param_rspan_t*)p_nh_com_para;
            dsnh_offset = p_nh_para->p_rspan_param->dsnh_offset;
            p_nhinfo->hdr.dsnh_entry_num  = 1;
             /*remote_chip = p_nh_para->p_rspan_param->remote_chip;*/
            pkt_nh_edit_mode  = SYS_NH_IGS_CHIP_EDIT_MODE;
        }
        break;

    case SYS_NH_TYPE_MPLS:
        {
            sys_nh_param_mpls_t* p_nh_para = (sys_nh_param_mpls_t*)p_nh_com_para;

            ctc_vlan_egress_edit_info_t* p_vlan_info = &p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.nh_com.vlan_info;

            dsnh_offset = p_nh_para->p_mpls_nh_param->dsnh_offset;
            if ((p_nh_para->p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
                (p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN))
            {
                if (!p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.loop_nhid)
                {
                    CTC_ERROR_RETURN(sys_usw_nh_check_mpls_tunnel_exp(lchip, p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.tunnel_id, &exp_type));
                    CTC_ERROR_RETURN(sys_usw_nh_check_mpls_tunnel_is_spme(lchip, p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.tunnel_id, &tunnel_is_spme));
                }
                if((p_vlan_info != NULL)
                    && ((CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID))
                     ||((CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
                     ||(CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
                     || (p_vlan_info->cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_STRIP_VLAN)))
                     && (!tunnel_is_spme))
                {
                    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                }

                if ((p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.label_num
                    && (CTC_NH_EXP_SELECT_PACKET == p_nh_para->p_mpls_nh_param->nh_para.nh_param_push.push_label[0].exp_type)))
                {
                    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                }

                if (exp_type)
                {
                    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                }

                if (p_nh_para->p_mpls_nh_param->aps_en)
                {
                    if ((p_nh_para->p_mpls_nh_param->nh_p_para.nh_p_param_push.label_num
                        && (CTC_NH_EXP_SELECT_PACKET == p_nh_para->p_mpls_nh_param->nh_p_para.nh_p_param_push.push_label[0].exp_type)))
                    {
                        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                    }

                    if (!p_nh_para->p_mpls_nh_param->nh_p_para.nh_p_param_push.loop_nhid)
                    {
                        CTC_ERROR_RETURN(sys_usw_nh_check_mpls_tunnel_exp(lchip, p_nh_para->p_mpls_nh_param->nh_p_para.nh_p_param_push.tunnel_id, &exp_type));
                    }
                    if (exp_type)
                    {
                        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
                    }

                }

            }

            if (p_nh_para->p_mpls_nh_param->aps_en)
            {
                p_nhinfo->hdr.dsnh_entry_num  = 2;
            }
            else
            {
                p_nhinfo->hdr.dsnh_entry_num  = 1;
            }
            pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE;

            if (CTC_FLAG_ISSET(p_nh_para->p_mpls_nh_param->flag, CTC_MPLS_NH_IS_UNRSV))
            {
                p_nhinfo->hdr.dsnh_entry_num = 0;
            }
        }
        break;

    case SYS_NH_TYPE_ECMP:
    case SYS_NH_TYPE_MCAST:
    case SYS_NH_TYPE_DROP:
    case SYS_NH_TYPE_TOCPU:
    case SYS_NH_TYPE_UNROV:
    case SYS_NH_TYPE_ILOOP:
        p_nhinfo->hdr.dsnh_entry_num  = 0;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    /*No need alloc  and use dsnh offset*/
    if (0 == p_nhinfo->hdr.dsnh_entry_num)
    {
        return CTC_E_NONE;
    }

    if (sys_usw_chip_get_rchain_en() && lchip && (pkt_nh_edit_mode == SYS_NH_EGS_CHIP_EDIT_MODE))
    {
        sys_nh_info_com_t* p_nh_com_info;
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(0, p_nh_com_para->hdr.nhid, (sys_nh_info_com_t**)&p_nh_com_info));
        dsnh_offset = p_nh_com_info->hdr.dsfwd_info.dsnh_offset;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Rchain enable, lchip %u nhid %u dsnh_offset;%u \n", lchip, p_nh_com_para->hdr.nhid, dsnh_offset);
    }

    if (use_rsv_dsnh)
    {
        p_nhinfo->hdr.dsfwd_info.dsnh_offset = dsnh_offset;
        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSNH);

    }
    else if(CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_REPLI_EN))
    {
        CTC_ERROR_RETURN(_sys_usw_nh_alloc_logic_replicate_offset(lchip, p_nhinfo, dest_gport, dsnh_offset));
        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH);
    }
    else if ((p_master->pkt_nh_edit_mode == 1 && (pkt_nh_edit_mode == SYS_NH_EGS_CHIP_EDIT_MODE) && dsnh_offset) ||
           (p_master->pkt_nh_edit_mode > 1 && (pkt_nh_edit_mode == SYS_NH_EGS_CHIP_EDIT_MODE)))
    {

        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
        {
            dsnh_entry_num = p_nhinfo->hdr.dsnh_entry_num *2;
        }
        else
        {
            dsnh_entry_num = p_nhinfo->hdr.dsnh_entry_num ;
        }
        p_nhinfo->hdr.dsfwd_info.dsnh_offset = dsnh_offset;
        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);

        if (!SYS_GCHIP_IS_REMOTE(lchip, dest_gchip) || (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV)))
        {
            CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, dsnh_offset));
            CTC_ERROR_RETURN(sys_usw_nh_check_glb_nh_offset(lchip, dsnh_offset, dsnh_entry_num, TRUE));
            CTC_ERROR_RETURN(sys_usw_nh_set_glb_nh_offset(lchip, p_nhinfo->hdr.dsfwd_info.dsnh_offset, dsnh_entry_num, TRUE));
        }

    }
    else
    {
        if (p_nh_com_para->hdr.nh_param_type == SYS_NH_TYPE_RSPAN)
        {
            /*Only used for compatible GG Ingress Edit Mode */
            sys_nh_param_rspan_t* p_nh_para = (sys_nh_param_rspan_t*)p_nh_com_para;

            dsnh_offset = SYS_DSNH_INDEX_FOR_REMOTE_MIRROR;
            p_master->rspan_vlan_id  = p_nh_para->p_rspan_param->rspan_vid;
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
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_free_offset_by_nhinfo(uint8 lchip, sys_usw_nh_type_t nh_type, sys_nh_info_com_t* p_nhinfo)
{

    sys_usw_nh_master_t* p_master;
    uint8 dsnh_entry_num = 0;


    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSNH) || !p_nhinfo->hdr.dsnh_entry_num)
    {
        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_REPLI_EN))
    {
        CTC_ERROR_RETURN(_sys_usw_nh_free_logic_replicate_offset(lchip, p_nhinfo));
        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH);
    }
    else if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH))
    {
            if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
            {
                CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, p_nhinfo->hdr.dsnh_entry_num, p_nhinfo->hdr.dsfwd_info.dsnh_offset));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, p_nhinfo->hdr.dsnh_entry_num, p_nhinfo->hdr.dsfwd_info.dsnh_offset));
            }


        CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH);

    }
    else
    {
      if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
      {
         dsnh_entry_num = p_nhinfo->hdr.dsnh_entry_num *2;
      }
      else
      {
         dsnh_entry_num = p_nhinfo->hdr.dsnh_entry_num;
      }
      CTC_ERROR_RETURN(sys_usw_nh_set_glb_nh_offset(lchip, p_nhinfo->hdr.dsfwd_info.dsnh_offset,dsnh_entry_num , FALSE));
    }

    p_nhinfo->hdr.dsnh_entry_num = 0;

    return CTC_E_NONE;
}
int32
sys_usw_nh_copy_nh_entry_flags(uint8 lchip, sys_nh_info_com_hdr_t *p_old_hdr, sys_nh_info_com_hdr_t *p_new_hdr )
{

    sal_memcpy(p_new_hdr, p_old_hdr, sizeof(sys_nh_info_com_hdr_t));

    p_new_hdr->nh_entry_flags = 0;
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
       CTC_SET_FLAG((p_new_hdr->nh_entry_flags), SYS_NH_INFO_FLAG_USE_DSNH8W);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSNH))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSNH);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_REPLI_EN))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_REPLI_EN);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_DSFWD_IS_6W))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_DSFWD_IS_6W);
    }
    if (CTC_FLAG_ISSET(p_old_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED))
    {
       CTC_SET_FLAG(p_new_hdr->nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED);
    }
    return CTC_E_NONE;
}

int32
sys_usw_nh_get_emcp_if_l2edit_offset(uint8 lchip, uint32* p_l2edit_offset)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    *p_l2edit_offset = p_nh_master->ecmp_if_resolved_l2edit;

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_ipmc_dsnh_offset(uint8 lchip, uint16 l3ifid, uint8 mtu_no_chk, uint32* p_dsnh_offset)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    if ( MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID) == l3ifid )
    {
        _sys_usw_nh_get_resolved_offset(lchip,
                                 SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, p_dsnh_offset);
        return CTC_E_NONE;
    }

    if (l3ifid > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Invalid interface ID! \n");
			return CTC_E_BADID;

    }

    if (p_nh_master->ipmc_dsnh_offset[l3ifid] == 0)
    {

        sys_nh_param_dsnh_t dsnh_param;
        sys_l3if_prop_t l3if_prop;

        sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));

        l3if_prop.l3if_id = l3ifid;
        CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop));
        dsnh_param.l2edit_ptr = 0;
        dsnh_param.l3edit_ptr = 0;
        dsnh_param.dest_vlan_ptr = l3if_prop.vlan_ptr;
        dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_IPMC;
        dsnh_param.mtu_no_chk = mtu_no_chk;

        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1,
                                                       &dsnh_param.dsnh_offset));

        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param));
        p_nh_master->ipmc_dsnh_offset[l3ifid] = dsnh_param.dsnh_offset;
        *p_dsnh_offset = dsnh_param.dsnh_offset;

    }
    else
    {
        *p_dsnh_offset = p_nh_master->ipmc_dsnh_offset[l3ifid];
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_check_max_glb_nh_offset(uint8 lchip, uint32 offset)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (p_nh_master->pkt_nh_edit_mode == SYS_NH_EGS_CHIP_EDIT_MODE
        && (offset >= p_nh_master->max_glb_nh_offset))
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_check_max_glb_met_offset(uint8 lchip, uint32 offset)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (offset >= p_nh_master->max_glb_met_offset)
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_check_glb_nh_offset(uint8 lchip, uint32 start_offset, uint32 entry_num,
                                     bool should_not_inuse)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32* p_bmp = NULL;
    uint32 curr_offset = 0;
    uint32 i = 0;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if (NULL == (p_bmp = p_nh_master->p_occupid_nh_offset_bmp))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    if (0 == entry_num)
    {
        return CTC_E_NONE;
    }

    for (i = 0; i < entry_num; i++)
    {
        curr_offset = start_offset + i;
        if (should_not_inuse && CTC_FLAG_ISSET(p_bmp[((curr_offset) >> BITS_SHIFT_OF_WORD)],
                                               (1 << (curr_offset & BITS_MASK_OF_WORD))))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Global DsNexthop offset or GroupId is used in other nexthop cant re-use it \n");
			return CTC_E_IN_USE;

        }

        if ((!should_not_inuse) && (!CTC_FLAG_ISSET(p_bmp[((curr_offset) >> BITS_SHIFT_OF_WORD)],
                                                    (1 << (curr_offset & BITS_MASK_OF_WORD)))))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Global DsNexthop offset or GroupId isnt used  cant free it \n");
			return CTC_E_IN_USE;

        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_set_glb_nh_offset(uint8 lchip, uint32 start_offset, uint32 entry_num, bool is_set)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint32* p_bmp = NULL;
    uint32 curr_offset = 0;
    int32 i = 0;

    if (0 == entry_num)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if ((start_offset + entry_num - 1) > p_nh_master->max_glb_nh_offset)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (NULL == (p_bmp = p_nh_master->p_occupid_nh_offset_bmp))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    for (i = 0; i < entry_num; i++)
    {
        curr_offset = start_offset + i;
        if (is_set)
        {
            CTC_SET_FLAG(p_bmp[(curr_offset >> BITS_SHIFT_OF_WORD)],
                         (1 << (curr_offset & BITS_MASK_OF_WORD)));
        }
        else
        {
            CTC_UNSET_FLAG(p_bmp[(curr_offset >> BITS_SHIFT_OF_WORD)],
                           (1 << (curr_offset & BITS_MASK_OF_WORD)));
        }
    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_merge_dsfwd_disable(uint8 lchip, uint32 nhid)
{
    sys_nh_info_com_t* p_nh_info = NULL;

    sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_info);
    if(!p_nh_info )
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop not exist \n");
		return CTC_E_NOT_EXIST;
    }

    switch(p_nh_info->hdr.nh_entry_type )
    {
        case SYS_NH_TYPE_IPUC:
        {
            sys_nh_info_ipuc_t* p_ipuc_info = (sys_nh_info_ipuc_t*)p_nh_info;

            if (p_ipuc_info->updateAd)
            {
                /*update old ad using dsfwd */
                p_ipuc_info->updateAd(lchip, p_ipuc_info->data, NULL);
                p_ipuc_info->updateAd = NULL;
                p_ipuc_info->data = NULL;
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Nexthop]Host nexthop update to dsfwd. \n");
    			return CTC_E_NONE;
            }
        }
            break;
        case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)p_nh_info;

            if (p_ip_tunnel_info->updateAd)
            {
                /*update old ad using dsfwd */
                p_ip_tunnel_info->updateAd(lchip, p_ip_tunnel_info->data, NULL);
                p_ip_tunnel_info->updateAd = NULL;
                p_ip_tunnel_info->data = NULL;
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Nexthop]IP-Tunnel nexthop update to dsfwd. \n");
    			return CTC_E_NONE;
            }
        }
            break;
        case SYS_NH_TYPE_MPLS:
        {
            sys_nh_info_mpls_t* p_mpls_info = (sys_nh_info_mpls_t*)p_nh_info;

            if (p_mpls_info->updateAd)
            {
                /*update old ad using dsfwd */
                p_mpls_info->updateAd(lchip, p_mpls_info->data, NULL);
                p_mpls_info->updateAd = NULL;
                p_mpls_info->data = NULL;
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Nexthop]MPLS nexthop update to dsfwd. \n");
    			return CTC_E_NONE;
            }
        }
            break;
        case SYS_NH_TYPE_TRILL:
        {
            sys_nh_info_trill_t* p_trill_info = (sys_nh_info_trill_t*)p_nh_info;

            if (p_trill_info->updateAd)
            {
                /*update old ad using dsfwd */
                p_trill_info->updateAd(lchip, p_trill_info->data, NULL);
                p_trill_info->updateAd = NULL;
                p_trill_info->data = NULL;
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Nexthop]Trill nexthop update to dsfwd. \n");
    			return CTC_E_NONE;
            }
        }
            break;
        case SYS_NH_TYPE_MISC:
        {
            sys_nh_info_misc_t* p_misc_info = (sys_nh_info_misc_t*)p_nh_info;

            if (p_misc_info->updateAd)
            {
                /*update old ad using dsfwd */
                p_misc_info->updateAd(lchip, p_misc_info->data, NULL);
                p_misc_info->updateAd = NULL;
                p_misc_info->data = NULL;
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " [Nexthop]Misc nexthop update to dsfwd. \n");
    			return CTC_E_NONE;
            }
        }
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_update_oam_ref_info(uint8 lchip, void* p_nhinfo, sys_nh_oam_info_t* p_oam_info, uint8 is_add)
{
    sys_nh_ref_oam_node_t* p_head = NULL;
    sys_nh_ref_oam_node_t* p_curr = NULL;
    sys_nh_ref_oam_node_t* p_nh_ref_list_node = NULL;
    sys_nh_ref_oam_node_t* p_prev = NULL;
    sys_nh_info_mpls_t* p_mpls_nh = NULL;
    sys_nh_info_ipuc_t* p_ipuc_nh = NULL;
    uint8 find_flag = 0;
    sys_nh_info_mpls_t* p_mpls_info = NULL;
    p_mpls_info = (sys_nh_info_mpls_t*)p_nhinfo;
    if (p_mpls_info->hdr.nh_entry_type == SYS_NH_TYPE_MPLS)
    {
        p_mpls_nh = (sys_nh_info_mpls_t*)p_nhinfo;
    	p_curr = p_mpls_nh->p_ref_oam_list;
    }
    else if (p_mpls_info->hdr.nh_entry_type == SYS_NH_TYPE_IPUC)
    {
        p_ipuc_nh = (sys_nh_info_ipuc_t*)p_nhinfo;
    	p_curr = p_ipuc_nh->p_ref_oam_list;
    }
    else
    {
        return CTC_E_INVALID_CONFIG;
    }

	p_head = p_curr;
	if (is_add)
	{
	    while (p_curr)
	    {
	        if ((p_curr->ref_oam.mep_index == p_oam_info->mep_index) && (p_curr->ref_oam.mep_type == p_oam_info->mep_type))
	        {
	            find_flag = 1;
                p_curr->ref_oam.ref_cnt++;
	            break;
	        }
	        p_curr = p_curr->p_next;
	    }

	    if (find_flag == 0)
	    {
	        p_nh_ref_list_node = (sys_nh_ref_oam_node_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_ref_oam_node_t));
	        if (p_nh_ref_list_node == NULL)
	        {
	            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    			return CTC_E_NO_MEMORY;
	        }

            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add mep info, mep_type:%d, mep_index:%d, ref_cnt:%u\n",
                p_oam_info->mep_type, p_oam_info->mep_index, p_oam_info->ref_cnt);
    		sal_memset(p_nh_ref_list_node, 0, sizeof(sys_nh_ref_oam_node_t));
    		p_nh_ref_list_node->ref_oam.mep_index = p_oam_info->mep_index;
    		p_nh_ref_list_node->ref_oam.mep_type = p_oam_info->mep_type;
            p_nh_ref_list_node->ref_oam.ref_cnt++;
            p_nh_ref_list_node->p_next = p_head;
            p_head = p_nh_ref_list_node;
            if (p_mpls_info->hdr.nh_entry_type == SYS_NH_TYPE_MPLS)
            {
    			((sys_nh_info_mpls_t*)p_nhinfo)->p_ref_oam_list = p_head;
            }
            else if (p_mpls_info->hdr.nh_entry_type == SYS_NH_TYPE_IPUC)
            {
    			((sys_nh_info_ipuc_t*)p_nhinfo)->p_ref_oam_list = p_head;
            }
	    }
    }
    else
    {
        while (p_curr)
        {
            if ((p_curr->ref_oam.mep_index == p_oam_info->mep_index) && (p_curr->ref_oam.mep_type == p_oam_info->mep_type))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Delete mep info, mep_type:%d, mep_index:%d ref_cnt:%u\n",
                    p_oam_info->mep_type, p_oam_info->mep_index, p_curr->ref_oam.ref_cnt);
                if (p_curr->ref_oam.ref_cnt == 1)
                {
                    if (NULL == p_prev)
                    /*Remove first node*/
                    {
                        if (p_mpls_info->hdr.nh_entry_type == SYS_NH_TYPE_MPLS)
                        {
            				p_mpls_nh->p_ref_oam_list = p_curr->p_next;
                        }
                        else if (p_mpls_info->hdr.nh_entry_type == SYS_NH_TYPE_IPUC)
                        {
            				p_ipuc_nh->p_ref_oam_list = p_curr->p_next;
                        }
                    }
                    else
                    {
                        p_prev->p_next = p_curr->p_next;
                    }
                    mem_free(p_curr);
                }
                else
                {
                    p_curr->ref_oam.ref_cnt--;
                }
                break;
            }
            p_prev = p_curr;
            p_curr = p_curr->p_next;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_update_oam_mep(uint8 lchip, sys_nh_ref_oam_node_t* p_head, uint16 arp_id)
{
    sys_nh_ref_oam_node_t* p_curr = NULL;
    uint32 mep_index = 0;
    sys_nh_db_arp_t* p_arp = NULL;

    p_arp = (sys_usw_nh_lkup_arp_id(lchip, arp_id));
    if (NULL == p_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

	p_curr = p_head;
	while (p_curr)
	{
    	uint32 cmd = 0;
        ds_t ds_mep;
        ds_t ds_mep_mask;
        tbl_entry_t  tbl_entry;

		mep_index = p_curr->ref_oam.mep_index;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Update oam mep: type:%d, mep_index:%d \n", p_curr->ref_oam.mep_type, mep_index);
        sal_memset(&ds_mep, 0, sizeof(ds_t));
        sal_memset(&ds_mep_mask, 0xFF, sizeof(ds_t));
        cmd = DRV_IOR(DsBfdMep_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, mep_index, cmd, &ds_mep));
        if (!p_curr->ref_oam.mep_type)
        {
            if (p_arp->destmap_profile)
            {
                SetDsBfdMep(V, destMap_f,&ds_mep, SYS_ENCODE_ARP_DESTMAP(p_arp->destmap_profile));
            }
            else
            {
                SetDsBfdMep(V, destMap_f,&ds_mep, SYS_ENCODE_DESTMAP(SYS_MAP_CTC_GPORT_TO_GCHIP(p_arp->gport),
                    SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_arp->gport)));
            }
            SetDsBfdMep(V, destMap_f,&ds_mep_mask, 0);
        }
        else
        {
            if (p_arp->destmap_profile)
            {
                SetDsEthMep(V, destMap_f,&ds_mep, SYS_ENCODE_ARP_DESTMAP(p_arp->destmap_profile));
            }
            else
            {
                SetDsEthMep(V, destMap_f,&ds_mep, SYS_ENCODE_DESTMAP(SYS_MAP_CTC_GPORT_TO_GCHIP(p_arp->gport),
                    SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_arp->gport)));
            }
            SetDsEthMep(V, destMap_f, &ds_mep_mask, 0);
        }
        cmd = DRV_IOW(DsBfdMep_t, DRV_ENTRY_FLAG);
        tbl_entry.data_entry = (uint32*)&ds_mep;
        tbl_entry.mask_entry = (uint32*)&ds_mep_mask;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, mep_index, cmd, &tbl_entry));
		p_curr = p_curr->p_next;
    }

    return CTC_E_NONE;
}

#define _________ds_write________
#if 0
/**
 @brief This function is deinit nexthop module   callback function for each nexthop type
 */
int32
sys_usw_nh_global_dync_entry_set_default(uint8 lchip, uint32 min_offset, uint32 max_offset)
{

    return CTC_E_NONE;
}
#endif

int32
sys_usw_nh_mpls_add_cw(uint8 lchip, uint32 cw,
                                 uint8* cw_index,
                                 bool is_add)
{
    uint8 bit = 0;
    uint32 cmd = 0;
    uint8 free = 0;
    uint32 field_val = 0;
    sys_usw_nh_master_t* p_master;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_master));

    if (is_add)
    {
        for (bit = 1; bit < SYS_NH_CW_NUM; bit++)
        {
            if (p_master->cw_ref_cnt[bit])
            {
                cmd = DRV_IOR(DsControlWord_t, DsControlWord_cwData_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, bit, cmd, &field_val));

                if (field_val == cw)
                {
                    if (0xFFFF == p_master->cw_ref_cnt[bit])
                    {
                        return CTC_E_NO_RESOURCE;
                    }
                    p_master->cw_ref_cnt[bit]++;
                    *cw_index = bit;
                    return CTC_E_NONE;
                }

                break;
            }
            else if (free == 0)
            {
                free = bit;
            }

        }

        if (free != 0)
        {
            field_val = cw;
            cmd = DRV_IOW(DsControlWord_t, DsControlWord_cwData_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, free, cmd, &field_val));
            p_master->cw_ref_cnt[free]++;
            *cw_index = free;
            return CTC_E_NONE;
        }

        return CTC_E_INVALID_PARAM;

    }
    else
    {
        bit = *cw_index;
        p_master->cw_ref_cnt[bit]--;
        *cw_index = 0;
        if (p_master->cw_ref_cnt[bit] == 0)
        {
            field_val = 0;
            cmd = DRV_IOW(DsControlWord_t, DsControlWord_cwData_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, bit, cmd, &field_val));
        }
    }

    return CTC_E_NONE;

}



STATIC int32
sys_usw_nh_dsnh_build_vlan_info(uint8 lchip, sys_nexthop_t* p_dsnh,
                                      ctc_vlan_egress_edit_info_t* p_vlan_info)
{
    if (!p_vlan_info)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid pointer \n");
			return CTC_E_INVALID_PTR;

    }

    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_VLAN_SWAP_EN))
    {
        /*1. cvlan_tag_op_disable = 1*/
        p_dsnh->cvlan_tag_disable = TRUE;

        /*2. svlan_tag_op_disable = 1*/
        p_dsnh->svlan_tag_disable = TRUE;

        /*3. tagged_mode = 1*/
        p_dsnh->tagged_mode = TRUE;

        /*Unset  parametre's cvid valid bit, other with output
        cvlan id valid will be overwrited in function
        sys_usw_nh_dsnh4w_assign_vid, bug11323*/
        CTC_UNSET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
        CTC_UNSET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);

        /*Swap TPID*/
        if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_TPID_SWAP_EN))
        {
            p_dsnh->tpid_swap = TRUE;
        }

        /*Swap Cos*/
        if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_COS_SWAP_EN))
        {
            p_dsnh->cos_swap = TRUE;
        }
    }
    else
    {
        switch (p_vlan_info->svlan_edit_type)
        {
        case CTC_VLAN_EGRESS_EDIT_NONE:
        case CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE:
            p_dsnh->output_svlan_id_valid = FALSE;
            CTC_UNSET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            p_dsnh->svlan_tagged = TRUE;
            p_dsnh->derive_stag_cos  = 1;
            p_dsnh->stag_cfi  = 0;       /*use  raw packet's cos*/
            break;
        case CTC_VLAN_EGRESS_EDIT_INSERT_VLAN:
            if (CTC_VLAN_EGRESS_EDIT_INSERT_VLAN != p_vlan_info->cvlan_edit_type &&
                CTC_VLAN_EGRESS_EDIT_NONE != p_vlan_info->cvlan_edit_type &&
                CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE != p_vlan_info->cvlan_edit_type)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Configure confilct if cvlan(svlan) edit type is insert-vlan svlan(cvlan) edit type should not be edit-none or edit-replace \n");
    			return CTC_E_INVALID_CONFIG;
            }

            p_dsnh->tagged_mode = TRUE; /*insert stag*/
            p_dsnh->svlan_tagged = TRUE;
            p_dsnh->stag_cfi  = 0;
            p_dsnh->derive_stag_cos  = 1;

            if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_SVLAN_AWARE))
            {
                p_dsnh->stag_action = 1;
            }
            break;

        case CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN:
            p_dsnh->svlan_tagged = TRUE;
            p_dsnh->stag_cfi  = 0;
            p_dsnh->derive_stag_cos  = 1;
            if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_SVLAN_AWARE))
            {
                p_dsnh->stag_action = 3;
            }
            break;

        case CTC_VLAN_EGRESS_EDIT_STRIP_VLAN:
            p_dsnh->svlan_tagged = FALSE;
            if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_SVLAN_AWARE))
            {
                p_dsnh->stag_action = 2;
            }
            break;

        default:
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Egress Vlan Edit type is invalid \n");
			return CTC_E_INVALID_CONFIG;

        }

       /* stag cos opreation */
        if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS))
        {
            p_dsnh->replace_stag_cos    = 1;
            p_dsnh->derive_stag_cos     = 0;
            p_dsnh->stag_cos            = p_vlan_info->stag_cos & 0x7;
            p_dsnh->stag_cfi  = 0;
        }
        else if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_MAP_SVLAN_COS))
        {
            p_dsnh->derive_stag_cos     = 1;
            p_dsnh->replace_stag_cos    = 1;
        }

        switch (p_vlan_info->cvlan_edit_type)
        {
        case CTC_VLAN_EGRESS_EDIT_NONE:

            /*Don't use DsNexthop's output cvlanid*/
            p_dsnh->output_cvlan_id_valid = FALSE;
            p_dsnh->cvlan_tag_disable = FALSE;
            CTC_UNSET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;

        case CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE:
            p_dsnh->cvlan_tag_disable = TRUE;
            break;

        case CTC_VLAN_EGRESS_EDIT_INSERT_VLAN:
            p_dsnh->output_cvlan_id_valid = TRUE;
            break;

        case CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN:
            break;

        case CTC_VLAN_EGRESS_EDIT_STRIP_VLAN:
            p_dsnh->cvlan_strip = TRUE;
            break;

        default:
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Egress Vlan Edit type is invalid \n");
			return CTC_E_INVALID_CONFIG;

        }
    }
    if((CTC_VLAN_EGRESS_EDIT_NONE == p_vlan_info->cvlan_edit_type && CTC_VLAN_EGRESS_EDIT_NONE == p_vlan_info->svlan_edit_type)
        && (!CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_VLAN_SWAP_EN)))
    {
        p_dsnh->vlan_xlate_mode = 0;
    }
    return CTC_E_NONE;
}

STATIC int32
sys_usw_nh_dsnh4w_assign_vid(uint8 lchip, sys_nexthop_t* p_dsnh,
                                   ctc_vlan_egress_edit_info_t* p_vlan_info)
{
    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
    {
        p_dsnh->output_cvlan_id_valid = TRUE;
        p_dsnh->output_cvlan_id = p_vlan_info->output_cvid;
    }

    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
    {
        p_dsnh->output_svlan_id_valid = TRUE;
        p_dsnh->output_svlan_id= p_vlan_info->output_svid;
    }

    return CTC_E_NONE;
}

STATIC int32
sys_usw_nh_dsnh8w_assign_vid(uint8 lchip, sys_nexthop_t* p_dsnh8w,
                                   ctc_vlan_egress_edit_info_t* p_vlan_info)
{
    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
    {
        p_dsnh8w->output_cvlan_id_valid = TRUE;
        p_dsnh8w->output_cvlan_id = p_vlan_info->output_cvid;
    }

    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
    {
        p_dsnh8w->output_svlan_id_valid = TRUE;
        p_dsnh8w->output_svlan_id = p_vlan_info->output_svid;
    }

    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID))
    {
        if(p_vlan_info->svlan_tpid_index>3)
        {
            return CTC_E_INVALID_PARAM;
        }
        p_dsnh8w->svlan_tpid_en = TRUE;
        p_dsnh8w->svlan_tpid = p_vlan_info->svlan_tpid_index;
    }

    return CTC_E_NONE;
}

/*
   inner_edit:ptr_type
   0- L2Edit
   1- L3Edit

  outer_edit: location/type
   0/1   L3Edit (Pipe0) -->( L3Edit (Pipe1),opt)--> L2Edit(Pipe2)
   0/0   L2Edit (Pipe0) --> L3Edit(Pipe1)
   1/0   L2Edit (Pipe2)
   1/1   L3Edit (Pipe1) --> L2Edit(Pipe2)
*/

int32
sys_usw_nh_write_entry_dsnh4w(uint8 lchip, sys_nh_param_dsnh_t* p_dsnh_param)
{

    sys_nexthop_t dsnh;
    uint32 op_bridge = SYS_NH_OP_NONE;
    uint8 xgpon_en = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8  bpe_pe_mode = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_dsnh_param);
    sal_memset(&dsnh, 0, sizeof(dsnh));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "\n\
                   lchip = % d, \n\
                   flag = 0x%x, \n\
                   dsnh_offset = % d, \n\
                   dsnh_type = % d\n, \
                   l2edit_ptr = % d, \n\
                   lspedit_ptr = % d, \n\
                   l3EditPtr = % d \n",
                   lchip,
                   p_dsnh_param->flag,
                   p_dsnh_param->dsnh_offset,
                   p_dsnh_param->dsnh_type,
                   p_dsnh_param->l2edit_ptr,
                   p_dsnh_param->lspedit_ptr,
                   p_dsnh_param->l3edit_ptr);

    if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP))
    {
        dsnh.aps_bridge_en = 1;
    }

    dsnh.stats_ptr  = p_dsnh_param->stats_ptr;
    dsnh.cid_valid = p_dsnh_param->cid?1:0;
    dsnh.cid       = p_dsnh_param->cid;

    cmd = DRV_IOR(EpeNextHopReserved_t, EpeNextHopReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &value));
    bpe_pe_mode = (value&0x1)?1:0;
    if (bpe_pe_mode && p_dsnh_param->stats_ptr)
    {
        /*bpe ecid enable, cannot support nh stats*/
        return CTC_E_INVALID_CONFIG;
    }

    if (dsnh.cid_valid && !g_usw_nh_master[lchip]->cid_use_4w)
    {
        return CTC_E_INVALID_CONFIG;
    }

    switch (p_dsnh_param->dsnh_type)
    {
    case SYS_NH_PARAM_DSNH_TYPE_IPUC:
        dsnh.mtu_check_en = !(p_dsnh_param->mtu_no_chk);

        /*IPUC only use ROUTE_COMPACT in current GB SDK*/
        if ((p_dsnh_param->l2edit_ptr == 0) && (p_dsnh_param->inner_l2edit_ptr == 0))
        {
            dsnh.payload_operation = SYS_NH_OP_ROUTE_COMPACT;
            dsnh.share_type = SYS_NH_SHARE_TYPE_MAC_DA;
            SYS_USW_NH_DSNH_ASSIGN_MACDA(dsnh, (*p_dsnh_param));
            if(CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ROUTE_NOTTL))
            {
                dsnh.ttl_no_dec = TRUE;
            }
            else
            {
                dsnh.ttl_no_dec = FALSE;
            }
        }
        else
        {
           CTC_SET_FLAG(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_USE_MAPPED_DSCP);
           SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);
           if(CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ROUTE_NOTTL))
           {
               dsnh.payload_operation = SYS_NH_OP_ROUTE_NOTTL;
           }
           else
           {
               dsnh.payload_operation = SYS_NH_OP_ROUTE;
           }
           dsnh.share_type = SYS_NH_SHARE_TYPE_L2EDIT_VLAN;

           dsnh.outer_edit_valid    = 1;
           dsnh.outer_edit_ptr_type = 0;/*L2edit*/
           if (p_dsnh_param->l2edit_ptr)
           {
               dsnh.outer_edit_location = 1;/*Pipe2*/
               dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;
           }
           else if (p_dsnh_param->inner_l2edit_ptr)
           {
               dsnh.outer_edit_location = 0;/*Pipe0*/
               dsnh.outer_edit_ptr = p_dsnh_param->inner_l2edit_ptr;
           }

           /*for phy if vlan, no use any more*/
           if(p_dsnh_param->p_vlan_info
              && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
               {
                dsnh.output_svlan_id_valid = p_dsnh_param->p_vlan_info->output_svid != 0;
                dsnh.output_svlan_id = p_dsnh_param->p_vlan_info->output_svid;
               }
        }
        dsnh.dest_vlan_ptr = p_dsnh_param->dest_vlan_ptr;
        break;

    case SYS_NH_PARAM_DSNH_TYPE_IPMC:
        dsnh.mtu_check_en = !(p_dsnh_param->mtu_no_chk);
        dsnh.svlan_tagged = 1;
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh.payload_operation = SYS_NH_OP_ROUTE_COMPACT;
        dsnh.share_type = SYS_NH_SHARE_TYPE_MAC_DA;

        break;

    case SYS_NH_PARAM_DSNH_TYPE_BRGUC:
        /*Set default cos action*/
        SYS_USW_NH_DSNH_SET_DEFAULT_COS_ACTION(dsnh);
        dsnh.svlan_tagged = 1;
        dsnh.dest_vlan_ptr = p_dsnh_param->dest_vlan_ptr;   /*value 0 means use srcvlanPtr*/
        dsnh.payload_operation = CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_SWAP_MAC) ?
            SYS_NH_OP_NONE : SYS_NH_OP_BRIDGE;
        dsnh.logic_dest_port     = p_dsnh_param->logic_port;
        dsnh.logic_port_check    = p_dsnh_param->logic_port_check;
        dsnh.share_type = SYS_NH_SHARE_TYPE_VLANTAG;
        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if (xgpon_en)
        {

            if (p_dsnh_param->l2edit_ptr)
            {
                dsnh.vlan_xlate_mode = 0;
                dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;
                dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;
            }
            else
            {
                if (DRV_IS_TSINGMA(lchip) && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG)
                && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
                {
                        dsnh.vlan_xlate_mode = 0;
                }
                else
                {
                    dsnh.vlan_xlate_mode = 1;       /*humber mode*/
                }
            }
            if (p_dsnh_param->l3edit_ptr && DRV_IS_DUET2(lchip))
            {
                sys_usw_nh_master_t* p_nh_master = NULL;

                sys_usw_nh_get_nh_master(lchip, &p_nh_master);
                dsnh.inner_edit_ptr_type = 1;
                dsnh.inner_edit_ptr = p_nh_master->mpls_edit_offset + p_dsnh_param->l3edit_ptr -1;
            }
        }
        else
        {
            dsnh.vlan_xlate_mode = 1;       /*humber mode*/
        }
        if (p_dsnh_param->p_vlan_info)
        {
            if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_STATS_EN))
            {
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_dsnh_param->p_vlan_info->stats_id, &(dsnh.stats_ptr)));
                if (bpe_pe_mode && dsnh.stats_ptr)
                {
                    /*bpe ecid enable, cannot support nh stats*/
                    return CTC_E_INVALID_CONFIG;
                }
            }
            if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
            {
                dsnh.is_leaf = 1;
            }
             if (p_dsnh_param->p_vlan_info->flag & CTC_VLAN_NH_MTU_CHECK_EN)
            {
                dsnh.mtu_check_en = 1;
            }
            if(CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag,CTC_VLAN_NH_HORIZON_SPLIT_EN))
            {
                dsnh.payload_operation = SYS_NH_OP_BRIDGE_VPLS;
            }
            else if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_PASS_THROUGH))
            {
                dsnh.payload_operation = SYS_NH_OP_BRIDGE_INNER;
            }
            CTC_ERROR_RETURN(sys_usw_nh_dsnh_build_vlan_info(lchip, &dsnh, p_dsnh_param->p_vlan_info));
            CTC_ERROR_RETURN(sys_usw_nh_dsnh4w_assign_vid(lchip, &dsnh, p_dsnh_param->p_vlan_info));
        }

        break;

    case SYS_NH_PARAM_DSNH_TYPE_BYPASS:
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh.payload_operation = SYS_NH_OP_NONE;
        dsnh.vlan_xlate_mode = 1;    /*humber mode*/
        break;

    case SYS_NH_PARAM_DSNH_TYPE_MPLS_PHP: /*PHP */

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_PHP_SHORT_PIPE))
        {
            dsnh.payload_operation = SYS_NH_OP_ROUTE_NOTTL;
            dsnh.use_packet_ttl = 1;
        }
        else
        {
            dsnh.payload_operation = SYS_NH_OP_ROUTE;
        }

        dsnh.mtu_check_en = TRUE;
        dsnh.dest_vlan_ptr = p_dsnh_param->dest_vlan_ptr;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);
        dsnh.share_type = SYS_NH_SHARE_TYPE_L2EDIT_VLAN;
        dsnh.outer_edit_valid    = 1;
        dsnh.outer_edit_ptr_type = 0;/*L2edit*/
        if (p_dsnh_param->l2edit_ptr)
        {
            dsnh.outer_edit_location = 1;/*Pipe2*/
            dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;
        }
        else if (p_dsnh_param->inner_l2edit_ptr)
        {
            dsnh.outer_edit_location = 0;/*Pipe0*/
            dsnh.outer_edit_ptr = p_dsnh_param->inner_l2edit_ptr;
        }
        /*for phy if vlan, no use any more*/
        if(p_dsnh_param->p_vlan_info
              && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
        {
                dsnh.output_svlan_id_valid = p_dsnh_param->p_vlan_info->output_svid != 0;
                dsnh.output_svlan_id = p_dsnh_param->p_vlan_info->output_svid;
        }
        break;
    case SYS_NH_PARAM_DSNH_TYPE_MPLS_OP_NONE:   /* mpls switch*/
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Should use DsNexthop8W table \n");
			return CTC_E_INVALID_CONFIG;

        break;

    case SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_NONE: /*Swap  label on LSR /Pop label and do   label  Swap on LER*/
        dsnh.payload_operation = SYS_NH_OP_NONE;
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);

        if(p_dsnh_param->lspedit_ptr || (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP)))
        {
            dsnh.outer_edit_location = 0;
            dsnh.outer_edit_ptr_type = 1;/*Pipe0: L3edit*/
            dsnh.outer_edit_ptr = p_dsnh_param->lspedit_ptr;
        }
        else
        {
            dsnh.outer_edit_location = p_dsnh_param->loop_edit?0:1;
            dsnh.outer_edit_ptr_type = 0;/*Pipe0: L2edit*/
            dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;
        }

        dsnh.inner_edit_ptr_type = 1;/*L3Edit*/
        dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;


        break;

    case SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_ROUTE: /* (L3VPN/FTN)*/
        dsnh.payload_operation = SYS_NH_OP_ROUTE;
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);

        dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

        if (dsnh.aps_bridge_en)
        {
            dsnh.outer_edit_location = 0;
            dsnh.outer_edit_ptr_type = 1;
            dsnh.outer_edit_ptr = p_dsnh_param->lspedit_ptr;/*aps group*/
        }
        else
        {
            if (p_dsnh_param->spme_mode)
            {
                dsnh.outer_edit_location = 1;
                dsnh.outer_edit_ptr_type = 1;
                dsnh.outer_edit_ptr = p_dsnh_param->lspedit_ptr;/*Pipe0: L3edit*/
            }
            else
            {
                if(p_dsnh_param->lspedit_ptr) /*LSP+ARP*/
                {
                    dsnh.outer_edit_location = 0;
                    dsnh.outer_edit_ptr_type = 1;
                    dsnh.outer_edit_ptr = p_dsnh_param->lspedit_ptr;/*Pipe0: L3edit*/
                }
                else /*ARP or loop*/
                {
                    dsnh.outer_edit_location = p_dsnh_param->loop_edit?0:1; /*for loop, using pipe0*/
                    dsnh.outer_edit_ptr_type = 0;
                    dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;/*Pipe3: l2edti*/
                }
            }

        }

        dsnh.inner_edit_ptr_type = 1;/* L3Edit*/
        dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;

        dsnh.mtu_check_en = TRUE;
        break;

    case SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_L2VPN:
        if (p_dsnh_param->hvpls)
        {
            op_bridge = SYS_NH_OP_BRIDGE_INNER;
        }
        else
        {
            op_bridge = SYS_NH_OP_BRIDGE_VPLS;
        }

        if (p_dsnh_param->spme_mode)
        {
            op_bridge = SYS_NH_OP_NONE;
        }

        dsnh.vlan_xlate_mode = 1;       /*humber mode*/
        dsnh.payload_operation = op_bridge;
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh.mtu_check_en = TRUE;
        dsnh.cw_index = p_dsnh_param->cw_index;
        dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

        if (p_dsnh_param->spme_mode)
        {
            dsnh.outer_edit_location = 0;
            dsnh.outer_edit_ptr_type = 1;
            dsnh.outer_edit_ptr = p_dsnh_param->l3edit_ptr; /*Pipe0: L3edit*/

            dsnh.inner_edit_ptr_type = 0; /* L3Edit(1)*/
            dsnh.inner_edit_ptr = p_dsnh_param->inner_l2edit_ptr;
        }
        else
        {
            if(p_dsnh_param->lspedit_ptr || (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP)))
            {
                dsnh.outer_edit_location = 0;
                dsnh.outer_edit_ptr_type = 1;
                dsnh.outer_edit_ptr = p_dsnh_param->lspedit_ptr; /*Pipe0: L3edit*/
            }
            else
            {
                dsnh.outer_edit_location = p_dsnh_param->loop_edit?0:1;
                dsnh.outer_edit_ptr_type = 0;
                dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr; /*Pipe0: L2edit*/
            }

            if (p_dsnh_param->loop_edit && p_dsnh_param->inner_l2edit_ptr)
            {
                /*do inner l2 edit*/
                dsnh.inner_edit_ptr_type = 0; /* L2Edit(1)*/
                dsnh.inner_edit_ptr = p_dsnh_param->inner_l2edit_ptr;
            }
            else
            {
                dsnh.inner_edit_ptr_type = 1; /* L3Edit(1)*/
                dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;
            }

        }

        if(!p_dsnh_param->spme_mode)
        {
            if (p_dsnh_param->p_vlan_info &&
                ((CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID)) ||
                 (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Should use DsNexthop8W table \n");
    			return CTC_E_INVALID_CONFIG;

            }
            else
            {
                /*Set default cos action*/
                if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
                {
                    dsnh.is_leaf = 1;
                }

                SYS_USW_NH_DSNH_SET_DEFAULT_COS_ACTION(dsnh);
                CTC_ERROR_RETURN(sys_usw_nh_dsnh_build_vlan_info(lchip, &dsnh, p_dsnh_param->p_vlan_info));
            }
        }

        break;

    case SYS_NH_PARAM_DSNH_TYPE_RSPAN:
        dsnh.vlan_xlate_mode = 1;    /*humber mode*/
        dsnh.tagged_mode = 1;
        dsnh.svlan_tagged = 1;
        dsnh.mirror_tag_add = 1;
        dsnh.derive_stag_cos = 1;    /*restore vlantag for mirror*/
        dsnh.replace_ctag_cos = 1;   /*add rspan vlan id*/
        dsnh.output_svlan_id_valid = 1;
        dsnh.output_svlan_id = p_dsnh_param->p_vlan_info->output_svid & 0xFFF;
        dsnh.dest_vlan_ptr = p_dsnh_param->dest_vlan_ptr;
        dsnh.payload_operation = SYS_NH_OP_MIRROR;
        dsnh.share_type = SYS_NH_SHARE_TYPE_VLANTAG;
        break;

    case SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL:

        dsnh.payload_operation = (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_NONE)) ?
            SYS_NH_OP_NONE  : ((CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ROUTE_NOTTL)) ?
            SYS_NH_OP_ROUTE_NOTTL : SYS_NH_OP_ROUTE);

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE))
        {
            dsnh.payload_operation = p_dsnh_param->hvpls?SYS_NH_OP_BRIDGE_INNER: SYS_NH_OP_BRIDGE_VPLS;
        }

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN))
        {
            dsnh.svlan_tagged = FALSE;
            dsnh.vlan_xlate_mode = 1;       /*humber mode*/
        }
        else
        {
           dsnh.svlan_tagged = TRUE;
        }
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);

        dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

         if (p_dsnh_param->inner_l2edit_ptr)
        { /*outer L2Edit*/
           dsnh.outer_edit_location = 0;
           dsnh.outer_edit_ptr_type = 1;
           dsnh.outer_edit_ptr = p_dsnh_param->l3edit_ptr;  /*Pipe0 L3edit*/

           dsnh.inner_edit_ptr_type = 0;
           dsnh.inner_edit_ptr = p_dsnh_param->l2edit_ptr;  /*inner L2edit*/

        }
        else
        {
            dsnh.outer_edit_location = 1;
            dsnh.outer_edit_ptr_type = 0;
            dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;  /*outer L2edit*/
            dsnh.inner_edit_ptr_type = 1;
            dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;  /*L3 edit*/
            if (p_dsnh_param->loop_edit)
            {
                dsnh.outer_edit_location = 0;
            }
        }

        dsnh.mtu_check_en = TRUE;
        break;

    case SYS_NH_PARAM_DSNH_TYPE_TRILL:

        dsnh.payload_operation = SYS_NH_OP_BRIDGE_VPLS;
        dsnh.svlan_tagged = TRUE;
        dsnh.mtu_check_en = TRUE;
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);

        dsnh.outer_edit_location = 1;
        dsnh.outer_edit_ptr_type = 0;
        dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;  /*outer L2edit*/
        dsnh.inner_edit_ptr_type = 1;
        dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;  /*L3 edit*/
        break;

    case SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL_FOR_MIRROR:
        dsnh.payload_operation = SYS_NH_OP_MIRROR;
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh, p_dsnh_param->flag);

        dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

        dsnh.outer_edit_location = 1;
        dsnh.outer_edit_ptr_type = 0;   /*Pipe2: L2edit*/
        dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;

        dsnh.inner_edit_ptr_type = 1;   /*L3edit*/
        dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;

        break;
case SYS_NH_PARAM_DSNH_TYPE_OF:
        dsnh.mtu_check_en = !(p_dsnh_param->mtu_no_chk);
        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_MISC_FLEX_PLD_BRIDGE))
        {
            dsnh.payload_operation = SYS_NH_OP_BRIDGE;
            dsnh.svlan_tagged = TRUE;
        }
        else
        {
            dsnh.payload_operation = SYS_NH_OP_NONE;
        }
        dsnh.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

        dsnh.outer_edit_location = 0;
        dsnh.outer_edit_ptr_type = 0; /*Pipe0: L2edit*/
        dsnh.outer_edit_ptr = p_dsnh_param->l2edit_ptr;

        dsnh.inner_edit_ptr_type = 1;/*L3edit*/
        dsnh.inner_edit_ptr = p_dsnh_param->l3edit_ptr;
        break;

    default:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] DsNexthop table type is invalid \n");
			return CTC_E_INVALID_CONFIG;

    }

    dsnh.logic_dest_port = p_dsnh_param->logic_port;
    dsnh.logic_port_check = p_dsnh_param->logic_port_check;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsNH4W :: DestVlanPtr = 0x%x, PldOP = %d ,\n \
                   copyCtagCos = %d, CvlanTagged = %d, DeriveStagCos = %d,\n \
                   MtuCheckEn = %d, OutputCvlanVlid = %d, OutputSvlanVlid = %d,\n \
                   ReplaceCtagCos = %d, StagCfi = %d, StagCos = %d,\n \
                   OutEditLoc = %d, OutEditPtrType = %d, OutEditPtr = %d,\n \
                   InnerEditPtrType = %d, InnerEditPtr = %d\n, \
                   SvlanTagged = %d, TaggedMode = %d, macDa = %s\n ",
                   dsnh.dest_vlan_ptr, dsnh.payload_operation,
                   dsnh.copy_ctag_cos, 0, dsnh.derive_stag_cos,
                   dsnh.mtu_check_en, dsnh.output_cvlan_id_valid, dsnh.output_svlan_id_valid,
                   dsnh.replace_ctag_cos,dsnh.stag_cfi, dsnh.stag_cos,
                   dsnh.outer_edit_location,dsnh.outer_edit_ptr_type, dsnh.outer_edit_ptr,
                   dsnh.inner_edit_ptr_type,dsnh.inner_edit_ptr,
                   dsnh.svlan_tagged, dsnh.tagged_mode, sys_output_mac(dsnh.mac_da));

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                       SYS_NH_ENTRY_TYPE_NEXTHOP_4W, p_dsnh_param->dsnh_offset, &dsnh));

    return CTC_E_NONE;
}

int32
sys_usw_nh_write_entry_dsnh8w(uint8 lchip, sys_nh_param_dsnh_t* p_dsnh_param)
{
    sys_nexthop_t dsnh_8w;
    uint32 op_bridge = 0;
    uint32 op_route_no_ttl = 0;
    sys_usw_nh_master_t* p_usw_nh_master = NULL;
    uint8 xgpon_en = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8  bpe_pe_mode = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_dsnh_param);
    sal_memset(&dsnh_8w, 0, sizeof(dsnh_8w));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "\n\
                   lchip = % d, \n\
                   dsnh_offset = % d, \n\
                   dsnh_type = % d\n, \
                   lspedit_ptr = % d, \n\
                   inner_l2edit_ptr = %d, \n\
                   l3EditPtr = % d \n",
                   lchip,
                   p_dsnh_param->dsnh_offset,
                   p_dsnh_param->dsnh_type,
                   p_dsnh_param->lspedit_ptr,
                   p_dsnh_param->inner_l2edit_ptr,
                   p_dsnh_param->l3edit_ptr);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_usw_nh_master));

    if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP))
    {
        dsnh_8w.aps_bridge_en = 1;
    }

    dsnh_8w.stag_cos |= 1;
    dsnh_8w.stats_ptr  = p_dsnh_param->stats_ptr;
    dsnh_8w.cid_valid = p_dsnh_param->cid?1:0;
    dsnh_8w.cid       = p_dsnh_param->cid;

    cmd = DRV_IOR(EpeNextHopReserved_t, EpeNextHopReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &value));
    bpe_pe_mode = (value&0x1)?1:0;
    if (bpe_pe_mode && p_dsnh_param->stats_ptr)
    {
        /*bpe ecid enable, cannot support nh stats*/
        return CTC_E_INVALID_CONFIG;
    }

    if (dsnh_8w.cid_valid &&  g_usw_nh_master[lchip]->cid_use_4w)
    {
        return CTC_E_INVALID_CONFIG;
    }

    switch (p_dsnh_param->dsnh_type)
    {
    case SYS_NH_PARAM_DSNH_TYPE_BRGUC:
        SYS_USW_NH_DSNH_SET_DEFAULT_COS_ACTION(dsnh_8w);
        dsnh_8w.payload_operation = SYS_NH_OP_BRIDGE;
        dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if (xgpon_en)
        {
            if (p_dsnh_param->p_vlan_info && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG)
                && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
            {
                dsnh_8w.vlan_xlate_mode = 0;
                CTC_UNSET_FLAG(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF);
            }
            else
            {
                dsnh_8w.vlan_xlate_mode = 1;       /*humber mode*/
            }
            if (p_dsnh_param->l3edit_ptr && DRV_IS_DUET2(lchip))
            {
                sys_usw_nh_master_t* p_nh_master = NULL;

                sys_usw_nh_get_nh_master(lchip, &p_nh_master);
                dsnh_8w.inner_edit_ptr_type = 1;
                dsnh_8w.inner_edit_ptr = p_nh_master->mpls_edit_offset + p_dsnh_param->l3edit_ptr -1;
            }
        }
        else
        {
            dsnh_8w.vlan_xlate_mode = 1;       /*humber mode*/
        }

        dsnh_8w.logic_dest_port     = p_dsnh_param->logic_port;
        dsnh_8w.logic_port_check    = p_dsnh_param->logic_port_check;
        dsnh_8w.outer_edit_ptr = p_dsnh_param->l2edit_ptr;

        if (p_dsnh_param->p_vlan_info && CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_STATS_EN))
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_dsnh_param->p_vlan_info->stats_id, &(dsnh_8w.stats_ptr)));
            if (bpe_pe_mode && dsnh_8w.stats_ptr)
            {
                /*bpe ecid enable, cannot support nh stats*/
                return CTC_E_INVALID_CONFIG;
            }
        }

        if(p_dsnh_param->logic_port_check)
        {
            dsnh_8w.logic_dest_port     = p_dsnh_param->logic_port;
            dsnh_8w.logic_port_check    = 1;
        }

        if (p_dsnh_param->p_vlan_info)
        {
            dsnh_8w.share_type = SYS_NH_SHARE_TYPE_VLANTAG;

            if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
            {
                dsnh_8w.is_leaf = 1;
            }
            if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_PASS_THROUGH))
            {
                dsnh_8w.payload_operation = SYS_NH_OP_BRIDGE_INNER;
            }

            CTC_ERROR_RETURN(sys_usw_nh_dsnh_build_vlan_info(lchip, (sys_nexthop_t*)(&dsnh_8w), p_dsnh_param->p_vlan_info));
            CTC_ERROR_RETURN(sys_usw_nh_dsnh8w_assign_vid(lchip, &dsnh_8w, p_dsnh_param->p_vlan_info));
        }

        break;

    case SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_NONE : /*Swap  label on LSR /Pop label and do   label  Swap on LER*/
        dsnh_8w.payload_operation = SYS_NH_OP_NONE;
        dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        SYS_USW_NH_DSNH_BUILD_FLAG(dsnh_8w, p_dsnh_param->flag);

        if (p_dsnh_param->lspedit_ptr || (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP)))
        {
            dsnh_8w.outer_edit_location = 0;
            dsnh_8w.outer_edit_ptr_type = 1;/*Pipe0: L3edit*/
            dsnh_8w.outer_edit_ptr = p_dsnh_param->lspedit_ptr;
        }
        else
        {
            dsnh_8w.outer_edit_location = p_dsnh_param->loop_edit?0:1;
            dsnh_8w.outer_edit_ptr_type = 0;/*Pipe0: L2edit*/
            dsnh_8w.outer_edit_ptr = p_dsnh_param->l2edit_ptr;
        }

        dsnh_8w.inner_edit_ptr_type = 1;/*L3Edit*/
        dsnh_8w.inner_edit_ptr = p_dsnh_param->l3edit_ptr;

        break;
    case SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_L2VPN:
        if (p_dsnh_param->hvpls)
        {
            op_bridge = SYS_NH_OP_BRIDGE_INNER;
        }
        else
        {
            op_bridge = SYS_NH_OP_BRIDGE_VPLS;
        }

        if (p_dsnh_param->spme_mode)
        {
            op_bridge = SYS_NH_OP_NONE;
        }

        if (p_dsnh_param->p_vlan_info)
        {
            if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
            {
                dsnh_8w.is_leaf = 1;
            }

            if (CTC_FLAG_ISSET(p_dsnh_param->p_vlan_info->flag, CTC_VLAN_NH_ETREE_LEAF))
            {
                dsnh_8w.is_leaf = 1;
            }


            dsnh_8w.vlan_xlate_mode = 1;       /*humber mode*/
            dsnh_8w.payload_operation = op_bridge;
            dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
            dsnh_8w.mtu_check_en = TRUE;

            dsnh_8w.share_type = SYS_NH_SHARE_TYPE_VLANTAG;

            if (p_dsnh_param->spme_mode)
            {
                dsnh_8w.outer_edit_location = 0;
                dsnh_8w.outer_edit_ptr_type = 1;
                dsnh_8w.outer_edit_ptr = p_dsnh_param->l3edit_ptr;

                dsnh_8w.inner_edit_ptr_type = 0;
                dsnh_8w.inner_edit_ptr = p_dsnh_param->inner_l2edit_ptr;
            }
            else
            {
                if(p_dsnh_param->lspedit_ptr || (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP)))
                {
                    dsnh_8w.outer_edit_location = 0;
                    dsnh_8w.outer_edit_ptr_type = 1;
                    dsnh_8w.outer_edit_ptr = p_dsnh_param->lspedit_ptr; /*Pipe0: L3edit*/
                }
                else
                {
                    dsnh_8w.outer_edit_location = p_dsnh_param->loop_edit?0:1;
                    dsnh_8w.outer_edit_ptr_type = 0;
                    dsnh_8w.outer_edit_ptr = p_dsnh_param->l2edit_ptr; /*Pipe0: L2edit*/
                }

                if (p_dsnh_param->loop_edit && p_dsnh_param->inner_l2edit_ptr)
                {
                    /*do inner l2 edit*/
                    dsnh_8w.inner_edit_ptr_type = 0; /* L2Edit(1)*/
                    dsnh_8w.inner_edit_ptr = p_dsnh_param->inner_l2edit_ptr;
                }else
                {
                    dsnh_8w.inner_edit_ptr_type = 1;
                    dsnh_8w.inner_edit_ptr = p_dsnh_param->l3edit_ptr;
                }

                CTC_ERROR_RETURN(sys_usw_nh_dsnh_build_vlan_info(lchip, (&dsnh_8w) /*Have same structure*/,
                                                                       p_dsnh_param->p_vlan_info));
                CTC_ERROR_RETURN(sys_usw_nh_dsnh8w_assign_vid(lchip, &dsnh_8w, p_dsnh_param->p_vlan_info));
            }

            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dsnh8w.dest_vlanptr = %d, payloadOp = %d \n",
                           dsnh_8w.dest_vlan_ptr, dsnh_8w.payload_operation);

        }
        else
        {
            CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
        }

        dsnh_8w.mtu_check_en = TRUE;
        dsnh_8w.cw_index = p_dsnh_param->cw_index;
        dsnh_8w.logic_dest_port     = p_dsnh_param->logic_port;

        break;

    case SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL:
        op_bridge = (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE)) ? 1: 0;
        op_route_no_ttl = (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ROUTE_NOTTL)) ? 1:0;

        if (op_bridge)
        {
            dsnh_8w.payload_operation = p_dsnh_param->hvpls?SYS_NH_OP_BRIDGE_INNER: SYS_NH_OP_BRIDGE_VPLS;
        }
        else
        {
            if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_NONE))
            {
                dsnh_8w.payload_operation = SYS_NH_OP_NONE;
            }
            else
            {
                dsnh_8w.payload_operation = op_route_no_ttl? SYS_NH_OP_ROUTE_NOTTL : SYS_NH_OP_ROUTE;
            }
        }

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN))
        {
            dsnh_8w.svlan_tagged = FALSE;
            dsnh_8w.vlan_xlate_mode = 1;       /*humber mode*/
        }
        else
        {
           dsnh_8w.svlan_tagged = TRUE;
        }
        dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh_8w.mtu_check_en = TRUE;

        dsnh_8w.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

       if (p_dsnh_param->inner_l2edit_ptr)
        { /*outer L2Edit*/
           dsnh_8w.outer_edit_location = 0;
           dsnh_8w.outer_edit_ptr_type = 1;
           dsnh_8w.outer_edit_ptr = p_dsnh_param->l3edit_ptr;  /*Pipe0 L3edit*/

           dsnh_8w.inner_edit_ptr_type = 0;
           dsnh_8w.inner_edit_ptr = p_dsnh_param->inner_l2edit_ptr;  /*inner L2edit*/

        }
        else
        {
            dsnh_8w.outer_edit_location = 1;
            dsnh_8w.outer_edit_ptr_type = 0;
            dsnh_8w.outer_edit_ptr = p_dsnh_param->l2edit_ptr;  /*outer L2edit*/

            dsnh_8w.inner_edit_ptr_type = 1;
            dsnh_8w.inner_edit_ptr = p_dsnh_param->l3edit_ptr;  /*L3 edit*/

        }

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ERSPAN_EN))
        {
            dsnh_8w.fid_valid          = 0;
            dsnh_8w.fid                = p_dsnh_param->span_id;
            dsnh_8w.span_en            = 1;
            dsnh_8w.share_type = SYS_NH_SHARE_TYPE_VLANTAG;  /*here using SYS_NH_SHARE_TYPE_VLANTAG just for stats ptr in dsnexthop8w*/
            dsnh_8w.payload_operation = SYS_NH_OP_MIRROR;

			if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ERSPAN_KEEP_IGS_TS))
            {
                dsnh_8w.keep_igs_ts = 1;
            }

        }
        else
        {
            dsnh_8w.fid_valid          = (0 == p_dsnh_param->fid)? 0 : 1;
            dsnh_8w.fid                = p_dsnh_param->fid;
        }

        if (p_dsnh_param->p_vlan_info)  /*misc over l2 edit with inner vlan edit*/
        {
            dsnh_8w.payload_operation = SYS_NH_OP_BRIDGE_INNER;
            dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
            dsnh_8w.vlan_xlate_mode = 1;       /*humber mode*/
            dsnh_8w.share_type = SYS_NH_SHARE_TYPE_VLANTAG;

            SYS_USW_NH_DSNH_SET_DEFAULT_COS_ACTION(dsnh_8w);
            CTC_ERROR_RETURN(sys_usw_nh_dsnh_build_vlan_info(lchip, (sys_nexthop_t*)(&dsnh_8w), p_dsnh_param->p_vlan_info));
            CTC_ERROR_RETURN(sys_usw_nh_dsnh8w_assign_vid(lchip, &dsnh_8w, p_dsnh_param->p_vlan_info));
        }

        dsnh_8w.is_vxlan_route_op  = (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_INNER)) ?
            0 : 1;
        dsnh_8w.logic_port_check   = p_dsnh_param->logic_port_check;
        dsnh_8w.logic_dest_port    = p_dsnh_param->logic_port;

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_UDF_EDIT_EN))
        {
            dsnh_8w.is_vxlan_route_op  = 1;
            dsnh_8w.is_wlan_encap  = 1;
            sal_memcpy(&dsnh_8w.mac_da[0], &p_dsnh_param->macDa[0], sizeof(mac_addr_t));
            dsnh_8w.rid = p_dsnh_param->rid;
            dsnh_8w.wlan_mc_en = p_dsnh_param->wlan_mc_en;
        }

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dsnh8w.dest_vlanptr = %d, payloadOp = %d \n",
                       dsnh_8w.dest_vlan_ptr, dsnh_8w.payload_operation);

        break;


    case SYS_NH_PARAM_DSNH_TYPE_XERSPAN:
        dsnh_8w.payload_operation = SYS_NH_OP_MIRROR;
        dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh_8w.mtu_check_en = TRUE;
        dsnh_8w.xerpsan_en = TRUE;
        dsnh_8w.share_type = SYS_NH_SHARE_TYPE_SPAN_PACKET;
        dsnh_8w.outer_edit_location = 0;
        dsnh_8w.outer_edit_ptr_type = 0;
        dsnh_8w.outer_edit_ptr = p_dsnh_param->l2edit_ptr;

        dsnh_8w.inner_edit_ptr_type = 1;
        dsnh_8w.inner_edit_ptr = p_dsnh_param->l3edit_ptr;
        dsnh_8w.hash_num = p_dsnh_param->hash_num;

        dsnh_8w.fid_valid          = 0;
        dsnh_8w.fid                = p_dsnh_param->span_id;
        dsnh_8w.span_en            = 1;

        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ERSPAN_KEEP_IGS_TS))
        {
            dsnh_8w.keep_igs_ts = 1;
        }


        break;

    case SYS_NH_PARAM_DSNH_TYPE_WLANTUNNEL:
        op_bridge = (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE)) ? 1: 0;
        op_route_no_ttl = (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_ROUTE_NOTTL)) ? 1:0;
        dsnh_8w.payload_operation = op_bridge ? SYS_NH_OP_NONE : SYS_NH_OP_ROUTE;
        dsnh_8w.svlan_tagged = p_dsnh_param->stag_en;
        if (op_bridge)
        {
            dsnh_8w.payload_operation = SYS_NH_OP_BRIDGE;
        }
        else
        {
            dsnh_8w.payload_operation = op_route_no_ttl? SYS_NH_OP_ROUTE_NOTTL : SYS_NH_OP_ROUTE;
        }

        dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh_8w.mtu_check_en = TRUE;

        dsnh_8w.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

       if (p_dsnh_param->inner_l2edit_ptr)
        { /*outer L2Edit*/
           dsnh_8w.outer_edit_location = 0;
           dsnh_8w.outer_edit_ptr_type = 1;
           dsnh_8w.outer_edit_ptr = p_dsnh_param->l3edit_ptr;  /*Pipe0 L3edit*/

           dsnh_8w.inner_edit_ptr_type = 0;
           dsnh_8w.inner_edit_ptr = p_dsnh_param->inner_l2edit_ptr;  /*inner L2edit*/

        }
        else
        {
            dsnh_8w.inner_edit_ptr_type = 1;
            dsnh_8w.inner_edit_ptr = p_dsnh_param->l3edit_ptr;  /*L3 edit*/

        }

        dsnh_8w.logic_port_check   = p_dsnh_param->logic_port_check;
        dsnh_8w.logic_dest_port    = p_dsnh_param->logic_port;
        dsnh_8w.is_wlan_encap = 1;
        dsnh_8w.wlan_tunnel_type = p_dsnh_param->wlan_tunnel_type;
        dsnh_8w.is_dot11 = p_dsnh_param->is_dot11;
        dsnh_8w.wlan_mc_en = p_dsnh_param->wlan_mc_en;
        dsnh_8w.rid = p_dsnh_param->rid;
        sal_memcpy(dsnh_8w.mac_da, p_dsnh_param->macDa, sizeof(mac_addr_t));

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dsnh8w.dest_vlanptr = %d, payloadOp = %d \n",
                       dsnh_8w.dest_vlan_ptr, dsnh_8w.payload_operation);

        break;

case SYS_NH_PARAM_DSNH_TYPE_OF:
        dsnh_8w.mtu_check_en = !(p_dsnh_param->mtu_no_chk);
        if (CTC_FLAG_ISSET(p_dsnh_param->flag, SYS_NH_PARAM_FLAG_MISC_FLEX_PLD_BRIDGE))
        {
            dsnh_8w.payload_operation = SYS_NH_OP_BRIDGE;
            dsnh_8w.svlan_tagged = 1;
        }
        else
        {
            dsnh_8w.payload_operation = SYS_NH_OP_NONE;
        }

        dsnh_8w.dest_vlan_ptr  = p_dsnh_param->dest_vlan_ptr;
        dsnh_8w.share_type = SYS_NH_SHARE_TYPE_L23EDIT;

        dsnh_8w.outer_edit_location = 0;
        dsnh_8w.outer_edit_ptr_type = 0; /*Pipe0: L2edit*/
        dsnh_8w.outer_edit_ptr = p_dsnh_param->l2edit_ptr;

        dsnh_8w.inner_edit_ptr_type = 1;/*L3edit*/
        dsnh_8w.inner_edit_ptr = p_dsnh_param->l3edit_ptr;

        if (p_dsnh_param->p_vlan_info)  /* vlan edit from nexthop not from L2edit */
        {
            dsnh_8w.vlan_xlate_mode = 1;       /*humber mode*/
            CTC_ERROR_RETURN(sys_usw_nh_dsnh_build_vlan_info(lchip, (sys_nexthop_t*)(&dsnh_8w), p_dsnh_param->p_vlan_info));
            CTC_ERROR_RETURN(sys_usw_nh_dsnh8w_assign_vid(lchip, &dsnh_8w, p_dsnh_param->p_vlan_info));
        }
        break;

    default:
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Should use DsNexthop4W table \n");
			return CTC_E_INVALID_CONFIG;

    }

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsNH4W :: DestVlanPtr = 0x%x, PldOP = %d ,\n \
                   copyCtagCos = %d, CvlanTagged = %d, DeriveStagCos = %d,\n \
                   MtuCheckEn = %d, OutputCvlanVlid = %d, OutputSvlanVlid = %d,\n \
                   ReplaceCtagCos = %d, StagCfi = %d, StagCos = %d,\n \
                   OutEditLoc = %d, OutEditPtrType = %d, OutEditPtr = %d,\n \
                   InnerEditPtrType = %d, InnerEditPtr = %d\n, \
                   SvlanTagged = %d, TaggedMode = %d, macDa = %s\n ",
                   dsnh_8w.dest_vlan_ptr, dsnh_8w.payload_operation,
                   dsnh_8w.copy_ctag_cos, 0, dsnh_8w.derive_stag_cos,
                   dsnh_8w.mtu_check_en, dsnh_8w.output_cvlan_id_valid, dsnh_8w.output_svlan_id_valid,
                   dsnh_8w.replace_ctag_cos,dsnh_8w.stag_cfi, dsnh_8w.stag_cos,
                   dsnh_8w.outer_edit_location,dsnh_8w.outer_edit_ptr_type, dsnh_8w.outer_edit_ptr,
                   dsnh_8w.inner_edit_ptr_type,dsnh_8w.inner_edit_ptr,
                   dsnh_8w.svlan_tagged, dsnh_8w.tagged_mode, sys_output_mac(dsnh_8w.mac_da));

    dsnh_8w.is_nexthop8_w = TRUE;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,
                                                       SYS_NH_ENTRY_TYPE_NEXTHOP_8W, p_dsnh_param->dsnh_offset, &dsnh_8w));

    return CTC_E_NONE;
}

/**
 @brief This function is used to get dsfwd info
 */
int32
sys_usw_nh_get_entry_dsfwd(uint8 lchip, uint32 dsfwd_offset, void* p_dsfwd)
{
    DsFwd_m dsfwd;
    DsFwdHalf_m dsfwd_half;

    sys_fwd_t* p_tmp_fwd = 0;
    uint8 is_half = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_dsfwd);
    p_tmp_fwd = (sys_fwd_t*)p_dsfwd;

    sal_memset(&dsfwd, 0, sizeof(dsfwd));
    sal_memset(&dsfwd_half, 0, sizeof(dsfwd_half));


    CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD,
                                                      dsfwd_offset, &dsfwd));

    is_half = GetDsFwd(V, isHalf_f, &dsfwd);

    if (0 == is_half)
    {
        p_tmp_fwd->dest_map = GetDsFwd(V, destMap_f, &dsfwd);
    }
    else
    {
        if (dsfwd_offset % 2 == 0)
        {
            GetDsFwdDualHalf(A, g_0_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
        }
        else
        {
            GetDsFwdDualHalf(A, g_1_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
        }

        p_tmp_fwd->dest_map = GetDsFwdHalf(V, destMap_f, &dsfwd_half);
    }

    return CTC_E_NONE;

}

/**
 @brief This function is used to build and write dsfwd table
 */
int32
sys_usw_nh_write_entry_dsfwd(uint8 lchip, sys_nh_param_dsfwd_t* p_dsfwd_param)
{

    sys_fwd_t sys_fwd;
    uint8 gchip = 0;
    uint8 sub_queue_id = 0;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_dsfwd_param);

    sal_memset(&sys_fwd, 0, sizeof(sys_fwd));


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "\n\
        lchip = %d, \n\
        dsfwd_offset = 0x%x, \n\
        dest_chip_id = %d, \n\
        dest_id = 0x%x, \n\
        dropPkt = %d, \n\
        dsnh_offset = 0x%x, \n\
        is_mcast = %d\n",
                   lchip,
                   p_dsfwd_param->dsfwd_offset,
                   p_dsfwd_param->dest_chipid,
                   p_dsfwd_param->dest_id,
                   p_dsfwd_param->drop_pkt,
                   p_dsfwd_param->dsnh_offset,
                   p_dsfwd_param->is_mcast);

    if (p_dsfwd_param->dsnh_offset > 0x3FFFF)
    {
        return CTC_E_INVALID_PARAM;
    }

    sys_fwd.aps_bridge_en = (p_dsfwd_param->aps_type == CTC_APS_BRIDGE);
    sys_fwd.force_back_en = p_dsfwd_param->is_reflective;
    sys_fwd.bypass_igs_edit = (p_dsfwd_param->is_egress_edit)?TRUE:FALSE;
    sys_fwd.nexthop_ptr = (p_dsfwd_param->dsnh_offset & 0x3FFFF);
    sys_fwd.nexthop_ext = (p_dsfwd_param->nexthop_ext)?TRUE:FALSE;
    sys_fwd.adjust_len_idx = p_dsfwd_param->adjust_len_idx;
    sys_fwd.cut_through_dis = p_dsfwd_param->cut_through_dis;

    if (p_dsfwd_param->drop_pkt)
    {
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        sys_fwd.nexthop_ptr = 0x3FFFF;
        sys_fwd.dest_map = SYS_ENCODE_DESTMAP(p_dsfwd_param->dest_chipid, SYS_RSV_PORT_DROP_ID);
    }

    else
    {
        if (p_dsfwd_param->is_lcpu)
        {
            if(CTC_PKT_CPU_REASON_GET_BY_NHPTR(p_dsfwd_param->dsnh_offset) == CTC_PKT_CPU_REASON_ARP_MISS)
            {
                CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_ARP_MISS, &sub_queue_id));
            }
            else
            {
                ret = sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_FWD_CPU, &sub_queue_id);
                sub_queue_id = (ret==CTC_E_NONE) ? sub_queue_id : MCHIP_CAP(SYS_CAP_PKT_CPU_QDEST_BY_DMA);
            }
            /*default to CPU by CPU,modify destination by QOS API*/
            sys_fwd.dest_map = SYS_ENCODE_EXCP_DESTMAP(p_dsfwd_param->dest_chipid, sub_queue_id);
        }
        else if (p_dsfwd_param->is_mcast)
        {
            if (p_dsfwd_param->use_destmap_profile)
            {
                sys_fwd.dest_map = SYS_ENCODE_ARP_DESTMAP(p_dsfwd_param->dest_id);
            }
            else
            {
                sys_fwd.dest_map = SYS_ENCODE_MCAST_IPE_DESTMAP(p_dsfwd_param->dest_id);
            }

            if (p_dsfwd_param->stats_ptr)
            {
                sys_fwd.nexthop_ptr = p_dsfwd_param->stats_ptr ;
            }
        }
        else
        {
            if (p_dsfwd_param->use_destmap_profile)
            {
                sys_fwd.dest_map = SYS_ENCODE_ARP_DESTMAP(p_dsfwd_param->dest_id);
            }
            else if (p_dsfwd_param->aps_type == CTC_APS_DISABLE && (p_dsfwd_param->dest_chipid != 0x1F)
                && p_dsfwd_param->is_cpu)
            {
                CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_FWD_CPU, &sub_queue_id));
                sys_fwd.dest_map = SYS_ENCODE_EXCP_DESTMAP(p_dsfwd_param->dest_chipid,sub_queue_id);
                sys_fwd.nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_FWD_CPU, 0);
            }
            else
            {
                if (p_dsfwd_param->aps_type == CTC_APS_BRIDGE)
                {
                    sys_fwd.dest_map = SYS_ENCODE_APS_DESTMAP(p_dsfwd_param->dest_chipid, p_dsfwd_param->dest_id);
                }
                else
                {
                    sys_fwd.dest_map = SYS_ENCODE_DESTMAP(p_dsfwd_param->dest_chipid, p_dsfwd_param->dest_id);
                }
            }
        }
    }

    /* when using nexthop ext, must using full dsfwd*/
    if (sys_fwd.force_back_en || p_dsfwd_param->nexthop_ext || p_dsfwd_param->truncate_profile_id || p_dsfwd_param->is_6w)
    {
        sys_fwd.is_6w = 1;
    }

    sys_fwd.offset = p_dsfwd_param->dsfwd_offset;
    sys_fwd.truncate_profile = p_dsfwd_param->truncate_profile_id;
    sys_fwd.cloud_sec_en = p_dsfwd_param->cloud_sec_en;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, p_dsfwd_param->dsfwd_offset, &sys_fwd));


    return CTC_E_NONE;
}

int32
sys_usw_nh_update_entry_dsfwd(uint8 lchip, uint32 *offset,
                               uint32 dest_map,
                               uint32 dsnh_offset,
                               uint8 dsnh_8w,
                               uint8 del,
                               uint8 critical_packet)
{

    sys_fwd_t sys_fwd;
    sal_memset(&sys_fwd, 0, sizeof(sys_fwd));


    if ((*offset) == 0)
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, offset));

    }
    else if(del)
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, *offset));

    }
    sys_fwd.nexthop_ptr = (dsnh_offset & 0x3FFFF);
    sys_fwd.nexthop_ext = dsnh_8w;
    sys_fwd.dest_map  = dest_map;
    sys_fwd.offset = *offset;
    sys_fwd.bypass_igs_edit = 1;
    sys_fwd.critical_packet = critical_packet;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, sys_fwd.offset, &sys_fwd));
    return CTC_E_NONE;

}



#define __________init___________
/**
 @brief This function is used to init resolved dsnexthop for bridge

 */
STATIC int32
_sys_usw_nh_dsnh_init_for_brg(uint8 lchip, sys_nh_offset_attr_t* p_offset_attr)
{
    uint32 nhp_ptr = 0;
    sys_nexthop_t dsnh;
    uint8 offset = 0;

    CTC_PTR_VALID_CHECK(p_offset_attr);
    sal_memset(&dsnh, 0, sizeof(dsnh));

    offset = SYS_DSNH4WREG_INDEX_FOR_BRG;
    nhp_ptr = SYS_NH_BUILD_INT_OFFSET(offset);

    p_offset_attr->offset_base = nhp_ptr;
    p_offset_attr->entry_size = 1;
    p_offset_attr->entry_num = 1;

    SYS_USW_NH_DSNH_SET_DEFAULT_COS_ACTION(dsnh);

    dsnh.svlan_tagged = TRUE;
    dsnh.payload_operation = SYS_NH_OP_BRIDGE;
    dsnh.dest_vlan_ptr = SYS_DSNH_DESTVLANPTR_SPECIAL;
    dsnh.offset = nhp_ptr;
    dsnh.share_type = SYS_NH_SHARE_TYPE_VLANTAG;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_INTERNAL,
                                                       offset, &dsnh));

    return CTC_E_NONE;

}

/**
 @brief This function is used to init resolved dsnexthop for mirror
 */
STATIC int32
_sys_usw_nh_dsnh_init_for_mirror(uint8 lchip, sys_nh_offset_attr_t* p_offset_attr)
{
    uint32 nhp_ptr = 0;
    sys_nexthop_t dsnh;
    uint8 offset = 0;

    CTC_PTR_VALID_CHECK(p_offset_attr);
    sal_memset(&dsnh, 0, sizeof(dsnh));

    offset = SYS_DSNH4WREG_INDEX_FOR_MIRROR;
    nhp_ptr = SYS_NH_BUILD_INT_OFFSET(offset);

    p_offset_attr->offset_base = nhp_ptr;
    p_offset_attr->entry_size = 1;
    p_offset_attr->entry_num = 1;

    dsnh.vlan_xlate_mode = TRUE;
    dsnh.payload_operation = SYS_NH_OP_MIRROR;
    dsnh.tagged_mode = TRUE;
    dsnh.share_type = SYS_NH_SHARE_TYPE_VLANTAG;
    dsnh.offset = nhp_ptr;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_INTERNAL,
                                                       offset, &dsnh));

    return CTC_E_NONE;

}


/**
 @brief This function is used to init resolved dsnexthop for egress bypass
 */
STATIC int32
_sys_usw_nh_dsnh_init_for_bypass(uint8 lchip, sys_nh_offset_attr_t* p_offset_attr)
{
    uint32 nhp_ptr = 0;
    sys_nexthop_t dsnh;
    uint8 offset = 0;

    CTC_PTR_VALID_CHECK(p_offset_attr);
    sal_memset(&dsnh, 0, sizeof(dsnh));

    offset = SYS_DSNH4WREG_INDEX_FOR_BYPASS;
    nhp_ptr = SYS_NH_BUILD_INT_OFFSET(offset);

    p_offset_attr->offset_base = nhp_ptr;
    p_offset_attr->entry_size = 1;
    p_offset_attr->entry_num = 1;

    dsnh.next_hop_bypass = TRUE;
    dsnh.offset = nhp_ptr;
    dsnh.is_nexthop8_w = 1;
    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_INTERNAL,
                                                       offset, &dsnh));

    return CTC_E_NONE;

}

/**
 @brief This function is used to init resolved dsnexthop for egress bpe ecid transparent
 */
int32
_sys_usw_nh_dsnh_init_for_bpe_transparent(uint8 lchip, sys_nh_offset_attr_t* p_offset_attr)
{
    uint32 nhp_ptr = 0;
    sys_nexthop_t dsnh;
    uint8 offset = 0;

    CTC_PTR_VALID_CHECK(p_offset_attr);
    sal_memset(&dsnh, 0, sizeof(dsnh));

    offset = SYS_DSNH4WREG_INDEX_FOR_BPE_TRANSPARENT;
    nhp_ptr = SYS_NH_BUILD_INT_OFFSET(offset);

    p_offset_attr->offset_base = nhp_ptr;
    p_offset_attr->entry_size = 1;
    p_offset_attr->entry_num = 1;

    dsnh.next_hop_bypass = TRUE;
    dsnh.offset = nhp_ptr;
    dsnh.is_nexthop8_w = 1;
    dsnh.ecid_valid = 1;
    dsnh.ecid = 0;
    dsnh.share_type =  SYS_NH_SHARE_TYPE_VLANTAG;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_INTERNAL,
                                                       offset, &dsnh));

    return CTC_E_NONE;

}


/**
 This function is used to init resolved dsFwd for loopback to itself
 */
STATIC int32
_sys_usw_nh_dsfwd_init_for_reflective(uint8 lchip, uint16* p_offset_attr)
{
    uint32 dsfwd_ptr = 0;
    uint32 nhp_ptr = 0;
    uint8 offset = 0;
    sys_fwd_t dsfwd;

    offset = SYS_DSNH4WREG_INDEX_FOR_BYPASS;
    nhp_ptr = SYS_NH_BUILD_INT_OFFSET(offset);
    sal_memset(&dsfwd, 0, sizeof(dsfwd));
    dsfwd.force_back_en = 1;
    dsfwd.nexthop_ptr = nhp_ptr;
    dsfwd.nexthop_ext = 1;
    dsfwd.is_6w = 1;
    CTC_ERROR_RETURN(
    sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 2, &dsfwd_ptr));

    CTC_ERROR_RETURN(
    sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, dsfwd_ptr, &dsfwd));

    *p_offset_attr = dsfwd_ptr;

    return CTC_E_NONE;

}


/**
 This function is used to init resolved dsFwd 0 for drop
*/
STATIC int32
_sys_usw_nh_dsfwd_init_for_drop(uint8 lchip)
{
    sys_nh_param_dsfwd_t dsfwd_param;

    sal_memset(&dsfwd_param, 0, sizeof(dsfwd_param));
    dsfwd_param.drop_pkt = 1;
    dsfwd_param.dsfwd_offset = 0;
    dsfwd_param.dest_id = 0xFFFF;
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));

    return CTC_E_NONE;
}

/**
 @brief This function is used to init resolved l2edit6w  for ecmp interface
 */
STATIC int32
_sys_usw_nh_l2edit_init_for_ecmp_if(uint8 lchip, uint16* p_offset_attr)
{
    uint32 nhp_ptr = 0;
    sys_dsl2edit_t l2edit;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (work_status != CTC_FTM_MEM_CHANGE_RECOVER))
    {
        return CTC_E_NONE;
    }
    sal_memset(&l2edit, 0, sizeof(l2edit));
    l2edit.ds_type  = SYS_NH_DS_TYPE_L2EDIT;
    l2edit.l2_rewrite_type  = SYS_NH_L2EDIT_TYPE_ETH_8W;
    l2edit.is_6w = 1;
    l2edit.use_port_mac = 1;

    CTC_ERROR_RETURN(
    sys_usw_nh_offset_alloc_with_multiple(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 2,
                                   2, &nhp_ptr));
    l2edit.offset = nhp_ptr;
    CTC_ERROR_RETURN(
       sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, nhp_ptr, &l2edit));


    *p_offset_attr = nhp_ptr;

    return CTC_E_NONE;

}



/**
 @brief This function is used to initilize dynamic offset, inclue offset pool,
        and resolved offset
*/
STATIC int32
_sys_usw_nh_offset_init(uint8 lchip, sys_usw_nh_master_t* p_master)
{
    sys_usw_opf_t opf;
    uint16 tbl_id = 0;
    uint32 tbl_entry_num[4] = {0};
    uint32 glb_tbl_entry_num[4] = {0};
    uint8  dyn_tbl_idx = 0;
    uint8  max_dyn_tbl_idx = 0;
    uint32  entry_enum = 0, nh_glb_entry_num = 0;
    uint32 nexthop_entry_num = 0;
    uint8  dyn_tbl_opf_type = 0;
    uint8  loop = 0;
    uint8  opf_type = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    tbl_id = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_FWD].table_id;
    sys_usw_ftm_get_dynamic_table_info(lchip, tbl_id,  &dyn_tbl_idx, &entry_enum, &nh_glb_entry_num);
    dyn_tbl_opf_type  = OPF_NH_DYN1_SRAM + dyn_tbl_idx;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_FWD].opf_pool_type = dyn_tbl_opf_type;
    tbl_entry_num[dyn_tbl_idx]  = entry_enum;
    max_dyn_tbl_idx = dyn_tbl_idx;

    tbl_id = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_MET].table_id;
    sys_usw_ftm_get_dynamic_table_info(lchip, tbl_id,  &dyn_tbl_idx, &entry_enum, &nh_glb_entry_num);
    /*global entry number from FTM module is mcast group number*/
    nh_glb_entry_num *= (p_master->met_mode ? 2 : 1);
    dyn_tbl_opf_type  = OPF_NH_DYN1_SRAM + dyn_tbl_idx;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_MET].opf_pool_type = dyn_tbl_opf_type;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_MET_6W].opf_pool_type = dyn_tbl_opf_type;
    tbl_entry_num[dyn_tbl_idx]  = entry_enum;
    glb_tbl_entry_num[dyn_tbl_idx]  = nh_glb_entry_num;
    max_dyn_tbl_idx = (dyn_tbl_idx > max_dyn_tbl_idx) ? dyn_tbl_idx : max_dyn_tbl_idx;


    tbl_id = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_NEXTHOP_4W].table_id;
    sys_usw_ftm_get_dynamic_table_info(lchip, tbl_id ,  &dyn_tbl_idx, &entry_enum, &nh_glb_entry_num);
    dyn_tbl_opf_type  = OPF_NH_DYN1_SRAM + dyn_tbl_idx;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_NEXTHOP_4W].opf_pool_type = dyn_tbl_opf_type;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_NEXTHOP_8W].opf_pool_type = dyn_tbl_opf_type;
    tbl_entry_num[dyn_tbl_idx]  = entry_enum;
    nexthop_entry_num = entry_enum;
    if (p_master->pkt_nh_edit_mode != SYS_NH_IGS_CHIP_EDIT_MODE)
    {
        glb_tbl_entry_num[dyn_tbl_idx]  = nh_glb_entry_num;
    }
    max_dyn_tbl_idx = (dyn_tbl_idx > max_dyn_tbl_idx) ? dyn_tbl_idx : max_dyn_tbl_idx;

    tbl_id = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W].table_id;
    sys_usw_ftm_get_dynamic_table_info(lchip, tbl_id,  &dyn_tbl_idx, &entry_enum, &nh_glb_entry_num);
    dyn_tbl_opf_type  = OPF_NH_DYN1_SRAM + dyn_tbl_idx;
    tbl_entry_num[dyn_tbl_idx]  = entry_enum;
    max_dyn_tbl_idx = (dyn_tbl_idx > max_dyn_tbl_idx) ? dyn_tbl_idx : max_dyn_tbl_idx;

    for (loop = SYS_NH_ENTRY_TYPE_L2EDIT_FROM; loop <= SYS_NH_ENTRY_TYPE_L3EDIT_TO; loop++)
    {
        p_master->nh_table_info_array[loop].opf_pool_type = dyn_tbl_opf_type;
    }

    /*Init  global dsnexthop & dsMet offset*/
    dyn_tbl_idx = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_NEXTHOP_4W].opf_pool_type - OPF_NH_DYN1_SRAM;
    if (p_master->pkt_nh_edit_mode == SYS_NH_IGS_CHIP_EDIT_MODE || (glb_tbl_entry_num[dyn_tbl_idx ] == 0))
    {
        p_master->max_glb_nh_offset  = 0;
    }
    else
    {
        p_master->max_glb_nh_offset  = glb_tbl_entry_num[dyn_tbl_idx ];
        p_master->p_occupid_nh_offset_bmp = (uint32*)mem_malloc(MEM_NEXTHOP_MODULE,
                                                                (sizeof(uint32) * (p_master->max_glb_nh_offset / BITS_NUM_OF_WORD + 1)));
        if (NULL == p_master->p_occupid_nh_offset_bmp)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_master->p_occupid_nh_offset_bmp, 0,  (sizeof(uint32) * (p_master->max_glb_nh_offset / BITS_NUM_OF_WORD + 1)));

    }

    dyn_tbl_idx = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_MET].opf_pool_type - OPF_NH_DYN1_SRAM;
    p_master->max_glb_met_offset  = glb_tbl_entry_num[dyn_tbl_idx ];
    if (p_master->max_glb_met_offset)
    {
        p_master->p_occupid_met_offset_bmp = (uint32*)mem_malloc(MEM_NEXTHOP_MODULE, (sizeof(uint32) * (p_master->max_glb_met_offset / BITS_NUM_OF_WORD + 1)));
        if (NULL == p_master->p_occupid_met_offset_bmp)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_master->p_occupid_met_offset_bmp, 0,  (sizeof(uint32) * (p_master->max_glb_met_offset / BITS_NUM_OF_WORD + 1)));
    }

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    for (loop = 0; loop <= max_dyn_tbl_idx; loop++)
    {
        dyn_tbl_idx = loop;


        /*all nexthop opf min offset should be greater than 2*/

        if (0 == glb_tbl_entry_num[dyn_tbl_idx])
        {
            glb_tbl_entry_num[dyn_tbl_idx]++;
            glb_tbl_entry_num[dyn_tbl_idx]++;
        }

        if (tbl_entry_num[dyn_tbl_idx] < glb_tbl_entry_num[dyn_tbl_idx])
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory, Global tbl entry exceed max table num \n");
            return CTC_E_NO_MEMORY;
        }

        tbl_entry_num[dyn_tbl_idx] = tbl_entry_num[dyn_tbl_idx] - glb_tbl_entry_num[dyn_tbl_idx];

        opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
        opf.pool_index = OPF_NH_DYN1_SRAM + loop;

        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, glb_tbl_entry_num[dyn_tbl_idx], tbl_entry_num[dyn_tbl_idx]));


    }

    tbl_id = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W].table_id;
    sys_usw_ftm_query_table_entry_num(lchip, tbl_id, &entry_enum);
    dyn_tbl_opf_type = OPF_NH_OUTER_L2EDIT;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W].opf_pool_type = dyn_tbl_opf_type;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W].opf_pool_type = dyn_tbl_opf_type;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = dyn_tbl_opf_type;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, entry_enum*2-1));


    tbl_id = p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L3EDIT_SPME].table_id;
    sys_usw_ftm_query_table_entry_num(lchip, tbl_id, &entry_enum);
    dyn_tbl_opf_type = OPF_NH_OUTER_SPME;
    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L3EDIT_SPME].opf_pool_type = dyn_tbl_opf_type;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = dyn_tbl_opf_type;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 2, entry_enum*2-1));


    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE].opf_pool_type = OPF_NH_L2EDIT_VLAN_PROFILE;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = OPF_NH_L2EDIT_VLAN_PROFILE;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, 63));


    p_master->nh_table_info_array[SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE].opf_pool_type = OPF_NH_DESTMAP_PROFILE;
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = OPF_NH_DESTMAP_PROFILE;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, 2047));

    p_master->internal_nexthop_base = 0xFFF;
    if (nexthop_entry_num >= 64*1024 && !(sys_usw_chip_get_rchain_en() && lchip))
    {
        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 16, 0xFFF0);
    }

    /*1. DsNexthop offset for bridge, */
    CTC_ERROR_RETURN(_sys_usw_nh_dsnh_init_for_brg(lchip,
                                                          &(p_master->sys_nh_resolved_offset \
                                                          [SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH])));

    /*2. DsNexthop for bypassall*/
    CTC_ERROR_RETURN(_sys_usw_nh_dsnh_init_for_bypass(lchip,
                                                             &(p_master->sys_nh_resolved_offset \
                                                             [SYS_NH_RES_OFFSET_TYPE_BYPASS_NH])));

    /*3. DsNexthop for mirror*/
    CTC_ERROR_RETURN(_sys_usw_nh_dsnh_init_for_mirror(lchip,
                                                             &(p_master->sys_nh_resolved_offset \
                                                             [SYS_NH_RES_OFFSET_TYPE_MIRROR_NH])));

    /*4. l2edit for ipmc phy if*/
    CTC_ERROR_RETURN(_sys_usw_nh_l2edit_init_for_ecmp_if(lchip,
                                                             &(p_master->ecmp_if_resolved_l2edit)));

    /*5. dsfwd for reflective (loopback)*/
    CTC_ERROR_RETURN(_sys_usw_nh_dsfwd_init_for_reflective(lchip,
                                                                  &(p_master->reflective_resolved_dsfwd_offset)));

    CTC_ERROR_RETURN(_sys_usw_nh_dsfwd_init_for_drop(lchip));

    /* init for ip tunnel*/
    CTC_ERROR_RETURN(sys_usw_nh_ip_tunnel_init(lchip));

    /* init opf for ecmp groupid */
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &opf_type, 1, "opf-ecmp-group"));
    g_usw_nh_master[lchip]->ecmp_group_opf_type = opf_type;
    opf.pool_type = g_usw_nh_master[lchip]->ecmp_group_opf_type;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, MCHIP_CAP(SYS_CAP_NH_ECMP_GROUP_ID_NUM)));

    /* init opf for ecmp member*/
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &opf_type, 1,  "opf-ecmp-member"));
    g_usw_nh_master[lchip]->ecmp_member_opf_type = opf_type;
    opf.pool_type = g_usw_nh_master[lchip]->ecmp_member_opf_type;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, MCHIP_CAP(SYS_CAP_NH_ECMP_MEMBER_NUM)));

    /* init for mcast group nhid vector*/
    p_master->mcast_group_vec = ctc_vector_init((p_master->max_glb_met_offset + SYS_NH_MCAST_GROUP_MAX_BLK_SIZE -1)/ SYS_NH_MCAST_GROUP_MAX_BLK_SIZE, SYS_NH_MCAST_GROUP_MAX_BLK_SIZE);
    if (NULL == p_master->mcast_group_vec)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_edit_spool_hash_make(sys_nh_db_edit_t* p_data)
{
    return ctc_hash_caculate(CTC_OFFSET_OF(sys_nh_db_edit_t, calc_key_len), &(p_data->edit_type));
}

STATIC bool
_sys_usw_nh_edit_spool_hash_cmp(sys_nh_db_edit_t* p_data0, sys_nh_db_edit_t* p_data1)
{
    if (!p_data0 || !p_data1)
    {
        return TRUE;
    }

    if (!sal_memcmp(&(p_data0->data), &(p_data1->data), sizeof(p_data0->data))
        && (p_data0->edit_type == p_data1->edit_type))
    {
        return TRUE;
    }

    return FALSE;
}


int32
_sys_usw_nh_edit_spool_alloc_index(sys_nh_db_edit_t* p_edit, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    int32 ret = 0;

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        ret = sys_usw_nh_offset_alloc_from_position(lchip, p_edit->edit_type, 1, p_edit->offset);
    }
    else
    {
        ret = sys_usw_nh_offset_alloc(lchip, p_edit->edit_type, 1, &p_edit->offset);

    }

    if(ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "spool alloc index failed,  line=%d\n",__LINE__);
    }
    return ret;
}


int32
_sys_usw_nh_edit_spool_free_index(sys_nh_db_edit_t* p_edit, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    ret = sys_usw_nh_offset_free(lchip, p_edit->edit_type, 1, p_edit->offset);
    if(ret)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "spool fress index failed,  offset=%d\n",p_edit->offset);
        return ret;
    }

    return ret;
}


STATIC int32
_sys_usw_nh_arp_hash_make(sys_nh_db_arp_t* p_data)
{
    return ctc_hash_caculate(sizeof(uint32), &(p_data->arp_id));
}

STATIC bool
_sys_usw_nh_arp_hash_cmp(sys_nh_db_arp_t* p_data0, sys_nh_db_arp_t* p_data1)
{
    if (!p_data0 || !p_data1)
    {
        return TRUE;
    }

    if (p_data0->arp_id == p_data1->arp_id)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_nh_hash_make(sys_nh_info_com_t* p_data)
{
    return ctc_hash_caculate(sizeof(uint32), &(p_data->hdr.nh_id));
}

STATIC bool
_sys_usw_nh_hash_cmp(sys_nh_info_com_t* p_data0, sys_nh_info_com_t* p_data1)
{
    if (!p_data0 || !p_data1)
    {
        return TRUE;
    }

    if (p_data0->hdr.nh_id == p_data1->hdr.nh_id)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_nh_id_init(uint8 lchip, sys_usw_nh_master_t* p_master)
{
    sys_usw_opf_t opf;
    uint32 internal_nh_num;
    uint8 opf_type = 0;
    ctc_hash_t* p_nh_hash = NULL;
    uint32 start_internal_nhid = SYS_NH_INTERNAL_NHID_BASE;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    internal_nh_num = SYS_NH_INTERNAL_NHID_MAX_SIZE;

    /*1. Creat hash, use hash to store internal nh_info*/
    p_nh_hash = ctc_hash_create(SYS_NH_HASH_MAX_BLK_NUM,
                                                    SYS_NH_HASH_BLK_SIZE,
                                                    (hash_key_fn)_sys_usw_nh_hash_make,
                                                    (hash_cmp_fn)_sys_usw_nh_hash_cmp);
    if (!p_nh_hash)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    p_master->nhid_hash = p_nh_hash;

    /*2. Init nhid pool, use opf to alloc/free internal nhid*/
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &opf_type, OPF_NH_DYN_MAX, "opf-nexthop"));
    p_master->nh_opf_type = opf_type;

    /*NHID resource :
     External nhid range: 0~0x7FFFFFFF
     Internal nhid range: 0x80000000~0xFFFFFFFF
    */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = p_master->nh_opf_type;
    opf.pool_index = OPF_NH_NHID_INTERNAL;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_internal_nhid, internal_nh_num));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_tunnel_id_init(uint8 lchip, ctc_vector_t** pp_tunnel_id_vec, uint16 max_tunnel_id)
{
    ctc_vector_t* p_tunnel_id_vec;
    uint16 block_size = (max_tunnel_id + SYS_NH_TUNNEL_ID_MAX_BLK_NUM - 1) / SYS_NH_TUNNEL_ID_MAX_BLK_NUM;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*1. Creat vector, use vector to store internal nh_info*/
    p_tunnel_id_vec = ctc_vector_init(SYS_NH_TUNNEL_ID_MAX_BLK_NUM, block_size);
    if (NULL == p_tunnel_id_vec)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    *pp_tunnel_id_vec = p_tunnel_id_vec;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_arp_id_init(uint8 lchip, ctc_hash_t** pp_arp_id_hah, uint16 arp_number)
{
    ctc_hash_t* p_arp_id_hash = NULL;
    uint16 block_size = (arp_number + SYS_NH_ARP_ID_MAX_BLK_NUM - 1) / SYS_NH_ARP_ID_MAX_BLK_NUM;

    p_arp_id_hash = ctc_hash_create(SYS_NH_ARP_ID_MAX_BLK_NUM,
                                                    block_size,
                                                    (hash_key_fn)_sys_usw_nh_arp_hash_make,
                                                    (hash_cmp_fn)_sys_usw_nh_arp_hash_cmp);
    if (!p_arp_id_hash)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    *pp_arp_id_hah = p_arp_id_hash;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_register_callback(uint8 lchip, sys_usw_nh_master_t* p_master, uint16 nh_param_type,
                                    p_sys_nh_create_cb_t nh_create_cb, p_sys_nh_delete_cb_t nh_del_cb,
                                    p_sys_nh_update_cb_t nh_update_cb)
{
    p_master->callbacks_nh_create[nh_param_type] = nh_create_cb;
    p_master->callbacks_nh_delete[nh_param_type] = nh_del_cb;
    p_master->callbacks_nh_update[nh_param_type] = nh_update_cb;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_callback_init(uint8 lchip, sys_usw_nh_master_t* p_master)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);



    /* 1. Ucast bridge */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_BRGUC,
                                                         sys_usw_nh_create_brguc_cb,
                                                         sys_usw_nh_delete_brguc_cb,
                                                         sys_usw_nh_update_brguc_cb));



    /* 2. Mcast bridge */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_MCAST,
                                                         &sys_usw_nh_create_mcast_cb,
                                                         &sys_usw_nh_delete_mcast_cb,
                                                         &sys_usw_nh_update_mcast_cb));



    /* 3. IPUC */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_IPUC,
                                                         &sys_usw_nh_create_ipuc_cb,
                                                         &sys_usw_nh_delete_ipuc_cb,
                                                         &sys_usw_nh_update_ipuc_cb));

    /* 4. ECMP */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_ECMP,
                                                         &sys_usw_nh_create_ecmp_cb,
                                                         &sys_usw_nh_delete_ecmp_cb,
                                                         &sys_usw_nh_update_ecmp_cb));

    /* 5. MPLS */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_MPLS,
                                                         &sys_usw_nh_create_mpls_cb,
                                                         &sys_usw_nh_delete_mpls_cb,
                                                         &sys_usw_nh_update_mpls_cb));

    /* 6. Drop */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_DROP,
                                                         &sys_usw_nh_create_special_cb,
                                                         &sys_usw_nh_delete_special_cb, NULL));

    /* 7. ToCPU */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_TOCPU,
                                                         &sys_usw_nh_create_special_cb,
                                                         &sys_usw_nh_delete_special_cb, NULL));

    /* 8. Unresolve */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_UNROV,
                                                         &sys_usw_nh_create_special_cb,
                                                         &sys_usw_nh_delete_special_cb, NULL));

    /* 9. ILoop */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_ILOOP,
                                                         &sys_usw_nh_create_iloop_cb,
                                                         &sys_usw_nh_delete_iloop_cb,
                                                         &sys_usw_nh_update_iloop_cb));

    /* 10. rspan */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_RSPAN,
                                                         &sys_usw_nh_create_rspan_cb,
                                                         &sys_usw_nh_delete_rspan_cb,
                                                         NULL));

    /* 11. ip-tunnel */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_IP_TUNNEL,
                                                         &sys_usw_nh_create_ip_tunnel_cb,
                                                         &sys_usw_nh_delete_ip_tunnel_cb,
                                                         &sys_usw_nh_update_ip_tunnel_cb));

    /* 12. trill-tunnel */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_TRILL,
                                                         &sys_usw_nh_create_trill_cb,
                                                         &sys_usw_nh_delete_trill_cb,
                                                         &sys_usw_nh_update_trill_cb));

    /* 13. misc */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_MISC,
                                                         &sys_usw_nh_create_misc_cb,
                                                         &sys_usw_nh_delete_misc_cb,
                                                         &sys_usw_nh_update_misc_cb));
    /* 14. wlan-tunnel */
    CTC_ERROR_RETURN(_sys_usw_nh_register_callback(lchip, p_master,
                                                         SYS_NH_TYPE_WLAN_TUNNEL,
                                                         &sys_usw_nh_create_wlan_tunnel_cb,
                                                         &sys_usw_nh_delete_wlan_tunnel_cb,
                                                         &sys_usw_nh_update_wlan_tunnel_cb));
    return CTC_E_NONE;
}

/**
 @brief This function is used to create and init nexthop module master data
 */
STATIC int32
_sys_usw_nh_master_new(uint8 lchip, sys_usw_nh_master_t** pp_nexthop_master, uint32 boundary_of_extnl_intnl_nhid, uint16 max_tunnel_id, uint16 arp_num)
{
    sys_usw_nh_master_t* p_master = NULL;
    ctc_spool_t spool;
    uint8 dyn_tbl_idx = 0;
    uint32 entry_enum = 0;
    uint32 nh_glb_entry_num = 0;
    int32 ret = CTC_E_NONE;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (NULL == pp_nexthop_master || (NULL != *pp_nexthop_master))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid pointer \n");
			return CTC_E_INVALID_PTR;

    }

    /*1. allocate memory for nexthop master*/
    p_master = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_usw_nh_master_t));
    if (NULL == p_master)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_master, 0, sizeof(sys_usw_nh_master_t));
    sal_memset(&spool, 0, sizeof(ctc_spool_t));

    /*2. l2&l3 edit spool init */
    sys_usw_ftm_get_dynamic_table_info(lchip, DsL2EditEth3W_t,  &dyn_tbl_idx, &entry_enum, &nh_glb_entry_num);
    if (0 == entry_enum)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " l2&l3 edit have no resource!! \n");
        ret = CTC_E_NOT_INIT;
        goto clean;
    }

    spool.lchip = lchip;
    spool.block_num = CTC_VEC_BLOCK_NUM(entry_enum, SYS_NH_L2EDIT_HASH_BLOCK_NUM);
    spool.block_size = SYS_NH_L2EDIT_HASH_BLOCK_NUM;
    spool.max_count = entry_enum;
    spool.user_data_size = sizeof(sys_nh_db_edit_t);
    spool.spool_key = (hash_key_fn) _sys_usw_nh_edit_spool_hash_make;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_nh_edit_spool_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_nh_edit_spool_alloc_index;
    spool.spool_free = (spool_free_fn)_sys_usw_nh_edit_spool_free_index;
    p_master->p_edit_spool = ctc_spool_create(&spool);
    if (!p_master->p_edit_spool)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto clean;
    }

    /*5. Nexthop hash init*/
    CTC_ERROR_GOTO(_sys_usw_nh_id_init(lchip, p_master), ret, clean);

    /*6. Tunnel id vector init*/
    CTC_ERROR_GOTO(_sys_usw_nh_tunnel_id_init(lchip, &(p_master->tunnel_id_vec), max_tunnel_id), ret, clean);

    /*7. Arp id hash init*/
    CTC_ERROR_GOTO(_sys_usw_nh_arp_id_init(lchip, &(p_master->arp_id_hash), arp_num), ret, clean);

    /*8. Eloop list init*/
    p_master->eloop_list = ctc_slist_new();
    if (p_master->eloop_list == NULL)
    {
        ret = CTC_E_INVALID_PARAM;
        goto clean;
    }

    /*9. logic rep list init*/
    p_master->repli_offset_list = ctc_slist_new();
    if (p_master->repli_offset_list == NULL)
    {
        ret = CTC_E_INVALID_PARAM;
        goto clean1;
    }

    /*Return PTR*/
    *pp_nexthop_master = p_master;

    return CTC_E_NONE;

clean1:
    ctc_slist_free(p_master->eloop_list);
clean:
    mem_free(p_master);
    return ret;
}

#define __________warmboot___________
int32
_sys_usw_nh_wb_l2edit_mapping(void* p_l2editeth,
            sys_wb_nh_l2edit_t *p_wb_dsl2edit, uint8 is_8w, uint8 sync)
{
    sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = NULL;
    sys_nh_db_dsl2editeth8w_t* p_l2edit_8w = NULL;

    if (sync)
    {
        p_l2edit_4w = (sys_nh_db_dsl2editeth4w_t*)p_l2editeth;
        sal_memcpy(&p_wb_dsl2edit->mac_da[0], &p_l2edit_4w->mac_da[0], sizeof(mac_addr_t));
        p_wb_dsl2edit->output_vid = p_l2edit_4w->output_vid;
        p_wb_dsl2edit->strip_inner_vlan = p_l2edit_4w->strip_inner_vlan;
        p_wb_dsl2edit->fpma = p_l2edit_4w->fpma;
        p_wb_dsl2edit->is_ecmp_if = p_l2edit_4w->is_ecmp_if;
        p_wb_dsl2edit->is_share_mac = p_l2edit_4w->is_share_mac;
        p_wb_dsl2edit->output_vlan_is_svlan = p_l2edit_4w->output_vlan_is_svlan;
        p_wb_dsl2edit->dynamic = p_l2edit_4w->dynamic;
        p_wb_dsl2edit->is_dot11 = p_l2edit_4w->is_dot11;
        p_wb_dsl2edit->derive_mac_from_label = p_l2edit_4w->derive_mac_from_label;
        p_wb_dsl2edit->derive_mcast_mac_for_mpls = p_l2edit_4w->derive_mcast_mac_for_mpls;
        p_wb_dsl2edit->derive_mcast_mac_for_trill = p_l2edit_4w->derive_mcast_mac_for_trill;
        p_wb_dsl2edit->dot11_sub_type = p_l2edit_4w->dot11_sub_type;
        p_wb_dsl2edit->derive_mcast_mac_for_ip = p_l2edit_4w->derive_mcast_mac_for_ip;

        p_wb_dsl2edit->is_8w = is_8w;
        if (is_8w)
        {
            p_l2edit_8w = (sys_nh_db_dsl2editeth8w_t*)p_l2editeth;
            sal_memcpy(&p_wb_dsl2edit->mac_sa[0], &p_l2edit_8w->mac_sa[0], sizeof(mac_addr_t));
            p_wb_dsl2edit->update_mac_sa = p_l2edit_8w->update_mac_sa;
            p_wb_dsl2edit->cos_domain = p_l2edit_8w->cos_domain;
            p_wb_dsl2edit->packet_type = p_l2edit_8w->packet_type;
        }
    }
    else
    {
        if (!is_8w)
        {
            p_l2edit_4w = p_l2editeth;
            sal_memcpy(&p_l2edit_4w->mac_da[0], &p_wb_dsl2edit->mac_da[0], sizeof(mac_addr_t));
            p_l2edit_4w->output_vid = p_wb_dsl2edit->output_vid;
            p_l2edit_4w->strip_inner_vlan = p_wb_dsl2edit->strip_inner_vlan;
            p_l2edit_4w->fpma = p_wb_dsl2edit->fpma;
            p_l2edit_4w->is_ecmp_if = p_wb_dsl2edit->is_ecmp_if;
            p_l2edit_4w->is_share_mac = p_wb_dsl2edit->is_share_mac;
            p_l2edit_4w->output_vlan_is_svlan = p_wb_dsl2edit->output_vlan_is_svlan;
            p_l2edit_4w->dynamic = p_wb_dsl2edit->dynamic;
            p_l2edit_4w->is_dot11 = p_wb_dsl2edit->is_dot11;
            p_l2edit_4w->derive_mac_from_label = p_wb_dsl2edit->derive_mac_from_label;
            p_l2edit_4w->derive_mcast_mac_for_mpls = p_wb_dsl2edit->derive_mcast_mac_for_mpls;
            p_l2edit_4w->derive_mcast_mac_for_trill = p_wb_dsl2edit->derive_mcast_mac_for_trill;
            p_l2edit_4w->dot11_sub_type = p_wb_dsl2edit->dot11_sub_type;
            p_l2edit_4w->derive_mcast_mac_for_ip = p_wb_dsl2edit->derive_mcast_mac_for_ip;
        }
        else
        {
            p_l2edit_8w = (sys_nh_db_dsl2editeth8w_t*)p_l2editeth;
            sal_memcpy(&p_l2edit_8w->mac_da[0], &p_wb_dsl2edit->mac_da[0], sizeof(mac_addr_t));
            p_l2edit_8w->output_vid = p_wb_dsl2edit->output_vid;
            p_l2edit_8w->strip_inner_vlan = p_wb_dsl2edit->strip_inner_vlan;
            p_l2edit_8w->fpma = p_wb_dsl2edit->fpma;
            p_l2edit_8w->is_ecmp_if = p_wb_dsl2edit->is_ecmp_if;
            p_l2edit_8w->is_share_mac = p_wb_dsl2edit->is_share_mac;
            p_l2edit_8w->output_vlan_is_svlan = p_wb_dsl2edit->output_vlan_is_svlan;
            p_l2edit_8w->dynamic = p_wb_dsl2edit->dynamic;
            p_l2edit_8w->is_dot11 = p_wb_dsl2edit->is_dot11;
            p_l2edit_8w->derive_mac_from_label = p_wb_dsl2edit->derive_mac_from_label;
            p_l2edit_8w->derive_mcast_mac_for_mpls = p_wb_dsl2edit->derive_mcast_mac_for_mpls;
            p_l2edit_8w->derive_mcast_mac_for_trill = p_wb_dsl2edit->derive_mcast_mac_for_trill;
            p_l2edit_8w->dot11_sub_type = p_wb_dsl2edit->dot11_sub_type;
            p_l2edit_8w->derive_mcast_mac_for_ip = p_wb_dsl2edit->derive_mcast_mac_for_ip;

            sal_memcpy(&p_l2edit_8w->mac_sa[0], &p_wb_dsl2edit->mac_sa[0], sizeof(mac_addr_t));
            p_l2edit_8w->update_mac_sa = p_wb_dsl2edit->update_mac_sa;
            p_l2edit_8w->cos_domain = p_wb_dsl2edit->cos_domain;
            p_l2edit_8w->packet_type = p_wb_dsl2edit->packet_type;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_l3edit_mapping(void* p_l3edit, sys_wb_nh_l3edit_t *p_wb_dsl3edit, uint8 is_v6, uint8 sync)
{
    sys_dsl3edit_tunnelv6_t* p_dsl3editv6 = (sys_dsl3edit_tunnelv6_t*)p_l3edit;
    sys_dsl3edit_tunnelv4_t* p_dsl3editv4 = (sys_dsl3edit_tunnelv4_t*)p_l3edit;

    if (sync)
    {
        p_wb_dsl3edit->share_type  = (is_v6)?p_dsl3editv6->share_type:p_dsl3editv4->share_type;
        p_wb_dsl3edit->ds_type  = (is_v6)?p_dsl3editv6->ds_type:p_dsl3editv4->ds_type;
        p_wb_dsl3edit->l3_rewrite_type  = (is_v6)?p_dsl3editv6->l3_rewrite_type:p_dsl3editv4->l3_rewrite_type;
        p_wb_dsl3edit->map_ttl  = (is_v6)?p_dsl3editv6->map_ttl:p_dsl3editv4->map_ttl;
        p_wb_dsl3edit->ttl  = (is_v6)?p_dsl3editv6->ttl:p_dsl3editv4->ttl;
        p_wb_dsl3edit->dscp_domain  = (is_v6)?p_dsl3editv6->dscp_domain:p_dsl3editv4->dscp_domain;
        p_wb_dsl3edit->ip_protocol_type  = (is_v6)?p_dsl3editv6->ip_protocol_type:p_dsl3editv4->ip_protocol_type;
        p_wb_dsl3edit->mtu_check_en  = (is_v6)?p_dsl3editv6->mtu_check_en:p_dsl3editv4->mtu_check_en;
        p_wb_dsl3edit->mtu_size  =   (is_v6)?p_dsl3editv6->mtu_size:p_dsl3editv4->mtu_size;
        p_wb_dsl3edit->ipsa_index  = (is_v6)?p_dsl3editv6->ipsa_index:p_dsl3editv4->ipsa_index;
        p_wb_dsl3edit->inner_header_valid  = (is_v6)?p_dsl3editv6->inner_header_valid:p_dsl3editv4->inner_header_valid;
        p_wb_dsl3edit->inner_header_type  = (is_v6)?p_dsl3editv6->inner_header_type:p_dsl3editv4->inner_header_type;
        p_wb_dsl3edit->stats_ptr  = (is_v6)?p_dsl3editv6->stats_ptr:p_dsl3editv4->stats_ptr;
        p_wb_dsl3edit->gre_protocol  = (is_v6)?p_dsl3editv6->gre_protocol:p_dsl3editv4->gre_protocol;
        p_wb_dsl3edit->gre_version  = (is_v6)?p_dsl3editv6->gre_version:p_dsl3editv4->gre_version;
        p_wb_dsl3edit->gre_flags  = (is_v6)?p_dsl3editv6->gre_flags:p_dsl3editv4->gre_flags;
        p_wb_dsl3edit->gre_key  = (is_v6)?p_dsl3editv6->gre_key:p_dsl3editv4->gre_key;
        p_wb_dsl3edit->out_l2edit_valid  = (is_v6)?p_dsl3editv6->out_l2edit_valid:p_dsl3editv4->out_l2edit_valid;
        p_wb_dsl3edit->xerspan_hdr_en  = (is_v6)?p_dsl3editv6->xerspan_hdr_en:p_dsl3editv4->xerspan_hdr_en;
        p_wb_dsl3edit->xerspan_hdr_with_hash_en  = (is_v6)?p_dsl3editv6->xerspan_hdr_with_hash_en:p_dsl3editv4->xerspan_hdr_with_hash_en;
        p_wb_dsl3edit->l2edit_ptr  = (is_v6)?p_dsl3editv6->l2edit_ptr:p_dsl3editv4->l2edit_ptr;
        p_wb_dsl3edit->is_evxlan  = (is_v6)?p_dsl3editv6->is_evxlan:p_dsl3editv4->is_evxlan;
        p_wb_dsl3edit->evxlan_protocl_index  = (is_v6)?p_dsl3editv6->evxlan_protocl_index:p_dsl3editv4->evxlan_protocl_index;
        p_wb_dsl3edit->is_geneve  = (is_v6)?p_dsl3editv6->is_geneve:p_dsl3editv4->is_geneve;
        p_wb_dsl3edit->l4_src_port  = (is_v6)?p_dsl3editv6->l4_src_port:p_dsl3editv4->l4_src_port;
        p_wb_dsl3edit->l4_dest_port  = (is_v6)?p_dsl3editv6->l4_dest_port:p_dsl3editv4->l4_dest_port;
        p_wb_dsl3edit->encrypt_en  = (is_v6)?p_dsl3editv6->encrypt_en:p_dsl3editv4->encrypt_en;
        p_wb_dsl3edit->encrypt_id  = (is_v6)?p_dsl3editv6->encrypt_id:p_dsl3editv4->encrypt_id;
        p_wb_dsl3edit->bssid_bitmap  = (is_v6)?p_dsl3editv6->bssid_bitmap:p_dsl3editv4->bssid_bitmap;
        p_wb_dsl3edit->rmac_en  = (is_v6)?p_dsl3editv6->rmac_en:p_dsl3editv4->rmac_en;
        p_wb_dsl3edit->data_keepalive  = (is_v6)?p_dsl3editv6->data_keepalive:p_dsl3editv4->data_keepalive;
        p_wb_dsl3edit->ecn_mode  = (is_v6)?p_dsl3editv6->ecn_mode:p_dsl3editv4->ecn_mode;
        p_wb_dsl3edit->udp_src_port_en = (is_v6)?p_dsl3editv6->udp_src_port_en:p_dsl3editv4->udp_src_port_en;
        p_wb_dsl3edit->copy_dont_frag =  (is_v6)?0:p_dsl3editv4->copy_dont_frag;
        p_wb_dsl3edit->dont_frag =  (is_v6)?0:p_dsl3editv4->dont_frag;
        p_wb_dsl3edit->derive_dscp =  (is_v6)?p_dsl3editv6->derive_dscp:p_dsl3editv4->derive_dscp;
        p_wb_dsl3edit->dscp =  (is_v6)?0:p_dsl3editv4->dscp;
        if (!is_v6)
        {
            p_wb_dsl3edit->l3.v4.ipda = p_dsl3editv4->ipda;
            p_wb_dsl3edit->l3.v4.ipsa = p_dsl3editv4->ipsa;
            p_wb_dsl3edit->l3.v4.ipsa_6to4 = p_dsl3editv4->ipsa_6to4;
            p_wb_dsl3edit->l3.v4.ipsa_valid = p_dsl3editv4->ipsa_valid;
            p_wb_dsl3edit->l3.v4.isatp_tunnel = p_dsl3editv4->isatp_tunnel;
            p_wb_dsl3edit->l3.v4.tunnel6_to4_da = p_dsl3editv4->tunnel6_to4_da;
            p_wb_dsl3edit->l3.v4.tunnel6_to4_sa = p_dsl3editv4->tunnel6_to4_sa;
            p_wb_dsl3edit->l3.v4.tunnel6_to4_da_ipv4_prefixlen = p_dsl3editv4->tunnel6_to4_da_ipv4_prefixlen;
            p_wb_dsl3edit->l3.v4.tunnel6_to4_da_ipv6_prefixlen = p_dsl3editv4->tunnel6_to4_da_ipv6_prefixlen;
            p_wb_dsl3edit->l3.v4.tunnel6_to4_sa_ipv4_prefixlen = p_dsl3editv4->tunnel6_to4_sa_ipv4_prefixlen;
            p_wb_dsl3edit->l3.v4.tunnel6_to4_sa_ipv6_prefixlen = p_dsl3editv4->tunnel6_to4_sa_ipv6_prefixlen;
        }
        else
        {
            p_wb_dsl3edit->l3.v6.flow_label = p_dsl3editv6->flow_label;
            p_wb_dsl3edit->l3.v6.new_flow_label_valid = p_dsl3editv6->new_flow_label_valid;
            p_wb_dsl3edit->l3.v6.new_flow_label_mode = p_dsl3editv6->new_flow_label_mode;
            p_wb_dsl3edit->l3.v6.ipda[0] = p_dsl3editv6->ipda[0];
            p_wb_dsl3edit->l3.v6.ipda[1] = p_dsl3editv6->ipda[1];
            p_wb_dsl3edit->l3.v6.ipda[2] = p_dsl3editv6->ipda[2];
            p_wb_dsl3edit->l3.v6.ipda[3] = p_dsl3editv6->ipda[3];
        }
    }
    else
    {
        if (!is_v6)
        {
            p_dsl3editv4->share_type = p_wb_dsl3edit->share_type;
            p_dsl3editv4->ds_type = p_wb_dsl3edit->ds_type;
            p_dsl3editv4->l3_rewrite_type = p_wb_dsl3edit->l3_rewrite_type;
            p_dsl3editv4->map_ttl = p_wb_dsl3edit->map_ttl;
            p_dsl3editv4->ttl = p_wb_dsl3edit->ttl;
            p_dsl3editv4->dscp_domain = p_wb_dsl3edit->dscp_domain;
            p_dsl3editv4->ip_protocol_type = p_wb_dsl3edit->ip_protocol_type;
            p_dsl3editv4->mtu_check_en = p_wb_dsl3edit->mtu_check_en;
            p_dsl3editv4->mtu_size = p_wb_dsl3edit->mtu_size;
            p_dsl3editv4->ipsa_index = p_wb_dsl3edit->ipsa_index;
            p_dsl3editv4->inner_header_valid = p_wb_dsl3edit->inner_header_valid;
            p_dsl3editv4->inner_header_type = p_wb_dsl3edit->inner_header_type;
            p_dsl3editv4->stats_ptr = p_wb_dsl3edit->stats_ptr;
            p_dsl3editv4->gre_protocol = p_wb_dsl3edit->gre_protocol;
            p_dsl3editv4->gre_version = p_wb_dsl3edit->gre_version;
            p_dsl3editv4->gre_flags = p_wb_dsl3edit->gre_flags;
            p_dsl3editv4->gre_key   = p_wb_dsl3edit->gre_key;
            p_dsl3editv4->out_l2edit_valid = p_wb_dsl3edit->out_l2edit_valid;
            p_dsl3editv4->xerspan_hdr_en = p_wb_dsl3edit->xerspan_hdr_en;
            p_dsl3editv4->xerspan_hdr_with_hash_en = p_wb_dsl3edit->xerspan_hdr_with_hash_en;
            p_dsl3editv4->l2edit_ptr = p_wb_dsl3edit->l2edit_ptr;
            p_dsl3editv4->is_evxlan = p_wb_dsl3edit->is_evxlan;
            p_dsl3editv4->evxlan_protocl_index = p_wb_dsl3edit->evxlan_protocl_index;
            p_dsl3editv4->is_geneve = p_wb_dsl3edit->is_geneve;
            p_dsl3editv4->l4_src_port = p_wb_dsl3edit->l4_src_port;
            p_dsl3editv4->l4_dest_port = p_wb_dsl3edit->l4_dest_port;
            p_dsl3editv4->encrypt_en = p_wb_dsl3edit->encrypt_en;
            p_dsl3editv4->encrypt_id = p_wb_dsl3edit->encrypt_id;
            p_dsl3editv4->bssid_bitmap = p_wb_dsl3edit->bssid_bitmap;
            p_dsl3editv4->rmac_en = p_wb_dsl3edit->rmac_en;
            p_dsl3editv4->data_keepalive = p_wb_dsl3edit->data_keepalive;
            p_dsl3editv4->ecn_mode = p_wb_dsl3edit->ecn_mode;
            p_dsl3editv4->udp_src_port_en = p_wb_dsl3edit->udp_src_port_en;
            p_dsl3editv4->copy_dont_frag = p_wb_dsl3edit->copy_dont_frag;
            p_dsl3editv4->dont_frag = p_wb_dsl3edit->dont_frag;
            p_dsl3editv4->derive_dscp = p_wb_dsl3edit->derive_dscp;
            p_dsl3editv4->dscp = p_wb_dsl3edit->dscp;

            p_dsl3editv4->ipda = p_wb_dsl3edit->l3.v4.ipda;
            p_dsl3editv4->ipsa = p_wb_dsl3edit->l3.v4.ipsa;
            p_dsl3editv4->ipsa_6to4 = p_wb_dsl3edit->l3.v4.ipsa_6to4;
            p_dsl3editv4->ipsa_valid = p_wb_dsl3edit->l3.v4.ipsa_valid;
            p_dsl3editv4->isatp_tunnel = p_wb_dsl3edit->l3.v4.isatp_tunnel;
            p_dsl3editv4->tunnel6_to4_da = p_wb_dsl3edit->l3.v4.tunnel6_to4_da;
            p_dsl3editv4->tunnel6_to4_sa = p_wb_dsl3edit->l3.v4.tunnel6_to4_sa;
            p_dsl3editv4->tunnel6_to4_da_ipv4_prefixlen = p_wb_dsl3edit->l3.v4.tunnel6_to4_da_ipv4_prefixlen;
            p_dsl3editv4->tunnel6_to4_da_ipv6_prefixlen = p_wb_dsl3edit->l3.v4.tunnel6_to4_da_ipv6_prefixlen;
            p_dsl3editv4->tunnel6_to4_sa_ipv4_prefixlen = p_wb_dsl3edit->l3.v4.tunnel6_to4_sa_ipv4_prefixlen;
            p_dsl3editv4->tunnel6_to4_sa_ipv6_prefixlen = p_wb_dsl3edit->l3.v4.tunnel6_to4_sa_ipv6_prefixlen;
        }
        else
        {
            p_dsl3editv6->share_type = p_wb_dsl3edit->share_type;
            p_dsl3editv6->ds_type = p_wb_dsl3edit->ds_type;
            p_dsl3editv6->l3_rewrite_type = p_wb_dsl3edit->l3_rewrite_type;
            p_dsl3editv6->map_ttl = p_wb_dsl3edit->map_ttl;
            p_dsl3editv6->ttl = p_wb_dsl3edit->ttl;
            p_dsl3editv6->dscp_domain = p_wb_dsl3edit->dscp_domain;
            p_dsl3editv6->ip_protocol_type = p_wb_dsl3edit->ip_protocol_type;
            p_dsl3editv6->mtu_check_en = p_wb_dsl3edit->mtu_check_en;
            p_dsl3editv6->mtu_size = p_wb_dsl3edit->mtu_size;
            p_dsl3editv6->ipsa_index = p_wb_dsl3edit->ipsa_index;
            p_dsl3editv6->inner_header_valid = p_wb_dsl3edit->inner_header_valid;
            p_dsl3editv6->inner_header_type = p_wb_dsl3edit->inner_header_type;
            p_dsl3editv6->stats_ptr = p_wb_dsl3edit->stats_ptr;
            p_dsl3editv6->gre_protocol = p_wb_dsl3edit->gre_protocol;
            p_dsl3editv6->gre_version = p_wb_dsl3edit->gre_version;
            p_dsl3editv6->gre_flags = p_wb_dsl3edit->gre_flags;
            p_dsl3editv6->gre_key   = p_wb_dsl3edit->gre_key;
            p_dsl3editv6->out_l2edit_valid = p_wb_dsl3edit->out_l2edit_valid;
            p_dsl3editv6->xerspan_hdr_en = p_wb_dsl3edit->xerspan_hdr_en;
            p_dsl3editv6->xerspan_hdr_with_hash_en = p_wb_dsl3edit->xerspan_hdr_with_hash_en;
            p_dsl3editv6->l2edit_ptr = p_wb_dsl3edit->l2edit_ptr;
            p_dsl3editv6->is_evxlan = p_wb_dsl3edit->is_evxlan;
            p_dsl3editv6->evxlan_protocl_index = p_wb_dsl3edit->evxlan_protocl_index;
            p_dsl3editv6->is_geneve = p_wb_dsl3edit->is_geneve;
            p_dsl3editv6->l4_src_port = p_wb_dsl3edit->l4_src_port;
            p_dsl3editv6->l4_dest_port = p_wb_dsl3edit->l4_dest_port;
            p_dsl3editv6->encrypt_en = p_wb_dsl3edit->encrypt_en;
            p_dsl3editv6->encrypt_id = p_wb_dsl3edit->encrypt_id;
            p_dsl3editv6->bssid_bitmap = p_wb_dsl3edit->bssid_bitmap;
            p_dsl3editv6->rmac_en = p_wb_dsl3edit->rmac_en;
            p_dsl3editv6->data_keepalive = p_wb_dsl3edit->data_keepalive;
            p_dsl3editv6->ecn_mode = p_wb_dsl3edit->ecn_mode;
            p_dsl3editv6->udp_src_port_en = p_wb_dsl3edit->udp_src_port_en;
            p_dsl3editv6->derive_dscp = p_wb_dsl3edit->derive_dscp;

            p_dsl3editv6->flow_label = p_wb_dsl3edit->l3.v6.flow_label;
            p_dsl3editv6->new_flow_label_valid = p_wb_dsl3edit->l3.v6.new_flow_label_valid;
            p_dsl3editv6->new_flow_label_mode = p_wb_dsl3edit->l3.v6.new_flow_label_mode;
            p_dsl3editv6->ipda[0] = p_wb_dsl3edit->l3.v6.ipda[0];
            p_dsl3editv6->ipda[1] = p_wb_dsl3edit->l3.v6.ipda[1];
            p_dsl3editv6->ipda[2] = p_wb_dsl3edit->l3.v6.ipda[2];
            p_dsl3editv6->ipda[3] = p_wb_dsl3edit->l3.v6.ipda[3];
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_nat_mapping(void* p_l3edit, sys_wb_nh_nat_t *p_wb_dsnat, uint8 is_8w, uint8 sync)
{
    sys_dsl3edit_nat4w_t* p_nat_4w = (sys_dsl3edit_nat4w_t*)p_l3edit;
    sys_dsl3edit_nat8w_t* p_nat_8w = (sys_dsl3edit_nat8w_t*)p_l3edit;

    if (sync)
    {
        p_wb_dsnat->ds_type             = (is_8w)?p_nat_8w->ds_type:p_nat_4w->ds_type;
        p_wb_dsnat->l3_rewrite_type    = (is_8w)?p_nat_8w->l3_rewrite_type:p_nat_4w->l3_rewrite_type;
        p_wb_dsnat->ip_da_prefix_length    = (is_8w)?p_nat_8w->ip_da_prefix_length:p_nat_4w->ip_da_prefix_length;
        p_wb_dsnat->ipv4_embeded_mode    = (is_8w)?p_nat_8w->ipv4_embeded_mode:p_nat_4w->ipv4_embeded_mode;
        p_wb_dsnat->l4_dest_port    = (is_8w)?p_nat_8w->l4_dest_port:p_nat_4w->l4_dest_port;
        p_wb_dsnat->replace_ip_da    = (is_8w)?p_nat_8w->replace_ip_da:p_nat_4w->replace_ip_da;
        p_wb_dsnat->replace_l4_dest_port    = (is_8w)?p_nat_8w->replace_l4_dest_port:p_nat_4w->replace_l4_dest_port;
        if (!is_8w)
        {
            p_wb_dsnat->ipda.ip_v4 = p_nat_4w->ipda;
        }
        else
        {
            p_wb_dsnat->ipda.ip_v6[0] = p_nat_8w->ipda[0];
            p_wb_dsnat->ipda.ip_v6[1] = p_nat_8w->ipda[1];
            p_wb_dsnat->ipda.ip_v6[2] = p_nat_8w->ipda[2];
            p_wb_dsnat->ipda.ip_v6[3] = p_nat_8w->ipda[3];
        }
    }
    else
    {
        if (!is_8w)
        {
            p_nat_4w->ds_type = p_wb_dsnat->ds_type;
            p_nat_4w->l3_rewrite_type = p_wb_dsnat->l3_rewrite_type;
            p_nat_4w->ip_da_prefix_length = p_wb_dsnat->ip_da_prefix_length;
            p_nat_4w->ipv4_embeded_mode = p_wb_dsnat->ipv4_embeded_mode;
            p_nat_4w->l4_dest_port = p_wb_dsnat->l4_dest_port;
            p_nat_4w->replace_ip_da = p_wb_dsnat->replace_ip_da;
            p_nat_4w->replace_l4_dest_port = p_wb_dsnat->replace_l4_dest_port;
            p_nat_4w->ipda = p_wb_dsnat->ipda.ip_v4;
        }
        else
        {
            p_nat_8w->ds_type = p_wb_dsnat->ds_type;
            p_nat_8w->l3_rewrite_type = p_wb_dsnat->l3_rewrite_type;
            p_nat_8w->ip_da_prefix_length = p_wb_dsnat->ip_da_prefix_length;
            p_nat_8w->ipv4_embeded_mode = p_wb_dsnat->ipv4_embeded_mode;
            p_nat_8w->l4_dest_port = p_wb_dsnat->l4_dest_port;
            p_nat_8w->replace_ip_da = p_wb_dsnat->replace_ip_da;
            p_nat_8w->replace_l4_dest_port = p_wb_dsnat->replace_l4_dest_port;

            p_nat_8w->ipda[0] = p_wb_dsnat->ipda.ip_v6[0];
            p_nat_8w->ipda[1] = p_wb_dsnat->ipda.ip_v6[1];
            p_nat_8w->ipda[2] = p_wb_dsnat->ipda.ip_v6[2];
            p_nat_8w->ipda[3] = p_wb_dsnat->ipda.ip_v6[3];
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_ecmp_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com,sys_wb_nh_info_com_t *p_wb_nh_info)
{
    sys_nh_info_ecmp_t  *p_nh_info = (sys_nh_info_ecmp_t  *)p_nh_info_com;

    p_wb_nh_info->info.ecmp.ecmp_cnt =  p_nh_info->ecmp_cnt;
    p_wb_nh_info->info.ecmp.valid_cnt =  p_nh_info->valid_cnt;
    p_wb_nh_info->info.ecmp.type =   p_nh_info->type ;
    p_wb_nh_info->info.ecmp.failover_en =   p_nh_info->failover_en;
    p_wb_nh_info->info.ecmp.ecmp_nh_id =  p_nh_info->ecmp_nh_id ;

    p_wb_nh_info->info.ecmp.ecmp_group_id =  p_nh_info->ecmp_group_id;
    p_wb_nh_info->info.ecmp.random_rr_en =  p_nh_info->random_rr_en;
    p_wb_nh_info->info.ecmp.stats_valid =   p_nh_info->stats_valid ;
    p_wb_nh_info->info.ecmp.gport =   p_nh_info->gport;
    p_wb_nh_info->info.ecmp.ecmp_member_base =  p_nh_info->ecmp_member_base ;
	p_wb_nh_info->info.ecmp.l3edit_offset_base =   p_nh_info->l3edit_offset_base;
    p_wb_nh_info->info.ecmp.l2edit_offset_base =  p_nh_info->l2edit_offset_base ;
    p_wb_nh_info->info.ecmp.stats_id = p_nh_info->stats_id;
    p_wb_nh_info->info.ecmp.mem_num = p_nh_info->mem_num;

    sal_memcpy(&p_wb_nh_info->info.ecmp.nh_array[0], p_nh_info->nh_array,p_nh_info->ecmp_cnt*sizeof(uint32));

    if (p_nh_info->type == CTC_NH_ECMP_TYPE_DLB)
    {
        sal_memcpy(&p_wb_nh_info->info.ecmp.member_id_array[0], p_nh_info->member_id_array, g_usw_nh_master[lchip]->max_ecmp*sizeof(uint8));
        sal_memcpy(&p_wb_nh_info->info.ecmp.entry_count_array[0], p_nh_info->entry_count_array, g_usw_nh_master[lchip]->max_ecmp*sizeof(uint8));
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_misc_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com,
            sys_wb_nh_info_com_t *p_wb_nh_info,uint8 sync)
{
    uint32 cmd = 0;
    sys_nh_info_misc_t  *p_nh_info = (sys_nh_info_misc_t*)p_nh_info_com;

    if (sync)
    {

        p_wb_nh_info->info.misc.dsl2edit_offset = p_nh_info->dsl2edit_offset ;
        p_wb_nh_info->info.misc.dsl3edit_offset = p_nh_info->dsl3edit_offset ;

        p_wb_nh_info->info.misc.gport =  p_nh_info->gport ;
        p_wb_nh_info->info.misc.is_swap_mac = p_nh_info->is_swap_mac ;
        p_wb_nh_info->info.misc.is_reflective = p_nh_info->is_reflective ;

        p_wb_nh_info->info.misc.misc_nh_type = p_nh_info->misc_nh_type;
        p_wb_nh_info->info.misc.l3edit_entry_type = p_nh_info->l3edit_entry_type;
        p_wb_nh_info->info.misc.l2edit_entry_type = p_nh_info->l2edit_entry_type;
        p_wb_nh_info->info.misc.vlan_profile_id = p_nh_info->vlan_profile_id;
        p_wb_nh_info->info.misc.next_l3edit_offset = p_nh_info->next_l3edit_offset;
        p_wb_nh_info->info.misc.truncated_len = p_nh_info->truncated_len;
        p_wb_nh_info->info.misc.l3ifid = p_nh_info->l3ifid;
    }
    else
    {
        p_nh_info->dsl2edit_offset = p_wb_nh_info->info.misc.dsl2edit_offset;
        p_nh_info->dsl3edit_offset = p_wb_nh_info->info.misc.dsl3edit_offset;

        p_nh_info->gport = p_wb_nh_info->info.misc.gport;
        p_nh_info->is_swap_mac = p_wb_nh_info->info.misc.is_swap_mac;
        p_nh_info->is_reflective = p_wb_nh_info->info.misc.is_reflective;

        p_nh_info->misc_nh_type = p_wb_nh_info->info.misc.misc_nh_type;
        p_nh_info->l3edit_entry_type = p_wb_nh_info->info.misc.l3edit_entry_type;
        p_nh_info->l2edit_entry_type = p_wb_nh_info->info.misc.l2edit_entry_type;
        p_nh_info->vlan_profile_id = p_wb_nh_info->info.misc.vlan_profile_id;
        p_nh_info->next_l3edit_offset = p_wb_nh_info->info.misc.next_l3edit_offset;
        p_nh_info->truncated_len = p_wb_nh_info->info.misc.truncated_len;
        p_nh_info->l3ifid = p_wb_nh_info->info.misc.l3ifid;
        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            /*recover opf*/
            sys_usw_nh_offset_alloc_from_position(lchip, p_nh_info->l2edit_entry_type, 1, p_nh_info->dsl2edit_offset);
            if (p_nh_info->vlan_profile_id)
            {
                sys_dsl2edit_vlan_edit_t vlan_edit;
                sal_memset(&vlan_edit, 0, sizeof(sys_dsl2edit_vlan_edit_t));

                /*using vlan proflie, recover spool*/
                cmd = DRV_IOR(DsOfEditVlanActionProfile_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_nh_info->vlan_profile_id, cmd, &vlan_edit.data);
                vlan_edit.profile_id = p_nh_info->vlan_profile_id;

                CTC_ERROR_RETURN(ctc_spool_add(g_usw_nh_master[lchip]->p_l2edit_vprof, &vlan_edit, NULL, NULL));
            }
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
            if (p_wb_nh_info->info.misc.l3edit_entry_type == SYS_NH_ENTRY_TYPE_L3EDIT_MPLS)
            {
                sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_nh_info->dsl3edit_offset);
                if (p_nh_info->next_l3edit_offset)
                {
                    sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_nh_info->next_l3edit_offset);
                }
            }
            else if (p_nh_info->l3edit_entry_type == SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W)
            {
                sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W, 1, p_nh_info->dsl3edit_offset);
                if (p_nh_info->next_l3edit_offset)
                {
                    sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W, 1, p_nh_info->next_l3edit_offset);
                }
            }
            else
            {
                sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_OF_6W, 1, p_nh_info->dsl3edit_offset);
                if (p_nh_info->next_l3edit_offset)
                {
                    sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_OF_6W, 1, p_nh_info->next_l3edit_offset);
                }

            }
        }

        if (p_nh_info->truncated_len)
        {
            uint8 truncate_proflile = 0;
            sys_usw_register_add_truncation_profile(lchip, p_nh_info->truncated_len, 0, &truncate_proflile);
        }

    }
    return CTC_E_NONE;

}

int32
sys_usw_nh_wb_mapping_brguc_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info, uint8 sync)
{
    sys_nh_info_brguc_t  *p_nh_info = (sys_nh_info_brguc_t  *)p_nh_info_com;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;

    if (sync)
    {
        p_wb_nh_info->info.brguc.dest_gport = p_nh_info->dest_gport;
        p_wb_nh_info->info.brguc.nh_sub_type = p_nh_info->nh_sub_type ;
        p_wb_nh_info->info.brguc.dest_logic_port =  p_nh_info->dest_logic_port ;
	    p_wb_nh_info->info.brguc.service_id =  p_nh_info->service_id ;
        p_wb_nh_info->info.brguc.loop_nhid =  p_nh_info->loop_nhid ;

        if (p_nh_info->loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->loop_nhid)
                {
                    p_wb_nh_info->info.brguc.loop_edit_ptr = p_eloop_node->edit_ptr;
                    break;
                }
            }
        }
    }
    else
    {

        p_nh_info->dest_gport = p_wb_nh_info->info.brguc.dest_gport;
        p_nh_info->nh_sub_type = p_wb_nh_info->info.brguc.nh_sub_type;
        p_nh_info->dest_logic_port = p_wb_nh_info->info.brguc.dest_logic_port;
        p_nh_info->service_id = p_wb_nh_info->info.brguc.service_id;
        p_nh_info->loop_nhid = p_wb_nh_info->info.brguc.loop_nhid;
        if (p_nh_info->loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->loop_nhid)
                {
                    p_eloop_node->ref_cnt++;
                    return CTC_E_NONE;
                }
            }

            p_eloop_node =  mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_eloop_node_t));
            if (NULL == p_eloop_node)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_eloop_node, 0, sizeof(sys_nh_eloop_node_t));
            p_eloop_node->nhid = p_nh_info->loop_nhid;
            p_eloop_node->edit_ptr = p_wb_nh_info->info.brguc.loop_edit_ptr;
            p_eloop_node->ref_cnt++;
            ctc_slist_add_tail(g_usw_nh_master[lchip]->eloop_list, &(p_eloop_node->head));
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, p_eloop_node->edit_ptr);
        }
    }

    return CTC_E_NONE;

}

int32
sys_usw_nh_wb_mapping_special_nhinfo(sys_nh_info_com_t* p_nh_info_com,sys_wb_nh_info_com_t *p_wb_nh_info,uint8 sync)
{
    sys_nh_info_special_t  *p_nh_info = (sys_nh_info_special_t  *)p_nh_info_com;

    if(sync)
    {
        p_wb_nh_info->info.spec.dest_gport = p_nh_info->dest_gport ;
    }
    else
    {
       p_nh_info->dest_gport = p_wb_nh_info->info.spec.dest_gport;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_ipuc_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info, uint8 sync)
{
    sys_nh_info_ipuc_t  *p_nh_info = (sys_nh_info_ipuc_t  *)p_nh_info_com;

    if (sync)
    {
        sal_memcpy(&p_wb_nh_info->info.ipuc.mac_da, &p_nh_info->mac_da[0], sizeof(mac_addr_t));
        p_wb_nh_info->info.ipuc.flag =  p_nh_info->flag;
        p_wb_nh_info->info.ipuc.l3ifid =  p_nh_info->l3ifid;
        p_wb_nh_info->info.ipuc.gport =   p_nh_info->gport ;
        p_wb_nh_info->info.ipuc.arp_id =   p_nh_info->arp_id;
        p_wb_nh_info->info.ipuc.stats_id =  p_nh_info->stats_id;
        if (p_nh_info->protection_path)
        {
            p_wb_nh_info->info.ipuc.protection_arp_id = p_nh_info->protection_path->arp_id;
            p_wb_nh_info->info.ipuc.protection_l3ifid = p_nh_info->protection_path->l3ifid;
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            if (!p_nh_info->arp_id)
            {
                p_wb_nh_info->info.ipuc.l2edit.offset = p_nh_info->l2_edit_ptr;
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit, &p_wb_nh_info->info.ipuc.l2edit,
                    p_nh_info->l2edit_8w, sync));
            }

            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN) && p_nh_info->protection_path->p_dsl2edit)
            {
                p_wb_nh_info->info.ipuc.protect_l2edit.offset = p_nh_info->protection_path->l2_edit_ptr;
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->protection_path->p_dsl2edit,
                    &p_wb_nh_info->info.ipuc.protect_l2edit, 0, sync));
            }
        }
    }
    else
    {
        sys_nh_db_dsl2editeth4w_t l2edit_4w;
        sys_nh_db_dsl2editeth8w_t l2edit_8w;
        sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = &l2edit_4w;
        sys_nh_db_dsl2editeth8w_t* p_l2edit_8w = &l2edit_8w;

        sal_memcpy(&p_nh_info->mac_da[0], &p_wb_nh_info->info.ipuc.mac_da, sizeof(mac_addr_t));
        p_nh_info->flag = p_wb_nh_info->info.ipuc.flag;
        p_nh_info->l3ifid = p_wb_nh_info->info.ipuc.l3ifid;
        p_nh_info->gport = p_wb_nh_info->info.ipuc.gport;
        p_nh_info->arp_id = p_wb_nh_info->info.ipuc.arp_id;
        p_nh_info->stats_id = p_wb_nh_info->info.ipuc.stats_id;
        p_nh_info->l2edit_8w =  p_wb_nh_info->info.ipuc.l2edit.is_8w;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT)&&!p_nh_info->arp_id)
        {
            sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
            sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));

            p_nh_info->l2_edit_ptr = p_wb_nh_info->info.ipuc.l2edit.offset;
            if (!p_nh_info->l2edit_8w)
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_nh_info->info.ipuc.l2edit, 0, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &p_nh_info->l2_edit_ptr));
                p_nh_info->p_dsl2edit = p_l2edit_4w;
            }
            else
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_nh_info->info.ipuc.l2edit, 1, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_l2edit_8w, &p_nh_info->l2_edit_ptr));
                p_nh_info->p_dsl2edit = (sys_nh_db_dsl2editeth4w_t*)p_l2edit_8w;
            }
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
        {
            p_nh_info->protection_path  = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_info_ipuc_edit_info_t));
            if (NULL == p_nh_info->protection_path)
            {
                return CTC_E_NO_MEMORY;
            }

            sal_memset(p_nh_info->protection_path, 0, sizeof(sys_nh_info_ipuc_edit_info_t));
            p_nh_info->protection_path->arp_id = p_wb_nh_info->info.ipuc.protection_arp_id;
            p_nh_info->protection_path->l3ifid = p_wb_nh_info->info.ipuc.protection_l3ifid;
            p_nh_info->protection_path->l2_edit_ptr = p_wb_nh_info->info.ipuc.protect_l2edit.offset;

            if (!p_nh_info->protection_path->arp_id && p_nh_info->protection_path->p_dsl2edit)
            {
                sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_nh_info->info.ipuc.protect_l2edit,
                    0, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &p_nh_info->l2_edit_ptr));
                p_nh_info->protection_path->p_dsl2edit = p_l2edit_4w;
            }
        }

    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_wb_mapping_rspan_nhinfo(sys_nh_info_com_t* p_nh_info_com,sys_wb_nh_info_com_t *p_wb_nh_info,uint8 sync)
{
   sys_nh_info_rspan_t  *p_nh_info = (sys_nh_info_rspan_t  *)p_nh_info_com;
   if(sync)
   {
       p_wb_nh_info->info.rspan.remote_chip = p_nh_info->remote_chip ;
   }
   else
   {
     p_nh_info->remote_chip= p_wb_nh_info->info.rspan.remote_chip;
   }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_ip_tunnel_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info, uint8 sync)
{
    sys_nh_info_ip_tunnel_t  *p_nh_info = (sys_nh_info_ip_tunnel_t  *)p_nh_info_com;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;

    if (sync)
    {
        p_wb_nh_info->info.ip_tunnel.flag =  p_nh_info->flag;
        p_wb_nh_info->info.ip_tunnel.l3ifid =  p_nh_info->l3ifid;
        p_wb_nh_info->info.ip_tunnel.gport =   p_nh_info->gport ;
        p_wb_nh_info->info.ip_tunnel.arp_id =   p_nh_info->arp_id;
        p_wb_nh_info->info.ip_tunnel.ecmp_if_id =  p_nh_info->ecmp_if_id ;
        p_wb_nh_info->info.ip_tunnel.sa_index =  p_nh_info->sa_index ;
        p_wb_nh_info->info.ip_tunnel.dsl3edit_offset =  p_nh_info->dsl3edit_offset ;
        p_wb_nh_info->info.ip_tunnel.loop_nhid =  p_nh_info->loop_nhid ;
        p_wb_nh_info->info.ip_tunnel.dest_vlan_ptr =  p_nh_info->dest_vlan_ptr ;
        p_wb_nh_info->info.ip_tunnel.span_id =  p_nh_info->span_id ;
        p_wb_nh_info->info.ip_tunnel.vn_id =  p_nh_info->vn_id ;
        p_wb_nh_info->info.ip_tunnel.dest_logic_port =  p_nh_info->dest_logic_port ;
        p_wb_nh_info->info.ip_tunnel.inner_l2_edit_offset =  p_nh_info->inner_l2_edit_offset ;
        p_wb_nh_info->info.ip_tunnel.outer_l2_edit_offset =  p_nh_info->outer_l2_edit_offset ;
        p_wb_nh_info->info.ip_tunnel.dot1ae_channel = p_nh_info->dot1ae_channel;
        p_wb_nh_info->info.ip_tunnel.sc_index = p_nh_info->sc_index;
        p_wb_nh_info->info.ip_tunnel.sci_en = p_nh_info->sci_en;
        p_wb_nh_info->info.ip_tunnel.dot1ae_en = p_nh_info->dot1ae_en;
        p_wb_nh_info->info.ip_tunnel.udf_profile_id = p_nh_info->udf_profile_id;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W) ||
                CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT))
            {
                /*nexthop using inner l2_edit 3w*/
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_inner, &p_wb_nh_info->info.ip_tunnel.l2_inner,
                        0, sync));
            }

            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W))
            {
                /*nexthop using inner l2_edit 6w*/
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_inner, &p_wb_nh_info->info.ip_tunnel.l2_inner,
                        1, sync));
            }

            if(CTC_FLAG_ISSET(p_nh_info->flag , SYS_NH_IP_TUNNEL_FLAG_L2EDIT8W) && !p_nh_info->ecmp_if_id)
            {
                /*nexthop using outer l2_edit 8w*/
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_4w, &p_wb_nh_info->info.ip_tunnel.l2_outer,
                        1, sync));
            }
            else
            {
                if (!p_nh_info->arp_id && !p_nh_info->ecmp_if_id&&p_nh_info->p_dsl2edit_4w)
                {
                    /*Using arpm outer l2 edit should manage by arp id*/
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_4w, &p_wb_nh_info->info.ip_tunnel.l2_outer,
                            1, sync));
                }
            }
        }

        if (p_nh_info->loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->loop_nhid)
                {
                    p_wb_nh_info->info.ip_tunnel.loop_edit_ptr = p_eloop_node->edit_ptr;
                    break;
                }
            }
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V4))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(p_nh_info->p_l3edit, &p_wb_nh_info->info.ip_tunnel.l3_edit, 0, sync));
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V6))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(p_nh_info->p_l3edit, &p_wb_nh_info->info.ip_tunnel.l3_edit, 1, sync));
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_NAT_V4) ||
                CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_nat_mapping(p_nh_info->p_l3edit, &p_wb_nh_info->info.ip_tunnel.nat, 0, sync));
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_nat_mapping(p_nh_info->p_l3edit, &p_wb_nh_info->info.ip_tunnel.nat, 1, sync));
            }
        }
    }
    else
    {
        p_nh_info->flag = p_wb_nh_info->info.ip_tunnel.flag;
        p_nh_info->l3ifid = p_wb_nh_info->info.ip_tunnel.l3ifid;
        p_nh_info->gport = p_wb_nh_info->info.ip_tunnel.gport;
        p_nh_info->arp_id = p_wb_nh_info->info.ip_tunnel.arp_id;
        p_nh_info->ecmp_if_id = p_wb_nh_info->info.ip_tunnel.ecmp_if_id;
        p_nh_info->sa_index = p_wb_nh_info->info.ip_tunnel.sa_index;
        p_nh_info->dsl3edit_offset = p_wb_nh_info->info.ip_tunnel.dsl3edit_offset;
        p_nh_info->loop_nhid = p_wb_nh_info->info.ip_tunnel.loop_nhid;
        p_nh_info->dest_vlan_ptr = p_wb_nh_info->info.ip_tunnel.dest_vlan_ptr;
        p_nh_info->span_id = p_wb_nh_info->info.ip_tunnel.span_id;
        p_nh_info->vn_id = p_wb_nh_info->info.ip_tunnel.vn_id;
        p_nh_info->dest_logic_port = p_wb_nh_info->info.ip_tunnel.dest_logic_port;
        p_nh_info->inner_l2_edit_offset = p_wb_nh_info->info.ip_tunnel.inner_l2_edit_offset;
        p_nh_info->outer_l2_edit_offset = p_wb_nh_info->info.ip_tunnel.outer_l2_edit_offset;
        p_nh_info->dot1ae_channel = p_wb_nh_info->info.ip_tunnel.dot1ae_channel;
        p_nh_info->sc_index = p_wb_nh_info->info.ip_tunnel.sc_index;
        p_nh_info->sci_en = p_wb_nh_info->info.ip_tunnel.sci_en;
        p_nh_info->dot1ae_en = p_wb_nh_info->info.ip_tunnel.dot1ae_en;
        p_nh_info->udf_profile_id = p_wb_nh_info->info.ip_tunnel.udf_profile_id;

        if (p_nh_info->udf_profile_id)
        {
            g_usw_nh_master[lchip]->udf_profile_ref_cnt[p_nh_info->udf_profile_id - 1]++;
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W) ||
                CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT))
            {
                sys_nh_db_dsl2editeth4w_t l2edit_4w;
                sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = &l2edit_4w;

                sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_nh_info->info.ip_tunnel.l2_inner, 0, sync));
                if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT))
                {
                    CTC_ERROR_RETURN(sys_usw_nh_add_swap_l2edit_inner(lchip, (sys_nh_db_dsl2editeth4w_t**)&p_l2edit_4w,
                                &p_nh_info->inner_l2_edit_offset));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_3w_inner(lchip, &p_l2edit_4w,
                            SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, &p_nh_info->inner_l2_edit_offset));
                }
                p_nh_info->p_dsl2edit_inner = p_l2edit_4w;
            }

            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W))
            {
                /*nexthop using inner l2_edit 6w*/
                sys_nh_db_dsl2editeth8w_t l2edit_8w;
                sys_nh_db_dsl2editeth8w_t* p_l2edit_8w = &l2edit_8w;

                sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_nh_info->info.ip_tunnel.l2_inner, 1, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_6w_inner(lchip, &p_l2edit_8w, &p_nh_info->inner_l2_edit_offset));
                p_nh_info->p_dsl2edit_inner = p_l2edit_8w;
            }

            if(CTC_FLAG_ISSET(p_wb_nh_info->info.ip_tunnel.flag , SYS_NH_IP_TUNNEL_FLAG_L2EDIT8W)&&!p_nh_info->arp_id)
            {
                /*nexthop using outer l2_edit 8w*/
                sys_nh_db_dsl2editeth8w_t l2edit_8w;
                sys_nh_db_dsl2editeth8w_t* p_l2edit_8w = &l2edit_8w;

                sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_nh_info->info.ip_tunnel.l2_outer, 1, sync));

                if (!p_nh_info->ecmp_if_id)
                {
                    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_l2edit_8w, &p_nh_info->outer_l2_edit_offset));
                    p_nh_info->p_dsl2edit_4w = (sys_nh_db_dsl2editeth4w_t*)p_l2edit_8w;
                }
            }
            else
            {
                sys_nh_db_dsl2editeth4w_t l2edit_4w;
                sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = &l2edit_4w;

                if (!p_nh_info->arp_id && !p_nh_info->ecmp_if_id && !CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_LOOP_NH))
                {
                    sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_nh_info->info.ip_tunnel.l2_outer, 0, sync));
                    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &p_nh_info->outer_l2_edit_offset));
                    p_nh_info->p_dsl2edit_4w = p_l2edit_4w;
                }
            }
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
            sys_dsl3edit_tunnelv4_t tunnel_v4;
            sys_dsl3edit_tunnelv6_t tunnel_v6;
            sys_dsl3edit_nat4w_t  nat_4w;
            sys_dsl3edit_nat8w_t  nat_8w;
            sys_dsl3edit_tunnelv4_t* p_tunnel_v4 = &tunnel_v4;
            sys_dsl3edit_tunnelv6_t* p_tunnel_v6 = &tunnel_v6;
            sys_dsl3edit_nat4w_t* p_nat_4w = &nat_4w;
            sys_dsl3edit_nat8w_t* p_nat_8w = &nat_8w;

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V4))
            {
                sal_memset(&tunnel_v4, 0, sizeof(sys_dsl3edit_tunnelv4_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(&tunnel_v4, &p_wb_nh_info->info.ip_tunnel.l3_edit, 0, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_tunnel_v4, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4,
                        &p_nh_info->dsl3edit_offset));
                p_nh_info->p_l3edit = p_tunnel_v4;
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V6))
            {
                sal_memset(&tunnel_v6, 0, sizeof(sys_dsl3edit_tunnelv6_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(&tunnel_v6, &p_wb_nh_info->info.ip_tunnel.l3_edit, 1, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_tunnel_v6, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6,
                        &p_nh_info->dsl3edit_offset));
                p_nh_info->p_l3edit = p_tunnel_v6;
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_NAT_V4) ||
                CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4))
            {
                sal_memset(&nat_4w, 0, sizeof(sys_dsl3edit_nat4w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_nat_mapping(&nat_4w, &p_wb_nh_info->info.ip_tunnel.nat, 0, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_nat_4w, SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W,
                        &p_nh_info->dsl3edit_offset));
                p_nh_info->p_l3edit = p_nat_4w;
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6))
            {
                sal_memset(&nat_8w, 0, sizeof(sys_dsl3edit_nat8w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_nat_mapping(&nat_8w, &p_wb_nh_info->info.ip_tunnel.nat, 0, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_nat_8w, SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W,
                        &p_nh_info->dsl3edit_offset));
                p_nh_info->p_l3edit = p_nat_8w;
            }
        }

        if (p_nh_info->loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->loop_nhid)
                {
                    p_eloop_node->ref_cnt++;
                    return CTC_E_NONE;
                }
            }

            p_eloop_node =  mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_eloop_node_t));
            if (NULL == p_eloop_node)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_eloop_node, 0, sizeof(sys_nh_eloop_node_t));
            p_eloop_node->nhid = p_nh_info->loop_nhid;
            p_eloop_node->edit_ptr = p_wb_nh_info->info.ip_tunnel.loop_edit_ptr;
            p_eloop_node->ref_cnt++;
            ctc_slist_add_tail(g_usw_nh_master[lchip]->eloop_list, &(p_eloop_node->head));
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, p_eloop_node->edit_ptr);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_wlan_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info, uint8 sync)
{
    sys_nh_info_wlan_tunnel_t  *p_nh_info = (sys_nh_info_wlan_tunnel_t  *)p_nh_info_com;

    if (sync)
    {
        p_wb_nh_info->info.wlan.flag =  p_nh_info->flag;
        p_wb_nh_info->info.wlan.dsl3edit_offset =  p_nh_info->dsl3edit_offset;
        p_wb_nh_info->info.wlan.gport =   p_nh_info->gport ;
        p_wb_nh_info->info.wlan.sa_index =   p_nh_info->sa_index;
        p_wb_nh_info->info.wlan.dest_vlan_ptr =  p_nh_info->dest_vlan_ptr;
        p_wb_nh_info->info.wlan.inner_l2_ptr =  p_nh_info->inner_l2_ptr ;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W))
        {
            /*nexthop using inner l2_edit 3w*/
            _sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_inner, &p_wb_nh_info->info.wlan.l2edit_inner, 0, sync);
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_WLAN_TUNNEL_FLAG_IN_V4))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(p_nh_info->p_dsl3edit_tunnel, &p_wb_nh_info->info.wlan.l3_edit, 0, sync));
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_WLAN_TUNNEL_FLAG_IN_V6))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(p_nh_info->p_dsl3edit_tunnel, &p_wb_nh_info->info.wlan.l3_edit, 1, sync));
            }
        }

    }
    else
    {
        p_nh_info->flag = p_wb_nh_info->info.wlan.flag;
        p_nh_info->dsl3edit_offset = p_wb_nh_info->info.wlan.dsl3edit_offset;
        p_nh_info->gport = p_wb_nh_info->info.wlan.gport;
        p_nh_info->sa_index = p_wb_nh_info->info.wlan.sa_index;
        p_nh_info->dest_vlan_ptr = p_wb_nh_info->info.wlan.dest_vlan_ptr;
        p_nh_info->inner_l2_ptr = p_wb_nh_info->info.wlan.inner_l2_ptr;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_3W))
        {
            /*nexthop using inner l2_edit 3w*/
            sys_nh_db_dsl2editeth4w_t l2edit_4w;
            sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = &l2edit_4w;

            sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
            CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_nh_info->info.wlan.l2edit_inner, 0, sync));
            CTC_ERROR_RETURN(sys_usw_nh_add_l2edit_3w_inner(lchip, &p_l2edit_4w,
                    SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, &p_nh_info->inner_l2_ptr));
            p_nh_info->p_dsl2edit_inner = p_l2edit_4w;
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
            sys_dsl3edit_tunnelv4_t tunnel_v4;
            sys_dsl3edit_tunnelv6_t tunnel_v6;
            sys_dsl3edit_tunnelv4_t* p_tunnel_v4 = &tunnel_v4;
            sys_dsl3edit_tunnelv6_t* p_tunnel_v6 = &tunnel_v6;

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_WLAN_TUNNEL_FLAG_IN_V4))
            {
                sal_memset(&tunnel_v4, 0, sizeof(sys_dsl3edit_tunnelv4_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(&tunnel_v4, &p_wb_nh_info->info.wlan.l3_edit, 0, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_tunnel_v4, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4,
                        &p_nh_info->dsl3edit_offset));
                p_nh_info->p_dsl3edit_tunnel = p_tunnel_v4;
            }

            if (CTC_FLAG_ISSET(p_nh_info->flag, SYS_NH_WLAN_TUNNEL_FLAG_IN_V6))
            {
                sal_memset(&tunnel_v6, 0, sizeof(sys_dsl3edit_tunnelv6_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l3edit_mapping(&tunnel_v6, &p_wb_nh_info->info.wlan.l3_edit, 1, sync));
                CTC_ERROR_RETURN(sys_usw_nh_add_l3edit_tunnel(lchip, (void**)&p_tunnel_v6, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6,
                        &p_nh_info->dsl3edit_offset));
                p_nh_info->p_dsl3edit_tunnel = p_tunnel_v6;
            }
        }

    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_arp(uint8 lchip, sys_nh_db_arp_t* p_db_arp, sys_wb_nh_arp_t *p_wb_arp, uint8 sync)
{

    if (sync)
    {
        sal_memcpy(&p_wb_arp->mac_da, &p_db_arp->mac_da, sizeof(mac_addr_t));
        p_wb_arp->flag                 =  p_db_arp->flag ;
        p_wb_arp->l3if_type            =  p_db_arp->l3if_type;
        p_wb_arp->offset               =  p_db_arp->offset ;
        p_wb_arp->in_l2_offset         =  p_db_arp->in_l2_offset;
        p_wb_arp->output_vid           =  p_db_arp->output_vid ;
        p_wb_arp->output_vlan_is_svlan =  p_db_arp->output_vlan_is_svlan;
        p_wb_arp->gport                =  p_db_arp->gport ;
        p_wb_arp->nh_id                =  p_db_arp->nh_id;
        p_wb_arp->destmap_profile      =  p_db_arp->destmap_profile;
        p_wb_arp->arp_id               =  p_db_arp->arp_id;
        p_wb_arp->ref_cnt              =  p_db_arp->ref_cnt;
        p_wb_arp->l3if_id              =  p_db_arp->l3if_id;
    }
    else
    {
        sal_memcpy(&p_db_arp->mac_da, &p_wb_arp->mac_da, sizeof(mac_addr_t));
        p_db_arp->flag                 = p_wb_arp->flag;
        p_db_arp->l3if_type            = p_wb_arp->l3if_type ;
        p_db_arp->offset               = p_wb_arp->offset ;
        p_db_arp->in_l2_offset         = p_wb_arp->in_l2_offset ;
        p_db_arp->output_vid           = p_wb_arp->output_vid ;
        p_db_arp->output_vlan_is_svlan = p_wb_arp->output_vlan_is_svlan;
        p_db_arp->gport                = p_wb_arp->gport;
        p_db_arp->nh_id                = p_wb_arp->nh_id;
        p_db_arp->destmap_profile      = p_wb_arp->destmap_profile;
        p_db_arp->arp_id               = p_wb_arp->arp_id;
        p_db_arp->ref_cnt              = p_wb_arp->ref_cnt;
        p_db_arp->l3if_id              = p_wb_arp->l3if_id;

        if (p_db_arp->in_l2_offset)
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1, p_db_arp->in_l2_offset);
        }

        if (p_db_arp->offset)
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_db_arp->offset);
        }

        if(p_db_arp->destmap_profile)
        {
            /*recover opf*/
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE, 1, p_db_arp->destmap_profile);
        }
    }
    return CTC_E_NONE;
}


int32
sys_usw_nh_wb_mapping_mpls_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info, uint8 sync)
{
    sys_nh_info_mpls_t  *p_nh_info = (sys_nh_info_mpls_t*)p_nh_info_com;
    sys_nh_info_mpls_edit_info_t* protection_path = NULL;
    int32 ret = 0;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;

    if (sync)
    {
        p_wb_nh_info->info.mpls.aps_group_id = p_nh_info->aps_group_id;
        p_wb_nh_info->info.mpls.gport = p_nh_info->gport;
        p_wb_nh_info->info.mpls.l3ifid = p_nh_info->l3ifid;
        p_wb_nh_info->info.mpls.dest_logic_port = p_nh_info->dest_logic_port;
        p_wb_nh_info->info.mpls.service_id = p_nh_info->service_id;

        p_wb_nh_info->info.mpls.cw_index = p_nh_info->cw_index;
        p_wb_nh_info->info.mpls.arp_id = p_nh_info->arp_id;
        p_wb_nh_info->info.mpls.inner_l2_ptr = p_nh_info->inner_l2_ptr;
        p_wb_nh_info->info.mpls.outer_l2_ptr = p_nh_info->outer_l2_ptr;
        p_wb_nh_info->info.mpls.dsl3edit_offset = p_nh_info->working_path.dsl3edit_offset;
        p_wb_nh_info->info.mpls.ma_idx = p_nh_info->ma_idx;
        p_wb_nh_info->info.mpls.dsma_valid = p_nh_info->dsma_valid;
        p_wb_nh_info->info.mpls.loop_nhid =  p_nh_info->working_path.loop_nhid;

        if (p_nh_info->working_path.loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->working_path.loop_nhid)
                {
                    p_wb_nh_info->info.mpls.loop_edit_ptr = p_eloop_node->edit_ptr;
                    break;
                }
            }
        }

        if (p_nh_info->working_path.p_mpls_tunnel)
        {
            p_wb_nh_info->info.mpls.tunnel_id = p_nh_info->working_path.p_mpls_tunnel->tunnel_id;
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN) && p_nh_info->protection_path)
        {
            if (p_nh_info->protection_path->p_mpls_tunnel)
            {
                p_wb_nh_info->info.mpls.p_tunnel_id = p_nh_info->protection_path->p_mpls_tunnel->tunnel_id;
            }
            p_wb_nh_info->info.mpls.p_dsl3edit_offset = p_nh_info->protection_path->dsl3edit_offset;
            p_wb_nh_info->info.mpls.aps_enable  = 1;
            if (p_nh_info->protection_path->loop_nhid)
            {
                p_wb_nh_info->info.mpls.p_loop_nhid =  p_nh_info->protection_path->loop_nhid;
                CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
                {
                    p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                    if (p_eloop_node->nhid == p_nh_info->protection_path->loop_nhid)
                    {
                        p_wb_nh_info->info.mpls.p_loop_edit_ptr = p_eloop_node->edit_ptr;
                        break;
                    }
                }
            }
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            /*nexthop using outer l2_edit 3w for mpls pop*/
            CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit, &p_wb_nh_info->info.mpls.l2edit_outer,
                    0, sync));
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W))
        {
            /*nexthop using inner l2_edit 6w*/
            CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_inner, &p_wb_nh_info->info.mpls.l2edit_inner,
                    1, sync));
        }

    }
    else
    {
        sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;

        p_nh_info->aps_group_id = p_wb_nh_info->info.mpls.aps_group_id;
        p_nh_info->gport = p_wb_nh_info->info.mpls.gport;
        p_nh_info->l3ifid = p_wb_nh_info->info.mpls.l3ifid;
        p_nh_info->dest_logic_port = p_wb_nh_info->info.mpls.dest_logic_port;
        p_nh_info->service_id = p_wb_nh_info->info.mpls.service_id;
        p_nh_info->cw_index = p_wb_nh_info->info.mpls.cw_index;
        p_nh_info->arp_id = p_wb_nh_info->info.mpls.arp_id;
        p_nh_info->inner_l2_ptr = p_wb_nh_info->info.mpls.inner_l2_ptr;
        p_nh_info->working_path.dsl3edit_offset = p_wb_nh_info->info.mpls.dsl3edit_offset;
        p_nh_info->outer_l2_ptr = p_wb_nh_info->info.mpls.outer_l2_ptr;
        p_nh_info->nhid = p_wb_nh_info->nh_id;
        p_nh_info->ma_idx = p_wb_nh_info->info.mpls.ma_idx;
        p_nh_info->dsma_valid = p_wb_nh_info->info.mpls.dsma_valid;
        p_nh_info->working_path.loop_nhid = p_wb_nh_info->info.mpls.loop_nhid;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT)&&p_wb_nh_info->info.mpls.dsl3edit_offset)
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_wb_nh_info->info.mpls.dsl3edit_offset);
        }

        /*restore mpls tunnel id*/
        CTC_ERROR_RETURN(sys_usw_nh_lkup_mpls_tunnel(lchip, p_wb_nh_info->info.mpls.tunnel_id, &p_mpls_tunnel));
        p_nh_info->working_path.p_mpls_tunnel = p_mpls_tunnel;
        if (p_mpls_tunnel)
        {
            CTC_ERROR_RETURN(_sys_usw_nh_update_tunnel_ref_info(lchip, p_nh_info, p_wb_nh_info->info.mpls.tunnel_id, 1));
        }
        if (p_wb_nh_info->info.mpls.aps_enable)
        {
            protection_path = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_info_mpls_edit_info_t));
            if (NULL == protection_path)
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    			return CTC_E_NO_MEMORY;
            }

            p_nh_info->protection_path = protection_path;
            p_nh_info->protection_path->loop_nhid = p_wb_nh_info->info.mpls.p_loop_nhid;
            p_nh_info->protection_path->dsl3edit_offset =p_wb_nh_info->info.mpls.p_dsl3edit_offset;
            CTC_ERROR_GOTO(sys_usw_nh_lkup_mpls_tunnel(lchip, p_wb_nh_info->info.mpls.p_tunnel_id, &p_mpls_tunnel), ret, error);
            if (p_mpls_tunnel)
            {
                p_nh_info->protection_path->p_mpls_tunnel = p_mpls_tunnel;
                CTC_ERROR_RETURN(_sys_usw_nh_update_tunnel_ref_info(lchip, p_nh_info, p_wb_nh_info->info.mpls.p_tunnel_id, 1));
            }

            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT)&&p_wb_nh_info->info.mpls.p_dsl3edit_offset)
            {
                sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_wb_nh_info->info.mpls.p_dsl3edit_offset);
            }
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            /*nexthop using outer l2_edit 3w for mpls pop*/
            sys_nh_db_dsl2editeth4w_t l2edit;
            sys_nh_db_dsl2editeth4w_t* p_l2edit = &l2edit;
            CTC_ERROR_GOTO(_sys_usw_nh_wb_l2edit_mapping(&l2edit, &p_wb_nh_info->info.mpls.l2edit_outer,0, sync), ret, error);
            CTC_ERROR_GOTO(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit, &p_nh_info->outer_l2_ptr), ret, error);
            p_nh_info->p_dsl2edit = p_l2edit;
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDI_6W))
        {
            /*nexthop using inner l2_edit 3w, outer l2 edit should process in mpls tunnel*/
            sys_nh_db_dsl2editeth8w_t l2edit;
            sys_nh_db_dsl2editeth8w_t* p_l2edit = &l2edit;

            sal_memset(&l2edit, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
            CTC_ERROR_GOTO(_sys_usw_nh_wb_l2edit_mapping(&l2edit, &p_wb_nh_info->info.mpls.l2edit_inner, 1, sync), ret, error);
            CTC_ERROR_GOTO(sys_usw_nh_add_l2edit_6w_inner(lchip, &p_l2edit, &p_nh_info->inner_l2_ptr), ret, error);
            p_nh_info->p_dsl2edit_inner = p_l2edit;
        }

        if (p_nh_info->working_path.loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->working_path.loop_nhid)
                {
                    p_eloop_node->ref_cnt++;
                    return CTC_E_NONE;
                }
            }

            p_eloop_node =  mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_eloop_node_t));
            if (NULL == p_eloop_node)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_eloop_node, 0, sizeof(sys_nh_eloop_node_t));
            p_eloop_node->nhid = p_nh_info->working_path.loop_nhid;
            p_eloop_node->edit_ptr = p_wb_nh_info->info.mpls.loop_edit_ptr;
            p_eloop_node->ref_cnt++;
            ctc_slist_add_tail(g_usw_nh_master[lchip]->eloop_list, &(p_eloop_node->head));
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, p_eloop_node->edit_ptr);
        }

        if (p_nh_info->protection_path && p_nh_info->protection_path->loop_nhid)
        {
            CTC_SLIST_LOOP(g_usw_nh_master[lchip]->eloop_list, ctc_slistnode)
            {
                p_eloop_node = _ctc_container_of(ctc_slistnode, sys_nh_eloop_node_t, head);
                if (p_eloop_node->nhid == p_nh_info->protection_path->loop_nhid)
                {
                    p_eloop_node->ref_cnt++;
                    return CTC_E_NONE;
                }
            }
            p_eloop_node =  mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_eloop_node_t));
            if (NULL == p_eloop_node)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_eloop_node, 0, sizeof(sys_nh_eloop_node_t));
            p_eloop_node->nhid = p_nh_info->protection_path->loop_nhid;
            p_eloop_node->edit_ptr = p_wb_nh_info->info.mpls.p_loop_edit_ptr;
            p_eloop_node->ref_cnt++;
            ctc_slist_add_tail(g_usw_nh_master[lchip]->eloop_list, &(p_eloop_node->head));
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_LPBK, 1, p_eloop_node->edit_ptr);
        }
    }

    return CTC_E_NONE;
error:
    if (protection_path)
    {
        mem_free(protection_path);
    }
    return ret;

}

int32
_sys_usw_nh_wb_mapping_mpls_tunnel(uint8 lchip, sys_nh_db_mpls_tunnel_t* p_db_mpls,
            sys_wb_nh_mpls_tunnel_t *p_wb_mpls, uint8 sync)
{
    uint16 loop1 = 0;
    uint16 loop2 = 0;
    uint8 is_8w = 0;
    uint16 loop_num = 0;

    if (sync)
    {
        p_wb_mpls->tunnel_id = p_db_mpls->tunnel_id;
        p_wb_mpls->gport = p_db_mpls->gport;
        p_wb_mpls->l3ifid = p_db_mpls->l3ifid;
        p_wb_mpls->flag = p_db_mpls->flag;
        p_wb_mpls->label_num = p_db_mpls->label_num;
        p_wb_mpls->stats_ptr = p_db_mpls->stats_ptr;
        p_wb_mpls->p_stats_ptr = p_db_mpls->p_stats_ptr;
        for (loop1 = 0; loop1 < SYS_NH_APS_M; loop1++)
        {
            p_wb_mpls->sr_loop_num[loop1] = p_db_mpls->sr_loop_num[loop1];
            p_wb_mpls->lsp_offset[loop1] = p_db_mpls->lsp_offset[loop1];
            if (p_db_mpls->sr[loop1])
            {
                sal_memcpy(p_wb_mpls->sr[loop1], p_db_mpls->sr[loop1], sizeof(sys_nh_mpls_sr_t)*p_db_mpls->sr_loop_num[loop1]);
            }
            for (loop2 = 0; loop2 < SYS_NH_APS_M; loop2++)
            {
                p_wb_mpls->l2edit_offset[loop1][loop2] = p_db_mpls->l2edit_offset[loop1][loop2];
                p_wb_mpls->spme_offset[loop1][loop2] = p_db_mpls->spme_offset[loop1][loop2];
                p_wb_mpls->arp_id[loop1][loop2] = p_db_mpls->arp_id[loop1][loop2];
                if (p_db_mpls->p_dsl2edit_4w[loop1][loop2])
                {
                    is_8w = CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W)?1:0;
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_db_mpls->p_dsl2edit_4w[loop1][loop2],
                        &p_wb_mpls->l2edit[loop1][loop2], is_8w, sync));
                }
            }
        }
    }
    else
    {
        p_db_mpls->tunnel_id = p_wb_mpls->tunnel_id;
        p_db_mpls->gport = p_wb_mpls->gport;
        p_db_mpls->l3ifid =  p_wb_mpls->l3ifid;
        p_db_mpls->flag  = p_wb_mpls->flag;
        p_db_mpls->label_num  = p_wb_mpls->label_num;
        p_db_mpls->stats_ptr = p_wb_mpls->stats_ptr;
        p_db_mpls->p_stats_ptr = p_wb_mpls->p_stats_ptr;
        for (loop1 = 0; loop1 < SYS_NH_APS_M; loop1++)
        {
            p_db_mpls->lsp_offset[loop1] = p_wb_mpls->lsp_offset[loop1];
            if (p_db_mpls->lsp_offset[loop1])
            {
                if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME))
                {
                    sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, p_db_mpls->lsp_offset[loop1]);
                }
                else
                {
                    if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR) && !DRV_IS_DUET2(lchip))
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W, 1, p_db_mpls->lsp_offset[loop1]);
                    }
                    else
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_db_mpls->lsp_offset[loop1]);
                    }
                }
            }
            for (loop2 = 0; loop2 < SYS_NH_APS_M; loop2++)
            {
                p_db_mpls->l2edit_offset[loop1][loop2] = p_wb_mpls->l2edit_offset[loop1][loop2];
                p_db_mpls->spme_offset[loop1][loop2] = p_wb_mpls->spme_offset[loop1][loop2];
                p_db_mpls->arp_id[loop1][loop2] = p_wb_mpls->arp_id[loop1][loop2];
                if (p_db_mpls->spme_offset[loop1][loop2])
                {
                    sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, p_db_mpls->spme_offset[loop1][loop2]);
                }
            }
        }

        is_8w = CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W)?1:0;

        if (!is_8w)
        {
            sys_nh_db_dsl2editeth4w_t l2edit_4w;
            sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = &l2edit_4w;
            uint32 l2edit_offset = 0;

            /*process one l2editptr first*/
            if (!p_db_mpls->arp_id[0][0])
            {
                sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_mpls->l2edit[0][0], 0, sync));
                l2edit_offset = p_db_mpls->l2edit_offset[0][0];
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &l2edit_offset));
                p_db_mpls->p_dsl2edit_4w[0][0] = p_l2edit_4w;
            }

            if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS) && !p_db_mpls->arp_id[1][0])
            {
                /*process lsp aps protect*/
                sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                p_l2edit_4w = &l2edit_4w;
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_mpls->l2edit[1][0], 0, sync));
                l2edit_offset = p_db_mpls->l2edit_offset[1][0];
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &l2edit_offset));
                p_db_mpls->p_dsl2edit_4w[1][0] = p_l2edit_4w;
            }

            if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
            {
                if (!p_db_mpls->arp_id[0][1])
                {
                    sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                    p_l2edit_4w = &l2edit_4w;
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_mpls->l2edit[0][1], 0, sync));
                    l2edit_offset = p_db_mpls->l2edit_offset[0][1];
                    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &l2edit_offset));
                    p_db_mpls->p_dsl2edit_4w[0][1] = p_l2edit_4w;
                }

                if (!p_db_mpls->arp_id[1][1])
                {
                    sal_memset(&l2edit_4w, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
                    p_l2edit_4w = &l2edit_4w;
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_4w, &p_wb_mpls->l2edit[1][1], 0, sync));
                    l2edit_offset = p_db_mpls->l2edit_offset[1][1];
                    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit_4w, &l2edit_offset));
                    p_db_mpls->p_dsl2edit_4w[1][1] = p_l2edit_4w;
                }

            }
        }
        else
        {
            sys_nh_db_dsl2editeth8w_t l2edit_8w;
            sys_nh_db_dsl2editeth8w_t* p_l2edit_8w = &l2edit_8w;
            uint32 l2edit_offset = 0;

            /*process one l2editptr first*/
            if (!p_db_mpls->arp_id[0][0])
            {
                sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_mpls->l2edit[0][0], 0, sync));
                l2edit_offset = p_db_mpls->l2edit_offset[0][0];
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_l2edit_8w, &l2edit_offset));
                p_db_mpls->p_dsl2edit_4w[0][0] = (sys_nh_db_dsl2editeth4w_t*)p_l2edit_8w;
            }

            if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS) && !p_db_mpls->arp_id[1][0])
            {
                /*process lsp aps protect*/
                sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                p_l2edit_8w = &l2edit_8w;
                CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_mpls->l2edit[1][0], 0, sync));
                l2edit_offset = p_db_mpls->l2edit_offset[1][0];
                CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_l2edit_8w, &l2edit_offset));
                p_db_mpls->p_dsl2edit_4w[1][0] = (sys_nh_db_dsl2editeth4w_t*)p_l2edit_8w;
            }

            if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS))
            {
                if (!p_db_mpls->arp_id[0][1])
                {
                    sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                    p_l2edit_8w = &l2edit_8w;
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_mpls->l2edit[0][1], 0, sync));
                    l2edit_offset = p_db_mpls->l2edit_offset[0][1];
                    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_l2edit_8w, &l2edit_offset));
                    p_db_mpls->p_dsl2edit_4w[0][1] = (sys_nh_db_dsl2editeth4w_t*)p_l2edit_8w;
                }

                if (!p_db_mpls->arp_id[1][1])
                {
                    sal_memset(&l2edit_8w, 0, sizeof(sys_nh_db_dsl2editeth8w_t));
                    p_l2edit_8w = &l2edit_8w;
                    CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit_8w, &p_wb_mpls->l2edit[1][1], 0, sync));
                    l2edit_offset = p_db_mpls->l2edit_offset[1][1];
                    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_8w_outer(lchip, &p_l2edit_8w, &l2edit_offset));
                    p_db_mpls->p_dsl2edit_4w[1][1] = (sys_nh_db_dsl2editeth4w_t*)p_l2edit_8w;
                }
            }
        }

        if (CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_IS_SR))
        {
            loop_num = CTC_FLAG_ISSET(p_db_mpls->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS)?2:1;
        }

        for (loop1 = 0; loop1 < loop_num; loop1++)
        {
            p_db_mpls->sr_loop_num[loop1] = p_wb_mpls->sr_loop_num[loop1];
            if (p_db_mpls->sr_loop_num[loop1])
            {
                sys_nh_mpls_sr_t* p_sr = NULL;
                p_db_mpls->sr[loop1] = (sys_nh_mpls_sr_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_mpls_sr_t) *p_db_mpls->sr_loop_num[loop1]);
                if (!p_db_mpls->sr[loop1])
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                    return CTC_E_NO_MEMORY;
                }
                sal_memset(p_db_mpls->sr[loop1], 0, sizeof(sys_nh_mpls_sr_t) *p_db_mpls->sr_loop_num[loop1]);
                sal_memcpy(p_db_mpls->sr[loop1], p_wb_mpls->sr[loop1], sizeof(sys_nh_mpls_sr_t)*p_db_mpls->sr_loop_num[loop1]);

                /*recover sr tunnel l3edit info*/
                for (loop2 = 0; loop2 < p_db_mpls->sr_loop_num[loop1]; loop2++)
                {
                    p_sr = p_db_mpls->sr[loop1] + loop2;
                    if (p_sr->dsnh_offset)
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1, p_sr->dsnh_offset);
                    }
                    if (p_sr->pw_offset && DRV_IS_DUET2(lchip))
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_sr->pw_offset);
                    }
                    if (p_sr->lsp_offset)
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 1, p_sr->lsp_offset);
                    }
                    if (p_sr->spme_offset)
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_SPME, 1, p_sr->spme_offset);
                    }
                    if (p_sr->l2edit_offset)
                    {
                        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 1, p_sr->l2edit_offset);
                    }
                }

                if (!DRV_IS_DUET2(lchip) && p_sr->pw_offset)
                {
                    sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W, 1, p_sr->pw_offset);
                }
            }
        }
    }
    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_mapping_trill_nhinfo(uint8 lchip, sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info, uint8 sync)
{
    sys_nh_info_trill_t  *p_nh_info = (sys_nh_info_trill_t*)p_nh_info_com;

    if (sync)
    {
        p_wb_nh_info->info.trill.ingress_nickname = p_nh_info->ingress_nickname;
        p_wb_nh_info->info.trill.egress_nickname = p_nh_info->egress_nickname;
        p_wb_nh_info->info.trill.l3ifid = p_nh_info->l3ifid;
        p_wb_nh_info->info.trill.gport = p_nh_info->gport;
        p_wb_nh_info->info.trill.dsl3edit_offset = p_nh_info->dsl3edit_offset;
        p_wb_nh_info->info.trill.l2_edit_ptr = p_nh_info->l2_edit_ptr;
        p_wb_nh_info->info.trill.dest_vlan_ptr = p_nh_info->dest_vlan_ptr;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            /*nexthop using outer l2_edit*/
            CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(p_nh_info->p_dsl2edit_4w, &p_wb_nh_info->info.trill.l2edit_key,
                    0, sync));
        }
    }
    else
    {

        p_nh_info->ingress_nickname = p_wb_nh_info->info.trill.ingress_nickname;
        p_nh_info->egress_nickname = p_wb_nh_info->info.trill.egress_nickname;
        p_nh_info->l3ifid = p_wb_nh_info->info.trill.l3ifid;
        p_nh_info->gport = p_wb_nh_info->info.trill.gport;
        p_nh_info->dsl3edit_offset = p_wb_nh_info->info.trill.dsl3edit_offset;
        p_nh_info->l2_edit_ptr = p_wb_nh_info->info.trill.l2_edit_ptr;
        p_nh_info->dest_vlan_ptr = p_wb_nh_info->info.trill.dest_vlan_ptr;

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_TRILL, 1, p_wb_nh_info->info.trill.dsl3edit_offset);
        }

        if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {
            /*nexthop using outer l2_edit 3w for mpls pop*/
            sys_nh_db_dsl2editeth4w_t l2edit;
            sys_nh_db_dsl2editeth4w_t* p_l2edit = &l2edit;

            sal_memset(&l2edit, 0, sizeof(sys_nh_db_dsl2editeth4w_t));
            CTC_ERROR_RETURN(_sys_usw_nh_wb_l2edit_mapping(&l2edit, &p_wb_nh_info->info.trill.l2edit_key,0, sync));
            CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, &p_l2edit, &p_nh_info->l2_edit_ptr));
            p_nh_info->p_dsl2edit_4w = p_l2edit;
        }
    }

    return CTC_E_NONE;

}

int32
_sys_usw_nh_wb_restore_nhinfo(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{
    sys_wb_nh_info_com_t wb_com;
    sys_wb_nh_info_com_t *p_wb_nh_info = &wb_com;
    sys_nh_info_com_t* p_nhinfo = NULL;
    uint32 db_entry_size = 0;
    uint16 entry_cnt = 0;
    int ret = 0;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = OPF_NH_NHID_INTERNAL;
    opf.multiple  = 1;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_info_com_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_INFO_COM);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_com, 0, sizeof(sys_wb_nh_info_com_t));

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8*)p_wb_nh_info, (uint8*)(p_wb_query->buffer) + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        p_wb_query->key_len + p_wb_query->data_len);
    entry_cnt++;
    _sys_usw_nh_get_nhentry_size_by_nhtype(lchip, p_wb_nh_info->nh_type, &db_entry_size);
    p_nhinfo = mem_malloc(MEM_NEXTHOP_MODULE, db_entry_size);
    if (NULL == p_nhinfo)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    sal_memset(p_nhinfo, 0, db_entry_size);

    p_nhinfo->hdr.adjust_len = p_wb_nh_info->hdr.adjust_len;
    p_nhinfo->hdr.dsnh_entry_num = p_wb_nh_info->hdr.dsnh_entry_num;
    p_nhinfo->hdr.nh_entry_type = p_wb_nh_info->nh_type;
    p_nhinfo->hdr.nh_entry_flags = p_wb_nh_info->hdr.nh_entry_flags;

    p_nhinfo->hdr.dsfwd_info.dsfwd_offset = p_wb_nh_info->hdr.dsfwd_offset;
    p_nhinfo->hdr.dsfwd_info.stats_ptr = p_wb_nh_info->hdr.stats_ptr;
    p_nhinfo->hdr.dsfwd_info.dsnh_offset = p_wb_nh_info->hdr.dsnh_offset;
    p_nhinfo->hdr.dsfwd_info.dest_chip = p_wb_nh_info->hdr.dest_chip;
    p_nhinfo->hdr.dsfwd_info.dest_id = p_wb_nh_info->hdr.dest_id;
    p_nhinfo->hdr.nh_id = p_wb_nh_info->nh_id;

    if (p_nhinfo->hdr.dsfwd_info.dsnh_offset < g_usw_nh_master[lchip]->max_glb_nh_offset)
    {
        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
        {
            sys_usw_nh_set_glb_nh_offset(lchip, p_nhinfo->hdr.dsfwd_info.dsnh_offset, p_wb_nh_info->hdr.dsnh_entry_num*2 , TRUE);
        }
        else
        {
            sys_usw_nh_set_glb_nh_offset(lchip, p_nhinfo->hdr.dsfwd_info.dsnh_offset, p_wb_nh_info->hdr.dsnh_entry_num , TRUE);

        }
    }
    else if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH))
    {
        if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, p_nhinfo->hdr.dsnh_entry_num, p_nhinfo->hdr.dsfwd_info.dsnh_offset);
        }
        else
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, p_nhinfo->hdr.dsnh_entry_num, p_nhinfo->hdr.dsfwd_info.dsnh_offset);
        }
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD,
                             (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_DSFWD_IS_6W)?2:1),
                             p_nhinfo->hdr.dsfwd_info.dsfwd_offset);
    }
    switch(p_wb_nh_info->nh_type)
    {
    case SYS_NH_TYPE_IPUC:
        sys_usw_nh_wb_mapping_ipuc_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_TOCPU:
    case SYS_NH_TYPE_DROP:
    case SYS_NH_TYPE_UNROV:
    case SYS_NH_TYPE_ILOOP:
        sys_usw_nh_wb_mapping_special_nhinfo(p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_MISC:
        sys_usw_nh_wb_mapping_misc_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_BRGUC:
        sys_usw_nh_wb_mapping_brguc_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_RSPAN:
        sys_usw_nh_wb_mapping_rspan_nhinfo(p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_IP_TUNNEL:
        sys_usw_nh_wb_mapping_ip_tunnel_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_WLAN_TUNNEL:
        sys_usw_nh_wb_mapping_wlan_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_MPLS:
        sys_usw_nh_wb_mapping_mpls_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_TRILL:
        sys_usw_nh_wb_mapping_trill_nhinfo(lchip, p_nhinfo, p_wb_nh_info, 0);
        break;
    case SYS_NH_TYPE_MCAST:
        {
            sys_nh_info_mcast_t* p_mcast_db;
            p_mcast_db = (sys_nh_info_mcast_t*)p_nhinfo;
            sal_printf("nhid:%u \n", p_wb_nh_info->nh_id);
            if (CTC_FLAG_ISSET(p_mcast_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_MCAST_PROFILE))
            {
                _sys_usw_nh_wb_restore_mcast(lchip, p_wb_query);
            }
        }
        break;
    default:
        break;
    }

    ctc_hash_insert(g_usw_nh_master[lchip]->nhid_hash, p_nhinfo);
    if (p_wb_nh_info->nh_id >=  g_usw_nh_master[lchip]->max_external_nhid)
    {
        sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_wb_nh_info->nh_id);
        g_usw_nh_master[lchip]->internal_nh_used_num++;
    }

    g_usw_nh_master[lchip]->nhid_used_cnt[p_wb_nh_info->nh_type]++;
    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    done:
    return ret;
}
extern int32
sys_usw_nh_wb_restore_ecmp_nhinfo(uint8 lchip,  sys_nh_info_com_t* p_nh_info_com,sys_wb_nh_info_com_t *p_wb_nh_info);

int32
_sys_usw_nh_wb_restore_ecmp(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{
    sys_wb_nh_info_com_t     wb_nh_info;
    sys_wb_nh_info_com_t     *p_wb_nh_info = &wb_nh_info;
    sys_nh_info_com_t* p_nhinfo = NULL;
    uint32 db_entry_size = 0;
    uint16 entry_cnt = 0;
    int ret = 0;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_info_com_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_ECMP);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_nh_info, 0, sizeof(sys_wb_nh_info_com_t));
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8 *)p_wb_nh_info, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    _sys_usw_nh_get_nhentry_size_by_nhtype(lchip, p_wb_nh_info->nh_type, &db_entry_size);
    p_nhinfo = mem_malloc(MEM_NEXTHOP_MODULE, db_entry_size);
    if (NULL == p_nhinfo)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_nhinfo, 0, db_entry_size);

    p_nhinfo->hdr.adjust_len = p_wb_nh_info->hdr.adjust_len;
    p_nhinfo->hdr.dsnh_entry_num = p_wb_nh_info->hdr.dsnh_entry_num;
    p_nhinfo->hdr.nh_entry_type = p_wb_nh_info->nh_type;
    p_nhinfo->hdr.nh_entry_flags = p_wb_nh_info->hdr.nh_entry_flags;

    p_nhinfo->hdr.dsfwd_info.dsfwd_offset = p_wb_nh_info->hdr.dsfwd_offset;
    p_nhinfo->hdr.dsfwd_info.stats_ptr = p_wb_nh_info->hdr.stats_ptr;
    p_nhinfo->hdr.dsfwd_info.dsnh_offset = p_wb_nh_info->hdr.dsnh_offset;
    p_nhinfo->hdr.dsfwd_info.dest_chip = p_wb_nh_info->hdr.dest_chip;
    p_nhinfo->hdr.dsfwd_info.dest_id = p_wb_nh_info->hdr.dest_id;
    p_nhinfo->hdr.nh_id = p_wb_nh_info->nh_id;

    sys_usw_nh_wb_restore_ecmp_nhinfo(lchip, p_nhinfo, p_wb_nh_info);

    ctc_hash_insert(g_usw_nh_master[lchip]->nhid_hash, p_nhinfo);
    g_usw_nh_master[lchip]->nhid_used_cnt[p_wb_nh_info->nh_type]++;

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    done:
    return ret;
}

int32
_sys_usw_nh_wb_restore_mcast_member(uint8 lchip, sys_nh_info_mcast_t* p_mcast_info,
	                                                         sys_wb_nh_info_mcast_t *p_wb_mcast_info,
	                                                         sys_nh_mcast_meminfo_t* p_member_info,
	                                                         uint16 loop)
{
    /*member info*/
    p_member_info->dsmet.flag = p_wb_mcast_info->flag;
    p_member_info->dsmet.logic_port = p_wb_mcast_info->logic_port;
    p_member_info->dsmet.dsmet_offset = p_wb_mcast_info->dsmet_offset;
    p_member_info->dsmet.next_dsmet_offset = p_wb_mcast_info->next_dsmet_offset;

   if (CTC_FLAG_ISSET(p_wb_mcast_info->flag, SYS_NH_DSMET_FLAG_USE_PBM))
   {
   }
    p_member_info->dsmet.dsnh_offset = p_wb_mcast_info->dsnh_offset;
    p_member_info->dsmet.ref_nhid = (p_wb_mcast_info->ref_nhid|(p_wb_mcast_info->ref_nhid_hi<<16));
    p_member_info->dsmet.ucastid = p_wb_mcast_info->ucastid;
    p_member_info->dsmet.replicate_num = p_wb_mcast_info->replicate_num;
	sal_memcpy(p_member_info->dsmet.pbm,p_wb_mcast_info->pbm,4*sizeof(uint32));

    p_member_info->dsmet.free_dsnh_offset_cnt = p_wb_mcast_info->free_dsnh_offset_cnt;
    p_member_info->dsmet.member_type = p_wb_mcast_info->member_type  ;
    p_member_info->dsmet.port_type = p_wb_mcast_info->port_type  ;
    p_member_info->dsmet.entry_type = p_wb_mcast_info->entry_type  ;
    if (p_wb_mcast_info->member_type == SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP)
    {
      p_member_info->dsmet.vid_list[loop] = p_wb_mcast_info->vid;
	  p_member_info->dsmet.vid =p_member_info->dsmet.vid_list[0];
    }
	else
	{
	   p_member_info->dsmet.vid = p_wb_mcast_info->vid;
	}

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_restore_mcast(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{
    sys_wb_nh_info_mcast_t wb_mcast_info;
    sys_wb_nh_info_mcast_t *p_wb_mcast_info = &wb_mcast_info;
    sys_nh_info_mcast_t    *p_mcast_info = NULL;
    sys_nh_mcast_meminfo_t	*p_member_info = NULL;
    uint32 last_met_offset = 0;
    uint32 last_nhid = 0;
    sys_usw_opf_t opf;
    uint16 loop = 0;
    uint16 entry_cnt = 0;
	uint16 physical_replication_loop = 0;
	uint8 add_member_flag = 0;
	uint8 entry_num = 0;
    int ret = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_nh_master[lchip]->nh_opf_type;
    opf.pool_index = OPF_NH_NHID_INTERNAL;
    opf.multiple  = 1;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_info_mcast_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_INFO_MCAST);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_mcast_info, 0, sizeof(sys_wb_nh_info_mcast_t));
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8 *)p_wb_mcast_info, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    if (last_nhid != p_wb_mcast_info->nh_id)
    {
        p_mcast_info = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_info_mcast_t));
        if (NULL == p_mcast_info)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }
        ctc_list_pointer_init(&p_mcast_info->p_mem_list);
    	if (CTC_FLAG_ISSET(p_wb_mcast_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
        {
            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD,
                                                         1, p_wb_mcast_info->hdr.dsfwd_offset);
        }
	    physical_replication_loop = 0;
        add_member_flag = 0;
        p_member_info = NULL;
    }

    if ((p_wb_mcast_info->physical_replication_num != 0)
		&& (last_met_offset != p_wb_mcast_info->dsmet_offset))
    {
        p_member_info = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_mcast_meminfo_t));
        if (NULL == p_member_info)
        {
            return CTC_E_NO_MEMORY;
        }

        sal_memset(p_member_info, 0, sizeof(sys_nh_mcast_meminfo_t));

        if ( p_wb_mcast_info->member_type == SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP)
        {
            if (p_member_info->dsmet.vid_list == NULL)
            {
                p_member_info->dsmet.vid_list = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(uint16) * SYS_NH_LOGICAL_REPLI_MAX_NUM);
                if (NULL == p_member_info->dsmet.vid_list)
                {
                    return CTC_E_NO_MEMORY;
                }
            }
        }

        if (p_wb_mcast_info->dsmet_offset != p_wb_mcast_info->basic_met_offset)
        {
            sys_usw_nh_offset_alloc_from_position(lchip, p_wb_mcast_info->entry_type, 1, p_wb_mcast_info->dsmet_offset);
        }
        add_member_flag = 0;
        loop = 0;
	}

    p_mcast_info->basic_met_offset = p_wb_mcast_info->basic_met_offset;
    p_mcast_info->physical_replication_num = p_wb_mcast_info->physical_replication_num;
    p_mcast_info->mcast_flag = p_wb_mcast_info->mcast_flag ;
    p_mcast_info->mirror_ref_cnt = p_wb_mcast_info->mirror_ref_cnt;

    p_mcast_info->hdr.adjust_len = p_wb_mcast_info->hdr.adjust_len;
    p_mcast_info->hdr.dsnh_entry_num = p_wb_mcast_info->hdr.dsnh_entry_num;
    p_mcast_info->hdr.nh_entry_flags = p_wb_mcast_info->hdr.nh_entry_flags;
    p_mcast_info->hdr.nh_entry_type = SYS_NH_TYPE_MCAST;

    p_mcast_info->hdr.dsfwd_info.dsfwd_offset = p_wb_mcast_info->hdr.dsfwd_offset;
    p_mcast_info->hdr.dsfwd_info.stats_ptr = p_wb_mcast_info->hdr.stats_ptr;
    p_mcast_info->hdr.dsfwd_info.dsnh_offset = p_wb_mcast_info->hdr.dsnh_offset;
    p_mcast_info->hdr.dsfwd_info.dest_chip = p_wb_mcast_info->hdr.dest_chip;
    p_mcast_info->hdr.dsfwd_info.dest_id = p_wb_mcast_info->hdr.dest_id;

    p_mcast_info->hdr.dsfwd_info.dsnh_offset = SYS_DSNH_INDEX_FOR_NONE;
    p_mcast_info->hdr.nh_id = p_wb_mcast_info->nh_id;


    if (p_wb_mcast_info->physical_replication_num != 0 && p_member_info)
    {/*mcast group have some members*/
         _sys_usw_nh_wb_restore_mcast_member( lchip, p_mcast_info,
                                                   p_wb_mcast_info,
                                                   p_member_info,
                                                   loop);
        if (p_wb_mcast_info->member_type == SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP)
        {
            if ((loop++)  == p_wb_mcast_info->replicate_num)
            {
                physical_replication_loop += 1 ;
				entry_num = p_wb_mcast_info->replicate_num + p_wb_mcast_info->free_dsnh_offset_cnt + 1;
				sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W,entry_num, p_wb_mcast_info->dsnh_offset);
     			add_member_flag = 1;
            }

        }
        else
        {
            if (CTC_FLAG_ISSET(p_wb_mcast_info->flag, SYS_NH_DSMET_FLAG_USE_PBM))
            {
                physical_replication_loop = physical_replication_loop + p_wb_mcast_info->replicate_num + 1;
            }
            else
            {
                physical_replication_loop += 1 ;
            }
			add_member_flag = 1;
        }
	}
	 /*add one member to mcast group*/
	if(add_member_flag)
	{
       ctc_list_pointer_insert_tail(&p_mcast_info->p_mem_list, &(p_member_info->list_head));
	}
    /*add one mcast group*/
    if (physical_replication_loop == p_wb_mcast_info->physical_replication_num )
    {
        if (FALSE == ctc_hash_insert(g_usw_nh_master[lchip]->nhid_hash, p_mcast_info))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;
        }

        if (p_mcast_info->hdr.nh_id >= g_usw_nh_master[lchip]->max_external_nhid )
        {
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_mcast_info->hdr.nh_id);
            g_usw_nh_master[lchip]->internal_nh_used_num++;
        }

        CTC_ERROR_RETURN(_sys_usw_nh_set_glb_met_offset(lchip, p_mcast_info->basic_met_offset, (g_usw_nh_master[lchip]->met_mode?2:1), TRUE));

        if (FALSE ==  ctc_vector_add(g_usw_nh_master[lchip]->mcast_group_vec, p_mcast_info->basic_met_offset, p_mcast_info))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }
        g_usw_nh_master[lchip]->nhid_used_cnt[SYS_NH_TYPE_MCAST]++;
    }
    last_met_offset = p_wb_mcast_info->dsmet_offset;
    last_nhid = p_wb_mcast_info->nh_id;
    CTC_WB_QUERY_ENTRY_END(p_wb_query);


  done:
    return ret;
}

int32
_sys_usw_nh_wb_restore_mpls_tunnel(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{
    sys_wb_nh_mpls_tunnel_t  wb_mpls_tunnel;
    sys_wb_nh_mpls_tunnel_t  *p_wb_mpls_tunnel = &wb_mpls_tunnel;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    uint16 entry_cnt = 0;
    int ret = 0;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_mpls_tunnel_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_MPLS_TUNNEL);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_mpls_tunnel, 0, sizeof(sys_wb_nh_mpls_tunnel_t));
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8 *)p_wb_mpls_tunnel, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    p_mpls_tunnel  = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_db_mpls_tunnel_t));
    if (NULL == p_mpls_tunnel)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    sal_memset(p_mpls_tunnel, 0, sizeof(sys_nh_db_mpls_tunnel_t));

    CTC_ERROR_RETURN(_sys_usw_nh_wb_mapping_mpls_tunnel(lchip, p_mpls_tunnel, p_wb_mpls_tunnel, 0));
    CTC_ERROR_RETURN(ctc_vector_add(g_usw_nh_master[lchip]->tunnel_id_vec, p_mpls_tunnel->tunnel_id, p_mpls_tunnel));

    CTC_WB_QUERY_ENTRY_END(p_wb_query);
 done:
    return ret;
}

int32
_sys_usw_nh_wb_restore_arp(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{

	sys_nh_db_arp_t* p_db_arp = NULL;
	sys_wb_nh_arp_t wb_arp;
	sys_wb_nh_arp_t *p_wb_arp = &wb_arp;

    uint16 entry_cnt = 0;
    int ret = 0;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_arp_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_ARP);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_arp, 0, sizeof(sys_wb_nh_arp_t));
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8 *)p_wb_arp, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    p_db_arp  = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_db_arp_t));
    if (NULL == p_db_arp)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_db_arp, 0, sizeof(sys_nh_db_arp_t));

    sys_usw_nh_wb_mapping_arp(lchip, p_db_arp,p_wb_arp,0);

    if(NULL == ctc_hash_insert(g_usw_nh_master[lchip]->arp_id_hash, p_db_arp))
    {
    	SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No Memory \n");
			return CTC_E_NO_MEMORY;

   	}
    CTC_WB_QUERY_ENTRY_END(p_wb_query);
 done:
    return ret;
}

int32
_sys_usw_nh_wb_restore_brguc_node(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{

    sys_hbnh_brguc_node_info_t *p_nh_brguc_node;
    sys_wb_nh_brguc_node_t     wb_brguc_node;
    sys_wb_nh_brguc_node_t    *p_wb_brguc_node = &wb_brguc_node;
    uint16 entry_cnt = 0;
    int ret = 0;
    uint8 work_status = 0;

	sys_usw_ftm_get_working_status(lchip, &work_status);
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_brguc_node_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_BRGUC_NODE);
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_brguc_node, 0, sizeof(sys_wb_nh_brguc_node_t));
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy((uint8 *)p_wb_brguc_node, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
        (p_wb_query->key_len + p_wb_query->data_len));

    entry_cnt++;
    if(3 == work_status)
    {
        if(p_wb_brguc_node->nhid_brguc != SYS_NH_INVALID_NHID)
        {
            CTC_ERROR_GOTO(_sys_usw_brguc_nh_create_by_type(lchip, p_wb_brguc_node->gport, SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC), ret, done);
        }

        if(p_wb_brguc_node->nhid_bypass != SYS_NH_INVALID_NHID)
        {
            CTC_ERROR_GOTO(_sys_usw_brguc_nh_create_by_type(lchip, p_wb_brguc_node->gport, SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS), ret, done);
        }
        continue;
    }
    p_nh_brguc_node  = mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_hbnh_brguc_node_info_t));
    if (NULL == p_nh_brguc_node)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    sal_memset(p_nh_brguc_node, 0, sizeof(sys_hbnh_brguc_node_info_t));

    p_nh_brguc_node->gport = p_wb_brguc_node->gport;
    p_nh_brguc_node->nhid_brguc = p_wb_brguc_node->nhid_brguc;
    p_nh_brguc_node->nhid_bypass = p_wb_brguc_node->nhid_bypass;

    if(FALSE == ctc_hash_insert(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, p_nh_brguc_node))
    {
    	SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No Memory\n");
        mem_free(p_nh_brguc_node);
		return CTC_E_NO_MEMORY;

   	}
	CTC_WB_QUERY_ENTRY_END(p_wb_query);
    done:
    return ret;
}

int32
_sys_usw_nh_wb_restore_nh_master(uint8 lchip,  ctc_wb_query_t *p_wb_query)
{
    sys_wb_nh_master_t   *p_wb_master = NULL;
    uint16 index = 0;
    sys_nh_ip_tunnel_sa_v4_node_t* p_node = NULL;
    sys_nh_ip_tunnel_sa_v6_node_t* p_node_v6 = NULL;
    uint32 entry_cnt = 0;
    uint32 nh_offset = 0;
    int32 ret = CTC_E_NONE;

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_nh_master_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_MASTER);
    p_wb_master = (sys_wb_nh_master_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_wb_nh_master_t));
    if (NULL == p_wb_master)
    {
        return CTC_E_NO_MEMORY;
    }
    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(p_wb_master, 0, sizeof(sys_wb_nh_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(p_wb_query), ret, out);

    if (p_wb_query->valid_cnt != 0)
    {
        sal_memcpy((uint8*)p_wb_master, (uint8*)(p_wb_query->buffer)+entry_cnt*(p_wb_query->key_len + p_wb_query->data_len),
            (p_wb_query->key_len+p_wb_query->data_len));

        if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_NEXTHOP, p_wb_master->version))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto out;
        }

        if (g_usw_nh_master[lchip]->pkt_nh_edit_mode != p_wb_master->pkt_nh_edit_mode)
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto out;
        }

        if ((g_usw_nh_master[lchip]->max_external_nhid > p_wb_master->max_external_nhid) ||
            (g_usw_nh_master[lchip]->max_tunnel_id > p_wb_master->max_tunnel_id))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto out;
        }

        g_usw_nh_master[lchip]->max_external_nhid = p_wb_master->max_external_nhid;
        g_usw_nh_master[lchip]->max_tunnel_id  = p_wb_master->max_tunnel_id;
        g_usw_nh_master[lchip]->no_dsfwd  = p_wb_master->no_dsfwd;
        g_usw_nh_master[lchip]->max_ecmp  = p_wb_master->max_ecmp;
        g_usw_nh_master[lchip]->cur_ecmp_cnt  = p_wb_master->cur_ecmp_cnt;
        g_usw_nh_master[lchip]->ipmc_logic_replication = p_wb_master->ipmc_logic_replication;
        g_usw_nh_master[lchip]->ecmp_if_resolved_l2edit = p_wb_master->ecmp_if_resolved_l2edit;
        g_usw_nh_master[lchip]->reflective_resolved_dsfwd_offset = p_wb_master->reflective_resolved_dsfwd_offset;
        g_usw_nh_master[lchip]->pkt_nh_edit_mode = p_wb_master->pkt_nh_edit_mode;
        g_usw_nh_master[lchip]->reflective_brg_en = p_wb_master->reflective_brg_en;
        g_usw_nh_master[lchip]->ip_use_l2edit = p_wb_master->ip_use_l2edit;
        g_usw_nh_master[lchip]->udf_profile_bitmap = p_wb_master->udf_profile_bitmap;
        g_usw_nh_master[lchip]->efd_nh_id = p_wb_master->efd_nh_id;

        sal_memcpy(g_usw_nh_master[lchip]->udf_ether_type, p_wb_master->udf_ether_type, sizeof(g_usw_nh_master[lchip]->udf_ether_type));
        sal_memcpy(g_usw_nh_master[lchip]->cw_ref_cnt, p_wb_master->cw_ref_cnt, SYS_NH_CW_NUM*sizeof(uint16));
        sal_memcpy(g_usw_nh_master[lchip]->ipmc_dsnh_offset, p_wb_master->ipmc_dsnh_offset, (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM))*sizeof(uint32));
        for (index = 0; index <= (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1); index++)
        {
            nh_offset = g_usw_nh_master[lchip]->ipmc_dsnh_offset[index];
            if (nh_offset)
            {
                sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1, nh_offset);
            }
        }

        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, 2, g_usw_nh_master[lchip]->ecmp_if_resolved_l2edit);

        /*sync ipv4 ipsa*/
        for (index = 0; index < MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM); index++)
        {
            p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_4]+index;

            p_node->count = p_wb_master->ipv4[index].count;
            sal_memcpy(&p_node->ipv4, &p_wb_master->ipv4[index].ipv4, sizeof(ip_addr_t));
            if (p_node->count)
            {
                /* means used */
                g_usw_nh_master[lchip]->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA]++;
            }
        }

        /*sync ipv6 ipsa*/
        for (index = 0; index < MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM); index++)
        {
            p_node_v6 = (sys_nh_ip_tunnel_sa_v6_node_t*)g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_6]+index;

            p_node_v6->count = p_wb_master->ipv6[index].count;
            sal_memcpy(&p_node_v6->ipv6, &p_wb_master->ipv6[index].ipv6, sizeof(ipv6_addr_t));

            if (p_node_v6->count)
            {
                /* means used */
                g_usw_nh_master[lchip]->nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA]++;
            }
        }

    }

out:
    if (p_wb_master)
    {
        mem_free(p_wb_master);
    }

    return ret;
}

int32
sys_usw_nh_wb_restore(uint8 lchip)
{
    ctc_wb_query_t    wb_query;
    int32 ret = 0;

   /*restore nh_matser*/
    wb_query.buffer = mem_malloc(MEM_NEXTHOP_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

   /*restore nh_matser*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_nh_master(lchip, &wb_query),ret,done);

   /*restore brguc_node*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_brguc_node(lchip,&wb_query),ret,done);

   /*restore mpls tunnel*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_mpls_tunnel(lchip,&wb_query),ret,done);

   /*restore arp*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_arp(lchip,&wb_query),ret,done);

   /*restore  nexthop info*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_nhinfo(lchip,&wb_query),ret,done);

   /*restore  mcast info*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_mcast(lchip,&wb_query),ret,done);

   /*restore  ecmp info*/
   CTC_ERROR_GOTO(_sys_usw_nh_wb_restore_ecmp(lchip,&wb_query),ret,done);

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return 0;
}

int32
_sys_usw_nh_wb_traverse_sync_brguc_node(void *data, void *user_data)
{
    int32 ret;
    sys_hbnh_brguc_node_info_t *p_nh_brguc_node = (sys_hbnh_brguc_node_info_t*)data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t*)user_data;
    sys_wb_nh_brguc_node_t   *p_wb_brguc_node;
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    p_wb_brguc_node = (sys_wb_nh_brguc_node_t *)wb_data->buffer + wb_data->valid_cnt;

    p_wb_brguc_node->gport = p_nh_brguc_node->gport;
    p_wb_brguc_node->nhid_brguc = p_nh_brguc_node->nhid_brguc;
    p_wb_brguc_node->nhid_bypass = p_nh_brguc_node->nhid_bypass;


    if (++wb_data->valid_cnt ==  max_buffer_cnt)
    {
        ret = ctc_wb_add_entry(wb_data);
        if ( ret != CTC_E_NONE )
        {
           return ret;
        }
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_traverse_sync_arp(void* bucket_data, void* user_data)
{
    int32 ret;
    sys_traverse_t *wb_traverse = (sys_traverse_t*)user_data;
    sys_nh_db_arp_t *p_db_arp = (sys_nh_db_arp_t*)bucket_data;
    ctc_wb_data_t *wb_data = wb_traverse->data;
    sys_wb_nh_arp_t   *p_wb_arp;
    uint8 lchip = 0;
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    p_wb_arp = (sys_wb_nh_arp_t *)wb_data->buffer + wb_data->valid_cnt;
    lchip = wb_traverse->value1;

    sys_usw_nh_wb_mapping_arp(lchip, p_db_arp,p_wb_arp,1);

    if (++wb_data->valid_cnt ==  max_buffer_cnt)
    {
        ret = ctc_wb_add_entry(wb_data);
        if ( ret != CTC_E_NONE )
        {
           return ret;
        }
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_traverse_sync_vni(void* bucket_data, void* user_data)
{
    int32 ret;
    sys_traverse_t *wb_traverse = (sys_traverse_t*)user_data;
    sys_nh_vni_mapping_key_t *p_db_vni = (sys_nh_vni_mapping_key_t*)bucket_data;
    ctc_wb_data_t *wb_data = wb_traverse->data;
    sys_wb_nh_vni_fid_t   *p_wb_vni;
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    p_wb_vni = (sys_wb_nh_vni_fid_t *)wb_data->buffer + wb_data->valid_cnt;

    p_wb_vni->vn_id = p_db_vni->vn_id;
    p_wb_vni->fid = p_db_vni->fid;
    p_wb_vni->ref_cnt = p_db_vni->ref_cnt;

    if (++wb_data->valid_cnt ==  max_buffer_cnt)
    {
        ret = ctc_wb_add_entry(wb_data);
        if ( ret != CTC_E_NONE )
        {
           return ret;
        }
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_traverse_sync_nhinfo(void* bucket_data, void* user_data)
{
    int32 ret;
    sys_nh_info_com_t *p_nh_info_com = (sys_nh_info_com_t*)bucket_data;
    sys_traverse_t *wb_traverse = (sys_traverse_t*)user_data;
    ctc_wb_data_t *wb_data = wb_traverse->data;
    sys_wb_nh_info_com_t   *p_wb_nh_info;
    uint16 max_buffer_cnt = wb_data->buffer_len/(wb_data->key_len + wb_data->data_len);
    uint8 lchip = wb_traverse->value1;

    if((wb_traverse->value3 && p_nh_info_com->hdr.nh_entry_type != SYS_NH_TYPE_ECMP)
   	|| (!wb_traverse->value3 && p_nh_info_com->hdr.nh_entry_type == SYS_NH_TYPE_ECMP))
    {
       return CTC_E_NONE;
    }
    p_wb_nh_info = (sys_wb_nh_info_com_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_nh_info, 0, sizeof(sys_wb_nh_info_com_t));

    p_wb_nh_info->nh_id   = p_nh_info_com->hdr.nh_id;
    p_wb_nh_info->nh_type =  p_nh_info_com->hdr.nh_entry_type;
    p_wb_nh_info->hdr.adjust_len =  p_nh_info_com->hdr.adjust_len;

    p_wb_nh_info->hdr.dsnh_entry_num = p_nh_info_com->hdr.dsnh_entry_num;
    p_wb_nh_info->hdr.nh_entry_flags = p_nh_info_com->hdr.nh_entry_flags;
    p_wb_nh_info->hdr.dsfwd_offset = p_nh_info_com->hdr.dsfwd_info.dsfwd_offset;
    p_wb_nh_info->hdr.stats_ptr = p_nh_info_com->hdr.dsfwd_info.stats_ptr;
    p_wb_nh_info->hdr.dsnh_offset = p_nh_info_com->hdr.dsfwd_info.dsnh_offset;
    p_wb_nh_info->hdr.dest_chip = p_nh_info_com->hdr.dsfwd_info.dest_chip;
    p_wb_nh_info->hdr.dest_id = p_nh_info_com->hdr.dsfwd_info.dest_id;

    switch(p_wb_nh_info->nh_type)
    {
        case SYS_NH_TYPE_IPUC:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_ipuc_nhinfo(wb_traverse->value1,p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_BRGUC:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_brguc_nhinfo(lchip, p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_MISC:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_misc_nhinfo(lchip, p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_TOCPU:
        case SYS_NH_TYPE_DROP:
        case SYS_NH_TYPE_UNROV:
        case SYS_NH_TYPE_ILOOP:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_special_nhinfo(p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_RSPAN:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_rspan_nhinfo(p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_ECMP:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_ecmp_nhinfo(lchip, p_nh_info_com, p_wb_nh_info));
            break;
        case SYS_NH_TYPE_IP_TUNNEL:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_ip_tunnel_nhinfo(lchip, p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_WLAN_TUNNEL:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_wlan_nhinfo(lchip, p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_MPLS:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_mpls_nhinfo(lchip, p_nh_info_com, p_wb_nh_info, 1));
            break;
        case SYS_NH_TYPE_TRILL:
            CTC_ERROR_RETURN(sys_usw_nh_wb_mapping_trill_nhinfo(lchip, p_nh_info_com, p_wb_nh_info, 1));
            break;
        default:
            return CTC_E_NONE;
            break;
    }

    if (++wb_data->valid_cnt ==  max_buffer_cnt)
    {
        ret = ctc_wb_add_entry(wb_data);
        if ( ret != CTC_E_NONE )
        {
           return ret;
        }
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_wb_traverse_sync_mcast(void *data,uint32 vec_index, void *user_data)
{
    sys_nh_info_mcast_t *p_nh_info = (sys_nh_info_mcast_t*)data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t*)user_data;
    sys_wb_nh_info_mcast_t   wb_mcast_info;
    sys_nh_mcast_meminfo_t *p_meminfo;
    ctc_list_pointer_node_t* p_list_node;
    uint16 loop = 0,loop_cnt = 0;
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    if((p_nh_info->physical_replication_num == 0)
	  || ctc_list_pointer_empty(&(p_nh_info->p_mem_list)))
  	{ /*no member*/

        sal_memset(&wb_mcast_info,0,sizeof(sys_wb_nh_info_mcast_t));
        wb_mcast_info.entry_type =  p_nh_info->hdr.nh_entry_type;
        wb_mcast_info.hdr.adjust_len =  p_nh_info->hdr.adjust_len;

        wb_mcast_info.hdr.dsnh_entry_num = p_nh_info->hdr.dsnh_entry_num;
        wb_mcast_info.hdr.nh_entry_flags = p_nh_info->hdr.nh_entry_flags;
        wb_mcast_info.hdr.dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
        wb_mcast_info.hdr.stats_ptr = p_nh_info->hdr.dsfwd_info.stats_ptr;
        wb_mcast_info.hdr.dsnh_offset = p_nh_info->hdr.dsfwd_info.dsnh_offset;
        wb_mcast_info.hdr.dest_chip = p_nh_info->hdr.dsfwd_info.dest_chip;
        wb_mcast_info.hdr.dest_id = p_nh_info->hdr.dsfwd_info.dest_id;

        wb_mcast_info.nh_id  = p_nh_info->hdr.nh_id;
        wb_mcast_info.basic_met_offset = p_nh_info->basic_met_offset;
        wb_mcast_info.dsmet_offset = p_nh_info->basic_met_offset;
        wb_mcast_info.physical_replication_num = 0;
        wb_mcast_info.mcast_flag = p_nh_info->mcast_flag;
        wb_mcast_info.mirror_ref_cnt = p_nh_info->mirror_ref_cnt;
        sal_memcpy( (uint8*)wb_data->buffer + wb_data->valid_cnt * sizeof(sys_wb_nh_info_mcast_t),
              (uint8*)&wb_mcast_info, sizeof(sys_wb_nh_info_mcast_t));

	   if (++wb_data->valid_cnt ==  max_buffer_cnt)
        {
          if ( ctc_wb_add_entry(wb_data) < 0 )
          {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

          }
          wb_data->valid_cnt = 0;
        }
	   return CTC_E_NONE;
  	}


    CTC_LIST_POINTER_LOOP(p_list_node, &p_nh_info->p_mem_list)
    {
         p_meminfo = _ctc_container_of(p_list_node, sys_nh_mcast_meminfo_t, list_head);

         sal_memset(&wb_mcast_info,0,sizeof(sys_wb_nh_info_mcast_t));
         wb_mcast_info.nh_id  = p_nh_info->hdr.nh_id;
         wb_mcast_info.basic_met_offset = p_nh_info->basic_met_offset;
         wb_mcast_info.physical_replication_num = p_nh_info->physical_replication_num;
         wb_mcast_info.mcast_flag = p_nh_info->mcast_flag;
         wb_mcast_info.mirror_ref_cnt = p_nh_info->mirror_ref_cnt;

        wb_mcast_info.hdr.adjust_len =  p_nh_info->hdr.adjust_len;
        wb_mcast_info.hdr.dsnh_entry_num = p_nh_info->hdr.dsnh_entry_num;
        wb_mcast_info.hdr.nh_entry_flags = p_nh_info->hdr.nh_entry_flags;
        wb_mcast_info.hdr.dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
        wb_mcast_info.hdr.stats_ptr = p_nh_info->hdr.dsfwd_info.stats_ptr;
        wb_mcast_info.hdr.dsnh_offset = p_nh_info->hdr.dsfwd_info.dsnh_offset;
        wb_mcast_info.hdr.dest_chip = p_nh_info->hdr.dsfwd_info.dest_chip;
        wb_mcast_info.hdr.dest_id = p_nh_info->hdr.dsfwd_info.dest_id;
        /*member info*/

         wb_mcast_info.dsmet_offset = p_meminfo->dsmet.dsmet_offset;
         wb_mcast_info.next_dsmet_offset = p_meminfo->dsmet.next_dsmet_offset;

         wb_mcast_info.flag = p_meminfo->dsmet.flag;
         wb_mcast_info.logic_port = p_meminfo->dsmet.logic_port;

         wb_mcast_info.dsnh_offset = p_meminfo->dsmet.dsnh_offset;
         wb_mcast_info.ref_nhid = (p_meminfo->dsmet.ref_nhid&0xffff);
         wb_mcast_info.ucastid = p_meminfo->dsmet.ucastid;
         wb_mcast_info.replicate_num = p_meminfo->dsmet.replicate_num;
         wb_mcast_info.ref_nhid_hi = (p_meminfo->dsmet.ref_nhid>>16)&0xffff;
		 sal_memcpy( wb_mcast_info.pbm,p_meminfo->dsmet.pbm,4*sizeof(uint32));

         wb_mcast_info.free_dsnh_offset_cnt = p_meminfo->dsmet.free_dsnh_offset_cnt;
         wb_mcast_info.member_type = p_meminfo->dsmet.member_type;
         wb_mcast_info.port_type = p_meminfo->dsmet.port_type;
         wb_mcast_info.entry_type = p_meminfo->dsmet.entry_type;
        if(wb_mcast_info.member_type == SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP)
        {
        	loop_cnt = p_meminfo->dsmet.replicate_num + 1;
        }
        else
        {
            loop_cnt  = 1;
        }

        for (loop = 0; loop < loop_cnt; loop++)
        {
            if (wb_mcast_info.member_type == SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP && (p_meminfo->dsmet.replicate_num > 0))
            {
                wb_mcast_info.vid =  p_meminfo->dsmet.vid_list[loop];
            }
            else
            {
                wb_mcast_info.vid = p_meminfo->dsmet.vid;
            }
            sal_memcpy( (uint8*)wb_data->buffer + wb_data->valid_cnt * sizeof(sys_wb_nh_info_mcast_t),
              (uint8*)&wb_mcast_info, sizeof(sys_wb_nh_info_mcast_t));

            if (++wb_data->valid_cnt ==  max_buffer_cnt)
            {
                if ( ctc_wb_add_entry(wb_data) < 0 )
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        			return CTC_E_NO_RESOURCE;
                }
                wb_data->valid_cnt = 0;
            }
        }
    }

    return CTC_E_NONE;
}


int32
_sys_usw_nh_wb_traverse_sync_mpls_tunnel(void *data,uint32 vec_index, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_nh_db_mpls_tunnel_t *p_tunnel = (sys_nh_db_mpls_tunnel_t *)data;
    sys_wb_nh_mpls_tunnel_t  *p_wb_tunnel;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    uint8 lchip = (uint8)traversal_data->value1;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_tunnel = (sys_wb_nh_mpls_tunnel_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_tunnel, 0, sizeof(sys_wb_nh_mpls_tunnel_t));

    CTC_ERROR_RETURN(_sys_usw_nh_wb_mapping_mpls_tunnel(lchip, p_tunnel, p_wb_tunnel, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_sync_ip_tunnel_ipsa(uint8 lchip, sys_wb_nh_master_t* p_wb_master)
{
    uint8 index = 0;
    sys_nh_ip_tunnel_sa_v4_node_t* p_node = NULL;
    sys_nh_ip_tunnel_sa_v6_node_t* p_node_v6 = NULL;

    /*sync ipv4 ipsa*/
    for (index = 0; index < MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM); index++)
    {
        p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_4]+index;

        p_wb_master->ipv4[index].count  =  p_node->count;
        sal_memcpy(&p_wb_master->ipv4[index].ipv4, &p_node->ipv4, sizeof(ip_addr_t));
    }

    /*sync ipv6 ipsa*/
    for (index = 0; index < MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM); index++)
    {
        p_node_v6 = (sys_nh_ip_tunnel_sa_v6_node_t*)g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_6]+index;
        p_wb_master->ipv6[index].count  =  p_node_v6->count;
        sal_memcpy(&p_wb_master->ipv6[index].ipv6, &p_node_v6->ipv6, sizeof(ipv6_addr_t));
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_nh_master_t* p_wb_nh_master = NULL;
    sys_traverse_t  wb_nh_traverse;
    sys_traverse_t  wb_arp_traverse;
    sys_traverse_t  wb_vni_traverse;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);


    p_wb_nh_master = (sys_wb_nh_master_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_wb_nh_master_t));
    wb_data.buffer = mem_malloc(MEM_NEXTHOP_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if ((NULL == wb_data.buffer) || (NULL == p_wb_nh_master))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(p_wb_nh_master, 0, sizeof(sys_wb_nh_master_t));
    if(3 == work_status)
    {
        /*only sync brguc nodex*/
        CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_brguc_node_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_BRGUC_NODE);
     	ctc_hash_traverse(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash,_sys_usw_nh_wb_traverse_sync_brguc_node,&wb_data);

        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }

        goto done;
    }
    /*syncup  nh_matser*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_master_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_MASTER);
    p_wb_nh_master->lchip = lchip;
    p_wb_nh_master->max_external_nhid = g_usw_nh_master[lchip]->max_external_nhid;
    p_wb_nh_master->max_tunnel_id = g_usw_nh_master[lchip]->max_tunnel_id;
    p_wb_nh_master->no_dsfwd = g_usw_nh_master[lchip]->no_dsfwd;
    p_wb_nh_master->max_ecmp = g_usw_nh_master[lchip]->max_ecmp;
    p_wb_nh_master->cur_ecmp_cnt = g_usw_nh_master[lchip]->cur_ecmp_cnt;
    p_wb_nh_master->ipmc_logic_replication = g_usw_nh_master[lchip]->ipmc_logic_replication;
    p_wb_nh_master->ecmp_if_resolved_l2edit = g_usw_nh_master[lchip]->ecmp_if_resolved_l2edit;
    p_wb_nh_master->reflective_resolved_dsfwd_offset = g_usw_nh_master[lchip]->reflective_resolved_dsfwd_offset;
    p_wb_nh_master->pkt_nh_edit_mode = g_usw_nh_master[lchip]->pkt_nh_edit_mode;
    p_wb_nh_master->reflective_brg_en = g_usw_nh_master[lchip]->reflective_brg_en;
    p_wb_nh_master->ip_use_l2edit = g_usw_nh_master[lchip]->ip_use_l2edit;
    p_wb_nh_master->version = SYS_WB_VERSION_NEXTHOP;
    p_wb_nh_master->udf_profile_bitmap = g_usw_nh_master[lchip]->udf_profile_bitmap;
    p_wb_nh_master->efd_nh_id = g_usw_nh_master[lchip]->efd_nh_id;

    sal_memcpy(p_wb_nh_master->udf_ether_type, g_usw_nh_master[lchip]->udf_ether_type, sizeof(g_usw_nh_master[lchip]->udf_ether_type));

    CTC_ERROR_GOTO(sys_usw_nh_wb_sync_ip_tunnel_ipsa(lchip, p_wb_nh_master), ret, done);

    sal_memcpy(p_wb_nh_master->cw_ref_cnt, g_usw_nh_master[lchip]->cw_ref_cnt,  SYS_NH_CW_NUM*sizeof(uint16));
    sal_memcpy(p_wb_nh_master->ipmc_dsnh_offset, g_usw_nh_master[lchip]->ipmc_dsnh_offset,  (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM))*sizeof(uint32));

    sal_memcpy((uint8*)wb_data.buffer, (uint8*)p_wb_nh_master, sizeof(sys_wb_nh_master_t));

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

   	/*sync brguc nodex*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_brguc_node_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_BRGUC_NODE);
 	ctc_hash_traverse(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash,_sys_usw_nh_wb_traverse_sync_brguc_node,&wb_data);

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

  	/*sync arp*/
    wb_arp_traverse.data = &wb_data;
    wb_arp_traverse.value1 = lchip;
    wb_arp_traverse.value2 = 0;
    wb_arp_traverse.value3 = 0;
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_arp_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_ARP);
 	ctc_hash_traverse(g_usw_nh_master[lchip]->arp_id_hash, _sys_usw_nh_wb_traverse_sync_arp,&wb_arp_traverse);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*sync nexthop info*/
    wb_nh_traverse.data = &wb_data;
    wb_nh_traverse.value1 = lchip;
    wb_nh_traverse.value2 = 0;
    wb_nh_traverse.value3 = 0;
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_info_com_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_INFO_COM);
	ctc_hash_traverse(g_usw_nh_master[lchip]->nhid_hash,_sys_usw_nh_wb_traverse_sync_nhinfo, &wb_nh_traverse);

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*sync ecmp nexthop*/
    wb_nh_traverse.value2 = 0;
    wb_nh_traverse.value3 = 1;

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_info_com_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_ECMP);
	ctc_hash_traverse(g_usw_nh_master[lchip]->nhid_hash,_sys_usw_nh_wb_traverse_sync_nhinfo, &wb_nh_traverse);

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

	/*sync mcast*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_info_mcast_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_INFO_MCAST);
	ctc_vector_traverse2(g_usw_nh_master[lchip]->mcast_group_vec,0,_sys_usw_nh_wb_traverse_sync_mcast,&wb_data);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*sync mpls tunnel info*/
    wb_nh_traverse.value2 = 1;
    wb_nh_traverse.value3 = 0;
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_mpls_tunnel_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_MPLS_TUNNEL);
	ctc_vector_traverse2(g_usw_nh_master[lchip]->tunnel_id_vec,0,_sys_usw_nh_wb_traverse_sync_mpls_tunnel,&wb_nh_traverse);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

	/*sync vni*/
    wb_vni_traverse.data = &wb_data;
    wb_vni_traverse.value1 = lchip;
    wb_vni_traverse.value2 = 0;
    wb_vni_traverse.value3 = 0;
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_nh_vni_fid_t, CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_VNI);
 	ctc_hash_traverse(g_usw_nh_master[lchip]->vxlan_vni_hash, _sys_usw_nh_wb_traverse_sync_vni,&wb_vni_traverse);
 	if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    if (p_wb_nh_master)
    {
        mem_free(p_wb_nh_master);
    }

    return ret;
}
STATIC int32
_sys_usw_nexthop_traverse_fprintf_vprof_pool(void* node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    sys_dsl2edit_vlan_edit_t*   p_vprof_db =  NULL;
    uint32*                     cnt         = (uint32 *)(user_date->value1);

    p_vprof_db = (sys_dsl2edit_vlan_edit_t*)((ctc_spool_node_t*)node)->data;
    SYS_DUMP_DB_LOG(p_file, "%-10s:%-5u,     %-10s:%-5u\n","Node", *cnt, "refcnt", ((ctc_spool_node_t*)node)->ref_cnt);
    SYS_DUMP_DB_LOG(p_file, "%-10s\n","----------------------------------------");
    SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","profile_id", p_vprof_db->profile_id);
    SYS_DUMP_DB_LOG(p_file, "  %-20s:%x\n","data",  *((uint32*)&p_vprof_db->data));

    return 0;
}

STATIC int32
_sys_usw_nexthop_traverse_fprintf_pool(void* node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    sys_nh_db_edit_t*           p_l2edit_db =  NULL;
    uint32*                     cnt         = (uint32 *)(user_date->value1);

    p_l2edit_db = (sys_nh_db_edit_t*)((ctc_spool_node_t*)node)->data;
    SYS_DUMP_DB_LOG(p_file, "%-10s:%-5u,     %-10s:%-5u\n","Node", *cnt, "refcnt", ((ctc_spool_node_t*)node)->ref_cnt);
    SYS_DUMP_DB_LOG(p_file, "%-10s\n","----------------------------------------");
    SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","edit_type", p_l2edit_db->edit_type);
    SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","offset", p_l2edit_db->offset);
    if ((p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W) || (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W) ||
        (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP))
    {
        sys_nh_db_dsl2editeth4w_t* p_l2edit_4w = (sys_nh_db_dsl2editeth4w_t*)&p_l2edit_db->data.l2edit_4w;

        SYS_DUMP_DB_LOG(p_file, "  %-20s:%.4x.%.4x.%.4x \n","mac_da", sal_ntohs(*(unsigned short*)&p_l2edit_4w->mac_da[0]),
        sal_ntohs(*(unsigned short*)&p_l2edit_4w->mac_da[2]), sal_ntohs(*(unsigned short*)&p_l2edit_4w->mac_da[4]));
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_ecmp_if",           p_l2edit_4w->is_ecmp_if);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","strip_inner_vlan",     p_l2edit_4w->strip_inner_vlan);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","output_vid",           p_l2edit_4w->output_vid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","output_vlan_is_svlan", p_l2edit_4w->output_vlan_is_svlan);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dynamic",              p_l2edit_4w->dynamic);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_share_mac",         p_l2edit_4w->is_share_mac);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_dot11",             p_l2edit_4w->is_dot11);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","fpma",                 p_l2edit_4w->fpma);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mac_from_label",      p_l2edit_4w->derive_mac_from_label);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mcast_mac_for_mpls",  p_l2edit_4w->derive_mcast_mac_for_mpls);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mcast_mac_for_trill", p_l2edit_4w->derive_mcast_mac_for_trill);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","map_cos",              p_l2edit_4w->map_cos);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dot11_sub_type",       p_l2edit_4w->dot11_sub_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mcast_mac_for_ip",    p_l2edit_4w->derive_mcast_mac_for_ip);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","mac_da_en",            p_l2edit_4w->mac_da_en);
    }
    else if ((p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W)||(p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W)||
        (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L2EDIT_SWAP))
    {
        sys_nh_db_dsl2editeth8w_t* p_l2edit_8w = (sys_nh_db_dsl2editeth8w_t*)&p_l2edit_db->data.l2edit_8w;
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%x-%x-%x-%x-%x-%x\n","mac_da", p_l2edit_8w->mac_da[0],p_l2edit_8w->mac_da[1],p_l2edit_8w->mac_da[2],
            p_l2edit_8w->mac_da[3], p_l2edit_8w->mac_da[4], p_l2edit_8w->mac_da[5]);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_ecmp_if",           p_l2edit_8w->is_ecmp_if);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","strip_inner_vlan",     p_l2edit_8w->strip_inner_vlan);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","output_vid",           p_l2edit_8w->output_vid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","output_vlan_is_svlan", p_l2edit_8w->output_vlan_is_svlan);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dynamic",              p_l2edit_8w->dynamic);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_share_mac",         p_l2edit_8w->is_share_mac);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_dot11",             p_l2edit_8w->is_dot11);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","fpma",                 p_l2edit_8w->fpma);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mac_from_label",      p_l2edit_8w->derive_mac_from_label);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mcast_mac_for_mpls",  p_l2edit_8w->derive_mcast_mac_for_mpls);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mcast_mac_for_trill", p_l2edit_8w->derive_mcast_mac_for_trill);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","map_cos",              p_l2edit_8w->map_cos);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dot11_sub_type",       p_l2edit_8w->dot11_sub_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_mcast_mac_for_ip",    p_l2edit_8w->derive_mcast_mac_for_ip);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","mac_da_en",            p_l2edit_8w->mac_da_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%.4x.%.4x.%.4x \n","mac_sa",  sal_ntohs(*(unsigned short*)&p_l2edit_8w->mac_sa[0]),
            sal_ntohs(*(unsigned short*)&p_l2edit_8w->mac_sa[2]), sal_ntohs(*(unsigned short*)&p_l2edit_8w->mac_sa[4]));
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","update_mac_sa",        p_l2edit_8w->update_mac_sa);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","packet_type",          p_l2edit_8w->packet_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","cos_domain",           p_l2edit_8w->cos_domain);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ether_type",           p_l2edit_8w->ether_type);
    }
    else if (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4)
    {
        sys_dsl3edit_tunnelv4_t* p_tunnel_v4 = (sys_dsl3edit_tunnelv4_t*)&p_l2edit_db->data.l3edit_v4;
        uint32 tempip = 0;
        char ipsa[100] = {0};
        char ipda[100] = {0};

        tempip = sal_ntohl(p_tunnel_v4->ipsa);
        sal_inet_ntop(AF_INET, &tempip, ipsa, CTC_IPV6_ADDR_STR_LEN);
        tempip = sal_ntohl(p_tunnel_v4->ipda);
        sal_inet_ntop(AF_INET, &tempip, ipda, CTC_IPV6_ADDR_STR_LEN);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","share_type",           p_tunnel_v4->share_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ds_type",              p_tunnel_v4->ds_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l3_rewrite_type",      p_tunnel_v4->l3_rewrite_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","map_ttl",              p_tunnel_v4->map_ttl);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ttl",                  p_tunnel_v4->ttl);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","copy_dont_frag",       p_tunnel_v4->copy_dont_frag);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dont_frag",            p_tunnel_v4->dont_frag);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_dscp",          p_tunnel_v4->derive_dscp);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dscp",                 p_tunnel_v4->dscp);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dscp_domain",          p_tunnel_v4->dscp_domain);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ip_protocol_type",     p_tunnel_v4->ip_protocol_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","mtu_check_en",         p_tunnel_v4->mtu_check_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%s\n","ipda",                 ipda);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%s\n","ipsa",                 ipsa);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ipsa_6to4",            p_tunnel_v4->ipsa_6to4);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ipsa_valid",           p_tunnel_v4->ipsa_valid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ipsa_index",           p_tunnel_v4->ipsa_index);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","inner_header_valid",   p_tunnel_v4->inner_header_valid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","inner_header_type",    p_tunnel_v4->inner_header_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","stats_ptr",            p_tunnel_v4->stats_ptr);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_protocol",         p_tunnel_v4->gre_protocol);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_version",          p_tunnel_v4->gre_version);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_flags",            p_tunnel_v4->gre_flags);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_key",              p_tunnel_v4->gre_key);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","isatp_tunnel",         p_tunnel_v4->isatp_tunnel);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tunnel6_to4_da",       p_tunnel_v4->tunnel6_to4_da);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tunnel6_to4_sa",       p_tunnel_v4->tunnel6_to4_sa);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tunnel6_to4_da_ipv4_prefixlen",   p_tunnel_v4->tunnel6_to4_da_ipv4_prefixlen);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tunnel6_to4_da_ipv6_prefixlen",   p_tunnel_v4->tunnel6_to4_da_ipv6_prefixlen);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tunnel6_to4_sa_ipv4_prefixlen",   p_tunnel_v4->tunnel6_to4_sa_ipv4_prefixlen);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tunnel6_to4_sa_ipv6_prefixlen",   p_tunnel_v4->tunnel6_to4_sa_ipv6_prefixlen);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","out_l2edit_valid",     p_tunnel_v4->out_l2edit_valid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","xerspan_hdr_en",       p_tunnel_v4->xerspan_hdr_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","xerspan_hdr_with_hash_en",        p_tunnel_v4->xerspan_hdr_with_hash_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l2edit_ptr",           p_tunnel_v4->l2edit_ptr);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_evxlan",            p_tunnel_v4->is_evxlan);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","evxlan_protocl_index",           p_tunnel_v4->evxlan_protocl_index);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_geneve",            p_tunnel_v4->is_geneve);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_gre_auto",          p_tunnel_v4->is_gre_auto);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_vxlan_auto",        p_tunnel_v4->is_vxlan_auto);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","udp_src_port_en",      p_tunnel_v4->udp_src_port_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l4_src_port",          p_tunnel_v4->l4_src_port);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l4_dest_port",         p_tunnel_v4->l4_dest_port);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","encrypt_en",           p_tunnel_v4->encrypt_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","encrypt_id",           p_tunnel_v4->encrypt_id);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","bssid_bitmap",         p_tunnel_v4->bssid_bitmap);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","udp_src_port_en",      p_tunnel_v4->udp_src_port_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","rmac_en",              p_tunnel_v4->rmac_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","data_keepalive",       p_tunnel_v4->data_keepalive);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ecn_mode",             p_tunnel_v4->ecn_mode);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","sc_index",             p_tunnel_v4->sc_index);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","sci_en",               p_tunnel_v4->sci_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","cloud_sec_en",         p_tunnel_v4->cloud_sec_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","udf_profile_id",       p_tunnel_v4->udf_profile_id);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u %u %u %u \n", "udf_edit",   p_tunnel_v4->udf_edit[0], p_tunnel_v4->udf_edit[1],
            p_tunnel_v4->udf_edit[2], p_tunnel_v4->udf_edit[3]);
    }
    else if (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6)
    {
        sys_dsl3edit_tunnelv6_t* p_tunnel_v6 = (sys_dsl3edit_tunnelv6_t*)&p_l2edit_db->data.l3edit_v6;
        char ipda[100] = {0};
        ipv6_addr_t sw_ipv6_address;

        sw_ipv6_address[0] = sal_ntohl(p_tunnel_v6->ipda[3]);
        sw_ipv6_address[1] = sal_ntohl(p_tunnel_v6->ipda[2]);
        sw_ipv6_address[2] = sal_ntohl(p_tunnel_v6->ipda[1]);
        sw_ipv6_address[3] = sal_ntohl(p_tunnel_v6->ipda[0]);

        sal_inet_ntop(AF_INET6, sw_ipv6_address, ipda, CTC_IPV6_ADDR_STR_LEN);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","share_type",           p_tunnel_v6->share_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ds_type",              p_tunnel_v6->ds_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l3_rewrite_type",      p_tunnel_v6->l3_rewrite_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","map_ttl",              p_tunnel_v6->map_ttl);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ttl",                  p_tunnel_v6->ttl);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","derive_dscp",          p_tunnel_v6->derive_dscp);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","tos",                  p_tunnel_v6->tos);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","dscp_domain",          p_tunnel_v6->dscp_domain);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ip_protocol_type",     p_tunnel_v6->ip_protocol_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","mtu_check_en",         p_tunnel_v6->mtu_check_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%s\n","ipda",                 ipda);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ipsa_index",           p_tunnel_v6->ipsa_index);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","inner_header_valid",   p_tunnel_v6->inner_header_valid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","inner_header_type",    p_tunnel_v6->inner_header_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","stats_ptr",            p_tunnel_v6->stats_ptr);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_protocol",         p_tunnel_v6->gre_protocol);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_version",          p_tunnel_v6->gre_version);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_flags",            p_tunnel_v6->gre_flags);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","gre_key",              p_tunnel_v6->gre_key);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","new_flow_label_valid", p_tunnel_v6->new_flow_label_valid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","new_flow_label_mode",  p_tunnel_v6->new_flow_label_mode);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","flow_label",           p_tunnel_v6->flow_label);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","out_l2edit_valid",     p_tunnel_v6->out_l2edit_valid);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","xerspan_hdr_en",       p_tunnel_v6->xerspan_hdr_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","xerspan_hdr_with_hash_en",        p_tunnel_v6->xerspan_hdr_with_hash_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l2edit_ptr",           p_tunnel_v6->l2edit_ptr);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_evxlan",            p_tunnel_v6->is_evxlan);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","evxlan_protocl_index",           p_tunnel_v6->evxlan_protocl_index);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_geneve",            p_tunnel_v6->is_geneve);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_gre_auto",          p_tunnel_v6->is_gre_auto);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","is_vxlan_auto",        p_tunnel_v6->is_vxlan_auto);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","udp_src_port_en",      p_tunnel_v6->udp_src_port_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l4_src_port",          p_tunnel_v6->l4_src_port);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l4_dest_port",         p_tunnel_v6->l4_dest_port);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","encrypt_en",           p_tunnel_v6->encrypt_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","encrypt_id",           p_tunnel_v6->encrypt_id);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","bssid_bitmap",         p_tunnel_v6->bssid_bitmap);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","udp_src_port_en",      p_tunnel_v6->udp_src_port_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","rmac_en",              p_tunnel_v6->rmac_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","data_keepalive",       p_tunnel_v6->data_keepalive);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ecn_mode",             p_tunnel_v6->ecn_mode);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","sc_index",             p_tunnel_v6->sc_index);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","sci_en",               p_tunnel_v6->sci_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","cloud_sec_en",         p_tunnel_v6->cloud_sec_en);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","udf_profile_id",       p_tunnel_v6->udf_profile_id);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u %u %u %u \n", "udf_edit",   p_tunnel_v6->udf_edit[0], p_tunnel_v6->udf_edit[1],
            p_tunnel_v6->udf_edit[2], p_tunnel_v6->udf_edit[3]);
    }
    else if (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W)
    {
        sys_dsl3edit_nat4w_t* p_nat_4w = (sys_dsl3edit_nat4w_t*)&p_l2edit_db->data.nat_4w;
        uint32 tempip = 0;
        char ipda[100] = {0};

        tempip = sal_ntohl(p_nat_4w->ipda);
        sal_inet_ntop(AF_INET, &tempip, ipda, CTC_IPV6_ADDR_STR_LEN);

        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ds_type",              p_nat_4w->ds_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l3_rewrite_type",      p_nat_4w->l3_rewrite_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ip_da_prefix_length",  p_nat_4w->ip_da_prefix_length);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ipv4_embeded_mode",    p_nat_4w->ipv4_embeded_mode);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%s\n","ipda",      ipda);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l4_dest_port",         p_nat_4w->l4_dest_port);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","replace_ip_da",        p_nat_4w->replace_ip_da);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","replace_l4_dest_port", p_nat_4w->replace_l4_dest_port);

    }
    else if (p_l2edit_db->edit_type == SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W)
    {
        sys_dsl3edit_nat8w_t* p_nat_8w = (sys_dsl3edit_nat8w_t*)&p_l2edit_db->data.nat_8w;
        char ipda[100] = {0};
        sal_inet_ntop(AF_INET6, p_nat_8w->ipda, ipda, CTC_IPV6_ADDR_STR_LEN);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ds_type",      p_nat_8w->ds_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l3_rewrite_type",      p_nat_8w->l3_rewrite_type);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ip_da_prefix_length",      p_nat_8w->ip_da_prefix_length);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","ipv4_embeded_mode",      p_nat_8w->ipv4_embeded_mode);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%s\n","ipda",      ipda);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","l4_dest_port",      p_nat_8w->l4_dest_port);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","replace_ip_da",      p_nat_8w->replace_ip_da);
        SYS_DUMP_DB_LOG(p_file, "  %-20s:%u\n","replace_l4_dest_port",      p_nat_8w->replace_l4_dest_port);
    }

    (*cnt)++;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_port_init(uint8 lchip, uint8 reverse)
{

    uint8 lchip_tmp                = 0;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8  gchip                   = 0;
    uint32 gport                   = 0;
    uint32 index                   = 0;
    int32  ret                     = CTC_E_NONE;

    lchip_tmp = lchip;
    sys_usw_get_gchip_id(lchip_tmp, &gchip);

    for (index = 0; index < SYS_USW_MAX_PORT_NUM_PER_CHIP; index++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(index));

        if ((index & 0xFF) < SYS_PHY_PORT_NUM_PER_SLICE)
        {
            /*init all chip port nexthop*/
            lchip = lchip_tmp;
            CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
            {
                ret = reverse? sys_usw_brguc_nh_delete(lchip, gport): sys_usw_brguc_nh_create(lchip, gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL);
                if ((CTC_E_EXIST != ret) && (CTC_E_NONE != ret) && (CTC_E_NOT_INIT != ret))
                {
                    return ret;
                }
            }
        }
    }

   return CTC_E_NONE;
}

int32
_sys_usw_nexthop_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 i = 0;
    sys_dump_db_traverse_param_t    cb_data;
    uint32 num_cnt = 0;

    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "#Nexthop");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","max_external_nhid",      g_usw_nh_master[lchip]->max_external_nhid);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","max_tunnel_id",          g_usw_nh_master[lchip]->max_tunnel_id);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","ecmp_if_resolved_l2edit",g_usw_nh_master[lchip]->ecmp_if_resolved_l2edit);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","reflective_resolved_dsfwd_offset",g_usw_nh_master[lchip]->reflective_resolved_dsfwd_offset);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","fatal_excp_base",        g_usw_nh_master[lchip]->fatal_excp_base);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","max_glb_nh_offset",      g_usw_nh_master[lchip]->max_glb_nh_offset);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","max_glb_met_offset",      g_usw_nh_master[lchip]->max_glb_met_offset);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","ipmc_logic_replication",      g_usw_nh_master[lchip]->ipmc_logic_replication);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","pkt_nh_edit_mode",      g_usw_nh_master[lchip]->pkt_nh_edit_mode);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","no_dsfwd",      g_usw_nh_master[lchip]->no_dsfwd);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","max_ecmp",      g_usw_nh_master[lchip]->max_ecmp);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","max_ecmp_group_num",      g_usw_nh_master[lchip]->max_ecmp_group_num);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","cur_ecmp_cnt",      g_usw_nh_master[lchip]->cur_ecmp_cnt);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","reflective_brg_en",      g_usw_nh_master[lchip]->reflective_brg_en);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","ip_use_l2edit",      g_usw_nh_master[lchip]->ip_use_l2edit);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","rspan_vlan_id",      g_usw_nh_master[lchip]->rspan_vlan_id);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","met_mode",      g_usw_nh_master[lchip]->met_mode);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","cid_use_4w",      g_usw_nh_master[lchip]->cid_use_4w);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","bpe_en",      g_usw_nh_master[lchip]->bpe_en);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","xlate_nh_offset",      g_usw_nh_master[lchip]->xlate_nh_offset);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","mpls_edit_offset",      g_usw_nh_master[lchip]->mpls_edit_offset);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","internal_nh_used_num",      g_usw_nh_master[lchip]->internal_nh_used_num);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","efd_nh_id",      g_usw_nh_master[lchip]->efd_nh_id);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","udf_profile_bitmap",      g_usw_nh_master[lchip]->udf_profile_bitmap);
    SYS_DUMP_DB_LOG(p_f, "%-35s:","udf_ether_type");
    for(i = 0; i < 4; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",g_usw_nh_master[lchip]->udf_ether_type[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","udf_profile_ref_cnt");
    for(i = 0; i < 4; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",g_usw_nh_master[lchip]->udf_profile_ref_cnt[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","nhid_used_cnt");
    for(i = 0; i < SYS_NH_TYPE_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",g_usw_nh_master[lchip]->nhid_used_cnt[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","nhtbl_used_cnt");
    for(i = 0; i < SYS_NH_ENTRY_TYPE_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",g_usw_nh_master[lchip]->nhtbl_used_cnt[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","cw_ref_cnt");
    for(i = 0; i < SYS_NH_CW_NUM; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",g_usw_nh_master[lchip]->cw_ref_cnt[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","sys_nh_resolved_offset");
    for(i = 0; i < SYS_NH_RES_OFFSET_TYPE_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%4u,%4u,%4u]",g_usw_nh_master[lchip]->sys_nh_resolved_offset[i].offset_base,
            g_usw_nh_master[lchip]->sys_nh_resolved_offset[i].entry_size, g_usw_nh_master[lchip]->sys_nh_resolved_offset[i].entry_num);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","nh_table_info_array");
    for(i = 0; i < SYS_NH_ENTRY_TYPE_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%4u,%4u,%4u,%4u,%4u]",g_usw_nh_master[lchip]->nh_table_info_array[i].table_id, g_usw_nh_master[lchip]->nh_table_info_array[i].entry_size,
            g_usw_nh_master[lchip]->nh_table_info_array[i].opf_pool_type, g_usw_nh_master[lchip]->nh_table_info_array[i].alloc_dir,
            g_usw_nh_master[lchip]->nh_table_info_array[i].table_8w);
        if ((((i+1)%6) == 0))
        {
            SYS_DUMP_DB_LOG(p_f, "\n");
            SYS_DUMP_DB_LOG(p_f, "%-35s:","");
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","ipmc_dsnh_offset");
    for(i = 0; i < SYS_SPEC_L3IF_NUM; i ++)
    {
        if (!g_usw_nh_master[lchip]->ipmc_dsnh_offset[i])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%u]", i, g_usw_nh_master[lchip]->ipmc_dsnh_offset[i]);
        if ((((i+1)%48) == 0))
        {
            SYS_DUMP_DB_LOG(p_f, "\n");
            SYS_DUMP_DB_LOG(p_f, "%-35s:","");
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","ipv4 ipsa");
    for(i = 0; i < MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV4_IPSA_NUM); i ++)
    {
        sys_nh_ip_tunnel_sa_v4_node_t* p_node = NULL;
        uint32 tempip = 0;
        char ip[100] = {0};

        p_node = (sys_nh_ip_tunnel_sa_v4_node_t*)g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_4] + i;
        if (!p_node->count)
        {
            continue;
        }
        tempip = sal_ntohl(p_node->ipv4);
        sal_inet_ntop(AF_INET, &tempip, ip, CTC_IPV6_ADDR_STR_LEN);
        SYS_DUMP_DB_LOG(p_f, "[%u:%u, %s]",i, p_node->count, ip);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","ipv6 ipsa");
    for(i = 0; i < MCHIP_CAP(SYS_CAP_NH_IP_TUNNEL_IPV6_IPSA_NUM); i ++)
    {
        sys_nh_ip_tunnel_sa_v6_node_t* p_node = NULL;
        char ip[100] = {0};
        ipv6_addr_t sw_ipv6_address;
        p_node = (sys_nh_ip_tunnel_sa_v6_node_t*)g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_6] + i;
        if (!p_node->count)
        {
            continue;
        }

        sw_ipv6_address[0] = sal_ntohl(p_node->ipv6[3]);
        sw_ipv6_address[1] = sal_ntohl(p_node->ipv6[2]);
        sw_ipv6_address[2] = sal_ntohl(p_node->ipv6[1]);
        sw_ipv6_address[3] = sal_ntohl(p_node->ipv6[0]);
        sal_inet_ntop(AF_INET6, sw_ipv6_address, ip, CTC_IPV6_ADDR_STR_LEN);
        SYS_DUMP_DB_LOG(p_f, "[%u:%u, %s]",i, p_node->count, ip);
        if (((i+1)%6) == 0)
        {
            SYS_DUMP_DB_LOG(p_f, "\n");
            SYS_DUMP_DB_LOG(p_f, "%-35s:","");
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s:","nhtbl_used_cnt");
    for(i = 0; i < SYS_NH_ENTRY_TYPE_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",g_usw_nh_master[lchip]->nhtbl_used_cnt[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    sys_usw_opf_fprint_alloc_used_info(lchip, g_usw_nh_master[lchip]->nh_opf_type, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, g_usw_nh_master[lchip]->ecmp_group_opf_type, p_f);
    sys_usw_opf_fprint_alloc_used_info(lchip, g_usw_nh_master[lchip]->ecmp_member_opf_type, p_f);

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    cb_data.value1 = &num_cnt;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","p_edit_spool");
    ctc_spool_traverse(g_usw_nh_master[lchip]->p_edit_spool, (spool_traversal_fn)_sys_usw_nexthop_traverse_fprintf_pool , (void*)(&cb_data));

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    num_cnt = 0;
    cb_data.value0 = p_f;
    cb_data.value1 = &num_cnt;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","p_l2edit_vprof(DsOfEditVlanActionProfile_t)");
    ctc_spool_traverse(g_usw_nh_master[lchip]->p_l2edit_vprof, (spool_traversal_fn)_sys_usw_nexthop_traverse_fprintf_vprof_pool, (void*)(&cb_data));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    return ret;
}

int32
sys_usw_nh_init(uint8 lchip, ctc_nh_global_cfg_t* nh_cfg)
{
    uint32 entry_num = 0;
    sys_usw_nh_master_t* p_master = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	LCHIP_CHECK(lchip);
    /*Nexthop module have initialize*/
    if (NULL != g_usw_nh_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (nh_cfg->max_external_nhid == 0)
    {
        nh_cfg->max_external_nhid = SYS_NH_EXTERNAL_NHID_DEFAUL_NUM;
    }

    if (nh_cfg->max_tunnel_id == 0)
    {
        nh_cfg->max_tunnel_id = SYS_NH_TUNNEL_ID_DEFAUL_NUM;
    }

    /*1. Create master*/
    CTC_ERROR_RETURN(_sys_usw_nh_master_new(lchip, &p_master, nh_cfg->max_external_nhid, nh_cfg->max_tunnel_id, SYS_NH_ARP_ID_DEFAUL_NUM));

    g_usw_nh_master[lchip] = p_master;
    g_usw_nh_master[lchip]->max_external_nhid = nh_cfg->max_external_nhid;
    g_usw_nh_master[lchip]->max_tunnel_id = nh_cfg->max_tunnel_id;

    if (nh_cfg->nh_edit_mode == CTC_NH_EDIT_MODE_SINGLE_CHIP)
    {
        g_usw_nh_master[lchip]->pkt_nh_edit_mode = SYS_NH_IGS_CHIP_EDIT_MODE;
    }
    else if(nh_cfg->nh_edit_mode == 2)
    {
        g_usw_nh_master[lchip]->pkt_nh_edit_mode  = SYS_NH_EGS_CHIP_EDIT_MODE ;
    }
    else
    {
        g_usw_nh_master[lchip]->pkt_nh_edit_mode  = SYS_NH_MISC_CHIP_EDIT_MODE ;
    }


    SYS_USW_NH_MAX_ECMP_CHECK(nh_cfg->max_ecmp);
    g_usw_nh_master[lchip]->max_ecmp  = (nh_cfg->max_ecmp > SYS_USW_MAX_ECPN)?SYS_USW_MAX_ECPN :nh_cfg->max_ecmp;
    if (0 == nh_cfg->max_ecmp)
    {
        g_usw_nh_master[lchip]->max_ecmp_group_num = MCHIP_CAP(SYS_CAP_NH_ECMP_GROUP_ID_NUM);
    }
    else
    {
        g_usw_nh_master[lchip]->max_ecmp_group_num = MCHIP_CAP(SYS_CAP_NH_ECMP_MEMBER_NUM) / nh_cfg->max_ecmp;
        if ( g_usw_nh_master[lchip]->max_ecmp_group_num > MCHIP_CAP(SYS_CAP_NH_ECMP_GROUP_ID_NUM))
        {
            g_usw_nh_master[lchip]->max_ecmp_group_num = MCHIP_CAP(SYS_CAP_NH_ECMP_GROUP_ID_NUM);
        }
    }

    g_usw_nh_master[lchip]->reflective_brg_en = 0;
    g_usw_nh_master[lchip]->ipmc_logic_replication = 0;

    g_usw_nh_master[lchip]->ip_use_l2edit = 0;
    g_usw_nh_master[lchip]->met_mode = 0;
    g_usw_nh_master[lchip]->cid_use_4w = 1;


    /*2. Install Semantic callback*/
    if (CTC_E_NONE != _sys_usw_nh_callback_init(lchip, g_usw_nh_master[lchip]))
    {
        mem_free( g_usw_nh_master[lchip]);
        g_usw_nh_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    if (CTC_E_NONE != sys_usw_nh_table_info_init(lchip))
    {
        mem_free( g_usw_nh_master[lchip]);
        g_usw_nh_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    /*3. Offset  init*/
    if (CTC_E_NONE != _sys_usw_nh_offset_init(lchip, p_master))
    {
        mem_free( g_usw_nh_master[lchip]);
        g_usw_nh_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    /*4. global cfg init*/
    if (CTC_E_NONE != sys_usw_nh_global_cfg_init(lchip))
    {
        mem_free( g_usw_nh_master[lchip]);
        g_usw_nh_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    if (CTC_E_NONE != sys_usw_nh_misc_init(lchip))
 	{
 	    mem_free(g_usw_nh_master[lchip]);
        g_usw_nh_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

 	}

    /* dump-db register */
    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_NEXTHOP, sys_usw_nexthop_dump_db));
    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_NEXTHOP, sys_usw_nh_wb_sync));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_nh_wb_restore(lchip));
    }

    /* set chip_capability */
    MCHIP_CAP(SYS_CAP_SPEC_ECMP_GROUP_NUM) = g_usw_nh_master[lchip]->max_ecmp_group_num + 1;
    MCHIP_CAP(SYS_CAP_SPEC_ECMP_MEMBER_NUM) = g_usw_nh_master[lchip]->max_ecmp;
    MCHIP_CAP(SYS_CAP_SPEC_EXTERNAL_NEXTHOP_NUM) = g_usw_nh_master[lchip]->max_external_nhid;
    MCHIP_CAP(SYS_CAP_SPEC_GLOBAL_DSNH_NUM) = g_usw_nh_master[lchip]->max_glb_nh_offset;
    MCHIP_CAP(SYS_CAP_SPEC_MPLS_TUNNEL_NUM) = g_usw_nh_master[lchip]->max_tunnel_id;
    sys_usw_ftm_query_table_entry_num(lchip, DsDlbFlowStateLeft_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_ECMP_DLB_FLOW_NUM) = entry_num;

    if (CTC_E_NONE != _sys_usw_nh_port_init(lchip, FALSE))
    {
        _sys_usw_nh_port_init(lchip, TRUE);
        mem_free( g_usw_nh_master[lchip]);
        g_usw_nh_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    return CTC_E_NONE;
}

#define ___________deinit___________
STATIC int32
_sys_usw_nh_free_node_data(void* node_data, void* user_data)
{
    uint8 type = 0;
    sys_nh_info_com_t* p_nhinfo = NULL;
    sys_nh_info_mcast_t* p_mcast = NULL;
    sys_nh_info_ipuc_t* p_ipuc = NULL;
    sys_nh_info_ecmp_t* p_ecmp = NULL;
    sys_nh_info_mpls_t* p_mpls = NULL;
    ctc_list_pointer_node_t* p_pos, * p_pos_next;
    sys_nh_mcast_meminfo_t* p_member;

    if (user_data)
    {
        type = *(uint8*)user_data;
        if (0 == type)
        {
            p_nhinfo=(sys_nh_info_com_t*)node_data;
            switch(p_nhinfo->hdr.nh_entry_type)
            {
                case SYS_NH_TYPE_MCAST:
                    p_mcast = (sys_nh_info_mcast_t*)(p_nhinfo);
                    CTC_LIST_POINTER_LOOP_DEL(p_pos, p_pos_next, &p_mcast->p_mem_list)
                    {
                        p_member = _ctc_container_of(p_pos, sys_nh_mcast_meminfo_t, list_head);
                        if (p_member->dsmet.vid_list)
                        {
                            mem_free(p_member->dsmet.vid_list);
                        }
                        mem_free(p_member);
                    }
                    break;
                case SYS_NH_TYPE_IPUC:
                    p_ipuc = (sys_nh_info_ipuc_t*)(p_nhinfo);
                    /*p_dsl2edit resource release in l2edithash free, here skip*/
                    if (p_ipuc->protection_path)
                    {
                        mem_free(p_ipuc->protection_path);
                    }
                    break;
                case SYS_NH_TYPE_ECMP:
                    p_ecmp = (sys_nh_info_ecmp_t*)(p_nhinfo);
                    if (p_ecmp->nh_array)
                    {
                        mem_free(p_ecmp->nh_array);
                    }
                    if (p_ecmp->member_id_array)
                    {
                        mem_free(p_ecmp->member_id_array);
                    }
                    if (p_ecmp->entry_count_array)
                    {
                        mem_free(p_ecmp->entry_count_array);
                    }
                    break;
                case SYS_NH_TYPE_MPLS:
                    p_mpls = (sys_nh_info_mpls_t*)(p_nhinfo);
                    /*p_dsl2edit resource release in l2edithash free, here skip*/
                    if (p_mpls->protection_path)
                    {
                        mem_free(p_mpls->protection_path);
                    }
                    break;
                case SYS_NH_TYPE_IP_TUNNEL:
                case SYS_NH_TYPE_WLAN_TUNNEL:
                case SYS_NH_TYPE_TRILL:
                    /*p_dsl2edit resource release in l2edithash free, here skip*/
                    break;

                default:
                    break;
            }

            if (p_nhinfo->hdr.p_ref_nh_list)
            {
                sys_nh_ref_list_node_t* p_ref_node, * p_tmp_node;
                p_ref_node = p_nhinfo->hdr.p_ref_nh_list;
                while (p_ref_node)
                {
                    /*Remove from db*/
                    p_tmp_node = p_ref_node;
                    p_ref_node = p_ref_node->p_next;
                    mem_free(p_tmp_node);
                }
            }
        }
    }
    mem_free(node_data);

    return CTC_E_NONE;
}
int32
sys_usw_nh_deinit(uint8 lchip)
{
    uint8 type = 0;
    ctc_slistnode_t* pnode = NULL;
    struct ctc_slistnode_s*  pnext   = NULL;
    sys_nh_eloop_node_t *p_eloop_node = NULL;
    sys_nh_repli_offset_node_t *p_offset_node = NULL;

    LCHIP_CHECK(lchip);
    if (NULL == g_usw_nh_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*free internal nhid vector*/
    ctc_hash_traverse(g_usw_nh_master[lchip]->nhid_hash, (hash_traversal_fn)_sys_usw_nh_free_node_data, &type);
    ctc_hash_free(g_usw_nh_master[lchip]->nhid_hash);

    /*free tunnel id vector*/
    ctc_vector_traverse(g_usw_nh_master[lchip]->tunnel_id_vec, (vector_traversal_fn)_sys_usw_nh_free_node_data, NULL);
    ctc_vector_release(g_usw_nh_master[lchip]->tunnel_id_vec);

    /*free mcast group nhid vector*/
    /*mcast vec data have freed in free nh info, here just free vec*/
    ctc_vector_release(g_usw_nh_master[lchip]->mcast_group_vec);

    /*free arp id hash*/
    ctc_hash_traverse(g_usw_nh_master[lchip]->arp_id_hash, (hash_traversal_fn)_sys_usw_nh_free_node_data, NULL);
    ctc_hash_free(g_usw_nh_master[lchip]->arp_id_hash);

    /*free l2edit data*/
    ctc_spool_free(g_usw_nh_master[lchip]->p_edit_spool);
    ctc_spool_free(g_usw_nh_master[lchip]->p_l2edit_vprof);

    /*free Eloop list*/
    CTC_SLIST_LOOP_DEL(g_usw_nh_master[lchip]->eloop_list, pnode, pnext)
    {
        p_eloop_node = _ctc_container_of(pnode, sys_nh_eloop_node_t, head);
        mem_free(p_eloop_node);
    }
    ctc_slist_free(g_usw_nh_master[lchip]->eloop_list);

    /*free logic rep list*/
    CTC_SLIST_LOOP_DEL(g_usw_nh_master[lchip]->repli_offset_list, pnode, pnext)
    {
        p_offset_node = _ctc_container_of(pnode, sys_nh_repli_offset_node_t, head);
        mem_free(p_offset_node);
    }
    ctc_slist_free(g_usw_nh_master[lchip]->repli_offset_list);

    mem_free(g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_4]);
    mem_free(g_usw_nh_master[lchip]->ipsa[CTC_IP_VER_6]);
    mem_free(g_usw_nh_master[lchip]->p_occupid_nh_offset_bmp);
    if (g_usw_nh_master[lchip]->max_glb_met_offset)
    {
        mem_free(g_usw_nh_master[lchip]->p_occupid_met_offset_bmp);
    }

    /*free vxlan hash*/
    ctc_hash_traverse(g_usw_nh_master[lchip]->vxlan_vni_hash, (hash_traversal_fn)_sys_usw_nh_free_node_data, NULL);
    ctc_hash_free(g_usw_nh_master[lchip]->vxlan_vni_hash);

    /*free opf resource*/
    sys_usw_opf_deinit(lchip, g_usw_nh_master[lchip]->ecmp_group_opf_type);
    sys_usw_opf_deinit(lchip, g_usw_nh_master[lchip]->ecmp_member_opf_type);
    sys_usw_opf_deinit(lchip, g_usw_nh_master[lchip]->nh_opf_type);
    sys_usw_opf_deinit(lchip, g_usw_nh_master[lchip]->vxlan_opf_type_inner_fid);

    mem_free(g_usw_nh_master[lchip]);

    return CTC_E_NONE;
}

#define ___________XGPON___________

int32
sys_usw_nh_update_logic_port(uint8 lchip, uint32 nhid, uint32 logic_port)
{
    uint32 dsnh_offset = 0;
    sys_nh_info_com_t*p_nhinfo = NULL;
    sys_nexthop_t dsnh_8w;

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo));
    if (NULL == p_nhinfo)
    {
        return CTC_E_NH_INVALID_NHID;
    }

    if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        return CTC_E_NOT_SUPPORT;
    }

    dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;

    CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, dsnh_offset, &dsnh_8w));

    dsnh_8w.logic_dest_port = logic_port;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, dsnh_offset, &dsnh_8w));


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_xgpon_reserve_mpls_edit(uint8 lchip)
{
    uint16 i = 0;
    uint32 offset_base = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_dsmpls_t  dsmpls;

    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 8192, &offset_base));
    p_nh_master->mpls_edit_offset = offset_base;

    sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));

    dsmpls.l3_rewrite_type = 1;
    dsmpls.ttl_index = 1;
    dsmpls.exp = 1;
    dsmpls.map_ttl_mode = 1;
    dsmpls.map_ttl = 1;
    for (i=0; i<8192; i++)
    {
        dsmpls.label = i+1;
        sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, offset_base+i, &dsmpls);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_xgpon_reserve_vlan_edit(uint8 lchip)
{
    uint16 i = 0;
    uint32 offset_base = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;
    sys_nh_param_dsnh_t dsnh_param;
    ctc_vlan_egress_edit_info_t vlan_edit_info;

    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 4096, &offset_base));
    p_nh_master->xlate_nh_offset = offset_base;

    sal_memset(&dsnh_param, 0, sizeof(dsnh_param));
    dsnh_param.dest_vlan_ptr = SYS_DSNH_DESTVLANPTR_SPECIAL;
    dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_BRGUC;

    sal_memset(&vlan_edit_info, 0, sizeof(vlan_edit_info));
    vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
    vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    CTC_SET_FLAG(vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
    CTC_SET_FLAG(vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
    CTC_SET_FLAG(vlan_edit_info.flag, CTC_VLAN_NH_ETREE_LEAF);
    vlan_edit_info.stag_cos = 0;

    dsnh_param.p_vlan_info = &vlan_edit_info;

    for (i=0; i<4096; i++)
    {
        vlan_edit_info.output_svid = i;
        dsnh_param.dsnh_offset = offset_base+i;
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param));
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_is_logic_repli_en(uint8 lchip, uint32 nhid, uint8 *is_repli_en)
{
    sys_nh_info_com_t* p_nhinfo = NULL;

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo));
    *is_repli_en = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_REPLI_EN);

    return CTC_E_NONE;
}

extern int32
sys_usw_port_set_isolation_mode(uint8 lchip, uint8 choice_mode);
int32
sys_usw_nh_set_xgpon_en(uint8 lchip, uint8 enable, uint32 flag)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;

    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    /*rsv group 0 for common group*/
    if (enable)
    {
        CTC_BIT_UNSET(p_nh_master->p_occupid_met_offset_bmp[0], 0);
        field_val = 1;
        if (DRV_IS_DUET2(lchip)&& (!CTC_FLAG_ISSET(flag, 1<<1)))
        {
            CTC_ERROR_RETURN(_sys_usw_nh_xgpon_reserve_vlan_edit(lchip));
        }
        if (DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_RETURN(_sys_usw_nh_xgpon_reserve_mpls_edit(lchip));
        }
        /*merge dsfwd mode*/
        sys_usw_nh_set_dsfwd_mode(lchip, 0);
        sys_usw_port_set_isolation_mode(lchip, 1);
    }
    else
    {
        CTC_BIT_SET(p_nh_master->p_occupid_met_offset_bmp[0], 0);
        field_val = 0;
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 4096, p_nh_master->xlate_nh_offset));
        if (DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_MPLS, 8192, p_nh_master->mpls_edit_offset));
        }
    }

    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_loopbackUseSourcePort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_nh_set_mcast_bitmap_ptr(uint8 lchip, uint32 offset)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;

    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    field_val = p_nh_master->xlate_nh_offset + offset;
    cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_portBitmapNextHopPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

 /*DsL2EditFlex for gem port*/
int32
sys_usw_nh_add_gem_port_l2edit_8w_outer(uint8 lchip, uint16 logic_dest_port, uint16 gem_vlan)
{
    uint32 offset = 0;
    sys_nh_db_dsl2editeth8w_t new_dsl2edit_8w;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add gem port logic_dest:%d ,gem_port %d!!\n",
                   logic_dest_port, gem_vlan);

    offset = (logic_dest_port>>3)*2;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", offset);
    sal_memset(&new_dsl2edit_8w, 0, sizeof(new_dsl2edit_8w));
    /* The output_vlan_id and output_vid represent the gem_vlan*/
    new_dsl2edit_8w.output_vid = gem_vlan;
    /* The output_vlan_is_svlan and dynamic represent the logic_dest_port*/
    new_dsl2edit_8w.output_vlan_is_svlan = (logic_dest_port>>8)&0xFF;
    new_dsl2edit_8w.dynamic = logic_dest_port&0xFF;
    CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth8w(lchip, offset, &new_dsl2edit_8w, SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W));
    return CTC_E_NONE;
}

int32
sys_usw_nh_remove_gem_port_l2edit_8w_outer(uint8 lchip, uint16 logic_dest_port)
{
    uint32 offset = 0;
    sys_nh_db_dsl2editeth8w_t new_dsl2edit_8w;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add gem port logic_dest:%d!!\n",
                   logic_dest_port);

    offset = (logic_dest_port>>3)*2;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit offset=%d\n", offset);
    sal_memset(&new_dsl2edit_8w, 0, sizeof(new_dsl2edit_8w));
    /* The output_vlan_id and output_vid represent the gem_vlan*/
    new_dsl2edit_8w.output_vid = 0;
    /* The output_vlan_is_svlan and dynamic represent the logic_dest_port*/
    new_dsl2edit_8w.output_vlan_is_svlan = (logic_dest_port>>8)&0xFF;
    new_dsl2edit_8w.dynamic = logic_dest_port&0xFF;
    CTC_ERROR_RETURN(_sys_usw_nh_write_entry_dsl2editeth8w(lchip, offset, &new_dsl2edit_8w, SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W));
    return CTC_E_NONE;
}

int32
sys_usw_nh_update_dsma(uint8 lchip, void* p_nh_info, void* user_date)
{
    uint32 cmd = 0;
    sys_nh_info_mpls_t* p_info = (sys_nh_info_mpls_t*)p_nh_info;
    uint8 protect_en = *((bool*)user_date)?1:0;

    if (p_info && p_info->dsma_valid)
    {
        /*update dsma*/
        DsMa_m ds_ma;
        sal_memset(&ds_ma, 0, sizeof(DsMa_m));

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update dsma protecting path,ma_index:%u, nhid:%u\n", p_info->ma_idx, p_info->nhid);

        cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_info->ma_idx, cmd, &ds_ma));
        SetDsMa(V, protectingPath_f, &ds_ma, (protect_en?1:0));
        CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, p_info->ma_idx, &ds_ma));
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_set_efd_redirect_nh(uint8 lchip, uint32 nh_id)
{
    SYS_NH_LOCK;
    g_usw_nh_master[lchip]->efd_nh_id = nh_id;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_efd_redirect_nh(uint8 lchip, uint32* p_nh_id)
{
    SYS_NH_LOCK;
    *p_nh_id = g_usw_nh_master[lchip]->efd_nh_id;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

