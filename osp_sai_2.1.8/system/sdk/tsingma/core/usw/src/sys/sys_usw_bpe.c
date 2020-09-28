#if (FEATURE_MODE == 0)
/**
 @file sys_usw_port.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-22

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_nexthop.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_register.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_port.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_bpe.h"
#include "sys_usw_dmps.h"
#include "drv_api.h"



#define MIN_ECID_INIT_VAL 0xFFF
#define MAX_ECID_INIT_VAL 0
#define SYS_BPE_MAX_ECID_NAMESPACE (2*1024-1)
#define SYS_BPE_MAX_ECID (16*1024-1)

#define SYS_BPE_SCL_SUB_GID_DST_ECID 0
#define SYS_BPE_SCL_SUB_GID_SRC_ECID 1


#define SYS_BPE_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(bpe, bpe, BPE_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);


struct sys_bpe_gem_port_s
{
    uint16 lport;                      /**< [TM] local port */
    uint16 gem_vlan;                   /**< [TM] gem port vlaue */
    uint16 logic_port;                 /**< [TM] Logic port */
    uint8  pass_through_en;            /**< [TM] pass through enable/disable */
    uint8  mac_security;            /**< [TM] mac limit : refer to ctc_maclimit_action_t */
};
typedef struct sys_bpe_gem_port_s sys_bpe_gem_port_t;

struct sys_bpe_master_s
{
    uint8  is_port_extender;
    uint8  pe_uplink_mode;
    uint32 pe_uc_dsfwd_base;
    uint32 pe_mc_dsfwd_base;
};
typedef struct sys_bpe_master_s sys_bpe_master_t;

static sys_bpe_master_t* p_usw_bpe_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};


