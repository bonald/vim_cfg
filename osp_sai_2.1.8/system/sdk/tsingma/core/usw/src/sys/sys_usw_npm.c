#if (FEATURE_MODE == 0)
/**
 @file sys_usw_npm.c

 @date 2014-10-28

 @version v3.0


*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_npm.h"
#include "ctc_packet.h"
#include "ctc_oam.h"

#include "sys_usw_npm.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_dmps.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_oam_db.h"
#include "sys_usw_register.h"

#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_NPM_MAX_SESSION_CHECK(max, id) \
    do { \
        if (id >= max){ \
            return CTC_E_INVALID_PARAM; } \
    } while (0)

#define SYS_NPM_1M_BYTES 125000     /*(1000*1000/8)*/
#define SYS_IPG_MAX_VALUE 63

#define SYS_NPM_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == g_npm_master[lchip]) \
        { \
            return CTC_E_NOT_INIT; \
        } \
    } while (0)

#define NPM_LOCK(lchip) \
    do { \
        if (g_npm_master[lchip]->npm_mutex) \
        { \
            sal_mutex_lock(g_npm_master[lchip]->npm_mutex); \
        } \
    } while (0)

#define NPM_UNLOCK(lchip) \
    do { \
        if (g_npm_master[lchip]->npm_mutex) \
        { \
            sal_mutex_unlock(g_npm_master[lchip]->npm_mutex); \
        } \
    } while (0)
#define SYS_USW_MAX_LEN_SESSION_MODE_8           96
#define SYS_USW_MAX_LEN_SESSION_MODE_6           128
#define SYS_USW_MAX_LEN_SESSION_MODE_4           192
#define SYS_NPM_UINT64_MAX      0xFFFFFFFFFFFFFFFFLLU

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/
sys_npm_master_t* g_npm_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

#define __INTERNAL_FUNC__
STATIC int
_sys_usw_byte_reverse_copy(uint8* dest, uint8* src, uint32 len)
{
    uint32 i = 0;
    for (i = 0; i < len; i++)
    {
        *(dest + i) = *(src + (len - 1 - i));
    }
    return 0;
}

