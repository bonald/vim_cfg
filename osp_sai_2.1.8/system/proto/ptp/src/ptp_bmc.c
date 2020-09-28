
#include "proto.h"
#include "genlog.h"
#include "glb_debug_define.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"

#include "ptp_api.h"
#include "ptp_datatype.h"
#include "ptp_error.h"
#include "ptp_util.h"
#include "ptp_desc.h"
#include "ptp_proto.h"
#include "ptp_msg.h"
#include "ptp_ops.h"
#include "ptp_bmc.h"
#include "ptp_constant.h"

enum ptp_dataset_compare_rlt_e
{
    PTP_CMP_DUPLICATE_OR_SAME = -2,
    PTP_CMP_RECEIVER_EQUAL_TO_SENDER = -1,
    PTP_CMP_A_EQUAL_TO_B = 0,
    PTP_CMP_A_BETTER_THAN_B = 1,
    PTP_CMP_A_TOPO_BETTER_THAN_B = 2,
    PTP_CMP_B_BETTER_THAN_A = 3,
    PTP_CMP_B_TOPO_BETTER_THAN_A = 4,
};
typedef enum ptp_dataset_compare_rlt_e ptp_dataset_compare_rlt_t;

#define PTP_FOREIGN_MASTER_TIME_WINDOW 4
#define PTP_FOREIGN_MASTER_THRESHOLD 2

typedef int32
(*PTP_LISTLOOP_ARG_CB_FUNC)(tbl_ptp_port_t* p_port, void* p_arg);

/* modified by yaom for bug 18501 20110306 */
extern int32 ptp_snmp_master_change_trap(tbl_ptp_foreign_t* p_best);
extern int32 ptp_snmp_accuracy_change_trap(uint32 accuracy);

static void
_ptp_bmc_update_master(tbl_ptp_port_t* p_port)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    /*Current data set update*/
    g_p_master->current_ds.steps_removed = 0;
    g_p_master->current_ds.offset_from_master = 0;
    g_p_master->current_ds.mean_path_delay = 0;

    /*Parent data set*/
    ptp_util_clockid_cpy(g_p_master->parent_ds.parent_port_identity.clock_id, g_p_master->default_ds.clock_identity);
    g_p_master->parent_ds.parent_port_identity.port_num = 0;
    g_p_master->parent_ds.parent_stats = PTP_PARENT_STATS_DEF;
    g_p_master->parent_ds.observed_parent_offset_scaled_log_variance = PTP_OBSERVED_PARENT_OSLV_DEF;
    g_p_master->parent_ds.observed_parent_clock_phase_change_rate = PTP_OBSERVED_PARENT_CPCR_DEF;

    ptp_util_clockid_cpy(g_p_master->parent_ds.grandmaster_identity, g_p_master->default_ds.clock_identity);
    g_p_master->parent_ds.grandmaster_clock_quality.clk_accuracy = g_p_master->default_ds.clock_quality.clk_accuracy;
    g_p_master->parent_ds.grandmaster_clock_quality.clk_class = g_p_master->default_ds.clock_quality.clk_class;
    g_p_master->parent_ds.grandmaster_clock_quality.offset_scaled_log_variance
            = g_p_master->default_ds.clock_quality.offset_scaled_log_variance;
    g_p_master->parent_ds.grandmaster_priority1 = g_p_master->default_ds.priority1;
    g_p_master->parent_ds.grandmaster_priority2 = g_p_master->default_ds.priority2;

    /*Time Properties data set*/
    sal_memcpy(&g_p_master->time_prop_ds, &g_p_master->time_prop_intern, sizeof(ptp_time_properties_t));

    return;
}

