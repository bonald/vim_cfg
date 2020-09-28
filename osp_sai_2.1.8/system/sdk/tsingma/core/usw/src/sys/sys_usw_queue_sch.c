/**
 @file sys_usw_queue_sch.c

 @date 2010-01-13

 @version v2.0

*/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_queue.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_qos.h"

#include "usw/include/drv_io.h"

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/

extern sys_queue_master_t* p_usw_queue_master[CTC_MAX_LOCAL_CHIP_NUM];

#define QOS_SCHED_MAX_QUE_CLASS 7
#define QOS_SCHED_MAX_EXT_QUE_CLASS 3
#define QOS_SCHED_MAX_CHAN_CLASS 3

/****************************************************************************
 *
 * Function
 *
 ****************************************************************************/
#if 0
int32
_sys_usw_queue_sch_threshold_compute(uint8 lchip, uint32 weight,
                                           uint16* p_threshold,
                                           uint8* p_shift)
{
   uint32 hw_weight;

   _sys_usw_qos_map_token_thrd_user_to_hw(lchip, weight, &hw_weight,4, ((1 << 10) -1));
   *p_threshold = hw_weight >> 4;
   *p_shift   = hw_weight & 0xF;

   return CTC_E_NONE;
}

int32
_sys_usw_queue_sch_threshold_compute_to_user(uint8 lchip, uint32 *user_weight,
                                                    uint16 threshold,
                                                    uint8 shift)
{
    uint32 hw_bucket_thrd = 0;

    hw_bucket_thrd = (threshold << 4)  | shift;
    _sys_usw_qos_map_token_thrd_hw_to_user(lchip, user_weight, hw_bucket_thrd, 4);
    *user_weight = threshold << shift;

    return CTC_E_NONE;
}
#endif
int32
sys_usw_qos_set_sch_wrr_enable(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint8  channel_id = 0;
    uint8  pps_mode = 0;
    DsQMgrNetSchPpsCfg_m qmgr_ctl;

    sal_memset(&qmgr_ctl, 0, sizeof(DsQMgrNetSchPpsCfg_m));
    pps_mode = enable ? 1 : 0;
    SetDsQMgrNetSchPpsCfg(V, ppsMode_f, &qmgr_ctl, pps_mode);

    cmd = DRV_IOW(DsQMgrNetSchPpsCfg_t, DRV_ENTRY_FLAG);
    for (channel_id = 0; channel_id < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); channel_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &qmgr_ctl));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_sch_set_queue_sched(uint8 lchip, ctc_qos_sched_queue_t* p_sched)
{
    uint32 cmd       = 0;
    uint16 queue_id  = 0;
    uint16 offset    = 0;
    uint16 group_id  = 0;
    uint32 field_val = 0;
    uint32 old_profile_id = 0;
    uint32 field_id  = 0;
    uint16 step      = 0;
    uint16 queue_index  = 0;
    uint32 fvalue[4];
    ds_t ds;
    uint8 index = 0;
    sys_queue_node_t* p_sys_queue_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->cfg_type = %d\n", p_sched->cfg_type);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue.queue_type = %d\n", p_sched->queue.queue_type);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue.queue_id = %d\n", p_sched->queue.queue_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue.gport = 0x%x\n", p_sched->queue.gport);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue.service_id = 0x%x\n", p_sched->queue.service_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->exceed_weight = %d\n", p_sched->exceed_weight);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->exceed_class = %d\n", p_sched->exceed_class);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_sched->queue,
                                                       &queue_id));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id = %d\n", queue_id);

    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec,queue_id);
    if (NULL == p_sys_queue_node)
    {
        return CTC_E_INVALID_PARAM;
    }

    offset = p_sys_queue_node->offset;

    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        group_id = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) / MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP) ;
    }
    else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        group_id = queue_id / 8;
    }
    else if (SYS_IS_CPU_QUEUE(queue_id))
    {
        queue_index = queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id = %d\n", group_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue offset = %d\n", offset);

    switch (p_sched->cfg_type)
    {
    case CTC_QOS_SCHED_CFG_CONFIRM_CLASS:
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    case CTC_QOS_SCHED_CFG_EXCEED_CLASS:
        field_val = p_sched->exceed_class;
        if (SYS_IS_EXT_QUEUE(queue_id))
        {
            CTC_MAX_VALUE_CHECK(p_sched->exceed_class, MCHIP_CAP(SYS_CAP_QOS_SCHED_MAX_EXT_QUE_CLASS));
            field_val = p_sched->exceed_class;
            step = DsQMgrExtSchMap_que_1_grpNodeSel_f - DsQMgrExtSchMap_que_0_grpNodeSel_f;
            field_id = DsQMgrExtSchMap_que_0_grpNodeSel_f + offset * step;
            cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        }
        else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
        {
            CTC_MAX_VALUE_CHECK(p_sched->exceed_class, QOS_SCHED_MAX_QUE_CLASS);
            field_val = p_sched->exceed_class;
            field_id = DsQMgrBaseSchMap_que_0_grpNodeSel_f + offset;
            cmd = DRV_IOW(DsQMgrBaseSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        }
        else if (SYS_IS_CPU_QUEUE(queue_id))
        {
            CTC_MAX_VALUE_CHECK(p_sched->exceed_class, QOS_SCHED_MAX_CHAN_CLASS);
            cmd = DRV_IOR(DsQMgrDmaQueShpProfId_t, DsQMgrDmaQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &old_profile_id));
            field_val = 1;
            cmd = DRV_IOW(DsQMgrDmaQueShpProfId_t, DsQMgrDmaQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &field_val));
            do
            {
                cmd = DRV_IOR(QMgrDmaQueSchState_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
                GetQMgrDmaQueSchState(A,  dmaQueInChListVec_f, ds, fvalue);
                if(!CTC_IS_BIT_SET(fvalue[queue_index >> 5], queue_index&0x1F))
                {
                    break;
                }
                if ((index++) > 10)
                {
                    sal_task_sleep(10);
                    if (index > 50)
                    {
                        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Cannot clear DmaQueSch status\n");
                        cmd = DRV_IOW(DsQMgrDmaQueShpProfId_t, DsQMgrDmaQueShpProfId_profId_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &old_profile_id));
                        return CTC_E_HW_FAIL;
                    }
                }
            }
            while (1);
            field_val = p_sched->exceed_class;
            field_id = DsQMgrDmaSchMap_chanNodeSel_f;
            cmd = DRV_IOW(DsQMgrDmaSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &field_val));
            cmd = DRV_IOW(DsQMgrDmaQueShpProfId_t, DsQMgrDmaQueShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &old_profile_id));
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }

        break;

    case CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT:
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    case CTC_QOS_SCHED_CFG_EXCEED_WEIGHT:
        {
            uint32 weight = 0;

            CTC_MIN_VALUE_CHECK(p_sched->exceed_weight, 1);
            weight = p_sched->exceed_weight;
            CTC_MAX_VALUE_CHECK(weight, 0x7F);

            if (SYS_IS_EXT_QUEUE(queue_id))
            {
                field_id = DsQMgrExtGrpWfqWeight_que_0_weight_f + offset;
                field_val = weight;
                cmd = DRV_IOW(DsQMgrExtGrpWfqWeight_t, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
            }
            else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
            {
                field_id = DsQMgrNetBaseGrpWfqWeight_que_0_weight_f + offset;
                field_val = weight;
                cmd = DRV_IOW(DsQMgrNetBaseGrpWfqWeight_t, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}


int32
sys_usw_queue_sch_set_c2c_group_sched(uint8 lchip, uint16 queue_id, uint8 class_priority)
{
    uint16  group_id = 0;
    uint8  channel_id = 0;
    uint32 cmd = 0;
    uint8  queue_index = 0;
    uint8  queue_offset = 0;
    DsQMgrBaseSchMap_m          base_sch_map;
    DsQMgrExtSchMap_m           ext_sch_map;
    DsQMgrDmaSchMap_m           dma_sch_map;

    sal_memset(&base_sch_map, 0, sizeof(DsQMgrBaseSchMap_m));
    sal_memset(&ext_sch_map, 0, sizeof(DsQMgrExtSchMap_m));
    sal_memset(&dma_sch_map, 0, sizeof(DsQMgrDmaSchMap_m));

    CTC_MAX_VALUE_CHECK(class_priority, QOS_SCHED_MAX_CHAN_CLASS);
    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        group_id = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) / MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        cmd = DRV_IOR(DsQMgrExtSchMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &ext_sch_map));
        SetDsQMgrExtSchMap(V, grpNode_0_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_1_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_2_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_3_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_4_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_5_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_6_chanNodeSel_f, &ext_sch_map, class_priority);
        SetDsQMgrExtSchMap(V, grpNode_7_chanNodeSel_f, &ext_sch_map, class_priority);

        cmd = DRV_IOW(DsQMgrExtSchMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &ext_sch_map));
    }
    else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        channel_id = queue_id / 8;
        cmd = DRV_IOR(DsQMgrBaseSchMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &base_sch_map));
        SetDsQMgrBaseSchMap(V, grpNode_0_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_1_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_2_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_3_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_4_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_5_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_6_chanNodeSel_f, &base_sch_map, class_priority);
        SetDsQMgrBaseSchMap(V, grpNode_7_chanNodeSel_f, &base_sch_map, class_priority);
        cmd = DRV_IOW(DsQMgrBaseSchMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &base_sch_map));
    }
    else if(SYS_IS_CPU_QUEUE(queue_id))
    {
        /*grp_id is reason grp*/
        group_id = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP))/8;
        SetDsQMgrDmaSchMap(V, chanNodeSel_f, &dma_sch_map, class_priority);
        cmd = DRV_IOW(DsQMgrDmaSchMap_t, DRV_ENTRY_FLAG);
        for (queue_offset = 0; queue_offset < MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM); queue_offset++)
        {
            queue_index = (group_id * MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM)) + queue_offset;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &dma_sch_map));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_sch_get_queue_sched(uint8 lchip, uint16 queue_id ,ctc_qos_sched_queue_t* p_sched)
{
    sys_queue_node_t* p_sys_queue_node = NULL;
    uint16 offset  = 0;
    uint16  group_id = 0;
    uint32 field_val = 0;
    uint32 field_id  = 0;
    uint32 cmd = 0;
    uint16  step = 0;
    uint16 weight = 0;
    uint16 queue_index = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(p_sched->cfg_type == CTC_QOS_SCHED_CFG_CONFIRM_CLASS || p_sched->cfg_type == CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (queue_id == CTC_MAX_UINT16_VALUE)
    {
        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_sched->queue,
                                                           &queue_id));
    }
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "queue_id = %d\n", queue_id);

    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec,queue_id);
    if (NULL == p_sys_queue_node)
    {
        return CTC_E_INVALID_PARAM;
    }

    offset = p_sys_queue_node->offset;

    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        group_id = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) / MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        step = DsQMgrExtSchMap_que_1_grpNodeSel_f - DsQMgrExtSchMap_que_0_grpNodeSel_f;
        field_id = DsQMgrExtSchMap_que_0_grpNodeSel_f + offset * step;
        cmd = DRV_IOR(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        p_sched->exceed_class = field_val;

        field_id = DsQMgrExtGrpWfqWeight_que_0_weight_f + offset;
        cmd = DRV_IOR(DsQMgrExtGrpWfqWeight_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        weight = field_val;

        p_sched->exceed_weight = weight;
    }
    else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        group_id = queue_id / 8;
        field_id = DsQMgrBaseSchMap_que_0_grpNodeSel_f + offset;
        cmd = DRV_IOR(DsQMgrBaseSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        p_sched->exceed_class = field_val;

        field_id = DsQMgrNetBaseGrpWfqWeight_que_0_weight_f + offset;
        cmd = DRV_IOR(DsQMgrNetBaseGrpWfqWeight_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        weight = field_val;

        p_sched->exceed_weight = weight;
    }
    else if (SYS_IS_CPU_QUEUE(queue_id))
    {
        queue_index = queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);

        CTC_MAX_VALUE_CHECK(p_sched->exceed_class, QOS_SCHED_MAX_CHAN_CLASS);
        field_id = DsQMgrDmaSchMap_chanNodeSel_f;
        cmd = DRV_IOR(DsQMgrDmaSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &field_val));
        p_sched->exceed_class = field_val;
    }
    else
    {
        p_sched->exceed_class = offset;
    }

    return CTC_E_NONE;

}