STATIC int32
_sys_usw_npm_parameter_check(uint8 lchip, ctc_npm_cfg_t* p_cfg)
{
    uint32 cmd = 0;
    uint8 mode = 0;
    uint8 max_session = 0;
    uint8 index = 0;
    uint8 sub_index = 0;
    AutoGenPktGlbCtl_m glb_ctl;
    ctc_npm_pkt_format_t* p_pkt_format = NULL;
    uint32 value = 0;
    uint32 tmp_value = 0;
    uint32 field_id = 0;
    uint8 is_match = 0;
    uint32 speed_mode = 0;
    uint32 max_speed = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_cfg);

    p_pkt_format = &(p_cfg->pkt_format);

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
       "lchip:%d, session_id:%d, rate:%d, pkt_num:%d, pkt_len:%d \n",
        lchip, p_cfg->session_id, p_cfg->rate, p_cfg->packet_num, p_pkt_format->frame_size);
    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
       "tx_mode:%d, tx_period:%d, burst_cnt:%d, ibg:%d, timeout:%d(s) \n",
        p_cfg->tx_mode, p_cfg->tx_period, p_cfg->burst_cnt, p_cfg->ibg, p_cfg->timeout);
    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
       "size_mode:%d, min_size:%d, flag:0x%x \n",
        p_pkt_format->frame_size_mode, p_pkt_format->min_frame_size, p_cfg->flag);

    CTC_MAX_VALUE_CHECK(p_cfg->tx_mode, (CTC_NPM_TX_MODE_MAX-1));

    if (p_cfg->vlan_id)
    {
        CTC_VLAN_RANGE_CHECK(p_cfg->vlan_id);
    }

    if (!p_cfg->rate)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((!CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_MCAST)) && (!CTC_IS_LINKAGG_PORT(p_cfg->dest_gport)))
    {
        sys_usw_dmps_get_port_property(lchip, p_cfg->dest_gport, SYS_DMPS_PORT_PROP_SPEED_MODE, &speed_mode);
        switch (speed_mode)
        {
            case CTC_PORT_SPEED_10M:
                max_speed = 10000;
                break;
            case CTC_PORT_SPEED_100M:
                max_speed = 100000;
                break;
            case CTC_PORT_SPEED_1G:
                max_speed = 1000000;
                break;
            case CTC_PORT_SPEED_2G5:
                max_speed = 2500000;
                break;
            case CTC_PORT_SPEED_5G:
                max_speed = 5000000;
                break;
            case CTC_PORT_SPEED_10G:
                max_speed = 10000000;
                break;
            case CTC_PORT_SPEED_20G:
                max_speed = 20000000;
                break;
            case CTC_PORT_SPEED_25G:
                max_speed = 25000000;
                break;
            case CTC_PORT_SPEED_40G:
                max_speed = 40000000;
                break;
            case CTC_PORT_SPEED_50G:
                max_speed = 50000000;
                break;
            case CTC_PORT_SPEED_100G:
                max_speed = 100000000;
                break;
            default:
                max_speed = 1000000;
                break;
        }

        /* kbps, can not exceed speed of dest port */
        if ((max_speed) && (p_cfg->rate > max_speed))
        {
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid rate, exceed port max rate \n");
            return CTC_E_INVALID_PARAM;
        }
        /*Tx 99.9% capbility of max_speed*/
        p_cfg->rate =  ((uint64)p_cfg->rate * 999 / 1000);
    }

    if ((!p_pkt_format->ipg) || (p_pkt_format->ipg > SYS_IPG_MAX_VALUE))
    {
        SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid IPG value \n");
        return CTC_E_INVALID_PARAM;
    }

    if ((p_cfg->tx_mode == CTC_NPM_TX_MODE_PACKET_NUM) && (!p_cfg->packet_num))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_cfg->tx_mode == CTC_NPM_TX_MODE_PERIOD) && (!p_cfg->tx_period))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_BURST_EN) && (!p_cfg->burst_cnt))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_MAX_VALUE_CHECK(p_pkt_format->pattern_type, (CTC_NPM_PATTERN_TYPE_MAX-1));
    CTC_MAX_VALUE_CHECK(p_pkt_format->frame_size_mode, 2);
    CTC_PTR_VALID_CHECK(p_pkt_format->pkt_header);

    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_ctl));
    mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);
    if (!p_pkt_format->header_len
    || ((mode == CTC_NPM_SESSION_MODE_8) && p_pkt_format->header_len > (SYS_USW_MAX_LEN_SESSION_MODE_8 - SYS_USW_PKT_HEADER_LEN))
    || ((mode == CTC_NPM_SESSION_MODE_6) && p_pkt_format->header_len > (SYS_USW_MAX_LEN_SESSION_MODE_6 - SYS_USW_PKT_HEADER_LEN))
    || ((mode == CTC_NPM_SESSION_MODE_4) && p_pkt_format->header_len > (SYS_USW_MAX_LEN_SESSION_MODE_4 - SYS_USW_PKT_HEADER_LEN)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_SEQ_EN)) &&
         ((p_pkt_format->seq_num_offset < 14) || (p_pkt_format->seq_num_offset > p_pkt_format->header_len - 4)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_TS_EN)) &&
         ((p_pkt_format->ts_offset < 14) || (p_pkt_format->ts_offset > p_pkt_format->header_len - 8)
         || (1 ==(p_pkt_format->ts_offset % 2)) || (p_pkt_format->ts_offset > MCHIP_CAP(SYS_CAP_NPM_MAX_TS_OFFSET))))
    {
        return CTC_E_INVALID_PARAM;
    }

    /*Increase mode*/
    if (p_pkt_format->frame_size_mode == 1)
    {
        if ((p_pkt_format->min_frame_size < 64) || (p_pkt_format->min_frame_size < (p_pkt_format->header_len+4)))
        {
            return CTC_E_INVALID_PARAM;
        }

        if ((p_pkt_format->frame_size < 64) || (p_pkt_format->frame_size < (p_pkt_format->header_len+4)))
        {
            return CTC_E_INVALID_PARAM;
        }

        if (p_pkt_format->min_frame_size > p_pkt_format->frame_size)
        {
            return CTC_E_INVALID_PARAM;
        }

        CTC_MAX_VALUE_CHECK(p_pkt_format->frame_size, 16127);
        CTC_MAX_VALUE_CHECK(p_pkt_format->min_frame_size, 16127);
    }
    else if (p_pkt_format->frame_size_mode == 2)
    {
        /*Emix mode*/
        if (!p_pkt_format->emix_size_num)
        {
            return CTC_E_INVALID_PARAM;
        }

        CTC_MAX_VALUE_CHECK(p_pkt_format->emix_size_num, CTC_NPM_MAX_EMIX_NUM);

        for (index = 0; index < p_pkt_format->emix_size_num; index++)
        {
            is_match = 0;
            value = p_pkt_format->emix_size[index];
            if ((value < 64) || (value < (p_pkt_format->header_len+4)))
            {
                return CTC_E_INVALID_PARAM;
            }

            CTC_MAX_VALUE_CHECK(value, 16127);

            for (sub_index = 0; sub_index < CTC_NPM_MAX_EMIX_NUM; sub_index++)
            {
                field_id = AutoGenPktGlbCtl_g_0_size_f + sub_index;
                drv_get_field(lchip, AutoGenPktGlbCtl_t, field_id, &glb_ctl, &tmp_value);
                if (value == tmp_value)
                {
                    is_match = 1;
                    break;
                }
            }

            if (!is_match)
            {
                return CTC_E_INVALID_PARAM;
            }

        }
    }
    else
    {
        /*Fix mode*/
        if ((p_pkt_format->frame_size < 64) || (p_pkt_format->frame_size < (p_pkt_format->header_len+4)))
        {
            return CTC_E_INVALID_PARAM;
        }

        CTC_MAX_VALUE_CHECK(p_pkt_format->frame_size, 16127);
    }

    if (p_cfg->dm_stats_mode > 2)
    {
        return CTC_E_INVALID_PARAM;
    }
    max_session = (mode)?((mode == 1)?6:4):8;
    SYS_NPM_MAX_SESSION_CHECK(max_session, p_cfg->session_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_npm_set_bridge_hdr(uint8 lchip, ctc_npm_cfg_t* p_cfg,  MsPacketHeader_m* p_bridge_hdr)
{
    uint8  gchip = 0;
    uint16 lport = 0;
    uint32 p_nhid;
    uint32 dest_map = 0;
    uint32 nexthop_ptr = 0;
    int32 ret = CTC_E_NONE;
    uint8 bridge_hdr[SYS_USW_PKT_HEADER_LEN] = {0};
    uint8 i=0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_MCAST))
    {
        if(!CTC_IS_LINKAGG_PORT(p_cfg->dest_gport))
        {
            SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_cfg->dest_gport, lchip, lport);
            gchip = SYS_DRV_GPORT_TO_GCHIP(p_cfg->dest_gport);
        }
    }
    else
    {
        if (CTC_E_NOT_EXIST == sys_usw_nh_get_mcast_nh(lchip, p_cfg->dest_gport, &p_nhid))
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_ILOOP))
    {
        if (CTC_IS_LINKAGG_PORT(p_cfg->dest_gport)
            ||(CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_MCAST)))
        {
            return CTC_E_INVALID_PARAM;
        }
        SetMsPacketHeader(V, destMap_f, p_bridge_hdr, SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_ILOOP_ID));
        SetMsPacketHeader(V, nextHopPtr_f, p_bridge_hdr, (((lport >> 7)&0x1) << 16) | (lport&0xFF));
    }
    else
    {
        /* get bypass nexthop */
        ret = sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, &nexthop_ptr);
        if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_MCAST))
        {
            SetMsPacketHeader(V, destMap_f, p_bridge_hdr, SYS_ENCODE_MCAST_IPE_DESTMAP(p_cfg->dest_gport));
            SetMsPacketHeader(V, nextHopPtr_f, p_bridge_hdr, 0);
        }
        else
        {
            dest_map = (CTC_IS_LINKAGG_PORT(p_cfg->dest_gport))?
                SYS_ENCODE_DESTMAP(CTC_LINKAGG_CHIPID, p_cfg->dest_gport&0xFF): SYS_ENCODE_DESTMAP(gchip, lport);
            SetMsPacketHeader(V, destMap_f, p_bridge_hdr, dest_map);
            SetMsPacketHeader(V, nextHopPtr_f, p_bridge_hdr, nexthop_ptr);
            SetMsPacketHeader(V, nextHopExt_f , p_bridge_hdr, 1);
        }
    }

    SetMsPacketHeader(V, srcVlanPtr_f, p_bridge_hdr, p_cfg->vlan_id);
    SetMsPacketHeader(V, outerVlanIsCVlan_f, p_bridge_hdr, (p_cfg->vlan_domain)?1:0);
    SetMsPacketHeader(V, sourcePort_f , p_bridge_hdr, SYS_RSV_PORT_OAM_CPU_ID);
    SetMsPacketHeader(V, color_f , p_bridge_hdr, 3);
    SetMsPacketHeader(V, fromCpuOrOam_f, p_bridge_hdr, 1);
    SetMsPacketHeader(V, operationType_f , p_bridge_hdr, 7);

    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_TS_EN))
    {
        SetMsPacketHeader(V, u1_oam_dmEn_f, p_bridge_hdr, TRUE);
        SetMsPacketHeader(V, u1_dmtx_dmOffset_f, p_bridge_hdr, p_cfg->pkt_format.ts_offset);
    }

    for (i = 0; i < (SYS_USW_PKT_HEADER_LEN / 4); i++)
    {
        bridge_hdr[i*4 + 0] = ((uint8*)p_bridge_hdr)[i*4 + 3];
        bridge_hdr[i*4 + 1] = ((uint8*)p_bridge_hdr)[i*4 + 2];
        bridge_hdr[i*4 + 2] = ((uint8*)p_bridge_hdr)[i*4 + 1];
        bridge_hdr[i*4 + 3] = ((uint8*)p_bridge_hdr)[i*4 + 0];
    }
    sal_memcpy((uint8*)p_bridge_hdr, bridge_hdr,  SYS_USW_PKT_HEADER_LEN);
    for (i = 0; i < (SYS_USW_PKT_HEADER_LEN / 4); i++)
    {
        ((uint32 *)bridge_hdr)[i] = DRV_SWAP32(((uint32 *)p_bridge_hdr)[i]);
    }
    _sys_usw_byte_reverse_copy((uint8*)p_bridge_hdr, bridge_hdr, SYS_USW_PKT_HEADER_LEN);

    return ret;
}

