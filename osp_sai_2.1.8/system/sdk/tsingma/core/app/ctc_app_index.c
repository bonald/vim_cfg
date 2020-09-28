#include "sal_types.h"
#include "ctc_const.h"
#include "ctc_opf.h"
#include "ctc_error.h"
#include "sal.h"
#include "ctc_nexthop.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_app.h"
#include "ctc_register.h"
#include "ctc_app_index.h"




uint32 g_opf_mapping[CTC_APP_INDEX_TYPE_MAX][3] =
{
    {CTC_OPF_MPLS_TUNNEL_ID, CTC_GLOBAL_CAPABILITY_MPLS_TUNNEL_NUM, 0},
    {CTC_OPF_NHID, CTC_GLOBAL_CAPABILITY_EXTERNAL_NEXTHOP_NUM, CTC_NH_RESERVED_NHID_MAX}, /* refer to ctc_nh_reserved_nhid_t*/
    {CTC_OPF_L3IF_ID, CTC_GLOBAL_CAPABILITY_L3IF_NUM, MIN_CTC_L3IF_ID},   /* 0 is used to disable L3if*/
    {CTC_OPF_MCAST_GROUP_ID, CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM, 1},   /*mcast group 0 is reserved for tx c2c packet on stacking env */
    {CTC_OPF_GLOBAL_DSNH_OFFSET, CTC_GLOBAL_CAPABILITY_GLOBAL_DSNH_NUM, CTC_NH_RESERVED_DSNH_OFFSET_MAX}, /* refer to ctc_nh_reserved_dsnh_offset_t*/
    {CTC_OPF_POLICER_ID, CTC_GLOBAL_CAPABILITY_POLICER_NUM, 1}, /* 0 is used to disable policer*/
    {CTC_OPF_STATSID_ID, CTC_GLOBAL_CAPABILITY_TOTAL_STATS_NUM, 1}, /* 0 is used to disable stats*/
    {CTC_OPF_LOGIC_PORT, CTC_GLOBAL_CAPABILITY_LOGIC_PORT_NUM, 1},/* 0 is used to disable logic port*/
    {CTC_OPF_APS_GROUP_ID, CTC_GLOBAL_CAPABILITY_APS_GROUP_NUM, 0},
    {CTC_OPF_ARP_ID, CTC_GLOBAL_CAPABILITY_ARP_ID_NUM, 1},/* 0 is used to disable arp*/

    /*scl/acl entry/group id, have no relationship with entry num of chip capability*/
    {CTC_OPF_ACL_ENTRY_ID, 0, 0},
    {CTC_OPF_ACL_GROUP_ID, 0, 0},
    {CTC_OPF_SCL_ENTRY_ID, 0, 0},
    {CTC_OPF_SCL_GROUP_ID, 0, 0}
};

uint8 g_chipset_type[CTC_MAX_GCHIP_CHIP_ID+1] =
{
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096,
    CTC_APP_CHIPSET_CTC8096
};

int32
ctc_app_index_get_lchip_id(uint8 gchip, uint8* lchip)
{
    uint8 lchip_id = 0;
    uint8 gchip_id = 0;
    uint8 lchip_num = 0;

    if (NULL == lchip)
    {
        return -1;
    }

    if (g_ctcs_api_en)
    {
        ctcs_get_local_chip_num(0, &lchip_num);
    }
    else
    {
        ctc_get_local_chip_num(&lchip_num);
    }

    for (lchip_id = 0; lchip_id < lchip_num; lchip_id++)
    {
        if (g_ctcs_api_en)
        {
            ctcs_get_gchip_id(lchip_id, &gchip_id);
        }
        else
        {
            ctc_get_gchip_id(lchip_id, &gchip_id);
        }

        if (gchip == gchip_id)
        {
            *lchip = lchip_id;
            return 0;
        }
    }

    return -1;
}

