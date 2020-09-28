/**
 @file sys_usw_aclqos_policer.c

 @date 2009-10-16

 @version v2.0

*/

/****************************************************************************
  *
  * Header Files
  *
  ****************************************************************************/
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_qos.h"
#include "ctc_debug.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_port.h"
#include "sys_usw_register.h"
#include "sys_usw_qos.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_acl_api.h"

#include "drv_api.h"

/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/
#define QOS_LOCK \
    if (p_usw_qos_master[lchip]->p_qos_mutex) sal_mutex_lock(p_usw_qos_master[lchip]->p_qos_mutex)
#define QOS_UNLOCK \
    if (p_usw_qos_master[lchip]->p_qos_mutex) sal_mutex_unlock(p_usw_qos_master[lchip]->p_qos_mutex)

#define SYS_QOS_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_qos_master[lchip]){ \
            return CTC_E_NOT_INIT;\
        } \
    } while (0)

/****************************************************************************
*
* Data structures
*
****************************************************************************/
/**
 @brief  Define sys layer qos global configure data structure
*/
struct sys_qos_master_s
{
    sal_mutex_t* p_qos_mutex;
};
typedef struct sys_qos_master_s sys_qos_master_t;

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
static sys_qos_master_t* p_usw_qos_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
* Function
*
*****************************************************************************/
#define __1_QOS_WB__

int32
sys_usw_qos_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_qos_master_t *p_wb_qos_master;

    wb_data.buffer = mem_malloc(MEM_QUEUE_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /* matser */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_qos_master_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_MASTER);

    p_wb_qos_master = (sys_wb_qos_master_t *)wb_data.buffer;
    sal_memset(p_wb_qos_master, 0, sizeof(sys_wb_qos_master_t));

    p_wb_qos_master->lchip = lchip;
    p_wb_qos_master->version = SYS_WB_VERSION_QOS;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /* policer */
    CTC_ERROR_GOTO(sys_usw_qos_policer_wb_sync(lchip, &wb_data), ret, done);

    /* queue */
    CTC_ERROR_GOTO(sys_usw_queue_wb_sync(lchip, &wb_data), ret, done);

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_qos_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_qos_master_t wb_qos_master;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_QUEUE_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /* master */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_qos_master_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_MASTER);

    sal_memset(&wb_qos_master, 0, sizeof(sys_wb_qos_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if ((wb_query.valid_cnt != 1) || (wb_query.is_end != 1))
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query qos master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    sal_memcpy((uint8 *)&wb_qos_master, (uint8 *)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_QOS, wb_qos_master.version))
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }

    /* policer */
    CTC_ERROR_GOTO(sys_usw_qos_policer_wb_restore(lchip, &wb_query), ret, done);

    /* queue */
    CTC_ERROR_GOTO(sys_usw_queue_wb_restore(lchip, &wb_query), ret, done);

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}

int32
sys_usw_qos_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    
    SYS_QOS_INIT_CHECK();
    QOS_LOCK;
    /* policer */
    ret = sys_usw_qos_policer_dump_db(lchip, p_f,p_dump_param);

    /* queue */
    ret = sys_usw_qos_queue_dump_db(lchip, p_f,p_dump_param);
    QOS_UNLOCK;
    
    return ret;
}

#define __2_QOS_SYS_API__

/* policer */
int32
sys_usw_qos_policer_id_check(uint8 lchip, uint16 policer_id, uint8 sys_policer_type, uint8* p_is_bwp)
{
    int32 ret = CTC_E_NONE;
    sys_qos_policer_param_t policer_param;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    sal_memset(&policer_param, 0, sizeof(policer_param));
    ret = _sys_usw_qos_policer_index_get(lchip, policer_id, sys_policer_type, &policer_param);
    if (p_is_bwp)
    {
        *p_is_bwp = policer_param.is_bwp;
    }

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_policer_index_get(uint8 lchip, uint16 policer_id, uint8 sys_policer_type,
                                                sys_qos_policer_param_t* p_policer_param)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_policer_index_get(lchip, policer_id, sys_policer_type, p_policer_param);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_policer_ingress_vlan_get(uint8 lchip, uint16* ingress_vlan_policer_num)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_policer_ingress_vlan_get(lchip, ingress_vlan_policer_num);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_policer_egress_vlan_get(uint8 lchip, uint16* egress_vlan_policer_num)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_policer_egress_vlan_get(lchip, egress_vlan_policer_num);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_policer_map_token_rate_user_to_hw(uint8 lchip, uint8 is_pps,
                                             uint32 user_rate, /*kb/s*/
                                             uint32 *hw_rate,
                                             uint16 bit_with,
                                             uint32 gran,
                                             uint32 upd_freq)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_policer_map_token_rate_user_to_hw(lchip,
                                                is_pps, user_rate, hw_rate, bit_with, gran, upd_freq);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_policer_map_token_rate_hw_to_user(uint8 lchip, uint8 is_pps,
                                                    uint32 hw_rate, uint32 *user_rate, uint32 upd_freq)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret =_sys_usw_qos_policer_map_token_rate_hw_to_user(lchip,
                                                    is_pps, hw_rate, user_rate, upd_freq);

    QOS_UNLOCK;

    return ret;
}