STATIC int32
_sys_usw_npm_set_user_data(uint8 lchip, ctc_npm_cfg_t* p_cfg, void* p_pkt_header)
{
    uint32 cmd = 0;
    uint32 up_ptr = 0;
    AutoGenPktPktHdr_m ds_pkt_hdr;
    AutoGenPktGlbCtl_m glb_ctl;
    uint32* p_hdr = NULL;
    uint8 index = 0;
    uint8 session_id = 0;
    uint8 session_mode = 0;
    uint16 header_len = 0;
    uint8 remnant = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    session_id = p_cfg->session_id;

    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_ctl));
    session_mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);

    if (session_mode == 0)
    {
        up_ptr = ((session_id << 4) - (session_id << 2));
    }
    else if (session_mode == 1)
    {
        up_ptr = (session_id << 4);
    }
    else
    {
        up_ptr = ((session_id << 5) - (session_id << 3));
    }

    sal_memset(&ds_pkt_hdr, 0, sizeof(AutoGenPktPktHdr_m));

    /* config one section */
    p_hdr = (uint32*)p_pkt_header;
    header_len = p_cfg->pkt_format.header_len + SYS_USW_PKT_HEADER_LEN;

    for (index = 0; index < header_len/8; index++)
    {
        SetAutoGenPktPktHdr(V, pktHdrUserdata0_f, &ds_pkt_hdr, DRV_SWAP32(p_hdr[index*2]));
        SetAutoGenPktPktHdr(V, pktHdrUserdata1_f, &ds_pkt_hdr, DRV_SWAP32(p_hdr[index*2+1]));
        cmd = DRV_IOW(AutoGenPktPktHdr_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (up_ptr+index), cmd, &ds_pkt_hdr));
    }

    remnant = header_len % 8;
    index = header_len/8;

    if (remnant)
    {
        sal_memset(&ds_pkt_hdr, 0, sizeof(AutoGenPktPktHdr_m));

        SetAutoGenPktPktHdr(V, pktHdrUserdata0_f, &ds_pkt_hdr, DRV_SWAP32(p_hdr[index*2]));
        SetAutoGenPktPktHdr(V, pktHdrUserdata1_f, &ds_pkt_hdr, DRV_SWAP32(p_hdr[index*2+1]));
        cmd = DRV_IOW(AutoGenPktPktHdr_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (up_ptr+index), cmd, &ds_pkt_hdr));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_npm_cal_rate(uint8 lchip, ctc_npm_cfg_t* p_cfg, uint32 autogen_clk)
{
    uint8 ipg = 0;
    uint64 temp = 0;
    uint32 token_freq = 0;
    uint32 token_remainder = 0;
    uint32 burst_interval = 0;
    uint32 burst_idle = 0;
    uint64 burst_bytes = 0;
    AutoGenPktCtl_m pkt_ctl;
    AutoGenPktPktCfg_m pkt_cfg;
    uint32 cmd = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&pkt_ctl, 0, sizeof(AutoGenPktCtl_m));
    sal_memset(&pkt_cfg, 0, sizeof(AutoGenPktPktCfg_m));

    /* get ipg */
    ipg = p_cfg->pkt_format.ipg;

    /* cal token update frequency */
    temp = ((uint64)p_cfg->rate*SYS_NPM_1M_BYTES)/1000;
    token_freq = temp/(autogen_clk*1000);
    token_remainder = ((temp-token_freq*(autogen_clk*1000))*65536)/(autogen_clk*1000);

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "token:%u, token_remainder:0x%x \n", (uint32)token_freq, (uint32)token_remainder);

    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_BURST_EN))
    {
        /*burst mode only support fix frame size*/
        if (p_cfg->pkt_format.frame_size_mode != 0)
        {
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;

        }

        burst_bytes = ((uint64)p_cfg->burst_cnt)*(p_cfg->pkt_format.frame_size+ipg);
        burst_interval = (burst_bytes*autogen_clk*8)/(p_cfg->rate);
        if (burst_bytes%(p_cfg->rate*1000))
        {
            burst_interval += 1;
        }
    }
    else
    {
        burst_interval = autogen_clk*1000;
    }

    if (p_cfg->ibg == 0)
    {
        burst_idle = 0;
    }
    else
    {
        burst_idle = (p_cfg->ibg*autogen_clk)/1000;
    }

    /*write hw*/
    cmd = DRV_IOR(AutoGenPktCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &pkt_ctl));
    SetAutoGenPktCtl(V,tokenBytes_f, &pkt_ctl, token_freq);
    SetAutoGenPktCtl(V,tokenBytesFrac_f, &pkt_ctl, token_remainder);
    cmd = DRV_IOW(AutoGenPktCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &pkt_ctl));

    cmd = DRV_IOR(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &pkt_cfg));
    SetAutoGenPktPktCfg(V, burstTxInterval_f, &pkt_cfg, burst_interval);
    SetAutoGenPktPktCfg(V, burstIdleInterval_f, &pkt_cfg, burst_idle);
    cmd = DRV_IOW(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &pkt_cfg));
   return CTC_E_NONE;
}