int32 ctc_app_index_get_dsnh_entry_num_greatbelt(ctc_app_index_t *app_index, uint8 *multiple)
{

    uint8 dsnh_8w = 0;
    uint8 use_rsv_dsnh = 0;
    uint8 entry_num = 0;

    if (!app_index->p_nh_param)
    {
        return CTC_E_INVALID_PARAM;
    }
    switch (app_index->nh_type)
    {
        case CTC_NH_TYPE_XLATE:
            {
                ctc_vlan_edit_nh_param_t* p_xlate = (ctc_vlan_edit_nh_param_t*)app_index->p_nh_param;
                if (!p_xlate->aps_en)
                {
                    if ((0 == p_xlate->vlan_edit_info.flag)
                        &&(0 == p_xlate->vlan_edit_info.edit_flag)
                    &&(0 == p_xlate->vlan_edit_info.loop_nhid)
                    && (p_xlate->vlan_edit_info.svlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE
                    && p_xlate->vlan_edit_info.cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE))
                    {
                        use_rsv_dsnh = 1;
                        entry_num = 0;

                    }
                    else
                    {
                        entry_num  = 1;

                        if ((p_xlate->vlan_edit_info.loop_nhid)
                            || (CTC_FLAG_ISSET(p_xlate->vlan_edit_info.flag, CTC_VLAN_NH_MTU_CHECK_EN)))
                        {
                            dsnh_8w = 1;
                        }
                    }
                }
                else
                {
                    entry_num  = 2;
                    if ((CTC_FLAG_ISSET(p_xlate->vlan_edit_info_p.flag, CTC_VLAN_NH_MTU_CHECK_EN))
                        || (CTC_FLAG_ISSET(p_xlate->vlan_edit_info_p.flag, CTC_VLAN_NH_MTU_CHECK_EN)))
                    {
                        dsnh_8w = 1;
                    }
                }
            }
            break;

        case CTC_NH_TYPE_IPUC:
            {
                ctc_ip_nh_param_t* p_ip_nh = NULL;
                p_ip_nh = (ctc_ip_nh_param_t*)app_index->p_nh_param;
                if (p_ip_nh->aps_en)
                {
                    entry_num  = 2;
                }
                else
                {
                    entry_num  = 1;
                }
            }
            break;

        case CTC_NH_TYPE_IP_TUNNEL:
            {
                entry_num  = 1;
            }
            break;

        case CTC_NH_TYPE_MISC:
            {
                ctc_misc_nh_param_t* p_misc_param = (ctc_misc_nh_param_t*)app_index->p_nh_param;
                if (CTC_MISC_NH_TYPE_TO_CPU == p_misc_param->type)
                {
                    entry_num  = 0;
                }
                else
                {
                    entry_num  = 1;
                    if (CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR == p_misc_param->type)
                    {
                        dsnh_8w = 1;
                    }
                }
            }
            break;

        case CTC_NH_TYPE_RSPAN:
            {
                entry_num  = 1;
            }
            break;

        case CTC_NH_TYPE_MPLS:
            {
                ctc_mpls_nexthop_param_t* p_mpls_nh_param = (ctc_mpls_nexthop_param_t*)app_index->p_nh_param;
                ctc_vlan_egress_edit_info_t* p_vlan_info = &p_mpls_nh_param->nh_para.nh_param_push.nh_com.vlan_info;

                if ((p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
                    (p_mpls_nh_param->nh_para.nh_param_push.nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN)
                && (p_vlan_info != NULL)
                && ((CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID))
                || ((CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
                || (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID)))))
                {
                    dsnh_8w = 1;
                }

                if (p_mpls_nh_param->aps_en)
                {
                    entry_num  = 2;
                }
                else
                {
                    entry_num  = 1;
                }
            }
            break;

        case CTC_NH_TYPE_ECMP:
        case CTC_NH_TYPE_MCAST:
        case CTC_NH_TYPE_ILOOP:
        case CTC_NH_TYPE_L2UC:
            entry_num  = 0;
            break;

        default:
            return CTC_E_INVALID_PARAM;

    }
    if (use_rsv_dsnh)
    {
        entry_num = 0;
    }
    if (dsnh_8w)
    {
        app_index->entry_num = entry_num *2 ;
        *multiple = 2;
    }
    else
    {
        *multiple = 1;
        app_index->entry_num = entry_num ;
    }
    return CTC_E_NONE;
}

