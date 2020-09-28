
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "lib_fs.h"
#include <errno.h>
#include "glb_const.h"
#include "cdb_ds.h"
#include "glb_ip_define.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "gen/tbl_rmon_stats_history_define.h"
#include "gen/tbl_rmon_stats_history.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_app_debug_define.h"
#include "gen/tbl_app_debug.h"
#include "appcfg.h"
#include "app_rmon.h"


uint32
rmon_debug_is_on(uint32 flag)
{
    tbl_app_debug_t *p_db_glb = NULL;

    p_db_glb = tbl_app_debug_get_app_debug();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->rmon, flag))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void
rmon_format_tv(uint64 tv, char *time)
{
    uint32 days = 0;
    uint32 hours = 0;
    uint32 minutes = 0;
    uint32 seconds = 0;

    days = tv / 86400;
    hours = (tv / 3600) % 24;
    minutes = (tv / 60) % 60;
    seconds = tv % 60;
    sal_sprintf(time, "%ddays %dh:%dm:%ds", days, hours, minutes, seconds);

    return;
}

static int32
rmon_check_oid_varlen(char *oid)
{
    int32 id_len  = 0;
    int32 var_len = sal_strlen(oid);
    int32 i = 0;
    
    while (i < var_len && oid[i] != '.')
    {
        if (!sal_isdigit(oid[i]))
        {
            return FALSE;
        }
        id_len++;
        i++;
    }
    if (id_len > 5 || 0 == id_len)
    {
        return FALSE;
    }

    return TRUE;
}

static void 
rmon_copy_alarm_value (tbl_rmon_ether_stats_t *p_db_stats, uint32 ether_stats_var, uint64 *value)
{
  if (ether_stats_var == ETHERSTATSDROPEVENTS)
    *value = p_db_stats->drop_events;

  if (ether_stats_var == ETHERSTATSOCTETS)
    *value = p_db_stats->octets;

  if (ether_stats_var == ETHERSTATSPKTS)
    *value = p_db_stats->pkts;

  if (ether_stats_var == ETHERSTATSBROADCASTPKTS)
    *value = p_db_stats->broadcast_pkts;

  if (ether_stats_var == ETHERSTATSMULTICASTPKTS)
    *value = p_db_stats->multicast_pkts;

  if (ether_stats_var == ETHERSTATSCRCALIGNERRORS)
    *value = p_db_stats->crc_align_errors;

  if (ether_stats_var == ETHERSTATSUNDERSIZEPKTS)
    *value = p_db_stats->undersize_pkts;

  if (ether_stats_var == ETHERSTATSOVERSIZEPKTS)
    *value = p_db_stats->oversize_pkts;

  if (ether_stats_var == ETHERSTATSFRAGMENTS)
    *value = p_db_stats->fragments;

  if (ether_stats_var == ETHERSTATSJABBERS)
    *value = p_db_stats->jabbers;

  if (ether_stats_var == ETHERSTATSCOLLISIONS)
    *value = p_db_stats->collisions;

  if (ether_stats_var == ETHERSTATSPKTS64OCTETS)
    *value = p_db_stats->pkts_64_octets;

  if (ether_stats_var == ETHERSTATSPKTS65TO127OCTETS)
    *value = p_db_stats->pkts_65to127_octets;

  if (ether_stats_var == ETHERSTATSPKTS128TO255OCTETS)
    *value = p_db_stats->pkts_128to255_octets;

  if (ether_stats_var == ETHERSTATSPKTS256TO511OCTETS)
    *value = p_db_stats->pkts_256to511_octets;

  if (ether_stats_var == ETHERSTATSPKTS512TO1023OCTETS)
    *value = p_db_stats->pkts_512to1023_octets;

  if (ether_stats_var == ETHERSTATSPKTS1024TO1518OCTETS)
    *value = p_db_stats->pkts_1024to1518_octets;

  return;
}
   
