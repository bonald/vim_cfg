/**
 @file sys_usw_internal_port.c

 @date 2010-03-29

 @version v2.0

*/

/****************************************************************************
 *  current queue allocation scheme (Alt + F12 for pretty):
 *
 *           type          port id    channel id      queue number    queue id range
 *     ----------------  ----------   ----------      ------------    --------------
 *      internal port     64 - 191       X              4 * 128        708 - 1219

 ****************************************************************************/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "ctc_debug.h"
#include "ctc_macro.h"
#include "ctc_error.h"
#include "ctc_vector.h"
#include "ctc_const.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_ftm.h"
#include "sys_usw_opf.h"
#include "sys_usw_l3if.h"
#include "sys_usw_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_dmps.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/
sys_inter_port_master_t* inter_port_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};


#define SYS_INTERNAL_PORT_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (inter_port_master[lchip] == NULL){ \
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

enum sys_usw_internal_port_op_type_e
{
    SYS_USW_INTERNAL_PORT_OP_TYPE_SET = 1,
    SYS_USW_INTERNAL_PORT_OP_TYPE_FREE,
    SYS_USW_INTERNAL_PORT_OP_TYPE_ALLOC,
    SYS_USW_INTERNAL_PORT_OP_TYPE_NUM = SYS_USW_INTERNAL_PORT_OP_TYPE_ALLOC
};
typedef enum sys_usw_internal_port_op_type_e sys_usw_internal_port_op_type_t;

#define SYS_INTERNAL_PORT_NUM        (SYS_INTERNAL_PORT_END - SYS_INTERNAL_PORT_START +  1)

#define SYS_INTERNAL_PORT_SET(lchip, dir, port)   CTC_BIT_SET(inter_port_master[lchip]->is_used[dir][(port)/BITS_NUM_OF_WORD], (port)% BITS_NUM_OF_WORD)
#define SYS_INTERNAL_PORT_UNSET(lchip, dir, port) CTC_BIT_UNSET(inter_port_master[lchip]->is_used[dir][(port)/BITS_NUM_OF_WORD], (port)% BITS_NUM_OF_WORD)
#define SYS_INTERNAL_PORT_ISSET(lchip, dir, port) CTC_IS_BIT_SET(inter_port_master[lchip]->is_used[dir][(port)/BITS_NUM_OF_WORD], (port)% BITS_NUM_OF_WORD)

/****************************************************************************
 *
 * Function
 *
 ****************************************************************************/
#define __________INTERNAL_FUNCTION__________