void
ptp_bmc_update_m1(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_BMC("Update m1 for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    _ptp_bmc_update_master(p_port);

    return;
}

static inline void
_ptp_bmc_update_m2(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_BMC("Update m2 for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    _ptp_bmc_update_master(p_port);

    return;
}

void
ptp_bmc_update_m3(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_BMC("Update m3 for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));

    return;
}

void
ptp_bmc_update_s1(tbl_ptp_port_t* p_port, ptp_common_message_header_t* hdr, ptp_msg_announce_t* msg)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_BMC("Update s1 for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    uint32 rc = 0;
    char clk_accuracy_str[16] = {0};
    
    /*Current DS*/
    g_p_master->current_ds.steps_removed = msg->steps_removed + 1;

    /*Parent DS*/
    ptp_util_clockid_cpy(g_p_master->parent_ds.parent_port_identity.clock_id, hdr->source_port_identity_clock_id);
    g_p_master->parent_ds.parent_port_identity.port_num = sal_ntoh16(hdr->source_port_identity_port_num);
    ptp_util_clockid_cpy(g_p_master->parent_ds.grandmaster_identity, msg->grandmaster_identity);

    /* modified by yaom for bug 18501 20110306 */
    if (g_p_master->parent_ds.grandmaster_clock_quality.clk_accuracy != msg->grandmaster_clock_quality.clk_accuracy)
    {
        g_p_master->parent_ds.grandmaster_clock_quality.clk_accuracy = msg->grandmaster_clock_quality.clk_accuracy;
        rc = ptp_snmp_accuracy_change_trap((uint32)msg->grandmaster_clock_quality.clk_accuracy);
        PTP_DBG_BMC("PTP master accuracy changed to 0x%x, rc %d.", (uint32)msg->grandmaster_clock_quality.clk_accuracy, rc);
        sal_snprintf(clk_accuracy_str, sizeof(clk_accuracy_str), "%x", (uint32)msg->grandmaster_clock_quality.clk_accuracy);
        logid_sys(LOG_PTP_4_PTP_ACCURACY_CHANGE, clk_accuracy_str);
    }
    
    g_p_master->parent_ds.grandmaster_clock_quality.clk_class = msg->grandmaster_clock_quality.clk_class;
    g_p_master->parent_ds.grandmaster_clock_quality.offset_scaled_log_variance
            = msg->grandmaster_clock_quality.offset_scaled_log_variance;
    g_p_master->parent_ds.grandmaster_priority1 = msg->grandmaster_priority1;
    g_p_master->parent_ds.grandmaster_priority2 = msg->grandmaster_priority2;

    /*Timeproperties DS*/
    g_p_master->time_prop_ds.current_utc_offset = msg->current_utc_offset;
    g_p_master->time_prop_ds.current_utc_offset_valid = hdr->flag_field_1_current_utc_offset_valid;
    g_p_master->time_prop_ds.leap59 = hdr->flag_field_1_leap59;
    g_p_master->time_prop_ds.leap61 = hdr->flag_field_1_leap61;
    g_p_master->time_prop_ds.time_traceable = hdr->flag_field_1_time_traceable;
    g_p_master->time_prop_ds.frequency_traceable = hdr->flag_field_1_frequency_traceable;
    g_p_master->time_prop_ds.ptp_timescale = hdr->flag_field_1_ptp_timescale;
    g_p_master->time_prop_ds.time_source = msg->time_source;

    return;
}

static inline void
_ptp_bmc_update_p1(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_BMC("Update p1 for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));

    return;
}

static inline void
_ptp_bmc_update_p2(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_BMC("Update p2 for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));

    return;
}

static void
_ptp_bmc_announce_receipt_trigger(void* p_data)
{
    /*TMR_FM_ANNOUNCE_RECEIPT*/
    PTP_DBG_TMR("FM announce-receipt timer expired");
    tbl_ptp_foreign_t* pst_fmds = p_data ? (tbl_ptp_foreign_t*)p_data : NULL;
    PTP_PTR_INVALID_RETURN_NULL(pst_fmds);

    pst_fmds->ptm_fm_announce_receipt_timer = NULL;

    if (pst_fmds->p_port_belong_to->rbest == pst_fmds)
    {
        pst_fmds->p_port_belong_to->rbest = NULL;
    }
    
    ptp_util_cancel_timer(&pst_fmds->ptm_fm_qualification_timer);
    tbl_ptp_foreign_del_ptp_foreign(&(pst_fmds->key));
    
    return;
}

static void
_ptp_bmc_qualify_trigger(void* p_data)
{
    /*TMR_FM_QUALIFY*/
    PTP_DBG_TMR("FM qualification timer expired");

    tbl_ptp_foreign_t* pst_fmds = p_data ? (tbl_ptp_foreign_t*)p_data : NULL;
    PTP_PTR_INVALID_RETURN_NULL(pst_fmds);

    pst_fmds->ptm_fm_qualification_timer = NULL;

    pst_fmds->qualified = (pst_fmds->foreign_master_announce_messages > PTP_FOREIGN_MASTER_THRESHOLD);
    if (!pst_fmds->qualified)
    {
        PTP_DBG_TMR("%s foreign_master_announce_messages is %d, not qualified, delete it", 
            ptp_desc_clockid(pst_fmds->key.foreign_master_port_identity.clock_id), pst_fmds->foreign_master_announce_messages);
        ptp_util_cancel_timer(&pst_fmds->ptm_fm_announce_receipt_timer);
        tbl_ptp_foreign_del_ptp_foreign(&(pst_fmds->key));
        
        return;
    }

    ptp_util_cancel_timer(&pst_fmds->ptm_fm_announce_receipt_timer);
    pst_fmds->ptm_fm_announce_receipt_timer = ptp_util_add_timer_log_window(_ptp_bmc_announce_receipt_trigger, pst_fmds,
        pst_fmds->p_port_belong_to->port_ds.log_announce_interval, pst_fmds->p_port_belong_to->port_ds.announce_receipt_timeout);

    return;
}

int32
ptp_bmc_add_foreign(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb)
{
    uint32 ret = 0;
    tbl_ptp_foreign_t tempds;
    ptp_common_message_header_t *hdr = (ptp_common_message_header_t *) skb->data;
    ptp_msg_announce_t msg;
    sal_memset(&msg, 0x00, sizeof(ptp_msg_announce_t));
    ptp_msg_unpack_announce(skb, &msg);

    sal_memset(&tempds, 0x00, sizeof(tbl_ptp_foreign_t));
    ptp_util_clockid_cpy(tempds.key.foreign_master_port_identity.clock_id, hdr->source_port_identity_clock_id);
    tempds.key.foreign_master_port_identity.port_num = sal_ntoh16(hdr->source_port_identity_port_num);
    sal_memcpy(&tempds.header, hdr, sizeof(ptp_common_message_header_t));
    sal_memcpy(&tempds.announce, &msg, sizeof(ptp_msg_announce_t));
    tempds.p_port_belong_to = p_port;
    tempds.key.ifindex = p_port->ifindex;

    /*check whether the foreign master is already known*/
    tbl_ptp_foreign_t *p_db_ptp_foreign = NULL;
    p_db_ptp_foreign = tbl_ptp_foreign_get_ptp_foreign(&(tempds.key));
    if (p_db_ptp_foreign)
    {
        char ifname_ext[IFNAME_EXT_SIZE] = {0};
        PTP_DBG_BMC("Foreign %s port %d from interface %s already exist", 
            ptp_desc_clockid(tempds.key.foreign_master_port_identity.clock_id), tempds.key.foreign_master_port_identity.port_num, IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
        p_db_ptp_foreign->foreign_master_announce_messages++;
        sal_memcpy(&p_db_ptp_foreign->header, &tempds.header, sizeof(ptp_common_message_header_t));
        sal_memcpy(&p_db_ptp_foreign->announce, &tempds.announce, sizeof(ptp_msg_announce_t));
        if (p_db_ptp_foreign->qualified)
        {
            /*TMR_ANNOUNCE_RECEIPT*/
            PTP_DBG_TMR("Restart FM announce-receipt timer");
            ptp_util_cancel_timer(&p_db_ptp_foreign->ptm_fm_announce_receipt_timer);
            p_db_ptp_foreign->ptm_fm_announce_receipt_timer = ptp_util_add_timer_log_window(_ptp_bmc_announce_receipt_trigger, p_db_ptp_foreign,
                p_db_ptp_foreign->p_port_belong_to->port_ds.log_announce_interval, p_db_ptp_foreign->p_port_belong_to->port_ds.announce_receipt_timeout);
        }

        return PTP_E_SUCCESS;
    }

    /*new foreign master*/
    ret = tbl_ptp_foreign_add_ptp_foreign(&tempds);
    if (0 != ret)
    {
        PTP_DBG_BMC("Add ptp foreign failed");
        return PTP_E_NO_MEMORY;
    }

    p_db_ptp_foreign = tbl_ptp_foreign_get_ptp_foreign(&(tempds.key));
    if (p_db_ptp_foreign == NULL)
    {
        PTP_DBG_BMC("Get new alloced ptp foreign failed");
        return PTP_E_NO_MEMORY;
    }

    /*TMR_QUALIFY*/
    PTP_DBG_TMR("Start FM qualification timer");
    ptp_util_cancel_timer(&p_db_ptp_foreign->ptm_fm_qualification_timer);
    p_db_ptp_foreign->ptm_fm_qualification_timer = ptp_util_add_timer_log_window(_ptp_bmc_qualify_trigger, p_db_ptp_foreign,
            p_port->port_ds.log_announce_interval, PTP_FOREIGN_MASTER_TIME_WINDOW);

    return PTP_E_SUCCESS;
}

void
ptp_bmc_destroy_foreign_db(tbl_ptp_port_t *p_port)
{
    tbl_ptp_foreign_master_t * p_master = tbl_ptp_foreign_get_master();
    tbl_ptp_foreign_t * p_db_ptp_foreign = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_master->ptp_foreign_list, p_db_ptp_foreign, listnode, next)
    {
        if (p_port->ifindex == p_db_ptp_foreign->key.ifindex)
        {
            ptp_util_cancel_timer(&p_db_ptp_foreign->ptm_fm_qualification_timer);
            ptp_util_cancel_timer(&p_db_ptp_foreign->ptm_fm_announce_receipt_timer);
            tbl_ptp_foreign_del_ptp_foreign(&(p_db_ptp_foreign->key));
        }
    }

    return;
}

static void
_ptp_bmc_copy_d0(tbl_ptp_foreign_t* d0)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    d0->announce.grandmaster_priority1 = g_p_master->default_ds.priority1;
    ptp_util_clockid_cpy(d0->announce.grandmaster_identity, g_p_master->default_ds.clock_identity);
    d0->announce.grandmaster_clock_quality.clk_class = g_p_master->default_ds.clock_quality.clk_class;
    d0->announce.grandmaster_clock_quality.clk_accuracy = g_p_master->default_ds.clock_quality.clk_accuracy;
    d0->announce.grandmaster_clock_quality.offset_scaled_log_variance
            = g_p_master->default_ds.clock_quality.offset_scaled_log_variance;
    d0->announce.grandmaster_priority2 = g_p_master->default_ds.priority2;
    d0->announce.steps_removed = 0;
    ptp_util_clockid_cpy(d0->key.foreign_master_port_identity.clock_id, g_p_master->default_ds.clock_identity);

    return;
}

static ptp_dataset_compare_rlt_t
_ptp_bmc_compare_dataset_same_gm(tbl_ptp_foreign_t *dsa, tbl_ptp_foreign_t *dsb)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    int32 rlt = 0;
    ptp_msg_announce_t *pa = &dsa->announce;
    ptp_msg_announce_t *pb = &dsb->announce;

    /*Algorithm part2 Fig 28*/
    if (pa->steps_removed > pb->steps_removed + 1)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for steps_removed A(%d) B(%d)", 
            pa->steps_removed, pb->steps_removed + 1);
        return PTP_CMP_B_BETTER_THAN_A;
    }
    else if (pb->steps_removed > pa->steps_removed + 1)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for steps_removed A(%d) B(%d)", 
            pa->steps_removed, pb->steps_removed + 1);
        return PTP_CMP_A_BETTER_THAN_B;
    }

    /*A within 1 of B*/
    if (pa->steps_removed > pb->steps_removed)
    {
        /*compare identities of reciver of A and sender of A*/
        rlt = ptp_util_clockid_cmp(g_p_master->default_ds.clock_identity, dsa->key.foreign_master_port_identity.clock_id);
        if (rlt < 0)
        {
            PTP_DBG_BMC("B_BETTER_THAN_A for clock_identity default_ds(%s) A(%s)", 
                ptp_desc_clockid(g_p_master->default_ds.clock_identity), ptp_desc_clockid(dsa->key.foreign_master_port_identity.clock_id));
            return PTP_CMP_B_BETTER_THAN_A;
        }
        else if (rlt > 0)
        {
            PTP_DBG_BMC("B_TOPO_BETTER_THAN_A for clock_identity default_ds(%s) A(%s)", 
                ptp_desc_clockid(g_p_master->default_ds.clock_identity), ptp_desc_clockid(dsa->key.foreign_master_port_identity.clock_id));
            return PTP_CMP_B_TOPO_BETTER_THAN_A;
        }
        else
        {
            /*Receiver=Sender, refer to p90 NOTE*/
            PTP_DBG_BMC("BMC_CALCULATE Sender=Receiver : Error -1 PTP_CMP_RECEIVER_EQUAL_TO_SENDER");
            return PTP_CMP_RECEIVER_EQUAL_TO_SENDER;
        }
    }
    else if (pb->steps_removed > pa->steps_removed)
    {
        /*compare identities of reciver of B and sender of B*/
        rlt = ptp_util_clockid_cmp(g_p_master->default_ds.clock_identity, dsb->key.foreign_master_port_identity.clock_id);
        if (rlt < 0)
        {
            PTP_DBG_BMC("A_BETTER_THAN_B for clock_identity default_ds(%s) B(%s)", 
                ptp_desc_clockid(g_p_master->default_ds.clock_identity), ptp_desc_clockid(dsb->key.foreign_master_port_identity.clock_id));
            return PTP_CMP_A_BETTER_THAN_B;
        }
        else if (rlt > 0)
        {
            PTP_DBG_BMC("A_TOPO_BETTER_THAN_B for clock_identity default_ds(%s) B(%s)", 
                ptp_desc_clockid(g_p_master->default_ds.clock_identity), ptp_desc_clockid(dsb->key.foreign_master_port_identity.clock_id));
            return PTP_CMP_A_TOPO_BETTER_THAN_B;
        }
        else
        {
            /*Receiver=Sender, refer to p90 NOTE*/
            PTP_DBG_BMC("BMC_CALCULATE Sender=Receiver : Error -1 PTP_CMP_RECEIVER_EQUAL_TO_SENDER");
            return PTP_CMP_RECEIVER_EQUAL_TO_SENDER;
        }
    }

    /*steps removed A = steps removed B*/
    /*compare identities of senders of A and B*/
    rlt = ptp_util_clockid_cmp(dsa->key.foreign_master_port_identity.clock_id, dsb->key.foreign_master_port_identity.clock_id);
    if (rlt < 0)
    {
        PTP_DBG_BMC("A_TOPO_BETTER_THAN_B for clock_identity A(%s) B(%s)", 
            ptp_desc_clockid(dsa->key.foreign_master_port_identity.clock_id), ptp_desc_clockid(dsb->key.foreign_master_port_identity.clock_id));
        return PTP_CMP_A_TOPO_BETTER_THAN_B;
    }
    else if (rlt > 0)
    {
        PTP_DBG_BMC("B_TOPO_BETTER_THAN_A for clock_identity A(%s) B(%s)", 
             ptp_desc_clockid(dsa->key.foreign_master_port_identity.clock_id), ptp_desc_clockid(dsb->key.foreign_master_port_identity.clock_id));
        return PTP_CMP_B_TOPO_BETTER_THAN_A;
    }

    /*compare port numbers of senders of A and B*/
    if (dsa->key.foreign_master_port_identity.port_num < dsb->key.foreign_master_port_identity.port_num)
    {
        PTP_DBG_BMC("A_TOPO_BETTER_THAN_B for port_num A(%d) B(%d)", 
            dsa->key.foreign_master_port_identity.port_num, dsb->key.foreign_master_port_identity.port_num);
        return PTP_CMP_A_TOPO_BETTER_THAN_B;
    }
    else if (dsa->key.foreign_master_port_identity.port_num > dsb->key.foreign_master_port_identity.port_num)
    {
        PTP_DBG_BMC("B_TOPO_BETTER_THAN_A for port_num A(%d) B(%d)", 
            dsa->key.foreign_master_port_identity.port_num, dsb->key.foreign_master_port_identity.port_num);
        return PTP_CMP_B_TOPO_BETTER_THAN_A;
    }

    /*A=B, refer to p90 NOTE*/
    PTP_DBG_BMC("BMC_CALCULATE Sender=Receiver : Error -2 PTP_CMP_DUPLICATE_OR_SAME");
    return PTP_CMP_DUPLICATE_OR_SAME;
}