STATIC int32
_sys_usw_npm_set_auto_gen(uint8 lchip, ctc_npm_cfg_t* p_cfg)
{
    AutoGenPktPktCfg_m autogen_pkt_cfg;
    AutoGenPktEn_m autogen_en;
    RefDivOamTokenUpd_m oam_div;
    AutoGenPktGlbCtl_m glb_ctl;
    uint32 cmd = 0;
    uint8 tx_mode = 0;
    uint8 is_use_period = 0;
    uint32 autogen_clock = 0;
    uint8 index = 0;
    uint8 sub_index = 0;
    uint32 field_id = 0;
    ctc_npm_pkt_format_t* p_pkt_format = NULL;
    uint32 burst_interval = 0;
    uint32 burst_idle = 0;
    uint32 tx_period = 0;
    uint32 tmp_value = 0;
    uint32 value = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_pkt_format = &p_cfg->pkt_format;
    sal_memset(&autogen_pkt_cfg, 0, sizeof(AutoGenPktPktCfg_m));
    sal_memset(&autogen_en, 0, sizeof(AutoGenPktEn_m));

    /* get AutoGen Pulse Clock */
    cmd = DRV_IOR(RefDivOamTokenUpd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_div));
    autogen_clock = GetRefDivOamTokenUpd(V, cfgRefDivOamTokenUpdPulse_f, &oam_div);
    if (!autogen_clock)
    {
        return CTC_E_INVALID_PARAM;
    }

     /*-autogen_clock = 32*1000/(autogen_clock>>8);*/
    autogen_clock = 1000;

    /* cal rate */
    CTC_ERROR_RETURN(_sys_usw_npm_cal_rate(lchip, p_cfg, autogen_clock));

    cmd = DRV_IOR(AutoGenPktEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_en));

    cmd = DRV_IOR(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_pkt_cfg));

    switch(p_cfg->tx_mode)
    {
        case CTC_NPM_TX_MODE_CONTINUOUS:
            tx_mode = 1;
            is_use_period = 0;
            break;
        case CTC_NPM_TX_MODE_PACKET_NUM:
            tx_mode = 0;
            is_use_period = 0;
            break;
        case CTC_NPM_TX_MODE_PERIOD:
            tx_mode = 1;
            is_use_period = 1;

            /*cal period */
            burst_interval = GetAutoGenPktPktCfg(V, burstTxInterval_f, &autogen_pkt_cfg);
            burst_idle = GetAutoGenPktPktCfg(V, burstIdleInterval_f, &autogen_pkt_cfg);
            if ((burst_interval+burst_idle) == 0)
            {
                return CTC_E_INVALID_PARAM;
            }
            tx_period = ((uint64)p_cfg->tx_period*autogen_clock*1000)/(burst_idle+burst_interval);

            break;
        default:
            break;
    }

    /*cfg tx mode*/
    SetAutoGenPktPktCfg(V, txMode_f, &autogen_pkt_cfg,  tx_mode);
    SetAutoGenPktPktCfg(V, isUsePeriod_f, &autogen_pkt_cfg,  is_use_period);
    SetAutoGenPktEn(V, periodCnt_f, &autogen_en,  tx_period);
    SetAutoGenPktEn(V, txPktCnt_f, &autogen_en,  p_cfg->packet_num);

    /*cfg seq number insert*/
    SetAutoGenPktPktCfg(V, seqNumOffset_f, &autogen_pkt_cfg, p_pkt_format->seq_num_offset + SYS_USW_PKT_HEADER_LEN);
    SetAutoGenPktPktCfg(V, txSeqNumEn_f , &autogen_pkt_cfg, (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_SEQ_EN)?1:0));
    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_SEQ_EN))
    {
        value = p_cfg->pkt_format.seq_num_offset;
        cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_seqNoOffset_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &value));
    }
    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_TS_EN))
    {
        value =  p_cfg->pkt_format.ts_offset;
        cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_tsOffset_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &value));
    }
    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_NTP_TS))
    {
        g_npm_master[lchip]->is_ntp_ts[p_cfg->session_id] = 1;
    }
    else
    {
        g_npm_master[lchip]->is_ntp_ts[p_cfg->session_id] = 0;
    }

    /*cfg packet size*/
    if (p_pkt_format->frame_size_mode == 0)
    {
        /*fix size*/
        SetAutoGenPktPktCfg(V, txPktSize_f, &autogen_pkt_cfg,  p_pkt_format->frame_size);
        SetAutoGenPktPktCfg(V, txPktSizeType_f, &autogen_pkt_cfg,  0);
    }
    else if (p_pkt_format->frame_size_mode == 1)
    {
        /*increase*/
        SetAutoGenPktPktCfg(V, txPktSize_f, &autogen_pkt_cfg,  p_pkt_format->frame_size);
        SetAutoGenPktPktCfg(V, txPktMinSize_f, &autogen_pkt_cfg,  p_pkt_format->min_frame_size);
        SetAutoGenPktPktCfg(V, txPktSizeType_f, &autogen_pkt_cfg,  2);
    }
    else if (p_pkt_format->frame_size_mode == 2)
    {
        /*emix*/
        SetAutoGenPktPktCfg(V, txPktSizeType_f, &autogen_pkt_cfg,  3);
        SetAutoGenPktPktCfg(V, eMixArraySize_f, &autogen_pkt_cfg,  (p_pkt_format->emix_size_num - 1));
        for (index = 0; index < p_pkt_format->emix_size_num; index++)
        {
            cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_ctl));

            for (sub_index = 0; sub_index < CTC_NPM_MAX_EMIX_NUM; sub_index++)
            {
                field_id = AutoGenPktGlbCtl_g_0_size_f + sub_index;
                drv_get_field(lchip, AutoGenPktGlbCtl_t, field_id, &glb_ctl, &tmp_value);
                if (p_pkt_format->emix_size[index] == tmp_value)
                {
                    value = sub_index;
                    break;
                }
            }

            field_id = AutoGenPktPktCfg_eMixArray_0_sizeIdx_f + index;
            drv_set_field(lchip, AutoGenPktPktCfg_t, field_id, &autogen_pkt_cfg, &value);

        }
    }

    /*cfg pattern*/
    SetAutoGenPktPktCfg(V, patternType_f, &autogen_pkt_cfg,  p_pkt_format->pattern_type);
    SetAutoGenPktPktCfg(V, repeatPattern_f, &autogen_pkt_cfg,  p_pkt_format->repeat_pattern);
    SetAutoGenPktPktCfg(V, ipg_f, &autogen_pkt_cfg,  p_pkt_format->ipg);

    /*cfg timeout*/
    if (p_cfg->timeout)
    {
        SetAutoGenPktPktCfg(V, isUseTimeout_f, &autogen_pkt_cfg,  1);
        burst_interval = GetAutoGenPktPktCfg(V, burstTxInterval_f, &autogen_pkt_cfg);
        burst_idle = GetAutoGenPktPktCfg(V, burstIdleInterval_f, &autogen_pkt_cfg);
        if ((burst_interval+burst_idle) == 0)
        {
            return CTC_E_INVALID_PARAM;
        }
        tx_period = ((uint64)p_cfg->timeout*autogen_clock*1000)/(burst_idle+burst_interval);
        SetAutoGenPktPktCfg(V, timeOutCfg_f, &autogen_pkt_cfg,  tx_period);
    }
    else
    {
        SetAutoGenPktPktCfg(V, isUseTimeout_f, &autogen_pkt_cfg,  0);
    }

    /*cfg burst*/
    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_BURST_EN))
    {
        if (p_cfg->ibg)
        {
            if (p_cfg->ibg*autogen_clock > 0xFFFFFFFF)
            {
                return CTC_E_INVALID_PARAM;
            }
            value = (p_cfg->ibg*autogen_clock)/1000;
            SetAutoGenPktPktCfg(V, burstIdleInterval_f, &autogen_pkt_cfg, value);
        }
    }

    SetAutoGenPktPktCfg(V, isHaveEndTlv_f, &autogen_pkt_cfg,  1);
    SetAutoGenPktPktCfg(V, pktHdrLen_f, &autogen_pkt_cfg,  p_pkt_format->header_len + SYS_USW_PKT_HEADER_LEN);

    /*cfg AutoGenPktPktCfg */
    cmd = DRV_IOW(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_pkt_cfg));

    /*cfg AutoGenPktEn */
    cmd = DRV_IOW(AutoGenPktEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_en));

    return CTC_E_NONE;

}


STATIC int32
_sys_usw_npm_show_session_status(uint8 lchip, ctc_npm_cfg_t* p_cfg)
{
    AutoGenPktPktCfg_m autogen_pkt_cfg;
    AutoGenPktEn_m autogen_en;
    AutoGenPktGlbCtl_m glb_ctl;
    uint32 cmd = 0;
    uint8 tx_mode = 0;
    uint8 is_use_period = 0;
    uint32 autogen_clock = 1000;
    uint32 burst_interval = 0;
    uint32 burst_idle = 0;
    uint32 tx_period = 0;
    uint32 value = 0;
    uint8  index = 0;
    uint32 tmp_value = 0;
    uint32 field_id = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&autogen_pkt_cfg, 0, sizeof(AutoGenPktPktCfg_m));
    sal_memset(&autogen_en, 0, sizeof(AutoGenPktEn_m));


    cmd = DRV_IOR(AutoGenPktEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_en));
    if (!GetAutoGenPktEn(V, autoGenEn_f, &autogen_en))
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_pkt_cfg));

    tx_mode = GetAutoGenPktPktCfg(V, txMode_f, &autogen_pkt_cfg);
    is_use_period = GetAutoGenPktPktCfg(V, isUsePeriod_f, &autogen_pkt_cfg);
    if (1 == tx_mode && 0 == is_use_period)
    {
        p_cfg->tx_mode = CTC_NPM_TX_MODE_CONTINUOUS;
    }
    else if(0 == tx_mode && 0 == is_use_period)
    {
        p_cfg->tx_mode = CTC_NPM_TX_MODE_PACKET_NUM;
        p_cfg->packet_num = GetAutoGenPktEn(V, txPktCnt_f, &autogen_en);
    }
    else if(1 == tx_mode && 1 == is_use_period)
    {
        p_cfg->tx_mode = CTC_NPM_TX_MODE_PERIOD;

        burst_interval = GetAutoGenPktPktCfg(V, burstTxInterval_f, &autogen_pkt_cfg);
        burst_idle = GetAutoGenPktPktCfg(V, burstIdleInterval_f, &autogen_pkt_cfg);
        if ((burst_interval + burst_idle) == 0)
        {
            return CTC_E_INVALID_PARAM;
        }
        tx_period = GetAutoGenPktEn(V, periodCnt_f, &autogen_en);
        p_cfg->tx_period = ((uint64)tx_period *(burst_idle + burst_interval)) / autogen_clock / 1000 ;
    }

    p_cfg->pkt_format.seq_num_offset = GetAutoGenPktPktCfg(V, seqNumOffset_f, &autogen_pkt_cfg) - SYS_USW_PKT_HEADER_LEN;

    /*fix size*/
    if (0 == GetAutoGenPktPktCfg(V, txPktSizeType_f, &autogen_pkt_cfg))
    {
        p_cfg->pkt_format.frame_size_mode = 0;
        p_cfg->pkt_format.frame_size = GetAutoGenPktPktCfg(V, txPktSize_f, &autogen_pkt_cfg);
    }

    else if (2 == GetAutoGenPktPktCfg(V, txPktSizeType_f, &autogen_pkt_cfg))
    {
        /*increase*/
        p_cfg->pkt_format.frame_size_mode = 1;
        p_cfg->pkt_format.frame_size = GetAutoGenPktPktCfg(V, txPktSize_f, &autogen_pkt_cfg);
        p_cfg->pkt_format.min_frame_size = GetAutoGenPktPktCfg(V, txPktMinSize_f, &autogen_pkt_cfg);

    }
    else if (3 == GetAutoGenPktPktCfg(V, txPktSizeType_f, &autogen_pkt_cfg))
    {
        /*emix*/
        p_cfg->pkt_format.emix_size_num = GetAutoGenPktPktCfg(V, eMixArraySize_f, &autogen_pkt_cfg) + 1;
        p_cfg->pkt_format.frame_size_mode = 2;

        cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_ctl));
        for (index = 0; index < CTC_NPM_MAX_EMIX_NUM; index++)
        {
            field_id = AutoGenPktGlbCtl_g_0_size_f + index;
            drv_get_field(lchip, AutoGenPktGlbCtl_t, field_id, &glb_ctl, &tmp_value);
            p_cfg->pkt_format.emix_size[index] = tmp_value;
        }

    }

    /*cfg pattern*/
    p_cfg->pkt_format.pattern_type = GetAutoGenPktPktCfg(V, patternType_f, &autogen_pkt_cfg);
    p_cfg->pkt_format.repeat_pattern = GetAutoGenPktPktCfg(V, repeatPattern_f, &autogen_pkt_cfg);
    p_cfg->pkt_format.ipg= GetAutoGenPktPktCfg(V, ipg_f, &autogen_pkt_cfg);

    /*cfg timeout*/
    p_cfg->timeout = GetAutoGenPktPktCfg(V, isUseTimeout_f, &autogen_pkt_cfg)? 1 : 0;

    if (GetAutoGenPktPktCfg(V, burstIdleInterval_f, &autogen_pkt_cfg))
    {
        CTC_SET_FLAG(p_cfg->flag, CTC_NPM_CFG_FLAG_BURST_EN);
    }

    value = GetAutoGenPktPktCfg(V, burstIdleInterval_f, &autogen_pkt_cfg );
    p_cfg->ibg = value *1000 /autogen_clock;

    return CTC_E_NONE;

}