void
rmon_alarm_timer_handler(void *p_data)
{
    tbl_rmon_alarm_t *p_db_alarm = (tbl_rmon_alarm_t*)p_data;
    tbl_rmon_event_t *p_db_rising_event  = NULL;
    tbl_rmon_event_t *p_db_falling_event = NULL;
    tbl_rmon_event_key_t event_key;
    tbl_rmon_ether_stats_key_t stats_key;
    tbl_rmon_ether_stats_t *p_db_ether_stats = NULL;
    
    uint32 ether_stats_var = 0;
    int32  first_delta = FALSE;
    struct sal_timeval   tv;
    char   time[256] = {0};

    if (!p_db_alarm)
        return;

    if (p_db_alarm->t_alarm_timer)
    {
        p_db_alarm->t_alarm_timer = NULL;
    }

    sal_memset(&event_key, 0, sizeof(event_key));
    sal_memset(&stats_key, 0, sizeof(event_key));

    stats_key.index = p_db_alarm->variable[ETHER_STATS_INDEX_POS];
    p_db_ether_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&stats_key);

    event_key.index = p_db_alarm->rising_event_index;
    p_db_rising_event = tbl_rmon_event_get_rmon_event(&event_key);
    event_key.index = p_db_alarm->falling_event_index;
    p_db_falling_event = tbl_rmon_event_get_rmon_event(&event_key);

    if (!p_db_ether_stats || !p_db_rising_event || !p_db_falling_event)
    {
        RMON_LOG_DEBUG(APPDBG_FLAG_RMON_ALARM, "RMON ALARM : Alarm %d under creation status because of invalid etherStatsTable or eventTable", 
                       p_db_alarm->key.index);

        p_db_alarm->t_alarm_timer = 
                CTC_TASK_ADD_TIME(rmon_alarm_timer_handler, p_db_alarm, p_db_alarm->interval);

        p_db_alarm->status = UNDER_CREATION_STATUS;
        p_db_alarm->last_event = RMON_NO_ALARM;
        p_db_alarm->value = 0;
        tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STATUS);
        tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_LAST_EVENT);
        tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_VALUE);
        return; 
    }
    else
    {
        if (p_db_alarm->status == UNDER_CREATION_STATUS)
        {
            p_db_alarm->status = VALID_STATUS;
            p_db_alarm->startup_alarm = RISING_OR_FALLING_ALARM;
            tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STATUS);
            tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STARTUP_ALARM);
        }
    }

    ether_stats_var = p_db_alarm->variable[ETHER_STATS_INDEX_POS - 1];
    rmon_copy_alarm_value(p_db_ether_stats, ether_stats_var, &p_db_alarm->curr_value);

    /* if the statistic is less than previous */
    /* enable minus value so we dont process the flushed stats */
    /* if (p_db_alarm->prev_value > p_db_alarm->curr_value)
    {
        is_flush = TRUE;
    } */

    if (p_db_alarm->sample_type == RMON_ALARM_DELTA)
    {
        p_db_alarm->value = p_db_alarm->curr_value - p_db_alarm->prev_value;
        if (p_db_alarm->startup_alarm == RISING_OR_FALLING_ALARM)
        {
            first_delta = TRUE;
            p_db_alarm->startup_alarm = 0;
        }
        p_db_alarm->prev_value = p_db_alarm->curr_value;
    }
    else if ((p_db_alarm->sample_type == RMON_ALARM_ABS))
    {
        p_db_alarm->value = p_db_alarm->curr_value;
    }
    tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_VALUE);

    if (first_delta)
    {
        RMON_LOG_DEBUG(APPDBG_FLAG_RMON_ALARM, "RMON ALARM : Not trige event for the start up delta alarm %d value %"PRId64"", 
                       p_db_alarm->key.index, p_db_alarm->value);
        /* the value is the first one for delta sample type, ignore first data */
        p_db_alarm->t_alarm_timer = 
                CTC_TASK_ADD_TIME(rmon_alarm_timer_handler, p_db_alarm, p_db_alarm->interval);

        return;
    }

    ctc_task_update_boottime(&tv);
    if (p_db_alarm->rising_threshold >= 0)
    {
        if ((p_db_alarm->last_event != RMON_RISE_ALARM) && p_db_alarm->value >= p_db_alarm->rising_threshold)
        {
            /* Trigger an event */
            if (p_db_rising_event->type == RMON_EVENT_LOG || p_db_rising_event->type == RMON_EVENT_LOG_TRAP)
            {
                logid_sys(LOG_RMON_4_RISING_ALARM, p_db_alarm->key.index, p_db_alarm->rising_threshold, 
                                                   p_db_alarm->value, p_db_alarm->rising_event_index);
            }
            if (p_db_rising_event->type == RMON_EVENT_TRAP || p_db_rising_event->type == RMON_EVENT_LOG_TRAP)
            {
                snmp_rmon_alarm_trap(p_db_alarm, RMON_RISE_ALARM);
            }
            p_db_alarm->last_event = RMON_RISE_ALARM;
            tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_LAST_EVENT);
            p_db_alarm->startup_alarm = RISING_ALARM;
            tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STARTUP_ALARM);
            p_db_rising_event->last_sent = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
            tbl_rmon_event_set_rmon_event_field(p_db_rising_event, TBL_RMON_EVENT_FLD_LAST_SENT);

            rmon_format_tv(p_db_falling_event->last_sent / 1000, time);
            RMON_LOG_DEBUG(APPDBG_FLAG_RMON_EVENT, "RMON EVENT : Event %d last send on %s", 
                           p_db_rising_event->key.index, time);
            RMON_LOG_DEBUG(APPDBG_FLAG_RMON_ALARM, "RMON ALARM : Alarm %d value %"PRId64" triged rising event %d", 
                           p_db_alarm->key.index, p_db_alarm->value, p_db_alarm->rising_event_index);
        }
    }

    if (p_db_alarm->falling_threshold >= 0)
    {
        if ((p_db_alarm->last_event != RMON_FALL_ALARM) && p_db_alarm->value <= p_db_alarm->falling_threshold)
        {
            /* Trigger an event */
            if (p_db_falling_event->type == RMON_EVENT_LOG || p_db_falling_event->type == RMON_EVENT_LOG_TRAP)
            {
                logid_sys(LOG_RMON_4_FALLING_ALARM, p_db_alarm->key.index, p_db_alarm->falling_threshold, 
                                                   p_db_alarm->value, p_db_alarm->falling_event_index);
            }
            if (p_db_falling_event->type == RMON_EVENT_TRAP || p_db_falling_event->type == RMON_EVENT_LOG_TRAP)
            {
                snmp_rmon_alarm_trap(p_db_alarm, RMON_FALL_ALARM);
            }
            p_db_alarm->last_event = RMON_FALL_ALARM;
            tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_LAST_EVENT);
            p_db_alarm->startup_alarm = FALLING_ALARM;
            tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STARTUP_ALARM);
            p_db_falling_event->last_sent = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
            tbl_rmon_event_set_rmon_event_field(p_db_falling_event, TBL_RMON_EVENT_FLD_LAST_SENT);

            rmon_format_tv(p_db_falling_event->last_sent / 1000, time);
            RMON_LOG_DEBUG(APPDBG_FLAG_RMON_EVENT, "RMON EVENT : Event %d last send on %s", 
                           p_db_falling_event->key.index, time);

            RMON_LOG_DEBUG(APPDBG_FLAG_RMON_ALARM, "RMON ALARM : Alarm %d value %"PRId64" triged falling event %d", 
                           p_db_alarm->key.index, p_db_alarm->value, p_db_alarm->rising_event_index);
        }
    }

    p_db_alarm->t_alarm_timer = 
                CTC_TASK_ADD_TIME(rmon_alarm_timer_handler, p_db_alarm, p_db_alarm->interval);

    return;
}

void
rmon_event_under_creation_timer(void *p_data)
{
    tbl_rmon_event_t *p_db_event = (tbl_rmon_event_t*)p_data;

    if (!p_db_event)
    {
        return;
    }

    CTC_TASK_STOP_TIMER(p_db_event->t_under_creation_timer);

    tbl_rmon_event_del_rmon_event(&p_db_event->key);
    return;
}

void
rmon_alarm_under_creation_timer(void *p_data)
{
    tbl_rmon_alarm_t *p_db_alarm = (tbl_rmon_alarm_t*)p_data;

    if (!p_db_alarm)
    {
        return;
    }

    CTC_TASK_STOP_TIMER(p_db_alarm->t_under_creation_timer);
    CTC_TASK_STOP_TIMER(p_db_alarm->t_alarm_timer);

    tbl_rmon_alarm_del_rmon_alarm(&p_db_alarm->key);
    return;
}

void
rmon_ether_under_creation_timer(void *p_data)
{
    tbl_rmon_ether_stats_t *p_db_stats = (tbl_rmon_ether_stats_t*)p_data;

    if (!p_db_stats)
    {
        return;
    }

    CTC_TASK_STOP_TIMER(p_db_stats->t_under_creation_timer);

    tbl_rmon_ether_stats_del_rmon_ether_stats(&p_db_stats->key);

    return;
}

void
rmon_history_under_creation_timer(void *p_data)
{
    tbl_rmon_history_t *p_db_his = (tbl_rmon_history_t*)p_data;

    if (!p_db_his)
    {
        return;
    }

    CTC_TASK_STOP_TIMER(p_db_his->t_under_creation_timer);
    CTC_TASK_STOP_TIMER(p_db_his->t_coll_history_timer);

    tbl_rmon_history_del_rmon_history(&p_db_his->key);

    return;
}