STATIC int32
_sys_usw_bpe_port_en(uint8 lchip, uint32 gport, bool enable)
{

    uint16 lport = 0;
    uint32 field = 0;
    uint32 cmd = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    field = enable ? 0 : 1;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_receiveDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

    field = enable ? 0 : 1;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_transmitDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_bpe_stp_en(uint8 lchip, uint32 gport, bool enable)
{


    uint16 lport = 0;
    uint32 field = 0;
    uint32 cmd = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    field = enable ? 0 : 1;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_stpDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

    field = enable ? 1 : 0;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_stpCheckEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_bpe_cfg_port(uint8 lchip, uint32 gport, ctc_bpe_extender_t* p_extender, uint8 src_mux_type, uint8 dest_mux_type)
{
    uint8  index = 0;
    uint8  gchip = 0;
    uint8  chan_id = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint16 port_vlan_base = 0;
    uint8  symbol = 0;
    ctc_internal_port_assign_para_t   inter_port_para;
    IpeHeaderAdjustPhyPortMap_m  ipe_map;
    EpeHeaderAdjustPhyPortMap_m  epe_l2_map;
    int32 ret = 0;
    uint8 need_rm_ext = 0;
    uint8 cfg_num1 = 0;
    uint8 cfg_num2 = 0;

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /*get channel from port */
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if ( chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    if (p_extender->enable)
    {
        CTC_VALUE_RANGE_CHECK(p_extender->port_base + p_extender->port_num - 1, SYS_INTERNAL_PORT_START, SYS_INTERNAL_PORT_END);

        /* cfg vlan base */
        cmd = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ipe_map));

        cmd = DRV_IOR(EpeHeaderEditPhyPortMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &epe_l2_map));

        SetIpeHeaderAdjustPhyPortMap(V, useLogicPort_f, &ipe_map, 0);
        SetIpeHeaderAdjustPhyPortMap(V, localPhyPort_f, &ipe_map, lport);

        if (p_extender->vlan_base >= p_extender->port_base)
        {
            port_vlan_base = p_extender->vlan_base - p_extender->port_base;
            symbol = 1;
        }
        else
        {
            port_vlan_base = p_extender->port_base - p_extender->vlan_base;
            symbol = 0;
        }

        SetIpeHeaderAdjustPhyPortMap(V, symbol_f, &ipe_map, symbol);
        SetIpeHeaderAdjustPhyPortMap(V, portVlanBase_f, &ipe_map, port_vlan_base);
        cmd = DRV_IOW(IpeHeaderAdjustPhyPortMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ipe_map));

        SetEpeHeaderEditPhyPortMap(V, symbol_f, &epe_l2_map, !symbol);
        SetEpeHeaderEditPhyPortMap(V, portVlanBase_f, &epe_l2_map, port_vlan_base);
        cmd = DRV_IOW(EpeHeaderEditPhyPortMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &epe_l2_map));

        for (index = 0; index < p_extender->port_num; index++)
        {
            /* set internal port */
            inter_port_para.gchip = gchip;
            inter_port_para.fwd_gport = gport;
            inter_port_para.inter_port = p_extender->port_base + index;
            inter_port_para.type = CTC_INTERNAL_PORT_TYPE_FWD;

            cfg_num1++;
            CTC_ERROR_GOTO(sys_usw_internal_port_set(lchip, &inter_port_para), ret, error_proc0);
        }

        for (index = 0; index < p_extender->port_num; index++)
        {
            cfg_num2++;

            CTC_ERROR_GOTO(sys_usw_get_channel_by_port(lchip, gport, &chan_id), ret, error_proc1);
            CTC_ERROR_GOTO(sys_usw_add_port_to_channel(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), chan_id),
                ret, error_proc1);
            CTC_ERROR_GOTO(sys_usw_qos_add_extend_port_to_channel(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), chan_id),
                ret, error_proc1);

            need_rm_ext = 1;
            /* cfg disable port stp */
            CTC_ERROR_GOTO(_sys_usw_bpe_stp_en(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, (p_extender->port_base + index)), FALSE),
                ret, error_proc1);

            field_val =  dest_mux_type;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), cmd, &field_val),
                ret, error_proc1);

            field_val = chan_id & 0x7F;
            cmd = DRV_IOW(DsPortLinkAgg_t, DsPortLinkAgg_u1_g2_sourceChannel_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), cmd, &field_val),
                ret, error_proc1);

            CTC_ERROR_GOTO(_sys_usw_bpe_port_en(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, (p_extender->port_base + index)), TRUE),
                ret, error_proc1);
        }


         if(CTC_BPE_8021QBG_M_VEPA != p_extender->mode)
        {
            /* cfg dest port */
            field_val =  dest_mux_type;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &field_val), ret, error_proc1);
        }

        /* cfg mux type */
        field_val =  src_mux_type;
        cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &field_val), ret, error_proc1);
    }
    else
    {
        /* release port extender */
        field_val =  0;
        cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

        for (index = 0; index < p_extender->port_num; index++)
        {

            cmd = DRV_IOR(DsDestPort_t, DsDestPort_muxPortType_f);
            DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), cmd, &field_val);
            if (!field_val)
            {
                /*internal port not using extend port*/
                SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Internal port %d is not extended \n", SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index));
                continue;
            }

            cmd = DRV_IOR(DsPortLinkAgg_t, DsPortLinkAgg_u1_g2_sourceChannel_f);
            DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), cmd, &field_val);
            if (field_val != chan_id)
            {
                /*internal port is not extended by src-port*/
                SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Extend port %d is not used \n", SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index));
                continue;
            }

            /* set internal port */
            inter_port_para.gchip = gchip;
            inter_port_para.fwd_gport = gport;
            inter_port_para.inter_port = p_extender->port_base + index;
            inter_port_para.type = CTC_INTERNAL_PORT_TYPE_FWD;

            sys_usw_internal_port_release(lchip, &inter_port_para);
            sys_usw_remove_port_from_channel(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), chan_id);
            sys_usw_qos_remove_extend_port_to_channel(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), chan_id);

            /* cfg enable port stp */
            _sys_usw_bpe_stp_en(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, (p_extender->port_base + index)), TRUE);

            field_val =  0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
            DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), cmd, &field_val);

            field_val = chan_id & 0x7F;
            cmd = DRV_IOW(DsPortLinkAgg_t, DsPortLinkAgg_u1_g2_sourceChannel_f);
            DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), cmd, &field_val);

            _sys_usw_bpe_port_en(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, (p_extender->port_base + index)), FALSE);

        }


        if (CTC_BPE_8021QBG_M_VEPA != p_extender->mode)
        {
            field_val =  0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
        }

    }

    return CTC_E_NONE;
error_proc1:
    for (index = 0; index < cfg_num2; index++)
    {
        sys_usw_remove_port_from_channel(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), chan_id);

        if (need_rm_ext)
        {
            sys_usw_qos_remove_extend_port_to_channel(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base + index), chan_id);
        }
    }

