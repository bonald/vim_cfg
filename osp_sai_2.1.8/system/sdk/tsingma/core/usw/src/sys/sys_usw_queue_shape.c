/**
 @file sys_usw_queue_shp_shape.c

 @date 2010-01-13

 @version v2.0

*/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_queue.h"
#include "ctc_spool.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_qos.h"
#include "sys_usw_ftm.h"
#include "sys_usw_port.h"
#include "sys_usw_dmps.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/

#define SYS_SPECIAL_SHAPE_BURST             0xFFFFFFFF
#define SYS_RESERVED_SHAPE_PROFILE_ID       0
#define SYS_RESERVED_GROUP_SHAPE_PROFILE_ID 0
#define SYS_QUEUE_SHAPE_HASH_BUCKET_SIZE     64
#define SYS_SHAPE_DEFAULT_RATE              0xFFFFFFFC
#define SYS_SHAPE_DEFAULT_THRD              267386

#define SYS_QUEUE_SHAPE_PROFILE_BLOCK_SIZE 8
#define SYS_GROUP_SHAPE_PROFILE_BLOCK_SIZE 8
#define SYS_QOS_SHAPE_MAX_COUNT     (0xFF<<0x10)
#define SYS_QOS_SHAPE_MAX_ECNMARK_RATE     32000000
#define AQM_PORT_THRD_HIGH       768
#define AQM_PORT_THRD_LOW        512
#define MIN(a,b)  ((a) > (b) ? (b) : (a))

#define SYS_QOS_COMPUTE_SHAPE_TOKEN_THRD(cbs, cir, shift,is_pps) {\
    if (0 == cbs)\
    {\
        if (is_pps)\
        {\
            cbs = 16 * 1000;\
        }\
        else\
        {\
            cbs = (cir >> shift) + 1;\
        }\
    }\
    else\
    {\
        if (is_pps)\
        {\
            cbs = cbs * MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES);\
        }\
        else\
        {\
            cbs = cbs * 125;\
        }\
    }\
}


extern sys_queue_master_t* p_usw_queue_master[CTC_MAX_LOCAL_CHIP_NUM];

/****************************************************************************
 *
 * Function
 *
 ****************************************************************************/

int32
_sys_usw_qos_map_token_thrd_user_to_hw(uint8 lchip, uint32  user_bucket_thrd,
                                            uint32 *hw_bucket_thrd,
                                            uint8 shift_bits,
                                            uint32 max_thrd)
{
/*PBS = DsChanShpProfile.tokenThrd * 2^DsChanShpProfile.tokenThrdShift*/
 /*user_bucket_thrd = min_near_division_value * 2^min_near_shift_value */

 /*tokenThrdShift = ceil(log2[(PBS + 255) / 256]) */
 /* tokenThrd = ceil(PBS / 2^DsChanShpProfile.tokenThrdShift) */

    int8 loop = 0;
    uint32 shift_bits_value = (1 << shift_bits) - 1;
    uint32 min_near_value = 1 << shift_bits_value;
    uint32 min_near_division_value = 0;
    uint32 temp_value = 0;
    uint32 mod_value = 0;
    uint32 division_value;
    uint32 shift_value = 0;
    uint8  min_near_shift_value = 0;

    if (user_bucket_thrd == 0)
    {
        *hw_bucket_thrd  = 0;
        return CTC_E_NONE;
    }

    for (loop = shift_bits_value; loop >= 0; loop--)
    {
        shift_value = (1 << loop);
        mod_value = user_bucket_thrd % shift_value;
        division_value = user_bucket_thrd / shift_value;
        if (division_value > max_thrd)
        {
            break;
        }

        if (division_value == 0)
        {
            continue;
        }

        if (mod_value == 0)
        {
            min_near_value = 0;
            min_near_division_value = division_value;
            min_near_shift_value = loop;
            break;
        }

        temp_value = (division_value + 1) * shift_value - user_bucket_thrd;
        if (temp_value < min_near_value)
        {
            min_near_value = temp_value;
            min_near_division_value = division_value + 1;
            min_near_shift_value = loop;
        }
    }

    if (min_near_value == (1<< shift_bits_value))
    {
        return CTC_E_INVALID_PARAM;
    }

    *hw_bucket_thrd = min_near_division_value << shift_bits | min_near_shift_value;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "valid_bucket_thrd:%d, user_bucket_thrd:%d\n",
    (min_near_division_value << min_near_shift_value),user_bucket_thrd);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_map_token_thrd_hw_to_user(uint8 lchip, uint32  *user_bucket_thrd,
                                            uint32 hw_bucket_thrd,
                                            uint8 shift_bits)
{
   uint16 shift_bits_value = (1 << shift_bits) - 1;

   uint32 bucket_thrd = hw_bucket_thrd >> shift_bits;
   uint16 bucket_shift  = hw_bucket_thrd & shift_bits_value;

   *user_bucket_thrd  = bucket_thrd << bucket_shift;

    return CTC_E_NONE;
}

/**
 @brief Compute channel shape token rate.
*/
int32
sys_usw_qos_get_shp_granularity(uint8 lchip, uint32 rate, /*kb/s*/
                                            uint8 type,
                                            uint32 *token_rate_gran)
{
    uint32 token_rate_Mbps = 0;
    uint8 loop = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (type == SYS_SHAPE_TYPE_PORT)
    {
        *token_rate_gran = 8;  /**/
    }
    else
    {
        *token_rate_gran = 8;
        token_rate_Mbps = rate / 1000;
        for (loop =0; loop < SYS_SHP_GRAN_RANAGE_NUM; loop++)
        {
            if ( token_rate_Mbps < p_usw_queue_master[lchip]->que_shp_gran[loop].max_rate)
            {
                *token_rate_gran = p_usw_queue_master[lchip]->que_shp_gran[loop].granularity;
                break;
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_map_token_rate_user_to_hw(uint8 lchip, uint8 is_pps,
                                             uint32 user_rate, /*kb/s*/
                                             uint32 *hw_rate,
                                             uint16 bit_with,
                                             uint32 gran,
                                             uint32 upd_freq,
                                             uint16 pps_pkt_byte)
{
    uint32 token_rate = 0;
    uint32 mod_rate = 0;
    uint8 token_rate_frac =0;

    if (is_pps)
    {
        token_rate =  user_rate / (upd_freq / pps_pkt_byte);   /* ---> user_rate *pps_pkt_byte/upd_freq, one packet convers to pps_pkt_byte(128)B */
        mod_rate = user_rate % (upd_freq / pps_pkt_byte);
        token_rate_frac = (mod_rate *bit_with) / (upd_freq / pps_pkt_byte);
        *hw_rate = token_rate << 8 | token_rate_frac;
    }
    else
    {
        user_rate  = (user_rate / gran) * gran;
        token_rate = (((uint64)user_rate * 1000) / 8) / (upd_freq);   /* ---> user_rate *1000/upd_freq */
        mod_rate = (((uint64)user_rate*1000) / 8) % (upd_freq);   /*kb*/

        /*mod_rate/upd_freq  = x/256   --> x = mod_rate *1000(bytes) * 256/upd_freq
        -- > mod_rate(kb) * 256 / (upd_freq / 1000) */
        token_rate_frac = (mod_rate * bit_with) / (upd_freq);
        *hw_rate = token_rate << 8 | token_rate_frac;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_map_token_rate_hw_to_user(uint8 lchip, uint8 is_pps,
                                            uint32 hw_rate,
                                            uint32 *user_rate,
                                            uint16 bit_with,
                                            uint32 upd_freq,
                                            uint16 pps_pkt_byte)
{
    uint32 token_rate = hw_rate >> 8;
    uint8  token_rate_frac = hw_rate & 0xFF;

    if (is_pps)
    {
        *user_rate = (((uint64)token_rate * upd_freq) + (uint64)token_rate_frac * upd_freq / bit_with) / pps_pkt_byte;
    }
    else
    {
        /*unit:kb/s*/
        /*(token_rate + token_rate_frac/bit_with) * upd_freq * 8 /1000  -->  kbps/s */
        *user_rate = ((uint64)token_rate * upd_freq / 1000)*8 + ((uint64)token_rate_frac*upd_freq/1000*8)/bit_with;
    }

    return 0;
}

STATIC int32
_sys_usw_qos_map_ecnmark_rate_user_to_hw(uint8 lchip,
                                            uint32 *hw_rate,
                                            uint32 user_rate)
{
    uint16 interval = 0;
    uint16 max_ptr = 0;
    uint32 cmd = 0;
    ErmAqmPortScanCtl_m aqm_port_ctl;
    uint16 corefreq = 0;

    sal_memset(&aqm_port_ctl, 0, sizeof(ErmAqmPortScanCtl_m));

    cmd = DRV_IOR(ErmAqmPortScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aqm_port_ctl));

    interval = GetErmAqmPortScanCtl(V, aqmPortScanInterval_f, &aqm_port_ctl);
    max_ptr = GetErmAqmPortScanCtl(V, aqmPortScanPtrMax_f, &aqm_port_ctl);
    corefreq = sys_usw_get_core_freq(lchip,0);
    /* updaterate=(userrate*1000*interval*maxptr)/(8*corefreq) */
    *hw_rate = ((uint64)user_rate * (interval+1) * (max_ptr+1)) / (corefreq * 1000 * 8);

    return CTC_E_NONE;
}

#define chan_shp ""
bool
sys_usw_queue_shp_get_channel_pps_enbale(uint8 lchip, uint8 chand_id)
{
    uint32 cmd = 0;
    uint32 pps_en = 0;

    if(chand_id < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        cmd = DRV_IOR(DsQMgrNetShpPpsCfg_t, DsQMgrNetShpPpsCfg_ppsMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chand_id, cmd, &pps_en));
    }
    if ((p_usw_queue_master[lchip]->shp_pps_en &&
        (chand_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0)
        || chand_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1)
        || chand_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2)
        || chand_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)
        || (p_usw_queue_master[lchip]->have_lcpu_by_eth && chand_id == p_usw_queue_master[lchip]->cpu_eth_chan_id)))
        || pps_en)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}
STATIC int32
_sys_usw_qos_shp_set_ecnmark_rate(uint8 lchip, uint16 channel,uint32 ecn_mark_rate)
{
    uint32 cmd                     = 0;
    uint32 update_rate = 0;
    DsErmAqmPortCfg_m aqm_port_cfg;
    DsErmAqmPortThrd_m aqm_port_thrd;

    sal_memset(&aqm_port_cfg, 0, sizeof(DsErmAqmPortCfg_m));
    sal_memset(&aqm_port_thrd, 0, sizeof(DsErmAqmPortThrd_m));

    CTC_ERROR_RETURN(_sys_usw_qos_map_ecnmark_rate_user_to_hw(lchip, &update_rate, ecn_mark_rate));
    cmd = DRV_IOR(DsErmAqmPortCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (channel & 0x3F), cmd, &aqm_port_cfg));
    SetDsErmAqmPortCfg(V, aqmPortUpdateEn_f, &aqm_port_cfg, (ecn_mark_rate ? 1 : 0));
    SetDsErmAqmPortCfg(V, avgPortLenResetEn_f, &aqm_port_cfg, (ecn_mark_rate ? 1 : 0));
    SetDsErmAqmPortCfg(V, aqmPortUpdateRate_f, &aqm_port_cfg, update_rate);
    cmd = DRV_IOW(DsErmAqmPortCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (channel & 0x3F), cmd, &aqm_port_cfg));

    cmd = DRV_IOR(DsErmAqmPortThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (channel & 0x3F), cmd, &aqm_port_thrd));
    SetDsErmAqmPortThrd(V, aqmPortThrdHigh_f, &aqm_port_thrd, (ecn_mark_rate ? AQM_PORT_THRD_HIGH : 0xFFFFFF));
    SetDsErmAqmPortThrd(V, aqmPortThrdLow_f, &aqm_port_thrd, (ecn_mark_rate ? AQM_PORT_THRD_LOW : 0xFFFFFF));
    cmd = DRV_IOW(DsErmAqmPortThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (channel & 0x3F), cmd, &aqm_port_thrd));

    return CTC_E_NONE;
}
int32
sys_usw_queue_shp_set_channel_shape_enable(uint8 lchip, uint16 channel,
                                             ctc_qos_shape_port_t* p_shape)
{
    uint32 cmd                     = 0;
    uint32 db_rate                 = 0;
    uint32 gran                    = 0;
    uint8  is_pps = 0;
    uint32 threshold = 0;
    uint32 value = 0;

    CTC_PTR_VALID_CHECK(p_shape);
    CTC_MAX_VALUE_CHECK(p_shape->ecn_mark_rate, SYS_QOS_SHAPE_MAX_ECNMARK_RATE);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_usw_qos_get_shp_granularity(lchip, p_shape->pir,
                                           SYS_SHAPE_TYPE_PORT, &gran);

    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);
    if(1 == is_pps)
    {
        CTC_MAX_VALUE_CHECK(p_shape->pir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE_PPS));
        p_shape->pbs = MIN(p_shape->pbs,(SYS_QOS_SHAPE_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_shape->pir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE));
        p_shape->pbs = MIN(p_shape->pbs,(SYS_QOS_SHAPE_MAX_COUNT / 125));
    }

    /* compute token rate */
    CTC_ERROR_RETURN(sys_usw_qos_map_token_rate_user_to_hw(lchip, is_pps, p_shape->pir,
                                                                  &db_rate,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                                  gran,
                                                                  p_usw_queue_master[lchip]->chan_shp_update_freq,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));

    SYS_QOS_COMPUTE_SHAPE_TOKEN_THRD( p_shape->pbs, db_rate, 8,is_pps);
    /* compute token threshold & shift */
    CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, p_shape->pbs,
                                                                  &threshold,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH),
                                                                  SYS_SHAPE_MAX_TOKEN_THRD));


    if (SYS_IS_NETWORK_CHANNEL(channel))
    {
        DsQMgrNetChanShpProfile_m  net_chan_shp_profile;
        sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));

        SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, (threshold >> 5));
        SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, threshold & 0x1F);
        SetDsQMgrNetChanShpProfile(V, tokenRateFrac_f, &net_chan_shp_profile, (db_rate & 0xFF));
        SetDsQMgrNetChanShpProfile(V, tokenRate_f  , &net_chan_shp_profile, (db_rate >> 8));

        cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &net_chan_shp_profile));

        cmd = DRV_IOW(DsQMgrNetChanShpToken_t, DsQMgrNetChanShpToken_token_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &value));

        CTC_ERROR_RETURN(_sys_usw_qos_shp_set_ecnmark_rate(lchip, channel, p_shape->ecn_mark_rate));
    }
    else if (SYS_IS_DMA_CHANNEL(channel))
    {
        QMgrDmaChanShpProfile_m  dma_chan_shp_profile;
        sal_memset(&dma_chan_shp_profile, 0, sizeof(QMgrDmaChanShpProfile_m));

        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrd_f, &dma_chan_shp_profile, (threshold >> 5));
        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrdShift_f  , &dma_chan_shp_profile, threshold & 0x1F);
        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenRateFrac_f, &dma_chan_shp_profile, (db_rate & 0xFF));
        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenRate_f  , &dma_chan_shp_profile, (db_rate >> 8));

        cmd = DRV_IOW(QMgrDmaChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_chan_shp_profile));

        cmd = DRV_IOW(QMgrDmaChanShpToken_t, QMgrDmaChanShpToken_dmaChanShpToken_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }
    else
    {
        DsQMgrMiscChanShpProfile_m  misc_chan_shp_profile;
        sal_memset(&misc_chan_shp_profile, 0, sizeof(DsQMgrMiscChanShpProfile_m));

        SetDsQMgrMiscChanShpProfile(V, tokenThrd_f, &misc_chan_shp_profile, (threshold >> 5));
        SetDsQMgrMiscChanShpProfile(V, tokenThrdShift_f  , &misc_chan_shp_profile, threshold & 0x1F);
        SetDsQMgrMiscChanShpProfile(V, tokenRateFrac_f, &misc_chan_shp_profile, (db_rate & 0xFF));
        SetDsQMgrMiscChanShpProfile(V, tokenRate_f  , &misc_chan_shp_profile, (db_rate >> 8));

        cmd = DRV_IOW(DsQMgrMiscChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel-MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM), cmd, &misc_chan_shp_profile));

        cmd = DRV_IOW(DsQMgrMiscChanShpToken_t, DsQMgrMiscChanShpToken_token_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel-MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM), cmd, &value));
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tokenRate=%d\n", (db_rate >> 8));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tokenRateFrac=%d\n", (db_rate & 0xFF));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tokenThrd=%d\n", (threshold >> 5));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tokenThrdShift=%d\n", (threshold & 0x1F));

    return CTC_E_NONE;
}

