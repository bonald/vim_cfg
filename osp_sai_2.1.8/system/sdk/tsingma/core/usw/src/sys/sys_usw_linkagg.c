/**
 @file sys_usw_linkagg.c

 @date 2009-10-19

 @version v2.0

 The file contains all Linkagg APIs of sys layer
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_debug.h"
#include "ctc_error.h"

#include "sys_usw_linkagg.h"
#include "sys_usw_chip.h"
#include "sys_usw_port.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_register.h"
#include "sys_usw_opf.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_dmps.h"
#include "sys_usw_common.h"

#include "drv_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
typedef struct sys_linkagg_rh_member_s
{
    uint32 dest_port;
    uint32 entry_count;
}sys_linkagg_rh_member_t;

typedef struct sys_linkagg_rh_entry_s
{
    uint32 dest_port;
    uint16  mem_idx;
    uint8  is_changed;
    uint8  rsv;
}sys_linkagg_rh_entry_t;

struct sys_linkagg_mem_s
{
    uint16  tid;                /**< linkAggId */
    uint16  mem_valid_num;      /**< member num of linkAgg group */
    uint8  linkagg_mode;       /**< ctc_linkagg_group_mode_t */
    uint8  need_pad;

    uint16  port_cnt;          /* member num of linkAgg group */
    uint8  ref_cnt;           /*for bpe cb cascade*/
    uint32 gport;             /**< member port */

    sys_linkagg_rh_entry_t* rh_entry; /*for resilent hash use*/
    uint16  port_index;
};
typedef struct sys_linkagg_mem_s sys_linkagg_mem_t;

struct sys_linkagg_stats_s
{
    uint8 lchip;
    uint32 total_group_cnt;
    uint32 total_member_cnt;
    uint32 static_group_cnt;
    uint32 static_group_lmpf_cnt;
    uint32 static_member_cnt;
    uint32 failover_group_cnt;
    uint32 failover_group_lmpf_cnt;
    uint32 failover_member_cnt;
    uint32 rr_group_cnt;
    uint32 rr_group_random_cnt;
    uint32 rr_member_cnt;
    uint32 dynamic_group_cnt;
    uint32 dynamic_member_cnt;
    uint32 resilient_group_cnt;
    uint32 resilient_member_cnt;
};
typedef struct sys_linkagg_stats_s sys_linkagg_stats_t;

/* dynamic interval */

#define SYS_LINKAGG_GOTO_ERROR(ret, error) \
    do { \
        if (ret < 0) \
        {\
            goto error;\
        } \
       } while (0)

#define SYS_LINKAGG_LOCK_INIT() \
    do { \
        int32 ret = sal_mutex_create(&(p_usw_linkagg_master[lchip]->p_linkagg_mutex)); \
        if (ret || !(p_usw_linkagg_master[lchip]->p_linkagg_mutex)) \
        { \
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Create linkagg mutex fail!\n"); \
            mem_free(p_usw_linkagg_master[lchip]); \
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");\
			return CTC_E_NO_MEMORY;\
 \
        } \
    } while (0)

#define SYS_LINKAGG_INIT_CHECK() \
    do { \
        SYS_LCHIP_CHECK_ACTIVE(lchip);       \
        if (NULL == p_usw_linkagg_master[lchip]){ \
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_TID_VALID_CHECK(tid) \
    if (tid >= p_usw_linkagg_master[lchip]->linkagg_num){\
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");\
			return CTC_E_BADID;\
 }

 #define SYS_MEM_NUM_VALID_CHECK(mem_num) \
    if (mem_num > MCHIP_CAP(SYS_CAP_LINKAGG_MEM_NUM)){\
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");\
			return CTC_E_INVALID_PARAM;\
 }

#define LINKAGG_LOCK \
    if (p_usw_linkagg_master[lchip]->p_linkagg_mutex) sal_mutex_lock(p_usw_linkagg_master[lchip]->p_linkagg_mutex)
#define LINKAGG_UNLOCK \
    if (p_usw_linkagg_master[lchip]->p_linkagg_mutex) sal_mutex_unlock(p_usw_linkagg_master[lchip]->p_linkagg_mutex)

#define SYS_LINKAGG_GET_GROUP_MEMBER_NUM(tid, mem_num) \
    if(p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_56) \
    { \
        mem_num = (tid > MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_TID_MAX)) ? 16 : 32; \
    } \
    else \
    { \
        uint8 linkagg_num = (p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_4)? 4:\
                            ((p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_8)? 8:\
                            ((p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_16)? 16:\
                            ((p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_32)? 32: 64)));\
        mem_num = 1024 / linkagg_num; \
    }

#define SYS_LINKAGG_GET_GROUP_MEMBER_BASE(tid, mem_num, mem_base) \
    if((p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_56) && (tid > MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_TID_MAX))) \
    { \
        mem_base =  MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_MEM_MAX) + (tid - MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_TID_MAX) -1) * mem_num; \
    } \
    else \
    { \
        mem_base = tid * mem_num; \
    }

STATIC bool
_sys_usw_linkagg_port_is_member(uint8 lchip, sys_linkagg_t* p_group, uint32 gport, uint16* index);

STATIC int32
_sys_usw_linkagg_remove_port(uint8 lchip, sys_linkagg_t* p_group, uint32 gport);

extern int32
sys_usw_l2_set_dsmac_for_linkagg_hw_learning(uint8 lchip, uint32 gport, bool b_add);

extern int32
sys_usw_parser_set_linkagg_hash_field(uint8 lchip, ctc_parser_linkagg_hash_ctl_t* p_hash_ctl);

extern int32
sys_usw_parser_get_linkagg_hash_field(uint8 lchip, ctc_parser_linkagg_hash_ctl_t* p_hash_ctl);

extern int32
sys_usw_linkagg_wb_sync(uint8 lchip);

extern int32
sys_usw_linkagg_wb_restore(uint8 lchip);

extern int32
sys_usw_linkagg_dump_db(uint8 lchip, sal_file_t dump_db_fp, ctc_global_dump_db_t* p_dump_param);

sys_linkagg_master_t* p_usw_linkagg_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

#define __HW_SYNC__
STATIC int32
_sys_usw_hw_sync_add_member(uint8 lchip, uint8 tid, uint16 lport)
{
    uint32 cmd = 0;
    DsLinkAggregateChannel_m linkagg_channel;
    uint32 channel_id = 0;
    uint8 gchip = 0;
    uint32 gport = 0;
    uint32 value = 0;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = 0x%x lport =  %d \n", tid, lport);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);

    cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));
    value = tid;
    SetDsLinkAggregateChannel(V, u1_g1_linkAggregationGroup_f, &linkagg_channel, value);
    SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 0);
    /*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*//*SYSTEM MODIFIED by xgu for sdk bug http://10.10.25.13/show_bug.cgi?id=35341, 2015-9-23*/
    SetDsLinkAggregateChannel(V, linkChangeEn_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 1);
    /*SYSTEM MODIFY end*/
    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_hw_sync_del_member(uint8 lchip, uint8 tid, uint16 lport)
{
    uint32 cmd = 0;
    DsLinkAggregateChannel_m linkagg_channel;
    uint8 channel_id = 0;
    uint8 gchip = 0;
    uint32 gport = 0;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = 0x%x lport =  %d \n", tid, lport);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);

    cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));
    SetDsLinkAggregateChannel(V, u1_g1_linkAggregationGroup_f, &linkagg_channel, 0);
    SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 0);
    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

int32
sys_usw_linkagg_failover_isr(uint8 lchip, uint32 intr, void* p_data)
{
    uint32 cmd = 0;
    LinkAggScanLinkDownChanRecord_m chan_down;
    uint16 index = 0;
    uint8 linkdown_chan = 0;
    sys_intr_type_t type;
    uint32 link_state[4] = {0};
    uint16 lport = 0;
    uint8  gchip = 0;
    uint32 gport = 0;

    SYS_LINKAGG_INIT_CHECK();
    type.intr = SYS_INTR_FUNC_CHAN_LINKDOWN_SCAN;
    type.sub_intr = INVG;

    /* mask linkdown interrupt */
    sys_usw_interrupt_set_en(lchip, &type, FALSE);

    sal_memset(&chan_down, 0, sizeof(chan_down));
    cmd = DRV_IOR(LinkAggScanLinkDownChanRecord_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_down));
    GetLinkAggScanLinkDownChanRecord(A, linkDownScanChanRecord_f, &chan_down, &(link_state[0]));

    for (index = 0; index < 64; index++)
    {
        if ((link_state[index/32] >>(index%32)) & 0x1)
        {
            linkdown_chan = index;
            /*normal trunk*/
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "chan_down:0x%x\n ", linkdown_chan);
        }
    }
    cmd = DRV_IOW(LinkAggScanLinkDownChanRecord_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_down));

    if (!p_usw_linkagg_master[lchip]->bind_gport_disable)
    {
        lport = SYS_GET_LPORT_ID_WITH_CHAN(lchip, linkdown_chan);
        SYS_MAX_PHY_PORT_CHECK(lport);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);;
        sys_usw_port_set_global_port(lchip, lport, gport, TRUE);
    }

    /* release linkdown interrupt */
    sys_usw_interrupt_set_en(lchip, &type, TRUE);

    return CTC_E_NONE;
}

#define __resilient_hash__
/*reset all member
  entry_buf is all zero
  member_buf need store all the dest gport
*/
STATIC int32
_sys_usw_rh_reset_all_member(uint8 lchip, uint16 entry_num, sys_linkagg_rh_entry_t* entry_buf,
                                               uint16 member_num, sys_linkagg_rh_member_t* member_buf)
{
    uint32 lower_bound = 0;
    uint32 upper_bound = 0;
    uint32 entry_idx = 0;
    uint32 temp_mem_idx = 0;
    uint32 threshold = 0;
    uint16 remained = 0;
    CTC_PTR_VALID_CHECK(entry_buf);
    CTC_PTR_VALID_CHECK(member_buf);

    lower_bound = entry_num /(member_num);
    upper_bound = entry_num %(member_num) ? (lower_bound+1):(lower_bound);
    remained = entry_num %(member_num);

    threshold = upper_bound;
    for(temp_mem_idx=0; temp_mem_idx<member_num; temp_mem_idx++)
    {
        /*random select upper bound or lower bound*/
        if(remained)
        {
            threshold = upper_bound;
            remained--;
        }
        else
        {
            threshold = lower_bound;
        }

        while(member_buf[temp_mem_idx].entry_count < threshold)
        {
            /*random select one entry, if entry is used, select the next*/
            entry_idx = sal_rand() % entry_num;

            while(entry_buf[entry_idx].is_changed)
            {
                entry_idx = (entry_idx+1)%entry_num;
            }

            entry_buf[entry_idx].dest_port = member_buf[temp_mem_idx].dest_port;
            entry_buf[entry_idx].is_changed = 1;

            (member_buf[temp_mem_idx].entry_count)++;
        }

    }
    return CTC_E_NONE;

}
STATIC int32
_sys_usw_rh_add_member_rebalance(uint8 lchip, uint16 entry_num, sys_linkagg_rh_entry_t* entry_buf,
                                               uint16 member_num, sys_linkagg_rh_member_t* member_buf,
                                               uint32 dest_port)
{
    uint32 lower_bound = 0;
    uint32 upper_bound = 0;
    uint32 entry_idx = 0;
    uint8  is_new_member = 0;
    uint32 temp_mem_idx = 0;
    uint32 next_entry_idx = 0;
    uint32 threshold = 0;
    CTC_PTR_VALID_CHECK(entry_buf);
    CTC_PTR_VALID_CHECK(member_buf);

    member_buf[member_num].dest_port = dest_port;
    member_buf[member_num].entry_count = 0;

    lower_bound = entry_num /(member_num + 1);
    upper_bound = entry_num %(member_num + 1) ? (lower_bound+1):(lower_bound);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lower bound:%d, upper bound:%d\n", lower_bound, upper_bound);

    threshold = upper_bound;
    while(member_buf[member_num].entry_count < lower_bound)
    {
        entry_idx = sal_rand() % entry_num;
        if(entry_buf[entry_idx].dest_port == dest_port)
        {
            is_new_member = 1;
        }
        else
        {
            is_new_member = 0;
            temp_mem_idx = entry_buf[entry_idx].mem_idx;
        }

        if(!is_new_member && (member_buf[temp_mem_idx].entry_count > threshold))
        {
            entry_buf[entry_idx].dest_port = dest_port;
            entry_buf[entry_idx].mem_idx =  member_num;
            entry_buf[entry_idx].is_changed =  1;
            member_buf[temp_mem_idx].entry_count --;
            member_buf[member_num].entry_count++;
        }
        else
        {
            next_entry_idx = (entry_idx+1)%(entry_num);
            while(next_entry_idx != entry_idx)
            {
                if(entry_buf[next_entry_idx].dest_port == dest_port)
                {
                    is_new_member = 1;
                }
                else
                {
                    is_new_member = 0;
                    temp_mem_idx = entry_buf[next_entry_idx].mem_idx;
                }

                if(!is_new_member && (member_buf[temp_mem_idx].entry_count > threshold))
                {
                    entry_buf[next_entry_idx].dest_port = dest_port;
                    entry_buf[next_entry_idx].mem_idx =  member_num;
                    entry_buf[next_entry_idx].is_changed =  1;

                    member_buf[temp_mem_idx].entry_count--;
                    member_buf[member_num].entry_count++;

                    break;
                }
                else
                {
                    next_entry_idx = (next_entry_idx+1)%(entry_num);
                }
            }

            if (next_entry_idx == entry_idx)
            {
                /* The entry count of all existing members has decreased
                 * to threshold. The entry count of the new member has
                 * not yet increased to lower_bound. Lower the threshold.
                 */
                threshold--;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_rh_remove_member_rebalance(uint8 lchip, uint16 entry_num, sys_linkagg_rh_entry_t* entry_buf,
                                               uint16 member_num, sys_linkagg_rh_member_t* member_buf,
                                               uint32 dest_port)
{
    uint32 lower_bound;
    uint32 upper_bound;
    uint32 entry_idx;
    uint32 temp_mem_idx;
    uint32 next_mem_idx;
    uint32 threshold;

    CTC_PTR_VALID_CHECK(entry_buf);
    CTC_PTR_VALID_CHECK(member_buf);

    lower_bound = entry_num / (member_num-1);
    upper_bound = entry_num %(member_num-1)?(lower_bound+1):(lower_bound);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lower bound:%d, upper bound:%d\n", lower_bound, upper_bound);

    threshold = lower_bound;
    for(entry_idx=0; entry_idx<entry_num; entry_idx++)
    {
        if(entry_buf[entry_idx].dest_port != dest_port)
        {
            continue;
        }

        member_buf[entry_buf[entry_idx].mem_idx].entry_count--;

        temp_mem_idx = sal_rand() % member_num;
        if((member_buf[temp_mem_idx].dest_port != dest_port) && (member_buf[temp_mem_idx].entry_count < threshold))
        {
            member_buf[temp_mem_idx].entry_count++;
            entry_buf[entry_idx].dest_port = member_buf[temp_mem_idx].dest_port;
            entry_buf[entry_idx].mem_idx = temp_mem_idx;
            entry_buf[entry_idx].is_changed =  1;
        }
        else
        {
            next_mem_idx = (temp_mem_idx+1)%member_num;
            while(next_mem_idx != temp_mem_idx)
            {
                if((member_buf[next_mem_idx].dest_port != dest_port) && (member_buf[next_mem_idx].entry_count < threshold))
                {
                    member_buf[next_mem_idx].entry_count++;
                    entry_buf[entry_idx].dest_port = member_buf[next_mem_idx].dest_port;
                    entry_buf[entry_idx].mem_idx = temp_mem_idx;
                    entry_buf[entry_idx].is_changed =  1;
                    break;
                }
                else
                {
                    next_mem_idx = (next_mem_idx+1)%member_num;
                }
            }

            if(next_mem_idx == temp_mem_idx)
            {
                threshold++;
                if((member_buf[temp_mem_idx].dest_port != dest_port) && (member_buf[temp_mem_idx].entry_count < threshold))
                {
                    member_buf[temp_mem_idx].entry_count++;
                    entry_buf[entry_idx].dest_port = member_buf[temp_mem_idx].dest_port;
                    entry_buf[entry_idx].mem_idx = temp_mem_idx;
                    entry_buf[entry_idx].is_changed =  1;
                }
                else
                {
                    next_mem_idx = (temp_mem_idx+1)%member_num;
                    while(next_mem_idx != temp_mem_idx)
                    {
                        if((member_buf[next_mem_idx].dest_port != dest_port) && (member_buf[next_mem_idx].entry_count < threshold))
                        {
                            member_buf[next_mem_idx].entry_count++;
                            entry_buf[entry_idx].dest_port = member_buf[next_mem_idx].dest_port;
                            entry_buf[entry_idx].mem_idx = temp_mem_idx;
                            entry_buf[entry_idx].is_changed =  1;
                            break;
                        }
                        else
                        {
                            next_mem_idx = (next_mem_idx+1)%member_num;
                        }
                    }

                    if(next_mem_idx == temp_mem_idx)
                    {
                        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "!!!Delete resilient hash member error!\n");
                        return CTC_E_INVALID_PARAM;
                    }
                }
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32  _sys_usw_rh_get_info_by_hw(uint8 lchip, uint16 entry_base, uint16 entry_num, sys_linkagg_rh_entry_t* entry_buf,
                                               sys_linkagg_rh_member_t* member_buf, uint16* mem_num_out)
{
    uint32 cmd = DRV_IOR( DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    DsLinkAggregateMember_m  linkagg_member;
    uint16 index;
    uint32 dest_port;
    uint16 member_idx = 0;

    *mem_num_out = 0;

    for(index=0; index < entry_num; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, entry_base+index, cmd, &linkagg_member));

        dest_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(GetDsLinkAggregateMember(V, destChipId_f, &linkagg_member),
                                                    GetDsLinkAggregateMember(V, destId_f, &linkagg_member));

        entry_buf[index].dest_port = dest_port;

        /*check is exist member*/
        for(member_idx=0; member_idx < *mem_num_out; member_idx++)
        {
            if(member_buf[member_idx].dest_port == dest_port)
            {
                member_buf[member_idx].entry_count++;
                break;
            }
        }
        /*Not exist member, new one*/
        if(member_idx == *mem_num_out)
        {
            member_buf[member_idx].dest_port = dest_port;
            member_buf[member_idx].entry_count = 1;

            (*mem_num_out)++;

        }

        entry_buf[index].mem_idx = member_idx;
    }

    return CTC_E_NONE;
}

#define __fragment_move__
/*
typedef int32 (* vector_traversal_fn)(void* array_data, void* user_data);
typedef void (* ctc_list_del_cb_t) (void* val);
typedef int32 (* ctc_list_cmp_cb_t) (void* val1, void* val2);
*/
STATIC int32 _sys_usw_linkagg_linklist_cmp(sys_linkagg_t* val1, sys_linkagg_t* val2)
{
    if(val1->member_base < val2->member_base)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

STATIC int32 _sys_usw_linkagg_fragment_traverse(sys_linkagg_t* val, ctc_linklist_t* p_member_list)
{
    ctc_listnode_add_sort(p_member_list, val);

    return CTC_E_NONE;
}

int32 sys_usw_linkagg_fragment(uint8 lchip)
{
    int32  ret = CTC_E_NONE;
    int32  free_length;
    uint8  find;
    sys_usw_opf_t opf;
    sys_linkagg_t   head_group;
    sys_linkagg_t* first_group = NULL;
    sys_linkagg_t* next_group = NULL;
    sys_linkagg_t* find_group = NULL;
    ctc_listnode_t* f_node;
    ctc_listnode_t* node;
    ctc_linklist_t* p_member_list = ctc_list_create((ctc_list_cmp_cb_t)_sys_usw_linkagg_linklist_cmp, NULL);

    if(!p_member_list)
    {
    	return CTC_E_NO_MEMORY;
    }

    if(p_usw_linkagg_master[lchip]->linkagg_mode != CTC_LINKAGG_MODE_FLEX)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Only flexiable mode support fragment\n");
        ret = CTC_E_NOT_SUPPORT;
        goto error;
    }

    sal_memset(&opf, 0, sizeof(opf));
    sal_memset(&head_group, 0, sizeof(head_group));

    ctc_listnode_add_sort(p_member_list, &head_group);
     /* vector raverse;*/
    CTC_ERROR_GOTO(ctc_vector_traverse(p_usw_linkagg_master[lchip]->group_vector,
                                (vector_traversal_fn)_sys_usw_linkagg_fragment_traverse, p_member_list), ret, error);

    for(f_node = p_member_list->head; f_node && f_node->next; f_node = f_node->next)
    {
        next_group = (sys_linkagg_t*)f_node->next->data;
        first_group = f_node->data;

    	free_length = next_group->member_base - first_group->member_base - first_group->max_member_num;

        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "first group info:\n");
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","tid", first_group->tid);
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","max member num", first_group->max_member_num);
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","member base", first_group->member_base);

    	if(free_length == 0)
    	{
            continue;
    	}
    	else if(free_length < 0)
    	{
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg fragment error!\n");
            ret =  CTC_E_INVALID_PARAM;
            goto error;
    	}

    	while(1)
    	{
             /*double check for while*/
            if(free_length <= 0)
            {
                break;
            }

            find = 0;
            for(node = p_member_list->tail; node; node=node->prev)
            {
                if(node == f_node)
                {
                    break;
                }

                find_group = (sys_linkagg_t*)node->data;

                if((find_group->mode != CTC_LINKAGG_MODE_DLB) && (find_group->mode != CTC_LINKAGG_MODE_STATIC_FAILOVER) &&
                    (find_group->max_member_num == free_length) && (find_group->member_base > first_group->member_base))
                {
                    find = 1;
                    break;
                }
            }

            if(find)
            {
                 /* first update  hardware*/
                uint16 mem_idx;
                DsLinkAggregateMember_m member;
                DsLinkAggregateGroup_m  group;
                uint16 new_member_base = first_group->member_base + first_group->max_member_num;
                uint32 cmdr = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
                uint32 cmdw = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);

                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "The find group info:\n");
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","tid", find_group->tid);
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","max member num", find_group->max_member_num);
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","member base", find_group->member_base);

                for(mem_idx=0; mem_idx < find_group->max_member_num; mem_idx++)
                {
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, find_group->member_base+mem_idx, cmdr, &member), ret, error);
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, new_member_base+mem_idx, cmdw, &member), ret, error);
                }

                cmdr = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
                cmdw = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);

                CTC_ERROR_GOTO(DRV_IOCTL(lchip, find_group->tid, cmdr, &group), ret, error);
                SetDsLinkAggregateGroup(V, linkAggMemBase_f, &group, new_member_base);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, find_group->tid, cmdw, &group), ret, error);

                opf.pool_type = p_usw_linkagg_master[lchip]->opf_type;
                opf.pool_index = 0;
                CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf, find_group->max_member_num,
                                                                        find_group->member_base), ret, error);

                CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, find_group->max_member_num,
                                                                        new_member_base), ret, error);

                find_group->member_base = new_member_base;
                ctc_listnode_delete_node(p_member_list, node);
                ctc_listnode_add_sort(p_member_list, find_group);
                break;
            }
            else
            {
                free_length -= MCHIP_CAP(SYS_CAP_LINKAGG_FRAGMENT_SIZE);
            }
    	}
    }

 error:
    ctc_list_delete(p_member_list);
    return ret;
}