error_proc0:
    for (index = 0; index < cfg_num1; index++)
    {
        /* set internal port */
        sal_memset(&inter_port_para, 0, sizeof(ctc_internal_port_assign_para_t));
        inter_port_para.gchip = gchip;
        inter_port_para.fwd_gport = gport;
        inter_port_para.inter_port = p_extender->port_base + index;
        inter_port_para.type = CTC_INTERNAL_PORT_TYPE_FWD;

        sys_usw_internal_port_release(lchip, &inter_port_para);
    }

    return ret;
}


STATIC int32
_sys_usw_bpe_extend_nh_create(uint8 lchip, uint32 gport, uint16 port_base, uint16 nh_num)
{
    int32  ret = 0;
    uint16 idx = 0;

    uint16 lport = 0;

    uint16 extend_gport = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    for (idx = 0; idx < nh_num; idx++)
    {
        extend_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(SYS_MAP_CTC_GPORT_TO_GCHIP(gport),\
                                                      SYS_MAP_SYS_LPORT_TO_DRV_LPORT(port_base) + idx);

        ret = sys_usw_brguc_nh_create(lchip, extend_gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL);
        if ((CTC_E_NONE != ret) && (CTC_E_EXIST != ret))
        {
            return ret;
        }

        CTC_ERROR_RETURN(sys_usw_l2_set_dsmac_for_bpe(lchip, extend_gport, TRUE));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_bpe_extend_nh_delete(uint8 lchip, uint32 gport, uint16 port_base, uint16 nh_num)
{
    uint16 idx = 0;

    uint16 lport = 0;

    uint16 extend_gport = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    for (idx = 0; idx < nh_num; idx++)
    {
        extend_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(SYS_MAP_CTC_GPORT_TO_GCHIP(gport),\
                                                      SYS_MAP_SYS_LPORT_TO_DRV_LPORT(port_base + idx));

        sys_usw_brguc_nh_delete(lchip, extend_gport);
    }

    return CTC_E_NONE;
}

int32
sys_usw_bpe_get_port_extend_mcast_en(uint8 lchip, uint8 *p_enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_enable);
    cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_portExtenderMcastEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    *p_enable = value;
    return CTC_E_NONE;
}

int32
sys_usw_bpe_set_port_extend_mcast_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    LCHIP_CHECK(lchip);
    value = enable ? 1 : 0;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_portExtenderMcastEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = enable ? 1 : 0;
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_portExtenderMcastEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

int32
_sys_usw_bpe_add_pe_forward_dest(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    int32 ret = 0;
    ctc_scl_entry_t   scl_entry;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    uint32 gid = 0;
    uint32 nh_id = 0;

    if ((CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_REDIRECT_TO_CPU)))
    {
        nh_id = CTC_NH_RESERVED_NHID_FOR_TOCPU;
    }
    else if (CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_MCAST))
    {
        CTC_ERROR_GOTO(sys_usw_nh_get_mcast_nh(lchip, p_extender_fwd->mc_grp_id, &nh_id), ret, Error0);
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_l2_get_ucast_nh(lchip, p_extender_fwd->dest_port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &nh_id), ret, Error0);
    }

     /*----------------------------------------------------------------------*/
    /* Dest ECID mappping to nh-id*/
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
    scl_entry.mode = 1;
    scl_entry.key_type = SYS_SCL_KEY_HASH_ECID;
    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_BPE, SYS_BPE_SCL_SUB_GID_DST_ECID, 0, 0, 0);
    CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1), ret, Error0);

    /* NameSpace */
    field_key.type = SYS_SCL_FIELD_KEY_NAMESPACE;
    field_key.data = p_extender_fwd->name_space;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, Error1);

    /* ECID */
    field_key.type = SYS_SCL_FIELD_KEY_ECID;
    field_key.data = p_extender_fwd->ecid;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, Error1);


    /* Valid */
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    field_key.data = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, Error1);



    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    field_action.data0 = nh_id;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, Error1);


    if (CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_COPY_TO_CPU))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;
        field_key.data = 1;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, Error1);
    }

    /* Install to hardware */
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, Error1);


    return CTC_E_NONE;


Error1:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

Error0:
    return ret;

}


