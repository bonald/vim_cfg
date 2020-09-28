#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_ptp.c

 @date 2010-6-7

 @version v2.0

  This file contains PTP sys layer function implementation
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_ptp.h"         /* ctc layer PTP ds */
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_vector.h"
#include "ctc_warmboot.h"

#include "sys_usw_ptp.h"  /* sys ds for all PTP sub modules */
#include "sys_usw_chip.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_register.h"
#include "sys_usw_packet.h"
#include "sys_usw_dmps.h"

#include "drv_api.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_PTP_MAX_SYNC_CLOCK_FREQUENCY_HZ (25000000)


#define SYS_PTP_MAX_TX_TS_BLOCK_NUM  4
#define SYS_PTP_MAX_TX_TS_BLOCK_SIZE  SYS_USW_MAX_PHY_PORT
#define SYS_PTP_MAX_RX_TS_BLOCK_NUM  2
#define SYS_PTP_MAX_RX_TS_BLOCK_SIZE  MCHIP_CAP(SYS_CAP_PTP_CAPTURED_RX_SEQ_ID)


#define SYS_PTP_TIEM_INTF_SYNCE_0 0
#define SYS_PTP_TIEM_INTF_SYNCE_1 1
#define SYS_PTP_TIEM_INTF_SERDES 4
#define SYS_PTP_TIEM_INTF_SYNC_PULSE 5
#define SYS_PTP_TIEM_INTF_TOD_1PPS 6
#define SYS_PTP_TIEM_INTF_GPIO_0 7
#define SYS_PTP_TIEM_INTF_GPIO_1 8


/* clock id */
#define SYS_PTP_DEFAULT_CLOCK_ID    0
#define SYS_PTP_RESERVED_CLOCK_ID   1
#define SYS_PTP_USER_CLOCK_ID_MIN   2
#define SYS_PTP_USER_CLOCK_ID_MAX   7

#define PTP_LOCK \
    if (p_usw_ptp_master[lchip]->p_ptp_mutex) sal_mutex_lock(p_usw_ptp_master[lchip]->p_ptp_mutex)
#define PTP_UNLOCK \
    if (p_usw_ptp_master[lchip]->p_ptp_mutex) sal_mutex_unlock(p_usw_ptp_master[lchip]->p_ptp_mutex)

#define SYS_PTP_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(ptp, ptp, PTP_PTP_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_PTP_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_ptp_master[lchip]){ \
            SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_PTP_RX_SOURCE_CHECK(source) \
    do { \
        if (source >= MCHIP_CAP(SYS_CAP_PTP_CAPTURED_RX_SOURCE)){ \
            SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local phyport is not exist \n");\
			return CTC_E_NOT_EXIST;\
 } \
    } while (0)

#define SYS_PTP_TX_SEQ_ID_CHECK(seq_id) \
    do { \
        if (seq_id >= MCHIP_CAP(SYS_CAP_PTP_CAPTURED_TX_SEQ_ID)){ \
            return CTC_E_INVALID_PARAM; } \
    } while (0)

#define SYS_PTP_RX_SEQ_ID_CHECK(seq_id) \
    do { \
        if (seq_id >= MCHIP_CAP(SYS_CAP_PTP_CAPTURED_RX_SEQ_ID)){ \
            return CTC_E_INVALID_PARAM; } \
    } while (0)

/****************************************************************************
*
* Data structures
*
****************************************************************************/
/**
 @brief   ptp captured timestamp
*/
struct sys_ptp_capured_ts_s
{
    uint16  lport_or_source;     /*save captured source or lport*/
    uint8  resv;
    uint16 seq_id;            /**< sequence ID*/
    uint32 system_time;
    ctc_ptp_time_t ts;
};
typedef struct sys_ptp_capured_ts_s sys_ptp_capured_ts_t;

/**
 @brief  Define sys layer ptp global configure data structure
*/
struct sys_ptp_master_s
{
    sal_mutex_t* p_ptp_mutex;
    uint8 quanta;
    uint8 intf_selected;
    uint16 cache_aging_time;  /**< aging time for TS, unit:seconds*/
    ctc_ptp_device_type_t device_type;
    ctc_vector_t* p_tx_ts_vector;
    ctc_vector_t* p_rx_ts_vector;
    uint8 tod_1pps_duty;
    uint32 tod_1pps_delay;/**< 1pps delay between Tod master and slave*/
};
typedef struct sys_ptp_master_s sys_ptp_master_t;

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
static sys_ptp_master_t* p_usw_ptp_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC int32
_sys_usw_ptp_set_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time);


#define __1_PTP_COMMON__
STATIC int32
_sys_usw_ptp_check_ptp_init(uint8 lchip)
{
    uint32 cmd = 0;
    int32 ret = CTC_E_NONE;
    TsEngineRefRcCtl_m ptp_frc_ctl;

    /* check ptp ref rc clock, if not exist, read will fail */
    cmd = DRV_IOR(TsEngineRefRcCtl_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &ptp_frc_ctl);

    return ret;
}

STATIC int32
_sys_usw_ptp_get_quanta(uint8 lchip, uint8* quanta)
{
    *quanta = MCHIP_CAP(SYS_CAP_PTP_RC_QUANTA);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_map_captured_src(ctc_ptp_captured_src_t* ctc_src_type, uint8* sys_src_type, uint8 is_ctc2sys)
{
    if (is_ctc2sys)
    {
        switch (*ctc_src_type)
        {
            case CTC_PTP_TIEM_INTF_GPIO_0:
                *sys_src_type = SYS_PTP_TIEM_INTF_GPIO_0;
                break;

            case CTC_PTP_TIEM_INTF_GPIO_1:
                *sys_src_type = SYS_PTP_TIEM_INTF_GPIO_1;
                break;

            case CTC_PTP_TIEM_INTF_SYNCE_0:
                *sys_src_type = SYS_PTP_TIEM_INTF_SYNCE_0;
                break;

            case CTC_PTP_TIEM_INTF_SYNCE_1:
                *sys_src_type = SYS_PTP_TIEM_INTF_SYNCE_1;
                break;

            case CTC_PTP_TIEM_INTF_SERDES:
                *sys_src_type = SYS_PTP_TIEM_INTF_SERDES;
                break;

            case CTC_PTP_TIEM_INTF_SYNC_PULSE:
                *sys_src_type = SYS_PTP_TIEM_INTF_SYNC_PULSE;
                break;

            case CTC_PTP_TIEM_INTF_TOD_1PPS:
                *sys_src_type = SYS_PTP_TIEM_INTF_TOD_1PPS;
                break;

            default:
                return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        switch (*sys_src_type)
        {
            case SYS_PTP_TIEM_INTF_GPIO_0:
                *ctc_src_type = CTC_PTP_TIEM_INTF_GPIO_0;
                break;

            case SYS_PTP_TIEM_INTF_GPIO_1:
                *ctc_src_type = CTC_PTP_TIEM_INTF_GPIO_1;
                break;

            case SYS_PTP_TIEM_INTF_SYNCE_0:
                *ctc_src_type = CTC_PTP_TIEM_INTF_SYNCE_0;
                break;

            case SYS_PTP_TIEM_INTF_SYNCE_1:
                *ctc_src_type = CTC_PTP_TIEM_INTF_SYNCE_1;
                break;

            case SYS_PTP_TIEM_INTF_SERDES:
                *ctc_src_type = CTC_PTP_TIEM_INTF_SERDES;
                break;

            case SYS_PTP_TIEM_INTF_SYNC_PULSE:
                *ctc_src_type = CTC_PTP_TIEM_INTF_SYNC_PULSE;
                break;

            case SYS_PTP_TIEM_INTF_TOD_1PPS:
                *ctc_src_type = CTC_PTP_TIEM_INTF_TOD_1PPS;
                break;

            default:
                return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

#define __2_PTP_TS_WB__
STATIC int32
_sys_usw_ptp_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_ptp_master_t *p_wb_ptp_master;

    if (NULL == p_usw_ptp_master[lchip])
    {
        return CTC_E_NONE;
    }

    PTP_LOCK;

    /*syncup buffer*/
    wb_data.buffer = mem_malloc(MEM_PTP_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*syncup statsid*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_ptp_master_t, CTC_FEATURE_PTP, SYS_WB_APPID_PTP_SUBID_MASTER);

    p_wb_ptp_master = (sys_wb_ptp_master_t *)wb_data.buffer;
    sal_memset(p_wb_ptp_master, 0, sizeof(sys_wb_ptp_master_t));

    p_wb_ptp_master->lchip = lchip;
    p_wb_ptp_master->version = SYS_WB_VERSION_PTP;
    p_wb_ptp_master->quanta = p_usw_ptp_master[lchip]->quanta;
    p_wb_ptp_master->intf_selected = p_usw_ptp_master[lchip]->intf_selected;
    p_wb_ptp_master->cache_aging_time = p_usw_ptp_master[lchip]->cache_aging_time;
    switch (p_usw_ptp_master[lchip]->device_type)
    {
        case CTC_PTP_DEVICE_NONE:
        {
            p_wb_ptp_master->device_type = SYS_WB_PTP_DEVICE_NONE;
            break;
        }
        case CTC_PTP_DEVICE_OC:
        {
            p_wb_ptp_master->device_type = SYS_WB_PTP_DEVICE_OC;
            break;
        }
        case CTC_PTP_DEVICE_BC:
        {
            p_wb_ptp_master->device_type = SYS_WB_PTP_DEVICE_BC;
            break;
        }
        case CTC_PTP_DEVICE_E2E_TC:
        {
            p_wb_ptp_master->device_type = SYS_WB_PTP_DEVICE_E2E_TC;
            break;
        }
        case CTC_PTP_DEVICE_P2P_TC:
        {
            p_wb_ptp_master->device_type = SYS_WB_PTP_DEVICE_P2P_TC;
            break;
        }
        default:
        {
            p_wb_ptp_master->device_type = SYS_WB_PTP_DEVICE_NONE;
            break;
        }
    }
    p_wb_ptp_master->tod_1pps_duty = p_usw_ptp_master[lchip]->tod_1pps_duty;
    p_wb_ptp_master->tod_1pps_delay = p_usw_ptp_master[lchip]->tod_1pps_delay;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    PTP_UNLOCK;

    return ret;
}

STATIC int32
_sys_usw_ptp_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_ptp_master_t wb_ptp_master;

    PTP_LOCK;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_PTP_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ptp_master_t, CTC_FEATURE_PTP, SYS_WB_APPID_PTP_SUBID_MASTER);

    sal_memset(&wb_ptp_master, 0, sizeof(sys_wb_ptp_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if ((wb_query.valid_cnt != 1) || (wb_query.is_end != 1))
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query ptp master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        CTC_ERROR_GOTO(CTC_E_INVALID_CONFIG, ret, done);
    }

    sal_memcpy((uint8 *)&wb_ptp_master, (uint8 *)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_PTP, wb_ptp_master.version))
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }

    p_usw_ptp_master[lchip]->quanta = wb_ptp_master.quanta;
    p_usw_ptp_master[lchip]->intf_selected = wb_ptp_master.intf_selected;
    p_usw_ptp_master[lchip]->cache_aging_time = wb_ptp_master.cache_aging_time;

    switch (wb_ptp_master.device_type)
    {
        case SYS_WB_PTP_DEVICE_NONE:
        {
            p_usw_ptp_master[lchip]->device_type = CTC_PTP_DEVICE_NONE;
            break;
        }
        case SYS_WB_PTP_DEVICE_OC:
        {
            p_usw_ptp_master[lchip]->device_type = CTC_PTP_DEVICE_OC;
            break;
        }
        case SYS_WB_PTP_DEVICE_BC:
        {
            p_usw_ptp_master[lchip]->device_type = CTC_PTP_DEVICE_BC;
            break;
        }
        case SYS_WB_PTP_DEVICE_E2E_TC:
        {
            p_usw_ptp_master[lchip]->device_type = CTC_PTP_DEVICE_E2E_TC;
            break;
        }
        case SYS_WB_PTP_DEVICE_P2P_TC:
        {
            p_usw_ptp_master[lchip]->device_type = CTC_PTP_DEVICE_P2P_TC;
            break;
        }
        default:
        {
            p_usw_ptp_master[lchip]->device_type = CTC_PTP_DEVICE_NONE;
            break;
        }
    }

    p_usw_ptp_master[lchip]->tod_1pps_duty = wb_ptp_master.tod_1pps_duty;
    p_usw_ptp_master[lchip]->tod_1pps_delay = wb_ptp_master.tod_1pps_delay;

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    PTP_UNLOCK;

    return ret;
}

#define __3_PTP_TS_CACHE__
STATIC int32
_sys_usw_ptp_add_ts_cache(uint8 lchip, ctc_ptp_captured_type_t type, sys_ptp_capured_ts_t* p_captured_ts)
{
    int32 ret = CTC_E_NONE;
    uint32 index = 0;
    sal_systime_t systime;
    sys_ptp_capured_ts_t* p_ts_node = NULL;
    sys_ptp_capured_ts_t* p_ts_node_get = NULL;
    bool add_cache = TRUE;
    uint8 gchip = 0;
    uint32 gport = 0;

    /*get systime*/
    sal_gettime(&systime);

    p_ts_node = (sys_ptp_capured_ts_t*)mem_malloc(MEM_PTP_MODULE, sizeof(sys_ptp_capured_ts_t));
    if (NULL == p_ts_node)
    {
        SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
	return CTC_E_NO_MEMORY;
    }

    sal_memcpy(p_ts_node, p_captured_ts, sizeof(sys_ptp_capured_ts_t));
    p_ts_node->system_time = systime.tv_sec;

    if (CTC_PTP_CAPTURED_TYPE_RX == type)
    {
        index = (p_ts_node->lport_or_source % MAX_CTC_PTP_TIEM_INTF) * SYS_PTP_MAX_RX_TS_BLOCK_SIZE + p_ts_node->seq_id;
        p_ts_node_get = (sys_ptp_capured_ts_t*)ctc_vector_get(p_usw_ptp_master[lchip]->p_rx_ts_vector, index);
        if (p_ts_node_get != NULL)
        {
            mem_free(p_ts_node_get);
        }

        add_cache = ctc_vector_add(p_usw_ptp_master[lchip]->p_rx_ts_vector, index, p_ts_node);
        if (!add_cache)
        {
            mem_free(p_ts_node);
            ret = CTC_E_NO_MEMORY;
        }
    }
    else if (CTC_PTP_CAPTURED_TYPE_TX == type)
    {
        CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip), ret, error);
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, p_ts_node->lport_or_source);
        index = p_ts_node->seq_id * SYS_PTP_MAX_TX_TS_BLOCK_SIZE + SYS_GET_MAC_ID(lchip, gport);
        p_ts_node_get = (sys_ptp_capured_ts_t*)ctc_vector_get(p_usw_ptp_master[lchip]->p_tx_ts_vector, index);
        if (p_ts_node_get != NULL)
        {
            mem_free(p_ts_node_get);
        }

        add_cache = ctc_vector_add(p_usw_ptp_master[lchip]->p_tx_ts_vector, index, p_ts_node);
        if (!add_cache)
        {
            mem_free(p_ts_node);
            ret = CTC_E_NO_MEMORY;
        }
    }
    else
    {
        mem_free(p_ts_node);
        ret = CTC_E_INVALID_PARAM;
    }

    return ret;