/**
 @brief Unset shaping for the given channel in a chip.
*/
int32
sys_usw_queue_shp_set_channel_shape_disable(uint8 lchip, uint16 channel,
                                                            ctc_qos_shape_port_t* p_shape)
{
    uint32 cmd = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_MAX_VALUE_CHECK(p_shape->ecn_mark_rate, SYS_QOS_SHAPE_MAX_ECNMARK_RATE);

    if (SYS_IS_NETWORK_CHANNEL(channel))
    {
        DsQMgrNetChanShpProfile_m  net_chan_shp_profile;
        sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));

        SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
        SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
        SetDsQMgrNetChanShpProfile(V, tokenRateFrac_f, &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
        SetDsQMgrNetChanShpProfile(V, tokenRate_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));

        cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &net_chan_shp_profile));

        CTC_ERROR_RETURN(_sys_usw_qos_shp_set_ecnmark_rate(lchip, channel, p_shape->ecn_mark_rate));
    }
    else if (SYS_IS_DMA_CHANNEL(channel))
    {
        QMgrDmaChanShpProfile_m  dma_chan_shp_profile;
        sal_memset(&dma_chan_shp_profile, 0, sizeof(QMgrDmaChanShpProfile_m));

        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrd_f, &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrdShift_f  , &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenRateFrac_f, &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
        SetQMgrDmaChanShpProfile(V, dmaChanShpTokenRate_f  , &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));

        cmd = DRV_IOW(QMgrDmaChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_chan_shp_profile));
    }
    else
    {
        DsQMgrMiscChanShpProfile_m  misc_chan_shp_profile;
        sal_memset(&misc_chan_shp_profile, 0, sizeof(DsQMgrMiscChanShpProfile_m));

        SetDsQMgrMiscChanShpProfile(V, tokenThrd_f, &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
        SetDsQMgrMiscChanShpProfile(V, tokenThrdShift_f  , &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
        SetDsQMgrMiscChanShpProfile(V, tokenRateFrac_f, &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
        SetDsQMgrMiscChanShpProfile(V, tokenRate_f  , &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));

        cmd = DRV_IOW(DsQMgrMiscChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel-MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM), cmd, &misc_chan_shp_profile));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_shp_set_channel_shape(uint8 lchip, ctc_qos_shape_port_t* p_shape)
{
    uint16 lport = 0;
    uint8 channel = 0;

    if (CTC_IS_CPU_PORT(p_shape->gport))
    {
        channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);
    }
    else
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_shape->gport, lchip, lport);
        _sys_usw_get_channel_by_port(lchip, p_shape->gport,&channel);
    }

    CTC_MAX_VALUE_CHECK(channel, MCHIP_CAP(SYS_CAP_CHANID_MAX) - 1);

    if (p_shape->enable)
    {
        CTC_ERROR_RETURN(
            sys_usw_queue_shp_set_channel_shape_enable(lchip, channel, p_shape));
    }
    else
    {
        CTC_ERROR_RETURN(
            sys_usw_queue_shp_set_channel_shape_disable(lchip, channel, p_shape));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_shp_get_channel_shape(uint8 lchip, ctc_qos_shape_port_t* p_shape)
{
    uint32 token_thrd = 0;
    uint32 token_rate = 0;
    uint8 enable = 0;

    CTC_ERROR_RETURN(_sys_usw_qos_get_port_shape_profile(lchip, p_shape->gport, &token_rate, &token_thrd, &enable));
    p_shape->enable = enable;
    p_shape->pir = token_rate;
    p_shape->pbs = token_thrd;

    return CTC_E_NONE;
}
int32
_sys_usw_queue_shp_init_channel_shape(uint8 lchip, uint8 slice_id)
{
    uint16 channel_id              = 0;
    uint32 cmd                     = 0;
    uint32 field_val               = 0;

    DsQMgrNetChanShpProfile_m net_chan_shp_profile;
    DsQMgrMiscChanShpProfile_m misc_chan_shp_profile;
    QMgrDmaChanShpProfile_m dma_chan_shp_profile;
    QMgrNetShpCtl_m  net_shape_ctl;
    QMgrMiscShpCtl_m misc_shape_ctl;
    ErmAqmPortScanCtl_m aqm_port_ctl;

    sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));
    sal_memset(&misc_chan_shp_profile, 0, sizeof(DsQMgrMiscChanShpProfile_m));
    sal_memset(&dma_chan_shp_profile, 0, sizeof(QMgrDmaChanShpProfile_m));
    sal_memset(&net_shape_ctl, 0, sizeof(QMgrNetShpCtl_m));
    sal_memset(&misc_shape_ctl, 0, sizeof(QMgrMiscShpCtl_m));
    sal_memset(&aqm_port_ctl, 0, sizeof(ErmAqmPortScanCtl_m));

    for (channel_id = 0; channel_id < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); channel_id++)
    {
        SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
        SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT)); /* 0xFFFFFF<(tokenThrd << tokenThrdShift) < 0x1FFFFFF*/
        SetDsQMgrNetChanShpProfile(V, tokenRateFrac_f, &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
        SetDsQMgrNetChanShpProfile(V, tokenRate_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
        cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &net_chan_shp_profile));
    }

    for (channel_id = 0; channel_id < 16; channel_id++)
    {
        SetDsQMgrMiscChanShpProfile(V, tokenThrd_f, &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
        SetDsQMgrMiscChanShpProfile(V, tokenThrdShift_f  , &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
        SetDsQMgrMiscChanShpProfile(V, tokenRateFrac_f, &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
        SetDsQMgrMiscChanShpProfile(V, tokenRate_f  , &misc_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
        cmd = DRV_IOW(DsQMgrMiscChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &misc_chan_shp_profile));
    }

    SetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrd_f, &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
    SetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrdShift_f  , &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    SetQMgrDmaChanShpProfile(V, dmaChanShpTokenRateFrac_f, &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
    SetQMgrDmaChanShpProfile(V, dmaChanShpTokenRate_f  , &dma_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
    cmd = DRV_IOW(QMgrDmaChanShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_chan_shp_profile));

    SetQMgrNetShpCtl(V, dot1qavShpMode_f, &net_shape_ctl, 0);
    SetQMgrNetShpCtl(V, netChanShpPtrMax_f, &net_shape_ctl, 63);
    SetQMgrNetShpCtl(V, netChanShpPtrMin_f, &net_shape_ctl, 0);
    SetQMgrNetShpCtl(V, netChanShpRefreshDis_f, &net_shape_ctl, 0);
    SetQMgrNetShpCtl(V, netChanShpTickInterval_f, &net_shape_ctl, 1);
    cmd = DRV_IOW(QMgrNetShpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_shape_ctl));

    SetQMgrMiscShpCtl(V, donotRefillDma_f, &misc_shape_ctl, 0);
    SetQMgrMiscShpCtl(V, miscChanShpPtrMax_f, &misc_shape_ctl, 15);
    SetQMgrMiscShpCtl(V, miscChanShpPtrMin_f, &misc_shape_ctl, 0);
    SetQMgrMiscShpCtl(V, miscChanShpRefreshDis_f, &misc_shape_ctl, 0);
    SetQMgrMiscShpCtl(V, miscChanShpTickInterval_f, &misc_shape_ctl, 1);
    cmd = DRV_IOW(QMgrMiscShpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_shape_ctl));

    cmd = DRV_IOW(ErmAqmPortCtl_t, ErmAqmPortCtl_legacyModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    SetErmAqmPortScanCtl(V, aqmPortScanEn_f, &aqm_port_ctl, 1);
    SetErmAqmPortScanCtl(V, aqmPortScanInterval_f, &aqm_port_ctl, 150);
    SetErmAqmPortScanCtl(V, aqmPortScanPtrMax_f, &aqm_port_ctl, 64);
    SetErmAqmPortScanCtl(V, aqmPortScanPtrPhyMax_f, &aqm_port_ctl, 64);
    cmd = DRV_IOW(ErmAqmPortScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aqm_port_ctl));

    return CTC_E_NONE;
}

#define group_shp ""

STATIC uint32
_sys_usw_queue_shp_hash_make_group_shape_profile(sys_group_shp_profile_t* p_prof)
{
    uint32 val = 0;
    uint8* data =  (uint8*)p_prof;
    uint8   length = sizeof(sys_group_shp_profile_t) - sizeof(uint16);
    val = ctc_hash_caculate(length, (data + sizeof(uint16)));

    return val;
}

/**
 @brief Queue shape hash comparison hook.
*/
STATIC bool
_sys_usw_queue_shp_hash_cmp_group_shape_profile(sys_group_shp_profile_t* p_old_prof,
                                            sys_group_shp_profile_t* p_new_prof)
{
    if (!p_new_prof || !p_old_prof)
    {
        return FALSE;
    }

    if (p_new_prof->bucket_rate != p_old_prof->bucket_rate ||
        p_new_prof->bucket_thrd != p_old_prof->bucket_thrd)
    {
        return FALSE;
    }

    return TRUE;
}

int32
_sys_usw_group_shp_alloc_profileId(sys_group_shp_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_group_shape;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
    p_node->profile_id = value_32;

    return CTC_E_NONE;
}

int32
_sys_usw_group_shp_restore_profileId(sys_group_shp_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_group_shape;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_group_shp_free_profileId(sys_group_shp_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_group_shape;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_queue_shp_hash_make_group_meter_profile(sys_group_meter_profile_t* p_prof)
{
    uint32 val = 0;
    uint8* data =  (uint8*)p_prof;
    uint8   length = sizeof(sys_group_shp_profile_t) - sizeof(uint32);

    val = ctc_hash_caculate(length, (data + sizeof(uint32)));

    return val;
}

/**
 @brief Queue shape hash comparison hook.
*/
STATIC bool
_sys_usw_queue_shp_hash_cmp_group_meter_profile(sys_group_meter_profile_t* p_old_prof,
                                            sys_group_meter_profile_t* p_new_prof)
{
    if (!p_new_prof || !p_old_prof)
    {
        return FALSE;
    }

    if (sal_memcmp(p_new_prof->c_bucket, p_old_prof->c_bucket, sizeof(p_old_prof->c_bucket)))
    {
        return FALSE;
    }

    return TRUE;
}

int32
_sys_usw_group_meter_alloc_profileId(sys_group_meter_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_group_meter;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
    p_node->profile_id = value_32;

    return CTC_E_NONE;
}

int32
_sys_usw_group_meter_restore_profileId(sys_group_meter_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_group_meter;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_group_meter_free_profileId(sys_group_meter_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_group_meter;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_group_shape_profile_to_asic(uint8 lchip,
                                               sys_queue_group_node_t* p_sys_group_shp)
{
    uint32 cmd = 0;
    uint32 token_rate = 0;
    uint32 token_rate_frac = 0;
    uint32 token_threshold = 0;
    uint32 token_thrd_shift = 0;
    uint32 tbl_index = 0;
    uint32 value = 0;
    DsQMgrExtGrpShpProfile_m grp_shp_profile;
    sys_group_shp_profile_t* p_sys_profile = NULL;

    CTC_PTR_VALID_CHECK(p_sys_group_shp);

    p_sys_profile = p_sys_group_shp->p_shp_profile;

    tbl_index  = p_sys_profile->profile_id;
    sal_memset(&grp_shp_profile, 0, sizeof(DsQMgrExtGrpShpProfile_m));

    cmd = DRV_IOR(DsQMgrExtGrpShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &grp_shp_profile));

    token_rate = p_sys_profile->bucket_rate >> 8;
    token_rate_frac = p_sys_profile->bucket_rate & 0xFF;
    token_threshold = p_sys_profile->bucket_thrd >> 5;
    token_thrd_shift = p_sys_profile->bucket_thrd & 0x1F;

    SetDsQMgrExtGrpShpProfile(V, tokenRate_f,      &grp_shp_profile, token_rate);
    SetDsQMgrExtGrpShpProfile(V, tokenRateFrac_f,  &grp_shp_profile, token_rate_frac);
    SetDsQMgrExtGrpShpProfile(V, tokenThrd_f,      &grp_shp_profile, token_threshold);
    SetDsQMgrExtGrpShpProfile(V, tokenThrdShift_f, &grp_shp_profile, token_thrd_shift);

    cmd = DRV_IOW(DsQMgrExtGrpShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &grp_shp_profile));

    cmd = DRV_IOW(DsQMgrExtGrpShpToken_t, DsQMgrExtGrpShpToken_token_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_group_shp->group_id, cmd, &value));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add profile_id = %d\n", tbl_index);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_get_group_shape_profile_from_asic(uint8 lchip,
                                               sys_group_shp_profile_t* p_sys_profile)
{
    uint32 cmd = 0;
    uint32 token_rate;
    uint32 token_rate_frac;
    uint32 token_threshold;
    uint32 token_thrd_shift;
    uint32 tbl_index = 0;
    DsQMgrExtGrpShpProfile_m grp_shp_profile;

    CTC_PTR_VALID_CHECK(p_sys_profile);

    tbl_index = p_sys_profile->profile_id;
    sal_memset(&grp_shp_profile, 0, sizeof(DsQMgrExtGrpShpProfile_m));

    cmd = DRV_IOR(DsQMgrExtGrpShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &grp_shp_profile));

    token_rate = GetDsQMgrExtGrpShpProfile(V, tokenRate_f, &grp_shp_profile);
    token_rate_frac = GetDsQMgrExtGrpShpProfile(V, tokenRateFrac_f, &grp_shp_profile);
    token_threshold = GetDsQMgrExtGrpShpProfile(V, tokenThrd_f, &grp_shp_profile);
    token_thrd_shift = GetDsQMgrExtGrpShpProfile(V, tokenThrdShift_f, &grp_shp_profile);

    p_sys_profile->bucket_rate = (token_rate << 8) | (token_rate_frac & 0xFF);
    p_sys_profile->bucket_thrd = (token_threshold << 5) | (token_thrd_shift & 0x1F);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_group_meter_profile_to_asic(uint8 lchip,
                                                sys_queue_group_node_t* p_sys_group_shp)
{
    uint32 cmd = 0;
    uint32 token_rate;
    uint32 token_rate_frac;
    uint32 token_threshold;
    uint32 token_thrd_shift;
    uint32 tbl_index = 0;
    uint32 value = 0;
    uint16 step1 = 0;
    uint16 step2 = 0;
    uint8 loop = 0;
    DsQMgrExtGrpMeterProfile_m grp_meter_profile;
    sys_group_meter_profile_t* p_sys_meter_profile = NULL;
    CTC_PTR_VALID_CHECK(p_sys_group_shp);

    p_sys_meter_profile = p_sys_group_shp->p_meter_profile;
    tbl_index  = p_sys_meter_profile->profile_id;
    sal_memset(&grp_meter_profile, 0, sizeof(DsQMgrExtGrpMeterProfile_m));
    step1 = DsQMgrExtGrpMeterProfile_g_1_tokenRate_f - DsQMgrExtGrpMeterProfile_g_0_tokenRate_f;
    step2 = DsQMgrExtGrpShpToken_meter_1_token_f - DsQMgrExtGrpShpToken_meter_0_token_f;

    cmd = DRV_IOR(DsQMgrExtGrpMeterProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &grp_meter_profile));
    for(loop = 0; loop < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); loop++)
    {
        token_rate = p_sys_meter_profile->c_bucket[loop].rate >> 8;
        token_rate_frac = p_sys_meter_profile->c_bucket[loop].rate & 0xFF;
        token_threshold = p_sys_meter_profile->c_bucket[loop].thrd >> 5;
        token_thrd_shift = p_sys_meter_profile->c_bucket[loop].thrd & 0x1F;

        SetDsQMgrExtGrpMeterProfile(V, g_0_tokenRate_f + loop*step1,      &grp_meter_profile, token_rate);
        SetDsQMgrExtGrpMeterProfile(V, g_0_tokenRateFrac_f + loop*step1,  &grp_meter_profile, token_rate_frac);
        SetDsQMgrExtGrpMeterProfile(V, g_0_tokenThrd_f + loop*step1,      &grp_meter_profile, token_threshold);
        SetDsQMgrExtGrpMeterProfile(V, g_0_tokenThrdShift_f + loop*step1, &grp_meter_profile, token_thrd_shift);
        cmd = DRV_IOW(DsQMgrExtGrpShpToken_t, DsQMgrExtGrpShpToken_meter_0_token_f + loop*step2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_group_shp->group_id, cmd, &value));
    }

    cmd = DRV_IOW(DsQMgrExtGrpMeterProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &grp_meter_profile));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add profile_id = %d\n", tbl_index);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_get_group_meter_profile_from_asic(uint8 lchip,
                                               sys_group_meter_profile_t* p_sys_meter_profile)
{
    uint32 cmd = 0;
    uint32 token_rate;
    uint32 token_rate_frac;
    uint32 token_threshold;
    uint32 token_thrd_shift;
    uint32 tbl_index = 0;
    uint16 step = 0;
    uint8 loop = 0;
    DsQMgrExtGrpMeterProfile_m grp_meter_profile;

    CTC_PTR_VALID_CHECK(p_sys_meter_profile);

    tbl_index  = p_sys_meter_profile->profile_id;
    sal_memset(&grp_meter_profile, 0, sizeof(DsQMgrExtGrpMeterProfile_m));
    step = DsQMgrExtGrpMeterProfile_g_1_tokenRate_f - DsQMgrExtGrpMeterProfile_g_0_tokenRate_f;
    cmd = DRV_IOR(DsQMgrExtGrpMeterProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &grp_meter_profile));
    for(loop = 0; loop < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); loop++)
    {
        token_rate = GetDsQMgrExtGrpMeterProfile(V, g_0_tokenRate_f + loop*step, &grp_meter_profile);
        token_rate_frac = GetDsQMgrExtGrpMeterProfile(V, g_0_tokenRateFrac_f + loop*step, &grp_meter_profile);
        token_threshold = GetDsQMgrExtGrpMeterProfile(V, g_0_tokenThrd_f + loop*step, &grp_meter_profile);
        token_thrd_shift = GetDsQMgrExtGrpMeterProfile(V, g_0_tokenThrdShift_f + loop*step, &grp_meter_profile);

        p_sys_meter_profile->c_bucket[loop].rate = (token_rate << 8) | (token_rate_frac & 0xFF);
        p_sys_meter_profile->c_bucket[loop].thrd = (token_threshold << 5) | (token_thrd_shift & 0x1F);
        if(p_sys_meter_profile->c_bucket[loop].rate != 0)
        {
             p_sys_meter_profile->c_bucket[loop].cir_type = SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_group_shape_profile_from_asic(uint8 lchip, uint8 slice_id,
                                                    sys_group_shp_profile_t* p_sys_profile)
{
    /*do nothing*/
    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_group_shape_to_asic(uint8 lchip,
                                       uint8 is_pir, uint16 group_id,
                                       sys_queue_group_node_t* p_sys_group)
{
    uint32 fld_value = 0;
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_sys_group);

    fld_value = 0;
    if (is_pir)
    {
        CTC_PTR_VALID_CHECK(p_sys_group->p_shp_profile);
        fld_value = p_sys_group->p_shp_profile->profile_id;
        cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &fld_value));
    }
    else
    {
        CTC_PTR_VALID_CHECK(p_sys_group->p_meter_profile);
        fld_value = p_sys_group->p_meter_profile->profile_id;
        cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profIdMeter_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &fld_value));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_group_shape_from_asic(uint8 lchip, uint8 is_pir, uint16 group_id,
                                            sys_queue_group_node_t* p_sys_group)
{
    uint32 fld_value = 0;
    uint32 cmd = 0;

    fld_value = 0;
    if (is_pir)
    {
        cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &fld_value));
    }
    else
    {
        cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profIdMeter_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &fld_value));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_group_shape_profile(uint8 lchip, uint8 is_pir,
                                          sys_group_shp_profile_t* p_sys_profile_old, sys_group_meter_profile_t* p_meter_profile_old)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (is_pir)
    {
        p_profile_pool = p_usw_queue_master[lchip]->p_group_profile_pool;
        CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profile_old, NULL));
    }
    else
    {
        p_profile_pool = p_usw_queue_master[lchip]->p_group_profile_meter_pool;
        CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_meter_profile_old, NULL));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_shp_group_profile_add_spool(uint8 lchip, sys_group_shp_profile_t* p_sys_profile_old,
                                                          sys_group_shp_profile_t* p_sys_profile_new,
                                                          sys_group_shp_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_queue_shp_hash_cmp_group_shape_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_queue_master[lchip]->p_group_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

STATIC int32
 _sys_usw_queue_shp_group_meter_profile_add_spool(uint8 lchip, sys_group_meter_profile_t* p_sys_profile_old,
                                                          sys_group_meter_profile_t* p_sys_profile_new,
                                                          sys_group_meter_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_queue_shp_hash_cmp_group_meter_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_queue_master[lchip]->p_group_profile_meter_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_group_shape_profile(uint8 lchip, uint8 is_pir, sys_queue_group_node_t* p_sys_group_shp,
                                                 sys_group_shp_profile_t* p_shp_profile, sys_group_meter_profile_t* p_meter_profile)
{
    sys_group_shp_profile_t* p_sys_profile_old = NULL;
    sys_group_shp_profile_t* p_sys_profile_get = NULL;
    sys_group_meter_profile_t* p_sys_meter_profile_old = NULL;
    sys_group_meter_profile_t* p_sys_meter_profile_get = NULL;

    p_sys_profile_old = p_sys_group_shp->p_shp_profile;
    p_sys_meter_profile_old = p_sys_group_shp->p_meter_profile;

    if (is_pir)
    {
        CTC_ERROR_RETURN(_sys_usw_queue_shp_group_profile_add_spool(lchip, p_sys_profile_old, p_shp_profile, &p_sys_profile_get));
        p_sys_group_shp->p_shp_profile = p_sys_profile_get;

        CTC_ERROR_RETURN(_sys_usw_queue_shp_add_group_shape_profile_to_asic(lchip, p_sys_group_shp));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_queue_shp_group_meter_profile_add_spool(lchip, p_sys_meter_profile_old, p_meter_profile, &p_sys_meter_profile_get));
        p_sys_group_shp->p_meter_profile = p_sys_meter_profile_get;

        CTC_ERROR_RETURN(_sys_usw_queue_shp_add_group_meter_profile_to_asic(lchip, p_sys_group_shp));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_shp_save_group_shape_bucket(uint8 lchip, uint8 enable,
                                            uint8  is_pir,
                                            uint8  queue_offset,
                                            uint32 bucket_rate,
                                            uint32 bucket_thrd,
                                            uint8 new_cir_type,
                                            uint8 group_mode,
                                            sys_queue_group_node_t* p_sys_group_node,
                                            sys_group_shp_profile_t* p_shp_profile,
                                            sys_group_meter_profile_t* p_meter_profile)
{
    uint8 offset    = 0;
    uint8 queue_num = 0;

    /*Profile operation*/

    if (is_pir)
    {
        p_shp_profile->bucket_rate = bucket_rate;
        p_shp_profile->bucket_thrd = bucket_thrd;

        queue_offset = MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET);
    }
    else
    {
        enable = enable && bucket_rate;
        queue_num = MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        if (p_sys_group_node->p_meter_profile)
        {
            for (offset = 0; offset < queue_num; offset++)
            {
                if(p_sys_group_node->p_meter_profile->c_bucket[offset].cir_type == SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER)
                {
                    p_meter_profile->c_bucket[offset].cir_type = SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER;
                    p_meter_profile->c_bucket[offset].rate = p_sys_group_node->p_meter_profile->c_bucket[offset].rate;
                    p_meter_profile->c_bucket[offset].thrd = p_sys_group_node->p_meter_profile->c_bucket[offset].thrd;
                }
            }
        }

        p_meter_profile->c_bucket[queue_offset].cir_type = new_cir_type;
        p_meter_profile->c_bucket[queue_offset].rate = bucket_rate;
        p_meter_profile->c_bucket[queue_offset].thrd = bucket_thrd;

        if (CTC_IS_BIT_SET(p_sys_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET))
            && p_sys_group_node->p_shp_profile)
        {
            p_shp_profile->bucket_rate = p_sys_group_node->p_shp_profile->bucket_rate;
            p_shp_profile->bucket_thrd = p_sys_group_node->p_shp_profile->bucket_thrd;
        }
    }

    if (enable)
    {
        CTC_BIT_SET(p_sys_group_node->shp_bitmap, queue_offset);
    }
    else
    {
        CTC_BIT_UNSET(p_sys_group_node->shp_bitmap, queue_offset);
    }

    if (!CTC_IS_BIT_SET(p_sys_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET)))
    {
        p_shp_profile->bucket_rate = (MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE) << 8 | MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
        p_shp_profile->bucket_thrd = (MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD) << 5| MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_shp_update_group_shape(uint8 lchip, uint16 queue_id,
                                       uint32 cir,
                                       uint32 cbs,
                                       uint8  queue_offset,
                                       uint8  is_pir,
                                       uint8  enable)
{
    uint32 token_thrd = 0;
    uint32 gran = 0;
    uint8  new_cir_type =0;
    uint8 is_pps = 0;
    uint8 mode = 0;
    uint16 group_id = 0;
    uint16 value = 0;
    sys_queue_group_node_t* p_sys_group_node = NULL;
    sys_group_shp_profile_t  shp_profile;
    sys_group_meter_profile_t meter_profile;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&shp_profile, 0, sizeof(sys_group_shp_profile_t));
    sal_memset(&meter_profile, 0, sizeof(sys_group_meter_profile_t));

    SYS_EXT_GROUP_ID(queue_id, group_id);

    if ((cir != 0) && (queue_offset > MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)))
    {
        return CTC_E_INVALID_PARAM;
    }

    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    if (!enable && (is_pir ? (NULL == p_sys_group_node->p_shp_profile) : (NULL == p_sys_group_node->p_meter_profile)))
    {
        return CTC_E_NONE;
    }

    /*config CIR Bucket :CIR = 0->CIR Fail;cir =CTC_MAX_UINT32_VALUE -> CIR pass */
    if (!is_pir)
    {
        if (cir == 0 || !enable)
        {
            new_cir_type = SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_FAIL;
            cir = 0;
        }
        else if (enable)
        {
            new_cir_type = SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER;
        }
    }

    if ( new_cir_type == SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER || is_pir)
    {
        is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, p_sys_group_node->chan_id);

        CTC_ERROR_RETURN(sys_usw_qos_get_shp_granularity(lchip, cir,SYS_SHAPE_TYPE_QUEUE, &gran));

        /* convert to internal token rate */
        CTC_ERROR_RETURN(sys_usw_qos_map_token_rate_user_to_hw(lchip, is_pps, cir,
                                                                      &cir,
                                                                      MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                                      gran,
                                                                      p_usw_queue_master[lchip]->que_shp_update_freq,
                                                                      MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
        SYS_QOS_COMPUTE_SHAPE_TOKEN_THRD(cbs, cir, 0,is_pps);

        /* convert to internal token thrd */
        CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, cbs,
                                                                      &token_thrd,
                                                                      MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH),
                                                                      SYS_SHAPE_MAX_TOKEN_THRD));

        cbs = token_thrd;
    }

    CTC_ERROR_RETURN(sys_usw_queue_shp_save_group_shape_bucket(lchip, enable, is_pir, queue_offset,
                                    cir, cbs, new_cir_type, mode, p_sys_group_node, &shp_profile, &meter_profile));
    value = DRV_IS_DUET2(lchip) ? 0xf : 0xff;
    /*add new prof*/
    if((is_pir && CTC_IS_BIT_SET(p_sys_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET)))||
        (is_pir == 0 && ((p_sys_group_node->shp_bitmap & value)!= 0)))
    {
        CTC_ERROR_RETURN(_sys_usw_queue_shp_add_group_shape_profile(lchip, is_pir, p_sys_group_node,
                                                                      &shp_profile, &meter_profile));

        CTC_ERROR_RETURN(_sys_usw_queue_shp_add_group_shape_to_asic(lchip, is_pir, group_id,
                                                                 p_sys_group_node));
    }
    else if (is_pir ? (NULL !=p_sys_group_node->p_shp_profile) : (NULL !=p_sys_group_node->p_meter_profile))
    {
        /*disable Group Shape*/
        CTC_ERROR_RETURN(_sys_usw_queue_shp_remove_group_shape_from_asic(lchip, is_pir, group_id,
                                                                      p_sys_group_node));
        /*remove prof*/
        CTC_ERROR_RETURN(_sys_usw_queue_shp_remove_group_shape_profile(lchip, is_pir,
                                                                    p_sys_group_node->p_shp_profile, p_sys_group_node->p_meter_profile));
        if (is_pir)
        {
            p_sys_group_node->p_shp_profile = 0;
        }
        else
        {
            p_sys_group_node->p_meter_profile = 0;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_set_group_shape(uint8 lchip, ctc_qos_shape_group_t* p_shape)
{
    uint16 queue_id = 0;
    uint8 is_pps = 0;
    uint16 ext_grp_id = 0;
    sys_queue_group_node_t* p_sys_group_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->enable = %d\n", p_shape->enable);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->pir = %d\n", p_shape->pir);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_shape->queue,
                                                           &queue_id));

    if (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_EXT_GROUP_ID(queue_id, ext_grp_id);
    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, ext_grp_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, p_sys_group_node->chan_id);
    if (is_pps)
    {
        CTC_MAX_VALUE_CHECK(p_shape->pir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE_PPS));
        p_shape->pbs = MIN(p_shape->pbs,(SYS_QOS_SHAPE_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_shape->pir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE));
        p_shape->pbs = MIN(p_shape->pbs,(SYS_QOS_SHAPE_MAX_COUNT / 125));
    }

    CTC_ERROR_RETURN(sys_usw_queue_shp_update_group_shape(lchip,
                                           queue_id,
                                           p_shape->pir,
                                           p_shape->pbs,
                                           0,
                                           1,
                                           p_shape->enable));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_get_group_shape(uint8 lchip, ctc_qos_shape_group_t* p_shape)
{
    uint32 token_thrd = 0;
    uint32 token_rate = 0;
    uint16 queue_id = 0;
    uint8 is_pps = 0;
    uint16 ext_grp_id = 0;
    sys_queue_group_node_t  * p_group_node = NULL;

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_shape->queue,
                                                           &queue_id));

    if (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_EXT_GROUP_ID(queue_id, ext_grp_id);
    p_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, ext_grp_id);
    if (NULL == p_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    p_shape->pir = SYS_SHAPE_DEFAULT_RATE;
    p_shape->pbs = SYS_SHAPE_DEFAULT_THRD;
    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, p_group_node->chan_id);

    if (p_group_node->shp_bitmap && p_group_node->p_shp_profile)
    {
        sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_group_node->p_shp_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                              p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
        _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd, p_group_node->p_shp_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
        SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
        p_shape->pir = token_rate;
        p_shape->pbs = token_thrd;
    }
    p_shape->enable = CTC_IS_BIT_SET(p_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET));

    return CTC_E_NONE;
}
int32
_sys_usw_queue_shp_init_group_shape(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;

    DsQMgrExtGrpShpProfile_m ext_grp_shp_profile;
    QMgrExtGrpShpCtl_m ext_grp_shp_ctl;

    sal_memset(&ext_grp_shp_profile, 0, sizeof(DsQMgrExtGrpShpProfile_m));
    sal_memset(&ext_grp_shp_ctl, 0, sizeof(QMgrExtGrpShpCtl_m));

    /* reserved queue shape profile 0 for max*/
    SetDsQMgrExtGrpShpProfile(V, tokenRateFrac_f, &ext_grp_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
    SetDsQMgrExtGrpShpProfile(V, tokenRate_f, &ext_grp_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
    SetDsQMgrExtGrpShpProfile(V, tokenThrdShift_f, &ext_grp_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    SetDsQMgrExtGrpShpProfile(V, tokenThrd_f, &ext_grp_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
    cmd = DRV_IOW(DsQMgrExtGrpShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ext_grp_shp_profile));

    SetQMgrExtGrpShpCtl(V, extGrpShpPtrMax_f, &ext_grp_shp_ctl, MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM)-1);
    SetQMgrExtGrpShpCtl(V, extGrpShpPtrMin_f, &ext_grp_shp_ctl, 0);
    SetQMgrExtGrpShpCtl(V, extGrpShpRefreshDis_f, &ext_grp_shp_ctl, 0);
    SetQMgrExtGrpShpCtl(V, extGrpShpTickInterval_f, &ext_grp_shp_ctl, 1);
    cmd = DRV_IOW(QMgrExtGrpShpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ext_grp_shp_ctl));

    return CTC_E_NONE;
}

#define queue_shp ""
STATIC uint32
_sys_usw_queue_shp_hash_make_queue_shape_profile(sys_queue_shp_profile_t* p_prof)
{
    uint32 val = 0;
    uint8* data = (uint8*)p_prof;
    uint8 length = (sizeof(sys_queue_shp_profile_t) - sizeof(uint8));

    val = ctc_hash_caculate(length, data + sizeof(uint8));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "hash val = %d\n", val);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof->bucket_rate:%d\n", p_prof->bucket_rate);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof->bucket_thrd:%d\n", p_prof->bucket_thrd);

    return val;
}