STATIC int32
_sys_usw_npm_clear_stats(uint8 lchip, uint8 session_id)
{
    uint32 cmd = 0;
    uint8 mode = 0;
    uint8 max_session = 0;
    uint32 value = 0;
    AutoGenPktTxPktStats_m tx_stats;
    AutoGenPktRxPktStats_m rx_stats;
    AutoGenPktTxPktAck_m tx_ack;
    AutoGenPktGlbCtl_m glb_ctl;

    sal_memset(&tx_stats, 0, sizeof(tx_stats));
    sal_memset(&rx_stats, 0, sizeof(rx_stats));
    sal_memset(&tx_ack, 0, sizeof(tx_ack));

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_ctl));
    mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);
    max_session = (mode)?((mode == 1)?6:4):8;
    SYS_NPM_MAX_SESSION_CHECK(max_session, session_id);

    /* clear AutoGen tx stats */
    cmd = DRV_IOW(AutoGenPktTxPktStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, &tx_stats));

    /* clear AutoGen rx stats */
    cmd = DRV_IOW(AutoGenPktRxPktStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, &rx_stats));

    /* clear AutoGen txAck stats */
    cmd = DRV_IOR(AutoGenPktTxPktAck_t, AutoGenPktTxPktAck_measureType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, &value));
    cmd = DRV_IOW(AutoGenPktTxPktAck_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, &tx_ack));
    cmd = DRV_IOW(AutoGenPktTxPktAck_t, AutoGenPktTxPktAck_measureType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, &value));
    /* clear stats in DB*/
    g_npm_master[lchip]->rx_bytes_all[session_id] = 0;
    g_npm_master[lchip]->tx_bytes_all[session_id] = 0;
    g_npm_master[lchip]->rx_pkts_all[session_id] = 0;
    g_npm_master[lchip]->tx_pkts_all[session_id] = 0;
    g_npm_master[lchip]->total_delay_all[session_id] = 0;
    g_npm_master[lchip]->total_far_delay_all[session_id] = 0;
    g_npm_master[lchip]->max_jitter[session_id] = 0;
    g_npm_master[lchip]->total_jitter_all[session_id] = 0;
    g_npm_master[lchip]->max_delay[session_id] = 0;
    g_npm_master[lchip]->min_delay[session_id] = SYS_NPM_UINT64_MAX;
    g_npm_master[lchip]->min_jitter[session_id] = CTC_MAX_UINT32_VALUE;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_npm_get_rx_tx_stats(uint8 lchip, uint8 session_id, uint8 dir, uint32* stats)
{
    uint32 cmd = 0;
    uint64 tmp_1st = 0;
    uint64 tmp_2nd = 0;
    uint32 field_value = 0;
    uint32 tmp[2] = {0};
    fld_id_t field_id;
    uint32 table_id = 0;
    uint32 stats_tmp[16] = {0};

    /*only for D2 NPM stats clearOnRead*/
    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);

    table_id = dir ? AutoGenPktTxPktStats_t : AutoGenPktRxPktStats_t;
    field_id = dir ? AutoGenPktTxPktStats_txBytes_f : AutoGenPktRxPktStats_totalDTs_f;

    do
    {
        /*first read stats*/
        cmd = DRV_IOR(table_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, stats));
        sal_memset(tmp, 0, sizeof(tmp));
        DRV_GET_FIELD(lchip, table_id, field_id, stats, tmp);
        tmp_1st = ((uint64)tmp[1] << 32) + tmp[0];

        field_value = 0;
        cmd = DRV_IOW(AutoGenPktGlbCtl_t, AutoGenPktGlbCtl_clearOnRead_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

        /*second read stats*/
        cmd = DRV_IOR(table_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, session_id, cmd, &stats_tmp));

        field_value = 1;
        cmd = DRV_IOW(AutoGenPktGlbCtl_t, AutoGenPktGlbCtl_clearOnRead_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

        sal_memset(tmp, 0, sizeof(tmp));
        DRV_GET_FIELD(lchip, table_id, field_id, &stats_tmp, tmp);
        tmp_2nd = ((uint64)tmp[1] << 32) + tmp[0];
    }
    while (tmp_2nd > tmp_1st);

    return CTC_E_NONE;
}

#define __SYS_INTERFACE__
int32
sys_usw_npm_set_config(uint8 lchip, ctc_npm_cfg_t* p_cfg)
{
    MsPacketHeader_m bridge_header;
    void* p_pkt_header = NULL;
    ctc_npm_pkt_format_t* p_pkt_format = NULL;
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 autogen_en = 0;
    uint32 value = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);
    SYS_NPM_INIT_CHECK(lchip);
    sal_memset(&bridge_header, 0, sizeof(MsPacketHeader_m));

    /*Config for Rx roler, only care ts_offset and seq_num_offset*/
    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_RX_ROLE_EN))
    {
        SYS_NPM_MAX_SESSION_CHECK(MCHIP_CAP(SYS_CAP_NPM_SESSION_NUM), p_cfg->session_id);
        value = p_cfg->rx_role_en;
        cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_autoGenEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &value));
        if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_SEQ_EN))
        {
            value = p_cfg->pkt_format.seq_num_offset;
            cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_seqNoOffset_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &value));
        }
        if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_TS_EN))
        {
            value =  p_cfg->pkt_format.ts_offset;
            cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_tsOffset_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &value));
        }
        return CTC_E_NONE;
    }

    NPM_LOCK(lchip);
   /*1. parameter check */
    CTC_ERROR_GOTO(_sys_usw_npm_parameter_check(lchip, p_cfg), ret, error);

    cmd = DRV_IOR(AutoGenPktEn_t, AutoGenPktEn_autoGenEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &autogen_en), ret, error);
    if(autogen_en)
    {
       ret = CTC_E_HW_BUSY;
       goto error;
    }
    p_pkt_format = &p_cfg->pkt_format;
    p_pkt_header = (uint8*)mem_malloc(MEM_OAM_MODULE, (p_pkt_format->header_len + SYS_USW_PKT_HEADER_LEN));
    if (NULL == p_pkt_header)
    {
       ret = CTC_E_NO_MEMORY;
       goto error;
    }

    /*2. cfg bridge header */
    CTC_ERROR_GOTO(_sys_usw_npm_set_bridge_hdr(lchip, p_cfg, &bridge_header), ret, error);

    sal_memcpy((uint8*)p_pkt_header, (uint8*)&bridge_header, SYS_USW_PKT_HEADER_LEN);
    sal_memcpy((uint8*)((uint8*)p_pkt_header + SYS_USW_PKT_HEADER_LEN), (uint8*)p_pkt_format->pkt_header, p_pkt_format->header_len);

    /*3. cfg user defined packet header */
    CTC_ERROR_GOTO(_sys_usw_npm_set_user_data(lchip, p_cfg, p_pkt_header), ret, error);

    /*4. cfg AutoGen rate */
    CTC_ERROR_GOTO(_sys_usw_npm_set_auto_gen(lchip, p_cfg), ret, error);

    if (CTC_FLAG_ISSET(p_cfg->flag, CTC_NPM_CFG_FLAG_DMR_NOT_TO_CPU))
    {
        SET_BIT(g_npm_master[lchip]->config_dmm_bitmap, p_cfg->session_id);
    }
    /*5. clear stats */
    CTC_ERROR_GOTO(_sys_usw_npm_clear_stats(lchip,p_cfg->session_id), ret, error);

    if (1 == p_cfg->dm_stats_mode)
    {
        value = 0;
    }
    else if (2 == p_cfg->dm_stats_mode)
    {
        value = 1;
    }
    else
    {
        value = 2;
    }
    cmd = DRV_IOW(AutoGenPktTxPktAck_t, AutoGenPktTxPktAck_measureType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_cfg->session_id, cmd, &value), ret, error);