#define __DLB__
STATIC uint16
_sys_usw_linkagg_driver2flows(uint8 lchip, uint8 driver_value)
{
    uint16 flow_num = 0;

    if(DRV_IS_DUET2(lchip))
    {
        switch (driver_value)
        {
            case 0:
                flow_num = 256;
                break;
            case 1:
                flow_num = 128;
                break;
            case 2:
                flow_num = 64;
                break;
            default:
                flow_num = 32;
        }
    }
    else
    {
        switch (driver_value)
        {
            case 0:
                flow_num = 16;
                break;
            case 1:
                flow_num = 32;
                break;
            case 2:
                flow_num = 64;
                break;
            case 3:
                flow_num = 128;
                break;
            case 4:
                flow_num = 256;
                break;
            case 5:
                flow_num = 512;
                break;
            case 6:
                flow_num = 1024;
                break;
            default:
                flow_num = 2048;
        }
    }

    return flow_num;
}
STATIC uint32
_sys_usw_linkagg_flows2driver(uint8 lchip, uint16 num_flow)
{
    uint32 driver_value = 0;

    if(DRV_IS_DUET2(lchip))
    {
        switch (num_flow)
        {
            case 256:
                driver_value = 0;
                break;
            case 128:
                driver_value = 1;
                break;
            case 64:
                driver_value = 2;
                break;
            case 32:
                driver_value = 3;
                break;
            default:
                return driver_value = 4;
        }
    }
    else
    {
        switch (num_flow)
        {
            case 16:
                driver_value = 0;
                break;
            case 32:
                driver_value = 1;
                break;
            case 64:
                driver_value = 2;
                break;
            case 128:
                driver_value = 3;
                break;
            case 256:
                driver_value = 4;
                break;
            case 512:
                driver_value = 5;
                break;
            case 1024:
                driver_value = 6;
                break;
            default:
                driver_value = 7;
        }
    }

    return driver_value;
}
STATIC char *
_sys_usw_linkagg_mode2str(uint8 mode)
{
    char *str = NULL;

    switch (mode)
    {
        case CTC_LINKAGG_MODE_4:
        {
            str = "mode 4";
            break;
        }
        case CTC_LINKAGG_MODE_8:
        {
            str = "mode 8";
            break;
        }
        case CTC_LINKAGG_MODE_16:
        {
            str = "mode 16";
            break;
        }
        case CTC_LINKAGG_MODE_32:
        {
            str = "mode 32";
            break;
        }
        case CTC_LINKAGG_MODE_64:
        {
            str = "mode 64";
            break;
        }
        case CTC_LINKAGG_MODE_56:
        {
            str = "mode 56";
            break;
        }
        case CTC_LINKAGG_MODE_FLEX:
        {
            str = "mode flex";
            break;
        }
        default:
        {
            str = "unknown";
            break;
        }
    }

    return str;
}

STATIC uint32
_sys_usw_dlb_get_real_member_num(uint8 lchip, uint8 tid)
{
    uint32 cmd_r;
    uint32 field_val = 0;

    cmd_r = DRV_IOR(DsLinkAggregateGroup_t, DsLinkAggregateGroup_linkAggMemNum_f);
    DRV_IOCTL(lchip, tid, cmd_r, &field_val);

    return field_val;
}

STATIC int32
_sys_usw_dlb_add_member_channel(uint8 lchip, uint8 tid, uint16 lport)
{
    uint32 cmd = 0;
    DsLinkAggregateChannel_m linkagg_channel;
    uint8 channel_id = 0;
    uint8 gchip = 0;
    uint32 gport = 0;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = 0x%x lport =  %d \n", tid, lport);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);

    SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 0);     /* port level */
    SetDsLinkAggregateChannel(V, linkChangeEn_f, &linkagg_channel, 0);
    SetDsLinkAggregateChannel(V, u1_g1_linkAggregationGroup_f, &linkagg_channel, tid);

    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dlb_del_member_channel(uint8 lchip, uint8 tid, uint16 lport)
{
    uint32 cmd = 0;
    DsLinkAggregateChannel_m linkagg_channel;
    uint8 channel_id = 0;
    uint8 gchip = 0;
    uint32 gport = 0;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = 0x%x lport =  %d \n", tid, lport);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);

    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dlb_add_member(uint8 lchip, uint8 tid, uint16 port_index, uint16 lport)
{
    uint32 cmd = 0;
    uint32 channel_id = 0;
    uint32 local_phy_port = 0;
    uint8 gchip = 0;
    uint32 gport = 0;

    if (port_index > MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Overrange the max number of dlb!\n");
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    local_phy_port = lport;

    cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + port_index * 2);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &channel_id));

    cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + port_index * 2);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &local_phy_port));

    CTC_ERROR_RETURN(_sys_usw_dlb_add_member_channel(lchip, tid, lport));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dlb_del_member(uint8 lchip, uint8 tid, uint16 port_index, uint16 tail_index, uint16 lport)
{
    uint32 cmd = 0;
    uint32 channel_id = 0;
    uint32 local_phy_port = 0;

    if ((port_index > MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM)) || (tail_index > MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM)))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Overrange the max number of dlb!\n");
        return CTC_E_INVALID_PARAM;
    }

    if (port_index != tail_index)
    {
        /* copy the last one to the removed position */
        cmd = DRV_IOR(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + tail_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &channel_id));

        cmd = DRV_IOR(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + tail_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &local_phy_port));

        cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + port_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &channel_id));

        cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + port_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &local_phy_port));

        /* set the last one to reserve */
        channel_id = SYS_RSV_PORT_DROP_ID;
        cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + tail_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &channel_id));

        local_phy_port = SYS_RSV_PORT_DROP_ID;
        cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + tail_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &local_phy_port));
    }
    else
    {
        /* set the removed one to reserve */
        channel_id = SYS_RSV_PORT_DROP_ID;
        cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + port_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &channel_id));

        local_phy_port = SYS_RSV_PORT_DROP_ID;
        cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + port_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &local_phy_port));
    }

    CTC_ERROR_RETURN(_sys_usw_dlb_del_member_channel(lchip, tid, lport));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dlb_del_member_without_copy(uint8 lchip, uint8 tid, uint16 port_index, uint16 lport)
{
    uint32 cmd = 0;
    uint32 channel_id = 0;
    uint32 local_phy_port = 0;

    if (port_index > MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Overrange the max number of dlb!\n");
        return CTC_E_INVALID_PARAM;
    }

    /* set the removed one to reserve */
    channel_id = SYS_RSV_PORT_DROP_ID;
    cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + port_index * 2);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &channel_id));

    local_phy_port = SYS_RSV_PORT_DROP_ID;
    cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + port_index * 2);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &local_phy_port));

    CTC_ERROR_RETURN(_sys_usw_dlb_del_member_channel(lchip, tid, lport));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_linkagg_reverve_member(uint8 lchip, uint8 tid)
{
    uint32 channel_id = 0;
    uint32 local_phy_port = 0;
    uint32 cmd_member_set = 0;
    uint16 index = 0;

    channel_id = SYS_RSV_PORT_DROP_ID;
    local_phy_port = SYS_RSV_PORT_DROP_ID;

    for (index = 0; index < 16; index++)
    {
        cmd_member_set = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_channelId_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd_member_set, &channel_id));
        cmd_member_set = DRV_IOW(DsLinkAggregateMemberSet_t, DsLinkAggregateMemberSet_array_0_localPhyPort_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd_member_set, &local_phy_port));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to clear flow active in linkagg group
*/
STATIC int32
_sys_usw_linkagg_clear_flow_active(uint8 lchip, uint8 tid)
{
    uint32 cmd_r = 0;
    uint32 cmd_w = 0;
    DsLinkAggregateGroup_m ds_link_aggregate_group;
    DsLinkAggregateMember_m ds_link_aggregate_member;
    uint16 flow_num = 0;
    uint32 agg_base = 0;
    uint16 index = 0;

    sal_memset(&ds_link_aggregate_group, 0, sizeof(DsLinkAggregateGroup_m));
    sal_memset(&ds_link_aggregate_member, 0, sizeof(DsLinkAggregateMember_m));

    cmd_r = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd_r, &ds_link_aggregate_group));

    agg_base = GetDsLinkAggregateGroup(V, linkAggMemBase_f, &ds_link_aggregate_group);
    flow_num = _sys_usw_linkagg_driver2flows(lchip, GetDsLinkAggregateGroup(V, linkAggFlowNum_f, &ds_link_aggregate_group));
    /* clear active */
    cmd_r = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    cmd_w = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);

    for (index = 0; index < flow_num; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, agg_base+index, cmd_r, &ds_link_aggregate_member));
        SetDsLinkAggregateMember(V, active_f, &ds_link_aggregate_member, 0);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, agg_base+index, cmd_w, &ds_link_aggregate_member));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_linkagg_add_padding_mem(uint8 lchip, sys_linkagg_t* p_group, sys_linkagg_mem_t * p_linkagg_mem, uint16 start_index)
{
    uint16 mem_num = 0;
    uint16 index = 0;
    uint32 cmd_member_w = 0;
    uint32 cmd_member_r = 0;
    uint16 agg_base = 0;
    DsLinkAggregateMember_m ds_linkagg_member;
    uint8 temp = 0;
    uint8 act_mem_num = 0;

    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));

    cmd_member_w = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    cmd_member_r = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    agg_base = p_group->member_base;
    mem_num = p_group->max_member_num;

    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));

    act_mem_num = p_linkagg_mem->mem_valid_num;

    for (index = start_index + 1; index < mem_num; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (agg_base + (temp % act_mem_num)), cmd_member_r, &ds_linkagg_member));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (agg_base + index), cmd_member_w, &ds_linkagg_member));
        temp++;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_linkagg_get_repeat_member_cnt(uint8 lchip, sys_linkagg_t* p_group, uint16 gport, uint16* repeat_cnt)
{
    uint16 mem_idx = 0;
    uint32 cmd = 0;
    uint8  chip_id = 0;
    uint16 destId = 0;
    uint32 gport_temp = 0;
    uint16 loop_num = 0;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateMemberSet_m ds_linkagg_memberset;

    CTC_PTR_VALID_CHECK(p_group);
    CTC_PTR_VALID_CHECK(repeat_cnt);

    (*repeat_cnt) = 0;
    if (1 >= p_group->real_member_num)
    {
        return CTC_E_NONE;
    }
    loop_num = (p_group->mode == CTC_LINKAGG_MODE_RESILIENT) ? p_group->max_member_num : (p_group->real_member_num - 1);
    for (mem_idx = p_group->member_base; mem_idx < (p_group->member_base + loop_num); mem_idx++)
    {
        if (p_group->mode == CTC_LINKAGG_MODE_DLB)
        {
            sal_memset(&ds_linkagg_memberset, 0, sizeof(ds_linkagg_memberset));
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &chip_id));
            cmd = DRV_IOR(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_group->tid, cmd, &ds_linkagg_memberset));
            destId = GetDsLinkAggregateMemberSet(V, array_0_localPhyPort_f + (mem_idx - p_group->member_base) * 2, &ds_linkagg_memberset);
        }
        else
        {
            sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));
            cmd = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_idx, cmd, &ds_linkagg_member));
            chip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
            destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
        }
        gport_temp = CTC_MAP_LPORT_TO_GPORT(chip_id, destId);
        if (gport_temp == gport)
        {
            (*repeat_cnt)++;
        }
    }
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:%d, repeat_cnt:%d\n", gport,*repeat_cnt);
    return CTC_E_NONE;
}

/**
 @brief The function is update asic table, add/remove member port from linkagg.
*/
STATIC int32
_sys_usw_linkagg_update_table(uint8 lchip, sys_linkagg_t* p_group, sys_linkagg_mem_t* p_linkagg_mem, bool is_add_port)
{
    uint32 cmd_group_r = 0;
    uint32 cmd_group_w = 0;
    uint32 cmd_member_w = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    uint32 cmd_member_r = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    uint16 port_cnt = 0;
    uint16 mem_base = p_group->member_base;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateGroup_m ds_linkagg_group;
    uint16 lport = 0;
    uint16 tmp_lport = 0;
    int32 ret = 0;
    uint16 tail_index = 0;
    uint8 dest_chip_id = 0;
    uint16 port_index  = p_linkagg_mem->port_index;
    uint16 repeat_mem_cnt = 0;

    sal_memset(&ds_linkagg_member, 0, sizeof(DsLinkAggregateMember_m));
    sal_memset(&ds_linkagg_group, 0, sizeof(DsLinkAggregateGroup_m));

    tail_index = p_linkagg_mem->mem_valid_num;

    if (TRUE == is_add_port)
    {
        port_cnt = p_linkagg_mem->port_cnt;

        /*update DsLinkAggregateMember_t */
        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_linkagg_mem->gport);

        switch(p_linkagg_mem->linkagg_mode)
        {
            case CTC_LINKAGG_MODE_STATIC:
            case CTC_LINKAGG_MODE_STATIC_FAILOVER:
            case CTC_LINKAGG_MODE_RR:
            {
                dest_chip_id = SYS_MAP_CTC_GPORT_TO_GCHIP(p_linkagg_mem->gport);
                SetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member, lport);
                SetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member, dest_chip_id);

                cmd_member_w = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + port_index), cmd_member_w, &ds_linkagg_member));

                if (p_linkagg_mem->linkagg_mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
                {
                    CTC_ERROR_RETURN(_sys_usw_hw_sync_add_member(lchip, p_linkagg_mem->tid, lport));
                }
                break;
            }
            case CTC_LINKAGG_MODE_DLB:
            {
                CTC_ERROR_RETURN(_sys_usw_dlb_add_member(lchip, p_linkagg_mem->tid, port_index, lport));
                break;
            }
            case CTC_LINKAGG_MODE_RESILIENT:
            {
                uint16 mem_idx;
                sys_linkagg_rh_entry_t* p_entry;

                for(mem_idx=0; mem_idx<p_linkagg_mem->mem_valid_num; mem_idx++)
                {
                    p_entry = p_linkagg_mem->rh_entry + mem_idx;
                    if(p_entry && p_entry->is_changed)
                    {
                        tmp_lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_entry->dest_port);
                        dest_chip_id = SYS_MAP_CTC_GPORT_TO_GCHIP(p_entry->dest_port);
                        SetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member, tmp_lport);
                        SetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member, dest_chip_id);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + mem_idx), cmd_member_w, &ds_linkagg_member));
                    }
                }
                break;
            }
            default:
                return CTC_E_NOT_SUPPORT;
        }

        /* update DsLinkAggregateGroup_t linkagg port cnt */
        cmd_group_r = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_linkagg_mem->tid, cmd_group_r, &ds_linkagg_group));
        SetDsLinkAggregateGroup(V, linkAggMemNum_f, &ds_linkagg_group, port_cnt);
        cmd_group_w = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_linkagg_mem->tid, cmd_group_w, &ds_linkagg_group));

        if (p_linkagg_mem->need_pad)
        {
            ret = _sys_usw_linkagg_add_padding_mem(lchip, p_group, p_linkagg_mem, p_linkagg_mem->mem_valid_num);
            if (ret < 0)
            {
                return ret;
            }
        }
    }
    else
    {
        /*before this function calling, the port cnt has been decreased.*/
        port_cnt = p_linkagg_mem->port_cnt;
        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_linkagg_mem->gport);

        /* update DsLinkAggregateGroup_t linkagg port cnt */
        cmd_group_r = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_linkagg_mem->tid, cmd_group_r, &ds_linkagg_group));
        SetDsLinkAggregateGroup(V, linkAggMemNum_f, &ds_linkagg_group, port_cnt);
        cmd_group_w = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_linkagg_mem->tid, cmd_group_w, &ds_linkagg_group));

        switch(p_linkagg_mem->linkagg_mode)
        {
            case CTC_LINKAGG_MODE_STATIC:
            case CTC_LINKAGG_MODE_STATIC_FAILOVER:
            case CTC_LINKAGG_MODE_RR:
            {
                if (0 == port_cnt)
                {
                    if (p_linkagg_mem->linkagg_mode != CTC_LINKAGG_MODE_DLB)
                    {
                        cmd_member_w = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + port_index), cmd_member_w, &ds_linkagg_member));
                    }
                }
                else
                {
                    if(CTC_FLAG_ISSET(p_group->flag, CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER))
                    {
                        uint16 tmp_index = port_index;
                        for(;tmp_index < tail_index; tmp_index++)
                        {
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_index+mem_base+1, cmd_member_r, &ds_linkagg_member));
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_index+mem_base, cmd_member_w, &ds_linkagg_member));
                        }
                    }
                    else
                    {
                        if (port_index != tail_index)
                        {
                            /*copy the last one to the removed port position,and remove member port from linkagg at tail*/
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + tail_index), cmd_member_r, &ds_linkagg_member));
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + port_index), cmd_member_w, &ds_linkagg_member));
                        }
                    }
                    if (p_linkagg_mem->need_pad)
                    {
                        CTC_ERROR_RETURN(_sys_usw_linkagg_add_padding_mem(lchip, p_group, p_linkagg_mem, tail_index - 1));
                    }
                }

                if (p_linkagg_mem->linkagg_mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
                {
                    CTC_ERROR_RETURN(_sys_usw_hw_sync_del_member(lchip, p_linkagg_mem->tid, lport));
                }
                break;
            }
            case CTC_LINKAGG_MODE_DLB:
            {
                CTC_ERROR_RETURN(_sys_usw_dlb_del_member(lchip, p_linkagg_mem->tid, port_index, tail_index, lport));
                break;
            }
            case CTC_LINKAGG_MODE_RESILIENT:
            {
                uint16 mem_idx;
                sys_linkagg_rh_entry_t* p_entry;

                for(mem_idx = 0; mem_idx < p_linkagg_mem->mem_valid_num; mem_idx++)
                {
                    p_entry = p_linkagg_mem->rh_entry + mem_idx;
                    if(p_entry && p_entry->is_changed)
                    {
                        tmp_lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_entry->dest_port);
                        dest_chip_id = SYS_MAP_CTC_GPORT_TO_GCHIP(p_entry->dest_port);
                        SetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member, tmp_lport);
                        SetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member, dest_chip_id);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + mem_idx), cmd_member_w, &ds_linkagg_member));
                    }
                }
                break;
            }
            default:
                return CTC_E_NOT_SUPPORT;
        }
    }

    CTC_ERROR_RETURN(_sys_usw_linkagg_get_repeat_member_cnt(lchip, p_group, p_linkagg_mem->gport, &repeat_mem_cnt));
    /*update mcast linkagg bitmap*/
    if (TRUE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(p_linkagg_mem->gport)) && p_linkagg_mem->tid < MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
    {
        if ((is_add_port == TRUE) || ((is_add_port == FALSE) && (repeat_mem_cnt == 0)))
        {
            CTC_ERROR_RETURN(sys_usw_port_update_mc_linkagg(lchip, p_linkagg_mem->tid, lport, is_add_port));
        }
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to check whether the port is member of the linkagg.
*/
STATIC bool
_sys_usw_linkagg_port_is_member(uint8 lchip, sys_linkagg_t* p_group, uint32 gport, uint16* index)
{
    uint16 mem_idx = 0;
    uint16 mem_num = 0;
    uint16 mem_max_num = 0;
    uint16 mem_base = 0;
    uint32 cmd_member = 0;
    uint32 cmd_memberset = 0;
    uint8  chip_id = 0;
    uint16 destId = 0;
    uint32 gport_temp = 0;
    uint8  tid = p_group->tid;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateMemberSet_m ds_linkagg_memberset;

    CTC_PTR_VALID_CHECK(index);

    if (0 == p_group->real_member_num)
    {
        return FALSE;
    }

    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));
    sal_memset(&ds_linkagg_memberset, 0, sizeof(ds_linkagg_memberset));

    cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    cmd_memberset = DRV_IOR(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);

    mem_base = p_group->member_base;
    mem_num = p_group->real_member_num;
    mem_max_num = p_group->max_member_num;

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member));
                chip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(chip_id, destId);
                if (gport_temp == gport)
                {
                    *index = mem_idx - mem_base;
                    return TRUE;
                }
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member));
                chip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd_memberset, &ds_linkagg_memberset));
                destId = GetDsLinkAggregateMemberSet(V, array_0_localPhyPort_f + (mem_idx - mem_base) * 2, &ds_linkagg_memberset);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(chip_id, destId);
                if (gport_temp == gport)
                {
                    *index = mem_idx - mem_base;
                    return TRUE;
                }
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_max_num); mem_idx++)
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member));
                chip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(chip_id, destId);
                if (gport_temp == gport)
                {
                    *index = mem_idx - mem_base;
                    return TRUE;
                }
            }
            break;
        }
        default:
        {
            return FALSE;
        }
    }

    return FALSE;
}