int32 
rmon_api_update_ether_stats(tbl_interface_t *p_db_if, tbl_port_stats_t *p_port_stats, uint32 field_id)
{
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;
    tbl_rmon_ether_stats_t *p_db_stats = NULL;
    tbl_rmon_ether_stats_master_t *p_stats_master = tbl_rmon_ether_stats_get_master();

    if (NULL == p_stats_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_stats_master->rmon_ether_stats_list, p_db_stats, listnode)
    {
        if (p_db_stats->data_source_ifindex == p_db_if->ifindex)
        {
            /* field_id always be TBL_PORT_STATS_FLD_MAX */
            p_db_stats->drop_events = p_port_stats->drop_events;
            p_db_stats->octets = p_port_stats->octets_rcv;
            p_db_stats->pkts = p_port_stats->pkts_rcv;
            p_db_stats->broadcast_pkts = p_port_stats->brdc_pkts_rcv;
            p_db_stats->multicast_pkts = p_port_stats->mc_pkts_rcv;
            p_db_stats->crc_align_errors = p_port_stats->bad_crc;
            p_db_stats->undersize_pkts = p_port_stats->undersize_pkts;
            p_db_stats->oversize_pkts = p_port_stats->oversize_pkts;
            p_db_stats->fragments = p_port_stats->fragments_pkts;
            p_db_stats->jabbers = p_port_stats->jabber_pkts;
            p_db_stats->collisions = p_port_stats->fragments_pkts;
            p_db_stats->pkts_64_octets = p_port_stats->pkts_64_rcv;
            p_db_stats->pkts_65to127_octets = p_port_stats->pkts_65_to_127_rcv;
            p_db_stats->pkts_128to255_octets = p_port_stats->pkts_128_to_255_rcv;
            p_db_stats->pkts_256to511_octets = p_port_stats->pkts_256_to_511_rcv;
            p_db_stats->pkts_512to1023_octets = p_port_stats->pkts_512_to_1023_rcv;
            p_db_stats->pkts_1024to1518_octets = p_port_stats->pkts_1024_to_1518_rcv;

            PM_E_RETURN(tbl_rmon_ether_stats_set_rmon_ether_stats_field(p_db_stats, TBL_RMON_ETHER_STATS_FLD_MAX));
        }
    }

    return rc;
}


int32 
rmon_store_sample_stats(tbl_rmon_history_t *p_db_his)
{
    int32 rc = PM_E_NONE;
    tbl_rmon_stats_history_t      stats_his;
    tbl_rmon_stats_history_t     *p_db_stats_his = NULL;
    tbl_rmon_stats_history_t      prev_stats_his;
    tbl_rmon_stats_history_t     *p_prev_stats_his = &prev_stats_his;
    tbl_port_stats_key_t          if_stats;
    tbl_port_stats_t             *p_db_port_stats = NULL;
    tbl_port_stats_rate_key_t     if_stats_rate;
    tbl_port_stats_rate_t        *p_db_port_stats_rate = NULL;
    struct sal_timeval   tv;

    sal_memset(&stats_his, 0, sizeof(stats_his));
    sal_memset(&if_stats, 0, sizeof(if_stats));
    sal_memset(&if_stats_rate, 0, sizeof(if_stats_rate));
    sal_memset(&prev_stats_his, 0, sizeof(prev_stats_his));

    stats_his.key.history_index = p_db_his->key.index;
    stats_his.key.stats_index = p_db_his->current_sample_no;

    if_stats.ifindex = p_db_his->data_source;
    p_db_port_stats = tbl_port_stats_get_port_stats(&if_stats);
    if (!p_db_port_stats)
    {
        return rc;
    }

    if_stats_rate.ifindex = p_db_his->data_source;
    p_db_port_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&if_stats_rate);
    if (!p_db_port_stats_rate)
    {
        return rc;
    }

    if (p_db_his->current_sample_no > 1)
    {
        prev_stats_his.key.history_index = p_db_his->key.index;
        prev_stats_his.key.stats_index = p_db_his->current_sample_no - 1;
        p_prev_stats_his = tbl_rmon_stats_history_get_rmon_stats_history(&prev_stats_his.key);
        if (!p_prev_stats_his)
        {
            p_prev_stats_his = &prev_stats_his;
        }
    }

    /* drop_events */
    if (p_db_port_stats->drop_events < p_prev_stats_his->total_drop_events)
    {
        stats_his.drop_events = p_db_port_stats->drop_events;
    }
    else
    {
        stats_his.drop_events = p_db_port_stats->drop_events - p_prev_stats_his->total_drop_events;
    }
    stats_his.total_drop_events = p_db_port_stats->drop_events;
    /* octets */
    if (p_db_port_stats->octets_rcv < p_prev_stats_his->total_octets)
    {
        stats_his.octets = p_db_port_stats->octets_rcv;
    }
    else
    {
        stats_his.octets = p_db_port_stats->octets_rcv - p_prev_stats_his->total_octets;
    }
    stats_his.total_octets = p_db_port_stats->octets_rcv;
    /* pkts */
    if (p_db_port_stats->pkts_rcv < p_prev_stats_his->total_pkts)
    {
        stats_his.pkts = p_db_port_stats->pkts_rcv;
    }
    else
    {
        stats_his.pkts = p_db_port_stats->pkts_rcv - p_prev_stats_his->total_pkts;
    }
    stats_his.total_pkts = p_db_port_stats->pkts_rcv;
    /* BroadcastPkts */
    if (p_db_port_stats->brdc_pkts_rcv < p_prev_stats_his->total_broadcast_pkts)
    {
        stats_his.broadcast_pkts = p_db_port_stats->brdc_pkts_rcv;
    }
    else
    {
        stats_his.broadcast_pkts = p_db_port_stats->brdc_pkts_rcv - p_prev_stats_his->total_broadcast_pkts;
    }
    stats_his.total_broadcast_pkts = p_db_port_stats->brdc_pkts_rcv;
    /* MulticastPkts */
    if (p_db_port_stats->mc_pkts_rcv < p_prev_stats_his->total_multicast_pkts)
    {
        stats_his.multicast_pkts = p_db_port_stats->mc_pkts_rcv;
    }
    else
    {
        stats_his.multicast_pkts = p_db_port_stats->mc_pkts_rcv - p_prev_stats_his->total_multicast_pkts;
    }
    stats_his.total_multicast_pkts = p_db_port_stats->mc_pkts_rcv;
    /* CRCAlignErrors */
    if (p_db_port_stats->bad_crc < p_prev_stats_his->total_crc_align_errors)
    {
        stats_his.crc_align_errors = p_db_port_stats->bad_crc;
    }
    else
    {
        stats_his.crc_align_errors = p_db_port_stats->bad_crc - p_prev_stats_his->total_crc_align_errors;
    }
    stats_his.total_crc_align_errors = p_db_port_stats->bad_crc;
    /* UndersizePkts */
    if (p_db_port_stats->undersize_pkts < p_prev_stats_his->total_undersize_pkts)
    {
        stats_his.undersize_pkts = p_db_port_stats->undersize_pkts;
    }
    else
    {
        stats_his.undersize_pkts = p_db_port_stats->undersize_pkts - p_prev_stats_his->total_undersize_pkts;
    }
    stats_his.total_undersize_pkts = p_db_port_stats->undersize_pkts;
    /* OversizePkts */
    if (p_db_port_stats->oversize_pkts < p_prev_stats_his->total_oversize_pkts)
    {
        stats_his.oversize_pkts = p_db_port_stats->oversize_pkts;
    }
    else
    {
        stats_his.oversize_pkts = p_db_port_stats->oversize_pkts - p_prev_stats_his->total_oversize_pkts;
    }
    stats_his.total_oversize_pkts = p_db_port_stats->oversize_pkts;
    /* Fragments */
    if (p_db_port_stats->fragments_pkts < p_prev_stats_his->total_fragments)
    {
        stats_his.fragments = p_db_port_stats->fragments_pkts;
    }
    else
    {
        stats_his.fragments = p_db_port_stats->fragments_pkts - p_prev_stats_his->total_fragments;
    }
    stats_his.total_fragments = p_db_port_stats->fragments_pkts;
    /* Jabbers */
    if (p_db_port_stats->jabber_pkts < p_prev_stats_his->total_jabbers)
    {
        stats_his.jabbers = p_db_port_stats->jabber_pkts;
    }
    else
    {
        stats_his.jabbers = p_db_port_stats->jabber_pkts - p_prev_stats_his->total_jabbers;
    }
    stats_his.total_jabbers = p_db_port_stats->jabber_pkts;
    /* Collisions */
    if (p_db_port_stats->fragments_pkts < p_prev_stats_his->total_collisions)
    {
        stats_his.collisions = p_db_port_stats->fragments_pkts;
    }
    else
    {
        stats_his.collisions = p_db_port_stats->fragments_pkts - p_prev_stats_his->total_collisions;
    }
    stats_his.total_collisions = p_db_port_stats->fragments_pkts; 
    /* Get utilization from port_stats_rate */
    stats_his.utilization = p_db_port_stats_rate->input_rate > 100 ? 100 : p_db_port_stats_rate->input_rate;

    //sal_gettimeofday(&tv);
    ctc_task_update_boottime(&tv);
    stats_his.interval_start = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;

    PM_E_RETURN(tbl_rmon_stats_history_add_rmon_stats_history(&stats_his));

    if (p_db_his->current_sample_no > p_db_his->buckets_granted)
    {
        sal_memset(&stats_his, 0, sizeof(stats_his));
        stats_his.key.history_index = p_db_his->key.index;
        stats_his.key.stats_index = p_db_his->current_sample_no - p_db_his->buckets_granted;

        p_db_stats_his = tbl_rmon_stats_history_get_rmon_stats_history(&stats_his.key);
        if (p_db_stats_his)
        {
            PM_E_RETURN(tbl_rmon_stats_history_del_rmon_stats_history(&stats_his.key));
        }
    }

    return rc;
}