STATIC int32
_sys_usw_internal_port_set_channel(uint8 lchip, uint16 lport, ctc_internal_port_type_t type, uint16 channel, uint8 is_bind)
{
    uint16 chan_id = 0;
    uint16 drv_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);

    if (CTC_INTERNAL_PORT_TYPE_ILOOP == type)
    {
        chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
    }
    else if (CTC_INTERNAL_PORT_TYPE_ELOOP == type)
    {

       chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);

    }
    else if (CTC_INTERNAL_PORT_TYPE_FWD == type)
    {
        chan_id = channel;
    }
    else
    {
        /* CTC_INTERNAL_PORT_TYPE_DISCARD */
        chan_id = MCHIP_CAP(SYS_CAP_CHANID_DROP);
    }

    if (is_bind)
    {
        CTC_ERROR_RETURN(sys_usw_add_port_to_channel(lchip, drv_lport, chan_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_remove_port_from_channel(lchip, drv_lport, chan_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_port_bitmap(uint8 lchip, ctc_internal_port_assign_para_t* p_port_assign, uint8 op_type)
{

    uint8  igs_set = 0;
    uint8  egs_set = 0;
    uint16 lport = 0;

    if (SYS_USW_INTERNAL_PORT_OP_TYPE_SET == op_type)
    {
        igs_set = ((SYS_INTERNAL_PORT_ISSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport))
                  && ((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ILOOP == p_port_assign->type)
                  || (CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type))) || (SYS_INTERNAL_PORT_ISSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport+1))
                  && (CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type)));

        egs_set = SYS_INTERNAL_PORT_ISSET(lchip, SYS_INTERNAL_PORT_DIR_EGS, p_port_assign->inter_port)
                  && ((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type)||(CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type));

        if (igs_set || egs_set)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Queue] Internal port in use \n");
			return CTC_E_IN_USE;

        }

        if ((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ILOOP == p_port_assign->type))
        {
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_IGS, p_port_assign->inter_port);
        }

        if(CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type)
        {
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_IGS, p_port_assign->inter_port);
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_IGS, p_port_assign->inter_port+1);
        }

        if ((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type))
        {
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_EGS, p_port_assign->inter_port);
        }

    }
    else if (SYS_USW_INTERNAL_PORT_OP_TYPE_FREE == op_type)
    {
        if((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ILOOP == p_port_assign->type))
        {
            SYS_INTERNAL_PORT_UNSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, p_port_assign->inter_port);
        }

        if(CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type)
        {
            SYS_INTERNAL_PORT_UNSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, p_port_assign->inter_port);
            SYS_INTERNAL_PORT_UNSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, p_port_assign->inter_port+1);
        }

        if((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type))
        {
            SYS_INTERNAL_PORT_UNSET(lchip, SYS_INTERNAL_PORT_DIR_EGS, p_port_assign->inter_port);
        }
    }
    else if (SYS_USW_INTERNAL_PORT_OP_TYPE_ALLOC == op_type)
    {
        /* SYS_INTERNAL_PORT_START means sys drv internal port start */
        for (lport = SYS_INTERNAL_PORT_START; lport < (SYS_INTERNAL_PORT_START + SYS_INTERNAL_PORT_NUM); lport++)
        {
            igs_set = ((SYS_INTERNAL_PORT_ISSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport))
                      && ((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ILOOP == p_port_assign->type)
                      || (CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type))) || (SYS_INTERNAL_PORT_ISSET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport+1))
                      && (CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type)));

            egs_set = SYS_INTERNAL_PORT_ISSET(lchip, SYS_INTERNAL_PORT_DIR_EGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport))
                      && ((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type));


            if (igs_set || egs_set)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if ((SYS_INTERNAL_PORT_START + SYS_INTERNAL_PORT_NUM) == lport)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Queue] Internal port no free count \n");
			return CTC_E_NO_RESOURCE;

        }

        p_port_assign->inter_port = SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport);

        if((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ILOOP == p_port_assign->type))
        {
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport));
        }

        if(CTC_INTERNAL_PORT_TYPE_WLAN == p_port_assign->type)
        {
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport));
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_IGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport+1));
        }

        if((CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type) || (CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type))
        {
            SYS_INTERNAL_PORT_SET(lchip, SYS_INTERNAL_PORT_DIR_EGS, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport));
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_loop_port(uint8 lchip, uint16 lport, uint32 nhid, uint8 is_add)
{
    DsDestPortLoopback_m ds_dest_port_loopback;
    uint8 index = 0;
    uint32 cmd  = 0;
    uint32 field_val = 0;
    uint32 max_index = 0;

    if (is_add && 0 == nhid)
    {
        return CTC_E_NONE;
    }

    if (is_add)
    {
        sys_nh_info_dsnh_t sys_nh_info_dsnh;
        sal_memset(&sys_nh_info_dsnh, 0, sizeof(sys_nh_info_dsnh_t));
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &sys_nh_info_dsnh, 0));

        sys_usw_ftm_query_table_entry_num(lchip, DsDestPortLoopback_t, &max_index);

        /*get loopback index*/
        for (index = 0; index < max_index; index++)
        {
            cmd = DRV_IOR(DsDestPortLoopback_t, DRV_ENTRY_FLAG);
            sal_memset(&ds_dest_port_loopback, 0, sizeof(ds_dest_port_loopback));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_dest_port_loopback));

            if ((0 == GetDsDestPortLoopback(V, lbDestMap_f, &ds_dest_port_loopback))
               && (0 == GetDsDestPortLoopback(V, lbNextHopPtr_f, &ds_dest_port_loopback)))
            {
                break;
            }
        }

        if (max_index == index)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

        }

        field_val = BPE_EGS_MUXTYPE_LOOPBACK_ENCODE;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_val));

        field_val = index;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_portLoopbackIndex_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_val));

        sal_memset(&ds_dest_port_loopback, 0, sizeof(DsDestPortLoopback_m));

        SetDsDestPortLoopback(V, lbDestMap_f, &ds_dest_port_loopback, sys_nh_info_dsnh.dest_map);

        SetDsDestPortLoopback(V, lbNextHopPtr_f, &ds_dest_port_loopback, sys_nh_info_dsnh.dsnh_offset);
        SetDsDestPortLoopback(V, lbNextHopExt_f, &ds_dest_port_loopback, sys_nh_info_dsnh.nexthop_ext);

        cmd = DRV_IOW(DsDestPortLoopback_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_dest_port_loopback));
    }
    else
    {
        field_val = BPE_EGS_MUXTYPE_NOMUX;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_val));

        field_val = 0;
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_portLoopbackIndex_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_val));

        index = field_val;
        sal_memset(&ds_dest_port_loopback, 0, sizeof(DsDestPortLoopback_m));
        cmd = DRV_IOW(DsDestPortLoopback_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_dest_port_loopback));

        field_val = 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_portLoopbackIndex_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_val));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_internal_port_e2iloop_init(uint8 lchip)
{
    uint32 lport = 0;
    uint32 cmd = 0;
    QWriteDot1AeCtl_m dot1_ae_ctl;

    sal_memset(&dot1_ae_ctl, 0, sizeof(QWriteDot1AeCtl_m));

    cmd = DRV_IOR(QWriteDot1AeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dot1_ae_ctl));
    lport = SYS_RSV_PORT_WLAN_E2ILOOP;
    SetQWriteDot1AeCtl(V, e2iLoopPort_f, &dot1_ae_ctl, lport);
    cmd = DRV_IOW(QWriteDot1AeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dot1_ae_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_internal_port_ip_tunnel_init(uint8 lchip)
{
    uint8 gchip = 0;
    uint32 lport = 0;
    uint32 cmd = 0;
    uint16 interface_id = 0;
    DsSrcInterface_m src_interface;
    DsDestInterface_m dest_interface;
    DsDestPort_m dest_port;
    DsSrcPort_m src_port;
    DsPhyPortExt_m phy_port_ext;
    DsPhyPort_m phy_port;
    DsDestPortLoopback_m ds_dest_port_loopback;

    sys_usw_get_gchip_id(lchip, &gchip);
    lport = SYS_RSV_PORT_IP_TUNNEL;
    interface_id = MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID);
    sal_memset(&src_interface, 0, sizeof(src_interface));
    SetDsSrcInterface(V, v4UcastEn_f, &src_interface, 1);
    SetDsSrcInterface(V, v4McastEn_f, &src_interface, 1);
    SetDsSrcInterface(V, v6UcastEn_f, &src_interface, 1);
    SetDsSrcInterface(V, v6McastEn_f, &src_interface, 1);
    SetDsSrcInterface(V, routeLookupMode_f, &src_interface, 1);
    SetDsSrcInterface(V, routeAllPackets_f, &src_interface, 1);
    cmd = DRV_IOW(DsSrcInterface_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, interface_id, cmd, &src_interface));

    sal_memset(&dest_interface, 0, sizeof(dest_interface));
    cmd = DRV_IOW(DsDestInterface_t, DRV_ENTRY_FLAG);
    SetDsDestInterface(V, mtuExceptionEn_f, &dest_interface, 1);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, interface_id, cmd, &dest_interface));

    sal_memset(&dest_port, 0, sizeof(dest_port));
    cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dest_port));
    SetDsDestPort(V, dot1QEn_f, &dest_port, 0);
    SetDsDestPort(V, defaultVlanId_f, &dest_port, 0);
    SetDsDestPort(V, muxPortType_f, &dest_port, 3);
    cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dest_port));

    sal_memset(&src_port, 0, sizeof(src_port));
    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &src_port));
    SetDsSrcPort(V, interfaceId_f, &src_port, interface_id);
    SetDsSrcPort(V, routedPort_f, &src_port, 1);
    SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, 1);
    cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &src_port));

    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
    SetDsPhyPortExt(V, defaultVlanId_f, &phy_port_ext, 0);
    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

    sal_memset(&phy_port, 0, sizeof(phy_port));
    SetDsPhyPort(V, fwdHashGenDis_f, &phy_port, 1);
    cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port));

    /*enable eloop for port*/
    sal_memset(&ds_dest_port_loopback, 0, sizeof(ds_dest_port_loopback));
    cmd = DRV_IOR(DsDestPortLoopback_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_dest_port_loopback)); /* LoopbackEncode */
    SetDsDestPortLoopback(V, lbDestMap_f, &ds_dest_port_loopback,SYS_ENCODE_DESTMAP(gchip,SYS_RSV_PORT_ILOOP_ID));
    SetDsDestPortLoopback(V, lbNextHopPtr_f, &ds_dest_port_loopback, ((((lport & 0x80)>>7) << 16) | (lport & 0x7F)));

    cmd = DRV_IOW(DsDestPortLoopback_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_dest_port_loopback));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_internal_port_queue_drop_init(uint8 lchip)
{
#ifdef NEVER
    uint32 cmd = 0;
    uint8 lport = 0;
    DsPhyPortExt_m phy_port_ext;

    lport = SYS_RSV_PORT_IPE_QUE_DROP;
    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
    SetDsPhyPortExt(V, enqDropSpanDestEn_f, &phy_port_ext, 1);
    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

    return CTC_E_NONE;
#endif /* never */
    return CTC_E_NONE;
}