STATIC int32
_sys_usw_lag_engine_dlb_init(uint8 lchip)
{
    uint32 value = 0;
    uint32 cmd = 0;
    /*uint32 core_frequecy = 0;*/
    uint8  i = 0;
    LagEngineDreTimerCtl_m lag_engine_dre_ctl;
    LagEngineCtl_m lag_engine_ctl;
    LagEngineDlbCtl_m lag_engine_dlb_ctl;
    DlbChanMaxLoadByteCnt_m chan_max_load_byte;
    uint8 chan_id = 0;

    sal_memset(&lag_engine_dre_ctl, 0, sizeof(lag_engine_dre_ctl));
    sal_memset(&lag_engine_ctl, 0, sizeof(lag_engine_ctl));
    sal_memset(&lag_engine_dlb_ctl, 0, sizeof(lag_engine_dlb_ctl));
    sal_memset(&chan_max_load_byte, 0, sizeof(chan_max_load_byte));

    /*core_frequecy = sys_usw_get_core_freq(lchip, 0);*/

    /* 1. config LagEngineDreTimerCtl_m and RefDivQMgrEnqDrePulse_t for Tp and Rp, if Tp=100us, Rp=1/512,
       than the history load will be subtract to 0 every 50ms */
    SetLagEngineDreTimerCtl(V, chanDreUpdEn_f, &lag_engine_dre_ctl, 1);
    SetLagEngineDreTimerCtl(V, chanDreInterval_f, &lag_engine_dre_ctl, 100);
     /*-V5.0 SetLagEngineDreTimerCtl(V, chanDreMaxPtr_f, &lag_engine_dre_ctl, 63);*/
    /* Rp = 1/(2^chanDreDiscountShift) = 1/512 */
    for (i = 0; i < 64; ++i)
    {
        SetLagEngineDreTimerCtl(V, array_0_chanDreDiscountShift_f + i, &lag_engine_dre_ctl, 9);
    }

    cmd = DRV_IOW(LagEngineDreTimerCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_dre_ctl));

    /* Config pulse timer for Tp=100us*/
    value = (625*25/DOWN_FRE_RATE) << 8;   /* value=4000000   */
    cmd = DRV_IOW(RefDivLinkAggDrePulse_t, RefDivLinkAggDrePulse_cfgRefDivLinkAggDrePulse_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 0;
    cmd = DRV_IOW(RefDivLinkAggDrePulse_t, RefDivLinkAggDrePulse_cfgResetDivLinkAggDrePulse_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* 2. config DlbChanMaxLoadByteCnt_t and shift, unit is 100 usec */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold0_f, &chan_max_load_byte,  12500);     /* 1G or 2.5G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold1_f, &chan_max_load_byte,  62500);     /* 5G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold2_f, &chan_max_load_byte,  125000);    /* 10G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold3_f, &chan_max_load_byte,  250000);    /* 20G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold4_f, &chan_max_load_byte,  312500);    /* 25G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold5_f, &chan_max_load_byte,  500000);    /* 40G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold6_f, &chan_max_load_byte,  625000);    /* 50G */
    SetDlbChanMaxLoadByteCnt(V, maxLoadThreshold7_f, &chan_max_load_byte, 1250000);    /* 100G */
    cmd = DRV_IOW(DlbChanMaxLoadByteCnt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_max_load_byte));

    value = 0;
    cmd = DRV_IOW(DlbEngineCtl_t, DlbEngineCtl_chanByteCountShift_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* config DlbChanMaxLoadType */
    for (chan_id = 0; chan_id < 64; chan_id++)
    {
        CTC_ERROR_RETURN(sys_usw_dmps_set_dlb_chan_type(lchip, chan_id));
    }

    /* 3. config LagEngineCtl_t for member move check */
    cmd = DRV_IOR(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl));

    SetLagEngineCtl(V, portLagMemberMoveCheckEn_f, &lag_engine_ctl, 1);
    SetLagEngineCtl(V, rrProtectDisable_f,&lag_engine_ctl, 0);
    SetLagEngineCtl(V, iLoopOamDestId_f, &lag_engine_ctl, SYS_RSV_PORT_ILOOP_ID);
    /*SetLagEngineCtl(V, chanBandwidthShift0_f, &lag_engine_ctl, 8);*/

    cmd = DRV_IOW(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl));

    /* 4. config LagEngineDlbCtl_t for dre counter */
    cmd = DRV_IOR(LagEngineDlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_dlb_ctl));

    SetLagEngineDlbCtl(V, dlbEn_f, &lag_engine_dlb_ctl, 1);
    SetLagEngineDlbCtl(V, byteCountShift_f, &lag_engine_dlb_ctl, 0);  /* unit of dre counter */
    SetLagEngineDlbCtl(V, ipg_f, &lag_engine_dlb_ctl, 20);     /* for dre ipg */

    cmd = DRV_IOW(LagEngineDlbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_dlb_ctl));

    value = 1;
    cmd = DRV_IOW(LagEngineLinkState_t, LagEngineLinkState_linkState_f);
    for(chan_id=0; chan_id < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); chan_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &value));
    }

    return CTC_E_NONE;
}

STATIC int32 _sys_usw_linkagg_add_db(uint8 lchip, uint16 tid, ctc_linkagg_group_t* p_linkagg_grp, sys_linkagg_t** p_group_out)
{
    uint32 mem_num;
    uint32 mem_base;
    int32  ret = CTC_E_NONE;
    sys_linkagg_t* p_group;
    sys_usw_opf_t opf;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_FLEX)
    {
        mem_num = p_linkagg_grp->member_num;
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = p_usw_linkagg_master[lchip]->opf_type;
        opf.multiple = MCHIP_CAP(SYS_CAP_LINKAGG_FRAGMENT_SIZE);
        ret = sys_usw_opf_alloc_offset(lchip, &opf, mem_num, &mem_base);
        if (ret == CTC_E_NO_RESOURCE)
        {
            sys_usw_linkagg_fragment(lchip);
            CTC_ERROR_GOTO(sys_usw_opf_alloc_offset(lchip, &opf, mem_num, &mem_base), ret, error);
        }
        else if (ret)
        {
            goto error;
        }
    }
    else
    {
        SYS_LINKAGG_GET_GROUP_MEMBER_NUM(tid, mem_num);
        SYS_LINKAGG_GET_GROUP_MEMBER_BASE(tid, mem_num, mem_base)
    }

    p_group = mem_malloc(MEM_LINKAGG_MODULE, sizeof(sys_linkagg_t));
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error_1;
    }

    sal_memset(p_group, 0, sizeof(sys_linkagg_t));
    p_group->max_member_num = mem_num;
    p_group->member_base = mem_base;
    p_group->mode = p_linkagg_grp->linkagg_mode;
    p_group->tid = tid;
    p_group->flag = p_linkagg_grp->flag;
    CTC_ERROR_GOTO(ctc_vector_add(p_usw_linkagg_master[lchip]->group_vector, tid, p_group), ret, error_2);

    if(p_group_out)
    {
        *p_group_out = p_group;
    }

    return CTC_E_NONE;

error_2:
    mem_free(p_group);
error_1:
    if (p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_FLEX)
    {
        sys_usw_opf_free_offset(lchip, &opf, mem_num, mem_base);
    }
error:
    return ret;
}

STATIC int32 _sys_usw_linkagg_remove_db(uint8 lchip, uint16 tid)
{
    sys_usw_opf_t opf;
    sys_linkagg_t* p_group = NULL;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(!p_group)
    {
        return CTC_E_NONE;
    }

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = p_usw_linkagg_master[lchip]->opf_type;
    sys_usw_opf_free_offset(lchip, &opf, p_group->max_member_num, p_group->member_base);

    ctc_vector_del(p_usw_linkagg_master[lchip]->group_vector, tid);

    mem_free(p_group);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_linkagg_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}
STATIC uint8
_sys_usw_linkagg_find_rr_profile_id(uint8 lchip, uint8 tid)
{
    uint8 loop;
    if(DRV_IS_DUET2(lchip))
    {
        return tid;
    }
    for(loop=0; loop < 16; loop++)
    {
        if(!CTC_IS_BIT_SET(p_usw_linkagg_master[lchip]->rr_profile_bmp, loop))
        {
            break;
        }
    }
    return loop;
}

int32 _sys_usw_linkagg_group_param_check(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp )
{
    CTC_PTR_VALID_CHECK(p_linkagg_grp);
    SYS_TID_VALID_CHECK(p_linkagg_grp->tid);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkAggId = 0x%x", p_linkagg_grp->tid);

    if(CTC_FLAG_ISSET(p_linkagg_grp->flag, CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER) && p_linkagg_grp->linkagg_mode != CTC_LINKAGG_MODE_STATIC)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Only static mode support member in ascend order\n");
        return CTC_E_INVALID_PARAM;
    }

    switch (p_linkagg_grp->linkagg_mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        {
            if(CTC_FLAG_ISSET(p_linkagg_grp->flag, CTC_LINKAGG_GROUP_FLAG_RANDOM_RR))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "static mode no support random rr!\n");
                return CTC_E_INVALID_PARAM;
            }
            /* in tsgingMa, mode 4 do not support static group for waste of member */
            if (DRV_IS_TSINGMA(lchip) && CTC_LINKAGG_MODE_4 == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode 4 do not support static group!\n");
                return CTC_E_INVALID_PARAM;
            }

            if (CTC_LINKAGG_MODE_FLEX == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                /* static member_num range: 1~24/32/64/128 */
                if (0 == p_linkagg_grp->member_num)
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "static member_num can not be zero!\n");
                    return CTC_E_INVALID_PARAM;
                }
                else if (p_linkagg_grp->member_num > SYS_LINKAGG_MEM_NUM_1 && (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) != 256)) /*duet2 static mode member_num*/
                {
                    if (SYS_LINKAGG_MEM_NUM_2 != p_linkagg_grp->member_num &&
                        SYS_LINKAGG_MEM_NUM_3 != p_linkagg_grp->member_num &&
                        SYS_LINKAGG_MEM_NUM_4 != p_linkagg_grp->member_num)
                    {
                        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "static member_num invalid!\n");
                        return CTC_E_INVALID_PARAM;
                    }
                }
                else if(p_linkagg_grp->member_num > 255)
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "static member_num out of max_num!\n");
                    return CTC_E_INVALID_PARAM;
                }
            }

            if (CTC_LINKAGG_MODE_FLEX != p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                /* nor flex mode do not support local member first */
                if (p_linkagg_grp->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "only flex mode  support local member first!\n");
                    return CTC_E_INVALID_PARAM;
                }
            }

            break;
        }
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        {
            if(CTC_FLAG_ISSET(p_linkagg_grp->flag,CTC_LINKAGG_GROUP_FLAG_RANDOM_RR) || \
                CTC_FLAG_ISSET(p_linkagg_grp->flag,CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER))
            {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode static failover do not support FLAG_RANDOM_RR or FLAG_MEM_ASCEND_ORDER!\n");
                return CTC_E_INVALID_PARAM;
            }
            /* mode 4 do not support failover group for waste of member */
            if (CTC_LINKAGG_MODE_4 == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode 4 do not support failover group !\n");
                return CTC_E_INVALID_PARAM;
            }

            if (SYS_LINKAGG_MEM_NUM_1 < p_linkagg_grp->member_num)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode static failover member_num invalid\n");
                return CTC_E_INVALID_PARAM;
            }

            if (CTC_LINKAGG_MODE_FLEX == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                /* static failover member_num range: 1~24 */
                if ( (0 == p_linkagg_grp->member_num) || (DRV_IS_DUET2(lchip) && p_linkagg_grp->member_num > SYS_LINKAGG_MEM_NUM_1))
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "static failover member_num invalid!\n");
                    return CTC_E_INVALID_PARAM;
                }
            }

            if (p_linkagg_grp->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "nor flex mode do not support local member first!\n");
                return CTC_E_INVALID_PARAM;
            }


            break;
        }
        case CTC_LINKAGG_MODE_RR:
        {
            if(CTC_FLAG_ISSET(p_linkagg_grp->flag,CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST) || \
                CTC_FLAG_ISSET(p_linkagg_grp->flag,CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode rr do not support FLAG_LOCAL_MEMBER_FIRST or FLAG_MEM_ASCEND_ORDER!\n");
                return CTC_E_INVALID_PARAM;
            }
            /* mode 4 do not support rr group for waste of member */
            if (CTC_LINKAGG_MODE_4 == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode 4 do not support rr group!\n");
                return CTC_E_INVALID_PARAM;
            }

            if (p_linkagg_grp->tid > MCHIP_CAP(SYS_CAP_LINKAGG_RR_TID_MAX))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");
                return CTC_E_BADID;
            }

            if (CTC_LINKAGG_MODE_FLEX == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                /* duet2 rr member_num range: 1~24, TsingMa:rr member_num range: 1-255*/
                uint16 rr_max_member = DRV_IS_DUET2(lchip)? SYS_LINKAGG_MEM_NUM_1 : MCHIP_CAP(SYS_CAP_LINKAGG_RR_MAX_MEM_NUM);
                if ((0 == p_linkagg_grp->member_num) || (p_linkagg_grp->member_num > rr_max_member))
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "rr member_num invalid!\n");
                    return CTC_E_INVALID_PARAM;
                }
            }

            p_linkagg_grp->flag &= CTC_LINKAGG_GROUP_FLAG_RANDOM_RR;
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            /* mode 64 do not support dlb group for lack of member */
            if (CTC_LINKAGG_MODE_64 == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "mode 64 do not support dlb group for lack of member!\n");
                return CTC_E_INVALID_PARAM;
            }

            /* in mode 56, only the first 8 group can be dlb group */
            if (CTC_LINKAGG_MODE_56 == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                if (p_linkagg_grp->tid > MCHIP_CAP(SYS_CAP_LINKAGG_MODE56_DLB_TID_MAX))
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");
                    return CTC_E_BADID;
                }
            }

            if (CTC_LINKAGG_MODE_FLEX == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                /* duet2 dlb member_num range: 32/64/128/255 TsingMa:dlb member_num range: 16/32/64/128/255/512/1024/2048 */
                if (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) == 256)
                {
                    if ((16 != p_linkagg_grp->member_num) &&
                        (32 != p_linkagg_grp->member_num) &&
                    (64 != p_linkagg_grp->member_num) &&
                    (128 != p_linkagg_grp->member_num) &&
                    (256 != p_linkagg_grp->member_num)&&
                    (512 != p_linkagg_grp->member_num)&&
                    (1024 != p_linkagg_grp->member_num)&&
                    (2048 != p_linkagg_grp->member_num))
                    {
                        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "dlb member_num invalid!\n");
                        return CTC_E_INVALID_PARAM;
                    }
                }
                else if ( (32 != p_linkagg_grp->member_num) &&
                          (64 != p_linkagg_grp->member_num) &&
                          (128 != p_linkagg_grp->member_num) &&
                          (256 != p_linkagg_grp->member_num))
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "dlb member_num invalid!\n");
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            /* resilient only support in flex mode */
            if (CTC_LINKAGG_MODE_FLEX != p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "resilient only support in flex mode!\n");
                return CTC_E_INVALID_PARAM;
            }

            if (CTC_LINKAGG_MODE_FLEX == p_usw_linkagg_master[lchip]->linkagg_mode)
            {
                /* resilient member_num range: 32/64/128/255 */
                if (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) == 256)
                {
                    if ((16 != p_linkagg_grp->member_num) &&
                        (32 != p_linkagg_grp->member_num) &&
                    (64 != p_linkagg_grp->member_num) &&
                    (128 != p_linkagg_grp->member_num) &&
                    (256 != p_linkagg_grp->member_num)&&
                    (512 != p_linkagg_grp->member_num)&&
                    (1024 != p_linkagg_grp->member_num)&&
                    (2048 != p_linkagg_grp->member_num))
                    {
                        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "resilient member_num invalid!\n");
                        return CTC_E_INVALID_PARAM;
                    }
                }
                else
                {
                    if ((32 != p_linkagg_grp->member_num) &&
                        (64 != p_linkagg_grp->member_num) &&
                    (128 != p_linkagg_grp->member_num) &&
                    (256 != p_linkagg_grp->member_num))
                    {
                        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "resilient member_num invalid!\n");
                        return CTC_E_INVALID_PARAM;
                    }
                }
                p_linkagg_grp->flag = 0;
                break;
            }
            break;
        }
        default:
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}
#define __APIs__
/**
 @brief The function is to init the linkagg module
*/
int32
sys_usw_linkagg_init(uint8 lchip, void* linkagg_global_cfg)
{
    int32 ret = CTC_E_NONE;
    uint8  gchip = 0;
    uint16 mem_idx = 0;
    uint16 index = 0;
    uint32 core_frequecy = 0;
    uint32 cmd_linkagg = 0;
    uint32 cmd_chan_port = 0;
    uint32 cmdr_chan_port = 0;
    uint32 cmd_timer = 0;
    ctc_linkagg_global_cfg_t* p_linkagg_global_cfg = NULL;
    DsLinkAggregateMember_m ds_linkagg_member;
    LagEngineTimerCtl0_m lag_engine_timer_ctl0;
    DsPortChannelLag_m ds_linkagg_port;
    LagEngineCtl_m     lag_engine_ctl;
    uint32 cmd = 0;
    sys_usw_opf_t opf;

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(linkagg_global_cfg);

    if (NULL != p_usw_linkagg_master[lchip])
    {
        return CTC_E_NONE;
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*init soft table*/
    p_usw_linkagg_master[lchip] = (sys_linkagg_master_t*)mem_malloc(MEM_LINKAGG_MODULE, sizeof(sys_linkagg_master_t));
    if (NULL == p_usw_linkagg_master[lchip])
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    sal_memset(p_usw_linkagg_master[lchip], 0, sizeof(sys_linkagg_master_t));

    ret = sal_mutex_create(&p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    if (ret || !p_usw_linkagg_master[lchip]->p_linkagg_mutex)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
	ret = CTC_E_NO_MEMORY;
	goto error;
    }

    p_linkagg_global_cfg = (ctc_linkagg_global_cfg_t*)linkagg_global_cfg;

    switch (p_linkagg_global_cfg->linkagg_mode)
    {
        case CTC_LINKAGG_MODE_4:
            p_usw_linkagg_master[lchip]->linkagg_num = 4;
            break;

        case CTC_LINKAGG_MODE_8:
            p_usw_linkagg_master[lchip]->linkagg_num = 8;
            break;

        case CTC_LINKAGG_MODE_16:
            p_usw_linkagg_master[lchip]->linkagg_num = 16;
            break;

        case CTC_LINKAGG_MODE_32:
            p_usw_linkagg_master[lchip]->linkagg_num = 32;
            break;

        case CTC_LINKAGG_MODE_64:
        case CTC_LINKAGG_MODE_FLEX:
            p_usw_linkagg_master[lchip]->linkagg_num = 64;
            break;

        case CTC_LINKAGG_MODE_56:
            p_usw_linkagg_master[lchip]->linkagg_num = 56;
            break;

        default:
            ret = CTC_E_INVALID_PARAM;
            goto error;
    }
    if ((p_linkagg_global_cfg->linkagg_mode == CTC_LINKAGG_MODE_FLEX)&&(DRV_IS_TSINGMA(lchip)))
    {
        p_usw_linkagg_master[lchip]->linkagg_num = MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM);

    }
    p_usw_linkagg_master[lchip]->linkagg_mode = p_linkagg_global_cfg->linkagg_mode;

    p_usw_linkagg_master[lchip]->bind_gport_disable = p_linkagg_global_cfg->bind_gport_disable;

    p_usw_linkagg_master[lchip]->group_vector =  ctc_vector_init(4, 2*MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM)/4);
    if (NULL == p_usw_linkagg_master[lchip]->group_vector)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Allocate vector for group fail!\n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    ret = sys_usw_opf_init(lchip, &(p_usw_linkagg_master[lchip]->opf_type), 1, "Linkagg member opf");
    if (ret < 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "opf init fail!\n");
        goto error;
    }

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = p_usw_linkagg_master[lchip]->opf_type;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM)), ret, error);

    /* init asic table */
    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));
    sal_memset(&lag_engine_timer_ctl0, 0, sizeof(lag_engine_timer_ctl0));

    core_frequecy = sys_usw_get_core_freq(lchip, 0);

    /* init dlb flow inactive timer, Tsunit =  updateThreshold0_f * (maxPtr0_f+1) / core_frequecy = 1ms */
    SetLagEngineTimerCtl0(V, maxPhyPtr0_f, &lag_engine_timer_ctl0, (MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM) - 1));
    SetLagEngineTimerCtl0(V, maxPtr0_f, &lag_engine_timer_ctl0, SYS_DLB_MAX_PTR);
    SetLagEngineTimerCtl0(V, minPtr0_f, &lag_engine_timer_ctl0, 0);
    SetLagEngineTimerCtl0(V, updateEn0_f, &lag_engine_timer_ctl0, 1);
    SetLagEngineTimerCtl0(V, tsThreshold0_f, &lag_engine_timer_ctl0, SYS_DLB_TS_THRES);   /* ts unit is 1ms */
    SetLagEngineTimerCtl0(V, updateThreshold0_f, &lag_engine_timer_ctl0, (core_frequecy * 1000000 / 1000 / (SYS_DLB_MAX_PTR + 1)));    /* must big than 73 */

    cmd_timer = DRV_IOW(LagEngineTimerCtl0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd_timer, &lag_engine_timer_ctl0), ret, error);

    sal_memset(&lag_engine_timer_ctl0, 0, sizeof(lag_engine_timer_ctl0));
    SetLagEngineTimerCtl1(V, maxPhyPtr1_f, &lag_engine_timer_ctl0, (MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_ALL_MEM_NUM)-1));
    SetLagEngineTimerCtl1(V, maxPtr1_f, &lag_engine_timer_ctl0, SYS_DLB_MAX_PTR);
    SetLagEngineTimerCtl1(V, minPtr1_f, &lag_engine_timer_ctl0, 0);
    SetLagEngineTimerCtl1(V, updateEn1_f, &lag_engine_timer_ctl0, 1);
    SetLagEngineTimerCtl1(V, tsThreshold1_f, &lag_engine_timer_ctl0, SYS_DLB_TS_THRES);   /* ts unit is 1ms */
    SetLagEngineTimerCtl1(V, updateThreshold1_f, &lag_engine_timer_ctl0,      /* must big than 73 */
        (core_frequecy * 1000000 / 1000 / (SYS_DLB_MAX_PTR+1)));

    cmd_timer = DRV_IOW(LagEngineTimerCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd_timer, &lag_engine_timer_ctl0), ret, error);

    /* init port linkagg member info */
    sys_usw_get_gchip_id(lchip, &gchip);
    mem_idx = MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM);
    cmd_linkagg = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    SetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member, gchip);
    SetDsLinkAggregateMember(V, destChannelId_f, &ds_linkagg_member, 0);
    for (index = 0; index < mem_idx; index++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd_linkagg, &ds_linkagg_member), ret, error);
    }

    /* init channel linkagg port info */
    cmdr_chan_port = DRV_IOR(DsPortChannelLag_t, DRV_ENTRY_FLAG);
    cmd_chan_port = DRV_IOW(DsPortChannelLag_t, DRV_ENTRY_FLAG);
    for (index = 0; index < SYS_USW_MAX_PORT_NUM_PER_CHIP; index++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmdr_chan_port, &ds_linkagg_port), ret, error);
        SetDsPortChannelLag(V, linkAggregationChannelGroup_f, &ds_linkagg_port, 0);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd_chan_port, &ds_linkagg_port), ret, error);
    }

    p_usw_linkagg_master[lchip]->chanagg_grp_num = MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_GRP_MAX) - 1; /*resv for drop*/

    p_usw_linkagg_master[lchip]->chanagg_mem_per_grp = MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_PER_GRP_MEM);

    p_usw_linkagg_master[lchip]->chan_group = ctc_vector_init(8, (p_usw_linkagg_master[lchip]->chanagg_grp_num + 7) / 8);
    if (NULL == p_usw_linkagg_master[lchip]->chan_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Allocate vector for chanagg fail!\n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    /* init hw sync */
    sal_memset(&lag_engine_ctl, 0, sizeof(lag_engine_ctl));
    cmd = DRV_IOR(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl), ret, error);

    SetLagEngineCtl(V, linkChangeEn_f, &lag_engine_ctl, 1);
    SetLagEngineCtl(V, lagGroupMemoryBase_f, &lag_engine_ctl, MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
    SetLagEngineCtl(V, lbMode0SupportLinkSelfHealing0_f, &lag_engine_ctl, 1);
    SetLagEngineCtl(V, lbMode0SupportLinkSelfHealing1_f, &lag_engine_ctl, 1);
    SetLagEngineCtl(V, lbMode4SupportLinkSelfHealing0_f, &lag_engine_ctl, 1);
    SetLagEngineCtl(V, lbMode4SupportLinkSelfHealing1_f, &lag_engine_ctl, 1);

	SetLagEngineCtl(V, chanBandwidthShift0_f, &lag_engine_ctl, 9);
	SetLagEngineCtl(V, chanBandwidthShift1_f, &lag_engine_ctl, 9);
    SetLagEngineCtl(V, lagMode_f, &lag_engine_ctl, 0);

    cmd = DRV_IOW(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl), ret, error);

    /* init LagEngineDreTimerCtl_t for dlb */
    CTC_ERROR_GOTO(_sys_usw_lag_engine_dlb_init(lchip), ret, error);

    sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_CHAN_LINKDOWN_SCAN, sys_usw_linkagg_failover_isr);

    /* set chip_capability*/
    MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_GROUP_NUM) = p_usw_linkagg_master[lchip]->linkagg_num;
    MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_GROUP_NUM) = p_usw_linkagg_master[lchip]->linkagg_num;

    if (p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_FLEX)
    {
        MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_MEMBER_NUM) = 256;
        MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_FLOW_NUM) = 256;
    }
    else
    {
        MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_MEMBER_NUM) = MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM) / p_usw_linkagg_master[lchip]->linkagg_num;
        MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_FLOW_NUM) = MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM) / p_usw_linkagg_master[lchip]->linkagg_num;
    }

    /* warmboot register */
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_LINKAGG, sys_usw_linkagg_wb_sync), ret, error);

    /* dump-db register */
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_LINKAGG, sys_usw_linkagg_dump_db), ret, error);

    /* warmboot data restore */
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_GOTO(sys_usw_linkagg_wb_restore(lchip), ret, error);
    }

    return CTC_E_NONE;