void
rmon_coll_history_timer_handler(void *p_data)
{
    tbl_rmon_history_t    *p_db_his = (tbl_rmon_history_t*)p_data;

    if (!p_db_his)
    {
        return;
    }
    if (p_db_his->t_coll_history_timer)
    {
        p_db_his->t_coll_history_timer = NULL;
    }

    rmon_store_sample_stats(p_db_his);

    p_db_his->current_sample_no++;
    tbl_rmon_history_set_rmon_history_field(p_db_his, TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO);

    RMON_LOG_DEBUG(APPDBG_FLAG_RMON_HISTORY, "RMON HISTORY : History %d saved statsHistoryTable index %d", 
                   p_db_his->key.index, p_db_his->current_sample_no - 1);

    p_db_his->t_coll_history_timer = 
            CTC_TASK_ADD_TIME(rmon_coll_history_timer_handler, p_db_his, p_db_his->interval);

    return;
}


void 
rmon_oid_to_string(uint32 *oidval, char *variable)
{
    int i = 0;
    
    for(i = 0; i < ETHER_STATS_OID_SIZE + 2; i++)
    {
         char tmpval[32] = {0};
         if (i != 11)
         {
             sal_snprintf(tmpval, sizeof(tmpval), "%d", oidval[i]);   
         }
         else
         {
             sal_snprintf(tmpval, sizeof(tmpval), "%d", oidval[i]);   
         }
         
         sal_strcat(variable, tmpval);
    }

    return;
}

uint32
rmon_event_get_count()
{
    tbl_rmon_event_master_t *p_event_master = tbl_rmon_event_get_master();

    if (NULL == p_event_master)
    {
        return PM_E_NONE;
    }

    return p_event_master->rmon_event_list->count;
}

uint32
rmon_alarm_get_count()
{
    tbl_rmon_alarm_master_t *p_alarm_master = tbl_rmon_alarm_get_master();

    if (NULL == p_alarm_master)
    {
        return PM_E_NONE;
    }

    return p_alarm_master->rmon_alarm_list->count;
}

int32
rmon_snmp_set_event_status (tbl_rmon_event_t *p_db_event, uint32 status)
{
    int32 rc = PM_E_NONE;

    switch (status)
    {
    case VALID_STATUS:
        if (!p_db_event)
        {
            return RMON_API_SET_FAILURE;
        }
        p_db_event->status = VALID_STATUS;
        PM_E_RETURN(tbl_rmon_event_set_rmon_event_field(p_db_event, TBL_RMON_EVENT_FLD_STATUS));
        CTC_TASK_STOP_TIMER(p_db_event->t_under_creation_timer);
        return RMON_API_SET_SUCCESS;
        break;

    case CREATE_REQ_STATUS:
        /* for snmp rmon event create request */
        if (p_db_event) /* Same entry already present */
        {
            return RMON_API_SET_FAILURE;
        }
        //rmon_api_add_event(REQ_CREATE);
        return RMON_API_SET_SUCCESS;
        break;

    case UNDER_CREATION_STATUS:
        if (!p_db_event)
        {
            return RMON_API_SET_FAILURE;
        }

        if (p_db_event->status == VALID_STATUS || p_db_event->status == UNDER_CREATION_STATUS)
        {
            /* start underCreationTimer */
            if (!p_db_event->t_under_creation_timer)
            {
                p_db_event->t_under_creation_timer = 
                    CTC_TASK_ADD_TIME(rmon_event_under_creation_timer, p_db_event, RMON_ROW_AGING_TIME);
            }
            p_db_event->status = UNDER_CREATION_STATUS;
            PM_E_RETURN(tbl_rmon_event_set_rmon_event_field(p_db_event, TBL_RMON_EVENT_FLD_STATUS));
        }
        return RMON_API_SET_FAILURE;
        break;

    case INVALID_STATUS:
        if (!p_db_event)
        {
            return RMON_API_SET_FAILURE;
        }

        p_db_event->status = INVALID_STATUS;
        CTC_TASK_STOP_TIMER(p_db_event->t_under_creation_timer);
        tbl_rmon_event_del_rmon_event(&p_db_event->key);
        return RMON_API_SET_SUCCESS;
        break;
    }

    return rc;
}

int32
rmon_event_set_active(tbl_rmon_event_t *p_db_event)
{
    int32     rc = PM_E_NONE;
    int32 status = VALID_STATUS;

    PM_E_RETURN(rmon_snmp_set_event_status(p_db_event, status));

    return rc;
}

int32
rmon_set_alarm_variable (tbl_rmon_alarm_t *p_alarm, oid_t *name)
{
    int32 rc = PM_E_NONE;
    uint32 ether_stats_var = 0;
    oid_t  ether_stats_oid[] = {1, 3, 6, 1, 2, 1, 16, 1, 1, 1};

    if (!p_alarm)
        return RMON_API_SET_FAILURE;

    if (sal_memcmp(name, ether_stats_oid, sizeof(ether_stats_oid))) 
        return RMON_API_SET_FAILURE;    

    ether_stats_var = name[ETHER_STATS_INDEX_POS - 1];

    if (ether_stats_var > ETHERSTATSSTATUS)
        return RMON_API_SET_FAILURE;

    sal_memcpy (p_alarm->variable, name, sizeof(p_alarm->variable));
    rmon_oid_to_string(p_alarm->variable, p_alarm->variable_word);   

    return rc;
}