/**
 @brief Queue shape hash comparison hook.
*/
STATIC bool
_sys_usw_queue_shp_hash_cmp_queue_shape_profile(sys_queue_shp_profile_t* p_prof1,
                                            sys_queue_shp_profile_t* p_prof2)
{
    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof1->bucket_rate:%d\n", p_prof1->bucket_rate);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof2->bucket_rate:%d\n", p_prof2->bucket_rate);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof1->bucket_thrd:%d\n", p_prof1->bucket_thrd);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof2->bucket_thrd:%d\n", p_prof2->bucket_thrd);

    if (p_prof1->bucket_rate == p_prof2->bucket_rate
        && p_prof1->bucket_thrd == p_prof2->bucket_thrd
        && p_prof1->queue_type == p_prof2->queue_type)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Found!!!\n");
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_queue_shp_alloc_profileId(sys_queue_shp_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_shape;
    switch(p_node->queue_type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        opf.pool_index = 0;
        break;
    case SYS_QUEUE_TYPE_EXCP:
        opf.pool_index = 1;
        break;
    case SYS_QUEUE_TYPE_EXTEND:
        opf.pool_index = 2;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
    p_node->profile_id = value_32;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_restore_profileId(sys_queue_shp_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_shape;
    switch(p_node->queue_type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        opf.pool_index = 0;
        break;
    case SYS_QUEUE_TYPE_EXCP:
        opf.pool_index = 1;
        break;
    case SYS_QUEUE_TYPE_EXTEND:
        opf.pool_index = 2;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_shp_free_profileId(sys_queue_shp_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_shape;
    switch(p_node->queue_type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        opf.pool_index = 0;
        break;
    case SYS_QUEUE_TYPE_EXCP:
        opf.pool_index = 1;
        break;
    case SYS_QUEUE_TYPE_EXTEND:
        opf.pool_index = 2;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_queue_shp_hash_make_queue_meter_profile(sys_queue_meter_profile_t* p_prof)
{
    uint32 val = 0;
    uint8* data = (uint8*)p_prof;
    uint8 length = (sizeof(sys_queue_meter_profile_t) - sizeof(uint8));

    val = ctc_hash_caculate(length, data + sizeof(uint8));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "hash val = %d\n", val);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof->bucket_rate:%d\n", p_prof->bucket_rate);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof->bucket_thrd:%d\n", p_prof->bucket_thrd);

    return val;
}

/**
 @brief Queue shape hash comparison hook.
*/
STATIC bool
_sys_usw_queue_shp_hash_cmp_queue_meter_profile(sys_queue_meter_profile_t* p_prof1,
                                            sys_queue_meter_profile_t* p_prof2)
{
    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof1->bucket_rate:%d\n", p_prof1->bucket_rate);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof2->bucket_rate:%d\n", p_prof2->bucket_rate);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof1->bucket_thrd:%d\n", p_prof1->bucket_thrd);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_prof2->bucket_thrd:%d\n", p_prof2->bucket_thrd);

    if (p_prof1->bucket_rate == p_prof2->bucket_rate
        && p_prof1->bucket_thrd == p_prof2->bucket_thrd)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Found!!!\n");
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_queue_meter_alloc_profileId(sys_queue_meter_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_meter;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
    p_node->profile_id = value_32;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_meter_restore_profileId(sys_queue_meter_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_meter;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_meter_free_profileId(sys_queue_meter_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_meter;
    opf.pool_index = 0;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_queue_meter_profile_to_asic(uint8 lchip,
                                               sys_queue_node_t* p_sys_queue)
{
    DsQMgrNetBaseQueMeterProfile_m net_que_meter_profile;
    uint32 cmd = 0;
    uint32 table_index = 0;
    uint32 value = 0;
    sys_queue_meter_profile_t* p_sys_profile = NULL;

    CTC_PTR_VALID_CHECK(p_sys_queue);
    p_sys_profile = p_sys_queue->p_meter_profile;
    CTC_MAX_VALUE_CHECK(p_sys_profile->bucket_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&net_que_meter_profile, 0 , sizeof(DsQMgrNetBaseQueMeterProfile_m));

    table_index = p_sys_profile->profile_id;

    SetDsQMgrNetBaseQueMeterProfile(V, tokenRateFrac_f, &net_que_meter_profile, p_sys_profile->bucket_rate & 0xFF);
    SetDsQMgrNetBaseQueMeterProfile(V, tokenRate_f, &net_que_meter_profile, p_sys_profile->bucket_rate >> 8);
    SetDsQMgrNetBaseQueMeterProfile(V, tokenThrdShift_f, &net_que_meter_profile, p_sys_profile->bucket_thrd & 0x1F);   /* low 5bit is shift */
    SetDsQMgrNetBaseQueMeterProfile(V, tokenThrd_f, &net_que_meter_profile, p_sys_profile->bucket_thrd >> 5);

    cmd = DRV_IOW(DsQMgrNetBaseQueMeterProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &net_que_meter_profile));

    cmd = DRV_IOW(DsQMgrNetBaseQueMeterToken_t, DsQMgrNetBaseQueMeterToken_token_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_queue->queue_id, cmd, &value));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_get_queue_meter_profile_from_asic(uint8 lchip,
                                               sys_queue_meter_profile_t* p_sys_profile)
{
    DsQMgrNetBaseQueMeterProfile_m net_que_meter_profile;
    uint32 cmd = 0;
    uint32 table_index = 0;
    uint32 value;
    uint32  bucket_rate = 0;
    uint16  bucket_thrd = 0;

    CTC_PTR_VALID_CHECK(p_sys_profile);
    CTC_MAX_VALUE_CHECK(p_sys_profile->bucket_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    table_index = p_sys_profile->profile_id;

    sal_memset(&net_que_meter_profile, 0 , sizeof(DsQMgrNetBaseQueMeterProfile_m));
    cmd = DRV_IOR(DsQMgrNetBaseQueMeterProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &net_que_meter_profile));

    value = GetDsQMgrNetBaseQueMeterProfile(V, tokenRateFrac_f, &net_que_meter_profile);
    bucket_rate |= value & 0xFF;
    value = GetDsQMgrNetBaseQueMeterProfile(V, tokenRate_f, &net_que_meter_profile);
    bucket_rate |= (value << 8);
    value = GetDsQMgrNetBaseQueMeterProfile(V, tokenThrdShift_f, &net_que_meter_profile);   /* low 5bit is shift */
    bucket_thrd |= value & 0x1F;
    value = GetDsQMgrNetBaseQueMeterProfile(V, tokenThrd_f, &net_que_meter_profile);
    bucket_thrd |= (value << 5);

    p_sys_profile->bucket_rate = bucket_rate;
    p_sys_profile->bucket_thrd = bucket_thrd;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_queue_shape_profile_to_asic(uint8 lchip,
                                               sys_queue_node_t* p_sys_queue)
{
    DsQMgrNetQueShpProfile_m net_que_shp_profile;
    DsQMgrDmaQueShpProfile_m dma_que_shp_profile;
    DsQMgrExtQueShpProfile_m ext_que_shp_profile;
    uint32 cmd = 0;
    uint32 table_index = 0;
    uint32 value = 0;
    sys_queue_shp_profile_t* p_sys_profile = NULL;

    CTC_PTR_VALID_CHECK(p_sys_queue);
    p_sys_profile = p_sys_queue->p_shp_profile;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    table_index = p_sys_profile->profile_id;

    switch(p_sys_profile->queue_type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        sal_memset(&net_que_shp_profile, 0, sizeof(DsQMgrNetQueShpProfile_m));

        SetDsQMgrNetQueShpProfile(V, tokenRateFrac_f, &net_que_shp_profile, p_sys_profile->bucket_rate & 0xFF);
        SetDsQMgrNetQueShpProfile(V, tokenRate_f, &net_que_shp_profile, p_sys_profile->bucket_rate >> 8);
        SetDsQMgrNetQueShpProfile(V, tokenThrdShift_f, &net_que_shp_profile, p_sys_profile->bucket_thrd & 0x1F);   /* low 5bit is shift */
        SetDsQMgrNetQueShpProfile(V, tokenThrd_f, &net_que_shp_profile, p_sys_profile->bucket_thrd >> 5);

        cmd = DRV_IOW(DsQMgrNetQueShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &net_que_shp_profile));

        table_index = (p_sys_queue->queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)) ? p_sys_queue->queue_id
                       : ((p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)) + MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC));

        cmd = DRV_IOW(DsQMgrNetQueShpToken_t, DsQMgrNetQueShpToken_token_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
        break;

    case SYS_QUEUE_TYPE_EXCP:
        sal_memset(&dma_que_shp_profile, 0, sizeof(DsQMgrDmaQueShpProfile_m));

        SetDsQMgrDmaQueShpProfile(V, tokenRateFrac_f, &dma_que_shp_profile, p_sys_profile->bucket_rate & 0xFF);
        SetDsQMgrDmaQueShpProfile(V, tokenRate_f, &dma_que_shp_profile, p_sys_profile->bucket_rate >> 8);
        SetDsQMgrDmaQueShpProfile(V, tokenThrdShift_f, &dma_que_shp_profile, p_sys_profile->bucket_thrd & 0x1F);   /* low 5bit is shift */
        SetDsQMgrDmaQueShpProfile(V, tokenThrd_f, &dma_que_shp_profile, p_sys_profile->bucket_thrd >> 5);

        cmd = DRV_IOW(DsQMgrDmaQueShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &dma_que_shp_profile));

        table_index = p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);
        cmd = DRV_IOW( DsQMgrDmaQueShpToken_t,  DsQMgrDmaQueShpToken_token_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
        break;

    case SYS_QUEUE_TYPE_EXTEND:
        sal_memset(&ext_que_shp_profile, 0, sizeof(DsQMgrExtQueShpProfile_m));

        SetDsQMgrExtQueShpProfile(V, tokenRateFrac_f, &ext_que_shp_profile, p_sys_profile->bucket_rate & 0xFF);
        SetDsQMgrExtQueShpProfile(V, tokenRate_f, &ext_que_shp_profile, p_sys_profile->bucket_rate >> 8);
        SetDsQMgrExtQueShpProfile(V, tokenThrdShift_f, &ext_que_shp_profile, p_sys_profile->bucket_thrd & 0x1F);   /* low 5bit is shift */
        SetDsQMgrExtQueShpProfile(V, tokenThrd_f, &ext_que_shp_profile, p_sys_profile->bucket_thrd >> 5);

        cmd = DRV_IOW(DsQMgrExtQueShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ext_que_shp_profile));

        table_index = p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT);
        cmd = DRV_IOW( DsQMgrExtQueShpToken_t,  DsQMgrExtQueShpToken_token_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &value));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_get_queue_shape_profile_from_asic(uint8 lchip,
                                               sys_queue_shp_profile_t* p_sys_profile)
{
    DsQMgrNetQueShpProfile_m net_que_shp_profile;
    DsQMgrDmaQueShpProfile_m dma_que_shp_profile;
    DsQMgrExtQueShpProfile_m ext_que_shp_profile;
    uint32 cmd = 0;
    uint32 table_index = 0;
    uint32 value;
    uint32  bucket_rate = 0;
    uint16  bucket_thrd = 0;

    CTC_PTR_VALID_CHECK(p_sys_profile);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    table_index = p_sys_profile->profile_id;

    switch(p_sys_profile->queue_type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        sal_memset(&net_que_shp_profile, 0 , sizeof(DsQMgrNetQueShpProfile_m));
        cmd = DRV_IOR(DsQMgrNetQueShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &net_que_shp_profile));

        value = GetDsQMgrNetQueShpProfile(V, tokenRateFrac_f, &net_que_shp_profile);
        bucket_rate |= value & 0xFF;
        value = GetDsQMgrNetQueShpProfile(V, tokenRate_f, &net_que_shp_profile);
        bucket_rate |= (value << 8);
        value = GetDsQMgrNetQueShpProfile(V, tokenThrdShift_f, &net_que_shp_profile);   /* low 5bit is shift */
        bucket_thrd |= value & 0x1F;
        value = GetDsQMgrNetQueShpProfile(V, tokenThrd_f, &net_que_shp_profile);
        bucket_thrd |= (value << 5);

        p_sys_profile->bucket_rate = bucket_rate;
        p_sys_profile->bucket_thrd = bucket_thrd;
        break;

    case SYS_QUEUE_TYPE_EXCP:
        sal_memset(&dma_que_shp_profile, 0 , sizeof(DsQMgrDmaQueShpProfile_m));
        cmd = DRV_IOR(DsQMgrDmaQueShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &dma_que_shp_profile));

        value = GetDsQMgrDmaQueShpProfile(V, tokenRateFrac_f, &dma_que_shp_profile);
        bucket_rate |= value & 0xFF;
        value = GetDsQMgrDmaQueShpProfile(V, tokenRate_f, &dma_que_shp_profile);
        bucket_rate |= (value << 8);
        value = GetDsQMgrDmaQueShpProfile(V, tokenThrdShift_f, &dma_que_shp_profile);   /* low 5bit is shift */
        bucket_thrd |= value & 0x1F;
        value = GetDsQMgrDmaQueShpProfile(V, tokenThrd_f, &dma_que_shp_profile);
        bucket_thrd |= (value << 5);

        p_sys_profile->bucket_rate = bucket_rate;
        p_sys_profile->bucket_thrd = bucket_thrd;
        break;

    case SYS_QUEUE_TYPE_EXTEND:
        sal_memset(&ext_que_shp_profile, 0 , sizeof(DsQMgrExtQueShpProfile_m));
        cmd = DRV_IOR(DsQMgrExtQueShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &ext_que_shp_profile));

        value = GetDsQMgrExtQueShpProfile(V, tokenRateFrac_f, &ext_que_shp_profile);
        bucket_rate |= value & 0xFF;
        value = GetDsQMgrExtQueShpProfile(V, tokenRate_f, &ext_que_shp_profile);
        bucket_rate |= (value << 8);
        value = GetDsQMgrExtQueShpProfile(V, tokenThrdShift_f, &ext_que_shp_profile);   /* low 5bit is shift */
        bucket_thrd |= value & 0x1F;
        value = GetDsQMgrExtQueShpProfile(V, tokenThrd_f, &ext_que_shp_profile);
        bucket_thrd |= (value << 5);

        p_sys_profile->bucket_rate = bucket_rate;
        p_sys_profile->bucket_thrd = bucket_thrd;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_queue_shape_profile_from_asic(uint8 lchip, uint8 slice_id,
                                                    sys_queue_shp_profile_t* p_sys_profile)
{
    /*do nothing*/
    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_queue_meter_to_asic(uint8 lchip,
                                       sys_queue_node_t* p_sys_queue)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 table_index = 0;

    CTC_PTR_VALID_CHECK(p_sys_queue);
    CTC_PTR_VALID_CHECK(p_sys_queue->p_meter_profile);

    field_val = p_sys_queue->p_meter_profile->profile_id;
    if (SYS_IS_NETWORK_BASE_QUEUE(p_sys_queue->queue_id))
    {
        table_index = p_sys_queue->queue_id;
        cmd = DRV_IOW(DsQMgrNetBaseQueMeterProfId_t, DsQMgrNetBaseQueMeterProfId_profId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id = %d\n", p_sys_queue->queue_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "table_index = %d\n", table_index);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_queue_shape_to_asic(uint8 lchip,
                                       sys_queue_node_t* p_sys_queue)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 table_index = 0;

    CTC_PTR_VALID_CHECK(p_sys_queue);
    CTC_PTR_VALID_CHECK(p_sys_queue->p_shp_profile);

    field_val = p_sys_queue->p_shp_profile->profile_id;
    switch(p_sys_queue->p_shp_profile->queue_type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        if (SYS_IS_NETWORK_BASE_QUEUE(p_sys_queue->queue_id) || SYS_IS_NETWORK_CTL_QUEUE(p_sys_queue->queue_id))
        {
            table_index = (p_sys_queue->queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)) ? p_sys_queue->queue_id
                          : ((p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)) + MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC));
            cmd = DRV_IOW(DsQMgrNetQueShpProfId_t, DsQMgrNetQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        }
        break;

    case SYS_QUEUE_TYPE_EXCP:
        if (SYS_IS_CPU_QUEUE(p_sys_queue->queue_id))
        {
            table_index = p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);
            cmd = DRV_IOW(DsQMgrDmaQueShpProfId_t, DsQMgrDmaQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        }
        break;

    case SYS_QUEUE_TYPE_EXTEND:
        if (SYS_IS_EXT_QUEUE(p_sys_queue->queue_id))
        {
            table_index = p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT);
            cmd = DRV_IOW(DsQMgrExtQueShpProfId_t, DsQMgrExtQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id = %d\n", p_sys_queue->queue_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "table_index = %d\n", table_index);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_queue_meter_from_asic(uint8 lchip,
                                            sys_queue_node_t* p_sys_queue)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 table_index = 0;

    CTC_PTR_VALID_CHECK(p_sys_queue);

    if (SYS_IS_NETWORK_BASE_QUEUE(p_sys_queue->queue_id))
    {
        table_index = p_sys_queue->queue_id;
        cmd = DRV_IOW(DsQMgrNetBaseQueMeterProfId_t, DsQMgrNetBaseQueMeterProfId_profId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id = %d\n", p_sys_queue->queue_id);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_queue_meter_profile(uint8 lchip,
                                          sys_queue_meter_profile_t* p_sys_profile_old)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_old);

    p_profile_pool = p_usw_queue_master[lchip]->p_queue_meter_profile_pool;

    CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profile_old, NULL));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_queue_shape_from_asic(uint8 lchip,
                                            sys_queue_node_t* p_sys_queue)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 table_index = 0;

    switch(p_sys_queue->type)
    {
    case SYS_QUEUE_TYPE_NORMAL:
        if (SYS_IS_NETWORK_BASE_QUEUE(p_sys_queue->queue_id) || SYS_IS_NETWORK_CTL_QUEUE(p_sys_queue->queue_id))
        {
            table_index = (p_sys_queue->queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC))
            ? p_sys_queue->queue_id
            : ((p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)) + MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC));
            cmd = DRV_IOW(DsQMgrNetQueShpProfId_t, DsQMgrNetQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        }
        break;

    case SYS_QUEUE_TYPE_EXCP:
        if (SYS_IS_CPU_QUEUE(p_sys_queue->queue_id))
        {
            table_index = p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);
            cmd = DRV_IOW(DsQMgrDmaQueShpProfId_t, DsQMgrDmaQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        }
        break;

    case SYS_QUEUE_TYPE_EXTEND:
        if (SYS_IS_EXT_QUEUE(p_sys_queue->queue_id))
        {
            table_index = p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT);
            cmd = DRV_IOW(DsQMgrExtQueShpProfId_t, DsQMgrExtQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id = %d\n", p_sys_queue->queue_id);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_remove_queue_shape_profile(uint8 lchip,
                                          sys_queue_shp_profile_t* p_sys_profile_old)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_old);

    p_profile_pool = p_usw_queue_master[lchip]->p_queue_profile_pool;

    CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_profile_old, NULL));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_meter_profile_add_spool(uint8 lchip, sys_queue_meter_profile_t* p_sys_profile_old,
                                                          sys_queue_meter_profile_t* p_sys_profile_new,
                                                          sys_queue_meter_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_queue_shp_hash_cmp_queue_meter_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_queue_master[lchip]->p_queue_meter_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_shp_profile_add_spool(uint8 lchip, sys_queue_shp_profile_t* p_sys_profile_old,
                                                          sys_queue_shp_profile_t* p_sys_profile_new,
                                                          sys_queue_shp_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_queue_shp_hash_cmp_queue_shape_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_queue_master[lchip]->p_queue_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_shp_add_queue_meter_profile(uint8 lchip, ctc_qos_shape_queue_t* p_shp_param,
                                       sys_queue_node_t* p_sys_queue)
{
    sys_queue_meter_profile_t* p_sys_profile_old = NULL;
    sys_queue_meter_profile_t sys_profile_new;
    sys_queue_meter_profile_t* p_sys_profile_get = NULL;
    int32 ret = 0;
    uint32 gran = 0;
    uint32 bucket_rate = 0;
    uint32 bucket_thrd = 0;
    uint8  is_pps = 0;
    uint8  channel = 0;

    CTC_PTR_VALID_CHECK(p_shp_param);
    CTC_PTR_VALID_CHECK(p_sys_queue);

    sal_memset(&sys_profile_new, 0, sizeof(sys_queue_shp_profile_t));

    CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip, p_sys_queue->queue_id, &channel));
    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);

    CTC_ERROR_RETURN(sys_usw_qos_get_shp_granularity(lchip, p_shp_param->cir,
                                                            SYS_SHAPE_TYPE_QUEUE, &gran));
    /* convert to internal token rate */

    CTC_ERROR_RETURN(sys_usw_qos_map_token_rate_user_to_hw(lchip, is_pps,
                                                                  p_shp_param->cir,
                                                                  &bucket_rate,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                                  gran,
                                                                  p_usw_queue_master[lchip]->que_shp_update_freq,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
    SYS_QOS_COMPUTE_SHAPE_TOKEN_THRD(p_shp_param->cbs, bucket_rate, 0, is_pps);

    /* convert to internal token thrd */
    CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, p_shp_param->cbs,
                                                                  &bucket_thrd,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH),
                                                                  SYS_SHAPE_MAX_TOKEN_THRD));

    p_sys_profile_old = p_sys_queue->p_meter_profile;
    sys_profile_new.bucket_rate = bucket_rate;
    sys_profile_new.bucket_thrd = bucket_thrd;
    CTC_ERROR_RETURN(_sys_usw_queue_meter_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_new, &p_sys_profile_get));
    p_sys_queue->p_meter_profile = p_sys_profile_get;

    /*write policer prof*/
    CTC_ERROR_RETURN(_sys_usw_queue_shp_add_queue_meter_profile_to_asic(lchip, p_sys_queue));

    return ret;
}