error:
    if (NULL != p_usw_linkagg_master[lchip])
    {
        if (0 != p_usw_linkagg_master[lchip]->opf_type)
        {
            sys_usw_opf_deinit(lchip, p_usw_linkagg_master[lchip]->opf_type);
        }

        if (NULL != p_usw_linkagg_master[lchip]->chan_group)
        {
            /*free chanagg data*/
            ctc_vector_traverse(p_usw_linkagg_master[lchip]->chan_group, (vector_traversal_fn)_sys_usw_linkagg_free_node_data, NULL);

            ctc_vector_release(p_usw_linkagg_master[lchip]->chan_group);
        }

        if (NULL != p_usw_linkagg_master[lchip]->group_vector)
        {
            /*free linkagg*/
            ctc_vector_traverse(p_usw_linkagg_master[lchip]->group_vector, (vector_traversal_fn)_sys_usw_linkagg_free_node_data, NULL);

            ctc_vector_release(p_usw_linkagg_master[lchip]->group_vector);
        }

        if (NULL != p_usw_linkagg_master[lchip]->p_linkagg_mutex)
        {
            sal_mutex_destroy(p_usw_linkagg_master[lchip]->p_linkagg_mutex);
        }

        mem_free(p_usw_linkagg_master[lchip]);
    }
    return ret;
}

int32
sys_usw_linkagg_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL != p_usw_linkagg_master[lchip])
    {
        if (0 != p_usw_linkagg_master[lchip]->opf_type)
        {
            sys_usw_opf_deinit(lchip, p_usw_linkagg_master[lchip]->opf_type);
        }

        if (NULL != p_usw_linkagg_master[lchip]->chan_group)
        {
            /*free chanagg data*/
            ctc_vector_traverse(p_usw_linkagg_master[lchip]->chan_group, (vector_traversal_fn)_sys_usw_linkagg_free_node_data, NULL);

            ctc_vector_release(p_usw_linkagg_master[lchip]->chan_group);
        }

        if (NULL != p_usw_linkagg_master[lchip]->group_vector)
        {
            /*free linkagg*/
            ctc_vector_traverse(p_usw_linkagg_master[lchip]->group_vector, (vector_traversal_fn)_sys_usw_linkagg_free_node_data, NULL);

            ctc_vector_release(p_usw_linkagg_master[lchip]->group_vector);
        }

        if (NULL != p_usw_linkagg_master[lchip]->p_linkagg_mutex)
        {
            sal_mutex_destroy(p_usw_linkagg_master[lchip]->p_linkagg_mutex);
        }

        mem_free(p_usw_linkagg_master[lchip]);
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to set linkagg property
*/
int32
sys_usw_linkagg_set_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32 value)
{
    uint32 cmd_group = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
    DsLinkAggregateGroup_m ds_linkagg_group;
    sys_linkagg_t*  p_group = NULL;

    /* sanity check */
    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);
    if (CTC_LINKAGG_PROP_LB_HASH_OFFSET == linkagg_prop)
    {
        SYS_USW_LB_HASH_CHK_OFFSET(value);
    }
    LINKAGG_LOCK;
    /* check exist */
    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg is not exist!\n");
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    }
    if (CTC_LINKAGG_MODE_RR == p_group->mode)
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_SUPPORT, p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    }
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    switch(linkagg_prop)
    {
        case CTC_LINKAGG_PROP_LB_HASH_OFFSET:
            SetDsLinkAggregateGroup(V, hashOffset_f, &ds_linkagg_group, value % 16);
            SetDsLinkAggregateGroup(V, hashSelect_f, &ds_linkagg_group, value / 16);
            break;
        default :
            break;
    }
    cmd_group = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), p_usw_linkagg_master[lchip]->p_linkagg_mutex);

    if ((p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
         && (linkagg_prop == CTC_LINKAGG_PROP_LB_HASH_OFFSET))
    {
        cmd_group = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), cmd_group, &ds_linkagg_group), p_usw_linkagg_master[lchip]->p_linkagg_mutex);
        SetDsLinkAggregateGroup(V, hashOffset_f, &ds_linkagg_group, value % 16);
        SetDsLinkAggregateGroup(V, hashSelect_f, &ds_linkagg_group, value / 16);
        cmd_group = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), cmd_group, &ds_linkagg_group), p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    }

    LINKAGG_UNLOCK;
    return CTC_E_NONE;
}

/**
 @brief The function is to get linkagg property
*/
int32
sys_usw_linkagg_get_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32* p_value)
{
    uint8 value = 0;
    uint8 lb_hash_valid = 0;
    uint32 cmd_group = DRV_IOR(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
    DsLinkAggregateGroup_m ds_linkagg_group;
    sys_linkagg_t*  p_group = NULL;

    /* sanity check */
    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);
    CTC_PTR_VALID_CHECK(p_value);
    LINKAGG_LOCK;
    /* check exist */
    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg is not exist!\n");
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    }
    if (CTC_LINKAGG_MODE_RR == p_group->mode)
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_SUPPORT, p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    }
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), p_usw_linkagg_master[lchip]->p_linkagg_mutex);
    lb_hash_valid = GetDsLinkAggregateGroup(V, hashCfgPriorityIsHigher_f, &ds_linkagg_group);
    switch(linkagg_prop)
    {
        case CTC_LINKAGG_PROP_LB_HASH_OFFSET:
            value = GetDsLinkAggregateGroup(V, hashSelect_f, &ds_linkagg_group);
            value = (value<<4) + GetDsLinkAggregateGroup(V, hashOffset_f, &ds_linkagg_group);
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "linkagg load balance hash offset is %s!\n", lb_hash_valid? "valid": "invalid");
            break;
        default :
            break;
    }
    *p_value = value;
    LINKAGG_UNLOCK;
    return CTC_E_NONE;
}

/**
 @brief The function is to create one linkagg
*/
int32
sys_usw_linkagg_create(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp)
{
    uint16 mem_idx = 0;
    uint8  tid = 0;
    uint32 cmd_group = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
    int32  ret = CTC_E_NONE;
    uint32 mem_num = 0;
    uint32 mem_base = 0;
    DsLinkAggregateGroup_m ds_linkagg_group;
    sys_linkagg_t* p_group = NULL;

    /* sanity check */
    SYS_LINKAGG_INIT_CHECK();
    CTC_ERROR_RETURN(_sys_usw_linkagg_group_param_check(lchip, p_linkagg_grp));

    /* do create */
    LINKAGG_LOCK;
    tid = p_linkagg_grp->tid;
    /* check exist */
    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg Has Exist!\n");
        ret = CTC_E_EXIST;
        goto error;
    }

    sal_memset(&ds_linkagg_group, 0, sizeof(ds_linkagg_group));

    if (p_linkagg_grp->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        sys_linkagg_t* p_lmpf_group = NULL;

        CTC_ERROR_GOTO(_sys_usw_linkagg_add_db(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), p_linkagg_grp, &p_lmpf_group), ret, error);
        SetDsLinkAggregateGroup(V, lbMode_f, &ds_linkagg_group, 4);
        SetDsLinkAggregateGroup(V, linkAggMemBase_f, &ds_linkagg_group, p_lmpf_group->member_base);
        SetDsLinkAggregateGroup(V, hashOffset_f, &ds_linkagg_group, 0);
        SetDsLinkAggregateGroup(V, hashSelect_f, &ds_linkagg_group, 2);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), cmd_group, &ds_linkagg_group), ret, error_1);
    }

    CTC_ERROR_GOTO(_sys_usw_linkagg_add_db(lchip, tid, p_linkagg_grp, &p_group), ret, error);

    mem_base = p_group->member_base;
    mem_num = p_group->max_member_num;
    SetDsLinkAggregateGroup(V, linkAggMemBase_f, &ds_linkagg_group, mem_base);
    SetDsLinkAggregateGroup(V, hashOffset_f, &ds_linkagg_group, 0);
    SetDsLinkAggregateGroup(V, hashSelect_f, &ds_linkagg_group, 2);

    switch (p_linkagg_grp->linkagg_mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        {
            if (p_linkagg_grp->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
            {
                SetDsLinkAggregateGroup(V, lbMode_f, &ds_linkagg_group, 4);
            }
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), ret, error_1);
            break;
        }
        case CTC_LINKAGG_MODE_RR:
        {
            uint8 rr_profile_id = _sys_usw_linkagg_find_rr_profile_id(lchip, tid);
            DsLinkAggregateRrCount_m ds_linkagg_rr;
			LagRrProfileIdMappingCtl_m lag_rrprofile;
            uint32 cmd_rr = DRV_IOW(DsLinkAggregateRrCount_t, DRV_ENTRY_FLAG);
			uint32 cmd_rrf = DRV_IOW(LagRrProfileIdMappingCtl_t, DRV_ENTRY_FLAG);

            if (rr_profile_id >= 16)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] rr mod is invalid,because it greater than 15 \n");
                ret = CTC_E_NO_RESOURCE;
                goto error_1;
            }

            sal_memset(&ds_linkagg_rr, 0, sizeof(ds_linkagg_rr));
            sal_memset(&lag_rrprofile, 0, sizeof(lag_rrprofile));
            if (p_linkagg_grp->flag & CTC_LINKAGG_GROUP_FLAG_RANDOM_RR)
            {
                SetDsLinkAggregateRrCount(V, randomRrEn_f, &ds_linkagg_rr, 1);
            }
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, rr_profile_id, cmd_rr, &ds_linkagg_rr), ret, error_1);

			SetLagRrProfileIdMappingCtl(V, rrProfileId_f, &lag_rrprofile, rr_profile_id);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_rrf, &lag_rrprofile), ret, error_1);

            SetDsLinkAggregateGroup(V, lbMode_f, &ds_linkagg_group, 2);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), ret, error_1);
            CTC_BIT_SET(p_usw_linkagg_master[lchip]->rr_profile_bmp, rr_profile_id);
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            DsLinkAggregateMember_m ds_linkagg_member;
            uint32 cmd_member;
            uint8  gchip = 0;

            SetDsLinkAggregateGroup(V, linkAggFlowNum_f, &ds_linkagg_group, _sys_usw_linkagg_flows2driver(lchip, mem_num));
            SetDsLinkAggregateGroup(V, lbMode_f, &ds_linkagg_group, 1);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), ret, error_1);

            /* init chipid and flush all DsLinkAggregateMemberSet_t to reverve channel for dlb mode */
            sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));

            /* init DsLinkAggregateMember chipid */
            cmd_member = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
            sys_usw_get_gchip_id(lchip, &gchip);
            SetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member, gchip);
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, error_1);
            }
            CTC_ERROR_GOTO(_sys_usw_linkagg_reverve_member(lchip, tid), ret, error_1);
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            SetDsLinkAggregateGroup(V, linkAggFlowNum_f, &ds_linkagg_group, _sys_usw_linkagg_flows2driver(lchip, mem_num));
            SetDsLinkAggregateGroup(V, lbMode_f, &ds_linkagg_group, 3);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_group, &ds_linkagg_group), ret, error_1);
            break;
        }
        default:
            break;
    }

    LINKAGG_UNLOCK;

    sys_usw_brguc_nh_create(lchip, CTC_MAP_TID_TO_GPORT(tid), CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC);
    /*SYSTEM MODIFIED BY huwx for acl agg redirect to agg nhid bypass get*/
    sys_usw_brguc_nh_create(lchip, CTC_MAP_TID_TO_GPORT(tid), CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS);
    sys_usw_l2_set_dsmac_for_linkagg_hw_learning(lchip, CTC_MAP_TID_TO_GPORT(tid),TRUE);

    return CTC_E_NONE;

error_1:
    if (p_linkagg_grp->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        _sys_usw_linkagg_remove_db(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
    }
    _sys_usw_linkagg_remove_db(lchip, tid);
error:
    LINKAGG_UNLOCK;
    return ret;
}

/**
 @brief The function is to delete one linkagg
*/
int32
sys_usw_linkagg_destroy(uint8 lchip, uint8 tid)
{
    int32  ret = CTC_E_NONE;
    uint8  gchip = 0;
    uint16 mem_idx = 0;
    uint16 mem_num = 0;
    uint16 mem_max_num = 0;
    uint16 mem_base = 0;
    uint32 cmd = 0;
    uint32 cmd_member = 0;
    uint32 cmd_memberset = 0;
    DsLinkAggregateGroup_m ds_linkagg_group;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateMemberSet_m ds_linkagg_memberset;
    sys_linkagg_t* p_group;
    sys_linkagg_t* p_group_local_first = NULL;
    uint8  gchip_id = 0;
    uint16 destId = 0;
    uint32 gport_temp = 0;
    uint32* gports = NULL;
    uint32 cnt = 0;
    int32 local_first_flag = 0;
    uint16 index = 0;

    /*sanity check*/
    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkAggId = 0x%x\n", tid);

    sal_memset(&ds_linkagg_group, 0, sizeof(ds_linkagg_group));
    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));

    /*do remove*/
    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto out;
    }
    MALLOC_ZERO(MEM_LINKAGG_MODULE, gports,p_group->max_member_num*sizeof(uint32));
    if(NULL == gports)
    {
        ret = CTC_E_NO_MEMORY;
        goto out;
    }
    local_first_flag = 0;
    if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        p_group_local_first = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
        if (p_group_local_first)
        {
            local_first_flag = 1;
        }
        else
        {
            ret = CTC_E_NOT_EXIST;
            goto out;
        }
    }

    /*for static fail over, real member num may be changed, must reriver*/
    if(p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        p_group->real_member_num = _sys_usw_dlb_get_real_member_num(lchip, p_group->tid);
    }

    mem_base = p_group->member_base;
    mem_num = p_group->real_member_num;
    mem_max_num = p_group->max_member_num;

    if (mem_num > 0)
    {
        switch (p_group->mode)
        {
            case CTC_LINKAGG_MODE_STATIC:
            case CTC_LINKAGG_MODE_STATIC_FAILOVER:
            case CTC_LINKAGG_MODE_RR:
            {
                cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);

                for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
                {
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, out);
                    gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                    destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                    gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                    gports[cnt] = gport_temp;
                    cnt++;
                }
                break;
            }
            case CTC_LINKAGG_MODE_DLB:
            {
                cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
                cmd_memberset = DRV_IOR(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);

                for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
                {
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, out);
                    gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_memberset, &ds_linkagg_memberset), ret, out);
                    destId = GetDsLinkAggregateMemberSet(V, array_0_localPhyPort_f + (mem_idx - mem_base) * 2, &ds_linkagg_memberset);
                    gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                    gports[cnt] = gport_temp;
                    cnt++;
                }
                break;
            }
            case CTC_LINKAGG_MODE_RESILIENT:
            {
                int32 is_new;
                int32 i;

                cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);

                for (mem_idx = mem_base; mem_idx < (mem_base + mem_max_num); mem_idx++)
                {
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, out);
                    gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                    destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                    gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);

                    is_new = 1;
                    for (i = 0; i < cnt; i++)
                    {
                        if (gport_temp == gports[i])
                        {
                            is_new = 0;
                            break;
                        }
                    }

                    if (is_new)
                    {
                        gports[cnt] = gport_temp;
                        cnt++;
                        if (cnt >= mem_num)
                        {
                            break;
                        }
                    }
                }
                break;
            }
            default:
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
                ret = CTC_E_INVALID_PARAM;
                goto out;
            }
        }
    }

    if(p_group->mode != CTC_LINKAGG_MODE_RESILIENT)
    {
        for (index = 0; index < cnt; index++)
        {
            CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group, gports[index]), ret, out);

            if (!local_first_flag)
            {
                continue;
            }

            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gports[index]);
            if (TRUE == sys_usw_chip_is_local(lchip, gchip))
            {
                CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group_local_first, gports[index]), ret, out);
            }
        }
    }
    else
    {
        for (index = 0; index < cnt; index++)
        {
            /*update mcast linkagg bitmap*/
            if (TRUE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(gports[index])))
            {
                CTC_ERROR_GOTO(sys_usw_port_update_mc_linkagg(lchip, p_group->tid, CTC_MAP_GPORT_TO_LPORT(gports[index]), 0), ret, out);
                /*set local port's global_port*/
                if (!p_usw_linkagg_master[lchip]->bind_gport_disable)
                {
                    CTC_ERROR_GOTO(sys_usw_port_set_global_port(lchip, CTC_MAP_GPORT_TO_LPORT(gports[index]), gports[index], FALSE), ret, out);
                }
            }
        }
    }

    /*reset linkagg group */
    cmd = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd, &ds_linkagg_group), ret, out);
    CTC_ERROR_GOTO(sys_usw_l2_set_dsmac_for_linkagg_hw_learning(lchip, CTC_MAP_TID_TO_GPORT(tid), FALSE), ret, out);
    if(p_group->mode == CTC_LINKAGG_MODE_RR)
    {
        uint32 rr_profile_id = 0;
        cmd = DRV_IOR(LagRrProfileIdMappingCtl_t, LagRrProfileIdMappingCtl_rrProfileId_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd, &rr_profile_id), ret,  out);
        CTC_BIT_UNSET(p_usw_linkagg_master[lchip]->rr_profile_bmp, rr_profile_id);
    }
    if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        cmd = DRV_IOW(DsLinkAggregateGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), cmd, &ds_linkagg_group), ret, out);
        _sys_usw_linkagg_remove_db(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
    }

    _sys_usw_linkagg_remove_db(lchip, tid);