int32
_sys_usw_bpe_remove_pe_forward_dest(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    int32 ret = 0;
    uint32 gid       = 0;
    uint32 entry_id  = 0;
    ctc_field_key_t field_key;
    sys_scl_lkup_key_t lkup_key;

     /*----------------------------------------------------------------------*/
    /* Dest ECID mappping to nh-id*/
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));
    lkup_key.key_type = SYS_SCL_KEY_HASH_ECID;
    lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_BPE, SYS_BPE_SCL_SUB_GID_DST_ECID, 0, 0, 0);
    lkup_key.group_id = gid;

    /* NameSpace */
    field_key.type = SYS_SCL_FIELD_KEY_NAMESPACE;
    field_key.data = p_extender_fwd->name_space;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    /* ECID */
    field_key.type = SYS_SCL_FIELD_KEY_ECID;
    field_key.data = p_extender_fwd->ecid;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    field_key.data = 1;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    field_key.data = 1;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    entry_id = lkup_key.entry_id;

    /* Install to hardware */
    CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, entry_id, 1));
    CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, entry_id, 1) );


    return ret;

}


int32
_sys_usw_bpe_add_pe_source_mapping(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    int32 ret = 0;
    ctc_scl_entry_t scl_entry;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    uint32 gid = 0;
    sys_scl_dot1br_t scl_dot1br;

    /*src ecid*/
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    /* KEY */
    scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
    scl_entry.mode = 1;
    scl_entry.key_type = SYS_SCL_KEY_HASH_ING_ECID;
    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_BPE, SYS_BPE_SCL_SUB_GID_SRC_ECID, 0, 0, 0);
    CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1), ret, Error0);

    /* NameSpace */
    field_key.type = SYS_SCL_FIELD_KEY_NAMESPACE;
    field_key.data = p_extender_fwd->name_space;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, Error1);

    /* ECID */
    field_key.type = SYS_SCL_FIELD_KEY_ECID;
    field_key.data = p_extender_fwd->ecid;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, Error1);

    /* Valid */
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    field_key.data = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, Error1);


    /* AD */
    sal_memset(&scl_dot1br, 0, sizeof(sys_scl_dot1br_t));
    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_DOT1BR_PE;
    field_action.ext_data = &scl_dot1br;
    scl_dot1br.src_gport_valid = 1;
    scl_dot1br.src_gport = p_extender_fwd->dest_port;

    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, Error1);

    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, Error1);


    return CTC_E_NONE;

Error1:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

Error0:
    return ret;

}


int32
_sys_usw_bpe_remove_pe_source_mapping(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    int32 ret = 0;
    uint32 gid       = 0;
    uint32 entry_id  = 0;
    ctc_field_key_t field_key;
    sys_scl_lkup_key_t lkup_key;

    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    lkup_key.key_type = SYS_SCL_KEY_HASH_ING_ECID;
    lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_BPE, SYS_BPE_SCL_SUB_GID_SRC_ECID, 0, 0, 0);
    lkup_key.group_id = gid;

    /* NameSpace */
    field_key.type = SYS_SCL_FIELD_KEY_NAMESPACE;
    field_key.data = p_extender_fwd->name_space;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    /* ECID */
    field_key.type = SYS_SCL_FIELD_KEY_ECID;
    field_key.data = p_extender_fwd->ecid;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    /* Valid */
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    field_key.data = 1;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    field_key.data = 1;
    sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key);


    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    entry_id = lkup_key.entry_id;

    /* Install to hardware */
    CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, entry_id, 1));
    CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, entry_id, 1) );


    return ret;

}
STATIC int32
_sys_usw_bpe_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# BPE");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","is_port_extender",p_usw_bpe_master[lchip]->is_port_extender);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","pe_uplink_mode",p_usw_bpe_master[lchip]->pe_uplink_mode);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","pe_uc_dsfwd_base",p_usw_bpe_master[lchip]->pe_uc_dsfwd_base);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","pe_mc_dsfwd_base",p_usw_bpe_master[lchip]->pe_mc_dsfwd_base);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    return CTC_E_NONE;
}
#define __GEMPORT_MUXDEMUX__ ""

int32
sys_usw_bpe_enable_gem_port(uint8 lchip, uint32 gport, uint32 enable)
{
    uint16 chan_id = 0;
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "gport: 0x%x, enable gem: %d\n",
                    gport,enable);

    /* cfg mux type */
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if ( chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAX))
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    field_val =  enable?EPE_IGS_MUXTYPE_GEM_PORT:0;
    cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

    return ret;
}