static ptp_dataset_compare_rlt_t
_ptp_bmc_compare_dataset(tbl_ptp_foreign_t *dsa, tbl_ptp_foreign_t *dsb)
{
    ptp_msg_announce_t *pa = &dsa->announce;
    ptp_msg_announce_t *pb = &dsb->announce;

    char buf_a[512] = {0};
    char buf_b[512] = {0};
    
    PTP_DBG_BMC("A:%s", cdb_ptp_msg_announce_val2str(buf_a, sizeof(buf_a), pa));
    PTP_DBG_BMC("B:%s", cdb_ptp_msg_announce_val2str(buf_b, sizeof(buf_b), pb));
    
    if (pa->grandmaster_priority1 < pb->grandmaster_priority1)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for grandmaster_priority1 A(%d) B(%d)", 
            pa->grandmaster_priority1, pb->grandmaster_priority1);
        return PTP_CMP_A_BETTER_THAN_B;
    }
    else if (pa->grandmaster_priority1 > pb->grandmaster_priority1)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for grandmaster_priority1 A(%d) B(%d)", 
            pa->grandmaster_priority1, pb->grandmaster_priority1);
        return PTP_CMP_B_BETTER_THAN_A;
    }

    if (pa->grandmaster_clock_quality.clk_class < pb->grandmaster_clock_quality.clk_class)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for clk_class A(%d) B(%d)", 
            pa->grandmaster_clock_quality.clk_class, pb->grandmaster_clock_quality.clk_class);
        return PTP_CMP_A_BETTER_THAN_B;
    }
    else if (pa->grandmaster_clock_quality.clk_class > pb->grandmaster_clock_quality.clk_class)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for clk_class A(%d) B(%d)", 
            pa->grandmaster_clock_quality.clk_class, pb->grandmaster_clock_quality.clk_class);
        return PTP_CMP_B_BETTER_THAN_A;
    }

    if (pa->grandmaster_clock_quality.clk_accuracy < pb->grandmaster_clock_quality.clk_accuracy)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for clk_accuracy A(%d) B(%d)", 
            pa->grandmaster_clock_quality.clk_accuracy, pb->grandmaster_clock_quality.clk_accuracy);
        return PTP_CMP_A_BETTER_THAN_B;
    }
    else if (pa->grandmaster_clock_quality.clk_accuracy > pb->grandmaster_clock_quality.clk_accuracy)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for clk_accuracy A(%d) B(%d)", 
            pa->grandmaster_clock_quality.clk_accuracy, pb->grandmaster_clock_quality.clk_accuracy);
        return PTP_CMP_B_BETTER_THAN_A;
    }

    if (pa->grandmaster_clock_quality.offset_scaled_log_variance
        < pb->grandmaster_clock_quality.offset_scaled_log_variance)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for offset_scaled_log_variance A(%d) B(%d)", 
            pa->grandmaster_clock_quality.offset_scaled_log_variance, pb->grandmaster_clock_quality.offset_scaled_log_variance);
        return PTP_CMP_A_BETTER_THAN_B;
    }
    else if (pa->grandmaster_clock_quality.offset_scaled_log_variance
        > pb->grandmaster_clock_quality.offset_scaled_log_variance)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for offset_scaled_log_variance A(%d) B(%d)", 
            pa->grandmaster_clock_quality.offset_scaled_log_variance, pb->grandmaster_clock_quality.offset_scaled_log_variance);
        return PTP_CMP_B_BETTER_THAN_A;
    }

    if (pa->grandmaster_priority2 < pb->grandmaster_priority2)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for grandmaster_priority2 A(%d) B(%d)", 
            pa->grandmaster_priority2, pb->grandmaster_priority2);
        return PTP_CMP_A_BETTER_THAN_B;
    }
    else if (pa->grandmaster_priority2 > pb->grandmaster_priority2)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for grandmaster_priority2 A(%d) B(%d)", 
            pa->grandmaster_priority2, pb->grandmaster_priority2);
        return PTP_CMP_B_BETTER_THAN_A;
    }

    /*Identity comparison*/
    int32 gm_identity_comparison = ptp_util_clockid_cmp(pa->grandmaster_identity, pb->grandmaster_identity);
    if (gm_identity_comparison < 0)
    {
        PTP_DBG_BMC("A_BETTER_THAN_B for grandmaster_identity A(%s)", 
            ptp_desc_clockid(pa->grandmaster_identity));
        PTP_DBG_BMC("B(%s), gm_identity_comparison(%d)", 
            ptp_desc_clockid(pb->grandmaster_identity), gm_identity_comparison);
        return PTP_CMP_A_BETTER_THAN_B;
    }
    else if (gm_identity_comparison > 0)
    {
        PTP_DBG_BMC("B_BETTER_THAN_A for grandmaster_identity A(%s)", 
            ptp_desc_clockid(pa->grandmaster_identity));
        PTP_DBG_BMC("B(%s), gm_identity_comparison(%d)", 
            ptp_desc_clockid(pb->grandmaster_identity), gm_identity_comparison);
        return PTP_CMP_B_BETTER_THAN_A;
    }

    return _ptp_bmc_compare_dataset_same_gm(dsa, dsb);
}