out:
    LINKAGG_UNLOCK;
    sys_usw_brguc_nh_delete(lchip, CTC_MAP_TID_TO_GPORT(tid));
    if(gports)
    {
        mem_free(gports);
    }
    return ret;
}

STATIC int32
_sys_usw_linkagg_check_link_stats(uint8 lchip)
{
    uint32  cmdr1 = DRV_IOR(LinkAggLinkScanInfo_t, LinkAggLinkScanInfo_linkDownScanState_f);
    uint32  cmdr2 = DRV_IOR(LinkAggState_t, LinkAggState_curLinkScanState_f);
    uint32  link_stats = 0;
    uint32  idle = 0;
    uint16  wait_cnt = 0;

    do
    {
        DRV_IOCTL(lchip, 0, cmdr1, &link_stats);
        DRV_IOCTL(lchip, 0, cmdr2, &idle);
        if (!link_stats && !idle)
        {
            break;
        }

        sal_task_sleep(1);
        wait_cnt++;

    }while(wait_cnt < 0x1000);

    DRV_IOCTL(lchip, 0, cmdr1, &link_stats);
    if(link_stats != 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Chip is processing, processing channel: 0x%x\n", link_stats);
        return 1;
    }

    DRV_IOCTL(lchip, 0, cmdr1, &idle);
    if(idle != 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Chip stats is not idle, can not add/remove port\n");
        return 1;
    }

    return 0;
}

STATIC int32
_sys_usw_linkagg_add_port(uint8 lchip, sys_linkagg_t* p_group, uint32 gport)
{
    uint8  gchip = 0;
    uint16 lport = 0;
    uint32 agg_gport = 0;
    int32 ret = CTC_E_NONE;
    sys_linkagg_mem_t linkagg_mem;
    sys_linkagg_rh_entry_t* entry_buf = NULL;
    sys_linkagg_rh_member_t* mem_buf = NULL;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&linkagg_mem, 0, sizeof(linkagg_mem));
    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);

    linkagg_mem.tid = p_group->tid;
    linkagg_mem.gport = gport;
    linkagg_mem.linkagg_mode = p_group->mode;
    linkagg_mem.mem_valid_num = p_group->real_member_num + 1;

    switch (linkagg_mem.linkagg_mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            if(CTC_FLAG_ISSET(p_group->flag, CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER))
            {
                uint16 tmp_index = p_group->real_member_num;
                DsLinkAggregateMember_m  hw_member;
                uint32 cmdr = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
                uint32 cmdw = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
                uint32 hw_gport;

                for(; tmp_index > 0; tmp_index--)
                {
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_index+p_group->member_base-1, cmdr, &hw_member));
                    hw_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(GetDsLinkAggregateMember(V, destChipId_f, &hw_member), \
                                                            GetDsLinkAggregateMember(V, destId_f, &hw_member));
                    if(gport >= hw_gport)
                    {
                        break;
                    }
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_index+p_group->member_base, cmdw, &hw_member));
                }
                linkagg_mem.port_index = tmp_index;
            }
            else
            {
                /*get the first unused pos*/
                linkagg_mem.port_index = p_group->real_member_num;
            }
            /*
             * member num 1~24:   member num in linkagg group:1~24
             * member num 25~32:  member num in linkagg group:25
             * member num 33~64:  member num in linkagg group:26
             * member num 65~128: member num in linkagg group:27
             */

            if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_1)
            {
                linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
                linkagg_mem.need_pad = 0;
            }
            else if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_2)
            {
                linkagg_mem.port_cnt = 25;
                if (linkagg_mem.mem_valid_num == 25)
                {
                    linkagg_mem.need_pad = 1;
                }
                else
                {
                    linkagg_mem.need_pad = 0;
                }
            }
            else if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_3)
            {
                linkagg_mem.port_cnt = 26;
                if (linkagg_mem.mem_valid_num == 33)
                {
                    linkagg_mem.need_pad = 1;
                }
                else
                {
                    linkagg_mem.need_pad = 0;
                }
            }
            else if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_4)
            {
                linkagg_mem.port_cnt = 27;
                if (linkagg_mem.mem_valid_num == 65)
                {
                    linkagg_mem.need_pad = 1;
                }
                else
                {
                    linkagg_mem.need_pad = 0;
                }
            }
            if (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) == 256)
            {
                linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
                linkagg_mem.need_pad = 0;
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
            linkagg_mem.port_index = p_group->real_member_num;
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            uint16 mem_num_out = 0;

            entry_buf = mem_malloc(MEM_LINKAGG_MODULE, p_group->max_member_num*sizeof(sys_linkagg_rh_entry_t));
            if(NULL == entry_buf)
            {
                ret = CTC_E_NO_MEMORY;
                goto out;
            }
            mem_buf = mem_malloc(MEM_LINKAGG_MODULE, p_group->max_member_num*sizeof(sys_linkagg_rh_member_t));
            if(NULL == mem_buf)
            {
                ret = CTC_E_NO_MEMORY;
                goto out;
            }
            sal_memset(entry_buf, 0, p_group->max_member_num*sizeof(sys_linkagg_rh_entry_t));
            sal_memset(mem_buf, 0, p_group->max_member_num*sizeof(sys_linkagg_rh_member_t));
            if(p_group->real_member_num)
            {
                _sys_usw_rh_get_info_by_hw(lchip, p_group->member_base, p_group->max_member_num, entry_buf, mem_buf, &mem_num_out);
                _sys_usw_rh_add_member_rebalance(lchip, p_group->max_member_num, (sys_linkagg_rh_entry_t*)entry_buf, mem_num_out, (sys_linkagg_rh_member_t*)mem_buf, gport);
            }
            else
            {
                uint16 entry_idx;

                for(entry_idx=0; entry_idx < p_group->max_member_num; entry_idx++)
                {
                    entry_buf[entry_idx].dest_port = gport;
                    entry_buf[entry_idx].is_changed = 1;
                }
            }
            linkagg_mem.rh_entry = &entry_buf[0];
            linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
            linkagg_mem.mem_valid_num = p_group->max_member_num;
            break;
        }
        default:
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            ret = CTC_E_INVALID_PARAM;
            goto out;
        }
    }

    /*write asic table*/
    ret = _sys_usw_linkagg_update_table(lchip, p_group, &linkagg_mem, TRUE);
    if (ret < 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg update asic table fail!\n");
        goto out;
    }

    /*for the first member in dlb mode ,need flush active */
    if ((linkagg_mem.linkagg_mode == CTC_LINKAGG_MODE_DLB) && (linkagg_mem.mem_valid_num == 1))
    {
        ret = _sys_usw_linkagg_clear_flow_active(lchip, p_group->tid);
        if (CTC_E_NONE != ret)
        {
            goto out;
        }
    }

    p_group->real_member_num++;

    if (!p_usw_linkagg_master[lchip]->bind_gport_disable)
    {
        if (TRUE == sys_usw_chip_is_local(lchip, gchip) && (p_group->tid < MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM)))
        {
            lport = CTC_MAP_GPORT_TO_LPORT(gport);
            agg_gport = CTC_MAP_TID_TO_GPORT(p_group->tid);
            sys_usw_port_set_global_port(lchip, lport, agg_gport, FALSE);
        }
    }

out:
    if(entry_buf)
    {
       mem_free(entry_buf);
    }
    if(mem_buf)
    {
       mem_free(mem_buf);
    }
    return ret;
}
/**
 @brief The function is to add a port to linkagg, do not support wrr
*/
int32
sys_usw_linkagg_add_port(uint8 lchip, uint8 tid, uint32 gport)
{
    int32 ret = 0;
    uint16  index;
    uint8  gchip = 0;
    sys_linkagg_t* p_group;
    sys_linkagg_t* p_group_local_first;
    uint32  field_val = 0;
    uint32 cmd = 0;
    uint32 mux_type = 0;
    uint32 channel_id = 0;
	uint8 is_local_chip  = 0;

    /*Sanity check*/
    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);

    SYS_GLOBAL_PORT_CHECK(gport);
    if(CTC_IS_CPU_PORT(gport) || CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[agg]Invalid gport!\n");
        return CTC_E_INVALID_PARAM;
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkAggId: 0x%x,mem_port: 0x%x\n", tid, gport);

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    SYS_GLOBAL_CHIPID_CHECK(gchip);

    if(_sys_usw_linkagg_check_link_stats(lchip))
    {
        return CTC_E_HW_BUSY;
    }

    /*do add*/
    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto out;
    }

   channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
   is_local_chip  =  sys_usw_chip_is_local(lchip, gchip);


   if ((CTC_LINKAGG_MODE_DLB == p_group->mode || CTC_LINKAGG_MODE_STATIC_FAILOVER == p_group->mode )
       &&   (!is_local_chip || channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM) ))
   {
       SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
       ret =  CTC_E_INVALID_PORT;
       goto out;
   }

   if (CTC_LINKAGG_MODE_DLB == p_group->mode)
   {
       cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
       CTC_ERROR_GOTO(DRV_IOCTL(lchip, channel_id, cmd, &mux_type), ret, out);
       if (mux_type > 0)
       {
           SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "DLB group do not support mux port!\n");
           ret =  CTC_E_INVALID_PARAM;
           goto out;
       }
   }
   else if (CTC_LINKAGG_MODE_STATIC_FAILOVER == p_group->mode)
   {
       cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
       CTC_ERROR_GOTO(DRV_IOCTL(lchip, channel_id, cmd, &mux_type), ret, out);
       if (mux_type > 0)
       {
           SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Static Failover group do not support mux port!\n");
           ret =  CTC_E_INVALID_PARAM;
           goto out;
       }

   }

    /*for static fail over, real member num may be changed, must reriver*/
    if(p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        uint32 cmd_r = DRV_IOR(DsLinkAggregateGroup_t, DsLinkAggregateGroup_linkAggMemNum_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_group->tid, cmd_r, &field_val), ret, out);
        p_group->real_member_num = field_val;
    }

    /*check if the port is member of linkagg*/
    if (TRUE == _sys_usw_linkagg_port_is_member(lchip, p_group, gport, &index))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The port is exist in linkagg group, add member port fail!\n");
        ret = CTC_E_EXIST;
        goto out;
    }

    if (p_group->real_member_num >= p_group->max_member_num)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
        ret = CTC_E_INVALID_PARAM;
        goto out;
    }

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        {
            break;
        }
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        {
            if (p_group->real_member_num >= SYS_LINKAGG_MEM_NUM_1)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
                ret = CTC_E_INVALID_PARAM;
                goto out;
            }
            break;
        }
        case CTC_LINKAGG_MODE_RR:
        {
            if (p_group->real_member_num >= MCHIP_CAP(SYS_CAP_LINKAGG_RR_MAX_MEM_NUM))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
                ret = CTC_E_INVALID_PARAM;
                goto out;
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            if (p_group->real_member_num >= MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
                ret = CTC_E_INVALID_PARAM;
                goto out;
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            break;
        }
        default:
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            ret = CTC_E_INVALID_PARAM;
            goto out;
        }
    }

    CTC_ERROR_GOTO(_sys_usw_linkagg_add_port(lchip, p_group, gport), ret, out);

    /*local member first*/
    if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        if((p_group->mode != CTC_LINKAGG_MODE_STATIC) && (p_group->mode != CTC_LINKAGG_MODE_STATIC_FAILOVER))
        {
            ret = CTC_E_NOT_SUPPORT;
            goto out;
        }

        p_group_local_first = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid+MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
        if(!p_group_local_first)
        {
            ret = CTC_E_NOT_EXIST;
            goto out;
        }

        if (TRUE == sys_usw_chip_is_local(lchip, gchip))
        {
            CTC_ERROR_GOTO(_sys_usw_linkagg_add_port(lchip, p_group_local_first, gport), ret, out);
        }
    }

out:
    LINKAGG_UNLOCK;
    return ret;
}



/**
 @brief The function is to remove the port from linkagg
*/
STATIC int32
_sys_usw_linkagg_remove_port(uint8 lchip, sys_linkagg_t* p_group, uint32 gport)
{
    int32   ret = CTC_E_NONE;
    uint8   gchip = 0;
    uint16  lport = 0;
    uint16  index = 0;
    sys_linkagg_mem_t linkagg_mem;
    sys_linkagg_rh_entry_t* entry_buf = NULL;
    sys_linkagg_rh_member_t* mem_buf = NULL;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&linkagg_mem, 0, sizeof(linkagg_mem));
    sal_memset(&mem_buf, 0, sizeof(mem_buf));

    /*check if port is a member of linkagg*/
    if (FALSE == _sys_usw_linkagg_port_is_member(lchip, p_group, gport, &index))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The port is not exist in linkagg group, remove fail!\n");
        return CTC_E_NOT_EXIST;
    }

    linkagg_mem.tid = p_group->tid;
    linkagg_mem.gport = gport;
    linkagg_mem.linkagg_mode = p_group->mode;
    linkagg_mem.mem_valid_num = p_group->real_member_num - 1;
    linkagg_mem.port_index = index;

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);

    switch (linkagg_mem.linkagg_mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_1)
            {
                linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
            }
            else if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_2)
            {
                linkagg_mem.port_cnt = 25;
            }
            else if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_3)
            {
                linkagg_mem.port_cnt = 26;
            }
            else if (linkagg_mem.mem_valid_num <= SYS_LINKAGG_MEM_NUM_4)
            {
                linkagg_mem.port_cnt = 27;
            }
            else
            {
                /* keep port_cnt not change */
            }

            if (linkagg_mem.port_cnt > 24)
            {
                linkagg_mem.need_pad = 1;
            }
            else
            {
                linkagg_mem.need_pad = 0;
            }
			if(MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) == 256)
			{
                linkagg_mem.need_pad = 0;
				linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            uint16 mem_num_out = 0;
            entry_buf = mem_malloc(MEM_LINKAGG_MODULE, p_group->max_member_num*sizeof(sys_linkagg_rh_entry_t));
            if(NULL == entry_buf)
            {
                ret = CTC_E_NO_MEMORY;
                goto OUT;
            }
            mem_buf = mem_malloc(MEM_LINKAGG_MODULE, p_group->max_member_num*sizeof(sys_linkagg_rh_member_t));
            if(NULL == mem_buf)
            {
                ret = CTC_E_NO_MEMORY;
                goto OUT;
            }
            sal_memset(entry_buf, 0, p_group->max_member_num*sizeof(sys_linkagg_rh_entry_t));
            sal_memset(mem_buf, 0, p_group->max_member_num*sizeof(sys_linkagg_rh_member_t));
            if(p_group->real_member_num > 1)
            {
                _sys_usw_rh_get_info_by_hw(lchip, p_group->member_base, p_group->max_member_num, entry_buf, mem_buf, &mem_num_out);
                _sys_usw_rh_remove_member_rebalance(lchip, p_group->max_member_num, entry_buf, mem_num_out, mem_buf, gport);
            }
            linkagg_mem.rh_entry = &entry_buf[0];
            linkagg_mem.port_cnt = linkagg_mem.mem_valid_num;
            linkagg_mem.mem_valid_num = p_group->max_member_num;
            break;
        }
        default:
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            ret = CTC_E_INVALID_PARAM;
            goto OUT;
        }
    }

    /*write asic table*/
    ret = _sys_usw_linkagg_update_table(lchip, p_group, &linkagg_mem, FALSE);
    if (ret < 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg update asic table fail!\n");
        goto OUT;
    }

    p_group->real_member_num--;

    /*set local port's global_port*/
    if (!p_usw_linkagg_master[lchip]->bind_gport_disable)
    {
        if (TRUE == sys_usw_chip_is_local(lchip, gchip) && p_group->tid < MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
        {
            /* check whether is the last one */
            if (FALSE == _sys_usw_linkagg_port_is_member(lchip, p_group, gport, &index))
            {
                lport = CTC_MAP_GPORT_TO_LPORT(gport);
                sys_usw_port_set_global_port(lchip, lport, gport, FALSE);
            }
        }
    }

OUT:
    if(entry_buf)
    {
       mem_free(entry_buf);
    }
    if(mem_buf)
    {
       mem_free(mem_buf);
    }
    return ret;
}

int32
sys_usw_linkagg_remove_port(uint8 lchip, uint8 tid, uint32 gport)
{
    int32 ret = CTC_E_NONE;
    uint8 gchip = 0;
    sys_linkagg_t* p_group;
    sys_linkagg_t* p_group_local_first;

    /*Sanity check*/
    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);

    SYS_GLOBAL_PORT_CHECK(gport);
    if (CTC_IS_CPU_PORT(gport) || CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[linkagg]Invalid gport!\n");
        return CTC_E_INVALID_PARAM;
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkAggId: 0x%x,mem_port: 0x%x\n", tid, gport);

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    SYS_GLOBAL_CHIPID_CHECK(gchip);

    if (_sys_usw_linkagg_check_link_stats(lchip))
    {
        return CTC_E_HW_BUSY;
    }

    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if (!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    /*for static fail over, real member num may be changed, must reriver*/
    if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        p_group->real_member_num = _sys_usw_dlb_get_real_member_num(lchip, p_group->tid);
    }

    if (0 == p_group->real_member_num)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member num of linkagg is zero, remove fail!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group, gport), ret, OUT);

    /*local member first*/
    if(p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        if((p_group->mode != CTC_LINKAGG_MODE_STATIC) && (p_group->mode != CTC_LINKAGG_MODE_STATIC_FAILOVER))
        {
            ret = CTC_E_NOT_SUPPORT;
            goto OUT;
        }

        p_group_local_first = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid+MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
        if(!p_group_local_first)
        {
            ret = CTC_E_NOT_EXIST;
            goto OUT;
        }

        if (TRUE == sys_usw_chip_is_local(lchip, gchip))
        {
            CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group_local_first, gport), ret, OUT);
        }
    }

OUT:
    LINKAGG_UNLOCK;
    return ret;
}

/**
 @brief The function is to get the a local member port of linkagg
*/
int32
sys_usw_linkagg_get_1st_local_port(uint8 lchip, uint8 tid, uint32* p_gport, uint16* local_cnt)
{
    uint16 mem_idx = 0;
    uint16 mem_num = 0;
    uint16 mem_max_num = 0;
    uint16 mem_base = 0;
    uint32 cmd_member = 0;
    uint32 cmd_memberset = 0;
    uint8  gchip_id = 0;
    uint16 destId = 0;
    uint32 gport_temp = 0;
    int32  ret = 0;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateMemberSet_m ds_linkagg_memberset;
    sys_linkagg_t* p_group = NULL;
    uint32* p_gport_temp = NULL;
    /*Sanity check*/
    SYS_LINKAGG_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_gport);
    CTC_PTR_VALID_CHECK(local_cnt);
    SYS_TID_VALID_CHECK(tid);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));
    sal_memset(&ds_linkagg_memberset, 0, sizeof(ds_linkagg_memberset));

    /*do it*/
    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if (!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Entry is not exist \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    *local_cnt = 0;

    /*for static fail over, real member num may be changed, must reriver*/
    if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        p_group->real_member_num = _sys_usw_dlb_get_real_member_num(lchip, p_group->tid);
    }

    if (0 == p_group->real_member_num)
    {
        goto error;
    }

    mem_base = p_group->member_base;
    mem_num = p_group->real_member_num;
    mem_max_num = p_group->max_member_num;

    cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    cmd_memberset = DRV_IOR(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, error);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);

                if (TRUE == sys_usw_chip_is_local(lchip, gchip_id))
                {
                    destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                    gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                    *p_gport = gport_temp;
                    (*local_cnt)++;
                }
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, error);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);

                if (TRUE == sys_usw_chip_is_local(lchip, gchip_id))
                {
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_memberset, &ds_linkagg_memberset), ret, error);
                    destId = GetDsLinkAggregateMemberSet(V, array_0_localPhyPort_f + (mem_idx - mem_base) * 2, &ds_linkagg_memberset);
                    gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                    *p_gport = gport_temp;
                    (*local_cnt)++;
                }
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            int32 is_new;
            int32 i;
            uint16 cnt = 0;

            if(NULL == (p_gport_temp = (uint32*)mem_malloc(MEM_LINKAGG_MODULE, mem_max_num*sizeof(uint32))))
            {
                ret = CTC_E_NO_MEMORY;
                goto error;
            }

            for (mem_idx = mem_base; mem_idx < (mem_base + mem_max_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, error);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);

                if (TRUE == sys_usw_chip_is_local(lchip, gchip_id))
                {
                    destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                    gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);

                    is_new = 1;
                    for (i = 0; i < cnt; i++)
                    {
                        if (gport_temp == p_gport_temp[i])
                        {
                            is_new = 0;
                            break;
                        }
                    }

                    if (is_new)
                    {
                        p_gport_temp[cnt] = gport_temp;
                        cnt++;

                        if (cnt >= mem_num)
                        {
                            break;
                        }
                    }
                }
            }

            if (cnt > 0)
            {
                *p_gport = p_gport_temp[cnt - 1];
                *local_cnt = cnt;
            }

            break;
        }
        default:
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            ret = CTC_E_INVALID_PARAM;
            goto error;
        }
    }

    if (*local_cnt > 0)
    {
        ret = CTC_E_NONE;
    }
    else
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local phyport is not exist \n");
        ret = CTC_E_NOT_EXIST;
    }