int32
_sys_usw_queue_shp_add_queue_shape_profile(uint8 lchip, ctc_qos_shape_queue_t* p_shp_param,
                                       sys_queue_node_t* p_sys_queue)
{
    sys_queue_shp_profile_t* p_sys_profile_old = NULL;
    sys_queue_shp_profile_t sys_profile_new;
    sys_queue_shp_profile_t* p_sys_profile_get = NULL;
    int32 ret = 0;
    uint32 gran = 0;
    uint32 bucket_rate = 0;
    uint32 bucket_thrd = 0;
    uint8  is_pps = 0;
    uint8  channel = 0;

    CTC_PTR_VALID_CHECK(p_shp_param);
    CTC_PTR_VALID_CHECK(p_sys_queue);

    sal_memset(&sys_profile_new, 0, sizeof(sys_queue_shp_profile_t));

    CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip, p_sys_queue->queue_id, &channel));
    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);

    CTC_ERROR_RETURN(sys_usw_qos_get_shp_granularity(lchip, p_shp_param->pir,
                                                            SYS_SHAPE_TYPE_QUEUE, &gran));
    /* convert to internal token rate */

   CTC_ERROR_RETURN(sys_usw_qos_map_token_rate_user_to_hw(lchip, is_pps,
                                                                 p_shp_param->pir,
                                                                 &bucket_rate,
                                                                 MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                                 gran,
                                                                 p_usw_queue_master[lchip]->que_shp_update_freq,
                                                                 MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
    SYS_QOS_COMPUTE_SHAPE_TOKEN_THRD(p_shp_param->pbs, bucket_rate, 0 ,is_pps);

    /* convert to internal token thrd */
    CTC_ERROR_RETURN(_sys_usw_qos_map_token_thrd_user_to_hw(lchip, p_shp_param->pbs,
                                                                  &bucket_thrd,
                                                                  MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH),
                                                                  SYS_SHAPE_MAX_TOKEN_THRD));

    p_sys_profile_old = p_sys_queue->p_shp_profile;
    sys_usw_queue_get_queue_type_by_queue_id(lchip, p_sys_queue->queue_id, &(sys_profile_new.queue_type));
    sys_profile_new.bucket_rate = bucket_rate;
    sys_profile_new.bucket_thrd = bucket_thrd;
    CTC_ERROR_RETURN(_sys_usw_queue_shp_profile_add_spool(lchip, p_sys_profile_old, &sys_profile_new, &p_sys_profile_get));
    p_sys_queue->p_shp_profile = p_sys_profile_get;

    /*write policer prof*/
     CTC_ERROR_RETURN(_sys_usw_queue_shp_add_queue_shape_profile_to_asic(lchip, p_sys_queue));

    return ret;
}