error:
    mem_free(p_ts_node);
    return ret;
}

STATIC int32
_sys_usw_ptp_get_ts_cache(uint8 lchip, ctc_ptp_captured_type_t type, sys_ptp_capured_ts_t* p_captured_ts)
{
    int32 ret = CTC_E_NONE;
    uint32 index = 0;
    sal_systime_t systime;
    sys_ptp_capured_ts_t* p_ts_node = NULL;
    uint8 gchip = 0;
    uint32 gport = 0;

    /*get systime*/
    sal_gettime(&systime);

    if (CTC_PTP_CAPTURED_TYPE_RX == type)
    {
        index = (p_captured_ts->lport_or_source % MAX_CTC_PTP_TIEM_INTF) * SYS_PTP_MAX_RX_TS_BLOCK_SIZE + p_captured_ts->seq_id;
        p_ts_node = (sys_ptp_capured_ts_t*)ctc_vector_get(p_usw_ptp_master[lchip]->p_rx_ts_vector, index);
        if (NULL == p_ts_node)
        {
            return CTC_E_INVALID_CONFIG;
        }

        sal_memcpy(p_captured_ts, p_ts_node, sizeof(sys_ptp_capured_ts_t));
    }
    else if (CTC_PTP_CAPTURED_TYPE_TX == type)
    {
        CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip), ret, error);
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, p_captured_ts->lport_or_source);
        index = p_captured_ts->seq_id * SYS_PTP_MAX_TX_TS_BLOCK_SIZE + SYS_GET_MAC_ID(lchip, gport);
        p_ts_node = (sys_ptp_capured_ts_t*)ctc_vector_get(p_usw_ptp_master[lchip]->p_tx_ts_vector, index);
        if (NULL == p_ts_node)
        {
            return CTC_E_INVALID_CONFIG;
        }

        sal_memcpy(p_captured_ts, p_ts_node, sizeof(sys_ptp_capured_ts_t));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /* the ts get form cache has already time out*/
    if ((systime.tv_sec - p_captured_ts->system_time) >= p_usw_ptp_master[lchip]->cache_aging_time)
    {
	return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;

error:
    return ret;
}

#define __4_PTP_FUNC__
STATIC int32
_sys_usw_ptp_set_cache_aging_time(uint8 lchip, uint32 value)
{
    p_usw_ptp_master[lchip]->cache_aging_time = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_get_cache_aging_time(uint8 lchip, uint32 *value)
{
    *value = p_usw_ptp_master[lchip]->cache_aging_time;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_set_sync_of_tod_input_select(uint8 lchip, uint32 value)
{
    uint32 cmd = 0;
    uint32 field_sync_value = 0;
    uint32 field_tod_value = 0;

    switch (value)
    {
        case CTC_PTP_INTF_SELECT_NONE:
        {
            field_sync_value = 0;
            cmd = DRV_IOW(TsEngineSyncIntfCfg_t, TsEngineSyncIntfCfg_tsCaptureEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_sync_value));

            field_tod_value = 0;
            cmd = DRV_IOW(TsEngineTodCfg_t, TsEngineTodCfg_todTsCaptureEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_tod_value));
            break;
        }
        case CTC_PTP_INTF_SELECT_SYNC:
        {
            field_tod_value = 0;
            cmd = DRV_IOW(TsEngineTodCfg_t, TsEngineTodCfg_todTsCaptureEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_tod_value));

            field_sync_value = 1;
            cmd = DRV_IOW(TsEngineSyncIntfCfg_t, TsEngineSyncIntfCfg_tsCaptureEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_sync_value));
            break;
        }
        case CTC_PTP_INTF_SELECT_TOD:
        {
            field_sync_value = 0;
            cmd = DRV_IOW(TsEngineSyncIntfCfg_t, TsEngineSyncIntfCfg_tsCaptureEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_sync_value));

            field_tod_value = 1;
            cmd = DRV_IOW(TsEngineTodCfg_t, TsEngineTodCfg_todTsCaptureEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_tod_value));
            break;
        }
        default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    p_usw_ptp_master[lchip]->intf_selected = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_get_sync_of_tod_input_select(uint8 lchip, uint32 *value)
{
    *value = p_usw_ptp_master[lchip]->intf_selected;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_get_clock_timestamp(uint8 lchip, ctc_ptp_time_t* timestamp)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 offset_ns = 0;
    uint32 offset_s = 0;
    uint32 rc_s = 0;
    uint32 rc_ns = 0;
    uint32 rc_nns = 0;
    TsEngineRefRc_m ptp_ref_frc;
    TsEngineOffsetAdj_m ptp_offset_adjust;

    /*read frc time*/
    sal_memset(&ptp_ref_frc, 0, sizeof(ptp_ref_frc));
    cmd = DRV_IOR(TsEngineRefRc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_ref_frc), ret, out);

    sal_memset(&ptp_offset_adjust, 0, sizeof(ptp_offset_adjust));
    cmd = DRV_IOR(TsEngineOffsetAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_offset_adjust), ret, out);

    GetTsEngineRefRc(A, rcSecond_f, &ptp_ref_frc, &rc_s);
    GetTsEngineRefRc(A, rcNs_f, &ptp_ref_frc, &rc_ns);
    GetTsEngineRefRc(A, rcFracNs_f, &ptp_ref_frc, &rc_nns);

    GetTsEngineOffsetAdj(A, offsetNs_f, &ptp_offset_adjust, &offset_ns);
    GetTsEngineOffsetAdj(A, offsetSecond_f, &ptp_offset_adjust, &offset_s);

    if ((rc_ns + offset_ns) >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))
    {
        timestamp->seconds = rc_s + offset_s + 1;
        timestamp->nanoseconds = rc_ns + offset_ns - MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE);
        timestamp->nano_nanoseconds = rc_nns;
    }
    else
    {
        timestamp->seconds = rc_s + offset_s;
        timestamp->nanoseconds = rc_ns + offset_ns;
        timestamp->nano_nanoseconds = rc_nns;
    }

    timestamp->is_negative = 0;

    return CTC_E_NONE;

out:
    return ret;
}

/**
 @brief Adjust offset for free running clock
*/
STATIC int32
_sys_usw_ptp_adjust_frc(uint8 lchip, ctc_ptp_time_t* offset)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 offset_ns = 0;
    uint32 offset_s = 0;
    uint32 adj_ns = 0;
    uint32 adj_s = 0;
    TsEngineOffsetAdj_m ptp_offset_adjust;

    sal_memset(&ptp_offset_adjust, 0, sizeof(ptp_offset_adjust));
    cmd = DRV_IOR(TsEngineOffsetAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_offset_adjust));

    GetTsEngineOffsetAdj(A, offsetNs_f, &ptp_offset_adjust, &adj_ns);
    GetTsEngineOffsetAdj(A, offsetSecond_f, &ptp_offset_adjust, &adj_s);

    /*calculate adjust*/
    if (offset->is_negative)  /*offset = adjust - offset*/
    {
        if (adj_ns < offset->nanoseconds) /* not enough for minus */
        {
            offset_ns = adj_ns + MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) - offset->nanoseconds;
            if ((adj_s < offset->seconds) || (adj_s -offset->seconds < 1))
            {
                return CTC_E_INVALID_PARAM;
            }
            offset_s = adj_s - offset->seconds - 1;
        }
        else
        {
            offset_ns = adj_ns - offset->nanoseconds;
            if (adj_s < offset->seconds)
            {
                return CTC_E_INVALID_PARAM;
            }
            offset_s = adj_s - offset->seconds;
        }
    }
    else  /*offset = adjust + offset*/
    {
        if ((offset->nanoseconds + adj_ns) >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))
        {
            offset_ns = offset->nanoseconds + adj_ns - MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE);
            offset_s = offset->seconds + adj_s + 1;
        }
        else
        {
            offset_ns = offset->nanoseconds + adj_ns;
            offset_s = offset->seconds + adj_s;
        }
    }

    SetTsEngineOffsetAdj(V, offsetNs_f, &ptp_offset_adjust, offset_ns);
    SetTsEngineOffsetAdj(V, offsetSecond_f, &ptp_offset_adjust, offset_s);
    CTC_ERROR_RETURN(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_PTP_ADJUST_OFFSET, offset_ns, offset_s));

    /*adjust frc offset*/
    cmd = DRV_IOW(TsEngineOffsetAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_offset_adjust));

    return ret;
}