error:
    if(p_gport_temp)
    {
        mem_free(p_gport_temp);
    }
    LINKAGG_UNLOCK;
    return ret;
}

/**
 @brief The function is to show member ports of linkagg.
*/
int32
sys_usw_linkagg_show_ports(uint8 lchip, uint8 tid, uint32* p_gports, uint16* cnt)
{
    uint16 mem_idx = 0;
    uint16 mem_num = 0;
    uint16 mem_max_num = 0;
    uint16 mem_base = 0;
    uint32 cmd_member = 0;
    uint32 cmd_memberset = 0;
    uint8  gchip_id = 0;
    uint16 destId = 0;
    uint32 gport_temp = 0;
    int32  ret = 0;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateMemberSet_m ds_linkagg_memberset;
    sys_linkagg_t* p_group = NULL;

    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);
    CTC_PTR_VALID_CHECK(p_gports);
    CTC_PTR_VALID_CHECK(cnt);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));
    sal_memset(&ds_linkagg_memberset, 0, sizeof(ds_linkagg_memberset));

    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Entry is not exist \n");
        ret = CTC_E_NOT_EXIST;
        goto out;
    }

    /*for static fail over, real member num may be changed, must reriver*/
    if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        p_group->real_member_num = _sys_usw_dlb_get_real_member_num(lchip, p_group->tid);
    }

    if (0 == p_group->real_member_num)
    {
        /*SYSTEM MODIFIED by kcao for get LAG member */
        *cnt = 0;
        goto out;
    }

    mem_num = p_group->real_member_num;
    mem_max_num = p_group->max_member_num;
    mem_base = p_group->member_base;

    cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    cmd_memberset = DRV_IOR(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, out);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                p_gports[(*cnt)++] = gport_temp;
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, out);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_memberset, &ds_linkagg_memberset), ret, out);
                destId = GetDsLinkAggregateMemberSet(V, array_0_localPhyPort_f + (mem_idx - mem_base) * 2, &ds_linkagg_memberset);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                p_gports[(*cnt)++] = gport_temp;
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            int32 is_new;
            int32 i;

            for (mem_idx = mem_base; mem_idx < (mem_base + mem_max_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, out);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);

                is_new = 1;
                for (i = 0; i < (*cnt); i++)
                {
                    if (gport_temp == p_gports[i])
                    {
                        is_new = 0;
                        break;
                    }
                }

                if (is_new)
                {
                    p_gports[*cnt] = gport_temp;
                    (*cnt)++;
                    if ((*cnt) >= mem_num)
                    {
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            ret = CTC_E_INVALID_PARAM;
            goto out;
        }
    }

out:
    LINKAGG_UNLOCK;
    return ret;
}

/**
 @brief The function is to show member ports of linkagg.
*/
int32
sys_usw_linkagg_show_all_member(uint8 lchip, uint8 tid)
{
    uint16 idx = 0;
    uint16 mem_num = 0;
    uint16 mem_max_num = 0;
    int32  ret = 0;
    sys_linkagg_t* p_group = NULL;

    char* mode[5] = {"static", "static-failover", "round-robin", "dynamic", "resilient"};
    char flag_str[64];
    int32 usedlen;
    uint32* p_gports = NULL;
    uint16 cnt  = 0;

    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if (!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Entry is not exist \n");
        LINKAGG_UNLOCK;
        return CTC_E_NOT_EXIST;
    }

    /*for static fail over, real member num may be changed, must reriver*/
    if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        p_group->real_member_num = _sys_usw_dlb_get_real_member_num(lchip, p_group->tid);
    }

    mem_num = p_group->real_member_num;
    mem_max_num = p_group->max_member_num;

    flag_str[0] = '\0';
    if (0 == (p_group->flag & (CTC_LINKAGG_GROUP_FLAG_RANDOM_RR | CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)))
    {
        sal_sprintf(flag_str, "%s", "- ");
    }
    else
    {
        usedlen = 0;

        if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_RANDOM_RR)
        {
            usedlen += sal_sprintf(flag_str + usedlen, "random ");
        }

        if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
        {
            usedlen += sal_sprintf(flag_str + usedlen, "lmpf ");
        }
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s: %u\n", "tid", tid);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s: %s\n", "mode", mode[p_group->mode]);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s: %s\n", "flag", flag_str);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s: %u\n", "count", mem_num);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s%s\n", "No.", "Member-Port");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------\n");
    LINKAGG_UNLOCK;

    p_gports = (uint32*)mem_malloc(MEM_LINKAGG_MODULE, sizeof(uint32)*mem_max_num);
    if (NULL == p_gports)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_gports, 0, sizeof(uint32)*mem_max_num);

    ret = sys_usw_linkagg_show_ports(lchip, tid, p_gports, &cnt);
    if (CTC_E_NONE != ret)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg show ports fail!\n");
        mem_free(p_gports);
        return ret;
    }

    for (idx = 0; idx < cnt; idx++)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11u0x%04X\n", idx + 1, p_gports[idx]);
    }
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------\n");

    mem_free(p_gports);
    return CTC_E_NONE;
}

/**
 @brief The function is to set member of linkagg hash key
*/
int32
sys_usw_linkagg_set_psc(uint8 lchip, ctc_linkagg_psc_t* p_psc)
{
    uint32 value = 0;
    ctc_parser_linkagg_hash_ctl_t hash_ctl;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LINKAGG_INIT_CHECK();

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_linkagg_hash_ctl_t));

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_INNER))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_INNER);
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L2))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2);

        value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_VLAN);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_VID : 0));

        value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_COS);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_COS : 0));
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_CTAG_COS : 0));

        value = CTC_FLAG_ISSET(p_psc->l2_flag ,CTC_LINKAGG_PSC_L2_ETHERTYPE);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE : 0));

        value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_DOUBLE_VLAN);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_VID : 0));
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_CTAG_VID : 0));

        value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_MACSA);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACSA : 0));

        value = CTC_FLAG_ISSET(p_psc->l2_flag,  CTC_LINKAGG_PSC_L2_MACDA);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACDA : 0));

        value = CTC_FLAG_ISSET(p_psc->l2_flag,  CTC_LINKAGG_PSC_L2_PORT);
        CTC_SET_FLAG(hash_ctl.l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_PORT : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_IP))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP);

        value = CTC_FLAG_ISSET(p_psc->ip_flag, CTC_LINKAGG_PSC_IP_PROTOCOL);
        CTC_SET_FLAG(hash_ctl.ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_PROTOCOL : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag, CTC_LINKAGG_PSC_IP_IPSA);
        CTC_SET_FLAG(hash_ctl.ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_IPSA : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag ,CTC_LINKAGG_PSC_IP_IPDA);
        CTC_SET_FLAG(hash_ctl.ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_IPDA : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag ,CTC_LINKAGG_PSC_IP_DSCP);
        CTC_SET_FLAG(hash_ctl.ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_DSCP : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag ,CTC_LINKAGG_PSC_IPV6_FLOW_LABEL);
        CTC_SET_FLAG(hash_ctl.ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag ,CTC_LINKAGG_PSC_IP_ECN);
        CTC_SET_FLAG(hash_ctl.ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_ECN : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L4))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4);

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_SRC_PORT);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_SRC_PORT : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_DST_PORT);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_DST_PORT : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_GRE_KEY);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_GRE_KEY : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_TYPE);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_TYPE : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_USER_TYPE);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_USER_TYPE : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_VXLAN_L4_SRC_PORT);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_TCP_FLAG);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_TCP_FLAG : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_TCP_ECN);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_TCP_ECN : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_NVGRE_VSID);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_NVGRE_FLOW_ID);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_FLOW_ID : 0));

        value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_LINKAGG_PSC_L4_VXLAN_VNI);
        CTC_SET_FLAG(hash_ctl.l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_PBB))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_PBB);

        value = CTC_FLAG_ISSET(p_psc->pbb_flag, CTC_LINKAGG_PSC_PBB_BMACDA);
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_CMAC_DA : 0));

        value = CTC_FLAG_ISSET(p_psc->pbb_flag, CTC_LINKAGG_PSC_PBB_BMACSA);
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_CMAC_SA : 0));

        value = CTC_FLAG_ISSET(p_psc->pbb_flag, CTC_LINKAGG_PSC_PBB_ISID);
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_ISID : 0));

        value = CTC_FLAG_ISSET(p_psc->pbb_flag, CTC_LINKAGG_PSC_PBB_BVLAN);
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_STAG_VID : 0));
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_CTAG_VID : 0));

        value = CTC_FLAG_ISSET(p_psc->pbb_flag, CTC_LINKAGG_PSC_PBB_BCOS);
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_STAG_COS : 0));
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_CTAG_COS : 0));

        value = CTC_FLAG_ISSET(p_psc->pbb_flag, CTC_LINKAGG_PSC_PBB_BCFI);
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_STAG_CFI : 0));
        CTC_SET_FLAG(hash_ctl.pbb_flag, (value ? CTC_PARSER_PBB_HASH_FLAGS_CTAG_CFI : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_MPLS))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_MPLS);

        value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_LINKAGG_PSC_MPLS_PROTOCOL);
        CTC_SET_FLAG(hash_ctl.mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL : 0));

        value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_LINKAGG_PSC_MPLS_IPSA);
        CTC_SET_FLAG(hash_ctl.mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_IPSA : 0));

        value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_LINKAGG_PSC_MPLS_IPDA);
        CTC_SET_FLAG(hash_ctl.mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_IPDA : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_TRILL))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_TRILL);

        value = CTC_FLAG_ISSET(p_psc->trill_flag, CTC_LINKAGG_PSC_TRILL_INNER_VID);
        CTC_SET_FLAG(hash_ctl.trill_flag, (value ? CTC_PARSER_TRILL_HASH_FLAGS_INNER_VLAN_ID : 0));

        value = CTC_FLAG_ISSET(p_psc->trill_flag, CTC_LINKAGG_PSC_TRILL_INGRESS_NICKNAME);
        CTC_SET_FLAG(hash_ctl.trill_flag, (value ? CTC_PARSER_TRILL_HASH_FLAGS_INGRESS_NICKNAME : 0));

        value = CTC_FLAG_ISSET(p_psc->trill_flag, CTC_LINKAGG_PSC_TRILL_EGRESS_NICKNAME);
        CTC_SET_FLAG(hash_ctl.trill_flag, (value ? CTC_PARSER_TRILL_HASH_FLAGS_EGRESS_NICKNAME : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_FCOE))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_FCOE);

        value = CTC_FLAG_ISSET(p_psc->fcoe_flag, CTC_LINKAGG_PSC_FCOE_SID);
        CTC_SET_FLAG(hash_ctl.fcoe_flag, (value ? CTC_PARSER_FCOE_HASH_FLAGS_SID : 0));

        value = CTC_FLAG_ISSET(p_psc->fcoe_flag, CTC_LINKAGG_PSC_FCOE_DID);
        CTC_SET_FLAG(hash_ctl.fcoe_flag, (value ? CTC_PARSER_FCOE_HASH_FLAGS_DID : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_COMMON))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_COMMON);

        value = CTC_FLAG_ISSET(p_psc->common_flag, CTC_LINKAGG_PSC_COMMON_DEVICEINFO);
        CTC_SET_FLAG(hash_ctl.common_flag, (value ? CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_UDF))
    {
        CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_UDF);
        hash_ctl.udf_bitmap = p_psc->udf_bitmap;
        hash_ctl.udf_id = p_psc->udf_id;
    }

    CTC_ERROR_RETURN(sys_usw_parser_set_linkagg_hash_field(lchip, &hash_ctl));

    return CTC_E_NONE;
}