/* queue enq */
int32
sys_usw_queue_get_enq_mode(uint8 lchip, uint8 *enq_mode)
{
    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    *enq_mode = _sys_usw_queue_get_enq_mode(lchip);

    QOS_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_queue_get_service_queue_mode(uint8 lchip, uint8 *service_queue_mode)
{
    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    *service_queue_mode = _sys_usw_queue_get_service_queue_mode(lchip);

    QOS_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_get_channel_by_port(uint8 lchip, uint32 gport, uint8 *channel)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_get_channel_by_port(lchip, gport, channel);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_add_port_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_add_port_to_channel(lchip, lport, channel);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_remove_port_from_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_remove_port_from_channel(lchip, lport, channel);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_add_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_add_extend_port_to_channel(lchip, lport, channel);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_remove_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_remove_extend_port_to_channel(lchip, lport, channel);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_add_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    int32 ret = CTC_E_NONE;
    
    SYS_QOS_INIT_CHECK();
    QOS_LOCK;
    ret = _sys_usw_qos_add_mcast_queue_to_channel(lchip, lport, channel);
    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_remove_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    int32 ret = CTC_E_NONE;
    
    SYS_QOS_INIT_CHECK();
    QOS_LOCK;
    ret = _sys_usw_qos_remove_mcast_queue_to_channel(lchip, lport, channel);
    QOS_UNLOCK;

    return ret;
}

/* queue shape */
#if 0
int32
sys_usw_queue_shp_set_group_shape(uint8 lchip, ctc_qos_shape_group_t* p_shape)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_queue_shp_set_group_shape(lchip, p_shape);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_port_shape_profile(uint8 lchip, uint32 gport, uint32* rate, uint32* thrd, uint8* p_shp_en)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_get_port_shape_profile(lchip, gport, rate, thrd, p_shp_en);

    QOS_UNLOCK;

    return ret;
}
#endif
int32
sys_usw_qos_map_token_thrd_user_to_hw(uint8 lchip, uint32  user_bucket_thrd,
                                            uint32 *hw_bucket_thrd,
                                            uint8 shift_bits,
                                            uint32 max_thrd)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_map_token_thrd_user_to_hw(lchip, user_bucket_thrd,
                                                                                    hw_bucket_thrd, shift_bits, max_thrd);

    QOS_UNLOCK;

    return ret;
}
#if 0
int32
sys_usw_qos_map_token_thrd_hw_to_user(uint8 lchip, uint32  *user_bucket_thrd,
                                            uint16 hw_bucket_thrd,
                                            uint8 shift_bits)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_map_token_thrd_hw_to_user(lchip, user_bucket_thrd,
                                                                                        hw_bucket_thrd, shift_bits);

    QOS_UNLOCK;

    return ret;
}
#endif
/* queue drop */
int32
sys_usw_queue_set_port_drop_en(uint8 lchip, uint32 gport, bool enable, sys_qos_shape_profile_t* p_shp_profile)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_queue_set_port_drop_en(lchip, gport, enable, p_shp_profile);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_queue_get_profile_from_hw(uint8 lchip, uint32 gport, sys_qos_shape_profile_t* p_shp_profile)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_queue_get_profile_from_hw(lchip, gport, p_shp_profile);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_set_dma_channel_drop_en(uint8 lchip, bool enable)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_set_dma_channel_drop_en(lchip, enable);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_set_aqmscan_high_priority_en(uint8 lchip, bool enable)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_set_aqmscan_high_priority_en(lchip, enable);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_queue_get_port_depth(uint8 lchip, uint32 gport, uint32* p_depth)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_queue_get_port_depth(lchip, gport, p_depth);

    QOS_UNLOCK;

    return ret;
}