error:
    NPM_UNLOCK(lchip);
    if (p_pkt_header)
    {
        mem_free(p_pkt_header);
    }

    return ret;
}


int32
sys_usw_npm_set_transmit_en(uint8 lchip, uint8 session_id, uint8 enable)
{
    uint32 cmd = 0;
    uint8 mode = 0;
    int32 ret = CTC_E_NONE;
    uint8 max_session = 0;
    uint32 value = 0;
    uint32 dest_map = 0;
    uint32 dest_map_db = 0;
    uint8  modify_dmm_destmap = 0;
    uint8  gchip = 0;
    AutoGenPktGlbCtl_m glb_ctl;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lchip:%d, session_id:%d, enable:%d \n", lchip, session_id, enable);
    LCHIP_CHECK(lchip);
    SYS_NPM_INIT_CHECK(lchip);

    NPM_LOCK(lchip);
    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error1);
    mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);
    max_session = (mode)?((mode == 1)?6:4):8;
    if (session_id >= max_session)
    {
        NPM_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_GOTO(sys_usw_cpu_reason_get_info(lchip, CTC_PKT_CPU_REASON_OAM + CTC_OAM_EXCP_DM_TO_CPU, &dest_map_db), ret, error1);
    if (enable)
    {
        if (IS_BIT_SET(g_npm_master[lchip]->config_dmm_bitmap, session_id))
        {
            if (!g_npm_master[lchip]->tx_dmm_en_bitmap)
            {
                modify_dmm_destmap  = 1;
            }
            SET_BIT(g_npm_master[lchip]->tx_dmm_en_bitmap, session_id);
        }
        if ((modify_dmm_destmap) && DRV_IS_DUET2(lchip))
        {
            sys_usw_get_gchip_id(lchip, &gchip);
            dest_map = SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_DROP_ID);
            cmd = DRV_IOW(DsOamExcp_t, DsOamExcp_destMap_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, SYS_OAM_EXCP_ETH_DM, cmd, &dest_map), ret, error1);
        }

        value = 1;
        cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_autoGenEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &value), ret, error2);
        cmd = DRV_IOW(AutoGenPktEn_t, AutoGenPktEn_autoGenEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &value), ret, error2);
    }
    else
    {
        value = 0;
        cmd = DRV_IOW(AutoGenPktEn_t, AutoGenPktEn_autoGenEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &value), ret, error1);
        sal_task_sleep(100);/*wait for autoGen rx proc*/
        cmd = DRV_IOW(AutoGenRxProc_t, AutoGenRxProc_autoGenEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &value), ret, error1);
        cmd = DRV_IOW(AutoGenPktEn_t, AutoGenPktEn_tokenBytesCnt_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &value), ret, error1);

        sal_task_sleep(50);
        CLEAR_BIT(g_npm_master[lchip]->tx_dmm_en_bitmap, session_id);
        if ((!g_npm_master[lchip]->tx_dmm_en_bitmap) && DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOW(DsOamExcp_t, DsOamExcp_destMap_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, SYS_OAM_EXCP_ETH_DM, cmd, &dest_map_db), ret, error1);
        }
    }
    NPM_UNLOCK(lchip);
    return ret;

error2:
    if ((modify_dmm_destmap) && DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(DsOamExcp_t, DsOamExcp_destMap_f);
        DRV_IOCTL(lchip, SYS_OAM_EXCP_ETH_DM, cmd, &dest_map_db);
    }