int32
sys_usw_queue_sch_set_group_sched(uint8 lchip, ctc_qos_sched_group_t* p_sched)
{
    sys_queue_node_t* p_sys_queue_node = NULL;
    uint32 cmd                     = 0;
    uint16 wfq_offset              = 0;
    uint16  group_id                = 0;
    uint32 field_val               = 0;
    uint32 field_id                = 0;
    uint16 queue_id                = 0;
    uint16 step                    = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->cfg_type = %d\n", p_sched->cfg_type);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->class_priority = %d\n", p_sched->class_priority);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue_class = %d\n", p_sched->queue_class);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue.service_id = 0x%x\n", p_sched->queue.service_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,  "p_sched->queue.gport = 0x%x\n", p_sched->queue.gport);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_sched->queue,
                                                       &queue_id));
    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        return CTC_E_INVALID_PARAM;
    }

    wfq_offset = p_sched->queue_class;

    /*get group_id*/
    if (SYS_IS_EXT_QUEUE(queue_id))
    {
        group_id = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) / MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP) ;
    }
    else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        group_id = queue_id / 8;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id = %d\n", group_id);

    switch (p_sched->cfg_type)
    {
    case CTC_QOS_SCHED_CFG_CONFIRM_CLASS:
    {
        CTC_MAX_VALUE_CHECK(p_sched->class_priority, QOS_SCHED_MAX_CHAN_CLASS);

        if (SYS_IS_EXT_QUEUE(queue_id))
        {
            CTC_MAX_VALUE_CHECK(wfq_offset, MCHIP_CAP(SYS_CAP_QOS_SCHED_MAX_EXT_QUE_CLASS));
            field_id = DsQMgrExtSchMap_grpNode_0_chanNodeSel_f + wfq_offset;
            field_val = p_sched->class_priority;
            cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        }
        else if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
        {
            CTC_MAX_VALUE_CHECK(wfq_offset, QOS_SCHED_MAX_QUE_CLASS);
            step = DsQMgrBaseSchMap_grpNode_1_chanNodeSel_f - DsQMgrBaseSchMap_grpNode_0_chanNodeSel_f;
            field_id = DsQMgrBaseSchMap_grpNode_0_chanNodeSel_f + wfq_offset * step;
            field_val = p_sched->class_priority;
            cmd = DRV_IOW(DsQMgrBaseSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    break;

    case CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT:
      {
          uint32 weight = 0;
          DsQMgrNetGrpSchWeight_m  grp_wfq_weight;

          CTC_MIN_VALUE_CHECK(p_sched->weight, 1);
          weight = p_sched->weight;
          CTC_MAX_VALUE_CHECK(weight, 0x7F);

          SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "weight = %d\n", weight);

          /*groupId[6] means extGrp*/
          if (SYS_IS_EXT_QUEUE(queue_id))
          {
              group_id = group_id + MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM);
          }
          cmd = DRV_IOR(DsQMgrNetGrpSchWeight_t, DRV_ENTRY_FLAG);
          CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &grp_wfq_weight));
          SetDsQMgrNetGrpSchWeight(V, node_0_weight_f, &grp_wfq_weight, weight);
          SetDsQMgrNetGrpSchWeight(V, node_1_weight_f, &grp_wfq_weight, weight);
          SetDsQMgrNetGrpSchWeight(V, node_2_weight_f, &grp_wfq_weight, weight);
          SetDsQMgrNetGrpSchWeight(V, node_3_weight_f, &grp_wfq_weight, weight);
          cmd = DRV_IOW(DsQMgrNetGrpSchWeight_t, DRV_ENTRY_FLAG);
          CTC_ERROR_RETURN(DRV_IOCTL(lchip,  group_id , cmd, &grp_wfq_weight));
      }
      break;
    case CTC_QOS_SCHED_CFG_SUB_GROUP_ID:
      {
          CTC_MAX_VALUE_CHECK(group_id, 63);
          CTC_MAX_VALUE_CHECK(p_sched->sub_group_id, 2);
          if (p_sched->queue.queue_type != CTC_QUEUE_TYPE_NETWORK_EGRESS)
          {
              return CTC_E_INVALID_PARAM;
          }
          CTC_MAX_VALUE_CHECK(wfq_offset, QOS_SCHED_MAX_QUE_CLASS);
          if(p_sched->sub_group_id == 2)
          {
              step = DsQMgrBaseSchMap_grpNode_1_ets0NodeSel_f - DsQMgrBaseSchMap_grpNode_0_ets0NodeSel_f;
              field_val = 0;
              cmd = DRV_IOW(DsQMgrBaseSchMap_t, DsQMgrBaseSchMap_grpNode_0_ets0NodeSel_f + step*wfq_offset);
              CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
          }
          else
          {
              step = DsQMgrBaseSchMap_grpNode_1_ets0NodeSel_f - DsQMgrBaseSchMap_grpNode_0_ets0NodeSel_f;
              field_val = 1;
              cmd = DRV_IOW(DsQMgrBaseSchMap_t, DsQMgrBaseSchMap_grpNode_0_ets0NodeSel_f + step*wfq_offset);
              CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
              step = DsQMgrBaseSchMap_grpNode_1_ets1NodeSel_f - DsQMgrBaseSchMap_grpNode_0_ets1NodeSel_f;
              field_val = p_sched->sub_group_id;
              cmd = DRV_IOW(DsQMgrBaseSchMap_t, DsQMgrBaseSchMap_grpNode_0_ets1NodeSel_f + step*wfq_offset);
              CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
          }
      }
      break;
    case CTC_QOS_SCHED_CFG_SUB_GROUP_WEIGHT:
      {
          CTC_MAX_VALUE_CHECK(group_id, 63);
          CTC_MAX_VALUE_CHECK(p_sched->sub_group_id, 1);
          CTC_MAX_VALUE_CHECK(p_sched->weight, 0x7F);
          if (p_sched->queue.queue_type != CTC_QUEUE_TYPE_NETWORK_EGRESS)
          {
              return CTC_E_INVALID_PARAM;
          }

          step = DsQMgrNetBaseGrpWfqWeight_ets1Node_1_weight_f - DsQMgrNetBaseGrpWfqWeight_ets1Node_0_weight_f;
          field_val = p_sched->weight;
          cmd = DRV_IOW(DsQMgrNetBaseGrpWfqWeight_t, DsQMgrNetBaseGrpWfqWeight_ets1Node_0_weight_f + step*p_sched->sub_group_id);
          CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
      }
      break;

    default:
        break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_sch_get_group_sched(uint8 lchip, uint16 grp_id, ctc_qos_sched_group_t* p_sched)
{
    sys_queue_node_t* p_sys_queue_node = NULL;
    DsQMgrNetGrpSchWeight_m  grp_wfq_weight;
    uint32 cmd                     = 0;
    uint16 wfq_offset              = 0;
    uint32 field_val               = 0;
    uint32 field_id                = 0;
    uint16 queue_id                = 0;
    uint16 weight                  = 0;
    uint16  group_id                = 0;
    uint16 step                    = 0;

    if (grp_id  == CTC_MAX_UINT16_VALUE)
    {
        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_sched->queue, &queue_id));
        p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);

        if ((NULL == p_sys_queue_node) || !(SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id)))
        {
            return CTC_E_INVALID_PARAM;
        }
        grp_id = SYS_GROUP_ID(queue_id);
    }

    wfq_offset = p_sched->queue_class;
    if(p_sched->cfg_type != CTC_QOS_SCHED_CFG_SUB_GROUP_ID && p_sched->cfg_type != CTC_QOS_SCHED_CFG_SUB_GROUP_WEIGHT)
    {
        if (SYS_IS_EXT_GROUP(grp_id))
        {
            CTC_MAX_VALUE_CHECK(wfq_offset, MCHIP_CAP(SYS_CAP_QOS_SCHED_MAX_EXT_QUE_CLASS));
            group_id = grp_id - MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM);
            field_id = DsQMgrExtSchMap_grpNode_0_chanNodeSel_f + wfq_offset;
            cmd = DRV_IOR(DsQMgrExtSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
            p_sched->class_priority = field_val;
        }
        else if (SYS_IS_BASE_GROUP(grp_id))
        {
            CTC_MAX_VALUE_CHECK(wfq_offset, QOS_SCHED_MAX_QUE_CLASS);
            group_id = grp_id;
            step = DsQMgrBaseSchMap_grpNode_1_chanNodeSel_f - DsQMgrBaseSchMap_grpNode_0_chanNodeSel_f;
            field_id = DsQMgrBaseSchMap_grpNode_0_chanNodeSel_f + wfq_offset * step;
            cmd = DRV_IOR(DsQMgrBaseSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
            p_sched->class_priority = field_val;
        }

        /*groupId[6] means extGrp*/
        if ((SYS_IS_EXT_GROUP(grp_id)) || (SYS_IS_BASE_GROUP(grp_id)))
        {
            group_id = grp_id;
            cmd = DRV_IOR(DsQMgrNetGrpSchWeight_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &grp_wfq_weight));

            weight = GetDsQMgrNetGrpSchWeight(V, node_0_weight_f, &grp_wfq_weight);
            p_sched->weight = weight;
        }
    }
    else
    {
        CTC_MAX_VALUE_CHECK(wfq_offset, QOS_SCHED_MAX_QUE_CLASS);
        CTC_MAX_VALUE_CHECK(grp_id, MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)-1);
        step = DsQMgrBaseSchMap_grpNode_1_ets0NodeSel_f - DsQMgrBaseSchMap_grpNode_0_ets0NodeSel_f;
        cmd = DRV_IOR(DsQMgrBaseSchMap_t, DsQMgrBaseSchMap_grpNode_0_ets0NodeSel_f + step*wfq_offset);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, grp_id, cmd, &field_val));
        if(field_val == 0)
        {
            p_sched->sub_group_id = 2;
        }
        else
        {
            step = DsQMgrBaseSchMap_grpNode_1_ets1NodeSel_f - DsQMgrBaseSchMap_grpNode_0_ets1NodeSel_f;
            cmd = DRV_IOR(DsQMgrBaseSchMap_t, DsQMgrBaseSchMap_grpNode_0_ets1NodeSel_f + step*wfq_offset);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, grp_id, cmd, &field_val));
            p_sched->sub_group_id = field_val;

            step = DsQMgrNetBaseGrpWfqWeight_ets1Node_1_weight_f - DsQMgrNetBaseGrpWfqWeight_ets1Node_0_weight_f;
            cmd = DRV_IOR(DsQMgrNetBaseGrpWfqWeight_t, DsQMgrNetBaseGrpWfqWeight_ets1Node_0_weight_f + step*p_sched->sub_group_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, grp_id, cmd, &field_val));
            p_sched->weight = field_val;
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_qos_set_sched(uint8 lchip, ctc_qos_sched_t* p_sched)
{
    switch (p_sched->type)
    {
    case CTC_QOS_SCHED_QUEUE:
        CTC_ERROR_RETURN(sys_usw_queue_sch_set_queue_sched(lchip, &p_sched->sched.queue_sched));
        break;

    case CTC_QOS_SCHED_GROUP:
        CTC_ERROR_RETURN(sys_usw_queue_sch_set_group_sched(lchip, &p_sched->sched.group_sched));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_get_sched(uint8 lchip, ctc_qos_sched_t* p_sched)
{
    uint16 queue_id = 0;
    uint8  grp_id = 0;
    sys_queue_node_t* p_sys_queue_node = NULL;

    switch (p_sched->type)
    {
    case CTC_QOS_SCHED_QUEUE:
        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_sched->sched.queue_sched.queue,
                                                           &queue_id));
        CTC_ERROR_RETURN(sys_usw_queue_sch_get_queue_sched(lchip, queue_id, &p_sched->sched.queue_sched));
        break;

    case CTC_QOS_SCHED_GROUP:
        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_sched->sched.group_sched.queue,
                                                           &queue_id));
        p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);

        if ((NULL == p_sys_queue_node) || !(SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id)))
        {
            return CTC_E_INVALID_PARAM;
        }
        grp_id = SYS_GROUP_ID(queue_id);
        CTC_ERROR_RETURN(sys_usw_queue_sch_get_group_sched(lchip, grp_id, &p_sched->sched.group_sched));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_sch_init_wfq_sched(uint8 lchip)
{
    uint32 cmd = 0;
    DsQMgrBaseSchMap_m          base_sch_map;
    DsQMgrExtSchMap_m           ext_sch_map;
    DsQMgrNetBaseGrpWfqWeight_m baseGrp_wfq_weight;
    DsQMgrExtGrpWfqWeight_m     extGrp_wfq_weight;
    DsQMgrNetGrpSchWeight_m     netGrp_wfq_weight;
    DsQMgrDmaSchMap_m           dma_sch_map;
    uint8  channel_id = 0;
    uint16  group_id   = 0;

    sal_memset(&baseGrp_wfq_weight, 0, sizeof(DsQMgrNetBaseGrpWfqWeight_m));
    sal_memset(&extGrp_wfq_weight, 0, sizeof(DsQMgrExtGrpWfqWeight_m));
    sal_memset(&netGrp_wfq_weight, 0, sizeof(DsQMgrNetGrpSchWeight_m));
    sal_memset(&base_sch_map, 0, sizeof(DsQMgrBaseSchMap_m));
    sal_memset(&ext_sch_map, 0, sizeof(DsQMgrExtSchMap_m));
    sal_memset(&dma_sch_map, 0, sizeof(DsQMgrDmaSchMap_m));

    /*config queue and ets1node weight of base group*/
    SetDsQMgrNetBaseGrpWfqWeight(V, ets1Node_0_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, ets1Node_1_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_0_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_1_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_2_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_3_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_4_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_5_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_6_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetBaseGrpWfqWeight(V, que_7_weight_f, &baseGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);

    cmd = DRV_IOW(DsQMgrNetBaseGrpWfqWeight_t, DRV_ENTRY_FLAG);
    for (channel_id = 0; channel_id < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); channel_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &baseGrp_wfq_weight));
    }

    /*config queue weight of extend group*/
    SetDsQMgrExtGrpWfqWeight(V, que_0_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_1_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_2_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_3_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_4_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_5_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_6_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrExtGrpWfqWeight(V, que_7_weight_f, &extGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);

    cmd = DRV_IOW(DsQMgrExtGrpWfqWeight_t, DRV_ENTRY_FLAG);
    for (group_id = 0; group_id < MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); group_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &extGrp_wfq_weight));
    }

    /*config chan node weight*/
    SetDsQMgrNetGrpSchWeight(V, node_0_weight_f, &netGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetGrpSchWeight(V, node_1_weight_f, &netGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetGrpSchWeight(V, node_2_weight_f, &netGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);
    SetDsQMgrNetGrpSchWeight(V, node_3_weight_f, &netGrp_wfq_weight, SYS_WFQ_DEFAULT_WEIGHT);

    cmd = DRV_IOW(DsQMgrNetGrpSchWeight_t, DRV_ENTRY_FLAG);
    for (group_id = 0; group_id < MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM) + MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); group_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &netGrp_wfq_weight));
    }

    /*config the mapping of base queue¡¢base group and chan */
    SetDsQMgrBaseSchMap(V, que_0_grpNodeSel_f, &base_sch_map, 0);
    SetDsQMgrBaseSchMap(V, que_1_grpNodeSel_f, &base_sch_map, 1);
    SetDsQMgrBaseSchMap(V, que_2_grpNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, que_3_grpNodeSel_f, &base_sch_map, 3);
    SetDsQMgrBaseSchMap(V, que_4_grpNodeSel_f, &base_sch_map, 4);
    SetDsQMgrBaseSchMap(V, que_5_grpNodeSel_f, &base_sch_map, 5);
    SetDsQMgrBaseSchMap(V, que_6_grpNodeSel_f, &base_sch_map, 6);
    SetDsQMgrBaseSchMap(V, que_7_grpNodeSel_f, &base_sch_map, 7);

    SetDsQMgrBaseSchMap(V, grpNodeSp_chanNodeSel_f, &base_sch_map, 3);
    SetDsQMgrBaseSchMap(V, grpNode_0_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_1_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_2_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_3_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_4_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_5_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_6_chanNodeSel_f, &base_sch_map, 2);
    SetDsQMgrBaseSchMap(V, grpNode_7_chanNodeSel_f, &base_sch_map, 2);

    cmd = DRV_IOW(DsQMgrBaseSchMap_t, DRV_ENTRY_FLAG);
    for (channel_id = 0; channel_id < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); channel_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &base_sch_map));
    }

    /*config the mapping of ext queue¡¢ext group and chan */
    SetDsQMgrExtSchMap(V, que_0_grpNodeSel_f, &ext_sch_map, 0);
    SetDsQMgrExtSchMap(V, que_1_grpNodeSel_f, &ext_sch_map, 1);
    SetDsQMgrExtSchMap(V, que_2_grpNodeSel_f, &ext_sch_map, 2);
    SetDsQMgrExtSchMap(V, que_3_grpNodeSel_f, &ext_sch_map, 3);
    SetDsQMgrExtSchMap(V, que_4_grpNodeSel_f, &ext_sch_map, 4);
    SetDsQMgrExtSchMap(V, que_5_grpNodeSel_f, &ext_sch_map, 5);
    SetDsQMgrExtSchMap(V, que_6_grpNodeSel_f, &ext_sch_map, 6);
    SetDsQMgrExtSchMap(V, que_7_grpNodeSel_f, &ext_sch_map, 7);
    SetDsQMgrExtSchMap(V, que_0_meterSel_f, &ext_sch_map, 0);
    SetDsQMgrExtSchMap(V, que_1_meterSel_f, &ext_sch_map, 1);
    SetDsQMgrExtSchMap(V, que_2_meterSel_f, &ext_sch_map, 2);
    SetDsQMgrExtSchMap(V, que_3_meterSel_f, &ext_sch_map, 3);
    SetDsQMgrExtSchMap(V, que_4_meterSel_f, &ext_sch_map, 4);
    SetDsQMgrExtSchMap(V, que_5_meterSel_f, &ext_sch_map, 5);
    SetDsQMgrExtSchMap(V, que_6_meterSel_f, &ext_sch_map, 6);
    SetDsQMgrExtSchMap(V, que_7_meterSel_f, &ext_sch_map, 7);
    SetDsQMgrExtSchMap(V, grpNode_0_chanNodeSel_f, &ext_sch_map, 2);
    SetDsQMgrExtSchMap(V, grpNode_1_chanNodeSel_f, &ext_sch_map, 2);
    SetDsQMgrExtSchMap(V, grpNode_2_chanNodeSel_f, &ext_sch_map, 2);
    SetDsQMgrExtSchMap(V, grpNode_3_chanNodeSel_f, &ext_sch_map, 2);
    if (DRV_IS_DUET2(lchip))
    {
        SetDsQMgrExtSchMap(V, grpNode_4_chanNodeSel_f, &ext_sch_map, 3);
    }
    else
    {
        SetDsQMgrExtSchMap(V, grpNode_4_chanNodeSel_f, &ext_sch_map, 2);
        SetDsQMgrExtSchMap(V, grpNode_5_chanNodeSel_f, &ext_sch_map, 2);
        SetDsQMgrExtSchMap(V, grpNode_6_chanNodeSel_f, &ext_sch_map, 2);
        SetDsQMgrExtSchMap(V, grpNode_7_chanNodeSel_f, &ext_sch_map, 2);
        SetDsQMgrExtSchMap(V, grpNode_8_chanNodeSel_f, &ext_sch_map, 3);
    }

    cmd = DRV_IOW(DsQMgrExtSchMap_t, DRV_ENTRY_FLAG);
    for (group_id = 0; group_id < MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); group_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &ext_sch_map));
    }

     /*SetDsQMgrDmaSchMap(V, chanNodeSel_f, &dma_sch_map, 3);*/

     /*field_val = 3; // bit[2:0], bit2:Mcast; bit1:UcastHigh; bit0:UcastLow, set 1 means use SP*/
     /*cmd = DRV_IOW(MetFifoWeightCfg_t, MetFifoWeightCfg_cfgMsgSpBitmap_f);*/
     /*CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));*/

    return CTC_E_NONE;
}

int32
sys_usw_qos_sched_dump_status(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------Scheduler-------------------------\n");

    cmd = DRV_IOR(DsQMgrNetSchPpsCfg_t, DsQMgrNetSchPpsCfg_ppsMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    if (field_val)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %s\n","Schedule Mode", "WRR");
    }
    else
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s: %s\n","Schedule Mode", "WDRR");
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

/**
 @brief Queue scheduler initialization.
*/
int32
sys_usw_queue_sch_init(uint8 lchip)
{
    CTC_ERROR_RETURN(_sys_usw_queue_sch_init_wfq_sched(lchip));

    return CTC_E_NONE;
}

int32
sys_usw_queue_sch_deinit(uint8 lchip)
{
    return CTC_E_NONE;
}