int32
sys_usw_queue_shp_set_queue_shape_enable(uint8 lchip, uint16 queue_id,
                                           ctc_qos_shape_queue_t* p_shape)
{
    sys_queue_node_t* p_sys_queue_node = NULL;
    int32 ret = 0;
    uint8 is_pps = 0;
    uint8 channel = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_shape);

    CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip, queue_id, &channel));
    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);
    if (is_pps)
    {
        CTC_MAX_VALUE_CHECK(p_shape->pir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE_PPS));
        CTC_MAX_VALUE_CHECK(p_shape->cir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE_PPS));
        p_shape->cbs = MIN(p_shape->cbs,(SYS_QOS_SHAPE_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
        p_shape->pbs = MIN(p_shape->pbs,(SYS_QOS_SHAPE_MAX_COUNT / MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES)));
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_shape->pir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE));
        CTC_MAX_VALUE_CHECK(p_shape->cir, MCHIP_CAP(SYS_CAP_QOS_SHP_RATE));
        p_shape->cbs = MIN(p_shape->cbs,(SYS_QOS_SHAPE_MAX_COUNT / 125));
        p_shape->pbs = MIN(p_shape->pbs,(SYS_QOS_SHAPE_MAX_COUNT / 125));
    }

    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    p_sys_queue_node->type = p_shape->queue.queue_type;
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id:%d\n", queue_id);

    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        CTC_ERROR_GOTO(sys_usw_queue_shp_update_group_shape(lchip, queue_id,
                                            p_shape->cir, p_shape->cbs, p_shape->queue.queue_id, 0, 1), ret, error0);
    }

    /*add queue meter prof*/
    if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        CTC_ERROR_GOTO(_sys_usw_queue_shp_add_queue_meter_profile(lchip, p_shape,
                                                                           p_sys_queue_node), ret, error1);
    }

    /*add queue shape prof*/
    CTC_ERROR_GOTO(_sys_usw_queue_shp_add_queue_shape_profile(lchip, p_shape,
                                                                            p_sys_queue_node), ret, error2);
        /*add queue meter prof*/
    if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        CTC_ERROR_GOTO(_sys_usw_queue_shp_add_queue_meter_to_asic(lchip,
                                                                            p_sys_queue_node), ret, error3);
    }
    CTC_ERROR_GOTO(_sys_usw_queue_shp_add_queue_shape_to_asic(lchip,
                                                                            p_sys_queue_node), ret, error3);

    p_sys_queue_node->shp_en = 1;

    return ret;