STATIC int32
_sys_usw_ptp_adjust_clock_offset(uint8 lchip, ctc_ptp_time_t* offset)
{
    int32 ret = CTC_E_NONE;
    ctc_ptp_time_t offset_tmp;
    ctc_ptp_time_t time;
    uint8 offset_adjust_step = 0;
    uint8 offset_adjust_remainder = 0;
    uint8 offset_adjust_num = 0;
    uint8 i = 0;

    /*1. adjust clock offset*/
    if ((offset->seconds != 0) || (offset->nanoseconds > MCHIP_CAP(SYS_CAP_PTP_TOD_ADJUSTING_THRESHOLD))
        || (CTC_PTP_INTF_SELECT_SYNC == p_usw_ptp_master[lchip]->intf_selected))
    {
        /* adjust offset */
        CTC_ERROR_GOTO(_sys_usw_ptp_adjust_frc(lchip, offset), ret, out);

        if (CTC_PTP_INTF_SELECT_SYNC != p_usw_ptp_master[lchip]->intf_selected)
        {
            /* read time*/
            sal_memset(&time, 0, sizeof(time));
            CTC_ERROR_GOTO(_sys_usw_ptp_get_clock_timestamp(lchip, &time), ret, out);

            /* set toggle time*/
            if ((time.nanoseconds + MCHIP_CAP(SYS_CAP_PTP_TOD_ADJUSTING_TOGGLE_STEP)) >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))
            {
                time.seconds++;
                time.nanoseconds = time.nanoseconds + MCHIP_CAP(SYS_CAP_PTP_TOD_ADJUSTING_TOGGLE_STEP) - MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE);
            }
            else
            {
                time.nanoseconds = time.nanoseconds + MCHIP_CAP(SYS_CAP_PTP_TOD_ADJUSTING_TOGGLE_STEP);
            }
            CTC_ERROR_GOTO(_sys_usw_ptp_set_sync_intf_toggle_time(lchip, &time), ret, out);
        }
    }
    else
    {
        offset_adjust_step = p_usw_ptp_master[lchip]->quanta - 1;
        offset_adjust_num = offset->nanoseconds / offset_adjust_step;
        offset_adjust_remainder = offset->nanoseconds % offset_adjust_step;

        sal_memset(&offset_tmp, 0, sizeof(offset_tmp));
        offset_tmp.is_negative = offset->is_negative;
        offset_tmp.seconds = 0;
        offset_tmp.nanoseconds = offset_adjust_step;

        for (i = 0; i < offset_adjust_num; i++)
        {
            CTC_ERROR_GOTO(_sys_usw_ptp_adjust_frc(lchip, &offset_tmp), ret, out);
        }

        offset_tmp.nanoseconds = offset_adjust_remainder;
        CTC_ERROR_GOTO(_sys_usw_ptp_adjust_frc(lchip, &offset_tmp), ret, out);
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_set_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint64 drift_rate = 0;
    TsEngineRefDriftRateAdj_m ptp_drift_rate_adjust;

    /*calculate drift rate,drift rate = drift * quanta*/
    drift_rate = (uint64)p_drift->nanoseconds * p_usw_ptp_master[lchip]->quanta;
    if (drift_rate >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))
    {
        ret = CTC_E_INVALID_PARAM;
        goto out;
    }

    sal_memset(&ptp_drift_rate_adjust, 0, sizeof(ptp_drift_rate_adjust));
    SetTsEngineRefDriftRateAdj(V, sign_f, &ptp_drift_rate_adjust, p_drift->is_negative ? 1 : 0);
    SetTsEngineRefDriftRateAdj(V, driftRate_f, &ptp_drift_rate_adjust, drift_rate & 0x3FFFFFFF);

    /*adjust frc drift */
    cmd = DRV_IOW(TsEngineRefDriftRateAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_drift_rate_adjust), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 drift_rate_ref = 0;
    TsEngineRefDriftRateAdj_m ptp_drift_rate_adjust;

    /*read frc drift*/
    sal_memset(&ptp_drift_rate_adjust, 0, sizeof(ptp_drift_rate_adjust));
    cmd = DRV_IOR(TsEngineRefDriftRateAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_drift_rate_adjust), ret, out);

    /*calculate drift,drift = drift_rate / quanta */
    p_drift->is_negative = GetTsEngineRefDriftRateAdj(V, sign_f, &ptp_drift_rate_adjust);
    p_drift->seconds = 0;
    GetTsEngineRefDriftRateAdj(A, driftRate_f, &ptp_drift_rate_adjust, &drift_rate_ref);
    p_drift->nanoseconds = drift_rate_ref / p_usw_ptp_master[lchip]->quanta;
    p_drift->nano_nanoseconds = 0;

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_set_device_type(uint8 lchip, ctc_ptp_device_type_t device_type)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    DsPtpProfile_m ptp_profile;

    sal_memset(&ptp_profile, 0, sizeof(ptp_profile));

    /*config device type*/
    switch (device_type)
    {
        case CTC_PTP_DEVICE_NONE:
        {
            break;
        }
        case CTC_PTP_DEVICE_OC:
        case CTC_PTP_DEVICE_BC:
        {
            SetDsPtpProfile(V, array_0_exceptionToCpu_f, &ptp_profile, 1); /*sync*/
            SetDsPtpProfile(V, array_0_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_0_applyIngressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_1_exceptionToCpu_f, &ptp_profile, 1); /*delay_req*/
            SetDsPtpProfile(V, array_1_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_1_applyEgressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_exceptionToCpu_f, &ptp_profile, 1); /*peer delay_req*/
            SetDsPtpProfile(V, array_2_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_applyEgressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_prevDiscardObeyException_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_exceptionToCpu_f, &ptp_profile, 1); /*peer delay_resp*/
            SetDsPtpProfile(V, array_3_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_applyIngressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_prevDiscardObeyException_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_8_exceptionToCpu_f, &ptp_profile, 1); /*follow_up*/
            SetDsPtpProfile(V, array_8_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_9_exceptionToCpu_f, &ptp_profile, 1); /*delay_resp*/
            SetDsPtpProfile(V, array_9_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_10_exceptionToCpu_f, &ptp_profile, 1); /*peer delay_resp_follow_up*/
            SetDsPtpProfile(V, array_10_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_10_prevDiscardObeyException_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_11_exceptionToCpu_f, &ptp_profile, 1); /*announce*/
            SetDsPtpProfile(V, array_11_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_12_exceptionToCpu_f, &ptp_profile, 1); /*signaling*/
            SetDsPtpProfile(V, array_12_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_13_exceptionToCpu_f, &ptp_profile, 1); /*management*/
            SetDsPtpProfile(V, array_13_discardPacket_f, &ptp_profile, 1);
            break;
        }
        case CTC_PTP_DEVICE_E2E_TC:
        {
            SetDsPtpProfile(V, array_0_updateResidenceTime_f, &ptp_profile, 1); /*sync*/
            SetDsPtpProfile(V, array_0_applyIngressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_1_updateResidenceTime_f, &ptp_profile, 1); /*delay_req*/
            SetDsPtpProfile(V, array_1_applyEgressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_discardPacket_f, &ptp_profile, 1); /*peer delay_req*/
            SetDsPtpProfile(V, array_3_discardPacket_f, &ptp_profile, 1); /*peer delay_resp*/
            SetDsPtpProfile(V, array_10_discardPacket_f, &ptp_profile, 1); /*peer delay_resp_follow_up*/
            break;
        }
        case CTC_PTP_DEVICE_P2P_TC:
        {
            SetDsPtpProfile(V, array_0_updateResidenceTime_f, &ptp_profile, 1); /*sync*/
            SetDsPtpProfile(V, array_0_applyIngressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_0_applyPathDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_1_updateResidenceTime_f, &ptp_profile, 1); /*delay_req*/
            SetDsPtpProfile(V, array_1_applyEgressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_1_applyPathDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_updateResidenceTime_f, &ptp_profile, 1); /*peer delay_req*/
            SetDsPtpProfile(V, array_2_applyEgressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_applyPathDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_exceptionToCpu_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_2_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_updateResidenceTime_f, &ptp_profile, 1); /*peer delay_resp*/
            SetDsPtpProfile(V, array_3_applyIngressAsymmetryDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_applyPathDelay_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_exceptionToCpu_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_3_discardPacket_f, &ptp_profile, 1);
            SetDsPtpProfile(V, array_10_exceptionToCpu_f, &ptp_profile, 1); /*peer delay_resp_follow_up*/
            SetDsPtpProfile(V, array_10_discardPacket_f, &ptp_profile, 1);
            break;
        }
        default:
        {
            ret = CTC_E_INVALID_PARAM;
            goto out;
        }
    }

    cmd = DRV_IOW(DsPtpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 1, cmd, &ptp_profile), ret, out);

    p_usw_ptp_master[lchip]->device_type = device_type;

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_device_type(uint8 lchip, ctc_ptp_device_type_t* device_type)
{
    *device_type = p_usw_ptp_master[lchip]->device_type;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_add_device_clock(uint8 lchip, ctc_ptp_clock_t* clock)
{
    int32 ret = CTC_E_NONE;
    DsPtpProfile_m ptp_profile;
    uint8  index = 0;
    uint32 cmd = 0;
    int32 step = 0;

    sal_memset(&ptp_profile, 0, sizeof(ptp_profile));
    cmd = DRV_IOR(DsPtpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, clock->clock_id, cmd, &ptp_profile), ret, out);

    step = DsPtpProfile_array_1_discardPacket_f - DsPtpProfile_array_0_discardPacket_f;

    for (index = 0; index < CTC_PTP_MSG_TYPE_MAX; index++)
    {
        if (CTC_FLAG_ISSET(clock->action_flag[index], CTC_PTP_ACTION_FLAG_DISCARD))
        {
            DRV_SET_FIELD_V(lchip, DsPtpProfile_t, DsPtpProfile_array_0_discardPacket_f + (index * step), &ptp_profile, 1);
        }

        if (CTC_FLAG_ISSET(clock->action_flag[index], CTC_PTP_ACTION_FLAG_COPY_TO_CPU))
        {
            DRV_SET_FIELD_V(lchip, DsPtpProfile_t, DsPtpProfile_array_0_exceptionToCpu_f + (index * step), &ptp_profile, 1);
        }

        if (CTC_FLAG_ISSET(clock->action_flag[index], CTC_PTP_ACTION_FLAG_RESIDENCE_TIME))
        {
            DRV_SET_FIELD_V(lchip, DsPtpProfile_t, DsPtpProfile_array_0_updateResidenceTime_f + (index * step), &ptp_profile, 1);
        }

        if (CTC_FLAG_ISSET(clock->action_flag[index], CTC_PTP_ACTION_FLAG_PATH_DELAY))
        {
            DRV_SET_FIELD_V(lchip, DsPtpProfile_t, DsPtpProfile_array_0_applyPathDelay_f + (index * step), &ptp_profile, 1);
        }

        if (CTC_FLAG_ISSET(clock->action_flag[index], CTC_PTP_ACTION_FLAG_IGS_ASYM_DELAY))
        {
            DRV_SET_FIELD_V(lchip, DsPtpProfile_t, DsPtpProfile_array_0_applyIngressAsymmetryDelay_f + (index * step), &ptp_profile, 1);
        }

        if (CTC_FLAG_ISSET(clock->action_flag[index], CTC_PTP_ACTION_FLAG_EGS_ASYM_DELAY))
        {
            DRV_SET_FIELD_V(lchip, DsPtpProfile_t, DsPtpProfile_array_0_applyEgressAsymmetryDelay_f + (index * step), &ptp_profile, 1);
        }
    }

    cmd = DRV_IOW(DsPtpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, clock->clock_id, cmd, &ptp_profile), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_remove_device_clock(uint8 lchip, ctc_ptp_clock_t* clock)
{
    int32 ret = CTC_E_NONE;
    DsPtpProfile_m ptp_profile;
    uint32 cmd = 0;

    sal_memset(&ptp_profile, 0, sizeof(ptp_profile));
    cmd = DRV_IOW(DsPtpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, clock->clock_id, cmd, &ptp_profile), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_set_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64 value)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint8 negtive = 0;
    uint8 chan_id = 0;
    uint32 value64[2];
    DsDestChannel_m dest_channel;
    DsSrcChannel_m src_channel;

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (type)
    {
        case CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY:
        {
            sal_memset(&src_channel, 0, sizeof(src_channel));
            cmd = DRV_IOR(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            SetDsSrcChannel(V, ingressLatency_f, &src_channel, value & 0xFFFFLL);
            cmd = DRV_IOW(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY:
        {
            sal_memset(&dest_channel, 0, sizeof(dest_channel));
            cmd = DRV_IOR(DsDestChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &dest_channel), ret, out);

            SetDsDestChannel(V, egressLatency_f, &dest_channel, value & 0xFFFFLL);
            cmd = DRV_IOW(DsDestChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &dest_channel), ret, out);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY:
        {
            negtive = (value >= 0) ? 0 : 1;
            if (value < 0)
            {
                value = -value;
            }

            sal_memset(&src_channel, 0, sizeof(src_channel));
            cmd = DRV_IOR(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            SetDsSrcChannel(V, asymmetryDelayNegtive_f, &src_channel, negtive);
            value64[1] = (value >> 32) & 0xFLL;
            value64[0] = value & 0xFFFFFFFFLL;
            SetDsSrcChannel(A, asymmetryDelay_f, &src_channel, value64);
            cmd = DRV_IOW(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY:
        {
            negtive = (value >= 0) ? 0 : 1;
            if (value < 0)
            {
                value = -value;
            }

            sal_memset(&dest_channel, 0, sizeof(dest_channel));
            cmd = DRV_IOR(DsDestChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &dest_channel), ret, out);

            SetDsDestChannel(V, asymmetryDelayNegtive_f, &dest_channel, negtive);
            value64[1] = (value >> 32) & 0xFLL;
            value64[0] = value & 0xFFFFFFFFLL;
            SetDsDestChannel(A, asymmetryDelay_f, &dest_channel, value64);
            cmd = DRV_IOW(DsDestChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &dest_channel), ret, out);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_PATH_DELAY:
        {
            sal_memset(&src_channel, 0, sizeof(src_channel));
            cmd = DRV_IOR(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            value64[1] = (value >> 32) & 0xFLL;
            value64[0] = value & 0xFFFFFFFFLL;
            SetDsSrcChannel(A, pathDelay_f, &src_channel, value64);
            cmd = DRV_IOW(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            break;
        }
        default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64* value)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint8 chan_id = 0;
    uint8 negtive = 0;
    DsDestChannel_m dest_channel;
    DsSrcChannel_m src_channel;

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (type)
    {
        case CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY:
        {
            sal_memset(&src_channel, 0, sizeof(src_channel));
            cmd = DRV_IOR(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            GetDsSrcChannel(A, ingressLatency_f, &src_channel, value);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY:
        {
            sal_memset(&dest_channel, 0, sizeof(dest_channel));
            cmd = DRV_IOR(DsDestChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &dest_channel), ret ,out);

            GetDsDestChannel(A, egressLatency_f, &dest_channel, value);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY:
        {
            sal_memset(&src_channel, 0, sizeof(src_channel));
            cmd = DRV_IOR(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            negtive = GetDsSrcChannel(V, asymmetryDelayNegtive_f, &src_channel);
            GetDsSrcChannel(A, asymmetryDelay_f, &src_channel, value);
            if (negtive)
            {
                *value = -(*value);
            }

            break;
        }
        case CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY:
        {
            sal_memset(&dest_channel, 0, sizeof(dest_channel));
            cmd = DRV_IOR(DsDestChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &dest_channel), ret, out);

            negtive = GetDsDestChannel(V, asymmetryDelayNegtive_f, &dest_channel);
            GetDsDestChannel(A, asymmetryDelay_f, &dest_channel, value);
            if (negtive)
            {
                *value = -(*value);
            }

            break;
        }
        case CTC_PTP_ADJUST_DELAY_PATH_DELAY:
        {
            sal_memset(&src_channel, 0, sizeof(src_channel));
            cmd = DRV_IOR(DsSrcChannel_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &src_channel), ret, out);

            GetDsSrcChannel(A, pathDelay_f, &src_channel, value);

            break;
        }
        default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_captured_ts(uint8 lchip, ctc_ptp_captured_type_t type, sys_ptp_capured_ts_t* p_cap_ts)
{
    uint32 cmd = 0;
    uint8 src_bit = 0;
    bool is_fifo_match = FALSE;
    uint8 seq_id = 0;
    uint16 adj_frc_bitmap = 0;
    TsEngineTsCaptureFifoAdjRc_m ptp_captured_adj_frc;
    sys_ptp_capured_ts_t capured_ts;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_cap_ts);

    /*1. read ts from FIFO until the FIFO is empty*/
    if (CTC_PTP_CAPTURED_TYPE_RX == type)
    {
        SYS_PTP_RX_SOURCE_CHECK(p_cap_ts->lport_or_source);
        SYS_PTP_RX_SEQ_ID_CHECK(p_cap_ts->seq_id);

        sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
        cmd = DRV_IOR(TsEngineTsCaptureFifoAdjRc_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));

        adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
        seq_id = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSeqId_f, &ptp_captured_adj_frc);

        sal_memset(&capured_ts, 0, sizeof(capured_ts));
        while (adj_frc_bitmap) /*the FIFO is empty when the bitmap is 0*/
        {
            if ((p_cap_ts->seq_id == seq_id) && ((adj_frc_bitmap >> p_cap_ts->lport_or_source) & 0x1)) /*seq_id and source are all match*/
            {
                p_cap_ts->ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
                p_cap_ts->ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
                is_fifo_match   = TRUE;
            }

            /* add ts to cache*/
            for (src_bit = 0; src_bit < 9; src_bit++) /*get captured source from bitmap*/
            {
                if ((adj_frc_bitmap >> src_bit) & 0x01)
                {
                    break;
                }
            }

            capured_ts.lport_or_source = src_bit;
            capured_ts.seq_id = seq_id;
            capured_ts.ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            capured_ts.ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
            CTC_ERROR_RETURN(_sys_usw_ptp_add_ts_cache(lchip, type, &capured_ts)); /*add ts to cache*/

            sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));
            adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
            seq_id = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSeqId_f, &ptp_captured_adj_frc);
        }
    }
    else if (CTC_PTP_CAPTURED_TYPE_TX == type)
    {
        return CTC_E_INVALID_PARAM;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /*2. if fail to read ts from FIFO, then read ts from cache*/
    if (!is_fifo_match)
    {
        if (_sys_usw_ptp_get_ts_cache(lchip, type, p_cap_ts) != CTC_E_NONE) /*read ts from cache*/
        {
            SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [PTP] Time stamp is not ready \n");
            return CTC_E_HW_FAIL;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_set_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_config)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineSyncIntfCfg_m ptp_sync_intf_cfg;
    TsEngineSyncIntfHalfPeriod_m ptp_sync_intf_half_period;
    TsEngineTsCaptureFifoAdjRc_m ptp_captured_adj_frc;
    uint16 adj_frc_bitmap = 0;

    /*set sync interface as input mode*/
    sal_memset(&ptp_sync_intf_cfg, 0, sizeof(ptp_sync_intf_cfg));
    cmd = DRV_IOR(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret , out);

    SetTsEngineSyncIntfCfg(V, syncIntfMode_f, &ptp_sync_intf_cfg, 0);
    if (p_config->mode)
    {
        SetTsEngineSyncIntfCfg(V, syncPulseOutMask_f, &ptp_sync_intf_cfg, 1);
    }
    cmd = DRV_IOW(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);

    /*set sync interface config*/
    SetTsEngineSyncIntfCfg(V, syncIntfMode_f, &ptp_sync_intf_cfg, p_config->mode ? 1 : 0);
    SetTsEngineSyncIntfCfg(V, lock_f, &ptp_sync_intf_cfg, p_config->lock ? 1 : 0);
    SetTsEngineSyncIntfCfg(V, epoch_f, &ptp_sync_intf_cfg, p_config->epoch);
    SetTsEngineSyncIntfCfg(V, accuracy_f, &ptp_sync_intf_cfg, p_config->accuracy);
    SetTsEngineSyncIntfCfg(V, syncPulseOutPeriod_f, &ptp_sync_intf_cfg, p_config->sync_clock_hz / p_config->sync_pulse_hz);
    SetTsEngineSyncIntfCfg(V, syncPulseOutThreshold_f, &ptp_sync_intf_cfg, p_config->sync_clock_hz / p_config->sync_pulse_hz * p_config->sync_pulse_duty / 100);
    cmd = DRV_IOW(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);

    /*calculate syncClock half period and syncPulse period&duty*/
    sal_memset(&ptp_sync_intf_half_period, 0, sizeof(ptp_sync_intf_half_period));
    SetTsEngineSyncIntfHalfPeriod(V, halfPeriodNs_f, &ptp_sync_intf_half_period, MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) / (p_config->sync_clock_hz * 2));
    cmd = DRV_IOW(TsEngineSyncIntfHalfPeriod_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_half_period), ret, out);

    /*for input mode need clear first*/
    if (!p_config->mode)
    {
        sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
        cmd = DRV_IOR(TsEngineTsCaptureFifoAdjRc_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc), ret, out);

        adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
        while (adj_frc_bitmap) /*the FIFO is empty when the bitmap is 0*/
        {
            sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc), ret, out);
            adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
        }
    }
    else
    {
        SetTsEngineSyncIntfCfg(V, syncPulseOutMask_f, &ptp_sync_intf_cfg, 0);
        cmd = DRV_IOW(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_config)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 half_period_ns = 0;
    uint32 sync_pulse_out_period = 0;
    uint32 sync_pulse_out_threshold = 0;
    TsEngineSyncIntfCfg_m ptp_sync_intf_cfg;
    TsEngineSyncIntfHalfPeriod_m ptp_sync_intf_half_period;

    /*set sync interface config*/
    sal_memset(&ptp_sync_intf_cfg, 0, sizeof(ptp_sync_intf_cfg));
    cmd = DRV_IOR(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);

    sal_memset(&ptp_sync_intf_half_period, 0, sizeof(ptp_sync_intf_half_period));
    cmd = DRV_IOR(TsEngineSyncIntfHalfPeriod_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_half_period), ret, out);

    p_config->mode = GetTsEngineSyncIntfCfg(V, syncIntfMode_f, &ptp_sync_intf_cfg);
    p_config->lock = GetTsEngineSyncIntfCfg(V, lock_f, &ptp_sync_intf_cfg);
    p_config->epoch = GetTsEngineSyncIntfCfg(V, epoch_f, &ptp_sync_intf_cfg);
    p_config->accuracy = GetTsEngineSyncIntfCfg(V, accuracy_f, &ptp_sync_intf_cfg);
    GetTsEngineSyncIntfCfg(A, syncPulseOutPeriod_f, &ptp_sync_intf_cfg, &sync_pulse_out_period);
    GetTsEngineSyncIntfCfg(A, syncPulseOutThreshold_f, &ptp_sync_intf_cfg, &sync_pulse_out_threshold);
    GetTsEngineSyncIntfHalfPeriod(A, halfPeriodNs_f, &ptp_sync_intf_half_period, &half_period_ns);

    if (0 == half_period_ns)
    {
        p_config->sync_clock_hz = 0;
    }
    else
    {
        p_config->sync_clock_hz = MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) / (half_period_ns * 2);
    }

    if (0 == sync_pulse_out_period)
    {
        p_config->sync_pulse_hz = 0;
        p_config->sync_pulse_duty = 0;
    }
    else
    {
        p_config->sync_pulse_hz = p_config->sync_clock_hz / sync_pulse_out_period;
        p_config->sync_pulse_duty = (sync_pulse_out_threshold * 100) / sync_pulse_out_period;
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_set_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32  value = 0;
    TsEngineSyncIntfToggle_m ptp_sync_intf_toggle;
    TsEngineSyncIntfCfg_m ptp_sync_intf_cfg;

    /*set sync interface as input mode*/
    sal_memset(&ptp_sync_intf_cfg, 0, sizeof(ptp_sync_intf_cfg));
    cmd = DRV_IOR(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);

    GetTsEngineSyncIntfCfg(A, syncIntfMode_f, &ptp_sync_intf_cfg, &value);
    SetTsEngineSyncIntfCfg(V, syncIntfMode_f, &ptp_sync_intf_cfg, 0);
    cmd = DRV_IOW(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);

    /*set toggle time*/
    sal_memset(&ptp_sync_intf_toggle, 0, sizeof(ptp_sync_intf_toggle));
    SetTsEngineSyncIntfToggle(V, toggleSecond_f, &ptp_sync_intf_toggle, p_toggle_time->seconds);
    SetTsEngineSyncIntfToggle(V, toggleNs_f, &ptp_sync_intf_toggle, p_toggle_time->nanoseconds);
    SetTsEngineSyncIntfToggle(V, toggleFracNs_f, &ptp_sync_intf_toggle, p_toggle_time->nano_nanoseconds);
    cmd = DRV_IOW(TsEngineSyncIntfToggle_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_toggle), ret, out);

    /*set sync interface as output mode*/
    SetTsEngineSyncIntfCfg(V, syncIntfMode_f, &ptp_sync_intf_cfg, value);
    cmd = DRV_IOW(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineSyncIntfToggle_m ptp_sync_intf_toggle;

    /*read toggle time*/
    sal_memset(&ptp_sync_intf_toggle, 0, sizeof(ptp_sync_intf_toggle));
    cmd = DRV_IOR(TsEngineSyncIntfToggle_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_toggle), ret, out);

    GetTsEngineSyncIntfToggle(A, toggleSecond_f, &ptp_sync_intf_toggle, &p_toggle_time->seconds);
    GetTsEngineSyncIntfToggle(A, toggleNs_f, &ptp_sync_intf_toggle, &p_toggle_time->nanoseconds);
    GetTsEngineSyncIntfToggle(A, toggleFracNs_f, &ptp_sync_intf_toggle, &p_toggle_time->nano_nanoseconds);
    p_toggle_time->is_negative = 0;

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_sync_intf_code(uint8 lchip, ctc_ptp_sync_intf_code_t* g_time_code)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint8 sync_code_in_rdy = 0;
    uint32 data88_to64 = 0;
    uint32 data63_to32 = 0;
    uint32 data31_to0 = 0;
    TsEngineSyncIntfInputCode_m input_code;

    sal_memset(&input_code, 0, sizeof(input_code));
    cmd = DRV_IOR(TsEngineSyncIntfInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &input_code), ret, out);

    GetTsEngineSyncIntfInputCode(A, syncCodeInRdy_f, &input_code, &sync_code_in_rdy);
    if (1 != sync_code_in_rdy) /*timecode has received already*/
    {
        ret = CTC_E_HW_FAIL;
        goto out;
    }

    /*get timecode*/
    GetTsEngineSyncIntfInputCode(A, data88To64_f, &input_code, &data88_to64);
    GetTsEngineSyncIntfInputCode(A, data63To32_f, &input_code, &data63_to32);
    GetTsEngineSyncIntfInputCode(A, data31To0_f, &input_code, &data31_to0);

    g_time_code->crc_error = GetTsEngineSyncIntfInputCode(V, crcErr_f, &input_code);
    g_time_code->crc = GetTsEngineSyncIntfInputCode(V, crc_f, &input_code);
    g_time_code->seconds = (data88_to64 & 0xffffff) << 24 | (data63_to32 & 0xffffff00) >> 8;
    g_time_code->nanoseconds = (data63_to32 & 0xff) << 24 | (data31_to0 & 0xffffff00) >> 8;
    g_time_code->lock = data88_to64 >> 24;
    g_time_code->accuracy = data31_to0 & 0xff;

    /*clear ready flag*/
    SetTsEngineSyncIntfInputCode(V, syncCodeInRdy_f, &input_code, 0);
    cmd = DRV_IOW(TsEngineSyncIntfInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &input_code), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_clear_sync_intf_code(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineSyncIntfInputCode_m input_code;

    /*clear ready flag,clear intf_code.sync_code_in_rdy*/
    sal_memset(&input_code, 0, sizeof(input_code));
    cmd = DRV_IOW(TsEngineSyncIntfInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &input_code), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_set_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* config)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineTodCfg_m ptp_tod_cfg;
    TsEngineTsCaptureFifoAdjRc_m ptp_captured_adj_frc;
    uint16 coreclock = 0;
    uint16 adj_frc_bitmap = 0;
    uint32 out_delay = 0;