int32
ctc_app_index_get_dsnh_entry_num_goldengate(ctc_app_index_t *app_index, uint8 *multiple)
{
    uint8 dsnh_8w = 0;
    uint8 use_rsv_dsnh = 0;
    uint8 entry_num = 0;


    switch(app_index->nh_type)
    {
        /*CTC_NH_TYPE_XLATE*/
    case CTC_NH_TYPE_XLATE:
        {
            ctc_vlan_edit_nh_param_t* p_xlate = NULL;
            if (!app_index->p_nh_param)
            {
                return CTC_E_INVALID_PARAM;
            }
            p_xlate = (ctc_vlan_edit_nh_param_t*)app_index->p_nh_param;
            if (!p_xlate->aps_en)
            {
                if ((0 == p_xlate->vlan_edit_info.flag)
                    &&(0 == p_xlate->vlan_edit_info.edit_flag)
                &&(0 == p_xlate->vlan_edit_info.loop_nhid)
                && (p_xlate->vlan_edit_info.svlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE)
                && (p_xlate->vlan_edit_info.cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE))
                {
                    entry_num = 1;
                    use_rsv_dsnh = 1;
                }
                else
                {
                    entry_num  = 1;
                    if ((p_xlate && (p_xlate->logic_port_valid))
                        ||(p_xlate->vlan_edit_info.loop_nhid))
                    {
                        dsnh_8w = 1;
                    }
                }
            }
            else
            {
                entry_num  = 2;
            }

            break;
        }

        /*CTC_NH_TYPE_IPUC*/
    case CTC_NH_TYPE_IPUC:
        {
            ctc_ip_nh_param_t* p_ip_nh = NULL;
            p_ip_nh = (ctc_ip_nh_param_t*)app_index->p_nh_param;
            if (p_ip_nh->aps_en)
            {
                entry_num  = 2;
            }
            else
            {
                entry_num  = 1;
            }
            break;
        }

        /*CTC_NH_TYPE_IP_TUNNEL*/
    case CTC_NH_TYPE_IP_TUNNEL:
        {
            ctc_ip_tunnel_nh_param_t* p_ip_nh_param = NULL;


            if (!app_index->p_nh_param)
            {
                return CTC_E_INVALID_PARAM;
            }
            p_ip_nh_param = (ctc_ip_tunnel_nh_param_t*)app_index->p_nh_param;
            if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI)
                || (p_ip_nh_param->tunnel_info.logic_port)
            || (p_ip_nh_param->tunnel_info.inner_packet_type == PKT_TYPE_IPV4 )
            || (p_ip_nh_param->tunnel_info.inner_packet_type == PKT_TYPE_IPV6 )
            || CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR)
            || (0 != p_ip_nh_param->tunnel_info.vn_id))
            {
                dsnh_8w = 1;
            }
            entry_num  = 1;

            break;
        }

    case CTC_NH_TYPE_TRILL:
        {
            entry_num  = 1;
            break;
        }


    case CTC_NH_TYPE_MISC:
        {
            ctc_misc_nh_param_t* p_misc_param ;
            if (!app_index->p_nh_param)
            {
                return CTC_E_INVALID_PARAM;
            }
            p_misc_param = (ctc_misc_nh_param_t*)app_index->p_nh_param;
            if (CTC_MISC_NH_TYPE_TO_CPU == p_misc_param->type)
            {
                entry_num = 0;
            }
            else
            {
                entry_num  = 1;
            }

            if ((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS) == p_misc_param->type || (CTC_MISC_NH_TYPE_OVER_L2 == p_misc_param->type))
            {
                dsnh_8w = 1;
            }
            break;
        }

        /*CTC_NH_TYPE_RSPAN*/
    case CTC_NH_TYPE_RSPAN:
        {
            entry_num  = 1;
            break;
        }

        /*CTC_NH_TYPE_MPLS*/
    case CTC_NH_TYPE_MPLS:
        {
            ctc_mpls_nexthop_param_t* p_mpls_nh_param ;
            if (!app_index->p_nh_param)
            {
                return CTC_E_INVALID_PARAM;
            }
            p_mpls_nh_param = (ctc_mpls_nexthop_param_t*)app_index->p_nh_param;
            if ((p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
                (p_mpls_nh_param->nh_para.nh_param_push.nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN))
            {
                dsnh_8w = 1;
            }
            if (p_mpls_nh_param->aps_en)
            {
                entry_num = 2;
            }
            else
            {
                entry_num = 1;
            }

            break;
        }
    case CTC_NH_TYPE_ECMP:
    case CTC_NH_TYPE_MCAST:
    case CTC_NH_TYPE_ILOOP:
    case CTC_NH_TYPE_L2UC:
        entry_num  = 0;
        break;

    default:
        return CTC_E_INVALID_PARAM;

    }

    if (use_rsv_dsnh)
    {
        entry_num = 0;
    }
    if (dsnh_8w)
    {
        app_index->entry_num = entry_num *2 ;
        *multiple = 2;
    }
    else
    {
        app_index->entry_num = entry_num;
        *multiple = 1;
    }
    return CTC_E_NONE;
}