/* cpu reason */
int32
sys_usw_get_sub_queue_id_by_cpu_reason(uint8 lchip, uint16 reason_id, uint8* sub_queue_id)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_get_sub_queue_id_by_cpu_reason(lchip, reason_id, sub_queue_id);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_cpu_reason_alloc_exception_index(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_cpu_reason_alloc_exception_index(lchip, dir, p_cpu_rason_info);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_cpu_reason_free_exception_index(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_cpu_reason_free_exception_index(lchip, dir, p_cpu_rason_info);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_cpu_reason_get_info(uint8 lchip, uint16 reason_id, uint32 *destmap)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_cpu_reason_get_info(lchip, reason_id, destmap);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_cpu_reason_get_reason_info(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_cpu_reason_get_reason_info(lchip, dir, p_cpu_rason_info);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_cpu_reason_alloc_dsfwd_offset(uint8 lchip, uint16 reason_id,
                                           uint32 *dsfwd_offset,
                                           uint32 *dsnh_offset,
                                           uint32 *dest_port)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_cpu_reason_alloc_dsfwd_offset(lchip, reason_id, dsfwd_offset, dsnh_offset, dest_port);

    QOS_UNLOCK;

    return ret;
}

/* stacking */
int32
sys_usw_queue_add_for_stacking(uint8 lchip, uint32 gport)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_queue_add_for_stacking(lchip, gport);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_queue_remove_for_stacking(uint8 lchip, uint32 gport)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_queue_remove_for_stacking(lchip, gport);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_bind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_bind_service_logic_dstport(lchip, service_id, logic_dst_port);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_unbind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_unbind_service_logic_dstport(lchip, service_id, logic_dst_port);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_bind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port)
{
    int32 ret = CTC_E_NONE;
    
    SYS_QOS_INIT_CHECK();

    QOS_LOCK;
    
    ret = _sys_usw_qos_bind_service_logic_srcport(lchip, service_id, logic_port);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_unbind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;
    
    ret = _sys_usw_qos_unbind_service_logic_srcport(lchip, service_id, logic_port);

    QOS_UNLOCK;
 
    return ret;
}

int32
sys_usw_qos_set_drop_resrc_check_mode(uint8 lchip, uint8 mode)
{
    int32 ret = CTC_E_NONE;
    SYS_QOS_INIT_CHECK();
    QOS_LOCK;
    ret = sys_usw_qos_set_resrc_check_mode(lchip, mode);
    QOS_UNLOCK;
    return ret;
}

#define __3_QOS_SHOW_API__
int32
sys_usw_qos_domain_map_dump(uint8 lchip, uint8 domain, uint8 type)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_domain_map_dump(lchip, domain, type);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_table_map_dump(uint8 lchip, uint8 table_map_id, uint8 type)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_table_map_dump(lchip, table_map_id, type);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_policer_dump(uint8 type, uint8 dir, uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_policer_dump(type, dir, lchip, start, end, detail);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_queue_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_queue_dump(lchip, start, end, detail);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_group_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_group_dump(lchip, start, end, detail);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_port_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_port_dump(lchip, start, end, detail);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_service_dump(uint8 lchip, uint16 service_id, uint32 gport)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_service_dump(lchip, service_id, gport);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_cpu_reason_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_cpu_reason_dump(lchip, start, end, detail);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_dump_status(uint8 lchip)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_dump_status(lchip);

    QOS_UNLOCK;

    return ret;
}

#define __4_QOS_CTC_API__

/*policer*/
int32
sys_usw_qos_set_policer(uint8 lchip, ctc_qos_policer_t* p_policer)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_qos_policer_set(lchip, p_policer);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_policer(uint8 lchip, ctc_qos_policer_t* p_policer)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_qos_policer_get(lchip, p_policer);

    QOS_UNLOCK;

    return ret;
}

/*shape*/
int32
sys_usw_qos_set_shape(uint8 lchip, ctc_qos_shape_t* p_shape)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_set_shape(lchip, p_shape);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_shape(uint8 lchip, ctc_qos_shape_t* p_shape)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_get_shape(lchip, p_shape);

    QOS_UNLOCK;

    return ret;
}
/*schedule*/
int32
sys_usw_qos_set_sched(uint8 lchip, ctc_qos_sched_t* p_sched)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_set_sched(lchip, p_sched);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_sched(uint8 lchip, ctc_qos_sched_t* p_sched)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = _sys_usw_qos_get_sched(lchip, p_sched);

    QOS_UNLOCK;

    return ret;
}
/*mapping*/
int32
sys_usw_qos_set_domain_map(uint8 lchip, ctc_qos_domain_map_t* p_domain_map)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_qos_domain_map_set(lchip, p_domain_map);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_domain_map(uint8 lchip, ctc_qos_domain_map_t* p_domain_map)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_qos_domain_map_get(lchip, p_domain_map);

    QOS_UNLOCK;

    return ret;
}