#if 0
    /*set GPIO 12, 13 use as tod pin*/
    sys_usw_chip_set_gpio_mode(lchip, 12, CTC_CHIP_SPECIAL_MODE);
    sys_usw_chip_set_gpio_mode(lchip, 13, CTC_CHIP_SPECIAL_MODE);
#endif

    /*set ToD interface config*/
    sal_memset(&ptp_tod_cfg, 0, sizeof(ptp_tod_cfg));
    cmd = DRV_IOR(TsEngineTodCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_cfg), ret, out);
    if (config->mode)
    {
        SetTsEngineTodCfg(V, todPulseOutMask_f, &ptp_tod_cfg, 1);

        if (config->pulse_delay >= 8)
        {
            out_delay = (MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) -(config->pulse_delay)) >> 3;
        }
        SetTsEngineTodCfg(V, todPulseOutDelay_f, &ptp_tod_cfg, out_delay); /* pre output 8*(125000000-todPulseOutDelay)ns */
    }
    else
    {
        p_usw_ptp_master[lchip]->tod_1pps_delay = config->pulse_delay;
    }

    cmd = DRV_IOW(TsEngineTodCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_cfg), ret, out);

    SetTsEngineTodCfg(V, todIntfMode_f, &ptp_tod_cfg, config->mode ? 1 : 0);
    SetTsEngineTodCfg(V, todPulseOutEnable_f, &ptp_tod_cfg, config->mode ? 1 : 0);
    SetTsEngineTodCfg(V, todTsCaptureEn_f, &ptp_tod_cfg, config->mode ? 0 : 1);
    SetTsEngineTodCfg(V, todCodeOutEnable_f, &ptp_tod_cfg, config->mode ? 1 : 0);
    SetTsEngineTodCfg(V, todTxStopBitNum_f, &ptp_tod_cfg, config->stop_bit_num);
    SetTsEngineTodCfg(V, todEpoch_f, &ptp_tod_cfg, config->epoch);

    /*1ms= todPulseOutHighPeriod*512/clockCore(M)/1000*/
    CTC_ERROR_GOTO(sys_usw_get_chip_clock(lchip, &coreclock), ret, out);
    SetTsEngineTodCfg(V, todPulseOutHighPeriod_f, &ptp_tod_cfg, (coreclock * 10000 * config->pulse_duty) >> 9);
    cmd = DRV_IOW(TsEngineTodCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_cfg), ret, out);

    p_usw_ptp_master[lchip]->tod_1pps_duty = config->pulse_duty;

    /*for input mode need clear first*/
    if (!config->mode)
    {
        sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
        cmd = DRV_IOR(TsEngineTsCaptureFifoAdjRc_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc), ret, out);

        adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
        while (adj_frc_bitmap) /*the FIFO is empty when the bitmap is 0*/
        {
            sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc), ret, out);
            adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
        }
    }
    else
    {
        SetTsEngineTodCfg(V, todPulseOutMask_f, &ptp_tod_cfg, 0);
        cmd = DRV_IOW(TsEngineTodCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_cfg), ret, out);
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* config)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineTodCfg_m ptp_tod_cfg;

    /*get ToD interface config*/
    sal_memset(&ptp_tod_cfg, 0, sizeof(ptp_tod_cfg));
    cmd = DRV_IOR(TsEngineTodCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_cfg), ret, out);

    config->mode = GetTsEngineTodCfg(V, todIntfMode_f, &ptp_tod_cfg);
    config->stop_bit_num = GetTsEngineTodCfg(V, todTxStopBitNum_f, &ptp_tod_cfg);
    config->epoch = GetTsEngineTodCfg(V, todEpoch_f, &ptp_tod_cfg);
    config->pulse_duty = p_usw_ptp_master[lchip]->tod_1pps_duty;
    if (config->mode)
    {
        config->pulse_delay = GetTsEngineTodCfg(V, todPulseOutDelay_f, &ptp_tod_cfg);
        if (config->pulse_delay)
        {
            config->pulse_delay = MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) - ((config->pulse_delay) << 3);
        }
    }
    else
    {
        config->pulse_delay = p_usw_ptp_master[lchip]->tod_1pps_delay;
    }

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_set_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* tx_code)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineTodCodeCfg_m ptp_tod_code_cfg;

    sal_memset(&ptp_tod_code_cfg, 0, sizeof(ptp_tod_code_cfg));
    SetTsEngineTodCodeCfg(V, todClockSrc_f, &ptp_tod_code_cfg, tx_code->clock_src);
    SetTsEngineTodCodeCfg(V, todClockSrcStatus_f, &ptp_tod_code_cfg, tx_code->clock_src_status);
    SetTsEngineTodCodeCfg(V, todLeapS_f, &ptp_tod_code_cfg, tx_code->leap_second);
    SetTsEngineTodCodeCfg(V, todMonitorAlarm_f, &ptp_tod_code_cfg, tx_code->monitor_alarm);
    SetTsEngineTodCodeCfg(V, todMsgClass_f, &ptp_tod_code_cfg, tx_code->msg_class);
    SetTsEngineTodCodeCfg(V, todMsgId_f, &ptp_tod_code_cfg, tx_code->msg_id);
    SetTsEngineTodCodeCfg(V, todPayloadLen_f, &ptp_tod_code_cfg, tx_code->msg_length);
    SetTsEngineTodCodeCfg(V, todPulseStatus_f, &ptp_tod_code_cfg, tx_code->pps_status);
    SetTsEngineTodCodeCfg(V, todTacc_f, &ptp_tod_code_cfg, tx_code->pps_accuracy);

    cmd = DRV_IOW(TsEngineTodCodeCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_code_cfg), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* tx_code)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineTodCodeCfg_m ptp_tod_code_cfg;

    sal_memset(&ptp_tod_code_cfg, 0, sizeof(ptp_tod_code_cfg));
    cmd = DRV_IOR(TsEngineTodCodeCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_code_cfg), ret, out);

    tx_code->clock_src = GetTsEngineTodCodeCfg(V, todClockSrc_f, &ptp_tod_code_cfg);
    tx_code->clock_src_status = GetTsEngineTodCodeCfg(V, todClockSrcStatus_f, &ptp_tod_code_cfg);
    tx_code->leap_second = GetTsEngineTodCodeCfg(V, todLeapS_f, &ptp_tod_code_cfg);
    tx_code->monitor_alarm = GetTsEngineTodCodeCfg(V, todMonitorAlarm_f, &ptp_tod_code_cfg);
    tx_code->msg_class = GetTsEngineTodCodeCfg(V, todMsgClass_f, &ptp_tod_code_cfg);
    tx_code->msg_id = GetTsEngineTodCodeCfg(V, todMsgId_f, &ptp_tod_code_cfg);
    tx_code->msg_length = GetTsEngineTodCodeCfg(V, todPayloadLen_f, &ptp_tod_code_cfg);
    tx_code->pps_status = GetTsEngineTodCodeCfg(V, todPulseStatus_f, &ptp_tod_code_cfg);
    tx_code->pps_accuracy = GetTsEngineTodCodeCfg(V, todTacc_f, &ptp_tod_code_cfg);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_get_tod_intf_rx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* rx_code)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint8 tod_code_in_rdy = 0;
    uint8 tod_rcv_msg_byte2 = 0;
    uint8 tod_rcv_msg_byte3 = 0;
    uint8 tod_rcv_msg_byte4 = 0;
    uint8 tod_rcv_msg_byte5 = 0;
    uint8 tod_rcv_msg_byte6 = 0;
    uint8 tod_rcv_msg_byte7 = 0;
    uint8 tod_rcv_msg_byte12 = 0;
    uint8 tod_rcv_msg_byte13 = 0;
    TsEngineTodInputCode_m input_code;

    sal_memset(&input_code, 0, sizeof(input_code));
    cmd = DRV_IOR(TsEngineTodInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &input_code), ret, out);

    GetTsEngineTodInputCode(A, todCodeInRdy_f, &input_code, &tod_code_in_rdy);
    if (1 != tod_code_in_rdy) /*message has received already*/
    {
        ret = CTC_E_HW_FAIL;
        goto out;
    }

    /*get timecode*/
    rx_code->msg_char0 = 'C';
    rx_code->msg_char1 = 'M';
    rx_code->msg_class = GetTsEngineTodInputCode(V, todRcvMsgByte0_f, &input_code);
    rx_code->msg_id = GetTsEngineTodInputCode(V, todRcvMsgByte1_f, &input_code);

    tod_rcv_msg_byte2 = GetTsEngineTodInputCode(V, todRcvMsgByte2_f, &input_code);
    tod_rcv_msg_byte3 = GetTsEngineTodInputCode(V, todRcvMsgByte3_f, &input_code);
    rx_code->msg_length = (tod_rcv_msg_byte2 << 8) + (tod_rcv_msg_byte3);

    tod_rcv_msg_byte4 = GetTsEngineTodInputCode(V, todRcvMsgByte4_f, &input_code);
    tod_rcv_msg_byte5 = GetTsEngineTodInputCode(V, todRcvMsgByte5_f, &input_code);
    tod_rcv_msg_byte6 = GetTsEngineTodInputCode(V, todRcvMsgByte6_f, &input_code);
    tod_rcv_msg_byte7 = GetTsEngineTodInputCode(V, todRcvMsgByte7_f, &input_code);
    rx_code->gps_second_time_of_week = (tod_rcv_msg_byte4 << 24) + (tod_rcv_msg_byte5 << 16) +
                                                                    (tod_rcv_msg_byte6 << 8) + (tod_rcv_msg_byte7);

    tod_rcv_msg_byte12 = GetTsEngineTodInputCode(V, todRcvMsgByte12_f, &input_code);
    tod_rcv_msg_byte13 = GetTsEngineTodInputCode(V, todRcvMsgByte13_f, &input_code);
    rx_code->gps_week = (tod_rcv_msg_byte12 << 8) + (tod_rcv_msg_byte13);

    rx_code->leap_second = GetTsEngineTodInputCode(V, todRcvMsgByte14_f, &input_code);
    rx_code->pps_status = GetTsEngineTodInputCode(V, todRcvMsgByte15_f, &input_code);
    rx_code->pps_accuracy = GetTsEngineTodInputCode(V, todRcvMsgByte16_f, &input_code);
    rx_code->crc = GetTsEngineTodInputCode(V, todRcvMsgCrc_f, &input_code);
    rx_code->crc_error = GetTsEngineTodInputCode(V, todRcvMsgCrcErr_f, &input_code);

    /*clear ready flag*/
    SetTsEngineTodInputCode(V, todCodeInRdy_f, &input_code, 0);
    cmd = DRV_IOW(TsEngineTodInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &input_code), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