int32
ctc_app_index_get_dsnh_entry_num_duet2(ctc_app_index_t *app_index, uint8 *multiple)
{
    uint8 dsnh_8w = 0;
    uint8 use_rsv_dsnh = 0;
    uint8 entry_num = 0;
    uint8 use_dsnh8w = 0;

    switch(app_index->nh_type)
    {
        /*CTC_NH_TYPE_XLATE*/
        case CTC_NH_TYPE_XLATE:
            {
                ctc_vlan_edit_nh_param_t* p_xlate = NULL;
                if (!app_index->p_nh_param)
                {
                    return CTC_E_INVALID_PARAM;
                }
                p_xlate = (ctc_vlan_edit_nh_param_t*)app_index->p_nh_param;
                if (!p_xlate->aps_en)
                {
                    if ((0 == p_xlate->vlan_edit_info.flag)
                        &&(0 == p_xlate->vlan_edit_info.edit_flag)
                    &&(0 == p_xlate->vlan_edit_info.loop_nhid)
                    && (p_xlate->vlan_edit_info.svlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE)
                    && (p_xlate->vlan_edit_info.cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_NONE)
                    && (!p_xlate->logic_port)
                    && (!p_xlate->logic_port_check))
                    {
                        entry_num = 1;
                        use_rsv_dsnh = 1;
                    }
                    else
                    {
                        entry_num  = 1;
                        use_dsnh8w = (p_xlate->logic_port ||p_xlate->logic_port_check)
                            && (p_xlate->vlan_edit_info.cvlan_edit_type != CTC_VLAN_EGRESS_EDIT_NONE)
                            && (p_xlate->vlan_edit_info.cvlan_edit_type != CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE);

                        if (use_dsnh8w
                            || p_xlate->vlan_edit_info.cvlan_edit_type == CTC_VLAN_EGRESS_EDIT_STRIP_VLAN
                            || (p_xlate->vlan_edit_info.loop_nhid && (p_xlate->vlan_edit_info.svlan_edit_type != CTC_VLAN_EGRESS_EDIT_NONE)
                        && (p_xlate->vlan_edit_info.cvlan_edit_type != CTC_VLAN_EGRESS_EDIT_NONE)))
                        {
                            dsnh_8w = 1;
                        }
                    }
                }
                else
                {
                    entry_num  = 2;
                }

                break;
            }

        /*CTC_NH_TYPE_IPUC*/
        case CTC_NH_TYPE_IPUC:
            {
                ctc_ip_nh_param_t* p_ip_nh = NULL;
                p_ip_nh = (ctc_ip_nh_param_t*)app_index->p_nh_param;
                if (p_ip_nh->aps_en)
                {
                    entry_num  = 2;
                }
                else
                {
                    entry_num  = 1;
                }
                break;
            }

        /*CTC_NH_TYPE_IP_TUNNEL*/
        case CTC_NH_TYPE_IP_TUNNEL:
            {
                ctc_ip_tunnel_nh_param_t* p_ip_nh_param = NULL;


                if (!app_index->p_nh_param)
                {
                    return CTC_E_INVALID_PARAM;
                }
                p_ip_nh_param = (ctc_ip_tunnel_nh_param_t*)app_index->p_nh_param;

                if (CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI)
                    || (p_ip_nh_param->tunnel_info.logic_port)
                    || (p_ip_nh_param->tunnel_info.inner_packet_type == PKT_TYPE_IPV4 )
                    || (p_ip_nh_param->tunnel_info.inner_packet_type == PKT_TYPE_IPV6 )
                    || CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR)
                    || (0 != p_ip_nh_param->tunnel_info.vn_id)
                    ||CTC_FLAG_ISSET(p_ip_nh_param->tunnel_info.flag,CTC_IP_NH_TUNNEL_FLAG_LOGIC_PORT_CHECK) )
                {
                    dsnh_8w = 1;
                }
                entry_num  = 1;

                break;
            }

        case CTC_NH_TYPE_TRILL:
            {
                entry_num  = 1;
                break;
            }


        case CTC_NH_TYPE_MISC:
            {
                ctc_misc_nh_param_t* p_misc_param ;
                if (!app_index->p_nh_param)
                {
                    return CTC_E_INVALID_PARAM;
                }
                p_misc_param = (ctc_misc_nh_param_t*)app_index->p_nh_param;
                if (CTC_MISC_NH_TYPE_TO_CPU == p_misc_param->type)
                {
                    entry_num = 0;
                }
                else
                {
                    entry_num  = 1;
                }

                if ((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == p_misc_param->type) ||
                    (CTC_MISC_NH_TYPE_OVER_L2 == p_misc_param->type) || !p_misc_param->stats_id)
                {
                    dsnh_8w = 1;
                }

                break;
            }

            /*CTC_NH_TYPE_RSPAN*/
        case CTC_NH_TYPE_RSPAN:
            {
                entry_num  = 1;
                break;
            }

            /*CTC_NH_TYPE_MPLS*/
        case CTC_NH_TYPE_MPLS:
            {
                ctc_mpls_nexthop_param_t* p_mpls_nh_param ;
                if (!app_index->p_nh_param)
                {
                    return CTC_E_INVALID_PARAM;
                }
                p_mpls_nh_param = (ctc_mpls_nexthop_param_t*)app_index->p_nh_param;
                if ((p_mpls_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE) &&
                    (p_mpls_nh_param->nh_para.nh_param_push.nh_com.opcode == CTC_MPLS_NH_PUSH_OP_L2VPN))
                {
                    dsnh_8w = 1;
                }
                if (p_mpls_nh_param->aps_en)
                {
                    entry_num = 2;
                }
                else
                {
                    entry_num = 1;
                }

                break;
            }
        case CTC_NH_TYPE_ECMP:
        case CTC_NH_TYPE_MCAST:
        case CTC_NH_TYPE_ILOOP:
        case CTC_NH_TYPE_L2UC:
            entry_num  = 0;
            break;

        default:
            return CTC_E_INVALID_PARAM;

    }

    if (use_rsv_dsnh)
    {
        entry_num = 0;
    }
    if (dsnh_8w)
    {
        app_index->entry_num = entry_num *2 ;
        *multiple = 2;
    }
    else
    {
        app_index->entry_num = entry_num;
        *multiple = 1;
    }
    return CTC_E_NONE;
}