/*global param*/
int32
sys_usw_qos_set_global_config(uint8 lchip, ctc_qos_glb_cfg_t* p_glb_cfg)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    switch (p_glb_cfg->cfg_type)
    {
    case CTC_QOS_GLB_CFG_POLICER_EN:
        ret = sys_usw_qos_set_policer_update_enable(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_POLICER_STATS_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case  CTC_QOS_GLB_CFG_POLICER_IPG_EN:
        ret = sys_usw_qos_set_policer_ipg_enable(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_POLICER_SEQENCE_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case  CTC_QOS_GLB_CFG_QUE_SHAPE_EN:
        ret = sys_usw_qos_set_queue_shape_enable(lchip, p_glb_cfg->u.value);
        break;

    case  CTC_QOS_GLB_CFG_GROUP_SHAPE_EN:
        ret = sys_usw_qos_set_group_shape_enable(lchip, p_glb_cfg->u.value);
        break;

    case  CTC_QOS_GLB_CFG_PORT_SHAPE_EN:
        ret = sys_usw_qos_set_port_shape_enable(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_SHAPE_IPG_EN:
        ret = sys_usw_qos_set_shape_ipg_enable(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_POLICER_FLOW_FIRST_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_GLB_CFG_RESRC_MGR_EN:
        ret = sys_usw_qos_resrc_mgr_en(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_QUE_STATS_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_GLB_CFG_PHB_MAP:
        ret = sys_usw_qos_set_phb(lchip, &p_glb_cfg->u.phb_map);
        break;

    case CTC_QOS_GLB_CFG_REASON_SHAPE_PKT_EN:
        ret = sys_usw_qos_set_reason_shp_base_pkt_en(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_POLICER_HBWP_SHARE_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_GLB_CFG_QUEUE_DROP_MONITOR_EN:
        ret = sys_usw_qos_set_monitor_drop_queue_id(lchip, &(p_glb_cfg->u.drop_monitor));
        break;

    case CTC_QOS_GLB_CFG_POLICER_MARK_ECN_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_GLB_CFG_SCH_WRR_EN:
        ret = sys_usw_qos_set_sch_wrr_enable(lchip, p_glb_cfg->u.value);
        break;

    case CTC_QOS_GLB_CFG_TABLE_MAP:
        ret = sys_usw_qos_table_map_set(lchip, &p_glb_cfg->u.table_map);
        break;
	case CTC_QOS_GLB_CFG_ECN_EN:
        ret = sys_usw_qos_set_ecn_enable(lchip, p_glb_cfg->u.value);
		break;
    case CTC_QOS_GLB_CFG_SHAPE_PKT_EN:
        ret = sys_usw_qos_set_port_shp_base_pkt_en(lchip, p_glb_cfg->u.value);
		break;
    case CTC_QOS_GLB_CFG_FCDL_INTERVAL:
        ret = sys_usw_qos_set_fcdl_interval(lchip, p_glb_cfg->u.value);
		break;
    case CTC_QOS_GLB_CFG_POLICER_PORT_STBM_EN:
        ret = sys_usw_qos_set_port_policer_stbm_enable(lchip, p_glb_cfg->u.value);
		break;
    default:
        ret = CTC_E_INVALID_PARAM;
        break;
    }

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_global_config(uint8 lchip, ctc_qos_glb_cfg_t* p_glb_cfg)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    switch (p_glb_cfg->cfg_type)
    {
    case CTC_QOS_GLB_CFG_TABLE_MAP:
        ret = sys_usw_qos_table_map_get(lchip, &p_glb_cfg->u.table_map);
        break;
	case CTC_QOS_GLB_CFG_ECN_EN:
        ret = sys_usw_qos_get_ecn_enable(lchip, &p_glb_cfg->u.value);
		break;
    case CTC_QOS_GLB_CFG_QUEUE_DROP_MONITOR_EN:
        ret = sys_usw_qos_get_monitor_drop_queue_id(lchip, &(p_glb_cfg->u.drop_monitor));
        break;
    case CTC_QOS_GLB_CFG_PHB_MAP:
        ret = sys_usw_qos_get_phb(lchip, &p_glb_cfg->u.phb_map);
        break;
    case  CTC_QOS_GLB_CFG_QUE_SHAPE_EN:
        ret = sys_usw_qos_get_queue_shape_enable(lchip, &p_glb_cfg->u.value);
        break;
    case  CTC_QOS_GLB_CFG_GROUP_SHAPE_EN:
        ret = sys_usw_qos_get_group_shape_enable(lchip, &p_glb_cfg->u.value);
        break;
    case  CTC_QOS_GLB_CFG_PORT_SHAPE_EN:
        ret = sys_usw_qos_get_port_shape_enable(lchip, &p_glb_cfg->u.value);
        break;
    default:
        ret = CTC_E_NOT_SUPPORT;
        break;
    }

    QOS_UNLOCK;

    return ret;
}

/*queue*/
int32
sys_usw_qos_set_queue(uint8 lchip, ctc_qos_queue_cfg_t* p_que_cfg)
{
    int32 ret = CTC_E_NONE;
    ctc_qos_queue_cpu_reason_dest_t* p_reason_dest = &p_que_cfg->value.reason_dest;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    /*to prevent qos deadlock, put acl code here*/
    if (CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST == p_que_cfg->type)
    {
        if (sys_usw_common_check_feature_support(CTC_FEATURE_WLAN) && (CTC_PKT_CPU_REASON_WLAN_CHECK_ERR == p_reason_dest->cpu_reason))
        {
            if (CTC_PKT_CPU_REASON_TO_DROP != p_reason_dest->dest_type)
            {
                CTC_ERROR_RETURN(sys_usw_acl_set_wlan_crypt_error_to_cpu_en(lchip, 1));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_acl_set_wlan_crypt_error_to_cpu_en(lchip, 0));
            }
        }
        else if (sys_usw_common_check_feature_support(CTC_FEATURE_DOT1AE) && (CTC_PKT_CPU_REASON_DOT1AE_CHECK_ERR == p_reason_dest->cpu_reason))
        {
            if (CTC_PKT_CPU_REASON_TO_DROP != p_reason_dest->dest_type)
            {
                CTC_ERROR_RETURN(sys_usw_acl_set_dot1ae_crypt_error_to_cpu_en(lchip, 1));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_acl_set_dot1ae_crypt_error_to_cpu_en(lchip, 0));
            }
        }
    }

    QOS_LOCK;

    ret = sys_usw_qos_queue_set(lchip, p_que_cfg);

    QOS_UNLOCK;

    /*to prevent qos deadlock, put acl code here*/
    if (ret)
    {
        if (CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST == p_que_cfg->type)
        {
            if (sys_usw_common_check_feature_support(CTC_FEATURE_WLAN) && (CTC_PKT_CPU_REASON_WLAN_CHECK_ERR == p_reason_dest->cpu_reason))
            {
                if (CTC_PKT_CPU_REASON_TO_DROP != p_reason_dest->dest_type)
                {
                    sys_usw_acl_set_wlan_crypt_error_to_cpu_en(lchip, 0);
                }
                else
                {
                    sys_usw_acl_set_wlan_crypt_error_to_cpu_en(lchip, 1);
                }
            }
            else if (sys_usw_common_check_feature_support(CTC_FEATURE_DOT1AE) && (CTC_PKT_CPU_REASON_DOT1AE_CHECK_ERR == p_reason_dest->cpu_reason))
            {
                if (CTC_PKT_CPU_REASON_TO_DROP != p_reason_dest->dest_type)
                {
                    sys_usw_acl_set_dot1ae_crypt_error_to_cpu_en(lchip, 0);
                }
                else
                {
                    sys_usw_acl_set_dot1ae_crypt_error_to_cpu_en(lchip, 1);
                }
            }
        }
    }

    return ret;
}

/*drop*/
int32
sys_usw_qos_set_drop_scheme(uint8 lchip, ctc_qos_drop_t* p_drop)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_set_drop(lchip, p_drop);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_drop_scheme(uint8 lchip, ctc_qos_drop_t* p_drop)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_get_drop(lchip, p_drop);

    QOS_UNLOCK;

    return ret;
}

/*Resrc*/
int32
sys_usw_qos_set_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_set_resrc(lchip, p_resrc);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_get_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_get_resrc(lchip, p_resrc);

    QOS_UNLOCK;

    return ret;
}

/*stats*/
int32
sys_usw_qos_query_pool_stats(uint8 lchip, ctc_qos_resrc_pool_stats_t* p_stats)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_query_pool_stats(lchip, p_stats);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_query_queue_stats(uint8 lchip, ctc_qos_queue_stats_t* p_queue_stats)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_stats_query(lchip, p_queue_stats);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_clear_queue_stats(uint8 lchip, ctc_qos_queue_stats_t* p_queue_stats)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_queue_stats_clear(lchip, p_queue_stats);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_query_policer_stats(uint8 lchip, ctc_qos_policer_stats_t* p_policer_stats)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_qos_policer_stats_query(lchip, p_policer_stats);

    QOS_UNLOCK;

    return ret;
}