/* For pon add/remove gem port */
int32
sys_usw_bpe_add_gem_port(uint8 lchip, void* p_gem)
{
    uint8 gchip_id = 0;
    int32 ret = 0;
    uint32 value = 0;
    DsGemPortHashKey_m gem_port_key;
    drv_acc_in_t in;
    drv_acc_out_t out;
    sys_bpe_gem_port_t* p_gem_port = NULL;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_gem_port = (sys_bpe_gem_port_t*) p_gem;
    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_gem_port);

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lport: 0x%x, gem_vlan: %d, logic_port: %d, security: %u  pass_through_en: %d\n",
                    p_gem_port->lport,
                    p_gem_port->gem_vlan,
                    p_gem_port->logic_port,
                    p_gem_port->mac_security,
                    p_gem_port->pass_through_en);


    sal_memset(&gem_port_key, 0, sizeof(DsGemPortHashKey_m));

    sys_usw_get_gchip_id(lchip, &gchip_id);
    value =  CTC_MAP_LPORT_TO_GPORT(gchip_id, p_gem_port->lport);
    /*get channel from port */
    value = SYS_GET_CHANNEL_ID(lchip, value);
    SetDsGemPortHashKey(V, channelId_f, &gem_port_key, value);
    value = p_gem_port->gem_vlan;
    SetDsGemPortHashKey(V, gemPort_f, &gem_port_key, value);
    value = p_gem_port->logic_port;
    SetDsGemPortHashKey(V, logicSrcPort_f, &gem_port_key, value);
    value = 1;
    SetDsGemPortHashKey(V, logicSrcPortValid_f, &gem_port_key, value);
    value = (CTC_MACLIMIT_ACTION_DISCARD == p_gem_port->mac_security) || (CTC_MACLIMIT_ACTION_TOCPU == p_gem_port->mac_security);
    SetDsGemPortHashKey(V, macSecurityDiscard_f, &gem_port_key, value);
    value = CTC_MACLIMIT_ACTION_TOCPU == p_gem_port->mac_security;
    SetDsGemPortHashKey(V, macSecurityExceptionEn_f, &gem_port_key, value);
    value = p_gem_port->pass_through_en;
    SetDsGemPortHashKey(V, onuPassThroughEn_f, &gem_port_key, value);
    value = 0;
    SetDsGemPortHashKey(V, userIdLookupDisable_f, &gem_port_key, value);
    value = 1;
    SetDsGemPortHashKey(V, valid_f, &gem_port_key, value);

    sal_memset(&in, 0, sizeof(in));
    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_KEY;
    in.tbl_id    = DsGemPortHashKey_t;
    in.data      = &gem_port_key;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Gem Port Hash key index:%d\n", out.key_index);

    return ret;
}


int32
sys_usw_bpe_remove_gem_port(uint8 lchip, void* p_gem)
{
    uint8 gchip_id = 0;
    int32 ret = 0;
    uint32 value = 0;
    DsGemPortHashKey_m gem_port_key;
    drv_acc_in_t in;
    drv_acc_out_t out;
    sys_bpe_gem_port_t* p_gem_port = NULL;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_gem_port = (sys_bpe_gem_port_t*) p_gem;
    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_gem_port);

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "port: 0x%x, gem_vlan: %d\n",
                    p_gem_port->lport,
                    p_gem_port->gem_vlan);

    sys_usw_get_gchip_id(lchip, &gchip_id);
    value =  CTC_MAP_LPORT_TO_GPORT(gchip_id, p_gem_port->lport);
    /*get channel from port */
    value = SYS_GET_CHANNEL_ID(lchip, value);

    SetDsGemPortHashKey(V, channelId_f, &gem_port_key, value);
    value = p_gem_port->gem_vlan;
    SetDsGemPortHashKey(V, gemPort_f, &gem_port_key, value);
    value = p_gem_port->logic_port;
    SetDsGemPortHashKey(V, logicSrcPort_f, &gem_port_key, value);
    value = 1;
    SetDsGemPortHashKey(V, logicSrcPortValid_f, &gem_port_key, value);
    value = CTC_MACLIMIT_ACTION_DISCARD == p_gem_port->mac_security;
    SetDsGemPortHashKey(V, macSecurityDiscard_f, &gem_port_key, value);
    value = CTC_MACLIMIT_ACTION_TOCPU == p_gem_port->mac_security;
    SetDsGemPortHashKey(V, macSecurityExceptionEn_f, &gem_port_key, value);
    value = p_gem_port->pass_through_en;
    SetDsGemPortHashKey(V, onuPassThroughEn_f, &gem_port_key, value);
    value = 0;
    SetDsGemPortHashKey(V, userIdLookupDisable_f, &gem_port_key, value);
    value = 1;
    SetDsGemPortHashKey(V, valid_f, &gem_port_key, value);

    sal_memset(&in, 0, sizeof(in));
    in.type = DRV_ACC_TYPE_DEL;
    in.op_type = DRV_ACC_OP_BY_KEY;
    in.tbl_id    = DsGemPortHashKey_t;
    in.data      = &gem_port_key;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    return ret;
}