int32
ctc_app_index_set_chipset_type(uint8 gchip, uint8 type)
{
    if (gchip > CTC_MAX_GCHIP_CHIP_ID )
    {
        return CTC_E_INVALID_CHIP_ID;
    }
    g_chipset_type[gchip] = type;
    return CTC_E_NONE;
}
int32
ctc_app_index_set_rchip_dsnh_offset_range(uint8 gchip, uint32 min_index, uint32 max_index)
{
    ctc_opf_t opf;
    sal_memset(&opf, 0, sizeof(ctc_opf_t));

    if (gchip > CTC_MAX_GCHIP_CHIP_ID )
    {
        return CTC_E_INVALID_CHIP_ID;
    }
    opf.pool_type = CTC_OPF_GLOBAL_DSNH_OFFSET;
    opf.pool_index = gchip;

    if (max_index >= min_index)
    {
        CTC_ERROR_RETURN(ctc_opf_init_offset(&opf, min_index, (max_index - min_index + 1)));
    }
    else
    {
        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "param error!! min_index = %d, max_index = %d\n", min_index, max_index);
    }
    return CTC_E_NONE;
}

int32
ctc_app_index_init(uint8 gchip)
{
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] ={0};
    uint32 max_index = 0;
    uint32 min_index = 0;
    uint8 loop = 0;
    uint8 lchip = 0;

    ctc_opf_t opf;
    sal_memset(&opf, 0, sizeof(ctc_opf_t));


    /*
    the index range according to sdk api (CTC_GLOBAL_CHIP_CAPABILITY) or sdk chip profile
    !!!!!!!!    Warning  !!!!!!
    !!!!!!!!attention please!!!!!!
    Users must modify the funtion ctc_app_index_init() according to the actual application
    scenarios, otherwise it can not work

    !!!!!!!!    Warning  !!!!!!

    */
    ////////////////////////need modify code//////////////////////////
    ctc_app_index_get_lchip_id(gchip, &lchip);
    if (g_ctcs_api_en)
    {

        CTC_ERROR_RETURN(ctcs_global_ctl_get(lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability));
    }
    else
    {
        CTC_ERROR_RETURN(ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability));

    }
    //////////////////////////need modify code//////////////////////////////
    for (loop = 0; loop < CTC_APP_INDEX_TYPE_MAX; loop++)
    {
        uint32 forward_bound = 0;
        uint32 reverse_bound = 0;
        CTC_ERROR_RETURN(ctc_opf_init(g_opf_mapping[loop][0], (CTC_MAX_GCHIP_CHIP_ID + 1)));
        opf.pool_type = g_opf_mapping[loop][0];
        opf.pool_index = gchip;

        ctc_opf_get_bound(&opf, &forward_bound, &reverse_bound);
        if (forward_bound || reverse_bound)
        {
            /*avoid init the same opf multi times*/
            continue;
        }

        if (CTC_APP_INDEX_TYPE_SCL_ENTRY_ID == loop)
        {
            max_index = CTC_SCL_MAX_USER_ENTRY_ID;
            min_index = CTC_SCL_MIN_USER_ENTRY_ID;
        }
        else if(CTC_APP_INDEX_TYPE_ACL_ENTRY_ID == loop)
        {
            max_index = CTC_ACL_MAX_USER_ENTRY_ID - 1;
            min_index = CTC_ACL_MIN_USER_ENTRY_ID;
        }
        else if(CTC_APP_INDEX_TYPE_ACL_GROUP_ID == loop)
        {
            max_index = CTC_ACL_GROUP_ID_NORMAL;
            min_index = 0;
        }
        else if(CTC_APP_INDEX_TYPE_SCL_GROUP_ID == loop)
        {
            max_index = CTC_SCL_GROUP_ID_MAX_NORMAL;
            min_index = CTC_SCL_GROUP_ID_MIN_NORMAL;
        }
        else
        {
            max_index = capability[g_opf_mapping[loop][1]]? (capability[g_opf_mapping[loop][1]]- 1):0;
            min_index = g_opf_mapping[loop][2];
        }

        if (max_index >= min_index )
        {
            CTC_ERROR_RETURN(ctc_opf_init_offset(&opf, min_index, (max_index - min_index + 1)));
            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "loop = %d, opf type = %d, min_index = %d, max_index = %u\n",
                            loop, opf.pool_type, min_index, max_index);
        }
    }
    return CTC_E_NONE;
}