STATIC int32
_sys_usw_ptp_clear_tod_intf_code(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    TsEngineTodInputCode_m input_code;

    /*clear ready flag,clear intf_code.tod_code_in_rdy*/
    sal_memset(&input_code, 0, sizeof(input_code));
    cmd = DRV_IOW(TsEngineTodInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &input_code), ret, out);

    return CTC_E_NONE;

out:
    return ret;
}

#define __5_PTP_ISR__
/**
 @brief  PTP RX Ts capture isr
*/
int32
sys_usw_ptp_isr_rx_ts_capture(uint8 lchip, uint32 intr, void* p_data)
{
    uint8 gchip = 0;
    uint32 cmd = 0;
    uint8 src_bit = 0;
    uint16 adj_frc_bitmap = 0;
    TsEngineTsCaptureFifoAdjRc_m ptp_captured_adj_frc;
    ctc_ptp_time_t captured_ts;
    sys_ptp_capured_ts_t sys_capured_ts;
    ctc_ptp_rx_ts_message_t rx_message;
    CTC_INTERRUPT_EVENT_FUNC cb = NULL;

    /*check*/
    SYS_PTP_INIT_CHECK();

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
    cmd = DRV_IOR(TsEngineTsCaptureFifoAdjRc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));

    sal_memset(&captured_ts, 0, sizeof(captured_ts));
    sal_memset(&sys_capured_ts, 0, sizeof(sys_capured_ts));
    sal_memset(&rx_message, 0, sizeof(rx_message));
    adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
    while (adj_frc_bitmap) /*the FIFO is empty when the bitmap is 0*/
    {
        for (src_bit = 0; src_bit < 9; src_bit++) /*get captured source from bitmap*/
        {
            if ((adj_frc_bitmap >> src_bit) & 0x01)
            {
                break;
            }
        }

        if ((src_bit != SYS_PTP_TIEM_INTF_SYNC_PULSE) && (src_bit != SYS_PTP_TIEM_INTF_TOD_1PPS))
        {
            CTC_ERROR_RETURN(_sys_usw_ptp_map_captured_src(&(rx_message.source), &src_bit, 0));
            rx_message.captured_ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            rx_message.captured_ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);

            /* call Users callback*/
            CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PTP_INPUT_CODE_READY, &cb));
            if (cb)
            {
                (void)cb(gchip, &rx_message);
            }
        }
        else  /* not mactch source, add ts to cache*/
        {
            sys_capured_ts.lport_or_source = src_bit;
            sys_capured_ts.seq_id = 0;
            sys_capured_ts.ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            sys_capured_ts.ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
            CTC_ERROR_RETURN(_sys_usw_ptp_add_ts_cache(lchip, CTC_PTP_CAPTURED_TYPE_RX, &sys_capured_ts)); /*add ts to cache*/
        }

        sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));
        adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
    }

    return CTC_E_NONE;
}

/**
 @brief  PTP TOD 1PPS in isr
*/
int32
sys_usw_ptp_isr_tod_pulse_in(uint8 lchip, uint32 intr, void* p_data)
{
    return CTC_E_NONE;
}