int32
sys_usw_bpe_port_vdev_en(uint8 lchip, uint32 gport, uint32 enable)
{
    uint16 chan_id = 0;
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "gport: 0x%x, enable vdev: %d\n",
                    gport,enable);

    /* cfg mux type */
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    field_val =  enable?1:0;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

    return ret;
}

#define __CB_MUXDEMUX__ ""

int32
sys_usw_bpe_set_port_extender(uint8 lchip, uint32 gport, ctc_bpe_extender_t* p_extender)
{
    uint16  src_lport = 0;
    uint16  dst_lport = 0;
    uint8  src_mux_type = 0;
    uint8  dest_mux_type = 0;
    uint32 port_type = 0;
    bool enable = 0;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    CTC_PTR_VALID_CHECK(p_extender);
    CTC_ERROR_RETURN(sys_usw_bpe_get_port_extender(lchip, gport, &enable));
    if(enable == TRUE && p_extender->enable)
    {
        return CTC_E_IN_USE;
    }
    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Set gport:%d vlan_base %d num:%d internal_gport:%d!\n",
                    gport, p_extender->vlan_base, p_extender->port_num, p_extender->port_base);


    switch(p_extender->mode)
    {
    case CTC_BPE_MUX_DEMUX_MODE:
        src_mux_type = BPE_IGS_MUXTYPE_MUXDEMUX;
        dest_mux_type = BPE_EGS_MUXTYPE_MUXDEMUX;
        break;
    case CTC_BPE_8021QBG_M_VEPA:
        src_mux_type = BPE_IGS_MUXTYPE_EVB;
        dest_mux_type = BPE_EGS_MUXTYPE_EVB;
        break;
    case CTC_BPE_8021BR_CB_CASCADE:
        src_mux_type = BPE_IGS_MUXTYPE_CB_DOWNLINK;
        dest_mux_type = BPE_EGS_MUXTYPE_CB_CASCADE;
        break;
    default:
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [BPE] Invalid BPE mode \n");
        return CTC_E_INVALID_CONFIG;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, src_lport);

    /*src port must be phycical port*/
    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [BPE] Invalid BPE capable gport \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*check port*/
    dst_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_extender->port_base);

    if ((dst_lport) >= MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM))
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [BPE] Invalid BPE capable gport \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*check port/vlan/ecid range*/
    if ((0 == p_extender->port_num)
        || ((p_extender->vlan_base + p_extender->port_num - 1) > CTC_MAX_VLAN_ID)
    || ((dst_lport & 0xFF) + p_extender->port_num - 1 > 0xFF) )
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [BPE] Invalid BPE vlan base \n");
        return CTC_E_INVALID_PARAM;
    }


    /*set port extender*/
    CTC_ERROR_RETURN(_sys_usw_bpe_cfg_port(lchip, gport, p_extender, src_mux_type, dest_mux_type));

    /*create nexthop for extender port*/
    if (p_extender->enable)
    {
        CTC_ERROR_RETURN(_sys_usw_bpe_extend_nh_create(lchip, gport, p_extender->port_base, p_extender->port_num));
    }
    else
    {
        _sys_usw_bpe_extend_nh_delete(lchip, gport, p_extender->port_base, p_extender->port_num);
    }

    return CTC_E_NONE;
}

int32
sys_usw_bpe_get_port_extender(uint8 lchip, uint32 gport, bool* p_enable)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8  mux_type = 0;
    IpePhyPortMuxCtl_m ipe_phy_port_mux_ctl;

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_enable);

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if ((lport & 0xFF) >= SYS_PHY_PORT_NUM_PER_SLICE)
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [BPE] Invalid BPE capable gport \n");
			return CTC_E_INVALID_CONFIG;

    }

    sal_memset(&ipe_phy_port_mux_ctl, 0, sizeof(IpePhyPortMuxCtl_m));
    cmd = DRV_IOR(IpePhyPortMuxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (lport & 0xFF) + SYS_MAP_DRV_LPORT_TO_SLICE(lport) * 64, cmd, &ipe_phy_port_mux_ctl));

    mux_type = GetIpePhyPortMuxCtl(V, muxType_f, &ipe_phy_port_mux_ctl);

    if (mux_type)
    {
        *p_enable = TRUE;
    }
    else
    {
        *p_enable = FALSE;
    }

    return CTC_E_NONE;
}