int32
ctc_app_index_alloc_dsnh_offset(uint8 gchip ,uint8 nh_type ,void* nh_param,
                                uint32 *dsnh_offset,uint32 *entry_num)
{
   int32 ret = 0;
   ctc_app_index_t app_index;
   sal_memset(&app_index, 0, sizeof(app_index));
   app_index.index_type = CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET;
   app_index.gchip = gchip;
   app_index.nh_type = nh_type;
   app_index.p_nh_param = nh_param;

   ret = ctc_app_index_alloc(&app_index);
   *dsnh_offset = app_index.index;
   *entry_num = app_index.entry_num;
   return ret;
}

int32
ctc_app_index_alloc(ctc_app_index_t* app_index)
{
    ctc_opf_t opf;
    sal_memset(&opf, 0, sizeof(ctc_opf_t));

    CTC_PTR_VALID_CHECK(app_index);

    if (app_index->index_type >= CTC_APP_INDEX_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    if (app_index->gchip > CTC_MAX_GCHIP_CHIP_ID )
    {
        return CTC_E_INVALID_CHIP_ID;
    }

    opf.pool_index = app_index->gchip;
    opf.pool_type = g_opf_mapping[app_index->index_type][0];
    opf.reverse = 0;

    if (CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET == app_index->index_type)
    {
        if (g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC8096 )
        {
            ctc_app_index_get_dsnh_entry_num_goldengate(app_index, &opf.multiple);
        }
        else if(g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC5160 )
        {
            ctc_app_index_get_dsnh_entry_num_greatbelt(app_index, &opf.multiple);
        }
        else if ((g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC7148) || (g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC7132))
        {
            ctc_app_index_get_dsnh_entry_num_duet2(app_index, &opf.multiple);
        }
    }
    else
    {
        if (0 == app_index->entry_num)
        {
            app_index->entry_num = 1;
        }
    }
    if (app_index->entry_num)
    {
        CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, app_index->entry_num, &(app_index->index)));
    }
    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc opf, type = %d, block_size = %d, multiple = %d, offset = %d\n",
                    opf.pool_type, app_index->entry_num, opf.multiple, app_index->index);

    return CTC_E_NONE;

}