#define __________EXTERNAL_FUNCTION___________

/**
 @brief Set internal port for special usage, for example, I-Loop, E-Loop.
*/
int32
sys_usw_internal_port_set(uint8 lchip, ctc_internal_port_assign_para_t* p_port_assign)
{
    uint16 dst_drv_lport = 0;
    uint16 interal_drv_lport = 0;
    uint16 chan_id = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_INTERNAL_PORT_INIT_CHECK();

    CTC_PTR_VALID_CHECK(p_port_assign);
    CTC_MAX_VALUE_CHECK(p_port_assign->type, CTC_INTERNAL_PORT_TYPE_WLAN);

    if (TRUE != sys_usw_chip_is_local(lchip, p_port_assign->gchip))
    {
        return CTC_E_INVALID_PARAM;
    }

    interal_drv_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_port_assign->inter_port);

    CTC_VALUE_RANGE_CHECK((interal_drv_lport&0xFF), SYS_INTERNAL_PORT_START, (SYS_INTERNAL_PORT_START + SYS_INTERNAL_PORT_NUM - 1));

    CTC_ERROR_RETURN(_sys_usw_port_set_port_bitmap(lchip, p_port_assign, SYS_USW_INTERNAL_PORT_OP_TYPE_SET));

    if (p_port_assign->type == CTC_INTERNAL_PORT_TYPE_FWD)
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_assign->fwd_gport, lchip, dst_drv_lport);
        if ((dst_drv_lport & 0xFF) >= SYS_PHY_PORT_NUM_PER_SLICE)
        {
            return CTC_E_INVALID_PARAM;
        }

        chan_id = SYS_GET_CHANNEL_ID(lchip, p_port_assign->fwd_gport);
    }
    else if (CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type)
    {
        CTC_ERROR_RETURN(_sys_usw_port_set_loop_port(lchip, p_port_assign->inter_port, p_port_assign->nhid, 1));
        chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
    }

    if ((p_port_assign->type != CTC_INTERNAL_PORT_TYPE_ILOOP)
        && (CTC_INTERNAL_PORT_TYPE_WLAN != p_port_assign->type))
    {
        CTC_ERROR_RETURN(_sys_usw_internal_port_set_channel(lchip, p_port_assign->inter_port, p_port_assign->type, chan_id, 1));
    }

    return CTC_E_NONE;
}