error3:
    if (NULL != p_sys_queue_node->p_shp_profile)
    {
        (void)_sys_usw_queue_shp_remove_queue_shape_profile(lchip, p_sys_queue_node->p_shp_profile);
    }
error2:
    if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        if (NULL != p_sys_queue_node->p_meter_profile)
        {
            (void)_sys_usw_queue_shp_remove_queue_meter_profile(lchip, p_sys_queue_node->p_meter_profile);
        }
    }
error1:
    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        (void)sys_usw_queue_shp_update_group_shape(lchip, queue_id, 0, 0, p_shape->queue.queue_id, 0, 0);
    }
error0:
    return ret;
}

/**
 @brief Unset shaping for the given queue in a chip.
*/
int32
sys_usw_queue_shp_set_queue_shape_disable(uint8 lchip, uint16 queue_id,
                                                                    ctc_qos_shape_queue_t* p_shape)
{
    int32 ret = 0;
    sys_queue_node_t* p_sys_queue_node = NULL;

    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    sys_usw_queue_get_queue_type_by_queue_id(lchip, queue_id, &(p_sys_queue_node->type));

    /*remove queue shape prof*/
    (void)_sys_usw_queue_shp_remove_queue_shape_from_asic(lchip, p_sys_queue_node);
    if (NULL != p_sys_queue_node->p_shp_profile)
    {
        (void)_sys_usw_queue_shp_remove_queue_shape_profile(lchip, p_sys_queue_node->p_shp_profile);
    }

    /*remove queue meter prof*/
    if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        (void)_sys_usw_queue_shp_remove_queue_meter_from_asic(lchip, p_sys_queue_node);
        if (NULL != p_sys_queue_node->p_meter_profile)
        {
            (void)_sys_usw_queue_shp_remove_queue_meter_profile(lchip, p_sys_queue_node->p_meter_profile);
        }
    }

    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        (void)sys_usw_queue_shp_update_group_shape(lchip, queue_id, 0, 0, p_shape->queue.queue_id, 0, 0);
    }

    p_sys_queue_node->shp_en = 0;
    p_sys_queue_node->p_shp_profile = NULL;
    p_sys_queue_node->p_meter_profile = NULL;

    return ret;
}