static ptp_state_t
_ptp_bmc_decision_dataset(tbl_ptp_port_t* p_port, tbl_ptp_foreign_t* p_best)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    tbl_ptp_foreign_t* p_rbest = p_port->rbest;

    char port_identity[32];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    uint32 rc = 0;

    tbl_interface_t *p_db_if = NULL;

    PTP_DBG_BMC("Start normal state decison for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    /*D0*/
    tbl_ptp_foreign_t default0;
    sal_memset(&default0, 0x00, sizeof(tbl_ptp_foreign_t));
    _ptp_bmc_copy_d0(&default0);

    ptp_dataset_compare_rlt_t rlt;
    if (g_p_master->default_ds.clock_quality.clk_class < 128)
    {
        if (NULL == p_rbest)
        {
            ptp_bmc_update_m1(p_port);
            PTP_DBG_BMC("%s's recommended state is master", ifname_ext);
            return PTP_STATE_MASTER;
        }

        rlt = _ptp_bmc_compare_dataset(&default0, p_rbest);
        if (PTP_CMP_A_BETTER_THAN_B == rlt || PTP_CMP_A_TOPO_BETTER_THAN_B == rlt)
        {
            ptp_bmc_update_m1(p_port);
            PTP_DBG_BMC("%s's recommended state is master", ifname_ext);
            return PTP_STATE_MASTER;
        }
        else
        {
            _ptp_bmc_update_p1(p_port);
            PTP_DBG_BMC("%s's recommended state is passive", ifname_ext);
            return PTP_STATE_PASSIVE;
        }
    }

    /*D0 is not class 1 through 127*/
    /*D0 better or topology than Ebset*/
    if (NULL == p_best)
    {
        _ptp_bmc_update_m2(p_port);
        PTP_DBG_BMC("%s's recommended state is master", ifname_ext);
        return PTP_STATE_MASTER;
    }

    rlt = _ptp_bmc_compare_dataset(&default0, p_best);
    if (PTP_CMP_A_BETTER_THAN_B == rlt || PTP_CMP_A_TOPO_BETTER_THAN_B == rlt)
    {
        _ptp_bmc_update_m2(p_port);
        PTP_DBG_BMC("%s's recommended state is master", ifname_ext);
        return PTP_STATE_MASTER;
    }

    /*D0 not better or topology than Ebset*/
    /*Ebest received on port r, that is Ebest=Erbest*/
    if (p_best == p_rbest)
    {
        if (ptp_util_portid_cmp(&g_p_master->parent_ds.parent_port_identity, &p_best->key.foreign_master_port_identity))
        {
            p_port->slave_change_master = TRUE;
            rc = ptp_snmp_master_change_trap(p_best);/* modified by yaom for bug 18501 20110306 */
            p_db_if = tbl_interface_get_interface_by_ifindex(p_best->p_port_belong_to->ifindex);
            if (NULL == p_db_if)
            {
                PTP_DBG_BMC("Interface %d is NULL, return fail", p_best->p_port_belong_to->ifindex);
                return PM_E_FAIL;
            }
            
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
            sal_snprintf(port_identity, sizeof(port_identity), "%s", ptp_desc_clockid(p_best->key.foreign_master_port_identity.clock_id));
            PTP_DBG_BMC("PTP master of interface %s changed to clock %s, rc %d.", ifname_ext, port_identity, rc);
            logid_sys(LOG_PTP_4_PTP_MASTER_CHANGE, ifname_ext, port_identity);
        }

        ptp_bmc_update_s1(p_port, &p_best->header, &p_best->announce);
        PTP_DBG_BMC("%s's recommended state is slave", ifname_ext);
        return PTP_STATE_SLAVE;
    }

    /*Ebest better by topology than Erbest*/
    if (NULL == p_rbest)
    {
        ptp_bmc_update_m3(p_port);
        PTP_DBG_BMC("%s's recommended state is master", ifname_ext);
        return PTP_STATE_MASTER;
    }

    rlt = _ptp_bmc_compare_dataset(p_best, p_rbest);
    if (PTP_CMP_A_TOPO_BETTER_THAN_B == rlt)
    {
        _ptp_bmc_update_p2(p_port);
        PTP_DBG_BMC("%s's recommended state is passive", ifname_ext);
        return PTP_STATE_PASSIVE;
    }
    else
    {
        ptp_bmc_update_m3(p_port);
        PTP_DBG_BMC("%s's recommended state is master", ifname_ext);
        return PTP_STATE_MASTER;
    }
}