int32
ctc_app_index_free(ctc_app_index_t* app_index)
{
    ctc_opf_t opf;


    CTC_PTR_VALID_CHECK(app_index);

    if (app_index->index_type >= CTC_APP_INDEX_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    if (app_index->gchip > CTC_MAX_GCHIP_CHIP_ID )
    {
        return CTC_E_INVALID_CHIP_ID;
    }

    sal_memset(&opf, 0, sizeof(ctc_opf_t));
    opf.pool_index = app_index->gchip;
    opf.pool_type = g_opf_mapping[app_index->index_type][0];

    if (CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET == app_index->index_type)
    {
        if (g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC8096 )
        {
            ctc_app_index_get_dsnh_entry_num_goldengate(app_index, &opf.multiple);
        }
        else if(g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC5160 )
        {
            ctc_app_index_get_dsnh_entry_num_greatbelt(app_index, &opf.multiple);
        }
        else if(g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC7148 )
        {
            ctc_app_index_get_dsnh_entry_num_duet2(app_index, &opf.multiple);
        }
    }
    else
    {
        if (0 == app_index->entry_num)
        {
            app_index->entry_num = 1;
        }
    }

    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free opf, type = %d, offset = %d, block_size = %d\n",
                    opf.pool_type, app_index->index, app_index->entry_num);
    CTC_ERROR_RETURN(ctc_opf_free_offset(&opf, app_index->entry_num, app_index->index));

    return CTC_E_NONE;
}

int32
ctc_app_index_alloc_from_position(ctc_app_index_t* app_index)
{
    ctc_opf_t opf;
    sal_memset(&opf, 0, sizeof(ctc_opf_t));

    CTC_PTR_VALID_CHECK(app_index);

    if (app_index->index_type >= CTC_APP_INDEX_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    if (app_index->gchip > CTC_MAX_GCHIP_CHIP_ID )
    {
        return CTC_E_INVALID_CHIP_ID;
    }

    opf.pool_index = app_index->gchip;
    opf.pool_type = g_opf_mapping[app_index->index_type][0];
    opf.reverse = 0;

    if (CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET == app_index->index_type)
    {
        if (g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC8096 )
        {
            ctc_app_index_get_dsnh_entry_num_goldengate(app_index, &opf.multiple);
        }
        else if(g_chipset_type[app_index->gchip] == CTC_APP_CHIPSET_CTC5160 )
        {
            ctc_app_index_get_dsnh_entry_num_greatbelt(app_index, &opf.multiple);

        }
    }
    else
    {
        if (0 == app_index->entry_num)
        {
            app_index->entry_num = 1;
        }
    }
    if (app_index->entry_num)
    {
        CTC_ERROR_RETURN(ctc_opf_alloc_offset_from_position(&opf, app_index->entry_num, app_index->index));
    }
    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc opf, type = %d, block_size = %d, multiple = %d, offset = %d\n",
                    opf.pool_type, app_index->entry_num, opf.multiple, app_index->index);

    return CTC_E_NONE;

}