/**
 @brief Allocate internal port for special usage, for example, I-Loop, E-Loop.
*/
int32
sys_usw_internal_port_allocate(uint8 lchip, ctc_internal_port_assign_para_t* p_port_assign)
{
    uint8 chan_id = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint16 internal_drv_lport = 0;
    uint16 dst_drv_lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_INTERNAL_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_assign);
    CTC_MAX_VALUE_CHECK(p_port_assign->type, CTC_INTERNAL_PORT_TYPE_WLAN);

    if (TRUE != sys_usw_chip_is_local(lchip, p_port_assign->gchip))
    {
        return CTC_E_INVALID_PARAM;
    }

    /*alloc a free internal port*/
    CTC_ERROR_RETURN(_sys_usw_port_set_port_bitmap(lchip, p_port_assign, SYS_USW_INTERNAL_PORT_OP_TYPE_ALLOC));

    internal_drv_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_port_assign->inter_port);

    if (CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type)
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_assign->fwd_gport, lchip, dst_drv_lport);

        if ((dst_drv_lport & 0xFF) >= SYS_PHY_PORT_NUM_PER_SLICE)
        {
            return CTC_E_INVALID_PARAM;
        }

        /* cfg src port disable stp */
        field_val = 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_stpDisable_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, internal_drv_lport, cmd, &field_val));

        /* cfg dest port disable stp */
        field_val = 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_stpCheckEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, internal_drv_lport, cmd, &field_val));

        chan_id = SYS_GET_CHANNEL_ID(lchip, p_port_assign->fwd_gport);
    }
    else if (CTC_INTERNAL_PORT_TYPE_ELOOP == p_port_assign->type)
    {
        chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
        CTC_ERROR_RETURN(_sys_usw_port_set_loop_port(lchip, p_port_assign->inter_port, p_port_assign->nhid, 1));
    }

    if ((p_port_assign->type != CTC_INTERNAL_PORT_TYPE_ILOOP)
        && (CTC_INTERNAL_PORT_TYPE_WLAN != p_port_assign->type))
    {
        CTC_ERROR_RETURN(_sys_usw_internal_port_set_channel(lchip, p_port_assign->inter_port, p_port_assign->type, chan_id, 1));
    }

    return CTC_E_NONE;
}