static ptp_state_t
_ptp_bmc_decision_slave_only_dataset(tbl_ptp_port_t* p_port, tbl_ptp_foreign_t* p_best)
{
    PTP_DBG_BMC("Start slave only state decison");
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    tbl_ptp_foreign_t* p_rbest = p_port->rbest;

    char port_identity[32];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    uint32 rc = 0;

    tbl_interface_t *p_db_if = NULL;

    IFNAME_ETH2FULL(p_port->key.name, ifname_ext);

    /*D0*/
    tbl_ptp_foreign_t default0;
    sal_memset(&default0, 0x00, sizeof(tbl_ptp_foreign_t));
    _ptp_bmc_copy_d0(&default0);

    ptp_dataset_compare_rlt_t rlt;
    if (g_p_master->default_ds.clock_quality.clk_class < 128)
    {
        PTP_DBG_BMC("%s's recommended state is listening", ifname_ext);
        return PTP_STATE_LISTENING;
    }

    /*D0 is not class 1 through 127*/
    /*D0 better or topology than Ebset*/
    if (NULL == p_best)
    {
        PTP_DBG_BMC("%s's recommended state is listening", ifname_ext);
        return PTP_STATE_LISTENING;
    }

    rlt = _ptp_bmc_compare_dataset(&default0, p_best);
    if (PTP_CMP_A_BETTER_THAN_B == rlt || PTP_CMP_A_TOPO_BETTER_THAN_B == rlt)
    {
        PTP_DBG_BMC("%s's recommended state is listening", ifname_ext);
        return PTP_STATE_LISTENING;
    }

    /*D0 not better or topology than Ebset*/
    /*Ebest received on port r, that is Ebest=Erbest*/
    if (p_best == p_rbest)
    {
        if (ptp_util_portid_cmp(&g_p_master->parent_ds.parent_port_identity, &p_best->key.foreign_master_port_identity))
        {
            p_port->slave_change_master = TRUE;
            rc = ptp_snmp_master_change_trap(p_best);/* modified by yaom for bug 18501 20110306 */
            p_db_if = tbl_interface_get_interface_by_ifindex(p_best->p_port_belong_to->ifindex);
            if (NULL == p_db_if)
            {
                PTP_DBG_BMC("Interface %d is NULL, return fail", p_best->p_port_belong_to->ifindex);
                return PM_E_FAIL;
            }
            
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
            sal_snprintf(port_identity, sizeof(port_identity), "%s", ptp_desc_clockid(p_best->key.foreign_master_port_identity.clock_id));
            PTP_DBG_BMC("PTP master of interface %s changed to clock %s, rc %d.", ifname_ext, port_identity, rc);
            logid_sys(LOG_PTP_4_PTP_MASTER_CHANGE, ifname_ext, port_identity);
        }

        ptp_bmc_update_s1(p_port, &p_best->header, &p_best->announce);
        PTP_DBG_BMC("%s's recommended state is slave", ifname_ext);
        return PTP_STATE_SLAVE;
    }

    PTP_DBG_BMC("%s's recommended state is listening", ifname_ext);
    return PTP_STATE_LISTENING;
}