/**
 @brief  PTP TOD code ready isr
*/
int32
sys_usw_ptp_isr_tod_code_rdy(uint8 lchip, uint32 intr, void* p_data)
{
    uint8 gchip = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint8 is_match = 0;
    uint8 src_bit = 0;
    uint8 tod_rcv_msg_byte2 = 0;
    uint8 tod_rcv_msg_byte3 = 0;
    uint8 tod_rcv_msg_byte4 = 0;
    uint8 tod_rcv_msg_byte5 = 0;
    uint8 tod_rcv_msg_byte6 = 0;
    uint8 tod_rcv_msg_byte7 = 0;
    uint8 tod_rcv_msg_byte12 = 0;
    uint8 tod_rcv_msg_byte13 = 0;
    uint16 adj_frc_bitmap = 0;
    ctc_ptp_rx_ts_message_t tod_message;
    TsEngineTodInputCode_m input_code;
    TsEngineTsCaptureFifoAdjRc_m ptp_captured_adj_frc;
    ctc_ptp_time_t offset;
    ctc_ptp_time_t input_ts;
    ctc_ptp_time_t captured_ts;
    sys_ptp_capured_ts_t sys_capured_ts;
    CTC_INTERRUPT_EVENT_FUNC cb = NULL;

    SYS_PTP_INIT_CHECK();

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    /* 1. read input code */
    sal_memset(&input_code, 0, sizeof(input_code));
    cmd = DRV_IOR(TsEngineTodInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &input_code));
    if (1 != GetTsEngineTodInputCode(V, todCodeInRdy_f, &input_code)) /*message has received already*/
    {
        return CTC_E_NONE;
    }

    /*get timecode*/
    tod_rcv_msg_byte2 = GetTsEngineTodInputCode(V, todRcvMsgByte2_f, &input_code);
    tod_rcv_msg_byte3 = GetTsEngineTodInputCode(V, todRcvMsgByte3_f, &input_code);
    tod_rcv_msg_byte4 = GetTsEngineTodInputCode(V, todRcvMsgByte4_f, &input_code);
    tod_rcv_msg_byte5 = GetTsEngineTodInputCode(V, todRcvMsgByte5_f, &input_code);
    tod_rcv_msg_byte6 = GetTsEngineTodInputCode(V, todRcvMsgByte6_f, &input_code);
    tod_rcv_msg_byte7 = GetTsEngineTodInputCode(V, todRcvMsgByte7_f, &input_code);
    tod_rcv_msg_byte12 = GetTsEngineTodInputCode(V, todRcvMsgByte12_f, &input_code);
    tod_rcv_msg_byte13 = GetTsEngineTodInputCode(V, todRcvMsgByte13_f, &input_code);

    sal_memset(&tod_message, 0, sizeof(tod_message));
    tod_message.source = CTC_PTP_TIEM_INTF_TOD_1PPS;

    tod_message.u.tod_intf_message.msg_char0 = 'C';
    tod_message.u.tod_intf_message.msg_char1 = 'M';
    tod_message.u.tod_intf_message.msg_class = GetTsEngineTodInputCode(V, todRcvMsgByte0_f, &input_code);
    tod_message.u.tod_intf_message.msg_id = GetTsEngineTodInputCode(V, todRcvMsgByte1_f, &input_code);
    tod_message.u.tod_intf_message.msg_length = (tod_rcv_msg_byte2 << 8) + (tod_rcv_msg_byte3);
    tod_message.u.tod_intf_message.gps_second_time_of_week = (tod_rcv_msg_byte4 << 24) + (tod_rcv_msg_byte5 << 16)
                                                                                                        + (tod_rcv_msg_byte6 << 8) + (tod_rcv_msg_byte7);
    tod_message.u.tod_intf_message.gps_week = (tod_rcv_msg_byte12 << 8) + (tod_rcv_msg_byte13);
    tod_message.u.tod_intf_message.leap_second = GetTsEngineTodInputCode(V, todRcvMsgByte14_f, &input_code);
    tod_message.u.tod_intf_message.pps_status = GetTsEngineTodInputCode(V, todRcvMsgByte15_f, &input_code);
    tod_message.u.tod_intf_message.pps_accuracy = GetTsEngineTodInputCode(V, todRcvMsgByte16_f, &input_code);
    tod_message.u.tod_intf_message.crc = GetTsEngineTodInputCode(V, todRcvMsgCrc_f, &input_code);
    tod_message.u.tod_intf_message.crc_error = GetTsEngineTodInputCode(V, todRcvMsgCrcErr_f, &input_code);

    sal_memset(&input_ts, 0, sizeof(input_ts));
    input_ts.seconds = tod_message.u.tod_intf_message.gps_week * MCHIP_CAP(SYS_CAP_PTP_SECONDS_OF_EACH_WEEK) + MCHIP_CAP(SYS_CAP_PTP_TAI_TO_GPS_SECONDS)
                                + tod_message.u.tod_intf_message.gps_second_time_of_week;

    /* 2. read captured ts from fifo*/
    is_match = 0;
    sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
    cmd = DRV_IOR(TsEngineTsCaptureFifoAdjRc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));

    sal_memset(&captured_ts, 0, sizeof(captured_ts));
    sal_memset(&sys_capured_ts, 0, sizeof(sys_capured_ts));
    adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
    while (adj_frc_bitmap) /*the FIFO is empty when the bitmap is 0*/
    {
        for (src_bit = 0; src_bit < 9; src_bit++) /*get captured source from bitmap*/
        {
            if ((adj_frc_bitmap >> src_bit) & 0x01)
            {
                break;
            }
        }

        if ((src_bit == SYS_PTP_TIEM_INTF_TOD_1PPS) && (!is_match)) /* source match*/
        {
            captured_ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            captured_ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
            tod_message.captured_ts.seconds = captured_ts.seconds;
            tod_message.captured_ts.nanoseconds = captured_ts.nanoseconds;
            is_match = 1;
        }
        else if (src_bit != SYS_PTP_TIEM_INTF_SYNC_PULSE)
        {
            CTC_ERROR_RETURN(_sys_usw_ptp_map_captured_src(&(tod_message.source), &src_bit, 0));
            tod_message.captured_ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            tod_message.captured_ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);

            CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PTP_INPUT_CODE_READY, &cb));
            if (cb)
            {
                (void)cb(gchip, &tod_message);
            }
        }
        else
        {
            sys_capured_ts.lport_or_source = src_bit;
            sys_capured_ts.seq_id = 0;
            sys_capured_ts.ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            sys_capured_ts.ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
            CTC_ERROR_RETURN(_sys_usw_ptp_add_ts_cache(lchip, CTC_PTP_CAPTURED_TYPE_RX, &sys_capured_ts)); /*add ts to cache*/
        }

        sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));
        adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
    }

    if (!is_match)
    {
        sys_capured_ts.lport_or_source = SYS_PTP_TIEM_INTF_TOD_1PPS;
        sys_capured_ts.seq_id = 0;

        if (_sys_usw_ptp_get_ts_cache(lchip, CTC_PTP_CAPTURED_TYPE_RX, &sys_capured_ts) != CTC_E_NONE) /*read ts from cache*/
        {
            return CTC_E_NOT_READY;
        }
        else
        {
            captured_ts.seconds = sys_capured_ts.ts.seconds;
            captured_ts.nanoseconds = sys_capured_ts.ts.nanoseconds;
            tod_message.captured_ts.seconds = sys_capured_ts.ts.seconds;
            tod_message.captured_ts.nanoseconds = sys_capured_ts.ts.nanoseconds;
            is_match = 1;
        }
    }

    if (is_match)
    {
        /* 3. calculate offset */
        if (p_usw_ptp_master[lchip]->tod_1pps_delay >= MCHIP_CAP(SYS_CAP_PTP_TOD_1PPS_DELAY))  /* tod_1pps_delay value bit 31 means delay < 0 */
        {
            if ((p_usw_ptp_master[lchip]->tod_1pps_delay - MCHIP_CAP(SYS_CAP_PTP_TOD_1PPS_DELAY) + captured_ts.nanoseconds) > MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))/*1pps delay between master and slave*/
            {
                captured_ts.seconds = captured_ts.seconds + 1;
                captured_ts.nanoseconds = p_usw_ptp_master[lchip]->tod_1pps_delay - MCHIP_CAP(SYS_CAP_PTP_TOD_1PPS_DELAY) + captured_ts.nanoseconds - MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE);
            }
            else
            {
                captured_ts.nanoseconds = p_usw_ptp_master[lchip]->tod_1pps_delay - MCHIP_CAP(SYS_CAP_PTP_TOD_1PPS_DELAY) + captured_ts.nanoseconds;
            }
        }
        else
        {
            if (p_usw_ptp_master[lchip]->tod_1pps_delay > captured_ts.nanoseconds) /*1pps delay between master and slave*/
            {
                captured_ts.seconds = captured_ts.seconds ? (captured_ts.seconds - 1) : 0;
                captured_ts.nanoseconds = MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) + captured_ts.nanoseconds - p_usw_ptp_master[lchip]->tod_1pps_delay;
            }
            else
            {
                captured_ts.nanoseconds = captured_ts.nanoseconds - p_usw_ptp_master[lchip]->tod_1pps_delay;
            }
        }

        if (input_ts.seconds > captured_ts.seconds) /*TS from ToD is second, no nano second*/
        {
            sal_memset(&offset, 0, sizeof(offset));
            offset.is_negative = 0;
            offset.seconds = input_ts.seconds - captured_ts.seconds;
            if (0 == captured_ts.nanoseconds)
            {
                offset.nanoseconds = 0;
            }
            else
            {
                offset.seconds = offset.seconds ? (offset.seconds - 1) : 0;
                offset.nanoseconds = MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) - captured_ts.nanoseconds;
            }
        }
        else
        {
            sal_memset(&offset, 0, sizeof(offset));
            offset.is_negative = 1;
            offset.seconds = captured_ts.seconds - input_ts.seconds;
            offset.nanoseconds = captured_ts.nanoseconds;
        }

        /* 4. adjust offset */
        if ((0x01 == tod_message.u.tod_intf_message.msg_class) && (0x20 == tod_message.u.tod_intf_message.msg_id ))/*time message*/
        {
            ret = _sys_usw_ptp_adjust_clock_offset(lchip, &offset);
            if (CTC_E_NONE != ret)
            {
                /*if adjust error need clear code rdy*/
                goto Err;
            }
        }

        /* 5. call Users callback to return code message */
        CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PTP_INPUT_CODE_READY, &cb));
        if (cb)
        {
            (void)cb(gchip, &tod_message);
        }
    }

Err:
     /* 6. clear ready flag*/
    SetTsEngineTodInputCode(V, todCodeInRdy_f, &input_code, 0);
    cmd = DRV_IOW(TsEngineTodInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &input_code));

    return CTC_E_NONE;
}

/**
 @brief  PTP syncPulse in isr
*/
int32
sys_usw_ptp_isr_sync_pulse_in(uint8 lchip, uint32 intr, void* p_data)
{
    return CTC_E_NONE;
}

/**
 @brief  PTP Sync Interface code ready isr
*/
int32
sys_usw_ptp_isr_sync_code_rdy(uint8 lchip, uint32 intr, void* p_data)
{
    uint8 gchip = 0;
    uint8 is_match = 0;
    uint8 src_bit = 0;
    uint16 adj_frc_bitmap = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 data88_to64 = 0;
    uint32 data63_to32 = 0;
    uint32 data31_to0 = 0;
    ctc_ptp_rx_ts_message_t sync_message;
    TsEngineSyncIntfInputCode_m input_code;
    TsEngineTsCaptureFifoAdjRc_m ptp_captured_adj_frc;
    ctc_ptp_time_t input_ts;
    ctc_ptp_time_t offset;
    ctc_ptp_time_t captured_ts;
    sys_ptp_capured_ts_t sys_capured_ts;
    uint64 input_nanoseconds = 0;
    uint64 captured_nanoseconds = 0;
    uint64 offset_nanoseconds = 0;
    CTC_INTERRUPT_EVENT_FUNC cb = NULL;

    SYS_PTP_INIT_CHECK();

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    /* 1. read input code */
    sal_memset(&input_code, 0, sizeof(input_code));
    cmd = DRV_IOR(TsEngineSyncIntfInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &input_code));
    if (1 != GetTsEngineSyncIntfInputCode(V, syncCodeInRdy_f, &input_code)) /*timecode has received already*/
    {
        return CTC_E_NONE;
    }

    /*get timecode*/
    GetTsEngineSyncIntfInputCode(A, data88To64_f, &input_code, &data88_to64);
    GetTsEngineSyncIntfInputCode(A, data63To32_f, &input_code, &data63_to32);
    GetTsEngineSyncIntfInputCode(A, data31To0_f, &input_code, &data31_to0);

    sal_memset(&sync_message, 0, sizeof(sync_message));
    sync_message.source = CTC_PTP_TIEM_INTF_SYNC_PULSE;

    sync_message.u.sync_intf_message.seconds = (data88_to64 & 0xffffff) << 24 | (data63_to32 & 0xffffff00) >> 8;
    sync_message.u.sync_intf_message.nanoseconds = (data63_to32 & 0xff) << 24 | (data31_to0 & 0xffffff00) >> 8;
    sync_message.u.sync_intf_message.lock = data88_to64 >> 24;
    sync_message.u.sync_intf_message.accuracy = data31_to0 & 0xff;
    sync_message.u.sync_intf_message.crc_error = GetTsEngineSyncIntfInputCode(V, crcErr_f, &input_code);
    sync_message.u.sync_intf_message.crc = GetTsEngineSyncIntfInputCode(V, crc_f, &input_code);

    sal_memset(&input_ts, 0, sizeof(input_ts));
    input_ts.seconds = sync_message.u.sync_intf_message.seconds;
    input_ts.nanoseconds = sync_message.u.sync_intf_message.nanoseconds;

    /* 2. read captured ts from fifo*/
    is_match = 0;
    sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
    cmd = DRV_IOR(TsEngineTsCaptureFifoAdjRc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));

    sal_memset(&sys_capured_ts, 0, sizeof(sys_capured_ts));
    sal_memset(&captured_ts, 0, sizeof(captured_ts));
    adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
    while (adj_frc_bitmap) /*the FIFO is empty when the bitmap is 0*/
    {
        for (src_bit = 0; src_bit < 9; src_bit++) /*get captured source from bitmap*/
        {
            if ((adj_frc_bitmap >> src_bit) & 0x01)
            {
                break;
            }
        }

        if ((src_bit == SYS_PTP_TIEM_INTF_SYNC_PULSE) && (!is_match)) /* source match*/
        {
            captured_ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            captured_ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
            sync_message.captured_ts.seconds = captured_ts.seconds;
            sync_message.captured_ts.nanoseconds = captured_ts.nanoseconds;
            is_match = 1;
        }
        else if (src_bit != SYS_PTP_TIEM_INTF_TOD_1PPS)
        {
            CTC_ERROR_RETURN(_sys_usw_ptp_map_captured_src(&(sync_message.source), &src_bit, 0));
            sync_message.captured_ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            sync_message.captured_ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);

            CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PTP_INPUT_CODE_READY, &cb));
            if (cb)
            {
                (void)cb(gchip, &sync_message);
            }
        }
        else
        {
            sys_capured_ts.lport_or_source = src_bit;
            sys_capured_ts.seq_id = 0;
            sys_capured_ts.ts.seconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcSecond_f, &ptp_captured_adj_frc);
            sys_capured_ts.ts.nanoseconds = GetTsEngineTsCaptureFifoAdjRc(V, adjRcNs_f, &ptp_captured_adj_frc);
            CTC_ERROR_RETURN(_sys_usw_ptp_add_ts_cache(lchip, CTC_PTP_CAPTURED_TYPE_RX, &sys_capured_ts)); /*add ts to cache*/
        }

        sal_memset(&ptp_captured_adj_frc, 0, sizeof(ptp_captured_adj_frc));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_captured_adj_frc));
        adj_frc_bitmap = GetTsEngineTsCaptureFifoAdjRc(V, adjRcBitmap_f, &ptp_captured_adj_frc);
    }

    if (!is_match)
    {
        sal_memset(&sys_capured_ts, 0, sizeof(sys_capured_ts));
        sys_capured_ts.lport_or_source = SYS_PTP_TIEM_INTF_SYNC_PULSE;
        sys_capured_ts.seq_id = 0;

        if (_sys_usw_ptp_get_ts_cache(lchip, CTC_PTP_CAPTURED_TYPE_RX, &sys_capured_ts) != CTC_E_NONE) /*read ts from cache*/
        {
            return CTC_E_NOT_READY;
        }
        else
        {
            captured_ts.seconds = sys_capured_ts.ts.seconds;
            captured_ts.nanoseconds = sys_capured_ts.ts.nanoseconds;
            sync_message.captured_ts.seconds = sys_capured_ts.ts.seconds;
            sync_message.captured_ts.nanoseconds = sys_capured_ts.ts.nanoseconds;
            is_match = 1;
        }
    }

    if (is_match)
    {
        /* 3. calculate offset */
        input_nanoseconds = ((uint64)input_ts.seconds) * MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) + input_ts.nanoseconds;
        captured_nanoseconds = ((uint64)captured_ts.seconds) * MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE) + captured_ts.nanoseconds;
        if (input_nanoseconds >= captured_nanoseconds)
        {
            offset.is_negative = 0;
            offset_nanoseconds = input_nanoseconds - captured_nanoseconds;
        }
        else
        {
            offset.is_negative = 1;
            offset_nanoseconds = captured_nanoseconds - input_nanoseconds;
        }

        offset.seconds = offset_nanoseconds / MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE);
        offset.nanoseconds = offset_nanoseconds % MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE);

        /* 4. adjust offset */
        ret = _sys_usw_ptp_adjust_clock_offset(lchip, &offset);
        if (CTC_E_NONE != ret )
        {
            goto Err;
        }

        /* 5. call Users callback to return code message */
        CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PTP_INPUT_CODE_READY, &cb));
        if (cb)
        {
            (void)cb(gchip, &sync_message);
        }
    }