/**
 @brief Release internal port.
*/
int32
sys_usw_internal_port_release(uint8 lchip, ctc_internal_port_assign_para_t* p_port_assign)
{

    uint8 chan_id = 0;
    uint16 internal_drv_lport = 0;
    uint16 dst_drv_lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_INTERNAL_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_assign);
    CTC_MAX_VALUE_CHECK(p_port_assign->type, CTC_INTERNAL_PORT_TYPE_WLAN);
    CTC_MAX_VALUE_CHECK(p_port_assign->inter_port, SYS_USW_MAX_PORT_NUM_PER_CHIP-1);

    if (TRUE != sys_usw_chip_is_local(lchip, p_port_assign->gchip))
    {
        return CTC_E_INVALID_PARAM;
    }

    internal_drv_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_port_assign->inter_port);

    CTC_VALUE_RANGE_CHECK((internal_drv_lport&0xFF), SYS_INTERNAL_PORT_START, (SYS_INTERNAL_PORT_START + SYS_INTERNAL_PORT_NUM - 1));
    if (CTC_INTERNAL_PORT_TYPE_FWD == p_port_assign->type)
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_assign->fwd_gport, lchip, dst_drv_lport);

        if ((dst_drv_lport & 0xFF) >= SYS_PHY_PORT_NUM_PER_SLICE)
        {
            return CTC_E_INVALID_PARAM;
        }

        chan_id = SYS_GET_CHANNEL_ID(lchip, p_port_assign->fwd_gport);
    }

    if (p_port_assign->type == CTC_INTERNAL_PORT_TYPE_ELOOP)
    {
        CTC_ERROR_RETURN(_sys_usw_port_set_loop_port(lchip,  p_port_assign->inter_port, p_port_assign->nhid, 0));
    }

    CTC_ERROR_RETURN(_sys_usw_port_set_port_bitmap(lchip, p_port_assign, SYS_USW_INTERNAL_PORT_OP_TYPE_FREE));

    if ((p_port_assign->type != CTC_INTERNAL_PORT_TYPE_ILOOP)
        && (CTC_INTERNAL_PORT_TYPE_WLAN != p_port_assign->type))
    {
        CTC_ERROR_RETURN(_sys_usw_internal_port_set_channel(lchip, p_port_assign->inter_port, p_port_assign->type, chan_id, 0));
    }

    return CTC_E_NONE;
}

int32
sys_usw_internal_port_get_rsv_port(uint8 lchip, sys_internal_port_type_t type, uint16* lport)
{
    SYS_INTERNAL_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(lport);

    *lport = inter_port_master[lchip]->inter_port[type];

    return CTC_E_NONE;
}