static int32
_ptp_bmc_computation_rbest_by_iterate(void *obj, tbl_iter_args_t *pargs)
{
    tbl_ptp_port_t* p_port = pargs->argv[0];
    tbl_ptp_foreign_t *fmds = obj;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    PTP_DBG_BMC("Start computation %s's rbest", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    /*the foreign is not belong to this port*/
    if (p_port->ifindex != fmds->key.ifindex)
    {
        PTP_DBG_BMC("The foreign is not belong to %s, for foreign's ifindex %d, port ifindex %d", ifname_ext, fmds->key.ifindex, p_port->ifindex);
        return PTP_E_SUCCESS;
    }
    
    if (fmds->qualified)
    {
        if (NULL == p_port->rbest)
        {
            p_port->rbest = fmds;
            PTP_DBG_BMC("Current %s's rbest is %s", ifname_ext, ptp_desc_clockid(fmds->key.foreign_master_port_identity.clock_id));
            return PTP_E_SUCCESS;
        }

        if (p_port->rbest == fmds)
        {
            PTP_DBG_BMC("Current %s's rbest is %s for fmds is equal to rbest", ifname_ext, ptp_desc_clockid(fmds->key.foreign_master_port_identity.clock_id));
            return PTP_E_SUCCESS;
        }

        ptp_dataset_compare_rlt_t rlt;
        rlt = _ptp_bmc_compare_dataset(fmds, p_port->rbest);
        if (PTP_CMP_A_BETTER_THAN_B == rlt || PTP_CMP_A_TOPO_BETTER_THAN_B == rlt)
        {
            p_port->rbest = fmds;
            PTP_DBG_BMC("Current %s's rbest is %s, is better than %s", ifname_ext, 
                ptp_desc_clockid(fmds->key.foreign_master_port_identity.clock_id), 
                ptp_desc_clockid(((tbl_ptp_foreign_t *)(p_port->rbest))->key.foreign_master_port_identity.clock_id));
        }
    }
    else
    {
        PTP_DBG_BMC("Exit computation rbest for foreign %s is not qualified, foreign_master_announce_messages %hu", ptp_desc_clockid(fmds->key.foreign_master_port_identity.clock_id), fmds->foreign_master_announce_messages);
    }

    return PTP_E_SUCCESS;
}

static int32
_ptp_bmc_computation_by_iterate(void *obj, tbl_iter_args_t *pargs)
{
    PTP_PTR_INVALID_RETURN(pargs);

    tbl_ptp_port_t* p_port = obj;
    tbl_ptp_foreign_t** pp_best = pargs->argv[0];
    tbl_ptp_foreign_t* p_best = *pp_best;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    tbl_iter_args_t args;    

    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (!p_port)
    {
        PTP_DBG_BMC("Exit current bmc computation for ptp port is NULL");
        return PTP_E_ERROR;//my add
    }

    if (!(p_port->enable && p_port->link_up))
    {
        return PTP_E_ERROR;//my add
    }

    PTP_DBG_BMC("Current computation %s's rbest", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));

    if (PTP_STATE_FAULTY == p_port->port_ds.port_state || PTP_STATE_DISABLED == p_port->port_ds.port_state)
    {
        PTP_DBG_BMC("Exit current bmc computation for state (%s) error ", ptp_state_strs[PTP_STATE_DISABLED]);
        return PTP_E_SUCCESS;
    }

    /*Spec.9.3.2.2 b*/

    /*computation rbest for port*/
    args.argv[0] = p_port;
    tbl_ptp_foreign_iterate(_ptp_bmc_computation_rbest_by_iterate, &args);
    if (NULL == p_port->rbest)
    {
        //PTP_DBG_BMC("Exit current bmc computation for %s rbest is null ", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
        return PTP_E_SUCCESS;
    }

    tbl_ptp_foreign_t* p_rbest = p_port->rbest;
    PTP_DBG_BMC("E_rbest of %s is %s", ifname_ext,
            ptp_desc_clockid(p_rbest->key.foreign_master_port_identity.clock_id));

    /*Spec.9.3.2.2 c*/
    if (NULL == p_best)
    {
        *pp_best = p_rbest;
        PTP_DBG_BMC("Current device best is %s in ifindex%d for best is null", ptp_desc_clockid(p_rbest->key.foreign_master_port_identity.clock_id), p_rbest->key.ifindex);
        return PTP_E_SUCCESS;
    }

    ptp_dataset_compare_rlt_t rlt = _ptp_bmc_compare_dataset(p_rbest, p_best);
    if (PTP_CMP_A_BETTER_THAN_B == rlt || PTP_CMP_A_TOPO_BETTER_THAN_B == rlt)
    {
        *pp_best = p_rbest;
        PTP_DBG_BMC("Current device best is %s in %d", ptp_desc_clockid(p_rbest->key.foreign_master_port_identity.clock_id), p_rbest->key.ifindex);
    }

    return PTP_E_SUCCESS;
}