int32
sys_usw_qos_clear_policer_stats(uint8 lchip, ctc_qos_policer_stats_t* p_policer_stats)
{
    int32 ret = CTC_E_NONE;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_INIT_CHECK();

    QOS_LOCK;

    ret = sys_usw_qos_policer_stats_clear(lchip, p_policer_stats);

    QOS_UNLOCK;

    return ret;
}

#define __5_QOS_INIT__

/*init*/
int32
sys_usw_qos_init(uint8 lchip, void* p_glb_parm)
{
    int32 ret = CTC_E_NONE;

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_glb_parm);

    if (NULL != p_usw_qos_master[lchip])
    {
        return CTC_E_NONE;
    }

    /* create qos master */
    p_usw_qos_master[lchip] = (sys_qos_master_t *)mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_master_t));
    if (NULL == p_usw_qos_master[lchip])
    {
	ret = CTC_E_NO_MEMORY;
	goto error0;
    }

    sal_memset(p_usw_qos_master[lchip], 0, sizeof(sys_qos_master_t));

    ret = sal_mutex_create(&p_usw_qos_master[lchip]->p_qos_mutex);
    if (ret || !p_usw_qos_master[lchip]->p_qos_mutex)
    {
	ret = CTC_E_NO_MEMORY;
	goto error1;
    }
    CTC_ERROR_GOTO(sys_usw_qos_class_init(lchip), ret, error2);
    CTC_ERROR_GOTO(sys_usw_qos_policer_init(lchip, p_glb_parm), ret, error3);
    CTC_ERROR_GOTO(sys_usw_queue_sch_init(lchip), ret, error4);
    CTC_ERROR_GOTO(sys_usw_queue_enq_init(lchip, p_glb_parm), ret, error5);
    CTC_ERROR_GOTO(sys_usw_queue_shape_init(lchip), ret, error6);
    CTC_ERROR_GOTO(sys_usw_queue_drop_init(lchip, p_glb_parm), ret, error7);
    CTC_ERROR_GOTO(sys_usw_queue_cpu_reason_init(lchip), ret, error8);
    /* warmboot register */
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_QOS, sys_usw_qos_wb_sync), ret, error9);
    /* dump db register */
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_QOS, sys_usw_qos_dump_db), ret, error9);
    /* warmboot data restore */
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_GOTO(sys_usw_qos_wb_restore(lchip), ret, error9);
    }
    
    return CTC_E_NONE;