#define __PE_DEVICE__ ""

int32
sys_usw_bpe_add_8021br_forward(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    int32 ret = 0;
    uint16 lport = 0;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_extender_fwd);

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag: 0x%x, namespace: %d, ecid: %d, dest_port: 0x%xm  mc_grp_id: 0x%x\n",
                    p_extender_fwd->flag,
                    p_extender_fwd->name_space,
                    p_extender_fwd->ecid,
                    p_extender_fwd->dest_port,
                    p_extender_fwd->mc_grp_id);

    CTC_MAX_VALUE_CHECK(p_extender_fwd->name_space, SYS_BPE_MAX_ECID_NAMESPACE);
    CTC_MAX_VALUE_CHECK(p_extender_fwd->ecid, SYS_BPE_MAX_ECID);

    SYS_GLOBAL_PORT_CHECK(p_extender_fwd->dest_port);

	if (!CTC_IS_LINKAGG_PORT(p_extender_fwd->dest_port) && CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_EXTEND_PORT))
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_extender_fwd->dest_port, lchip, lport);
    }


    CTC_ERROR_RETURN(_sys_usw_bpe_add_pe_forward_dest(lchip, p_extender_fwd));

    /*----------------------------------------------------------------------*/
    /* Src ECID mappping to source port*/
    if (CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_EXTEND_PORT))
    {
        ret = _sys_usw_bpe_add_pe_source_mapping(lchip, p_extender_fwd);
        if (CTC_E_NONE != ret)
        {
            _sys_usw_bpe_remove_pe_forward_dest(lchip, p_extender_fwd);
        }
    }

    return ret;
}


int32
sys_usw_bpe_remove_8021br_forward(uint8 lchip, ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    int32 ret = 0;
    uint16 lport = 0;

    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_extender_fwd);

    SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag: 0x%x, namespace: %d, ecid: %d, dest_port: 0x%xm  mc_grp_id: 0x%x\n",
                    p_extender_fwd->flag,
                    p_extender_fwd->name_space,
                    p_extender_fwd->ecid,
                    p_extender_fwd->dest_port,
                    p_extender_fwd->mc_grp_id);

    CTC_MAX_VALUE_CHECK(p_extender_fwd->name_space, SYS_BPE_MAX_ECID_NAMESPACE);
    CTC_MAX_VALUE_CHECK(p_extender_fwd->ecid, SYS_BPE_MAX_ECID);

    SYS_GLOBAL_PORT_CHECK(p_extender_fwd->dest_port);

    if (!CTC_IS_LINKAGG_PORT(p_extender_fwd->dest_port)&& CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_EXTEND_PORT))
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_extender_fwd->dest_port, lchip, lport);
    }



    CTC_ERROR_RETURN(_sys_usw_bpe_remove_pe_forward_dest(lchip, p_extender_fwd));

    /*----------------------------------------------------------------------*/
    /* Src ECID mappping to source port*/
    if (CTC_FLAG_ISSET(p_extender_fwd->flag, CTC_BPE_FORWARD_EXTEND_PORT))
    {
        CTC_ERROR_RETURN(_sys_usw_bpe_remove_pe_source_mapping(lchip, p_extender_fwd));
    }

    return ret;
}