int32
rmon_set_alarm_variableword(tbl_rmon_alarm_t *p_alarm, char *Variable)
{
    int32 rc = PM_E_NONE;
    char *tempStr = NULL;
    int  ether_stats_var;
    if(!p_alarm || !Variable)
        return RMON_API_SET_FAILURE;

    tempStr = sal_strchr(Variable,'.');
    ++tempStr;
    ether_stats_var = sal_atoi(tempStr);
    tempStr = sal_strchr(tempStr,'.');

    switch(ether_stats_var)
    {
    case ETHERSTATSDROPEVENTS:
         sal_memcpy(p_alarm->variable_word, ETHERSTATSDROPEVENTSVSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSOCTETS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSOCTETSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSPKTSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSBROADCASTPKTS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSBDCPKTSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSMULTICASTPKTS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSMCPKTSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSCRCALIGNERRORS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSCRCALIGNERRSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSUNDERSIZEPKTS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSUSZPKTSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSOVERSIZEPKTS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSOVSZPKTSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSFRAGMENTS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSFRGSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;
    case ETHERSTATSJABBERS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSJABBERSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSCOLLISIONS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSCOLLISIONSSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS64OCTETS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSPKT64OCTSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS65TO127OCTETS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSPKT65TO127OCTSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS128TO255OCTETS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSPKT128TO255OCTSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS256TO511OCTETS:
         sal_memcpy(p_alarm->variable_word ,ETHERSTATSPKT256TO511OCTSTR,
                    RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS512TO1023OCTETS:
         sal_memcpy(p_alarm->variable_word ,
                    ETHERSTATSPKT512TO1023OCTSTR ,RMON_ALARM_VAR_WORD_LENGTH);
         break;

    case ETHERSTATSPKTS1024TO1518OCTETS:
         sal_memcpy(p_alarm->variable_word ,
                    ETHERSTATSPKT1024TO15180OCTSTR,RMON_ALARM_VAR_WORD_LENGTH);
         break;

    default :
         return RMON_API_VARIABLE_SET_FAILURE;
         break;
    }

    sal_strcat(p_alarm->variable_word, tempStr);

    return rc;
}


int32
rmon_get_ether_stats_var(char *Variable, oid_t *ether_stats_var)
{
    int32 rc = PM_E_FAIL;

    if (NULL == Variable)
        return PM_E_FAIL;
#if 0
    if (!sal_strncmp(Variable, ETHERSTATSINDEXSTR, strlen(ETHERSTATSINDEXSTR)))
    {
        *ether_stats_var = ETHERSTATSINDEX;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSDSRCSTR, strlen(ETHERSTATSDSRCSTR)))
    {
        *ether_stats_var = ETHERSTATSDATASOURCE;
        return RESULT_OK;
    }
#endif
    if (!sal_strncmp(Variable, ETHERSTATSDROPEVENTSVSTR,
                                     strlen(ETHERSTATSDROPEVENTSVSTR)))
    {
        *ether_stats_var = ETHERSTATSDROPEVENTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSOCTETSSTR, strlen(ETHERSTATSOCTETSSTR)))
    {
        *ether_stats_var = ETHERSTATSOCTETS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKTSSTR, strlen(ETHERSTATSPKTSSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSBDCPKTSSTR,
                                     strlen(ETHERSTATSBDCPKTSSTR)))
    {
        *ether_stats_var = ETHERSTATSBROADCASTPKTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSMCPKTSSTR, strlen(ETHERSTATSMCPKTSSTR)))
    {
        *ether_stats_var = ETHERSTATSMULTICASTPKTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSCRCALIGNERRSTR,
                                     strlen(ETHERSTATSCRCALIGNERRSTR)))
    {
        *ether_stats_var = ETHERSTATSCRCALIGNERRORS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSUSZPKTSSTR,
                                     strlen(ETHERSTATSUSZPKTSSTR)))
    {
        *ether_stats_var = ETHERSTATSUNDERSIZEPKTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSOVSZPKTSSTR,
                                     strlen(ETHERSTATSOVSZPKTSSTR)))
    {
        *ether_stats_var = ETHERSTATSOVERSIZEPKTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSFRGSSTR, strlen(ETHERSTATSFRGSSTR)))
    {
        *ether_stats_var = ETHERSTATSFRAGMENTS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSJABBERSSTR,strlen(ETHERSTATSJABBERSSTR)))
    {
        *ether_stats_var = ETHERSTATSJABBERS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSCOLLISIONSSTR,
                                     strlen(ETHERSTATSCOLLISIONSSTR)))
    {
        *ether_stats_var = ETHERSTATSCOLLISIONS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKT64OCTSTR,
                                     strlen(ETHERSTATSPKT64OCTSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS64OCTETS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKT65TO127OCTSTR,
                                     strlen(ETHERSTATSPKT65TO127OCTSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS65TO127OCTETS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKT128TO255OCTSTR,
                                     strlen(ETHERSTATSPKT128TO255OCTSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS128TO255OCTETS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKT256TO511OCTSTR,
                                     strlen(ETHERSTATSPKT256TO511OCTSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS256TO511OCTETS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKT512TO1023OCTSTR,
                                     strlen(ETHERSTATSPKT512TO1023OCTSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS512TO1023OCTETS;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSPKT1024TO15180OCTSTR,
                                     strlen(ETHERSTATSPKT1024TO15180OCTSTR)))
    {
        *ether_stats_var = ETHERSTATSPKTS1024TO1518OCTETS;
        return RESULT_OK;
    }
#if 0
    if (!sal_strncmp(Variable, ETHERSTATSOWNERSTR,
                                     strlen(ETHERSTATSOWNERSTR)))
    {
        *ether_stats_var = ETHERSTATSOWNER;
        return RESULT_OK;
    }
    if (!sal_strncmp(Variable, ETHERSTATSSTATUSSTR,
                                     strlen(ETHERSTATSSTATUSSTR)))
    {
        *ether_stats_var = ETHERSTATSSTATUS;
        return RESULT_OK;
    }
#endif
    return rc;
}

int32
rmon_api_alarm_add(tbl_rmon_alarm_t *p_alarm, char *Variable, cfg_cmd_para_t *para, uint32 alarmStartupAlarm)
{
    int32 rc  = PM_E_NONE;
    uint32 k =0,j=0;
    int32 etherstatsvar = 0;
    char *tempStr = NULL;
    oid_t oidval[] = {RMONMIBOID,1,1,1,0,0};

    if (!sal_strncmp(ETHERSTATSENTRY, Variable, strlen (ETHERSTATSENTRY)))
    {
        tempStr = sal_strchr(Variable, '.');

        if (!tempStr)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }

        for (j = 0, k = 10; j < 2; j++, k++)
        {
            tempStr++;
            if (!rmon_check_oid_varlen(tempStr))
            {
                CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
            }
            
            oidval[k] = sal_atoi(tempStr);
            tempStr = sal_strchr(tempStr, '.');
            if (!tempStr && 11 != k)
            {
                CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
            }
        }

        if (oidval[10] < 3 || oidval[10] > 19)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }

        if (tempStr != NULL)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }

        /* Variable is in the format "etherStatsEntry.x.x" */
        etherstatsvar = RMON_ETHER_STATS_ENTRY;
    }
    else if(!sal_strncmp(ETHERSTATSNUM, Variable, strlen (ETHERSTATSNUM)))
    {
        tempStr = Variable;
        oidval[0] = sal_atoi(tempStr);
        tempStr = sal_strchr(tempStr, '.');
        for (j = 1; j < 12; j++)
        {
            tempStr++;
            if (!rmon_check_oid_varlen(tempStr))
            {
                CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
            }

            oidval[j] = sal_atoi(tempStr);
            tempStr = sal_strchr(tempStr, '.');
            if (!tempStr && 11 != j)
            {
                CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
            }
        }

        if (oidval[10] < 3 || oidval[10] > 19)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }

        if (tempStr != NULL)
        {            
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }
        etherstatsvar = RMON_ETHER_STATS_VARIABLE;
    }
    else
    {
        tempStr = sal_strchr(Variable,'.');

        if (tempStr == NULL)
        {            
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }
        tempStr++;
        if (!rmon_check_oid_varlen(tempStr))
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }

        oidval[11] = sal_atoi(tempStr);

        rc = rmon_get_ether_stats_var(Variable, &oidval[10]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }
        /* Variable is in the format "etherStatsOctets.x" */
        etherstatsvar = RMON_ETHER_STATS_VARIABLE;

        tempStr = sal_strchr(tempStr, '.');
        if (tempStr != NULL)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }
    }

    if (oidval[11] > RMON_INDEX_MAX || oidval[11] < RMON_INDEX_MIN)
    {
        CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
    }

    if (RMON_ETHER_STATS_VARIABLE == etherstatsvar)
    {
        sal_memcpy(p_alarm->variable_word, Variable, strlen(Variable));
    }
    else
    {
        if (rmon_set_alarm_variableword(p_alarm, Variable) < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid rmon alarm variable %s", Variable);
        }
    }

    uint32 i =0;
    for(i = 0; i < RMON_ALARM_VARIABLE_MAXSIZE; ++i)
    {
        p_alarm->variable[i] = oidval[i];
    }

    p_alarm->last_event = RMON_NO_ALARM;     
    p_alarm->status = UNDER_CREATION_STATUS;
    p_alarm->startup_alarm = alarmStartupAlarm;
         
    PM_E_RETURN(tbl_rmon_alarm_add_rmon_alarm(p_alarm));
    
    return rc;
}

