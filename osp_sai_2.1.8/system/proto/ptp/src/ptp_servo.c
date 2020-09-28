
#include "proto.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"

#include "ptp_constant.h"
#include "ptp_error.h"
#include "ptp_adpt.h"
#include "ptp_util.h"
#include "ptp_desc.h"
#include "ptp_servo.h"

#define PTP_SERVO_NS_THRESHOLD_WINDOW 1000000
#define PTP_SERVO_NNS_THRESHOLD_WINDOW 500000
#define PTP_SERVO_NNS_BEFORE_OFFSET_ADJUST 4
#define PTP_SERVO_NNS_LOCK_JITTER_WINDOW 20
#define PTP_SERVO_NNS_LOCK_BEFORE_IN_JITTER_TIMES 50
#define PTP_SERVO_PATH_DELAY_JITTER_WINDOW 50
#define PTP_SERVO_PATH_DELAY_JITTER_RETRY 10
#define PTP_SERVO_UNCALIBRATED_UPDATE_OFFSET_TIMES_MAX 4
#define PTP_SERVO_UNCALIBRATED_UPDATE_LIMIT_TIMES_MAX 4
#define PTP_SERVO_NS_THRESHOLD_SWITCH 10000

static inline uint32
_ptp_servo_check_band_pass_filter_fail(int64 value_to_be_checked, int32 median_of_bpf, int32 half_bandwidth,
                                       char* name_of_bpf)
{
    if (value_to_be_checked > (median_of_bpf + half_bandwidth))
    {
        PTP_DBG_SYNC("Check value [%s] BPF(F0=%d, H-BW=%d, VAL=%"PRId64") bigger than max fail",
                    name_of_bpf, median_of_bpf, half_bandwidth, (value_to_be_checked));
        return TRUE;
    }
    else if (value_to_be_checked < (median_of_bpf - half_bandwidth))
    {
        PTP_DBG_SYNC("Check value [%s] BPF(F0=%d, H-BW=%d, VAL=%"PRId64") little than min fail",
                    name_of_bpf, median_of_bpf, half_bandwidth, (value_to_be_checked));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void
ptp_servo_update_offset(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    /* calculate 'master_to_slave_delay' */
    int64 master_to_slave_delay = ptp_util_time_sub_time(&p_port->p.sync_receive_time, &p_port->p.sync_send_time);
    PTP_DBG_SYNC("Update offset for %s master_to_slave_delay %"PRId64, IFNAME_ETH2FULL(p_port->key.name, ifname_ext), (master_to_slave_delay));
    int64 correction_field_with_mean_path_delay = p_port->p.last_sync_correction_field.v >> PTP_SCALED_NS_BITS;

    /*take care about correctionField*/
    master_to_slave_delay -= (p_port->p.last_sync_correction_field.v >> PTP_SCALED_NS_BITS);    

    if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism && p_port->master_clock_selected)
    {
        p_port->p.last_delay_m_s = master_to_slave_delay;
    }

    /* update 'offsetFromMaster' */
    if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
    {
        master_to_slave_delay -= p_port->port_ds.peer_mean_path_delay;
        correction_field_with_mean_path_delay += p_port->port_ds.peer_mean_path_delay;
    }
    else if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
    {
        master_to_slave_delay -= g_p_master->current_ds.mean_path_delay;
        correction_field_with_mean_path_delay += g_p_master->current_ds.mean_path_delay;
    }

    g_p_master->current_ds.offset_from_master = master_to_slave_delay;

    PTP_DBG_SYNC("Update offset for %s master_to_slave_delay, subtract mean_path_delay result %"PRId64, ifname_ext, (g_p_master->current_ds.offset_from_master));

    return;
    
    //not support
#if 0
    if (!g_p_master->frequency_recovery_enable)
    {
        return;
    }

    int64 send_interval = ptp_util_time_sub_time(&p_port->p.sync_send_time, &g_p_master->c.last_sync_tx_ts);
    int64 recv_interval = ptp_util_time_sub_time(&p_port->p.sync_receive_time, &g_p_master->c.last_sync_rx_ts);
    sal_memcpy(&g_p_master->c.last_sync_tx_ts, &p_port->p.sync_send_time, sizeof(glb_ptp_timestamp_t));
    sal_memcpy(&g_p_master->c.last_sync_rx_ts, &p_port->p.sync_receive_time, sizeof(glb_ptp_timestamp_t));
    recv_interval += g_p_master->c.last_master_to_slave_delay;
    g_p_master->c.last_master_to_slave_delay = master_to_slave_delay;
    send_interval += (correction_field_with_mean_path_delay - g_p_master->c.last_correction_field_with_mean_path_delay);
    g_p_master->c.last_correction_field_with_mean_path_delay = correction_field_with_mean_path_delay;

    if (g_p_master->c.servo_nns_ready && 0 != send_interval)
    {
        int64 nns = ((recv_interval - send_interval) * GLB_PTP_NNS_PER_NS) / send_interval;
        PTP_DBG_SYNC("Update offset nns %"PRId64, (nns));
        if (_ptp_servo_check_band_pass_filter_fail(nns, 0, PTP_SERVO_NNS_THRESHOLD_WINDOW, "nns"))
        {
            return;
        }

        int32 idx, mean_nns;
        if (g_p_master->c.servo_nns_cnt == PTP_SERVO_NNS_CACHE_MAX)
        {
            for (idx = 0, mean_nns = 0; idx < g_p_master->c.servo_nns_cnt; idx++)
            {
                mean_nns += g_p_master->c.servo_nns_cache[idx];
            }

            mean_nns /= g_p_master->c.servo_nns_cnt;
            if (_ptp_servo_check_band_pass_filter_fail(nns, mean_nns, PTP_SERVO_NNS_LOCK_JITTER_WINDOW, "mean_nns"))
            {
                if(g_p_master->c.servo_nns_lock_cnt <= 0)
                {
                    g_p_master->c.servo_nns_lock_cnt = 0;
                    g_p_master->c.servo_nns_lock = FALSE;
                }
                else
                {
                    g_p_master->c.servo_nns_lock_cnt--;
                }
            }
            else
            {
                g_p_master->c.servo_nns_lock_cnt++;
                if(g_p_master->c.servo_nns_lock_cnt >= PTP_SERVO_NNS_LOCK_BEFORE_IN_JITTER_TIMES)
                {
                    g_p_master->c.servo_nns_lock_cnt = PTP_SERVO_NNS_LOCK_BEFORE_IN_JITTER_TIMES;
                    g_p_master->c.servo_nns_lock = TRUE;
                }
            }
        }

        g_p_master->c.mean_nns = nns;
        g_p_master->c.servo_nns_cnt++;
        if (!g_p_master->c.servo_nns_first_2_ignored)
        {
            if (g_p_master->c.servo_nns_cnt >= 2)
            {
                g_p_master->c.servo_nns_cnt = 0;
                g_p_master->c.servo_nns_first_2_ignored = TRUE;
            }

            return;
        }

        if (!g_p_master->c.servo_nns_lock)
        {
            g_p_master->c.servo_nns_cache[g_p_master->c.servo_nns_pos] = nns;
            g_p_master->c.servo_nns_pos++;
            g_p_master->c.servo_nns_pos %= PTP_SERVO_NNS_CACHE_MAX;
        }

        if (g_p_master->c.servo_nns_cnt >= PTP_SERVO_NNS_CACHE_MAX)
        {
            g_p_master->c.servo_nns_cnt = PTP_SERVO_NNS_CACHE_MAX;
        }
    }
    
    return;
#endif
}

void
ptp_servo_update_delay(tbl_ptp_port_t* p_port)
{
    int32 idx;
    int64 path_delay;
    int64 mean_path_delay;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_SYNC("Update delay for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    if (!p_port->master_clock_selected)
    {
        return;
    }
    
    /* calculate 'slave_to_master_delay' (Master to Slave delay is already computed in updateOffset)*/
    p_port->p.delay_s_m = ptp_util_time_sub_time(&p_port->p.delay_req_receive_time, &p_port->p.delay_req_send_time);
    PTP_DBG_SYNC("Update delay for %s, delay_s_m %"PRId64", offset_adjusted %"PRId64, 
        ifname_ext, (p_port->p.delay_s_m), (p_port->p.offset_adjusted));
    p_port->p.delay_s_m -= p_port->p.offset_adjusted;
    
    /* update 'one_way_delay' */
    path_delay = p_port->p.delay_s_m + p_port->p.delay_m_s;

    PTP_DBG_SYNC("Update delay for %s, delay_s_m %"PRId64", delay_m_s %"PRId64", path_delay %"PRId64", last_delay_resp_correction_field %"PRId64, 
        ifname_ext, (p_port->p.delay_s_m), (p_port->p.delay_m_s), (path_delay), 
        (p_port->p.last_delay_resp_correction_field.v >> PTP_SCALED_NS_BITS));

    /*subtract correctionField*/
    path_delay -= (p_port->p.last_delay_resp_correction_field.v >> PTP_SCALED_NS_BITS);
    PTP_DBG_SYNC("Update delay for %s, path_delay subtract correctionField result %"PRId64, 
        ifname_ext, (path_delay));
        
    /*Compute one-way delay*/
    path_delay /= 2;

    PTP_DBG_SYNC("Update delay path_delay one-way result %"PRId64, (path_delay));

    /*fix bug 48894 by chenc*/
    if (path_delay < 0)
    {
        PTP_DBG_SYNC("Path delay result %"PRId64" is smaller than 0, not add to cache, return", (path_delay));
        return;
    }
    
    if (_ptp_servo_check_band_pass_filter_fail(path_delay, 0, PTP_SERVO_NS_THRESHOLD_WINDOW, "path_delay"))
    {
        return;
    }

    /*calc mean_path_delay*/
    if (p_port->c.servo_pdc_cnt == PTP_SERVO_PATH_DELAY_CACHE_MAX)
    {
        for (idx = 0, mean_path_delay = 0; idx < p_port->c.servo_pdc_cnt; idx++)
        {
            mean_path_delay += p_port->c.path_delay_cache[idx];
        }

        mean_path_delay /= p_port->c.servo_pdc_cnt;
        
        PTP_DBG_SYNC("Update delay mean_path_delay %"PRId64, (mean_path_delay));
        if (_ptp_servo_check_band_pass_filter_fail(path_delay, mean_path_delay,
                    PTP_SERVO_PATH_DELAY_JITTER_WINDOW, "mean_path_delay"))
        {
            p_port->c.servo_pdc_rty++;
            PTP_DBG_SYNC("Update delay mean_path_delay %"PRId64" bigger than max, servo_pdc_rty %hhu", (mean_path_delay), p_port->c.servo_pdc_rty);
            if (p_port->c.servo_pdc_rty >= PTP_SERVO_PATH_DELAY_JITTER_RETRY)
            {
                PTP_DBG_SYNC("Update delay Reset path_delay BPF");
                sal_memset(&p_port->c, 0x00, sizeof(p_port->c));
            }

            return;
        }
    }
    
    p_port->c.servo_pdc_cnt++;
    p_port->c.servo_pdc_rty = 0;
    if (p_port->c.servo_pdc_cnt >= PTP_SERVO_PATH_DELAY_CACHE_MAX)
    {
        p_port->c.servo_pdc_cnt = PTP_SERVO_PATH_DELAY_CACHE_MAX;
    }

    p_port->c.path_delay_cache[p_port->c.servo_pdc_pos] = path_delay;
    p_port->c.servo_pdc_pos++;
    p_port->c.servo_pdc_pos %= PTP_SERVO_PATH_DELAY_CACHE_MAX;

    /*calc mean_path_delay*/
    for (idx = 0, mean_path_delay = 0; idx < p_port->c.servo_pdc_cnt; idx++)
    {
        mean_path_delay += p_port->c.path_delay_cache[idx];
    }

    mean_path_delay /= p_port->c.servo_pdc_cnt;
    PTP_DBG_SYNC("Update delay mean_path_delay result %"PRId64, (mean_path_delay));
    g_p_master->current_ds.mean_path_delay = mean_path_delay;

    return;
}

void
ptp_servo_update_peerdelay(tbl_ptp_port_t* p_port)
{
    int32 idx;
    int64 path_delay;
    int64 peer_mean_path_delay;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    PTP_DBG_SYNC("Update pdelay for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    /*!!!FIXME!!! on peer-delay our device can only work on two-step mode, so one step is not implement now*/
    
    /* calculate 'slave_to_master_delay' */
    p_port->p.pdelay_m_s = ptp_util_time_sub_time(&p_port->p.pdelay_resp_receive_time, &p_port->p.pdelay_resp_send_time);
    p_port->p.pdelay_s_m = ptp_util_time_sub_time(&p_port->p.pdelay_req_receive_time, &p_port->p.pdelay_req_send_time);
    PTP_DBG_SYNC("Update pdelay for %s, pdelay_m_s %"PRId64", pdelay_s_m %"PRId64, 
        ifname_ext, (p_port->p.pdelay_m_s), (p_port->p.pdelay_s_m));
    
    /* update 'one_way_delay' */
    path_delay = p_port->p.pdelay_s_m + p_port->p.pdelay_m_s;

    PTP_DBG_SYNC("Update pdelay for %s, path_delay %"PRId64", correction %"PRId64, 
        ifname_ext, (path_delay), (p_port->p.last_pdelay_resp_correction_field.v >> PTP_SCALED_NS_BITS));
        
    /*subtract correctionField*/
    path_delay -= (p_port->p.last_pdelay_resp_correction_field.v >> PTP_SCALED_NS_BITS);

    /*Compute one-way delay*/
    path_delay /= 2;

    PTP_DBG_SYNC("Update pdelay correction %"PRId64", subtract correctionField and one-way path_delay %"PRId64, 
        (p_port->p.last_pdelay_resp_correction_field.v >> PTP_SCALED_NS_BITS), (path_delay));
        
    if (_ptp_servo_check_band_pass_filter_fail(path_delay, 0, PTP_SERVO_NS_THRESHOLD_WINDOW, "path_delay"))
    {
        return;
    }

    if (p_port->c.servo_pdc_cnt == PTP_SERVO_PATH_DELAY_CACHE_MAX)
    {
        for (idx = 0, peer_mean_path_delay = 0; idx < p_port->c.servo_pdc_cnt; idx++)
        {
            peer_mean_path_delay += p_port->c.path_delay_cache[idx];
        }

        peer_mean_path_delay /= p_port->c.servo_pdc_cnt;
        PTP_DBG_SYNC("Update pdelay peer_mean_path_delay result %"PRId64, (peer_mean_path_delay));
        
        if (_ptp_servo_check_band_pass_filter_fail(path_delay, peer_mean_path_delay, PTP_SERVO_PATH_DELAY_JITTER_WINDOW, "peer_mean_path_delay"))
        {
            p_port->c.servo_pdc_rty++;
            PTP_DBG_SYNC("Update pdelay peer_mean_path_delay %"PRId64" bigger than max, servo_pdc_rty %hhu", (peer_mean_path_delay), p_port->c.servo_pdc_rty);
            if (p_port->c.servo_pdc_rty >= PTP_SERVO_PATH_DELAY_JITTER_RETRY)
            {
                PTP_DBG_SYNC("Update pdelay Reset path_delay BPF");
                sal_memset(&p_port->c, 0x00, sizeof(p_port->c));
            }

            return;
        }
    }

    p_port->c.servo_pdc_cnt++;
    p_port->c.servo_pdc_rty = 0;
    if (p_port->c.servo_pdc_cnt >= PTP_SERVO_PATH_DELAY_CACHE_MAX)
    {
        p_port->c.servo_pdc_cnt = PTP_SERVO_PATH_DELAY_CACHE_MAX;
    }

    p_port->c.path_delay_cache[p_port->c.servo_pdc_pos] = path_delay;
    p_port->c.servo_pdc_pos++;
    p_port->c.servo_pdc_pos %= PTP_SERVO_PATH_DELAY_CACHE_MAX;
    for (idx = 0, peer_mean_path_delay = 0; idx < p_port->c.servo_pdc_cnt; idx++)
    {
        peer_mean_path_delay += p_port->c.path_delay_cache[idx];
    }

    peer_mean_path_delay /= p_port->c.servo_pdc_cnt;
    p_port->port_ds.peer_mean_path_delay = peer_mean_path_delay;
    
    PTP_DBG_SYNC("Update pdelay for %s, peer_mean_path_delay result %"PRId64, ifname_ext, (peer_mean_path_delay));
    ptp_adpt_set_path_delay(p_port, p_port->port_ds.peer_mean_path_delay);

    return;
}

void
ptp_servo_update_clock(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    int64 offset = -g_p_master->current_ds.offset_from_master;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    PTP_DBG_SYNC("Update clock [%s %s]update clock adjust offset %"PRId64, IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state), (offset));

    if (!p_port->master_clock_selected)
    {
        /*added by wangl for bug 27581, 2014-3-13*//*adjust big diff*/
        if(((offset > PTP_SERVO_NS_THRESHOLD_SWITCH) || (offset < -PTP_SERVO_NS_THRESHOLD_SWITCH)) || 
            (g_p_master->current_ds.mean_path_delay != 0) ||(p_port->port_ds.peer_mean_path_delay !=0))
        {
            PTP_DBG_SYNC("Update clock offset %"PRId64", PTP_SERVO_NS_THRESHOLD_SWITCH %d, mean_path_delay %"PRId64", peer_mean_path_delay %"PRId64", adjust big offset!",
                offset, PTP_SERVO_NS_THRESHOLD_SWITCH, (g_p_master->current_ds.mean_path_delay), (p_port->port_ds.peer_mean_path_delay));
            ptp_adpt_adjust_offset(offset);
        }
        
        if (_ptp_servo_check_band_pass_filter_fail(offset, 0, PTP_SERVO_NS_THRESHOLD_WINDOW, "offset"))
        {
            return;
        }

        p_port->uncalibrated_update_offset_times++;
        if (p_port->uncalibrated_update_offset_times >= PTP_SERVO_UNCALIBRATED_UPDATE_OFFSET_TIMES_MAX)
        {
            PTP_DBG_SYNC("Update clock uncalibrated update offset times %d is big or equal to 4, master_clock_selected change to TRUE", p_port->uncalibrated_update_offset_times);
            p_port->uncalibrated_update_offset_times = 0;
            p_port->master_clock_selected = TRUE;
        }
        else
        {
            PTP_DBG_SYNC("Update clock uncalibrated update offset times %d smaller than 4, return", p_port->uncalibrated_update_offset_times);
        }

        return;
    }

    if (_ptp_servo_check_band_pass_filter_fail(offset, 0, PTP_SERVO_NS_THRESHOLD_WINDOW, "offset")) 
    {
        p_port->uncalibrated_update_limit_times++;
        PTP_DBG_SYNC("Update clock check band pass filter failed when master_clock_selected, uncalibrated update limit times %hhu", p_port->uncalibrated_update_limit_times);
        if (p_port->uncalibrated_update_limit_times >= PTP_SERVO_UNCALIBRATED_UPDATE_LIMIT_TIMES_MAX)
        {
            PTP_DBG_SYNC("Update clock synchronization fault when master_clock_selected, uncalibrated update limit times %hhu", p_port->uncalibrated_update_limit_times);
            p_port->uncalibrated_update_limit_times = 0;
            p_port->synchronization_fault = TRUE;
        }

        return;
    }

    p_port->uncalibrated_update_limit_times = 0;

#if 0    
    if(g_p_master->current_ds.mean_path_delay != 0)
    {
        ptp_adpt_adjust_offset(offset);
    }
#endif   

    if(g_p_master->current_ds.offset_from_master != 0) /*modify by yoush in 2017-03-20*/
    {
        ptp_adpt_adjust_offset(offset);
    }
    p_port->p.offset_adjusted += offset;

    return;
    //not support
    #if 0
    if (!g_p_master->frequency_recovery_enable)
    {
        return;
    }

    g_p_master->c.servo_offset_cnt++;
    if (g_p_master->c.servo_offset_cnt >= PTP_SERVO_NNS_BEFORE_OFFSET_ADJUST)
    {
        g_p_master->c.servo_offset_cnt = PTP_SERVO_NNS_BEFORE_OFFSET_ADJUST;
        g_p_master->c.servo_nns_ready = TRUE;
    }

    if (g_p_master->c.servo_nns_ready && !g_p_master->c.servo_nns_lock && 0 != g_p_master->c.mean_nns)
    {
        float64 ffo = g_p_master->c.mean_nns * 1.0 / GLB_PTP_NNS_PER_NS;
        PTP_DBG_SYNC("Update clock [%s]adjust ffo %e", ptp_desc_state(p_port->port_ds.port_state), (-ffo));
        ptp_adpt_adjust_ffo(-ffo);
    }
    
    return;
    #endif
}