int32
sys_usw_internal_port_set_overlay_property(uint8 lchip, uint16 lport)
{
    uint32 value = 0;
    uint32 cmd = 0;
    SYS_INTERNAL_PORT_INIT_CHECK();

    value = 0;
    cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_fwdHashGenDis_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));

    value = 1;
    cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_nvgreEnable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));

    return CTC_E_NONE;
}

int32
sys_usw_internal_port_show(uint8 lchip)
{
    uint16 loop = 0;
    char port_type[SYS_INTERNAL_PORT_TYPE_MAX][16] = {"ILOOP", "ELOOP", "DROP", "IP_TUNNEL", "MIRROR", "WLAN_ENCAP", "WLAN_E2ILOOP"};
    char dir_name[SYS_INTERNAL_PORT_DIR_MAX][16] = {"Ingress", "Egress"};
    char* port_list = NULL;
    uint16 len = 0;
    uint8 dir = 0;
    uint8 is_first = 1;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_INTERNAL_PORT_INIT_CHECK();

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Internal Rsv Port\n");
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------\n");
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s%s\n", "Type", "Port");
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------\n");
    for (loop = SYS_INTERNAL_PORT_TYPE_MIN; loop < SYS_INTERNAL_PORT_TYPE_MAX; loop++)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s%d\n", port_type[loop], inter_port_master[lchip]->inter_port[loop]);
    }

    port_list = (char*)mem_malloc(MEM_PORT_MODULE, sizeof(char)*1024);
    if (NULL == port_list)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(port_list, 0, sizeof(char)*1024);
    for (dir = SYS_INTERNAL_PORT_DIR_IGS; dir < SYS_INTERNAL_PORT_DIR_MAX; dir++)
    {
        len = 0;
        for (loop = SYS_INTERNAL_PORT_START; loop < (SYS_INTERNAL_PORT_START + SYS_INTERNAL_PORT_NUM); loop++)
        {
            if (SYS_INTERNAL_PORT_ISSET(lchip, dir, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(loop)))
            {
                if (((loop == SYS_INTERNAL_PORT_START)
                    && !SYS_INTERNAL_PORT_ISSET(lchip, dir, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(loop+1)))
                    || (loop == (SYS_INTERNAL_PORT_START + SYS_INTERNAL_PORT_NUM - 1))
                    || !SYS_INTERNAL_PORT_ISSET(lchip, dir, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(loop+1)))
                {
                    len += sal_sprintf(port_list + len, "%d ", loop);
                }
                else if (((loop == SYS_INTERNAL_PORT_START)
                        && SYS_INTERNAL_PORT_ISSET(lchip, dir, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(loop+1)))
                        || !SYS_INTERNAL_PORT_ISSET(lchip, dir, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(loop-1)))
                {
                    len += sal_sprintf(port_list + len, "%d-", loop);
                }
            }
        }

        if (len)
        {
            if (is_first)
            {
                is_first = 0;
                SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nInternal Port Used List\n");
                SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");
            }

            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s: %s\n", dir_name[dir], port_list);
        }
    }
    mem_free(port_list);

    return CTC_E_NONE;
}

int32 sys_usw_internal_port_wb_sync(uint8 lchip)
{
    uint16 idx1 = 0;
    uint16 idx2 = 0;
    int32 ret = CTC_E_NONE;
    uint32  max_entry_cnt = 0;
    ctc_wb_data_t wb_data;
    sys_wb_interport_master_t* p_wb_inter_master = NULL;
    sys_wb_interport_used_t* p_wb_inter_used = NULL;

    wb_data.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_PORT_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_port_master_t, CTC_FEATURE_PORT, SYS_WB_APPID_INTERPORT_SUBID_MASTER);

    p_wb_inter_master = (sys_wb_interport_master_t *)wb_data.buffer;
    /*mapping data*/
    p_wb_inter_master->lchip = lchip;
    p_wb_inter_master->version = SYS_WB_VERSION_INTER_PORT;
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_interport_used_t, CTC_FEATURE_PORT, SYS_WB_APPID_INTERPORT_SUBID_USED);
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH/(wb_data.key_len + wb_data.data_len);
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    for (idx1 = 0; idx1 < SYS_INTERNAL_PORT_DIR_MAX; idx1++)
    {
        for (idx2 = 0; idx2 < (SYS_USW_MAX_PORT_NUM_PER_CHIP / BITS_NUM_OF_WORD); idx2++)
        {
            p_wb_inter_used = (sys_wb_interport_used_t *)wb_data.buffer + idx1 * (SYS_USW_MAX_PORT_NUM_PER_CHIP / BITS_NUM_OF_WORD) + idx2;
            p_wb_inter_used->idx1 = idx1;
            p_wb_inter_used->idx2 = idx2;
            p_wb_inter_used->used = inter_port_master[lchip]->is_used[idx1][idx2];
            if (++wb_data.valid_cnt == max_entry_cnt)
            {
                CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
                wb_data.valid_cnt = 0;
            }
        }
    }
    if(wb_data.valid_cnt)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