static int32
_ptp_bmc_port_state_decision_by_iterate(void *obj, tbl_iter_args_t *pargs)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_ptp_port_t* p_port = obj;
    
    tbl_ptp_foreign_t *p_best = pargs->argv[0];

    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (!p_port)
    {
        PTP_DBG_BMC("Exit current state decision for ptp port is NULL");
        return PTP_E_ERROR;//my add
    }

    if (!(p_port->enable && p_port->link_up))
    {
        //PTP_DBG_BMC("Exit current bmc computation for ptp port is NULL");
        return PTP_E_ERROR;//my add
    }

    if (PTP_STATE_FAULTY == p_port->port_ds.port_state || PTP_STATE_DISABLED == p_port->port_ds.port_state)
    {
        return PTP_E_SUCCESS;
    }

    if (!p_port->rbest && PTP_STATE_LISTENING == p_port->port_ds.port_state)
    {
        return PTP_E_SUCCESS;
    }

    if (ptp_ops_is_oc() && g_p_master->default_ds.slave_only)
    {
        p_port->recommended_state = _ptp_bmc_decision_slave_only_dataset(p_port, p_best);
    }
    else
    {
        p_port->recommended_state = _ptp_bmc_decision_dataset(p_port, p_best);
    }

    PTP_DBG_BMC("%s's recommended state is %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->recommended_state));
    if (PM_E_FAIL == p_port->recommended_state)
    {
        PTP_DBG_BMC("%s's recommended state is %s, is failed, return failed", ifname_ext, ptp_desc_state(p_port->recommended_state));
        return PTP_E_ERROR;
    }
    ptp_proto_to_state(p_port);
    
    return PTP_E_SUCCESS;
}