int32
sys_usw_queue_shp_set_queue_shape(uint8 lchip, ctc_qos_shape_queue_t* p_shape)
{
    uint16 queue_id = 0;

    /*get queue_id*/

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->queue_id = %d\n", p_shape->queue.queue_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->queue_type = %d\n", p_shape->queue.queue_type);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->enable = %d\n", p_shape->enable);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->gport = %d\n", p_shape->queue.gport);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->pir = %d\n", p_shape->pir);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_shape->cir = %d\n", p_shape->cir);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,  &p_shape->queue, &queue_id));

    if (p_shape->enable)
    {
        CTC_ERROR_RETURN(sys_usw_queue_shp_set_queue_shape_enable(lchip, queue_id, p_shape));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_queue_shp_set_queue_shape_disable(lchip, queue_id, p_shape));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_shp_get_queue_shape(uint8 lchip, ctc_qos_shape_queue_t* p_shape)
{
    uint16 queue_id = 0;
    uint8 que_offset = 0;
    uint8 is_pps = 0;
    uint16 ext_grp_id = 0;
    uint32 token_rate = 0;
    uint32 token_thrd = 0;
    uint8 channel = 0;
    sys_queue_node_t* p_queue_node = NULL;
    sys_queue_group_node_t* p_group_node = NULL;
    sys_group_meter_profile_t* p_meter_profile = NULL;

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_shape->queue,
                                                           &queue_id));
    p_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec,queue_id);
    if (NULL == p_queue_node)
    {
        return CTC_E_INVALID_PARAM;
    }
    p_shape->cir = SYS_SHAPE_DEFAULT_RATE;
    p_shape->cbs = SYS_SHAPE_DEFAULT_THRD;
    p_shape->pir = SYS_SHAPE_DEFAULT_RATE;
    p_shape->pbs = SYS_SHAPE_DEFAULT_THRD;
    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        SYS_EXT_GROUP_ID(queue_id, ext_grp_id);
        p_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, ext_grp_id);
        if (NULL == p_group_node)
        {
            return CTC_E_INVALID_PARAM;
        }
        que_offset = p_queue_node->offset;
        p_meter_profile = p_group_node->p_meter_profile;
        is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, p_group_node->chan_id);
        p_shape->cir = SYS_SHAPE_DEFAULT_RATE;
        p_shape->cbs = SYS_SHAPE_DEFAULT_THRD;
        if (p_meter_profile != NULL && p_meter_profile->c_bucket[que_offset].cir_type == SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER)
        {
            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_meter_profile->c_bucket[que_offset].rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                  p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
            _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_meter_profile->c_bucket[que_offset].thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            p_shape->cir = token_rate;
            p_shape->cbs = token_thrd;
        }
    }

    if (p_queue_node->shp_en && p_queue_node->p_shp_profile)
    {
        CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip,p_queue_node->queue_id, &channel));
        is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);
        if (SYS_IS_NETWORK_BASE_QUEUE(queue_id) && p_queue_node->p_meter_profile)
        {
            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_queue_node->p_meter_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                  p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
            _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_queue_node->p_meter_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            p_shape->cir = token_rate;
            p_shape->cbs = token_thrd;
        }
        else if(SYS_IS_NETWORK_BASE_QUEUE(queue_id) && p_queue_node->p_meter_profile == NULL)
        {
            p_shape->cir = SYS_SHAPE_DEFAULT_RATE;
            p_shape->cbs = SYS_SHAPE_DEFAULT_THRD;
        }
        sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_queue_node->p_shp_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
        _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_queue_node->p_shp_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
        SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
        p_shape->pir = token_rate;
        p_shape->pbs = token_thrd;
        p_shape->enable = 1;
    }

    return CTC_E_NONE;
}
int32
_sys_usw_queue_shp_init_queue_shape(uint8 lchip, uint8 slice_id)
{
    DsQMgrNetQueShpProfile_m net_queue_profile;
    DsQMgrDmaQueShpProfile_m dma_queue_profile;
    DsQMgrExtQueShpProfile_m ext_queue_profile;
    uint32 cmd;

    sal_memset(&net_queue_profile, 0, sizeof(DsQMgrNetQueShpProfile_m));
    sal_memset(&dma_queue_profile, 0, sizeof(DsQMgrDmaQueShpProfile_m));
    sal_memset(&ext_queue_profile, 0, sizeof(DsQMgrExtQueShpProfile_m));

    /* reserved queue shape profile 0 for max*/
    SetDsQMgrNetQueShpProfile(V, tokenRateFrac_f, &net_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
    SetDsQMgrNetQueShpProfile(V, tokenRate_f, &net_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
    SetDsQMgrNetQueShpProfile(V, tokenThrdShift_f, &net_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    SetDsQMgrNetQueShpProfile(V, tokenThrd_f, &net_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
    cmd = DRV_IOW(DsQMgrNetQueShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_queue_profile));

    SetDsQMgrDmaQueShpProfile(V, tokenRateFrac_f, &dma_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
    SetDsQMgrDmaQueShpProfile(V, tokenRate_f, &dma_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
    SetDsQMgrDmaQueShpProfile(V, tokenThrdShift_f, &dma_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    SetDsQMgrDmaQueShpProfile(V, tokenThrd_f, &dma_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
    cmd = DRV_IOW(DsQMgrDmaQueShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_queue_profile));

    /*rsv profile 1 for dma queue shape pir 0*/
    sal_memset(&dma_queue_profile, 0, sizeof(DsQMgrDmaQueShpProfile_m));
    cmd = DRV_IOW(DsQMgrDmaQueShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 1, cmd, &dma_queue_profile));
    SetDsQMgrExtQueShpProfile(V, tokenRateFrac_f, &ext_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC));
    SetDsQMgrExtQueShpProfile(V, tokenRate_f, &ext_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
    SetDsQMgrExtQueShpProfile(V, tokenThrdShift_f, &ext_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    SetDsQMgrExtQueShpProfile(V, tokenThrd_f, &ext_queue_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
    cmd = DRV_IOW(DsQMgrExtQueShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ext_queue_profile));

    /*tmp = 0;
    cmd = DRV_IOW(QMgrNetPktAdj_t, QMgrNetPktAdj_netPktAdjVal0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
    tmp = 0;
    cmd = DRV_IOW(QMgrNetPktAdj_t, QMgrNetPktAdj_netPktAdjVal1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
    tmp = 0;
    cmd = DRV_IOW(QMgrNetPktAdj_t, QMgrNetPktAdj_netPktAdjVal2_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
    tmp = 0;
    cmd = DRV_IOW(QMgrNetPktAdj_t, QMgrNetPktAdj_netPktAdjVal3_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));*/

    return CTC_E_NONE;
}

#define global_shaping_enable ""

/**
 @brief Globally set shape mode.
  mode 0: chan_shp_update_freq = que_shp_update_freq
  mode 1: chan_shp_update_freq = 16*que_shp_update_freq
*/
int32
sys_usw_qos_set_shape_mode(uint8 lchip, uint8 mode)
{
    return CTC_E_NONE;
}

/**
 @brief Globally enable/disable channel shaping function.
*/
int32
sys_usw_qos_set_port_shape_enable(uint8 lchip, uint8 enable)
{
    uint32 cmd;
    uint32 tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    tmp = enable ? 1 : 0;

    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_chanShpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sys_shape_ctl.channel_shape_enable = %d\n", enable);

    return CTC_E_NONE;
}

/**
 @brief Get channel shape global enable stauts.
*/
int32
sys_usw_qos_get_port_shape_enable(uint8 lchip, uint32* p_enable)
{
    uint32 cmd;
    uint32 tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_enable);

    cmd = DRV_IOR(QMgrCtl_t, QMgrCtl_chanShpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    *p_enable = tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sys_shape_ctl.channel_shape_enable = %d\n", *p_enable);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_get_port_shape_profile(uint8 lchip, uint32 gport, uint32* rate, uint32* thrd, uint8* p_shp_en)
{
    uint32 cmd;
    uint8 channel = 0;
    uint32 token_thrd = 0;
    uint32 token_thrd_shift = 0;
    uint32 token_rate = 0;
    uint32 token_rate_frac = 0;
    uint8 is_pps = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(rate);
    CTC_PTR_VALID_CHECK(thrd);
    if (CTC_IS_CPU_PORT(gport))
    {
        channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);
    }
    else
    {
        _sys_usw_get_channel_by_port(lchip, gport, &channel);
    }

    CTC_MAX_VALUE_CHECK(channel, MCHIP_CAP(SYS_CAP_CHANID_MAX) - 1);

    if (channel < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        DsQMgrNetChanShpProfile_m  net_chan_shp_profile;
        sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));

        cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &net_chan_shp_profile));

        token_thrd = GetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile);
        token_thrd_shift = GetDsQMgrNetChanShpProfile(V, tokenThrdShift_f, &net_chan_shp_profile);
        token_rate_frac = GetDsQMgrNetChanShpProfile(V, tokenRateFrac_f, &net_chan_shp_profile);
        token_rate = GetDsQMgrNetChanShpProfile(V, tokenRate_f, &net_chan_shp_profile);
    }
    else if (SYS_IS_DMA_CHANNEL(channel))
    {
        QMgrDmaChanShpProfile_m  dma_chan_shp_profile;
        sal_memset(&dma_chan_shp_profile, 0, sizeof(QMgrDmaChanShpProfile_m));

        cmd = DRV_IOR(QMgrDmaChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_chan_shp_profile));

        token_thrd = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrd_f, &dma_chan_shp_profile);
        token_thrd_shift = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrdShift_f  , &dma_chan_shp_profile);
        token_rate_frac = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenRateFrac_f, &dma_chan_shp_profile);
        token_rate = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenRate_f  , &dma_chan_shp_profile);
    }
    else
    {
        DsQMgrMiscChanShpProfile_m  misc_chan_shp_profile;
        sal_memset(&misc_chan_shp_profile, 0, sizeof(DsQMgrMiscChanShpProfile_m));

        cmd = DRV_IOR(DsQMgrMiscChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel-MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM), cmd, &misc_chan_shp_profile));

        token_thrd = GetDsQMgrMiscChanShpProfile(V, tokenThrd_f, &misc_chan_shp_profile);
        token_thrd_shift = GetDsQMgrMiscChanShpProfile(V, tokenThrdShift_f  , &misc_chan_shp_profile);
        token_rate_frac = GetDsQMgrMiscChanShpProfile(V, tokenRateFrac_f, &misc_chan_shp_profile);
        token_rate = GetDsQMgrMiscChanShpProfile(V, tokenRate_f  , &misc_chan_shp_profile);
    }
    is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);
    *p_shp_en = (token_thrd_shift == 0x11) ? 0 : 1;

    sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, (token_rate << 8 | token_rate_frac)  , &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                            p_usw_queue_master[lchip]->chan_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
    _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , (token_thrd << MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH) | token_thrd_shift), MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
    SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);

    *rate = token_rate;
    *thrd  = token_thrd;

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_queue_shape_enable(uint8 lchip, uint8 enable)
{
    uint32 tmp;
    uint32 cmd;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    tmp = enable ? 1 : 0;
    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_queShpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sys_shape_ctl.queue_shape_enable = %d\n", enable);

    return CTC_E_NONE;
}

/**
 @brief Get queue shape global enable status.
*/
int32
sys_usw_qos_get_queue_shape_enable(uint8 lchip, uint32* p_enable)
{
    uint32 cmd;
    uint32 tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_enable);

    cmd = DRV_IOR(QMgrCtl_t, QMgrCtl_queShpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    *p_enable = tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sys_shape_ctl.queue_shape_enable = %d\n", *p_enable);

    return CTC_E_NONE;
}

/**
 @brief Globally enable/disable group shaping function.
*/
int32
sys_usw_qos_set_group_shape_enable(uint8 lchip, uint8 enable)
{
    uint32 tmp;
    uint32 cmd;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    tmp = enable ? 1 : 0;
    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_grpShpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_grpMeterEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sys_shape_ctl.queue_shape_enable = %d\n", enable);

    return CTC_E_NONE;
}

/**
 @brief Get group shape global enable status.
*/
int32
sys_usw_qos_get_group_shape_enable(uint8 lchip, uint32* p_enable)
{
    uint32 cmd;
    uint32 tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_enable);

    cmd = DRV_IOR(QMgrCtl_t, QMgrCtl_grpShpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

    *p_enable = tmp;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_shape_enable = %d\n", *p_enable);

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_shape_ipg_enable(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = enable ? CTC_DEFAULT_IPG : 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjust_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    else
    {
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g_0_ipgLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(ErmAqmPortCtl_t, ErmAqmPortCtl_ipgLenAdjust_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        field_val = enable;
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_ipgEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_port_shp_base_pkt_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8  channel = 0;
    DsQMgrNetShpPpsCfg_m net_shp_pps;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&net_shp_pps, 0, sizeof(DsQMgrNetShpPpsCfg_m));
    field_val = enable? 1 : 0;

    /* channel always enable */
    for(channel = 0; channel < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); channel ++)
    {
        SetDsQMgrNetShpPpsCfg(V, ppsMode_f, &net_shp_pps, field_val);
        SetDsQMgrNetShpPpsCfg(V, ppsShift_f, &net_shp_pps, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_SHIFT));
        cmd = DRV_IOW(DsQMgrNetShpPpsCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &net_shp_pps));
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_reason_shp_base_pkt_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8  channel = 0;
    DsQMgrNetShpPpsCfg_m net_shp_pps;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&net_shp_pps, 0, sizeof(DsQMgrNetShpPpsCfg_m));
    field_val = enable? 1 : 0;

    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_dmaShpPpsMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    p_usw_queue_master[lchip]->shp_pps_en = enable;

    /* channel always enable */
    if (p_usw_queue_master[lchip]->have_lcpu_by_eth)
    {
        channel = p_usw_queue_master[lchip]->cpu_eth_chan_id;

        SetDsQMgrNetShpPpsCfg(V, ppsMode_f, &net_shp_pps, field_val);
        SetDsQMgrNetShpPpsCfg(V, ppsShift_f, &net_shp_pps, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_SHIFT));
        cmd = DRV_IOW(DsQMgrNetShpPpsCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &net_shp_pps));
    }

    return CTC_E_NONE;
}