error1:
    NPM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_npm_get_stats(uint8 lchip, uint8 session_id, ctc_npm_stats_t* p_stats)
{
    uint32 cmd = 0;
    uint8 mode = 0;
    uint8 max_session = 0;
    int32 ret = 0;
    AutoGenPktTxPktStats_m tx_stats;
    AutoGenPktRxPktStats_m rx_stats;
    AutoGenPktTxPktAck_m tx_ack;
    AutoGenPktGlbCtl_m glb_ctl;
    uint32 autogen_en = 0;
    uint32 tmp[2] = {0};
    uint32 tmp_rx[2] = {0};
    uint64 total_delay;
    uint64 total_far_delay;
    uint64 tx_pkts;
    uint64 rx_pkts;
    uint64 tx_bytes;
    uint64 rx_bytes;
    uint64 min_delay = 0;
    uint64 max_delay = 0;
    uint64 min_jitter = 0;
    uint64 max_jitter = 0;
    uint64 total_jitter = 0;
    uint8  is_ntp_mode = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_stats);
    LCHIP_CHECK(lchip);
    SYS_NPM_INIT_CHECK(lchip);

    NPM_LOCK(lchip);
    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error);
    mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);
    max_session = (mode)?((mode == 1)?6:4):8;
    if (session_id >= max_session)
    {
        NPM_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(AutoGenPktEn_t, AutoGenPktEn_autoGenEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &autogen_en), ret, error);
    p_stats->tx_en = autogen_en ? 1:0;


    /* get AutoGen tx stats */
    if (DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_GOTO(_sys_usw_npm_get_rx_tx_stats(lchip, session_id, 1, (uint32*)&tx_stats), ret, error);
    }
    else
    {
        cmd = DRV_IOR(AutoGenPktTxPktStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &tx_stats), ret, error);
    }
    tx_pkts = GetAutoGenPktTxPktStats(V, txPkts_f, &tx_stats);
    g_npm_master[lchip]->tx_pkts_all[session_id] += tx_pkts;

    GetAutoGenPktTxPktStats(A, txBytes_f , &tx_stats, &tmp);
    tx_bytes = ((uint64)tmp[1]<<32) + tmp[0];
    g_npm_master[lchip]->tx_bytes_all[session_id] += tx_bytes;

    /* get AutoGen rx stats */
    if (DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_GOTO(_sys_usw_npm_get_rx_tx_stats(lchip, session_id, 0, (uint32*)&rx_stats), ret, error);
    }
    else
    {
        cmd = DRV_IOR(AutoGenPktRxPktStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &rx_stats), ret, error);
    }
    rx_pkts = GetAutoGenPktRxPktStats(V, rxPkts_f, &rx_stats);
    g_npm_master[lchip]->rx_pkts_all[session_id] += rx_pkts;

    GetAutoGenPktRxPktStats(A, rxBytes_f , &rx_stats, &tmp_rx);
    rx_bytes = ((uint64)tmp_rx[1]<<32) + tmp_rx[0];
    g_npm_master[lchip]->rx_bytes_all[session_id] += rx_bytes;

    is_ntp_mode = g_npm_master[lchip]->is_ntp_ts[session_id];
    sal_memset(tmp, 0, sizeof(tmp));
    GetAutoGenPktRxPktStats(A, totalDTs_f , &rx_stats, &tmp);
    /*NTP mode: low 32 bits uint: 0.232 ns*/
    total_delay = is_ntp_mode ? ((uint64)tmp[1]*1000000000 + tmp[0]*232 / 1000) : (((uint64)tmp[1] << 32) + tmp[0]);
    g_npm_master[lchip]->total_delay_all[session_id] += total_delay;

    sal_memset(tmp, 0, sizeof(tmp));
    GetAutoGenPktRxPktStats(A, totalDts2_f , &rx_stats, &tmp);
    total_far_delay = is_ntp_mode ? ((uint64)tmp[1]*1000000000 + tmp[0]*232 / 1000) : (((uint64)tmp[1] << 32) + tmp[0]);
    g_npm_master[lchip]->total_far_delay_all[session_id] += total_far_delay;

    p_stats->tx_bytes = g_npm_master[lchip]->tx_bytes_all[session_id];
    p_stats->tx_pkts = g_npm_master[lchip]->tx_pkts_all[session_id];

    p_stats->rx_pkts = g_npm_master[lchip]->rx_pkts_all[session_id];
    p_stats->rx_bytes = g_npm_master[lchip]->rx_bytes_all[session_id];

    p_stats->total_delay = g_npm_master[lchip]->total_delay_all[session_id];
    p_stats->total_far_delay = g_npm_master[lchip]->total_far_delay_all[session_id];
    if (p_stats->total_delay > p_stats->total_far_delay)
    {
        p_stats->total_near_delay = p_stats->total_delay - p_stats->total_far_delay;
    }

    max_jitter = GetAutoGenPktRxPktStats(V, maxJitter_f, &rx_stats);
    max_jitter = is_ntp_mode ? (max_jitter *232 / 1000) : max_jitter;
    g_npm_master[lchip]->max_jitter[session_id] = (g_npm_master[lchip]->max_jitter[session_id] < max_jitter) ? max_jitter : g_npm_master[lchip]->max_jitter[session_id];
    p_stats->max_jitter = g_npm_master[lchip]->max_jitter[session_id];

    total_jitter = GetAutoGenPktRxPktStats(V, totalJitter_f, &rx_stats);
    total_jitter = is_ntp_mode ? (total_jitter *232 / 1000) : total_jitter;
    g_npm_master[lchip]->total_jitter_all[session_id] += total_jitter;
    p_stats->total_jitter = g_npm_master[lchip]->total_jitter_all[session_id];

    sal_memset(tmp, 0, sizeof(tmp));
    GetAutoGenPktRxPktStats(A, minDelay_f , &rx_stats, &tmp);
    min_delay = is_ntp_mode ? ((uint64)tmp[1]*1000000000 + tmp[0]*232 / 1000) : (((uint64)tmp[1] << 32) + tmp[0]);
    min_jitter = GetAutoGenPktRxPktStats(V, minJitter_f, &rx_stats);
    min_jitter = is_ntp_mode ? (min_jitter *232 / 1000) : min_jitter;
    if (min_delay)/*min_delay=0, means stats has been cleared on read*/
    {
        g_npm_master[lchip]->min_delay[session_id] = (g_npm_master[lchip]->min_delay[session_id] > min_delay) ? min_delay : g_npm_master[lchip]->min_delay[session_id];
        g_npm_master[lchip]->min_jitter[session_id] = (g_npm_master[lchip]->min_jitter[session_id] > min_jitter) ? min_jitter : g_npm_master[lchip]->min_jitter[session_id];
    }
    p_stats->min_delay = g_npm_master[lchip]->min_delay[session_id];
    p_stats->min_delay = (SYS_NPM_UINT64_MAX == p_stats->min_delay) ? 0 : p_stats->min_delay;
    p_stats->min_jitter = g_npm_master[lchip]->min_jitter[session_id];
    p_stats->min_jitter = (CTC_MAX_UINT32_VALUE == p_stats->min_jitter) ? 0 : p_stats->min_jitter;

    sal_memset(tmp, 0, sizeof(tmp));
    GetAutoGenPktRxPktStats(A, maxDelay_f , &rx_stats, &tmp);
    max_delay= is_ntp_mode ? ((uint64)tmp[1]*1000000000 + tmp[0]*232 / 1000) : (((uint64)tmp[1] << 32) + tmp[0]);
    g_npm_master[lchip]->max_delay[session_id] = (g_npm_master[lchip]->max_delay[session_id] < max_delay) ? max_delay : g_npm_master[lchip]->max_delay[session_id];
    p_stats->max_delay = g_npm_master[lchip]->max_delay[session_id];

    sal_memset(tmp, 0, sizeof(tmp));
    GetAutoGenPktRxPktStats(A, lastTs_f , &rx_stats, &tmp);
    p_stats->last_ts = is_ntp_mode ? ((uint64)tmp[1]*1000000000 + tmp[0]*232/1000) : ((uint64)tmp[1]*1000000000 + tmp[0]);


    /* get AutoGen txAck stats */
    cmd = DRV_IOR(AutoGenPktTxPktAck_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, session_id, cmd, &tx_ack), ret, error);
    sal_memset(tmp, 0, sizeof(tmp));
    GetAutoGenPktTxPktAck(A, firstTs_f , &tx_ack, &tmp);
    if (!g_npm_master[lchip]->is_ntp_ts[session_id])
    {
        p_stats->first_ts = (uint64)tmp[1]*1000000000 + tmp[0];
    }
    else
    {
        p_stats->first_ts = (uint64)tmp[1]*1000000000 + tmp[0]*232/1000;
    }

    p_stats->tx_fcf = GetAutoGenPktTxPktAck(V, txSeqNum_f , &tx_ack);
    p_stats->tx_fcb = GetAutoGenPktTxPktAck(V, txFcb_f , &tx_ack);
    p_stats->rx_fcl = GetAutoGenPktTxPktAck(V, rxFcl_f , &tx_ack);

error:
    NPM_UNLOCK(lchip);
    return ret;

}


int32
sys_usw_npm_clear_stats(uint8 lchip, uint8 session_id)
{
    int32 ret = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    NPM_LOCK(lchip);
    ret = _sys_usw_npm_clear_stats( lchip,  session_id);
    NPM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_npm_set_global_config(uint8 lchip, ctc_npm_global_cfg_t* p_npm)
{
    uint32 cmd = 0;
    int32 ret = 0;
    AutoGenPktGlbCtl_m glb_ctl;
    uint32 value = 0;
    uint8 index = 0;
    uint8 loop = 0;
    uint32 field_id = 0;
    uint32 autogen_en = 0;

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_npm);
    CTC_MAX_VALUE_CHECK(p_npm->session_mode, (CTC_NPM_SESSION_MODE_MAX-1));

    sal_memset(&glb_ctl, 0, sizeof(AutoGenPktGlbCtl_m));
    SYS_NPM_INIT_CHECK(lchip);

    NPM_LOCK(lchip);
    cmd = DRV_IOR(AutoGenPktEn_t, AutoGenPktEn_autoGenEn_f);
    for (loop = 0; loop < 8; loop++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &autogen_en), ret, error);
        if (autogen_en)
        {
            ret = CTC_E_HW_BUSY;
            goto error;
        }
    }

    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error);
    value = p_npm->session_mode;
    SetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl, value);

    for (index = 0; index < CTC_NPM_MAX_EMIX_NUM; index++)
    {
        field_id = AutoGenPktGlbCtl_g_0_size_f + index;
        value = p_npm->emix_size[index];
        if ((value < 64) && value)
        {
            NPM_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }

        SYS_MAX_VALUE_CHECK_WITH_UNLOCK(value, 16127, g_npm_master[lchip]->npm_mutex);
        drv_set_field(lchip, AutoGenPktGlbCtl_t, field_id, &glb_ctl, &value);
    }

    cmd = DRV_IOW(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error);