Err:
    /* 6. clear ready flag*/
    SetTsEngineSyncIntfInputCode(V, syncCodeInRdy_f, &input_code, 0);
    cmd = DRV_IOW(TsEngineSyncIntfInputCode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &input_code));

    return CTC_E_NONE;
}

/**
 @brief  PTP Sync Interface code acc isr
*/
int32
sys_usw_ptp_isr_sync_code_acc(uint8 lchip, uint32 intr, void* p_data)
{
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ptp_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Ptp");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","quanta",p_usw_ptp_master[lchip]->quanta);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","intf_selected",p_usw_ptp_master[lchip]->intf_selected);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","cache_aging_time",p_usw_ptp_master[lchip]->cache_aging_time);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","device_type",p_usw_ptp_master[lchip]->device_type);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","tod_1pps_duty",p_usw_ptp_master[lchip]->tod_1pps_duty);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","tod_1pps_delay",p_usw_ptp_master[lchip]->tod_1pps_delay);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    return CTC_E_NONE;
}

#define __6_PTP_API__
/**
 @brief Set ptp property
*/
int32
sys_usw_ptp_set_global_property(uint8 lchip, ctc_ptp_global_prop_t property, uint32 value)
{
    int32 ret = CTC_E_NONE;
    uint32 field_val = 0;
    uint32 cmd = 0;

    /*check*/
    SYS_PTP_INIT_CHECK();

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set ptp global property, property:%d, value:%d\n", property, value);

    /*set global property*/
    switch (property)
    {
        case CTC_PTP_GLOBAL_PROP_CACHE_AGING_TIME:
        {
            CTC_MAX_VALUE_CHECK(value, 0xFFFF);

            PTP_LOCK;

            ret = _sys_usw_ptp_set_cache_aging_time(lchip, value);

            PTP_UNLOCK;

            break;
        }
        case CTC_PTP_GLOBAL_PROP_SYNC_OR_TOD_INPUT_SELECT:
        {
            CTC_MAX_VALUE_CHECK(value, (MAX_CTC_PTP_INTF_SELECT - 1));

            PTP_LOCK;

            ret = _sys_usw_ptp_set_sync_of_tod_input_select(lchip, value);

            PTP_UNLOCK;

            break;
        }
        case CTC_PTP_GLOBAL_PROP_PORT_BASED_PTP_EN:
        {
            field_val = value ? 0 : 1;
            cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_ptpUseVlanIndex_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            break;
        }
        default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return ret;
}

/**
 @brief Get ptp property
*/
int32
sys_usw_ptp_get_global_property(uint8 lchip, ctc_ptp_global_prop_t property, uint32* value)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(value);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get ptp global property, property:%d, value:%d\n", property, *value);

    switch (property)
    {
        case CTC_PTP_GLOBAL_PROP_CACHE_AGING_TIME:
        {
            PTP_LOCK;

            ret = _sys_usw_ptp_get_cache_aging_time(lchip, value);

            PTP_UNLOCK;

            break;
        }
        case CTC_PTP_GLOBAL_PROP_SYNC_OR_TOD_INPUT_SELECT:
        {
            PTP_LOCK;

            ret = _sys_usw_ptp_get_sync_of_tod_input_select(lchip, value);

            PTP_UNLOCK;

            break;
        }
        case CTC_PTP_GLOBAL_PROP_PORT_BASED_PTP_EN:
        {
            cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_ptpUseVlanIndex_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));
            *value = *value ? 0 : 1;
            break;
        }
        default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return ret;
}

/**
 @brief Get timestamp from free running clock
*/
int32
sys_usw_ptp_get_clock_timestamp(uint8 lchip, ctc_ptp_time_t* timestamp)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(timestamp);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get ptp clock timestamp, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_clock_timestamp(lchip, timestamp);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Adjust offset for free running clock
*/
int32
sys_usw_ptp_adjust_clock_offset(uint8 lchip, ctc_ptp_time_t* offset)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(offset);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Adjust ptp clock offset, lchip:%d, seconds:%d, nanoseconds:%d, is_negative:%d\n", \
                                                                                lchip, offset->seconds, offset->nanoseconds, offset->is_negative);

    if (offset->nanoseconds >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))
    {
        return CTC_E_INVALID_PARAM;
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_adjust_clock_offset(lchip, offset);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Set drift for free running clock
*/
int32
sys_usw_ptp_set_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_drift);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set ptp clock drift, lchip:%d, nanoseconds:%d, is_negative:%d\n", \
                                                                                lchip, p_drift->nanoseconds, p_drift->is_negative);

    if (p_drift->nanoseconds >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE))
    {
        return CTC_E_INVALID_PARAM;
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_set_clock_drift(lchip, p_drift);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Get drift for free running clock
*/
int32
sys_usw_ptp_get_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_drift);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get ptp clock drift, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_clock_drift(lchip, p_drift);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Set PTP device type
*/
int32
sys_usw_ptp_set_device_type(uint8 lchip, ctc_ptp_device_type_t device_type)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set device type, lchip:%d, device_type:%d\n", lchip, device_type);

    CTC_MAX_VALUE_CHECK(device_type, (MAX_CTC_PTP_DEVICE - 1));

    PTP_LOCK;

    ret = _sys_usw_ptp_set_device_type(lchip, device_type);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Set PTP device type
*/
int32
sys_usw_ptp_get_device_type(uint8 lchip, ctc_ptp_device_type_t* device_type)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(device_type);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_device_type(lchip, device_type);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Add ptp clock type
*/
int32
sys_usw_ptp_add_device_clock(uint8 lchip, ctc_ptp_clock_t* clock)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(clock);
    CTC_VALUE_RANGE_CHECK(clock->clock_id, SYS_PTP_USER_CLOCK_ID_MIN, SYS_PTP_USER_CLOCK_ID_MAX);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add ptp device clock type, clock_id:%d\n", clock->clock_id);

    PTP_LOCK;

    ret = _sys_usw_ptp_add_device_clock(lchip, clock);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Remove ptp clock type
*/
int32
sys_usw_ptp_remove_device_clock(uint8 lchip, ctc_ptp_clock_t* clock)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(clock);
    CTC_VALUE_RANGE_CHECK(clock->clock_id, SYS_PTP_USER_CLOCK_ID_MIN, SYS_PTP_USER_CLOCK_ID_MAX);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove ptp device clock type, clock_id:%d\n", clock->clock_id);

    PTP_LOCK;

    ret = _sys_usw_ptp_remove_device_clock(lchip, clock);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Set adjust delay for PTP message delay correct
*/
int32
sys_usw_ptp_set_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64 value)
{
    int32 ret = CTC_E_NONE;
    uint16 lport = 0;

    /*check*/
    SYS_PTP_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    SYS_MAX_PHY_PORT_CHECK(lport);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set ptp adjust delay, gport:%d, type:%d, value:%"PRId64"\n", gport, type, value);

    CTC_MAX_VALUE_CHECK(type, (MAX_CTC_PTP_ADJUST_DELAY - 1));

    switch (type)
    {
        case CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY:
        case CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY:
        {
            if (value < 0)
            {
                return CTC_E_INVALID_PARAM;
            }

            CTC_MAX_VALUE_CHECK(value, 0xFFFFLL);

            break;
        }
        case CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY:
        case CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY:
        {
            if (value >= 0)
            {
                if (((value >> 32) & 0xFFFFFFFFLL) > 0xF)
                {
                    return CTC_E_INVALID_PARAM;
                }
            }
            else
            {
                if ((((-value) >> 32) & 0xFFFFFFFFLL) > 0xF)
                {
                    return CTC_E_INVALID_PARAM;
                }
            }

            break;
        }
        case CTC_PTP_ADJUST_DELAY_PATH_DELAY:
        {
            if ((((value >> 32) & 0xFFFFFFFFLL) > 0xF) || (value < 0))
            {
                return CTC_E_INVALID_PARAM;
            }

            break;
        }
        default:
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_set_adjust_delay(lchip, gport, type, value);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief Set adjust delay for PTP message delay correct
*/
int32
sys_usw_ptp_get_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64* value)
{
    int32 ret = CTC_E_NONE;
    uint16 lport = 0;

    /*check*/
    SYS_PTP_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    SYS_MAX_PHY_PORT_CHECK(lport);
    CTC_PTR_VALID_CHECK(value);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get ptp adjust delay, gport:%d, type:%d\n", gport, type);

    CTC_MAX_VALUE_CHECK(type, (MAX_CTC_PTP_ADJUST_DELAY - 1));

    PTP_LOCK;

    ret = _sys_usw_ptp_get_adjust_delay(lchip, gport, type, value);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Get timestamp captured by local clock
*/
int32
sys_usw_ptp_get_captured_ts(uint8 lchip, ctc_ptp_capured_ts_t* p_captured_ts)
{
    int32 ret = CTC_E_NONE;
    sys_ptp_capured_ts_t get_ts;
    ctc_ptp_captured_src_t ctc_src = 0;
    uint8 source = 0;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_captured_ts);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get captured ts, lchip:%d, type:%d\n", lchip, p_captured_ts->type);

    if (CTC_PTP_CAPTURED_TYPE_RX == p_captured_ts->type) /*read captured rx ts triggered by syncPulse or 1PPS*/
    {
        SYS_PTP_RX_SEQ_ID_CHECK(p_captured_ts->seq_id);

        ctc_src = p_captured_ts->u.captured_src;
        CTC_ERROR_GOTO(_sys_usw_ptp_map_captured_src(&ctc_src, &source, 1), ret, out);
        sal_memset(&get_ts, 0, sizeof(get_ts));
        get_ts.lport_or_source = source;
        get_ts.seq_id = p_captured_ts->seq_id;
    }
    else if (CTC_PTP_CAPTURED_TYPE_TX == p_captured_ts->type) /*read captured tx ts triggered by packet transmit*/
    {
        /* Duet2 do not support capture tx */
        ret = CTC_E_INVALID_PARAM;
        goto out;
    }
    else
    {
        ret = CTC_E_INVALID_PARAM;
        goto out;
    }

    CTC_ERROR_GOTO(_sys_usw_ptp_get_captured_ts(lchip, p_captured_ts->type, &get_ts), ret, out);
    p_captured_ts->ts.seconds = get_ts.ts.seconds;
    p_captured_ts->ts.nanoseconds = get_ts.ts.nanoseconds;

    return CTC_E_NONE;

out:
    return ret;
}

/**
 @brief  Set sync interface config
*/
int32
sys_usw_ptp_set_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_config)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_config);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set sync interface config, lchip:%d\n", lchip);

    if ((p_config->sync_clock_hz > SYS_PTP_MAX_SYNC_CLOCK_FREQUENCY_HZ) || (p_config->sync_clock_hz < 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_config->sync_pulse_hz > MCHIP_CAP(SYS_CAP_PTP_SYNC_PULSE_FREQUENCY_HZ)) || (p_config->sync_pulse_hz < 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_config->sync_pulse_duty > 99) || (p_config->sync_pulse_duty < 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (((p_config->accuracy > 49) || (p_config->accuracy < 32)) && (p_config->accuracy != 0xFE))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_config->epoch > 63)
    {
        return CTC_E_INVALID_PARAM;
    }

    /*the frequency relationship between syncClock and syncPulse must set as follow:
    (1)the frequency of syncClock must be integer multiples of syncPulse
    (2)the period of syncClock must be longer than ( epoch + MCHIP_CAP(SYS_CAP_PTP_SYNC_CODE_BIT) )
    */
    if ((p_config->sync_pulse_hz > p_config->sync_clock_hz)
        || (p_config->sync_clock_hz % p_config->sync_pulse_hz)
        || ((p_config->sync_clock_hz / p_config->sync_pulse_hz) < (p_config->epoch + MCHIP_CAP(SYS_CAP_PTP_SYNC_CODE_BIT))))
    {
        return CTC_E_INVALID_PARAM;
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_set_sync_intf(lchip, p_config);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Get sync interface config
*/
int32
sys_usw_ptp_get_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_config)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_config);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get sync interface config, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_sync_intf(lchip, p_config);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Set toggle time to trigger sync interface output syncColck signal
*/
int32
sys_usw_ptp_set_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_toggle_time);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set sync interface toggle time, lchip:%d, seconds:%d, nanoseconds:%d, nano nanoseconds:%d\n", \
                    lchip, p_toggle_time->seconds, p_toggle_time->nanoseconds, p_toggle_time->nano_nanoseconds);

    if ((p_toggle_time->nanoseconds >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE)) ||
        (p_toggle_time->nano_nanoseconds >= MCHIP_CAP(SYS_CAP_PTP_NS_OR_NNS_VALUE)))
    {
        return CTC_E_INVALID_PARAM;
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_set_sync_intf_toggle_time(lchip, p_toggle_time);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Set toggle time to trigger sync interface output syncColck signal
*/
int32
sys_usw_ptp_get_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_toggle_time);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get sync interface toggle time, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_sync_intf_toggle_time(lchip, p_toggle_time);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Get sync interface input time information
*/
int32
sys_usw_ptp_get_sync_intf_code(uint8 lchip, ctc_ptp_sync_intf_code_t* g_time_code)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(g_time_code);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get sync interface time code, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_sync_intf_code(lchip, g_time_code);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Clear sync interface input time information
*/
int32
sys_usw_ptp_clear_sync_intf_code(uint8 lchip)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Clear sync interface time code, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_clear_sync_intf_code(lchip);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Set ToD interface config
*/
int32
sys_usw_ptp_set_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* config)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(config);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set tod interface config, lchip:%d\n", lchip);

    if ((config->pulse_duty > 99) || (config->pulse_duty < 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (config->epoch > 0x7FFF)
    {
        return CTC_E_INVALID_PARAM;
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_set_tod_intf(lchip, config);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Get ToD interface config
*/
int32
sys_usw_ptp_get_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* config)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(config);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get tod interface config, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_tod_intf(lchip, config);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Set ToD interface output message config
*/
int32
sys_usw_ptp_set_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* tx_code)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(tx_code);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set tod interface tx time code, lchip:%d\n", lchip);

    if (tx_code->clock_src > 3)
    {
        return CTC_E_INVALID_PARAM;
    }

    PTP_LOCK;

    ret = _sys_usw_ptp_set_tod_intf_tx_code(lchip, tx_code);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Get ToD interface output message config
*/
int32
sys_usw_ptp_get_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* tx_code)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(tx_code);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get tod interface tx time code, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_tod_intf_tx_code(lchip, tx_code);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Get ToD interface input time information
*/
int32
sys_usw_ptp_get_tod_intf_rx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* rx_code)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(rx_code);

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get tod interface rx time code, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_get_tod_intf_rx_code(lchip, rx_code);

    PTP_UNLOCK;

    return ret;
}