/**
 @brief The function is to get member of linkagg hash key
*/
int32
sys_usw_linkagg_get_psc(uint8 lchip, ctc_linkagg_psc_t* p_psc)
{
    uint32 value1 = 0;
    uint32 value2 = 0;
    ctc_parser_linkagg_hash_ctl_t hash_ctl;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LINKAGG_INIT_CHECK();

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_linkagg_hash_ctl_t));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_INNER);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_INNER : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L2);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_L2 : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_IP);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_IP : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L4);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_L4 : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_PBB);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_PBB : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_MPLS);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_MPLS : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_TRILL);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_TRILL : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_FCOE);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_FCOE : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_COMMON);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_COMMON : 0));

    value1 = CTC_FLAG_ISSET(p_psc->psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_UDF);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, (value1 ? CTC_PARSER_HASH_TYPE_FLAGS_UDF : 0));

    hash_ctl.udf_id = p_psc->udf_id;

    CTC_ERROR_RETURN(sys_usw_parser_get_linkagg_hash_field(lchip, &hash_ctl));

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID);
        value2 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID);

        if (value1 && value2)
        {
            CTC_SET_FLAG(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_DOUBLE_VLAN);
        }
        else if (value1 || value2)
        {
            CTC_SET_FLAG(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_VLAN);
        }

        value1 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_COS);
        value2 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_COS);

        if (value1 || value2)
        {
            CTC_SET_FLAG(p_psc->l2_flag, CTC_LINKAGG_PSC_L2_COS);
        }

        value1 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE);
        CTC_SET_FLAG(p_psc->l2_flag, (value1 ? CTC_LINKAGG_PSC_L2_ETHERTYPE : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACSA );
        CTC_SET_FLAG(p_psc->l2_flag, value1 ? CTC_LINKAGG_PSC_L2_MACSA : 0);

        value1 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACDA );
        CTC_SET_FLAG(p_psc->l2_flag, value1 ? CTC_LINKAGG_PSC_L2_MACDA : 0);

        value1 = CTC_FLAG_ISSET(hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_PORT);
        CTC_SET_FLAG(p_psc->l2_flag, value1 ? CTC_LINKAGG_PSC_L2_PORT: 0);
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_PROTOCOL);
        CTC_SET_FLAG(p_psc->ip_flag, (value1 ? CTC_LINKAGG_PSC_IP_PROTOCOL : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPSA);
        CTC_SET_FLAG(p_psc->ip_flag, (value1 ? CTC_LINKAGG_PSC_IP_IPSA  : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPDA);
        CTC_SET_FLAG(p_psc->ip_flag, (value1 ? CTC_LINKAGG_PSC_IP_IPDA : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_DSCP);
        CTC_SET_FLAG(p_psc->ip_flag, (value1 ? CTC_LINKAGG_PSC_IP_DSCP  : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL);
        CTC_SET_FLAG(p_psc->ip_flag, (value1 ? CTC_LINKAGG_PSC_IPV6_FLOW_LABEL  : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_ECN);
        CTC_SET_FLAG(p_psc->ip_flag, (value1 ? CTC_LINKAGG_PSC_IP_ECN  : 0));
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_SRC_PORT);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_SRC_PORT : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_DST_PORT);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_DST_PORT : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_GRE_KEY);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_GRE_KEY : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_TYPE);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_TYPE : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_USER_TYPE);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_USER_TYPE : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_VXLAN_L4_SRC_PORT : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_TCP_FLAG);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_TCP_FLAG : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_TCP_ECN);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_TCP_ECN : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_NVGRE_VSID : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_FLOW_ID);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_NVGRE_FLOW_ID : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI);
        CTC_SET_FLAG(p_psc->l4_flag, (value1 ? CTC_LINKAGG_PSC_L4_VXLAN_VNI : 0));
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_PBB))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CMAC_DA);
        CTC_SET_FLAG(p_psc->pbb_flag, (value1 ? CTC_LINKAGG_PSC_PBB_BMACDA : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CMAC_SA);
        CTC_SET_FLAG(p_psc->pbb_flag, (value1 ? CTC_LINKAGG_PSC_PBB_BMACSA : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_ISID);
        CTC_SET_FLAG(p_psc->pbb_flag, (value1 ? CTC_LINKAGG_PSC_PBB_ISID : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_STAG_VID);
        value2 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CTAG_VID);
        CTC_SET_FLAG(p_psc->pbb_flag, ((value1 && value2) ? CTC_LINKAGG_PSC_PBB_BVLAN : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_STAG_COS);
        value2 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CTAG_COS);
        CTC_SET_FLAG(p_psc->pbb_flag, ((value1 && value2) ? CTC_LINKAGG_PSC_PBB_BCOS : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_STAG_CFI);
        value2 = CTC_FLAG_ISSET(hash_ctl.pbb_flag, CTC_PARSER_PBB_HASH_FLAGS_CTAG_CFI);
        CTC_SET_FLAG(p_psc->pbb_flag, ((value1 && value2) ? CTC_LINKAGG_PSC_PBB_BCFI : 0));
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_MPLS))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL);
        CTC_SET_FLAG(p_psc->mpls_flag, (value1 ? CTC_LINKAGG_PSC_MPLS_PROTOCOL : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPSA);
        CTC_SET_FLAG(p_psc->mpls_flag, (value1 ? CTC_LINKAGG_PSC_MPLS_IPSA : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPDA);
        CTC_SET_FLAG(p_psc->mpls_flag, (value1 ? CTC_LINKAGG_PSC_MPLS_IPDA : 0));
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_TRILL))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_INNER_VLAN_ID);
        CTC_SET_FLAG(p_psc->trill_flag, (value1 ? CTC_LINKAGG_PSC_TRILL_INNER_VID : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_INGRESS_NICKNAME);
        CTC_SET_FLAG(p_psc->trill_flag, (value1 ? CTC_LINKAGG_PSC_TRILL_INGRESS_NICKNAME : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_EGRESS_NICKNAME);
        CTC_SET_FLAG(p_psc->trill_flag, (value1 ? CTC_LINKAGG_PSC_TRILL_EGRESS_NICKNAME : 0));
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_FCOE))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.fcoe_flag, CTC_PARSER_FCOE_HASH_FLAGS_SID);
        CTC_SET_FLAG(p_psc->fcoe_flag, (value1 ? CTC_LINKAGG_PSC_FCOE_SID : 0));

        value1 = CTC_FLAG_ISSET(hash_ctl.fcoe_flag, CTC_PARSER_FCOE_HASH_FLAGS_DID);
        CTC_SET_FLAG(p_psc->fcoe_flag, (value1 ? CTC_LINKAGG_PSC_FCOE_DID : 0));
    }

    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_COMMON))
    {
        value1 = CTC_FLAG_ISSET(hash_ctl.common_flag, CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO);
        CTC_SET_FLAG(p_psc->common_flag, (value1 ? CTC_LINKAGG_PSC_COMMON_DEVICEINFO : 0));
    }
    if (CTC_FLAG_ISSET(hash_ctl.hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_UDF))
    {
        p_psc->udf_bitmap = hash_ctl.udf_bitmap;
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get member of linkagg, only useful in nor flex mode
*/
int32
sys_usw_linkagg_get_max_mem_num(uint8 lchip, uint16* max_num)
{
    uint16 mem_num = 0;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LINKAGG_INIT_CHECK();
    CTC_PTR_VALID_CHECK(max_num);

    switch (p_usw_linkagg_master[lchip]->linkagg_mode)
    {
        case CTC_LINKAGG_MODE_FLEX:
        {
            mem_num = 2048;
            break;
        }
        case CTC_LINKAGG_MODE_56:
        {
            mem_num = 32;
            break;
        }
        case CTC_LINKAGG_MODE_4:
        case CTC_LINKAGG_MODE_8:
        case CTC_LINKAGG_MODE_16:
        case CTC_LINKAGG_MODE_32:
        case CTC_LINKAGG_MODE_64:
        {
            mem_num = 1024/ p_usw_linkagg_master[lchip]->linkagg_num;
            break;
        }
        default:
        {
            break;
        }
    }

    *max_num = mem_num;

    return CTC_E_NONE;
}

/** replace all members for linkagg group, the api is used for sorting the ports and supporting wrr */
int32
sys_usw_linkagg_replace_ports(uint8 lchip, uint8 tid, uint32* gport, uint16 mem_num)
{
    uint16 index                = 0;
    uint16 index1               = 0;
    uint16 max_mem_num          = 0;
    int32  ret                  = 0;
    uint32 cmd                  = 0;
    uint32 mux_type             = 0;
    uint8  gchip                = 0;
    sys_linkagg_port_t* p_port  = NULL;
    uint16  lport               = 0;
    sys_linkagg_t* p_group = NULL;
    sys_linkagg_t* p_group_local_first = NULL;
    uint16 mem_idx = 0;
    uint16 mem_num_offset = 0;
    uint16 mem_base = 0;
    uint32 cmd_member = 0;
    uint32 cmd_memberset = 0;
    uint8  gchip_id = 0;
    uint16 destId = 0;
    uint32 gport_temp = 0;
    uint16 cnt = 0;
    int32 is_repeat;
    int32 local_first_flag = 0;
    DsLinkAggregateMember_m ds_linkagg_member;
    DsLinkAggregateMemberSet_m ds_linkagg_memberset;
    uint32 channel_id = 0;
    sys_linkagg_rh_entry_t* entry_buf = NULL;
    sys_linkagg_rh_member_t* mem_buf = NULL;

    SYS_LINKAGG_INIT_CHECK();
    CTC_PTR_VALID_CHECK(gport);
    SYS_TID_VALID_CHECK(tid);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid);
    if (NULL == p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    /*get max member number of per group*/
    max_mem_num = p_group->max_member_num;

    if (mem_num > max_mem_num)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
        ret = CTC_E_INVALID_PARAM;
        goto OUT;
    }

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        {
            break;
        }
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        {
            if (mem_num > SYS_LINKAGG_MEM_NUM_1)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
                ret = CTC_E_INVALID_PARAM;
                goto OUT;
            }
            break;
        }
        case CTC_LINKAGG_MODE_RR:
        {
            if (mem_num > MCHIP_CAP(SYS_CAP_LINKAGG_RR_MAX_MEM_NUM))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
                ret = CTC_E_INVALID_PARAM;
                goto OUT;
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            if (mem_num > MCHIP_CAP(SYS_CAP_DLB_MEMBER_NUM))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member of linkagg group reach Max, add member port fail!\n");
                ret = CTC_E_INVALID_PARAM;
                goto OUT;
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            break;
        }
        default:
        {
            ret = CTC_E_INVALID_PARAM;
            goto OUT;
        }
    }

    /* static and rr group support wrr and do not check repeat */
    if ((CTC_LINKAGG_MODE_STATIC_FAILOVER == p_group->mode) ||
        (CTC_LINKAGG_MODE_DLB == p_group->mode) ||
        (CTC_LINKAGG_MODE_RESILIENT == p_group->mode))
    {
        /* check repeat */
        if (mem_num > 0)
        {
            is_repeat = 0;
            for (index = 0; index < (mem_num - 1); index++)
            {
                for (index1 = (index + 1); index1 < mem_num; index1++)
                {
                    if (gport[index] == gport[index1])
                    {
                        is_repeat = 1;
                        break;
                    }
                }

                if (is_repeat)
                {
                    break;
                }
            }

            if (is_repeat)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Repeat port , replace member port fail!\n");
                ret = CTC_E_EXIST;
                goto OUT;
            }
        }
    }

    /*check*/
    if (CTC_LINKAGG_MODE_DLB == p_group->mode)
    {
        cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);

        for (index = 0; index < mem_num; index++)
        {
            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport[index]);
            if (FALSE == sys_usw_chip_is_local(lchip, gchip))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "DLB group do not support remote port!\n");
                ret =  CTC_E_INVALID_PORT;
                goto OUT;
            }

            if ((CTC_MAP_GPORT_TO_LPORT(gport[index]) & 0xFF) >= SYS_INTERNAL_PORT_START)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "DLB group do not support internal port!\n");
                ret =  CTC_E_INVALID_PARAM;
                goto OUT;
            }
            channel_id = SYS_GET_CHANNEL_ID(lchip, (uint16)gport[index]);
            if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
                ret =  CTC_E_INVALID_PORT;
                goto OUT;
            }

            CTC_ERROR_GOTO(DRV_IOCTL(lchip, channel_id, cmd, &mux_type), ret, OUT);
            if (mux_type > 0)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "DLB group do not support mux port!\n");
                ret =  CTC_E_INVALID_PARAM;
                goto OUT;
            }
        }
    }
    else if (CTC_LINKAGG_MODE_STATIC_FAILOVER == p_group->mode)
    {
        cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);

        for (index = 0; index < mem_num; index++)
        {
            if ((CTC_MAP_GPORT_TO_LPORT(gport[index]) & 0xFF) >= SYS_INTERNAL_PORT_START)
            {
                SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Static Failover group do not support internal port!\n");
                ret =  CTC_E_INVALID_PARAM;
                goto OUT;
            }

            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport[index]);
            if (TRUE == sys_usw_chip_is_local(lchip, gchip))
            {
                channel_id = SYS_GET_CHANNEL_ID(lchip, (uint16)gport[index]);
                if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
                    ret =  CTC_E_INVALID_PORT;
                    goto OUT;
                }
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, channel_id, cmd, &mux_type), ret, OUT);
                if (mux_type > 0)
                {
                    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Static Failover group do not support mux port!\n");
                    ret =  CTC_E_INVALID_PARAM;
                    goto OUT;
                }
            }
        }
    }

    /*p_port save the previous info of p_linkagg */
    p_port = (sys_linkagg_port_t*)mem_malloc(MEM_LINKAGG_MODULE, max_mem_num * sizeof(sys_linkagg_port_t));
    if (p_port == NULL)
    {
        ret = CTC_E_NO_MEMORY;
        goto OUT;
    }

    sal_memset(p_port, 0, max_mem_num * sizeof(sys_linkagg_port_t));

    /*for static fail over, real member num may be changed, must reriver*/
    if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
    {
        p_group->real_member_num = _sys_usw_dlb_get_real_member_num(lchip, p_group->tid);
    }

    mem_num_offset = p_group->real_member_num;
    mem_base = p_group->member_base;

    cmd_member = DRV_IOR(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
    cmd_memberset = DRV_IOR(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num_offset); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, OUT);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                p_port[cnt].gport = gport_temp;
                p_port[cnt].valid = 1;
                cnt++;
            }
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            for (mem_idx = mem_base; mem_idx < (mem_base + mem_num_offset); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, OUT);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, tid, cmd_memberset, &ds_linkagg_memberset), ret, OUT);
                destId = GetDsLinkAggregateMemberSet(V, array_0_localPhyPort_f+ (mem_idx - mem_base)*2, &ds_linkagg_memberset);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);
                p_port[cnt].gport = gport_temp;
                p_port[cnt].valid = 1;
                cnt++;
            }
            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            int32 is_new;
            int32 i;

            if (0 == mem_num_offset)
            {
                break;
            }

            for (mem_idx = mem_base; mem_idx < (mem_base + max_mem_num); mem_idx++)
            {
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, mem_idx, cmd_member, &ds_linkagg_member), ret, OUT);
                gchip_id = GetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member);
                destId = GetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member);
                gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip_id, destId);

                is_new = 1;
                for (i = 0; i < cnt; i++)
                {
                    if (gport_temp == p_port[i].gport)
                    {
                        is_new = 0;
                        break;
                    }
                }

                if (is_new)
                {
                    p_port[cnt].gport = gport_temp;
                    p_port[cnt].valid = 1;
                    cnt++;
                    if (cnt >= mem_num_offset)
                    {
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "linkagg group mode error!\n");
            ret = CTC_E_INVALID_PARAM;
            goto OUT;
        }
    }

    for (index1 = 0; index1 < cnt; index1++)
    {
        for (index = 0; index < mem_num; index++)
        {
            if (p_port[index1].gport == gport[index])
            {
                p_port[index1].valid = 0;
                break;
            }
        }
    }

    local_first_flag = 0;
    if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
    {
        p_group_local_first = ctc_vector_get(p_usw_linkagg_master[lchip]->group_vector, tid+MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM));
        if (p_group_local_first)
        {
            local_first_flag = 1;
        }
        else
        {
            ret = CTC_E_NOT_EXIST;
            goto OUT;
        }
    }

    if(CTC_LINKAGG_MODE_RESILIENT == p_group->mode)
    {
        uint32 value = 0;
        entry_buf = mem_malloc(MEM_LINKAGG_MODULE, p_group->max_member_num*sizeof(sys_linkagg_rh_entry_t));
        if(NULL == entry_buf)
        {
            ret = CTC_E_NO_MEMORY;
            goto OUT;
        }
        mem_buf = mem_malloc(MEM_LINKAGG_MODULE, p_group->max_member_num*sizeof(sys_linkagg_rh_member_t));
        if(NULL == mem_buf)
        {
            ret = CTC_E_NO_MEMORY;
            goto OUT;
        }

        sal_memset(entry_buf, 0, p_group->max_member_num*sizeof(sys_linkagg_rh_entry_t));
        sal_memset(mem_buf, 0, p_group->max_member_num*sizeof(sys_linkagg_rh_member_t));
        for (index = 0; index < cnt; index++)
        {
            /*update mcast linkagg bitmap*/
            if (TRUE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(p_port[index].gport)))
            {
                CTC_ERROR_GOTO(sys_usw_port_update_mc_linkagg(lchip, p_group->tid, CTC_MAP_GPORT_TO_LPORT(p_port[index].gport), 0), ret, OUT);
                /*set local port's global_port*/
                if (!p_usw_linkagg_master[lchip]->bind_gport_disable)
                {
                    CTC_ERROR_GOTO(sys_usw_port_set_global_port(lchip, CTC_MAP_GPORT_TO_LPORT(p_port[index].gport), p_port[index].gport, FALSE), ret, OUT);
                }
            }
        }

        if(mem_num)
        {
            for(index=0; index < mem_num; index++)
            {
                /*update mcast linkagg bitmap*/
                if (TRUE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(gport[index])))
                {
                    CTC_ERROR_GOTO(sys_usw_port_update_mc_linkagg(lchip, p_group->tid, CTC_MAP_GPORT_TO_LPORT(gport[index]), 1), ret, OUT);
                    /*set local port's global_port*/
                    if (!p_usw_linkagg_master[lchip]->bind_gport_disable)
                    {
                        CTC_ERROR_GOTO(sys_usw_port_set_global_port(lchip, CTC_MAP_GPORT_TO_LPORT(gport[index]), gport[index], FALSE), ret, OUT);
                    }
                }
                mem_buf[index].dest_port = gport[index];
            }
            CTC_ERROR_GOTO(_sys_usw_rh_reset_all_member(lchip, p_group->max_member_num, entry_buf, mem_num, mem_buf), ret, OUT);
        }

        /*write hw*/
        cmd = DRV_IOW(DsLinkAggregateMember_t, DRV_ENTRY_FLAG);
        for(index = 0; index < p_group->max_member_num; index++)
        {
            sal_memset(&ds_linkagg_member, 0, sizeof(ds_linkagg_member));
            SetDsLinkAggregateMember(V, destId_f, &ds_linkagg_member, SYS_MAP_CTC_GPORT_TO_DRV_LPORT(entry_buf[index].dest_port));
            SetDsLinkAggregateMember(V, destChipId_f, &ds_linkagg_member, SYS_MAP_CTC_GPORT_TO_GCHIP(entry_buf[index].dest_port));
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, (p_group->member_base+index), cmd, &ds_linkagg_member), ret, OUT);
        }
        value = mem_num;
        cmd = DRV_IOW(DsLinkAggregateGroup_t, DsLinkAggregateGroup_linkAggMemNum_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_group->tid, cmd, &value), ret, OUT);
        p_group->real_member_num = mem_num;
    }
    else
    {
        /*if mem_num == 0, clear all members of the linkagg group*/
        if((!mem_num) && (cnt))
        {
            for (index = 0; index < cnt; index++)
            {
                CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group, p_port[index].gport), ret, OUT);

                if (!local_first_flag)
                {
                    continue;
                }

                gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_port[index].gport);
                if (TRUE == sys_usw_chip_is_local(lchip, gchip))
                {
                    CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group_local_first, p_port[index].gport), ret, OUT);
                }
            }
        }
        else
        {
            uint16 mem_count = 0;
            uint16 local_mem_count = 0;

            p_group->real_member_num = 0;

            /*do replace*/
            for(index = 0; index < mem_num; index++)
            {
                CTC_ERROR_GOTO(_sys_usw_linkagg_add_port(lchip, p_group, gport[index]), ret, OUT);
                mem_count++;

                if (!local_first_flag)
                {
                    continue;
                }

                gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport[index]);
                if (TRUE == sys_usw_chip_is_local(lchip, gchip))
                {
                    local_mem_count++;
                    p_group_local_first->real_member_num = ((local_mem_count==1) ? 0: p_group_local_first->real_member_num);
                    CTC_ERROR_GOTO(_sys_usw_linkagg_add_port(lchip, p_group_local_first, gport[index]), ret, OUT);
                }
            }

            if (local_first_flag && (!local_mem_count))
            {
                for (index = 0; index < cnt; index++)
                {
                    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_port[index].gport);
                    if (TRUE == sys_usw_chip_is_local(lchip, gchip))
                    {
                        CTC_ERROR_GOTO(_sys_usw_linkagg_remove_port(lchip, p_group_local_first, p_port[index].gport), ret, OUT);
                    }
                }
            }

            /*clear the different gport info*/
            for (index = 0; index < cnt; index++)
            {
                gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_port[index].gport);
                lport = CTC_MAP_GPORT_TO_LPORT(p_port[index].gport);
                if (index >= mem_count)
                {
                    if (p_group->mode == CTC_LINKAGG_MODE_DLB)
                    {
                        _sys_usw_dlb_del_member_without_copy(lchip, tid, index, lport);
                    }
                    if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
                    {
                        _sys_usw_hw_sync_del_member(lchip, tid, lport);
                    }
                }

                if (local_first_flag)
                {
                    if (index >= local_mem_count)
                    {
                        if (p_group->mode == CTC_LINKAGG_MODE_DLB)
                        {
                            _sys_usw_dlb_del_member_without_copy(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), index, lport);
                        }
                        if (p_group->mode == CTC_LINKAGG_MODE_STATIC_FAILOVER)
                        {
                            _sys_usw_hw_sync_del_member(lchip, tid + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM), lport);
                        }
                    }
                }

                if (TRUE == sys_usw_chip_is_local(lchip, gchip))
                {
                    if (!p_port[index].valid) /*  p_port[index].valid = 1 , indicate p_port[index1].gport different with all new gports*/
                    {
                        continue;
                    }
                    sys_usw_port_set_global_port(lchip, lport, p_port[index].gport, FALSE);
                    sys_usw_port_update_mc_linkagg(lchip, tid, lport, FALSE);
                }
            }
        }
    }
    mem_free(p_port);
    if(entry_buf)
    {
        mem_free(entry_buf);
    }
    if(mem_buf)
    {
        mem_free(mem_buf);
    }
    LINKAGG_UNLOCK;
    return CTC_E_NONE;

OUT:
    if(entry_buf)
    {
        mem_free(entry_buf);
    }

    if(mem_buf)
    {
        mem_free(mem_buf);
    }

    if (NULL != p_port)
    {
        mem_free(p_port);
    }
    LINKAGG_UNLOCK;
    return ret;
}

#define __CHANNEL_AGG__
#if 0
int32
sys_usw_linkagg_set_channel_agg_ref_cnt(uint8 lchip, uint8 tid, bool is_add)
{
    int ret = CTC_E_NONE;
    sys_linkagg_t* p_chanagg = NULL;

    SYS_LINKAGG_INIT_CHECK();

    if ((tid >= p_usw_linkagg_master[lchip]->chanagg_grp_num) || (0 == tid))
    {
        return CTC_E_INVALID_PARAM;
    }

    LINKAGG_LOCK;

    p_chanagg = ctc_vector_get(p_usw_linkagg_master[lchip]->chan_group, tid);
    if (NULL == p_chanagg)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    if (is_add)
    {
        p_chanagg->ref_cnt++;
    }
    else
    {
        p_chanagg->ref_cnt = p_chanagg->ref_cnt? (p_chanagg->ref_cnt - 1) : 0;
    }

OUT:
    LINKAGG_UNLOCK;
    return ret;
}

int32
sys_usw_linkagg_get_channel_agg_ref_cnt(uint8 lchip, uint8 tid, uint8* ref_cnt)
{
    int ret = CTC_E_NONE;
    sys_linkagg_t* p_chanagg = NULL;

    SYS_LINKAGG_INIT_CHECK();

    if ((tid >= p_usw_linkagg_master[lchip]->chanagg_grp_num) || (0 == tid))
    {
        return CTC_E_INVALID_PARAM;
    }

    LINKAGG_LOCK;

    p_chanagg = ctc_vector_get(p_usw_linkagg_master[lchip]->chan_group, tid);
    if (NULL == p_chanagg)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    *ref_cnt = p_chanagg->ref_cnt;

OUT:
    LINKAGG_UNLOCK;
    return ret;
}
#endif
STATIC int32
_sys_usw_linkagg_update_channel_table(uint8 lchip, sys_linkagg_mem_t* p_chankagg, bool is_add_port, uint16 port_index)
{
    uint32 cmd = 0;
    uint32 mem_base = 0;
    DsLinkAggregateChannelMember_m ds_chanagg_member;
    DsLinkAggregateChannelGroup_m ds_chanagg_group;
    uint16 tail_index = 0;
    uint8  chan_id = 0;
    uint16 port_cnt=0;

    CTC_PTR_VALID_CHECK(p_chankagg);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_chanagg_member, 0, sizeof(DsLinkAggregateChannelMember_m));
    sal_memset(&ds_chanagg_group, 0, sizeof(DsLinkAggregateChannelGroup_m));

    mem_base = p_chankagg->tid * p_usw_linkagg_master[lchip]->chanagg_mem_per_grp;
    tail_index = p_chankagg->port_cnt;
    port_cnt = p_chankagg->port_cnt;

    if (TRUE == is_add_port)
    {
        /*update member */
        /*get channel from port */
        chan_id = SYS_GET_CHANNEL_ID(lchip, p_chankagg->gport);

        if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
        {
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
            return CTC_E_INVALID_PORT;
        }

        SetDsLinkAggregateChannelMember(V, channelId_f, &ds_chanagg_member, chan_id);
        cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mem_base + port_index), cmd, &ds_chanagg_member));

        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Write DsLinkAggregateChannelMember %d\n", (mem_base + port_index));

        /* update group member cnt */
        cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chankagg->tid, cmd, &ds_chanagg_group));
        SetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_chanagg_group, port_cnt);
        cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chankagg->tid, cmd, &ds_chanagg_group));
    }
    else
    {
        /* update group member cnt */
        cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chankagg->tid, cmd, &ds_chanagg_group));
        SetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_chanagg_group, port_cnt);
        cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chankagg->tid, cmd, &ds_chanagg_group));

        if (0 == p_chankagg->port_cnt)
        {
            cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip,  (mem_base + port_index), cmd, &ds_chanagg_member));
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Clear DsLinkAggregateChannelMember %d\n",(mem_base + port_index));
        }
        else
        {
            /*copy the last one to the removed port position,and remove member port from linkagg at tail*/
            cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip,  (mem_base + tail_index), cmd, &ds_chanagg_member));
            cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip,  (mem_base + port_index), cmd, &ds_chanagg_member));
            SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Copy DsLinkAggregateChannelMember from %d to %d\n",  (mem_base + tail_index), (mem_base + port_index));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_linkagg_create_channel_agg(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp)
{
    uint32 cmd = 0;
    int32  ret = CTC_E_NONE;
    uint16 mem_num = 0;
    uint16 mem_base = 0;
    DsLinkAggregateChannelGroup_m ds_chanagg_group;
    sys_linkagg_t* p_chan_group = NULL;

    /* sanity check */
    SYS_LINKAGG_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_linkagg_grp);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ChanAggId = 0x%x\n", p_linkagg_grp->tid);

    if ((p_linkagg_grp->tid >= p_usw_linkagg_master[lchip]->chanagg_grp_num) || (0 == p_linkagg_grp->tid))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");
	return CTC_E_BADID;
    }

    /* do create */
    LINKAGG_LOCK;

    /* check exist */
    p_chan_group = ctc_vector_get(p_usw_linkagg_master[lchip]->chan_group, p_linkagg_grp->tid);
    if (p_chan_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg Has Exist!\n");
        ret = CTC_E_EXIST;
        goto error_0;
    }

    /* init linkAgg group info */
    p_chan_group = mem_malloc(MEM_LINKAGG_MODULE, sizeof(sys_linkagg_t));
    if(!p_chan_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No memory!\n");
        ret = CTC_E_NO_MEMORY;
        goto error_0;
    }

    mem_num = p_usw_linkagg_master[lchip]->chanagg_mem_per_grp;
    mem_base = p_linkagg_grp->tid * mem_num;

    sal_memset(p_chan_group, 0, sizeof(sys_linkagg_t));
    p_chan_group->tid = p_linkagg_grp->tid;
    p_chan_group->mode = CTC_LINKAGG_MODE_STATIC;
    p_chan_group->member_base = mem_base;
    p_chan_group->max_member_num = mem_num;

    CTC_ERROR_GOTO(ctc_vector_add(p_usw_linkagg_master[lchip]->chan_group, p_chan_group->tid, p_chan_group), ret, error_1);

    sal_memset(&ds_chanagg_group, 0, sizeof(ds_chanagg_group));
    SetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_chanagg_group, mem_base);
    /*not support if needed
    if (CTC_FLAG_ISSET(p_linkagg_grp->flag, CTC_LINKAGG_GROUP_FLAG_LB_HASH_OFFSET_VALID))
    {
        SetDsLinkAggregateChannelGroup(V, hashCfgPriorityIsHigher_f, &ds_chanagg_group, 1);
        SetDsLinkAggregateChannelGroup(V, hashOffset_f, &ds_chanagg_group, (p_linkagg_grp->lb_hash_offset % 16));
        SetDsLinkAggregateChannelGroup(V, hashSelect_f, &ds_chanagg_group, (p_linkagg_grp->lb_hash_offset / 16));
    }
    */
    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    if ((ret = DRV_IOCTL(lchip, p_linkagg_grp->tid, cmd, &ds_chanagg_group)) < 0)
    {
        SYS_LINKAGG_GOTO_ERROR(ret, error_2);
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Groupid = %d, mem_base = %d!\n", p_linkagg_grp->tid, mem_base);

    LINKAGG_UNLOCK;

    return CTC_E_NONE;

error_2:
   ctc_vector_del(p_usw_linkagg_master[lchip]->chan_group, p_chan_group->tid);
error_1:
    mem_free(p_chan_group);
error_0:
    LINKAGG_UNLOCK;
    return ret;
}