int32
sys_usw_bpe_init(uint8 lchip, void* bpe_global_cfg)
{

    ctc_bpe_global_cfg_t* p_bpe_global_cfg = NULL;
    uint32 cmd = 0;
    uint32 val = 0;
    uint8 work_status = 0;
    IpeMuxHeaderAdjustCtl_m mux_hdr_ctl;


    if (NULL != p_usw_bpe_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_bpe_global_cfg = (ctc_bpe_global_cfg_t*)bpe_global_cfg;

    /*init soft table*/
    p_usw_bpe_master[lchip] = (sys_bpe_master_t*)mem_malloc(MEM_PORT_MODULE, sizeof(sys_bpe_master_t));
    if (NULL == p_usw_bpe_master[lchip])
    {
        SYS_BPE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_bpe_master[lchip], 0, sizeof(sys_bpe_master_t));
    p_usw_bpe_master[lchip]->is_port_extender = p_bpe_global_cfg->is_port_extender;

    /*is_port_extender 0: mux/demux 1: pe 2:cb*/
    if (1 == p_usw_bpe_master[lchip]->is_port_extender)
    {
        uint32 field_val = 0;
        ctc_scl_group_info_t group;
	    uint32 gid = 0;

        field_val = 1;
        cmd = DRV_IOW(IpeMuxHeaderAdjustCtl_t, IpeMuxHeaderAdjustCtl_peUplinkPortMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &field_val));


        field_val = 1;
        cmd = DRV_IOW(IpeMuxHeaderAdjustCtl_t, IpeMuxHeaderAdjustCtl_peCascadePortMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &field_val));

        field_val = 1;
        cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_internalPortStripEtag_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &field_val));

        field_val = 0xF;
        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_userIdPEModeBitMap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &field_val));

        field_val = 1;
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_ecidRetrieveMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &field_val));


        sal_memset(&group, 0, sizeof(group));
        group.type = CTC_SCL_GROUP_TYPE_NONE;
        group.priority = 0;

        sys_usw_ftm_get_working_status(lchip, &work_status);
        if (!(CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)) || (work_status == CTC_FTM_MEM_CHANGE_RECOVER))
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_BPE, SYS_BPE_SCL_SUB_GID_DST_ECID, 0, 0, 0);
            CTC_ERROR_RETURN(sys_usw_scl_create_group(lchip, gid, &group, 1));

            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_BPE, SYS_BPE_SCL_SUB_GID_SRC_ECID, 0, 0, 0);
            CTC_ERROR_RETURN(sys_usw_scl_create_group(lchip, gid, &group, 1));
        }

    }
    else if (2 == p_usw_bpe_master[lchip]->is_port_extender)
    {
        /*BPE nexthop mcast replicate mode*/
        cmd = DRV_IOR(EpeNextHopReserved_t, EpeNextHopReserved_reserved_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &val));
        CTC_BIT_SET(val, 0);
        cmd = DRV_IOW(EpeNextHopReserved_t, EpeNextHopReserved_reserved_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &val));

        val = 1;
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_nexthopEcidValid_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &val));

        CTC_ERROR_RETURN(sys_usw_nh_set_bpe_en(lchip, TRUE));
    }

    val = 1;
    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_unEtagLookupUserId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 , cmd, &val));

    cmd = DRV_IOR(IpeMuxHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mux_hdr_ctl));
    SetIpeMuxHeaderAdjustCtl(V, muxVlanCheck_f, &mux_hdr_ctl, 1);
    SetIpeMuxHeaderAdjustCtl(V, ecidNoneZeroDiscard_f, &mux_hdr_ctl, 1);
    SetIpeMuxHeaderAdjustCtl(V, portExtenderMetBase_f, &mux_hdr_ctl, 0);
    SetIpeMuxHeaderAdjustCtl(V, etagTpid_f, &mux_hdr_ctl, CTC_BPE_ETAG_TPID);
    SetIpeMuxHeaderAdjustCtl(V, evbTpid_f, &mux_hdr_ctl, CTC_BPE_EVB_TPID);
    SetIpeMuxHeaderAdjustCtl(V, tpid_f, &mux_hdr_ctl, 0x88A8);
    SetIpeMuxHeaderAdjustCtl(V, localEcidChkEn_f, &mux_hdr_ctl, 1);
    SetIpeMuxHeaderAdjustCtl(V, localMinEcid_f, &mux_hdr_ctl, MIN_ECID_INIT_VAL);
    SetIpeMuxHeaderAdjustCtl(V, localMaxEcid_f, &mux_hdr_ctl, MAX_ECID_INIT_VAL);
    cmd = DRV_IOW(IpeMuxHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 ,cmd, &mux_hdr_ctl));

    val = CTC_BPE_ETAG_TPID;
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portExtenderTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 ,cmd, &val));


    val = CTC_BPE_EVB_TPID;
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_evbTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0 ,cmd, &val));

    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_BPE, _sys_usw_bpe_dump_db));


    return CTC_E_NONE;

}

int32
sys_usw_bpe_deinit(uint8 lchip)
{
    if (NULL == p_usw_bpe_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(p_usw_bpe_master[lchip]);
    return 0;
}

#endif

