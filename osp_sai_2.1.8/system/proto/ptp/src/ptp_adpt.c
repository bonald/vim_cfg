
#include "proto.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_fea_ptp_global_define.h"
#include "gen/tbl_fea_ptp_global.h"
#include "gen/tbl_fea_ptp_ts_define.h"
#include "gen/tbl_fea_ptp_ts.h"

#include "glb_ptp_define.h"

#include "ptp_error.h"
#include "ptp_desc.h"
#include "ptp_adpt.h"
#include "ptp_api.h"

#include "fei.h"

void
ptp_adpt_adjust_offset(int64 offset)
{
    ptp_api_set_hsrv_adjust_offset(offset);
}

void
ptp_adpt_set_path_delay(tbl_ptp_port_t *p_db_ptp_port, int64 path_delay)
{
    /*add by chenc for bug 46576, 2019-01-10*/
    if (path_delay < 0)
    {
        PTP_LOG_WARN("Invalid path delay %"PRId64, path_delay);
        return;
    }
    /*bug 46576, end*/
    
    ptp_api_set_port_hsrv_path_delay(p_db_ptp_port, path_delay);
}

int32
ptp_adpt_switch_device_type(uint32 dev_type)
{
    ptp_api_set_hsrv_device_type(dev_type);
    return PTP_E_SUCCESS;
}

#define HAL_PTP_GET_TX_TS_REASON_PTP 1
int32
ptp_adpt_get_tx_ts(uint32 ifindex, glb_ptp_timestamp_t* p_tx_ts, uint16 seq_id, char *ifname)
{
    int32 ret;
    hal_ptp_get_tx_ts_req_t req;
    hal_ptp_get_tx_ts_resp_t resp;
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(ifname, ifname_ext);
 
    req.ifindex = ifindex;
    req.type = HAL_PTP_GET_TX_TS_REASON_PTP;
    req.seq_id = seq_id;
    //PTP_DBG_COMM("Get TX timestamp from interface %s", ifname_ext);
    ret = fei_talk_sync(FEI_PTP_GET_TX_TS, &req, sizeof(hal_ptp_get_tx_ts_req_t), &resp,
                        sizeof(hal_ptp_get_tx_ts_resp_t));
    if (0 != ret)
    {
        PTP_DBG_COMM("Failed to get TX timestamp from interface %s", ifname_ext);
        PTP_LOG_ERR("[ADPT]Failed to get transmitted timestamp from interface %s.", ifname_ext);
        return PTP_E_CHIP_TX_TIME_GET_FAIL;
    }

    if (resp.invalid)
    {
        PTP_DBG_COMM("Failed to get TX timestamp from interface %s for timestamp is invalid", ifname_ext);
        PTP_LOG_ERR("[ADPT]Failed to get transmitted timestamp from interface %s for timestamp is invalid.", ifname_ext);
        return PTP_E_CHIP_TX_TIME_GET_FAIL;
    }

    if (resp.blocked)
    {
        PTP_DBG_COMM("Failed to get TX timestamp from blocked interface %s", ifname_ext);
        return PTP_E_IF_BLOCKED;
    }

    PTP_DBG_COMM("Get TX %s from interface %s",
                ptp_desc_ptp_timestamp(&(resp.ts)), ifname_ext);

    p_tx_ts->sec.msb = resp.ts.sec.msb;
    p_tx_ts->sec.lsb = resp.ts.sec.lsb;
    p_tx_ts->ns = resp.ts.ns;

    return PTP_E_SUCCESS;
}

int32
ptp_adpt_reset_tx_ts(void)
{
    int32 ret;
    int32 resp = 0;
    int32 req = 0;
    
    //PTP_DBG_COMM("Reset TX timestamp");
    ret = fei_talk_sync(FEI_PTP_RESET_TX_TS, &req, sizeof(int32), &resp,
                        sizeof(int32));
    if (0 != ret)
    {
        PTP_DBG_COMM("Failed to reset TX timestamp for send message fail");
        PTP_LOG_ERR("[ADPT]Failed to reset transmitted timestamp for send message fail.");
        return PTP_E_CHIP_TX_TIME_GET_FAIL;
    }

    if (FALSE == resp)
    {
        PTP_DBG_COMM("Failed to reset TX timestamp");
        PTP_LOG_ERR("[ADPT]Failed to reset transmitted timestamp.");
        return PTP_E_CHIP_TX_TIME_GET_FAIL;
    }

    return PTP_E_SUCCESS;
}

int32
ptp_adpt_update_system_time(int16 utc_offset)
{
    PTP_DBG_COMM("Update current system time");
    ptp_api_set_hsrv_update_system_time(utc_offset);
    return PTP_E_SUCCESS;
}

int32
ptp_adpt_get_hardware_second(int32* second, int32* nanoseconds)
{
    hal_ptp_get_hardware_second_resp_t hw_time;

    sal_memset(&hw_time, 0, sizeof(hw_time));
    int32 ret = fei_talk_sync(FEI_PTP_GET_HW_SECOND, NULL, 0, &hw_time, sizeof(hw_time));
    if (0 != ret)
    {
        PTP_DBG_COMM("Failed to get current TAI time");
        PTP_LOG_ERR("[ADPT]Failed to get hardware second time.");
        return PTP_E_ERROR;
    }
    *second = hw_time.second;
    *nanoseconds = hw_time.nanoseconds;

    return PTP_E_SUCCESS;
}