int32
rmon_alarm_set_active(tbl_rmon_alarm_t *p_db_alarm)
{
    int32 rc = PM_E_NONE;

    if (!p_db_alarm)
        return RESULT_ERROR;

    if (p_db_alarm->interval <= 0)
        return RESULT_ERROR;

    p_db_alarm->status = VALID_STATUS;
    PM_E_RETURN(tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STATUS));

    p_db_alarm->t_alarm_timer = 
                CTC_TASK_ADD_TIME(rmon_alarm_timer_handler, p_db_alarm, p_db_alarm->interval);
    return rc;
}

int32
rmon_alarm_set_inactive(tbl_rmon_alarm_t *p_db_alarm)
{
    int32 rc = PM_E_NONE;

    if (!p_db_alarm)
        return RESULT_ERROR;

    p_db_alarm->status = INVALID_STATUS;
    PM_E_RETURN(tbl_rmon_alarm_set_rmon_alarm_field(p_db_alarm, TBL_RMON_ALARM_FLD_STATUS));

    CTC_TASK_STOP_TIMER(p_db_alarm->t_alarm_timer);

    return rc;
}

int32
rmon_api_stats_check_if(uint32 ifindex)
{
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_ether_stats_t *p_db_stats = NULL;
    tbl_rmon_ether_stats_master_t *p_stats_master = tbl_rmon_ether_stats_get_master();

    if (NULL == p_stats_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_stats_master->rmon_ether_stats_list, p_db_stats, listnode, next)
    {
        if (p_db_stats->data_source_ifindex == ifindex)
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}

int32
rmon_snmp_set_ether_stats_status (tbl_rmon_ether_stats_t *p_db_stats, uint32 status)
{
    int32 rc = PM_E_NONE;

    switch (status)
    {
    case VALID_STATUS:
        if (!p_db_stats)
        {
            return RMON_API_SET_FAILURE;
        }

        if (p_db_stats->data_source_ifindex == 0)
        {
            return RMON_API_SET_FAILURE;
        }

        p_db_stats->status = VALID_STATUS;
        PM_E_RETURN(tbl_rmon_ether_stats_set_rmon_ether_stats_field(p_db_stats, TBL_RMON_ETHER_STATS_FLD_STATUS));
        CTC_TASK_STOP_TIMER(p_db_stats->t_under_creation_timer);

        return RMON_API_SET_SUCCESS;
        break;

    case CREATE_REQ_STATUS:
        if (p_db_stats)
        {
            return RMON_API_SET_FAILURE;
        }

        //rmon_api_add_stats(REQ_CREATE);

        return RMON_API_SET_SUCCESS;
        break;

    case UNDER_CREATION_STATUS:
        if (!p_db_stats)
        {
            return RMON_API_SET_FAILURE;
        }

        if (p_db_stats->status == VALID_STATUS || p_db_stats->status == UNDER_CREATION_STATUS)
        {
            /* start underCreationTimer */
            if (!p_db_stats->t_under_creation_timer)
            {
                p_db_stats->t_under_creation_timer = 
                    CTC_TASK_ADD_TIME(rmon_ether_under_creation_timer, p_db_stats, RMON_ROW_AGING_TIME);
            }
            p_db_stats->status = UNDER_CREATION_STATUS;
            PM_E_RETURN(tbl_rmon_ether_stats_set_rmon_ether_stats_field(p_db_stats, TBL_RMON_ETHER_STATS_FLD_STATUS));
            return RMON_API_SET_SUCCESS;
        }
        return RMON_API_SET_FAILURE;
        break;

    case INVALID_STATUS:
        if (!p_db_stats)
        {
            return RMON_API_SET_FAILURE;
        }

        CTC_TASK_STOP_TIMER(p_db_stats->t_under_creation_timer);
        PM_E_RETURN(tbl_rmon_ether_stats_del_rmon_ether_stats(&p_db_stats->key));
        return RMON_API_SET_SUCCESS;
        break;
    }

    return RMON_API_SET_FAILURE;
}

int32
rmon_api_ether_stats_add(tbl_interface_t *p_db_if, tbl_rmon_ether_stats_t *p_stats)
{
    int32 rc = PM_E_NONE;
    tbl_rmon_ether_stats_t *p_db_stats = NULL;
    oid_t interface_oid[] = {MIB2IFGROUPOID, 2, 1, 1, 0};
    uint32 i = 0;

    p_stats->data_source_ifindex = p_db_if->ifindex;
    interface_oid[RMON_INTERFACE_INDEX_POSITION-1] = p_db_if->ifindex;
    for(i = 0; i < RMON_INTERFACE_INDEX_POSITION; ++i)
    {
        p_stats->data_source[i] = interface_oid[i];
    }

    PM_E_RETURN(tbl_rmon_ether_stats_add_rmon_ether_stats(p_stats));

    p_db_stats = tbl_rmon_ether_stats_get_rmon_ether_stats(&p_stats->key);
    if (!p_db_stats)
    {
        return PM_E_NO_MEMORY;
    }
    rmon_snmp_set_ether_stats_status(p_db_stats, VALID_STATUS);

    return rc;
}

int32
rmon_api_history_check_if(uint32 ifindex)
{
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_history_t *p_db_history = NULL;
    tbl_rmon_history_master_t *p_history_master = tbl_rmon_history_get_master();

    if (NULL == p_history_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_history_master->rmon_history_list, p_db_history, listnode, next)
    {
        if (p_db_history->data_source == ifindex)
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}

int32
rmon_history_set_active(tbl_rmon_history_t *p_db_history)
{
    int32 rc = PM_E_NONE;

    if (!p_db_history)
    {
        return PM_E_FAIL;
    }
    
    if (p_db_history->data_source == 0)
    {
        return PM_E_FAIL;
    }

    p_db_history->current_sample_no = 1;
    PM_E_RETURN(tbl_rmon_history_set_rmon_history_field(p_db_history, TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO));

    p_db_history->status = VALID_STATUS;
    PM_E_RETURN(tbl_rmon_history_set_rmon_history_field(p_db_history, TBL_RMON_HISTORY_FLD_STATUS));

    CTC_TASK_STOP_TIMER(p_db_history->t_coll_history_timer);
    p_db_history->t_coll_history_timer = 
            CTC_TASK_ADD_TIME(rmon_coll_history_timer_handler, p_db_history, p_db_history->interval);

    return rc;
}

int32
rmon_history_set_inactive(tbl_rmon_history_t *p_db_history)
{
    int32 rc = PM_E_NONE;
    tbl_rmon_stats_history_master_t *p_statshis_master = tbl_rmon_stats_history_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_stats_history_t *p_db_statshis = NULL;

    if (!p_db_history)
    {
        return PM_E_FAIL;
    }

    CTC_TASK_STOP_TIMER(p_db_history->t_coll_history_timer);
    CTC_TASK_STOP_TIMER(p_db_history->t_under_creation_timer);

    p_db_history->status = INVALID_STATUS;
    PM_E_RETURN(tbl_rmon_history_set_rmon_history_field(p_db_history, TBL_RMON_HISTORY_FLD_STATUS));

    CTCLIB_SLIST_LOOP_DEL(p_statshis_master->rmon_stats_history_list, p_db_statshis, listnode, next)
    {
        if (p_db_statshis->key.history_index == p_db_history->key.index)
        {
            tbl_rmon_stats_history_del_rmon_stats_history(&p_db_statshis->key);
        }
    }

    return rc;
}

int32
rmon_api_history_add(tbl_interface_t *p_db_if, tbl_rmon_history_t *p_history)
{
    int32 rc = PM_E_NONE;
    tbl_rmon_history_t *p_db_history = NULL;
    oid_t interface_oid[] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0};
    uint32 i =0;

    p_history->data_source = p_db_if->ifindex;
    interface_oid[RMON_INTERFACE_INDEX_POSITION-1] = p_db_if->ifindex;
    for(i = 0; i < RMON_INTERFACE_INDEX_POSITION; ++i)
    {
        p_history->data[i] = interface_oid[i];
    }

    PM_E_RETURN(tbl_rmon_history_add_rmon_history(p_history));

    p_db_history = tbl_rmon_history_get_rmon_history(&p_history->key);
    if (!p_db_history)
    {
        return PM_E_NO_MEMORY;
    }
    rmon_history_set_active(p_db_history);

    return rc;
}

int32
rmon_show_event(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_event_t *p_db_event)
{
    int32 rc = PM_E_NONE;
    char time[256] = {0};

    if (!p_db_event)
    {
        return rc;
    }

    if (p_db_event->status == VALID_STATUS)
    {
        sal_fprintf(fp, "Event Index = %d \n", p_db_event->key.index);
        sal_fprintf(fp, "   %-20s%s\n", "Description", p_db_event->desc);

        if (p_db_event->type == RMON_EVENT_NONE )
        {
            sal_fprintf(fp, "   %-20s%s\n", "Type", "None");
        }
        else if (p_db_event->type == RMON_EVENT_LOG )
        {
            sal_fprintf(fp, "   %-20s%s\n", "Type", "Log");
        }
        else if (p_db_event->type == RMON_EVENT_TRAP )
        {
            sal_fprintf(fp, "   %-20s%s\n", "Type", "SnmpTrap");
        }
        else if (p_db_event->type == RMON_EVENT_LOG_TRAP)
        {
            sal_fprintf(fp, "   %-20s%s\n", "Type", "Log & Trap");
        }

        if ((p_db_event->type == RMON_EVENT_TRAP ) ||
            (p_db_event->type == RMON_EVENT_LOG_TRAP))
        {
            sal_fprintf(fp, "   %-20s%s\n", "Community name", p_db_event->community);
        }

        if (p_db_event->last_sent)
        {
            rmon_format_tv(p_db_event->last_sent / 1000, time);
            sal_fprintf(fp, "   %-20s%s\n", "Last Time Sent", time);
        }
        else
        {
            sal_fprintf(fp, "   %-20s%s\n", "Last Time Sent", "not sent yet");
        }

        sal_fprintf(fp, "   %-20s%s\n", "Owner", p_db_event->owner); 

        sal_fprintf(fp, "\n");
    }

    return rc;
}

int32
rmon_show_alarm(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_alarm_t *p_db_alarm)
{
    int32 rc = PM_E_NONE;
    char *st = NULL;

    if (p_db_alarm == NULL)
    {
        return rc;
    }

    if ((p_db_alarm->status != VALID_STATUS) && (p_db_alarm->status != UNDER_CREATION_STATUS))
    {
        return rc;
    }

    sal_fprintf(fp, "Alarm Index = %d \n", p_db_alarm->key.index);

    switch(p_db_alarm->status)
    {
        case VALID_STATUS :
            st = "VALID";
            break;
        case CREATE_REQ_STATUS :
            st = "CREATE_REQ_STATUS";
            break;
        case UNDER_CREATION_STATUS :
            st = "UNDER_CREATION_STATUS";
            break;
        case INVALID_STATUS :
            st = "INVALID_STATUS";
            break;
        default:
            st = "INVALID ALARM STATUS";
            break;
    }

    sal_fprintf(fp, "  %-20s%s\n", "Status", st);
    sal_fprintf(fp, "  %-20s%s\n", "Variable", p_db_alarm->variable_word);
    sal_fprintf(fp, "  %-20s%ds\n", "Interval", p_db_alarm->interval);

    if (p_db_alarm->sample_type == RMON_ALARM_ABS)
    {
        sal_fprintf(fp, "  %-20s%s\n", "Type", "Absolute");
    }
    else
    {
        sal_fprintf(fp, "  %-20s%s\n", "Type", "Delta");
    }

    sal_fprintf(fp, "  %-20s%"PRId64"\n", "Value", p_db_alarm->value);
    sal_fprintf(fp, "  %-20s%d\n", "Rising Threshold", p_db_alarm->rising_threshold);
    if (p_db_alarm->rising_event_index)
    {
        sal_fprintf(fp, "  %-20s%d\n", "Rising Event", p_db_alarm->rising_event_index);
    }
    else
    {
        sal_fprintf(fp, "  %-20s%s\n", "Rising Event", "not configured yet");
    }

    sal_fprintf(fp, "  %-20s%d\n", "Falling Threshold", p_db_alarm->falling_threshold);
    if (p_db_alarm->falling_event_index)
    {
        sal_fprintf(fp, "  %-20s%d\n", "Falling Event", p_db_alarm->falling_event_index);
    }
    else
    {
        sal_fprintf(fp, "  %-20s%s\n", "Falling Event", "not configured yet");
    }
    sal_fprintf(fp, "  %-20s%s\n", "Owner", p_db_alarm->owner);
    sal_fprintf(fp, "\n");

    return rc;
}

int32
rmon_show_stats(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_ether_stats_t *p_db_stats)
{
    int32 rc = PM_E_NONE;

    if (!p_db_stats)
    {
        return rc;
    }

    sal_fprintf(fp, "Rmon collection index %d \n", p_db_stats->key.index);
    sal_fprintf(fp, "  Statistics ifindex = %d, Owner: %s \n", p_db_stats->data_source_ifindex, p_db_stats->owner);
    sal_fprintf(fp, "  Input packets %"PRIu64", octets %"PRIu64", dropped %"PRIu64"\n", 
                p_db_stats->pkts, p_db_stats->octets, p_db_stats->drop_events);
    sal_fprintf(fp, "  Broadcast packets %"PRIu64", multicast packets %"PRIu64"\n", 
                p_db_stats->broadcast_pkts, p_db_stats->multicast_pkts);
    sal_fprintf(fp, "  CRC alignment errors %"PRIu64", collisions %"PRIu64"\n", 
                p_db_stats->crc_align_errors, p_db_stats->collisions);
    sal_fprintf(fp, "  Undersized packets %"PRIu64", oversized packets %"PRIu64", fragments %"PRIu64"\n", 
                p_db_stats->undersize_pkts, p_db_stats->oversize_pkts, p_db_stats->fragments);
    sal_fprintf(fp, "  Packets received of length (in octets):\n");
    sal_fprintf(fp, "    64: %"PRIu64",  65-127: %"PRIu64", 128-255: %"PRIu64"\n", 
                p_db_stats->pkts_64_octets, p_db_stats->pkts_65to127_octets, p_db_stats->pkts_128to255_octets);
    sal_fprintf(fp, "    256-511: %"PRIu64",  512-1023: %"PRIu64", 1024-1518: %"PRIu64"\n", 
                p_db_stats->pkts_256to511_octets, p_db_stats->pkts_512to1023_octets, p_db_stats->pkts_1024to1518_octets);
    sal_fprintf(fp, "\n");

    return rc;
}

int32
rmon_show_history_stats(tbl_rmon_history_t *p_db_his, FILE *fp)
{
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;
    struct sal_timeval  tv;
    char time[256] = {0};

    tbl_rmon_stats_history_master_t *p_stats_his_master = tbl_rmon_stats_history_get_master();
    tbl_rmon_stats_history_t *p_db_stats = NULL;

    CTCLIB_SLIST_LOOP(p_stats_his_master->rmon_stats_history_list, p_db_stats, listnode)
    {
        if (p_db_stats->key.history_index != p_db_his->key.index)
        {
            continue;
        }
        tv.tv_sec = p_db_stats->interval_start / 1000;
        uint32 days = 0;
        uint32 hours = 0;
        uint32 minutes = 0;
        uint32 seconds = 0;
        days = tv.tv_sec / 86400;
        hours = (tv.tv_sec / 3600) % 24;
        minutes = (tv.tv_sec / 60) % 60;
        seconds = tv.tv_sec % 60;
        sal_sprintf(time, "%ddays %dh:%dm:%ds", days, hours, minutes, seconds);
        sal_fprintf(fp, "    Sample # %d Begin at %s\n", p_db_stats->key.stats_index, time);
        sal_fprintf(fp, "     Received %"PRIu64" octets, %"PRIu64" packets\n", p_db_stats->octets, p_db_stats->pkts);
        sal_fprintf(fp, "     %"PRIu64" broadcast and %"PRIu64" multicast packets\n", p_db_stats->broadcast_pkts, p_db_stats->multicast_pkts);
        sal_fprintf(fp, "     %"PRIu64" undersized and %"PRIu64" oversized packets\n", p_db_stats->undersize_pkts, p_db_stats->oversize_pkts);
        sal_fprintf(fp, "     %"PRIu64" dropped packet events and %"PRIu64" fragments\n", p_db_stats->drop_events, p_db_stats->fragments);
        sal_fprintf(fp, "     %"PRIu64" CRC alignment errors and %"PRIu64" collisions.\n",p_db_stats->crc_align_errors, p_db_stats->collisions);
        sal_fprintf(fp, "     Network utilization is estimated at %d\n", p_db_stats->utilization);
    }

    return rc;
}

int32
rmon_show_history(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_history_t *p_db_his)
{
    int32 rc = PM_E_NONE;

    if (!p_db_his)
    {
        return rc;
    }

    if (p_db_his->status != VALID_STATUS)
    {
        sal_fprintf(fp, "Rmon history index %d is not valid currently\n\n", p_db_his->key.index);
    }

    sal_fprintf(fp, "History index = %d\n", p_db_his->key.index);
    sal_fprintf(fp, "  Data source ifindex = %d\n", p_db_his->data_source);
    sal_fprintf(fp, "  Buckets requested = %d\n", p_db_his->buckets_requested);
    sal_fprintf(fp, "  Buckets granted = %d\n", p_db_his->buckets_granted);
    sal_fprintf(fp, "  Interval = %ds\n", p_db_his->interval);
    sal_fprintf(fp, "  Owner: %s\n", p_db_his->owner);

    rmon_show_history_stats(p_db_his, fp);

    sal_fprintf(fp, "\n");
    
    return rc;
}

int32
app_rmon_start()
{
    cdb_register_cfg_tbl_cb(TBL_RMON_EVENT, rmon_cmd_process_event);
    cdb_register_cfg_tbl_cb(TBL_RMON_ALARM, rmon_cmd_process_alarm);
    cdb_register_cfg_tbl_cb(TBL_RMON_ETHER_STATS, rmon_cmd_process_stats);
    cdb_register_cfg_tbl_cb(TBL_RMON_HISTORY, rmon_cmd_process_history);

    cdb_register_cfg_act_cb(ACT_SHOW_RMON_EVENT, rmon_cmd_show_event);
    cdb_register_cfg_act_cb(ACT_SHOW_RMON_ALARM, rmon_cmd_show_alarm);
    cdb_register_cfg_act_cb(ACT_SHOW_RMON_ETHER_STATS, rmon_cmd_show_stats);
    cdb_register_cfg_act_cb(ACT_SHOW_RMON_HISTORY, rmon_cmd_show_history);
    return PM_E_NONE;
}