/**
 @brief  Clear TOD interface input time information
*/
int32
sys_usw_ptp_clear_tod_intf_code(uint8 lchip)
{
    int32 ret = CTC_E_NONE;

    /*check*/
    SYS_PTP_INIT_CHECK();

    /*debug info*/
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Clear tod interface rx time code, lchip:%d\n", lchip);

    PTP_LOCK;

    ret = _sys_usw_ptp_clear_tod_intf_code(lchip);

    PTP_UNLOCK;

    return ret;
}

#define __7_PTP_INIT__
/**
 @brief Initialize PTP module and set ptp default config
*/
int32
sys_usw_ptp_init(uint8 lchip, ctc_ptp_global_config_t* ptp_global_cfg)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint16 coreclock = 0;
    uint32 field_value = 0;
    IpeIntfMapperCtl_m ipe_intf_mapper_ctl;
    TsEngineRcQuanta_m ptp_frc_quanta;
    TsEngineRefRcCtl_m ptp_frc_ctl;
    TsEngineSyncIntfCfg_m ptp_sync_intf_cfg;
    TsEngineTodCfg_m ptp_tod_cfg;
    TsEngineFifoDepthRecord_m ptp_engine_fifo_depth_record;

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(ptp_global_cfg);

    if (CTC_E_NONE != _sys_usw_ptp_check_ptp_init(lchip))
    {
        return CTC_E_NONE;
    }

    if (p_usw_ptp_master[lchip] != NULL)
    {
        return CTC_E_NONE;
    }

    /* create ptp master */
    p_usw_ptp_master[lchip] = (sys_ptp_master_t*)mem_malloc(MEM_PTP_MODULE, sizeof(sys_ptp_master_t));
    if (NULL == p_usw_ptp_master[lchip])
    {
        SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
	ret = CTC_E_NO_MEMORY;
	goto error_malloc;
    }

    sal_memset(p_usw_ptp_master[lchip], 0, sizeof(sys_ptp_master_t));

    ret = sal_mutex_create(&p_usw_ptp_master[lchip]->p_ptp_mutex);
    if (ret || !p_usw_ptp_master[lchip]->p_ptp_mutex)
    {
        SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
	ret = CTC_E_NO_MEMORY;
	goto error_mutex;
    }

    /* init ts buffer */
    p_usw_ptp_master[lchip]->p_tx_ts_vector = ctc_vector_init(SYS_PTP_MAX_TX_TS_BLOCK_NUM, SYS_PTP_MAX_TX_TS_BLOCK_SIZE);
    if (NULL == p_usw_ptp_master[lchip]->p_tx_ts_vector)
    {
        SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
	ret = CTC_E_NO_MEMORY;
	goto error_tx_ts_vector;
    }

    p_usw_ptp_master[lchip]->p_rx_ts_vector = ctc_vector_init(SYS_PTP_MAX_RX_TS_BLOCK_NUM, SYS_PTP_MAX_RX_TS_BLOCK_SIZE);
    if (NULL == p_usw_ptp_master[lchip]->p_rx_ts_vector)
    {
        SYS_PTP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
	ret = CTC_E_NO_MEMORY;
	goto error_rx_ts_vector;
    }

    /* set cache aging time */
    p_usw_ptp_master[lchip]->cache_aging_time = ptp_global_cfg->cache_aging_time;

    /*init quanta*/
    CTC_ERROR_GOTO(_sys_usw_ptp_get_quanta(lchip, &(p_usw_ptp_master[lchip]->quanta)), ret, error_ioctl);
    if (0 == p_usw_ptp_master[lchip]->quanta)
    {
        p_usw_ptp_master[lchip]->quanta = MCHIP_CAP(SYS_CAP_PTP_RC_QUANTA);
    }

    /* SYSTEM Modified by tongzb,2019-03-13, for bug 51274 */
    p_usw_ptp_master[lchip]->quanta = 4;
    sal_memset(&ptp_frc_quanta, 0, sizeof(ptp_frc_quanta));
    SetTsEngineRcQuanta(V, quanta_f, &ptp_frc_quanta, p_usw_ptp_master[lchip]->quanta);
    cmd = DRV_IOW(TsEngineRcQuanta_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_frc_quanta), ret, error_ioctl);

    /*init ptp register*/
    sal_memset(&ipe_intf_mapper_ctl, 0, sizeof(ipe_intf_mapper_ctl));
    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl), ret, error_ioctl);

    SetIpeIntfMapperCtl(V, ptpUseVlanIndex_f, &ipe_intf_mapper_ctl, 0);
    SetIpeIntfMapperCtl(V, ptpIndexObeyPort_f, &ipe_intf_mapper_ctl, 0);
    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl), ret, error_ioctl);

    /*init sync interface*/
    sal_memset(&ptp_sync_intf_cfg, 0, sizeof(ptp_sync_intf_cfg));
    SetTsEngineSyncIntfCfg(V, syncClockOutEn_f, &ptp_sync_intf_cfg, 1);
    SetTsEngineSyncIntfCfg(V, syncPulseOutEnable_f, &ptp_sync_intf_cfg, 1);
    SetTsEngineSyncIntfCfg(V, syncCodeOutEnable_f, &ptp_sync_intf_cfg, 1);
    SetTsEngineSyncIntfCfg(V, syncPulseInIntrEn_f, &ptp_sync_intf_cfg, 0);
    SetTsEngineSyncIntfCfg(V, ignoreSyncCodeInRdy_f, &ptp_sync_intf_cfg, 0);
    SetTsEngineSyncIntfCfg(V, syncPulseOutNum_f, &ptp_sync_intf_cfg, 0);
    SetTsEngineSyncIntfCfg(V, offsetAdjMode_f, &ptp_sync_intf_cfg, 1);
    SetTsEngineSyncIntfCfg(V, offsetHwAdjDisperseEn_f, &ptp_sync_intf_cfg, 1);
    if (CTC_PTP_INTF_SELECT_SYNC == ptp_global_cfg->intf_selected)
    {
        SetTsEngineSyncIntfCfg(V, tsCaptureEn_f, &ptp_sync_intf_cfg, 1);
    }
    cmd = DRV_IOW(TsEngineSyncIntfCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_sync_intf_cfg), ret, error_ioctl);

    /*init ToD interface*/
    sal_memset(&ptp_tod_cfg, 0, sizeof(ptp_tod_cfg));
    SetTsEngineTodCfg(V, todCodeOutEnable_f, &ptp_tod_cfg, 1);
    SetTsEngineTodCfg(V, todPulseOutEnable_f, &ptp_tod_cfg, 1);
    SetTsEngineTodCfg(V, todPulseInIntrEn_f, &ptp_tod_cfg, 0);
    SetTsEngineTodCfg(V, ignoreTodCodeInRdy_f, &ptp_tod_cfg, 0);
    SetTsEngineTodCfg(V, todCodeSampleThreshold_f, &ptp_tod_cfg, 7);
    SetTsEngineTodCfg(V, todPulseOutMode_f, &ptp_tod_cfg, 1);
    SetTsEngineTodCfg(V, todPulseOutDelay_f, &ptp_tod_cfg, 0); /* not pre output */
    /*1ms= todPulseReqMaskInterval*512/clockCore(M)/1000*/
    CTC_ERROR_GOTO(sys_usw_get_chip_clock(lchip, &coreclock), ret, error_ioctl);
    SetTsEngineTodCfg(V, todPulseReqMaskInterval_f, &ptp_tod_cfg, ((coreclock * 1000000) >> 9) - 100); /* 1ms */
    if (CTC_PTP_INTF_SELECT_TOD == ptp_global_cfg->intf_selected)
    {
        SetTsEngineTodCfg(V, todTsCaptureEn_f, &ptp_tod_cfg, 1);
    }
    cmd = DRV_IOW(TsEngineTodCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_tod_cfg), ret, error_ioctl);

    p_usw_ptp_master[lchip]->intf_selected = ptp_global_cfg->intf_selected;
    p_usw_ptp_master[lchip]->tod_1pps_delay = 0;

    /*init PllMiscCfg*/
    field_value = 0;
    cmd = DRV_IOW(PllMiscCfg_t, PllMiscCfg_cfgPllMiscReset_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_ioctl);

    /*set ts capture ctl*/
    field_value = 0x1FF & (~(1 << CTC_PTP_TIEM_INTF_TOD_1PPS));
    cmd = DRV_IOW(TsEngineTsCaptureCtrl_t, TsEngineTsCaptureCtrl_tsCaptureFifoMask_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_ioctl);

    field_value = 1;
    cmd = DRV_IOW(TsEngineTsCaptureCtrl_t, TsEngineTsCaptureCtrl_tsCaptureFifoIntrThreshold_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_ioctl);

    /*set ts fifo depth*/
    sal_memset(&ptp_engine_fifo_depth_record, 0, sizeof(ptp_engine_fifo_depth_record));
    SetTsEngineFifoDepthRecord(V, tsCaptureFifoFifoDepth_f, &ptp_engine_fifo_depth_record, 15);
    cmd = DRV_IOW(TsEngineFifoDepthRecord_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_engine_fifo_depth_record), ret, error_ioctl);

    /*enable FRC counter*/
    sal_memset(&ptp_frc_ctl, 0, sizeof(ptp_frc_ctl));
    SetTsEngineRefRcCtl(V, rcEn_f, &ptp_frc_ctl, 1);
    SetTsEngineRefRcCtl(V, rcFifoFullThreshold_f, &ptp_frc_ctl, 7);
    cmd = DRV_IOW(TsEngineRefRcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ptp_frc_ctl), ret, error_ioctl);

    /*init cfgTsUseIntRefClk */
    /* SYSTEM MODIFIED by tongzb for init ptp with internal clock, 2019-07-25  */
    ptp_global_cfg->use_internal_clock_en = 1;
    if (ptp_global_cfg->use_internal_clock_en)
    {
        field_value = 1;
    }
    else
    {
        field_value = 0;
    }
    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(SupMiscCfg_t, SupMiscCfg_cfgTsUseIntRefClk_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_ioctl);
    }
    else
    {
        cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgTsUseIntRefClk_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_ioctl);
    }

    /* support 4ns default use 250Mhz*/
    field_value = coreclock/250;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkTsRefDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkTsRefDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkTsRefDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    (void)sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_PTP_TOD_CODE_IN_RDY, sys_usw_ptp_isr_tod_code_rdy);
    (void)sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_PTP_SYNC_CODE_IN_RDY, sys_usw_ptp_isr_sync_code_rdy);

    /* warmboot register */
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_PTP, _sys_usw_ptp_wb_sync), ret, error_wb);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_PTP, _sys_usw_ptp_dump_db), ret, error_wb);

    /* warmboot data restore */
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_GOTO(_sys_usw_ptp_wb_restore(lchip), ret, error_wb);
    }

    return CTC_E_NONE;

error_wb:
error_ioctl:
    ctc_vector_release(p_usw_ptp_master[lchip]->p_rx_ts_vector);
error_rx_ts_vector:
    ctc_vector_release(p_usw_ptp_master[lchip]->p_tx_ts_vector);
error_tx_ts_vector:
    sal_mutex_destroy(p_usw_ptp_master[lchip]->p_ptp_mutex);
error_mutex:
    mem_free(p_usw_ptp_master[lchip]);
error_malloc:
    return ret;
}

int32
sys_usw_ptp_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == p_usw_ptp_master[lchip])
    {
        return CTC_E_NONE;
    }

    (void)sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_PTP_SYNC_CODE_IN_RDY, NULL);
    (void)sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_PTP_TOD_CODE_IN_RDY, NULL);

    /*free rx ts vector*/
    ctc_vector_traverse(p_usw_ptp_master[lchip]->p_rx_ts_vector, (vector_traversal_fn)_sys_usw_ptp_free_node_data, NULL);
    ctc_vector_release(p_usw_ptp_master[lchip]->p_rx_ts_vector);

    /*free tx ts vector*/
    ctc_vector_traverse(p_usw_ptp_master[lchip]->p_tx_ts_vector, (vector_traversal_fn)_sys_usw_ptp_free_node_data, NULL);
    ctc_vector_release(p_usw_ptp_master[lchip]->p_tx_ts_vector);

    sal_mutex_destroy(p_usw_ptp_master[lchip]->p_ptp_mutex);

    mem_free(p_usw_ptp_master[lchip]);

    return CTC_E_NONE;
}

#endif