error:
    NPM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_npm_get_global_config(uint8 lchip, ctc_npm_global_cfg_t* p_npm)
{
    uint32 cmd = 0;
    AutoGenPktGlbCtl_m glb_ctl;
    uint8 index = 0;
    uint32 field_id = 0;
    uint32 value = 0;
    int32 ret = 0;
    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_npm);
    LCHIP_CHECK(lchip);
    sal_memset(&glb_ctl, 0, sizeof(AutoGenPktGlbCtl_m));
    SYS_NPM_INIT_CHECK(lchip);

    NPM_LOCK(lchip);
    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error);
    p_npm->session_mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);

    for (index = 0; index < CTC_NPM_MAX_EMIX_NUM; index++)
    {
        field_id = AutoGenPktGlbCtl_g_0_size_f + index;
        drv_get_field(lchip, AutoGenPktGlbCtl_t, field_id, &glb_ctl, &value);
        p_npm->emix_size[index] = value;
    }

error:
    NPM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_npm_show_status(uint8 lchip)
{
    int32 ret = 0;
    uint8 loop = 0;
    uint32 cmd = 0;
    uint32 autogen_en = 0;
    uint8 session_count = 0;
    ctc_npm_cfg_t cfg;
    char* PATTEN_TYPE_STR[8] =
    {
        "Repeat", "Pbrs", "Inc_byte",
        "Dec_byte", "Inc_word", "Dec_word"
    };

    char* PKT_SIZE_MODE_STR[5] =
    {
        "Fix", "Increase", "Emix"
    };

    char* TX_MODE_STR[5] =
    {
        "Continuous", "Stop after fix num", "Stop after fix time"
    };

    sal_memset(&cfg, 0, sizeof(cfg));

    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_NPM_INIT_CHECK(lchip);

    NPM_LOCK(lchip);
    for (loop = 0; loop < 8; loop++)
    {
        cmd = DRV_IOR(AutoGenPktEn_t, AutoGenPktEn_autoGenEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &autogen_en), ret, error);
        if (autogen_en)
        {
            cfg.session_id = loop;
            ret = _sys_usw_npm_show_session_status(lchip, &cfg);
            if (ret < 0)
            {
                goto error;
            }
            session_count ++;
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nNPM Session %u\n", loop);
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------\n");
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%u \n", "IPG", cfg.pkt_format.ipg);
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%s \n", "Tx-mode", TX_MODE_STR[cfg.tx_mode]);
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%s \n", "Pkt-size-mode", PKT_SIZE_MODE_STR[cfg.pkt_format.frame_size_mode]);
            SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :%s \n", "Payload pattern type", PATTEN_TYPE_STR[cfg.pkt_format.pattern_type]);
        }
    }
    SYS_NPM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nTotal NPM Session number :%u\n", session_count);

error:
    NPM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_npm_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 loop = 0;
    uint32 valid_count = 0;
    SYS_NPM_INIT_CHECK(lchip);
    LCHIP_CHECK(lchip);

    NPM_LOCK(lchip);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# NPM");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%x\n","config_dmm_bitmap", g_npm_master[lchip]->config_dmm_bitmap);
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%x\n","tx_dmm_en_bitmap", g_npm_master[lchip]->tx_dmm_en_bitmap);

    SYS_DUMP_DB_LOG(p_f, "%-30s:","total_delay_all");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->total_delay_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->total_delay_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","total_far_delay_all");
    valid_count = 0;
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->total_far_delay_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->total_far_delay_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","tx_pkts_all");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->tx_pkts_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->tx_pkts_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","rx_pkts_all");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->rx_pkts_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->rx_pkts_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","tx_bytes_all");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->tx_bytes_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->tx_bytes_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","rx_bytes_all");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->rx_bytes_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->rx_bytes_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","max_delay");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->max_delay[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->max_delay[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","min_delay");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (SYS_NPM_UINT64_MAX == g_npm_master[lchip]->min_delay[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->min_delay[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","max_jitter");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->max_jitter[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->max_jitter[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","min_jitter");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (CTC_MAX_UINT32_VALUE == g_npm_master[lchip]->min_jitter[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->min_jitter[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","total_jitter_all");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->total_jitter_all[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%"PRIu64"]", loop, g_npm_master[lchip]->total_jitter_all[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","is_ntp_ts");
    for(loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        if (0 == g_npm_master[lchip]->is_ntp_ts[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%u]", loop, g_npm_master[lchip]->is_ntp_ts[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    NPM_UNLOCK(lchip);

    return ret;
}

int32
sys_usw_npm_init(uint8 lchip)
{
    uint32 cmd = 0;
    int32 ret = 0;
    uint32 loop = 0;
    AutoGenPktGlbCtl_m glb_ctl;
    RefDivOamTokenUpd_m oam_div;
    uint32 field_value = 0;
    LCHIP_CHECK(lchip);
    /* check init */
    if (g_npm_master[lchip])
    {
        return CTC_E_NONE;
    }

    g_npm_master[lchip] = (sys_npm_master_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_npm_master_t));
    if (NULL == g_npm_master[lchip])
    {
        goto error1;
    }
    sal_memset(g_npm_master[lchip], 0 ,sizeof(sys_npm_master_t));

    if (CTC_E_NONE != sal_mutex_create(&(g_npm_master[lchip]->npm_mutex)))
    {
        goto error1;
    }

    for (loop = 0; loop < SYS_NPM_MAX_SESSION_NUM; loop++)
    {
        g_npm_master[lchip]->min_delay[loop] = SYS_NPM_UINT64_MAX;
        g_npm_master[lchip]->min_jitter[loop] = CTC_MAX_UINT32_VALUE;
    }
    NPM_LOCK(lchip);
    /* 1. default stats should be clear on read */
    cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error);
    SetAutoGenPktGlbCtl(V, clearOnRead_f, &glb_ctl, 1);
    cmd = DRV_IOW(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &glb_ctl), ret, error);

    cmd = DRV_IOR(RefDivOamTokenUpd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &oam_div), ret, error);
    SetRefDivOamTokenUpd(V, cfgResetDivOamTokenUpdPulse_f, &oam_div, 0); /* default using 1M clock for AutoGen */
    cmd = DRV_IOW(RefDivOamTokenUpd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &oam_div), ret, error);

    /* 2. Set AutoGen Ref Clock, div from 32M Clock or 156.25M, for emulation using 256.25M*/
    cmd = DRV_IOR(RefDivOamTokenUpd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &oam_div), ret, error);
    SetRefDivOamTokenUpd(V, cfgRefDivOamTokenUpdPulse_f, &oam_div, 0x9b40); /* default using 1M clock for AutoGen */
    cmd = DRV_IOW(RefDivOamTokenUpd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &oam_div), ret, error);


    field_value = 100;    /*smWithDmTlvType = 0x64*/
    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_smWithDmTlvType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);

    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_smWithDmTlvType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 0;
    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_twampLightEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);

    field_value = 1;
    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_twampTsType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 0;
    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_dontCareAclTwampTsType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 1;
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_twampTsType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 0;
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dontCareTwampTsType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_twampSbit_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 64;
    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_twampScale_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 1;
    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_twampMulti_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 1;
    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_twampTsType_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    field_value = 0;
    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_twampTsTypeChkDis_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_NPM, sys_usw_npm_dump_db), ret, error);
    NPM_UNLOCK(lchip);
    return CTC_E_NONE;

error1:
    if (NULL != g_npm_master[lchip]->npm_mutex)
    {
        sal_mutex_destroy(g_npm_master[lchip]->npm_mutex);
        g_npm_master[lchip]->npm_mutex = NULL;
    }

    if (NULL != g_npm_master[lchip])
    {
        mem_free(g_npm_master[lchip]);
    }
    return CTC_E_NO_MEMORY;

error:
    NPM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_npm_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == g_npm_master[lchip])
    {
        return CTC_E_NONE;
    }
    sal_mutex_destroy(g_npm_master[lchip]->npm_mutex);
    mem_free(g_npm_master[lchip]);
    return CTC_E_NONE;
}

#endif