done:
    if (wb_data.buffer )
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32 sys_usw_internal_port_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 entry_cnt = 0;
    ctc_wb_query_t    wb_query;
    sys_wb_interport_master_t  wb_inter_master = {0};
    sys_wb_interport_used_t  wb_inter_used = {0};

    wb_query.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_PORT_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_interport_master_t, CTC_FEATURE_PORT, SYS_WB_APPID_INTERPORT_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        ret = wb_query.valid_cnt ? CTC_E_INVALID_CONFIG : CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8*)&wb_inter_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_PORT, wb_inter_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_interport_used_t, CTC_FEATURE_PORT, SYS_WB_APPID_INTERPORT_SUBID_USED);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_inter_used, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        inter_port_master[lchip]->is_used[wb_inter_used.idx1][wb_inter_used.idx2] = wb_inter_used.used;
    CTC_WB_QUERY_ENTRY_END((&wb_query));
        
done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

   return ret;
}

/**
 @brief Internal port initialization.
*/

int32
sys_usw_internal_port_init(uint8 lchip)
{
    uint8 port_type = 0;
    uint8 chan_id = 0;
    uint16 lport = 0;

    if (NULL != inter_port_master[lchip])
    {
        return CTC_E_NONE;
    }

    MALLOC_POINTER(sys_inter_port_master_t, inter_port_master[lchip]);
    if (NULL == inter_port_master[lchip])
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(inter_port_master[lchip], 0, sizeof(sys_inter_port_master_t));


    for (port_type = SYS_INTERNAL_PORT_TYPE_MIN; port_type < SYS_INTERNAL_PORT_TYPE_MAX; port_type++)
    {
        switch(port_type)
        {
        case SYS_INTERNAL_PORT_TYPE_ILOOP:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_ILOOP_ID;
            chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
            break;
        case SYS_INTERNAL_PORT_TYPE_ELOOP:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_ELOOP_ID;
            chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
            break;

         case SYS_INTERNAL_PORT_TYPE_DROP:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_DROP_ID;
             chan_id = MCHIP_CAP(SYS_CAP_CHANID_DROP);
            break;

        case SYS_INTERNAL_PORT_TYPE_IP_TUNNEL:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_IP_TUNNEL;
              chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
            break;

         case SYS_INTERNAL_PORT_TYPE_MIRROR:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_MIRROR;
             chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
            break;

         case SYS_INTERNAL_PORT_TYPE_WLAN_ENCAP:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_WLAN_ENCAP;
             chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
            break;

         case SYS_INTERNAL_PORT_TYPE_WLAN_E2ILOOP:
            inter_port_master[lchip]->inter_port[port_type]   = SYS_RSV_PORT_WLAN_E2ILOOP;
             chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
        lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(inter_port_master[lchip]->inter_port[port_type]);
        CTC_ERROR_RETURN(sys_usw_add_port_to_channel(lchip, lport, chan_id));
    }

    CTC_ERROR_RETURN(_sys_usw_internal_port_e2iloop_init(lchip));
    CTC_ERROR_RETURN(_sys_usw_internal_port_ip_tunnel_init(lchip));
    CTC_ERROR_RETURN(_sys_usw_internal_port_queue_drop_init(lchip));
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_internal_port_wb_restore(lchip));
    }
    
    return CTC_E_NONE;
}


/**
 @brief Internal port initialization.
*/

int32
sys_usw_internal_port_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == inter_port_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(inter_port_master[lchip]);
    return CTC_E_NONE;
}