int32
sys_usw_linkagg_destroy_channel_agg(uint8 lchip, uint8 tid)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    DsLinkAggregateChannelGroup_m ds_chanagg_group;
    sys_linkagg_t* p_group = NULL;

    /*sanity check*/
    SYS_LINKAGG_INIT_CHECK();

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ChanAggId = 0x%x\n", tid);

    if ((tid >= p_usw_linkagg_master[lchip]->chanagg_grp_num) || (0 == tid))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");
        return CTC_E_BADID;
    }

    /*do remove*/
    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->chan_group, tid);
    if(!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Linkagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    /*reset linkagg group */
    sal_memset(&ds_chanagg_group, 0, sizeof(ds_chanagg_group));

    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    if ((ret = DRV_IOCTL(lchip, tid, cmd, &ds_chanagg_group)) < 0)
    {
        ret = CTC_E_HW_FAIL;
        goto OUT;
    }

    ctc_vector_del(p_usw_linkagg_master[lchip]->chan_group, tid);

    mem_free(p_group);

OUT:
    LINKAGG_UNLOCK;
    return ret;
}

int32
sys_usw_linkagg_add_channel(uint8 lchip, uint8 tid, uint32 gport)
{
    uint16 index = 0;
    uint8 gchip = 0;
    int32 ret = CTC_E_NONE;
    sys_linkagg_mem_t p_chanagg;
    uint16 mem_num = 0;
    uint16 chanagg_base=0;
    uint16 chan_Id = 0;
    uint32 gport_temp = 0;
    uint32 cmd=0;
    uint32 cmd_grp =0;
    uint32 cmd_member=0;
    sys_linkagg_t* p_group = NULL;
    DsLinkAggregateChannelGroup_m ds_chanagg_group;
    DsLinkAggregateChannelMember_m ds_chanagg_member;
    DsPortChannelLag_m ds_chanagg_grp;

    /*Sanity check*/
    SYS_LINKAGG_INIT_CHECK();
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ChanAggId: 0x%x,mem_port: 0x%x\n", tid, gport);

    if ((tid >= p_usw_linkagg_master[lchip]->chanagg_grp_num) || (0 == tid))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");
	return CTC_E_BADID;
    }

    SYS_GLOBAL_PORT_CHECK(gport);
    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    SYS_GLOBAL_CHIPID_CHECK(gchip);

    /*do add*/
    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->chan_group, tid);
    if (!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    /*check exist*/
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    cmd_member = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);

    sal_memset(&ds_chanagg_group, 0, sizeof(ds_chanagg_group));
    ret = DRV_IOCTL(lchip, tid, cmd, &ds_chanagg_group);
    if (ret < 0)
    {
        goto OUT;
    }

    chanagg_base=GetDsLinkAggregateChannelGroup(V,channelLinkAggMemBase_f,&ds_chanagg_group);
    mem_num=GetDsLinkAggregateChannelGroup(V,channelLinkAggMemNum_f,&ds_chanagg_group);
    sal_memset(&ds_chanagg_member, 0, sizeof(ds_chanagg_member));
    for (index = chanagg_base; index < (mem_num + chanagg_base); index++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd_member, &ds_chanagg_member), ret, OUT);
        chan_Id = GetDsLinkAggregateChannelMember(V, channelId_f, &ds_chanagg_member);
        gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip, chan_Id);
        if (gport_temp == gport)
        {
            ret = CTC_E_EXIST;
            goto OUT;
        }
    }

    /*get the first unused pos*/
    index = p_group->real_member_num;

    sal_memset(&p_chanagg, 0, sizeof(p_chanagg));
    p_chanagg.tid = tid;
    p_chanagg.gport = gport;
    p_chanagg.mem_valid_num = p_group->real_member_num;
    if ((p_chanagg.mem_valid_num >= MCHIP_CAP(SYS_CAP_LINKAGG_CHAN_PER_GRP_MEM)))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg mem valid num error!\n");
        ret = CTC_E_INVALID_PARAM;
        goto OUT;
    }

    p_chanagg.mem_valid_num++;
    p_chanagg.port_cnt = p_chanagg.mem_valid_num;

    sal_memset(&ds_chanagg_grp, 0, sizeof(ds_chanagg_grp));
    SetDsPortChannelLag(V, linkAggregationChannelGroup_f, &ds_chanagg_grp, tid);
    cmd_grp =DRV_IOW(DsPortChannelLag_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, p_chanagg.gport, cmd_grp, &ds_chanagg_grp);
    if (ret < 0)
    {
        goto OUT;
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index = %d, port_cnt = %d\n", index,  p_chanagg.port_cnt);

    /*write asic table*/
    ret = _sys_usw_linkagg_update_channel_table(lchip, &p_chanagg, TRUE, index);
    if (ret < 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg update asic table fail!\n");
        goto OUT;
    }

    p_group->real_member_num++;

OUT:
    LINKAGG_UNLOCK;
    return ret;
}

int32
sys_usw_linkagg_remove_channel(uint8 lchip, uint8 tid, uint32 gport)
{
    int32 ret = CTC_E_NONE;
    uint16 index = 0;
    uint8 gchip = 0;
    uint16 mem_num = 0;
    uint16 chanagg_base=0;
    uint16 chan_Id = 0;
    uint32 gport_temp = 0;
    uint32 cmd=0;
    uint32 cmd_grp = 0;
    uint32 cmd_member=0;
    sys_linkagg_mem_t p_chanagg;
    sys_linkagg_t* p_group = NULL;
    DsLinkAggregateChannelGroup_m ds_chanagg_group;
    DsLinkAggregateChannelMember_m ds_chanagg_member;
    DsPortChannelLag_m ds_chanagg_grp;

    /*Sanity check*/
    SYS_LINKAGG_INIT_CHECK();
    SYS_TID_VALID_CHECK(tid);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "linkAggId: 0x%x,mem_port: 0x%x\n", tid, gport);

    if ((tid >= p_usw_linkagg_master[lchip]->chanagg_grp_num) || (0 == tid))
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Linkagg] Tid is invalid \n");
	return CTC_E_BADID;
    }

    SYS_GLOBAL_PORT_CHECK(gport);
    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    SYS_GLOBAL_CHIPID_CHECK(gchip);

    LINKAGG_LOCK;

    p_group = ctc_vector_get(p_usw_linkagg_master[lchip]->chan_group, tid);
    if (!p_group)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg Not Exist!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    /*check if port is a member of linkagg*/
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    cmd_member = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);

    sal_memset(&ds_chanagg_group, 0, sizeof(ds_chanagg_group));
    ret = DRV_IOCTL(lchip, tid, cmd, &ds_chanagg_group);
    if (ret < 0)
    {
        goto OUT;
    }

    chanagg_base=GetDsLinkAggregateChannelGroup(V,channelLinkAggMemBase_f,&ds_chanagg_group);
    mem_num=GetDsLinkAggregateChannelGroup(V,channelLinkAggMemNum_f,&ds_chanagg_group);
    sal_memset(&ds_chanagg_member, 0, sizeof(ds_chanagg_member));
    for (index = chanagg_base; index < (mem_num + chanagg_base); index++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd_member, &ds_chanagg_member), ret, OUT);
        chan_Id = GetDsLinkAggregateChannelMember(V, channelId_f, &ds_chanagg_member);
        gport_temp = CTC_MAP_LPORT_TO_GPORT(gchip, chan_Id);
        if(gport_temp == gport)
        {
            index = index - chanagg_base;
            break;
        }
    }

    if (index >= (mem_num + chanagg_base))  /* not find the member*/
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The port is not exist in chanagg group, remove fail!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }

    sal_memset(&p_chanagg, 0, sizeof(p_chanagg));
    p_chanagg.tid = tid;
    p_chanagg.mem_valid_num = p_group->real_member_num;
    if (0 == p_chanagg.mem_valid_num)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "The member num of linkagg is zero, remove fail!\n");
        ret = CTC_E_NOT_EXIST;
        goto OUT;
    }
    p_chanagg.mem_valid_num--;
    p_chanagg.port_cnt = p_chanagg.mem_valid_num;

    cmd_grp = DRV_IOW(DsPortChannelLag_t, DRV_ENTRY_FLAG);
    sal_memset(&ds_chanagg_grp, 0, sizeof(ds_chanagg_grp));
    ret = DRV_IOCTL(lchip, gport, cmd_grp, &ds_chanagg_grp);
    if (ret < 0)
    {
        ret = CTC_E_HW_FAIL;
        goto OUT;
    }

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index = %d, port_cnt = %d\n", index,  p_chanagg.port_cnt);

    /*write asic table*/
    ret = _sys_usw_linkagg_update_channel_table(lchip, &p_chanagg, FALSE, index);
    if (ret < 0)
    {
        SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Chanagg update asic table fail!\n");
        goto OUT;
    }

    p_group->real_member_num--;

OUT:
    LINKAGG_UNLOCK;
    return ret;
}

STATIC int32
_sys_usw_linkagg_show_node_data(void* node_data, void* user_data)
{
    sys_linkagg_t *p_group = (sys_linkagg_t *)node_data;
    sys_linkagg_stats_t *stats_data = (sys_linkagg_stats_t *)user_data;
    uint8 lchip = 0;

    if ((NULL == p_group) || (NULL == stats_data))
    {
        return CTC_E_NONE;
    }

    lchip = stats_data->lchip;

    switch (p_group->mode)
    {
        case CTC_LINKAGG_MODE_STATIC:
        {
            if (p_group->tid < MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
            {
                stats_data->static_group_cnt++;
                if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
                {
                    stats_data->static_group_lmpf_cnt++;
                }
                stats_data->total_group_cnt++;
            }

            stats_data->static_member_cnt += p_group->max_member_num;
            stats_data->total_member_cnt += p_group->max_member_num;

            break;
        }
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        {
            if (p_group->tid < MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
            {
                stats_data->failover_group_cnt++;
                if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST)
                {
                    stats_data->failover_group_lmpf_cnt++;
                }
                stats_data->total_group_cnt++;
            }

            stats_data->failover_member_cnt += p_group->max_member_num;
            stats_data->total_member_cnt += p_group->max_member_num;

            break;
        }
        case CTC_LINKAGG_MODE_RR:
        {
            if (p_group->tid >= MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
            {
                break;
            }

            stats_data->rr_group_cnt++;
            if (p_group->flag & CTC_LINKAGG_GROUP_FLAG_RANDOM_RR)
            {
                stats_data->rr_group_random_cnt++;
            }
            stats_data->total_group_cnt++;

            stats_data->rr_member_cnt += p_group->max_member_num;
            stats_data->total_member_cnt += p_group->max_member_num;

            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        {
            if (p_group->tid >= MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
            {
                break;
            }

            stats_data->dynamic_group_cnt++;
            stats_data->total_group_cnt++;

            stats_data->dynamic_member_cnt += p_group->max_member_num;
            stats_data->total_member_cnt += p_group->max_member_num;

            break;
        }
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            if (p_group->tid >= MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM))
            {
                break;
            }

            stats_data->resilient_group_cnt++;
            stats_data->total_group_cnt++;

            stats_data->resilient_member_cnt += p_group->max_member_num;
            stats_data->total_member_cnt += p_group->max_member_num;

            break;
        }
        default:
        {
            break;
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_linkagg_status_show(uint8 lchip)
{
    sys_linkagg_stats_t stats_data;

    SYS_LINKAGG_INIT_CHECK();

    LINKAGG_LOCK;

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Work Mode--------------------------------\n");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--bind_gport_disable", p_usw_linkagg_master[lchip]->bind_gport_disable);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %s\n", "--linkagg_mode", _sys_usw_linkagg_mode2str(p_usw_linkagg_master[lchip]->linkagg_mode));
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--linkagg_num", p_usw_linkagg_master[lchip]->linkagg_num);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--chanagg_grp_num", p_usw_linkagg_master[lchip]->chanagg_grp_num);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--chanagg_mem_per_grp", p_usw_linkagg_master[lchip]->chanagg_mem_per_grp);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    sal_memset(&stats_data, 0, sizeof(stats_data));
    stats_data.lchip = lchip;
    ctc_vector_traverse(p_usw_linkagg_master[lchip]->group_vector, (vector_traversal_fn)_sys_usw_linkagg_show_node_data, &stats_data);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Group Resource Usage---------------------------\n");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "ALL Group Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Total count", p_usw_linkagg_master[lchip]->linkagg_num);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.total_group_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Free count", p_usw_linkagg_master[lchip]->linkagg_num - stats_data.total_group_cnt);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Static Group Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (All)", stats_data.static_group_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (Lmpf)", stats_data.static_group_lmpf_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Failover Group Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (All)", stats_data.failover_group_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (Lmpf)", stats_data.failover_group_lmpf_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "RR Group Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (All)", stats_data.rr_group_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (Random)", stats_data.rr_group_random_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Dynamic Group Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (All)", stats_data.dynamic_group_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Resilient Group Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count (All)", stats_data.resilient_group_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Member Resource Usage---------------------------\n");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "ALL Member Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Total count", MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM));
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.total_member_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Free count", MCHIP_CAP(SYS_CAP_LINKAGG_ALL_MEM_NUM) - stats_data.total_member_cnt);

    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Static Member Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.static_member_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Failover Member Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.failover_member_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "RR Member Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.rr_member_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Dynamic Member Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.dynamic_member_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s\n", "Resilient Member Count");
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-24s: %u\n", "--Used count", stats_data.resilient_member_cnt);
    SYS_LINKAGG_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    LINKAGG_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_usw_linkagg_wb_mapping_group(uint8 lchip, sys_wb_linkagg_group_t* p_wb_linkagg, sys_linkagg_t* p_linkagg, uint8 sync, uint8 is_chan_agg)
{
    if (sync)
    {
        p_wb_linkagg->tid = p_linkagg->tid;
        p_wb_linkagg->flag = p_linkagg->flag;
        p_wb_linkagg->mode = p_linkagg->mode;
        p_wb_linkagg->ref_cnt = p_linkagg->ref_cnt;
        p_wb_linkagg->max_member_num = p_linkagg->max_member_num;
        p_wb_linkagg->real_member_num = p_linkagg->real_member_num;
    }
    else
    {
        uint32 cmd = is_chan_agg ? DRV_IOR(DsLinkAggregateChannelGroup_t, DsLinkAggregateChannelGroup_channelLinkAggMemBase_f)
                    : DRV_IOR(DsLinkAggregateGroup_t, DsLinkAggregateGroup_linkAggMemBase_f);

        uint32 mem_base = 0;
        sys_usw_opf_t opf;

        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = p_usw_linkagg_master[lchip]->opf_type;

        DRV_IOCTL(lchip, p_wb_linkagg->tid, cmd, &mem_base);

        p_linkagg->tid	 = p_wb_linkagg->tid;
        p_linkagg->flag = p_wb_linkagg->flag;
        p_linkagg->mode = p_wb_linkagg->mode;
        p_linkagg->max_member_num = p_wb_linkagg->max_member_num;
        p_linkagg->ref_cnt	= p_wb_linkagg->ref_cnt;
        p_linkagg->real_member_num = p_wb_linkagg->real_member_num;
        p_linkagg->member_base = mem_base;
        if (is_chan_agg)
        {
            return CTC_E_NONE;
        }
        if(p_usw_linkagg_master[lchip]->linkagg_mode == CTC_LINKAGG_MODE_FLEX)
        {
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, p_linkagg->max_member_num, p_linkagg->member_base))
        }
        if(!DRV_IS_DUET2(lchip) && p_linkagg->mode == CTC_LINKAGG_MODE_RR)
        {
            uint32 cmd_rr = DRV_IOR(LagRrProfileIdMappingCtl_t, LagRrProfileIdMappingCtl_rrProfileId_f);
            uint32 rr_profile_id;
            DRV_IOCTL(lchip, p_wb_linkagg->tid, cmd_rr, &rr_profile_id);
            CTC_BIT_SET(p_usw_linkagg_master[lchip]->rr_profile_bmp, rr_profile_id);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_linkagg_wb_sync_func(void* array_data, uint32 index, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_linkagg_t *p_linkagg = (sys_linkagg_t *)array_data;
    sys_wb_linkagg_group_t  *p_wb_linkagg;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    uint8 lchip = (uint8)traversal_data->value1;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_linkagg = (sys_wb_linkagg_group_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_linkagg, 0, sizeof(sys_wb_linkagg_group_t));

    CTC_ERROR_RETURN(_sys_usw_linkagg_wb_mapping_group(lchip, p_wb_linkagg, p_linkagg, 1, 0));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_linkagg_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_linkagg_master_t  *p_wb_linkagg_master;

    LINKAGG_LOCK;

    wb_data.buffer = mem_malloc(MEM_LINKAGG_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    user_data.data = &wb_data;
    user_data.value1 = lchip;

    /*syncup linkagg master*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_linkagg_master_t, CTC_FEATURE_LINKAGG, SYS_WB_APPID_LINKAGG_SUBID_MASTER);

    p_wb_linkagg_master = (sys_wb_linkagg_master_t  *)wb_data.buffer;
    sal_memset(wb_data.buffer, 0, sizeof(sys_wb_linkagg_master_t));

    p_wb_linkagg_master->lchip = lchip;
    p_wb_linkagg_master->version = SYS_WB_VERSION_LINKAGG;
    p_wb_linkagg_master->linkagg_mode = p_usw_linkagg_master[lchip]->linkagg_mode;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup linkagg group*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_linkagg_group_t, CTC_FEATURE_LINKAGG, SYS_WB_APPID_LINKAGG_SUBID_GROUP);

    CTC_ERROR_GOTO(ctc_vector_traverse2(p_usw_linkagg_master[lchip]->group_vector, 0, _sys_usw_linkagg_wb_sync_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*syncup linkagg channel group*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_linkagg_group_t, CTC_FEATURE_LINKAGG, SYS_WB_APPID_LINKAGG_SUBID_CHANGROUP);

    CTC_ERROR_GOTO(ctc_vector_traverse2(p_usw_linkagg_master[lchip]->chan_group, 0, _sys_usw_linkagg_wb_sync_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    LINKAGG_UNLOCK;

    return ret;
}

int32
sys_usw_linkagg_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_linkagg_master_t  wb_linkagg_master;
    sys_wb_linkagg_group_t wb_linkagg_group;
    sys_linkagg_t *p_linkagg;

    LINKAGG_LOCK;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_LINKAGG_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*restore linkagg master*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_linkagg_master_t, CTC_FEATURE_LINKAGG, SYS_WB_APPID_LINKAGG_SUBID_MASTER);
    sal_memset(&wb_linkagg_master, 0, sizeof(sys_wb_linkagg_master_t));
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query linkagg master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        CTC_ERROR_GOTO(CTC_E_INVALID_CONFIG, ret, done);
    }
    sal_memcpy((uint8 *)&wb_linkagg_master, (uint8 *)wb_query.buffer, wb_query.key_len + wb_query.data_len);
    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_LINKAGG, wb_linkagg_master.version))
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }
    if (p_usw_linkagg_master[lchip]->linkagg_mode != wb_linkagg_master.linkagg_mode)
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }

    /*restore linkagg group*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_linkagg_group_t, CTC_FEATURE_LINKAGG, SYS_WB_APPID_LINKAGG_SUBID_GROUP);
    entry_cnt = 0;
    sal_memset(&wb_linkagg_group, 0, sizeof(sys_wb_linkagg_group_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8 *)&wb_linkagg_group, (uint8 *)wb_query.buffer + entry_cnt * (wb_query.key_len + wb_query.data_len), (wb_query.key_len + wb_query.data_len));
        entry_cnt++;
        p_linkagg = mem_malloc(MEM_LINKAGG_MODULE, sizeof(sys_linkagg_t));
        if (NULL == p_linkagg)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_linkagg, 0, sizeof(sys_linkagg_t));
        ret = _sys_usw_linkagg_wb_mapping_group(lchip, &wb_linkagg_group, p_linkagg, 0, 0);
        if (ret)
        {
            mem_free(p_linkagg);
            continue;
        }
        ctc_vector_add(p_usw_linkagg_master[lchip]->group_vector, p_linkagg->tid, p_linkagg);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*restore linkagg channel group*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_linkagg_group_t, CTC_FEATURE_LINKAGG, SYS_WB_APPID_LINKAGG_SUBID_CHANGROUP);
    entry_cnt = 0;
    sal_memset(&wb_linkagg_group, 0, sizeof(sys_wb_linkagg_group_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8 *)&wb_linkagg_group, (uint8 *)wb_query.buffer + entry_cnt * (wb_query.key_len + wb_query.data_len), (wb_query.key_len + wb_query.data_len));
        entry_cnt++;
        p_linkagg = mem_malloc(MEM_LINKAGG_MODULE,  sizeof(sys_linkagg_t));
        if (NULL == p_linkagg)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_linkagg, 0, sizeof(sys_linkagg_t));
        ret = _sys_usw_linkagg_wb_mapping_group(lchip, &wb_linkagg_group, p_linkagg, 0, 1);
        if (ret)
        {
            mem_free(p_linkagg);
            continue;
        }
        /*add to soft table*/
        ctc_vector_add(p_usw_linkagg_master[lchip]->chan_group, p_linkagg->tid, p_linkagg);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    LINKAGG_UNLOCK;

    return ret;
}

int32
sys_usw_linkagg_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;

    SYS_LINKAGG_INIT_CHECK();
    LINKAGG_LOCK;

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "##Linkagg");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "{");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","bind_gport_disable",p_usw_linkagg_master[lchip]->bind_gport_disable);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","linkagg_mode",p_usw_linkagg_master[lchip]->linkagg_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","linkagg_num",p_usw_linkagg_master[lchip]->linkagg_num);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","chanagg_grp_num",p_usw_linkagg_master[lchip]->chanagg_grp_num);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","chanagg_mem_per_grp",p_usw_linkagg_master[lchip]->chanagg_mem_per_grp);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type",p_usw_linkagg_master[lchip]->opf_type);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","rr_profile_bmp",p_usw_linkagg_master[lchip]->rr_profile_bmp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_linkagg_master[lchip]->opf_type, dump_db_fp);
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "}");

    LINKAGG_UNLOCK;

    return ret;
}