error9:
    (void)sys_usw_queue_cpu_reason_deinit(lchip);
error8:
    (void)sys_usw_queue_drop_deinit(lchip);
error7:
    (void)sys_usw_queue_shape_deinit(lchip);
error6:
    (void)sys_usw_queue_enq_deinit(lchip);
error5:
    (void)sys_usw_queue_sch_deinit(lchip);
error4:
    (void)sys_usw_qos_policer_deinit(lchip);
error3:
    (void)sys_usw_qos_class_deinit(lchip);
error2:
    sal_mutex_destroy(p_usw_qos_master[lchip]->p_qos_mutex);
error1:
    mem_free(p_usw_qos_master[lchip]);
error0:
    return ret;
}

/*deinit*/
int32
sys_usw_qos_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == p_usw_qos_master[lchip])
    {
        return CTC_E_NONE;
    }

    (void)sys_usw_queue_cpu_reason_deinit(lchip);

    (void)sys_usw_queue_drop_deinit(lchip);

    (void)sys_usw_queue_sch_deinit(lchip);

    (void)sys_usw_queue_shape_deinit(lchip);

    (void)sys_usw_queue_enq_deinit(lchip);

    (void)sys_usw_qos_policer_deinit(lchip);

    (void)sys_usw_qos_class_deinit(lchip);

    sal_mutex_destroy(p_usw_qos_master[lchip]->p_qos_mutex);

    mem_free(p_usw_qos_master[lchip]);

    return CTC_E_NONE;
}