static int32
_ptp_bmc_check_ports_state_by_iterate(void *obj, tbl_iter_args_t *pargs)
{
    tbl_ptp_port_t* p_port = obj;
    uint32 *p_clear_global_ds = pargs->argv[0];
    uint32 clear_global_ds = *p_clear_global_ds;

    PTP_API_INVALID_INTERFACE_RET(p_port);

    if (!p_port)
    {
        PTP_DBG_BMC("Exit current port state check for ptp port is NULL");
        return PTP_E_ERROR;//my add
    }

    if (!(p_port->enable && p_port->link_up))
    {
        //PTP_DBG_BMC("Exit current state decision for ptp port is NULL");
        return PTP_E_ERROR;//my add
    }
    
    if (!clear_global_ds)
    {
        PTP_DBG_BMC("Exit current port state check for clear_global_ds is FALSE");
        return PTP_E_SUCCESS;
    }

    switch (p_port->port_ds.port_state)
    {
    case PTP_STATE_PRE_MASTER:
    case PTP_STATE_MASTER:
    case PTP_STATE_SLAVE:
    case PTP_STATE_UNCALIBRATED:
        *p_clear_global_ds = FALSE;
        break;
    default:
        break;
    }

    return PTP_E_SUCCESS;
}

void
ptp_bmc_execute(void)
{
    uint32 clear_global_ds = TRUE;
    tbl_ptp_foreign_t* p_best = NULL;
    tbl_iter_args_t args;    
    PTP_DBG_BMC("------BMC Execute Start--------");

    args.argv[0] = &p_best;
    tbl_ptp_port_iterate(_ptp_bmc_computation_by_iterate, &args);

    if (p_best)
    {
        PTP_DBG_BMC("E_best is %s", ptp_desc_clockid(p_best->key.foreign_master_port_identity.clock_id));
    }

    args.argv[0] = p_best;
    tbl_ptp_port_iterate(_ptp_bmc_port_state_decision_by_iterate, &args);

    args.argv[0] = &clear_global_ds;
    tbl_ptp_port_iterate(_ptp_bmc_check_ports_state_by_iterate, &args);

    if (clear_global_ds)
    {
        //PTP_DBG_BMC("Init dynamic properties");
        ptp_ops_init_dynamic_properties();
    }

    PTP_DBG_BMC("------BMC Execute End--------");
    
    return;
}