#if 0
int32
sys_usw_qos_set_shp_remark_color_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8 priority = 0;
    uint8 in_profile = 0;
    uint8 index = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    for (priority = 0; priority <= MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX); priority++)
    {
        /*In profiel (CIR)*/
        in_profile = 1;
        index = (priority << 1) | in_profile;
        field_val = enable? CTC_QOS_COLOR_GREEN : 0;
        cmd = DRV_IOW(DsBufRetrvColorMap_t, DsBufRetrvColorMap_color_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

        /*Out profiel (PIR)*/
        field_val = enable? CTC_QOS_COLOR_YELLOW : 0;
        in_profile = 0;
        index = (priority << 1) | in_profile;
        cmd = DRV_IOW(DsBufRetrvColorMap_t, DsBufRetrvColorMap_color_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }

    return CTC_E_NONE;
}
#endif
#define shp_api ""

STATIC int32
_sys_usw_qos_get_channel_status(uint8 lchip, ctc_qos_shape_t* p_shape, uint8* b_channel)
{
    uint32 gport = 0;
    uint8  bpe_en = 0;

    if ((CTC_QOS_SHAPE_PORT == p_shape->type)
        || (CTC_QOS_SHAPE_GROUP == p_shape->type))
    {
        gport = (CTC_QOS_SHAPE_PORT == p_shape->type) ? p_shape->shape.port_shape.gport : p_shape->shape.group_shape.gport;

        CTC_ERROR_RETURN(sys_usw_qos_get_mux_port_enable(lchip, gport, &bpe_en));

        if (CTC_IS_CPU_PORT(gport))
        {
            *b_channel = 1;
        }
        else if(bpe_en)
        {
            *b_channel = 0;
        }
        else
        {
            *b_channel = 1;
        }
    }

    return CTC_E_NONE;
}
int32
_sys_usw_qos_set_shape(uint8 lchip, ctc_qos_shape_t* p_shape)
{
    uint8 b_channel = 0;
    uint32 dot1ae_en = 0;
    CTC_PTR_VALID_CHECK(p_shape);

    CTC_ERROR_RETURN(_sys_usw_qos_get_channel_status(lchip, p_shape, &b_channel));

    /*CTC_ERROR_RETURN(
           sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_DOT1AE_EN, CTC_EGRESS, &dot1ae_en));*/

    switch (p_shape->type)
    {
    case CTC_QOS_SHAPE_PORT:
        if (0 == b_channel)
        {
            ctc_qos_shape_group_t group_shape;
            sal_memset(&group_shape, 0, sizeof(ctc_qos_shape_group_t));

            group_shape.enable      = p_shape->shape.port_shape.enable;
            group_shape.pbs         = p_shape->shape.port_shape.pbs;
            group_shape.pir         = p_shape->shape.port_shape.pir;
            group_shape.queue.gport         = p_shape->shape.port_shape.gport;
            group_shape.queue.queue_id      = 0;
            group_shape.queue.queue_type    = CTC_QUEUE_TYPE_NETWORK_EGRESS;

            CTC_ERROR_RETURN(
                _sys_usw_queue_shp_set_group_shape(lchip, &group_shape));
        }
        else
        {
            CTC_ERROR_RETURN(
                sys_usw_queue_shp_set_channel_shape(lchip, &p_shape->shape.port_shape));

            if (dot1ae_en)
            {
                ctc_qos_shape_group_t group_shape;
                sal_memset(&group_shape, 0, sizeof(ctc_qos_shape_group_t));

                group_shape.enable      = p_shape->shape.port_shape.enable;
                group_shape.pbs         = p_shape->shape.port_shape.pbs;
                group_shape.pir         = p_shape->shape.port_shape.pir;
                group_shape.queue.gport         = p_shape->shape.port_shape.gport;
                group_shape.queue.queue_id      = 0;
                group_shape.queue.queue_type    = CTC_QUEUE_TYPE_NETWORK_EGRESS;

                CTC_ERROR_RETURN(
                    _sys_usw_queue_shp_set_group_shape(lchip, &group_shape));
            }
        }
        break;

    case CTC_QOS_SHAPE_QUEUE:
        CTC_ERROR_RETURN(
            sys_usw_queue_shp_set_queue_shape(lchip, &p_shape->shape.queue_shape));
        break;

    case CTC_QOS_SHAPE_GROUP:
        if ((0 == b_channel) || dot1ae_en)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
        else
        {
            CTC_ERROR_RETURN(
                _sys_usw_queue_shp_set_group_shape(lchip, &p_shape->shape.group_shape));
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
};

int32
_sys_usw_qos_get_shape(uint8 lchip, ctc_qos_shape_t* p_shape)
{
    uint8 b_channel = 0;
    uint32 dot1ae_en = 0;
    CTC_PTR_VALID_CHECK(p_shape);

    CTC_ERROR_RETURN(_sys_usw_qos_get_channel_status(lchip, p_shape, &b_channel));

    /*CTC_ERROR_RETURN(
           sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_DOT1AE_EN, CTC_EGRESS, &dot1ae_en));*/

    switch (p_shape->type)
    {
    case CTC_QOS_SHAPE_PORT:
        if (0 == b_channel)
        {
            ctc_qos_shape_group_t group_shape;
            sal_memset(&group_shape, 0, sizeof(ctc_qos_shape_group_t));

            group_shape.queue.gport         = p_shape->shape.port_shape.gport;
            group_shape.queue.queue_id      = 0;
            group_shape.queue.queue_type    = CTC_QUEUE_TYPE_NETWORK_EGRESS;

            CTC_ERROR_RETURN(_sys_usw_queue_shp_get_group_shape(lchip, &group_shape));
            p_shape->shape.port_shape.enable = group_shape.enable;
            p_shape->shape.port_shape.pbs = group_shape.pbs;
            p_shape->shape.port_shape.pir = group_shape.pir;
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_queue_shp_get_channel_shape(lchip, &p_shape->shape.port_shape));
        }
        break;

    case CTC_QOS_SHAPE_QUEUE:
        CTC_ERROR_RETURN(sys_usw_queue_shp_get_queue_shape(lchip, &p_shape->shape.queue_shape));
        break;

    case CTC_QOS_SHAPE_GROUP:
        if ((0 == b_channel) || dot1ae_en)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_queue_shp_get_group_shape(lchip, &p_shape->shape.group_shape));
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
};
int32
sys_usw_qos_shape_dump_status(uint8 lchip)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------Shape--------------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Shape mode for To LCPU", p_usw_queue_master[lchip]->shp_pps_en);
    /*cmd = DRV_IOR(IpeIpgCtl_t, IpeIpgCtl_array_0_ipg_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Shape IPG enable", (value1 != 0));
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "IPG Size", value1);*/
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Total Queue shape profile", MCHIP_CAP(SYS_CAP_QOS_QUEUE_SHAPE_PROFILE));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "--Used count", p_usw_queue_master[lchip]->p_queue_profile_pool->count + p_usw_queue_master[lchip]->p_queue_meter_profile_pool->count + 5);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "Total Group shape profile", MCHIP_CAP(SYS_CAP_QOS_GROUP_SHAPE_PROFILE) * 2);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %d\n", "--Used count",  p_usw_queue_master[lchip]->p_group_profile_pool->count + p_usw_queue_master[lchip]->p_group_profile_meter_pool->count + 2);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

/**
 @brief Queue shaper initialization.
*/
int32
sys_usw_queue_shape_init(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    uint8  slice_id = 0;
    uint32 cmd = 0;
    uint32 max_size = 0;
    uint32 field_val = 0;
    uint8 index = 0;
    ctc_spool_t spool;
    QMgrCtl_m qmgr_ctl;
    RefDivQMgrDeqShpPulse_m shp_pulse;
    sys_qos_rate_granularity_t shp_gran[SYS_SHP_GRAN_RANAGE_NUM] = {{ 2,       10} ,
                                                                    {100,      40},
                                                                    {1000,     80},
                                                                    {10000,    500},
                                                                    {40000,    1000},
                                                                    {100000,   2000}};

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    /* init queue shape hash table */
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_QUEUE_SHAPE_PROFILE_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_QUEUE_SHAPE_PROFILE) - 1;
    spool.user_data_size = sizeof(sys_queue_shp_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_queue_shp_hash_make_queue_shape_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_queue_shp_hash_cmp_queue_shape_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_queue_shp_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_queue_shp_free_profileId;
    p_usw_queue_master[lchip]->p_queue_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_queue_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    /*init queue shape profile opf(net queue/dma queue/extend queue)*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_queue_shape,
                                                                        3, "opf-queue-shape-profile"), ret, error1);

    sys_usw_ftm_query_table_entry_num(lchip, DsQMgrNetQueShpProfile_t, &max_size);
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_queue_shape;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, max_size - 1), ret, error2);

    sys_usw_ftm_query_table_entry_num(lchip, DsQMgrDmaQueShpProfile_t, &max_size);
    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 2, max_size - 2), ret, error2);

    sys_usw_ftm_query_table_entry_num(lchip, DsQMgrExtQueShpProfile_t, &max_size);
    opf.pool_index = 2;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, max_size - 1), ret, error2);

    /* init group shape hash table */
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_GROUP_SHAPE_PROFILE_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_GROUP_SHAPE_PROFILE) - 1;
    spool.user_data_size = sizeof(sys_group_shp_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_queue_shp_hash_make_group_shape_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_queue_shp_hash_cmp_group_shape_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_group_shp_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_group_shp_free_profileId;
    p_usw_queue_master[lchip]->p_group_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_group_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }

    /*init group shape profile opf(ext group shp/ext group meter)*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_group_shape,
                                                                        1, "opf-group-shape-profile"), ret, error3);

    sys_usw_ftm_query_table_entry_num(lchip, DsQMgrExtGrpShpProfile_t, &max_size);
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_group_shape;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, max_size - 1), ret, error4);

    /* init group meter hash table */
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_GROUP_SHAPE_PROFILE_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_GROUP_SHAPE_PROFILE) - 1;
    spool.user_data_size = sizeof(sys_group_meter_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_queue_shp_hash_make_group_meter_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_queue_shp_hash_cmp_group_meter_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_group_meter_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_group_meter_free_profileId;
    p_usw_queue_master[lchip]->p_group_profile_meter_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_group_profile_meter_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error4;
    }

    /*init group shape profile opf(ext group shp/ext group meter)*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_group_meter,
                                                                        1, "opf-group-meter-profile"), ret, error5);

    sys_usw_ftm_query_table_entry_num(lchip, DsQMgrExtGrpMeterProfile_t, &max_size);
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_group_meter;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, max_size - 1), ret, error6);

    /* init base queue meter hash table */
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_QUEUE_SHAPE_PROFILE_BLOCK_SIZE;
    spool.max_count = MCHIP_CAP(SYS_CAP_QOS_QUEUE_METER_PROFILE) - 1;
    spool.user_data_size = sizeof(sys_queue_meter_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_queue_shp_hash_make_queue_meter_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_queue_shp_hash_cmp_queue_meter_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_queue_meter_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_queue_meter_free_profileId;
    p_usw_queue_master[lchip]->p_queue_meter_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_queue_meter_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error6;
    }

    /*init base queue meter profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_queue_meter,
                                                                        1, "opf-queue-meter-profile"), ret, error7);

    sys_usw_ftm_query_table_entry_num(lchip, DsQMgrNetBaseQueMeterProfile_t, &max_size);
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_queue_meter;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, max_size - 1), ret, error8);

    p_usw_queue_master[lchip]->que_shp_update_freq = DRV_IS_DUET2(lchip)?128000 :64000; /*128k for support 400M core_freq*/
    p_usw_queue_master[lchip]->chan_shp_update_freq = DRV_IS_DUET2(lchip)?128000 :64000;
    p_usw_queue_master[lchip]->shp_pps_en = 1;
    sal_memcpy(p_usw_queue_master[lchip]->que_shp_gran, shp_gran,
               SYS_SHP_GRAN_RANAGE_NUM * sizeof(sys_qos_rate_granularity_t));

    sal_memset(&qmgr_ctl, 0, sizeof(QMgrCtl_m));
    cmd = DRV_IOR(QMgrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &qmgr_ctl), ret, error8);

    SetQMgrCtl(V, chanShpEn_f, &qmgr_ctl, 1);        /*Temp Disable for EMU testing*/
    SetQMgrCtl(V, discardAgedPkt_f, &qmgr_ctl, 1);
    SetQMgrCtl(V, dmaShpPpsMode_f  , &qmgr_ctl, 0);
    SetQMgrCtl(V, dmaShpPpsShift_f  , &qmgr_ctl, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_SHIFT));
    SetQMgrCtl(V, grpMeterEn_f, &qmgr_ctl, 1);
    SetQMgrCtl(V, grpShpEn_f  , &qmgr_ctl, 1);
    SetQMgrCtl(V, packetLengthAdjustReturnEn_f, &qmgr_ctl, 0);
    SetQMgrCtl(V, queShpEn_f  , &qmgr_ctl, 1);
    cmd = DRV_IOW(QMgrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &qmgr_ctl), ret, error8);

    /*Freq: 128k, pulse:0x4c3b4*/
    sal_memset(&shp_pulse, 0, sizeof(RefDivQMgrDeqShpPulse_m));
    SetRefDivQMgrDeqShpPulse(V, cfgRefDivQMgrDeqExtShpPulse_f, &shp_pulse, DRV_IS_DUET2(lchip)?0x4c3b4 :0x98868);
    SetRefDivQMgrDeqShpPulse(V, cfgResetDivQMgrDeqExtShpPulse_f, &shp_pulse, 0);
    SetRefDivQMgrDeqShpPulse(V, cfgRefDivQMgrDeqMiscShpPulse_f, &shp_pulse, DRV_IS_DUET2(lchip)?0x4c3b4 :0x98868);
    SetRefDivQMgrDeqShpPulse(V, cfgResetDivQMgrDeqMiscShpPulse_f, &shp_pulse, 0);
    SetRefDivQMgrDeqShpPulse(V, cfgRefDivQMgrDeqNetShpPulse_f, &shp_pulse, DRV_IS_DUET2(lchip)?0x4c3b4 :0x98868);
    SetRefDivQMgrDeqShpPulse(V, cfgResetDivQMgrDeqNetShpPulse_f, &shp_pulse, 0);
    cmd = DRV_IOW(RefDivQMgrDeqShpPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &shp_pulse), ret, error8);

    for (index = 0; index < MCHIP_CAP(SYS_CAP_CHANID_MAX); index++)
    {
        cmd = DRV_IOW(DsErmChannel_t, DsErmChannel_ipgIndex_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }

    CTC_ERROR_GOTO(_sys_usw_queue_shp_init_queue_shape(lchip, slice_id), ret, error8);
    CTC_ERROR_GOTO(_sys_usw_queue_shp_init_group_shape(lchip, slice_id), ret, error8);
    CTC_ERROR_GOTO(_sys_usw_queue_shp_init_channel_shape(lchip, slice_id), ret, error8);

    CTC_ERROR_GOTO(sys_usw_qos_set_reason_shp_base_pkt_en(lchip, TRUE), ret, error8);

    return CTC_E_NONE;

error8:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_meter);
error7:
    ctc_spool_free(p_usw_queue_master[lchip]->p_queue_meter_profile_pool);
    p_usw_queue_master[lchip]->p_queue_meter_profile_pool = NULL;
error6:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_group_meter);
error5:
    ctc_spool_free(p_usw_queue_master[lchip]->p_group_profile_meter_pool);
    p_usw_queue_master[lchip]->p_group_profile_meter_pool = NULL;
error4:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_group_shape);
error3:
    ctc_spool_free(p_usw_queue_master[lchip]->p_group_profile_pool);
    p_usw_queue_master[lchip]->p_group_profile_pool = NULL;
error2:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_shape);
error1:
    ctc_spool_free(p_usw_queue_master[lchip]->p_queue_profile_pool);
    p_usw_queue_master[lchip]->p_queue_profile_pool = NULL;
error0:
    return ret;
}

int32
sys_usw_queue_shape_deinit(uint8 lchip)
{
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_meter);

    ctc_spool_free(p_usw_queue_master[lchip]->p_queue_meter_profile_pool);
    p_usw_queue_master[lchip]->p_queue_meter_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_group_meter);

    ctc_spool_free(p_usw_queue_master[lchip]->p_group_profile_meter_pool);
    p_usw_queue_master[lchip]->p_group_profile_meter_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_group_shape);

    ctc_spool_free(p_usw_queue_master[lchip]->p_group_profile_pool);
    p_usw_queue_master[lchip]->p_group_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_shape);

    ctc_spool_free(p_usw_queue_master[lchip]->p_queue_profile_pool);
    p_usw_queue_master[lchip]->p_queue_profile_pool = NULL;

    return CTC_E_NONE;
}

