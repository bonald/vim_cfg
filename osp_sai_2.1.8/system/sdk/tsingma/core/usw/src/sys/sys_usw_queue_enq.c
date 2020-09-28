/**
 @file sys_usw_queue_enq.c

 @date 2010-01-13

 @version v2.0

*/

/****************************************************************************
 *  Default Queue Mode
 *
 *           type          port id    QSelType     queue number    Enqueue Param
 *     ----------------  ----------   ---------   ------------    --------------
 *      network egress     0 - 47      0            16 * 48       Based on Channel Id
 *      drop port          48          0            16            Based on Channel Id
 *      RX OAM             55          0            16            Based on RxOamType
 *      Reserved port      56 - 63     0            16*8          Based on Channel Id
        exception to cpu   49 (DMA)    1            128           Based on SubQueId
        exception to cpu   50 (ETH)    1            16            Based on SubQueId(share network egress queue)
*********************************************************************************/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_vector.h"
#include "ctc_linklist.h"
#include "ctc_qos.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_dma.h"

#include "sys_usw_qos.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_port.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_dmps.h"
#include "sys_usw_stats.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/

#define SYS_QUEUE_HASH_MAX   256
#define SYS_QUEUE_HASH_BLOCK_SIZE 8
#define SYS_QUEUE_HASH_BLOCK_NUM (SYS_QUEUE_HASH_MAX + (SYS_QUEUE_HASH_BLOCK_SIZE - 1) / SYS_QUEUE_HASH_BLOCK_SIZE)

#define SYS_SERVICE_HASH_MAX   512
#define SYS_SERVICE_HASH_BLOCK_SIZE 8
#define SYS_SERVICE_HASH_BLOCK_NUM (SYS_SERVICE_HASH_MAX / SYS_SERVICE_HASH_BLOCK_SIZE)

#define SYS_QOS_QUEUE_ENQ_PRIORITY_MAX      15
#define SYS_QOS_QUEUE_ENQ_COLOR_MIN     (CTC_QOS_COLOR_RED)
#define SYS_QOS_QUEUE_ENQ_COLOR_MAX     (MAX_CTC_QOS_COLOR - 1)
#define SYS_QOS_QUEUE_ENQ_QUEUE_SELECT_MAX  7
#define SYS_QOS_QUEUE_ENQ_ROP_PRECEDENCE_MAX    3


#define SYS_QOS_RX_DMA_CHANNEL(excp_group) {\
   if (p_usw_queue_master[lchip]->max_dma_rx_num == 4)\
   {\
      if (excp_group < 4) \
  	  {\
  	     channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);\
  	  }\
      else if (excp_group < 8)\
      {\
          channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1);\
      }\
      else if (excp_group < 12)\
      {\
         channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2);\
      }\
      else\
      {\
         channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3);\
      }\
   }\
   else if (p_usw_queue_master[lchip]->max_dma_rx_num == 2)\
   {\
      if (excp_group < 8) \
  	  {\
  	     channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);\
  	  }\
      else\
      {\
          channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1);\
      }\
   }\
   else if (p_usw_queue_master[lchip]->max_dma_rx_num == 1)\
   {\
     channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);\
   }  \
}

sys_queue_master_t* p_usw_queue_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
uint32
_sys_usw_qos_unbind_service_dstport(uint8 lchip, uint32 service_id, uint16 dest_port, uint16 logic_port, uint8 dir);

/****************************************************************************
 *
 * Function
 *
 ****************************************************************************/
uint32
sys_usw_logicport_service_hash_make(sys_qos_logicport_service_t* p_node)
{
    uint32 data[1] = {0};
    uint32 length = 0;
    data[0] = p_node->logic_src_port;
    length = sizeof(uint16);
    return ctc_hash_caculate(length, (uint8*)data);
}
bool
sys_usw_logicport_service_hash_cmp(sys_qos_logicport_service_t* p_node0,sys_qos_logicport_service_t* p_node1)
{
    if (p_node0->logic_src_port == p_node1->logic_src_port)
    {
        return TRUE;
    }
    return FALSE;
}
sys_qos_logicport_service_t*
sys_usw_logicport_service_lookup(uint8 lchip, sys_qos_logicport_service_t* p_node)
{
    return ctc_hash_lookup(p_usw_queue_master[lchip]->p_logicport_service_hash, p_node);
}
sys_qos_logicport_service_t*
sys_usw_logicport_service_add(uint8 lchip, sys_qos_logicport_service_t* p_node)
{


    return ctc_hash_insert(p_usw_queue_master[lchip]->p_logicport_service_hash, p_node);
}
sys_qos_logicport_service_t*
sys_usw_logicport_service_remove(uint8 lchip, sys_qos_logicport_service_t* p_node)
{
    return ctc_hash_remove(p_usw_queue_master[lchip]->p_logicport_service_hash, p_node);
}
uint32
sys_usw_service_hash_make(sys_service_node_t* p_node)
{
    uint32 data[1] = {0};
    uint32 length = 0;
    data[0] = p_node->service_id;
    length = sizeof(uint32)*1;
    return ctc_hash_caculate(length, (uint8*)data);
}
bool
sys_usw_service_hash_cmp(sys_service_node_t* p_node0,sys_service_node_t* p_node1)
{
    if (p_node0->service_id == p_node1->service_id)
    {
        return TRUE;
    }
    return FALSE;
}
sys_service_node_t*
sys_usw_service_lookup(uint8 lchip, sys_service_node_t* p_node)
{
    return ctc_hash_lookup(p_usw_queue_master[lchip]->p_service_hash, p_node);
}
sys_service_node_t*
sys_usw_service_add(uint8 lchip, sys_service_node_t* p_node)
{
    return ctc_hash_insert(p_usw_queue_master[lchip]->p_service_hash, p_node);
}
sys_service_node_t*
sys_usw_service_remove(uint8 lchip, sys_service_node_t* p_node)
{
    return ctc_hash_remove(p_usw_queue_master[lchip]->p_service_hash, p_node);
}
STATIC int32
_sys_usw_queue_vec_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_hash_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_hash_free_list_node_data(void* node_data, void* user_data)
{
    sys_service_node_t* p_service = (sys_service_node_t*)node_data;
    ctc_slistnode_t *node, *next_node;
    sys_qos_destport_t* p_dest_port_node = NULL;
    sys_qos_logicport_t* p_logic_port_node = NULL;

    CTC_SLIST_LOOP_DEL(p_service->p_logic_port_list, node,next_node)
    {
        p_logic_port_node = _ctc_container_of(node, sys_qos_logicport_t, head);
        mem_free(p_logic_port_node);
    }

    CTC_SLIST_LOOP_DEL(p_service->p_dest_port_list, node,next_node)
    {
        p_dest_port_node = _ctc_container_of(node, sys_qos_destport_t, head);
        mem_free(p_dest_port_node);
    }

    ctc_slist_free(p_service->p_logic_port_list);
    ctc_slist_free(p_service->p_dest_port_list);

    mem_free(node_data);

    return CTC_E_NONE;
}


uint32
sys_usw_port_extender_hash_make(sys_extend_que_node_t* p_node)
{
    uint8* data = (uint8*)(p_node);
    uint8  length = sizeof(sys_extend_que_node_t) - sizeof(uint32)*2;

    return ctc_hash_caculate(length, data);
}

bool
sys_usw_port_extender_hash_cmp(sys_extend_que_node_t* p_node0,
                                  sys_extend_que_node_t* p_node1)
{
    if (p_node0->lchip == p_node1->lchip &&
        p_node0->lport == p_node1->lport &&
        p_node0->channel == p_node1->channel &&
        p_node0->type == p_node1->type &&
        p_node0->service_id == p_node1->service_id &&
        p_node0->logic_src_port == p_node1->logic_src_port)
    {
        return TRUE;
    }

    return FALSE;
}

sys_extend_que_node_t*
sys_usw_port_extender_lookup(uint8 lchip, sys_extend_que_node_t* p_node)
{
    return ctc_hash_lookup(p_usw_queue_master[lchip]->p_port_extender_hash, p_node);
}

int32
sys_usw_port_extender_add(uint8 lchip, sys_extend_que_node_t* p_node)
{
    if (NULL == ctc_hash_insert(p_usw_queue_master[lchip]->p_port_extender_hash, p_node))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_extender_remove(uint8 lchip, sys_extend_que_node_t* p_node)
{
    ctc_hash_remove(p_usw_queue_master[lchip]->p_port_extender_hash, p_node);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_create_service(uint8 lchip, uint32 service_id)
{
    CTC_MIN_VALUE_CHECK(service_id, 1);
    if(p_usw_queue_master[lchip]->service_queue_mode == 0 || p_usw_queue_master[lchip]->service_queue_mode == 2)
    {
        sys_service_node_t* p_service = NULL;
        sys_service_node_t service;

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        sal_memset(&service, 0, sizeof(service));
        service.service_id = service_id;
        p_service = sys_usw_service_lookup(lchip, &service);
        if (NULL != p_service)
        {
            return CTC_E_ENTRY_EXIST;
        }
        p_service =  mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_service_node_t));
        if (NULL == p_service)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_service, 0, sizeof(sys_service_node_t));
        p_service->service_id= service_id;
        p_service->p_logic_port_list = ctc_slist_new();
        if (p_service->p_logic_port_list == NULL)
        {
            mem_free(p_service);
            return CTC_E_NO_MEMORY;
        }
        p_service->p_dest_port_list = ctc_slist_new();
        if (p_service->p_dest_port_list == NULL)
        {
            ctc_slist_free(p_service->p_logic_port_list);
            mem_free(p_service);
            return CTC_E_NO_MEMORY;
        }
        sys_usw_service_add(lchip, p_service);
    }

    return CTC_E_NONE;

}

int32
_sys_usw_qos_destroy_service(uint8 lchip, uint32 service_id)
{
    CTC_MIN_VALUE_CHECK(service_id, 1);
    if(p_usw_queue_master[lchip]->service_queue_mode == 0 || p_usw_queue_master[lchip]->service_queue_mode == 2)
    {
        uint8 gchip = 0;
        uint16 dest_port = 0;
        sys_service_node_t* p_service = NULL;
        sys_service_node_t service;
        sys_qos_destport_t *p_dest_port_node = NULL;
        ctc_slistnode_t *ctc_slistnode = NULL;
        ctc_slistnode_t *ctc_slistnode_new = NULL;

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        sal_memset(&service, 0, sizeof(service));
        service.service_id = service_id;
        p_service = sys_usw_service_lookup(lchip, &service);
        if (NULL == p_service)
        {
            return CTC_E_NOT_EXIST;
        }
        /*1.check logic port is NULL*/
        if(!CTC_LIST_ISEMPTY(p_service->p_logic_port_list))
        {
            return CTC_E_INVALID_PARAM;
        }
        /*2.unbind all destport queue*/
        CTC_SLIST_LOOP_DEL(p_service->p_dest_port_list, ctc_slistnode, ctc_slistnode_new)
        {
            p_dest_port_node = _ctc_container_of(ctc_slistnode, sys_qos_destport_t, head);
            dest_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip,p_dest_port_node->lport);
            _sys_usw_qos_unbind_service_dstport(p_dest_port_node->lchip,service_id,dest_port,0,CTC_INGRESS);
        }
        sys_usw_service_remove(lchip, p_service);

        ctc_slist_free(p_service->p_logic_port_list);
        ctc_slist_free(p_service->p_dest_port_list);

        mem_free(p_service);
    }
    return CTC_E_NONE;
}

int32 sys_usw_qos_get_mux_port_enable(uint8 lchip, uint32 gport, uint8* enable)
{
    uint8 channel = 0xFF;
    uint32 cmd = 0;
    uint32 field_val =0;

    _sys_usw_get_channel_by_port(lchip, gport, &channel);
    if (channel >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &field_val));
    /*SYS_STK_MUX_TYPE_HDR_REGULAR_PORT     = 0,  4'b0000: Regular port(No MUX)
      SYS_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL   = 6,   4'b0110: BridgeHeader without tunnel
      #define MUXTYPE_MUXDEMUX                         "1"
      #define MUXTYPE_EVB                              "2"
      #define MUXTYPE_CB_DOWNLINK                      "3"
      #define MUXTYPE_PE_DOWNLINK_WITH_CASCADE_PORT    "4"
      #define MUXTYPE_PE_UPLINK                        "5"*/
    if (field_val >= 5 || field_val == 0)
    {
        *enable = 0;
    }
    else
    {
        *enable = !!p_usw_queue_master[lchip]->queue_num_per_extend_port;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_get_stacking_enable(uint8 lchip, uint8 channel, uint8* enable)
{
    uint32 cmd                     = 0;
    uint32 packet_header_en[2]     = {0};
    EpeHdrAdjustChanCtl_m    epe_hdr_adjust_chan_ctl;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (channel >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        return CTC_E_NONE;
    }
    sal_memset(&epe_hdr_adjust_chan_ctl, 0, sizeof(epe_hdr_adjust_chan_ctl));
    cmd = DRV_IOR(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl));
    GetEpeHdrAdjustChanCtl(A, packetHeaderEn_f, &epe_hdr_adjust_chan_ctl, packet_header_en);

    *enable = CTC_IS_BIT_SET(packet_header_en[channel >> 5], channel & 0x1F) && SYS_C2C_QUEUE_MODE(lchip);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_get_ext_queue_enable(uint8 lchip, uint32 gport, uint8* stacking_queue_en,
                                                     uint8* bpe_en, uint32* dot1ae_en, uint8* ext_que_en)
{
    uint8 channel = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    _sys_usw_get_channel_by_port(lchip, gport, &channel);
    _sys_usw_queue_get_stacking_enable(lchip, channel, stacking_queue_en);
    sys_usw_qos_get_mux_port_enable(lchip, gport, bpe_en);
    /*sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_DOT1AE_EN, CTC_EGRESS, dot1ae_en);*/

    *ext_que_en = *stacking_queue_en || *bpe_en || *dot1ae_en;

    return CTC_E_NONE;
}

/*
 get ext queue id
*/
int32
_sys_usw_queue_get_ext_queue_id(uint8 lchip, ctc_qos_queue_id_t* p_queue,
                                  uint16* queue_id)
{
    uint8 channel = 0;
    uint16 lport = 0;
    uint32 dot1ae_en = 0;
    uint8 dot1ae_enq_chan = 0;
    uint8 stacking_queue_en = 0;
    uint8 bpe_en = 0;
    uint8 ext_que_en = 0;
    uint16 group_id = 0;
    uint8 type = 0;
    sys_extend_que_node_t extend_que_node;
    sys_extend_que_node_t* p_extend_que_node = NULL;
    sal_memset(&extend_que_node, 0, sizeof(sys_extend_que_node_t));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    _sys_usw_get_channel_by_port(lchip, p_queue->gport, &channel);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_queue->gport, lchip, lport);
    _sys_usw_queue_get_ext_queue_enable(lchip, p_queue->gport, &stacking_queue_en, &bpe_en, &dot1ae_en, &ext_que_en);
    /*CTC_ERROR_RETURN(sys_usw_datapath_get_dot1ae_enq_chan(lchip, &dot1ae_enq_chan));
    */
    if (dot1ae_en)
    {
        channel = dot1ae_enq_chan;
    }

    if (stacking_queue_en)
    {
        type = SYS_EXTEND_QUE_TYPE_C2C;
    }
    else
    {
        type = SYS_EXTEND_QUE_TYPE_BPE;
        extend_que_node.lport = lport;
    }

    extend_que_node.type = type;
    extend_que_node.lchip = lchip;
    extend_que_node.channel = channel;
    p_extend_que_node = sys_usw_port_extender_lookup(lchip, &extend_que_node);
    if (NULL == p_extend_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    group_id = p_extend_que_node->group_id;

    if (bpe_en || dot1ae_en)
    {
        CTC_MAX_VALUE_CHECK(p_queue->queue_id, p_usw_queue_master[lchip]->queue_num_per_extend_port - 1);
        *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT) + (group_id * MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)) + p_queue->queue_id;
    }
    else if (stacking_queue_en) /* 8(uc+mc)+8(c2c)*/
    {
        CTC_MAX_VALUE_CHECK(p_queue->queue_id, p_usw_queue_master[lchip]->queue_num_per_chanel + SYS_C2C_QUEUE_NUM - 1);
        if (p_queue->queue_id >= 8)
        {
            *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT) + (group_id * MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)) + (p_queue->queue_id % 8);
        }
        else
        {
            *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL) + (channel * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM)) + p_queue->queue_id;
        }

    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "channel:%d, lport:%d, queue_id:%d\n", channel, lport, *queue_id);

    return CTC_E_NONE;
}

/*
 get queue id by queue type
*/
int32
sys_usw_queue_get_queue_id(uint8 lchip, ctc_qos_queue_id_t* p_queue,
                                  uint16* queue_id)
{
    uint8 channel = 0;
    uint16 lport = 0;
    uint32 dot1ae_en = 0;
    uint8 stacking_queue_en = 0;
    uint8 bpe_en = 0;
    uint16 group_id = 0;
    uint8 ext_que_en = 0;
    sys_extend_que_node_t port_extender_node;
    sys_extend_que_node_t* p_port_extender_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&port_extender_node, 0, sizeof(port_extender_node));

    switch (p_queue->queue_type)
    {
    case CTC_QUEUE_TYPE_NETWORK_EGRESS:
    case CTC_QUEUE_TYPE_INTERNAL_PORT:
    {
        CTC_ERROR_RETURN(_sys_usw_get_channel_by_port(lchip, p_queue->gport, &channel));
        _sys_usw_queue_get_ext_queue_enable(lchip, p_queue->gport, &stacking_queue_en, &bpe_en, &dot1ae_en, &ext_que_en);

        if (ext_que_en)
        {
            CTC_ERROR_RETURN(_sys_usw_queue_get_ext_queue_id(lchip, p_queue, queue_id));
        }
        else if (SYS_C2C_QUEUE_MODE(lchip)) /* 8(uc+mc)*/
        {
            CTC_MAX_VALUE_CHECK(p_queue->queue_id, p_usw_queue_master[lchip]->queue_num_per_chanel - 1);
            *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL) + (channel * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM)) + p_queue->queue_id;
        }
        else
        {
            CTC_MAX_VALUE_CHECK(p_queue->queue_id, p_usw_queue_master[lchip]->queue_num_per_chanel - 1);
            if(p_usw_queue_master[lchip]->enq_mode == 2 ||(p_usw_queue_master[lchip]->enq_mode == 1 && DRV_IS_TSINGMA(lchip)))
            {
                if (8 == p_queue->queue_id) /*mcast*/
                {
                    *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC) + (channel * MCHIP_CAP(SYS_CAP_QOS_MISC_QUEUE_NUM)) + 1;
                }
                else if (9 == p_queue->queue_id)/*log*/
                {
                    *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC) + (channel * MCHIP_CAP(SYS_CAP_QOS_MISC_QUEUE_NUM));
                }
                else
                {
                    *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL) + (channel * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM)) + p_queue->queue_id;
                }
            }
            else if(p_usw_queue_master[lchip]->enq_mode == 1 && DRV_IS_DUET2(lchip) )
            {
                if((MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_CHAN)+MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)) == p_queue->queue_id)/*log*/
                {
                    *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC) + (channel * MCHIP_CAP(SYS_CAP_QOS_MISC_QUEUE_NUM));
                }
                else if(p_queue->queue_id < (MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_CHAN)+MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP))
                        && p_queue->queue_id > (MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_CHAN)-1))
                {
                    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_queue->gport, lchip, lport);
                    port_extender_node.type = SYS_EXTEND_QUE_TYPE_MCAST;
                    port_extender_node.lchip = lchip;
                    port_extender_node.lport = lport;
                    p_port_extender_node = sys_usw_port_extender_lookup(lchip, &port_extender_node);
                    if (NULL == p_port_extender_node)
                    {
                        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
                        return CTC_E_NOT_EXIST;
                    }
                    group_id = p_port_extender_node->group_id;
                    *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)+ (group_id * MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP))
                                + p_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_GRP);
                }
                else
                {
                    *queue_id  =  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL) + (channel * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM)) + p_queue->queue_id;
                }
            }
        }

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "channel:%d, queue_id:%d\n", channel, *queue_id);
    }
    break;
    case CTC_QUEUE_TYPE_EXCP_CPU:
    {
        sys_cpu_reason_t  *p_cpu_reason;
        if (p_queue->cpu_reason >= CTC_PKT_CPU_REASON_MAX_COUNT)
        {
            return CTC_E_INVALID_PARAM;
        }

        p_cpu_reason = &p_usw_queue_master[lchip]->cpu_reason[p_queue->cpu_reason];
        if (p_cpu_reason->sub_queue_id == CTC_MAX_UINT16_VALUE)
        {
            return CTC_E_NOT_READY;
        }
        if (p_queue->cpu_reason == CTC_PKT_CPU_REASON_C2C_PKT)
        {
            CTC_MAX_VALUE_CHECK(p_queue->queue_id, MCHIP_CAP(SYS_CAP_QOS_REASON_C2C_MAX_QUEUE_NUM) - 1);
        }
        else
        {
            CTC_MAX_VALUE_CHECK(p_queue->queue_id, 0);
        }

        if (p_cpu_reason->dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_CPU)
        {
            *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP) + p_cpu_reason->sub_queue_id + p_queue->queue_id;
        }
        else if (p_cpu_reason->dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH)
        {
            *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL)
                        + p_usw_queue_master[lchip]->cpu_eth_chan_id * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM)
                        + p_queue->queue_id;
        }
        else
        {
            /*TBD*/
            return CTC_E_INVALID_PARAM;
        }
    }
    break;
    case CTC_QUEUE_TYPE_ILOOP:
        CTC_MAX_VALUE_CHECK(p_queue->queue_id, MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_CHAN) - 1);

        *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL)
                    + (MCHIP_CAP(SYS_CAP_CHANID_ILOOP) * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM))
                    + p_queue->queue_id;
        break;

    case CTC_QUEUE_TYPE_ELOOP:
        *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL)
                    + (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM))
                    + p_queue->queue_id;
        break;
    case CTC_QUEUE_TYPE_OAM:
        CTC_MAX_VALUE_CHECK(p_queue->queue_id, MCHIP_CAP(SYS_CAP_QOS_OAM_QUEUE_NUM) - 1);
        *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL)
                    + (MCHIP_CAP(SYS_CAP_CHANID_OAM) * MCHIP_CAP(SYS_CAP_QOS_NORMAL_QUEUE_NUM))
                    + p_queue->queue_id;
        break;
    case CTC_QUEUE_TYPE_SERVICE_INGRESS:
        {
            sys_extend_que_node_t service_que_node;
            sys_extend_que_node_t* p_service_que_node = NULL;
            sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));

            SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_queue->gport, lchip, lport);
            service_que_node.type = SYS_EXTEND_QUE_TYPE_SERVICE;
            service_que_node.service_id = p_queue->service_id;
            service_que_node.lchip = lchip;
            service_que_node.lport = lport;
            p_service_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
            if (NULL == p_service_que_node)
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
                return CTC_E_NOT_EXIST;
            }
            group_id = p_service_que_node->group_id;

            CTC_MAX_VALUE_CHECK(p_queue->queue_id, MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP) - 1);
            *queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)
                        + (group_id * MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP))
                        + p_queue->queue_id;
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

uint8
_sys_usw_queue_get_enq_mode(uint8 lchip)
{
    return p_usw_queue_master[lchip]->enq_mode;
}

uint8
_sys_usw_queue_get_service_queue_mode(uint8 lchip)
{
    return p_usw_queue_master[lchip]->service_queue_mode;
}

int32
sys_usw_queue_get_queue_type_by_queue_id(uint8 lchip, uint16 queue_id, uint8 *queue_type)
{
    CTC_PTR_VALID_CHECK(queue_type);

    if (SYS_IS_CPU_QUEUE(queue_id))
    {
        *queue_type = SYS_QUEUE_TYPE_EXCP;
    }
    else if (SYS_IS_EXT_QUEUE(queue_id))
    {
        *queue_type = SYS_QUEUE_TYPE_EXTEND;
    }
    else
    {
        *queue_type = SYS_QUEUE_TYPE_NORMAL;
    }

    return CTC_E_NONE;
}

int32
sys_usw_get_channel_by_queue_id(uint8 lchip, uint16 queue_id, uint8 *channel)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint16 group_id = 0;

    if (SYS_IS_NETWORK_BASE_QUEUE(queue_id))
    {
        *channel = queue_id / 8;
    }
    else if (SYS_IS_MISC_CHANNEL_QUEUE(queue_id))
    {
        *channel = queue_id / 8;
    }
    else if (SYS_IS_CPU_QUEUE(queue_id))
    {
        if (p_usw_queue_master[lchip]->max_dma_rx_num == 4)
        {
            *channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0) + (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP)) / 32;
        }
        else
        {
            *channel = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0) + (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP)) / 64;
        }
    }
    else if (SYS_IS_NETWORK_CTL_QUEUE(queue_id))
    {
        *channel = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)) / MCHIP_CAP(SYS_CAP_QOS_MISC_QUEUE_NUM) ;
    }
    else if (SYS_IS_EXT_QUEUE(queue_id))
    {
        group_id = (queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) / MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        cmd = DRV_IOR(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        *channel = field_val;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_get_channel_by_port(uint8 lchip, uint32 gport, uint8 *channel)
{
    uint16 lport = 0;
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint8 pmac_channel[24] = {4,5,6,7,16,17,18,19,8,9,10,11,24,25,26,27,40,41,42,43,56,57,58,59};
    uint8 index = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    if(!DRV_IS_DUET2(lchip))
    {
        sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_XPIPE_EN, &field_val);
        if(field_val)
        {
            for(index = 0; index < 24; index ++)
            {
                if(pmac_channel[index] == lport)
                {
                    *channel = lport;
                    return CTC_E_NONE;
                }
            }
        }
    }
    cmd = DRV_IOR(DsQWriteDestPortChannelMap_t, DsQWriteDestPortChannelMap_channelId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
    *channel = field_val;

    return CTC_E_NONE;
}

int32
_sys_usw_add_port_to_channel(uint8 lchip, uint32 lport, uint8 channel)
{
    ds_t ds;
    uint32 cmd = 0;
    uint32 field_val = channel;

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(DsQWriteDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    SetDsQWriteDestPortChannelMap(V, channelId_f, ds, channel);
    cmd = DRV_IOW(DsQWriteDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    cmd = DRV_IOR(DsDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    SetDsDestPortChannelMap(V, channelId_f, ds, channel);
    cmd = DRV_IOW(DsDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    cmd = DRV_IOW(DsLinkAggregationPortChannelMap_t, DsLinkAggregationPortChannelMap_channelId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    return CTC_E_NONE;
}

int32
_sys_usw_remove_port_from_channel(uint8 lchip, uint32 lport, uint8 channel)
{
    ds_t   ds;
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8 gchip = 0;
    uint32 gport = 0;

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    channel = SYS_GET_CHANNEL_ID(lchip, gport);
    if(channel >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        channel = MCHIP_CAP(SYS_CAP_CHANID_DROP);
    }

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(DsQWriteDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    SetDsQWriteDestPortChannelMap(V, channelId_f, ds, channel);
    cmd = DRV_IOW(DsQWriteDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    cmd = DRV_IOR(DsDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    SetDsQWriteDestPortChannelMap(V, channelId_f, ds, channel);
    cmd = DRV_IOW(DsDestPortChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, ds));

    field_val = channel;
    cmd = DRV_IOW(DsLinkAggregationPortChannelMap_t, DsLinkAggregationPortChannelMap_channelId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
    return CTC_E_NONE;
}

/**
 @brief set priority color map to queue select and drop_precedence.
*/
int32
sys_usw_set_priority_queue_map(uint8 lchip, ctc_qos_pri_map_t* p_queue_pri_map)
{
    uint32 index = 0;
    uint32 cmd = 0;
    uint8 mcast = 0;
    DsErmPrioScTcMap_m erm_prio_sc_tc_map;
    DsErmColorDpMap_m erm_color_dp_map;

    CTC_MAX_VALUE_CHECK(p_queue_pri_map->priority, SYS_QOS_QUEUE_ENQ_PRIORITY_MAX);
    CTC_VALUE_RANGE_CHECK(p_queue_pri_map->color, SYS_QOS_QUEUE_ENQ_COLOR_MIN, SYS_QOS_QUEUE_ENQ_COLOR_MAX);
    CTC_MAX_VALUE_CHECK(p_queue_pri_map->queue_select, SYS_QOS_QUEUE_ENQ_QUEUE_SELECT_MAX);
    CTC_MAX_VALUE_CHECK(p_queue_pri_map->drop_precedence, SYS_QOS_QUEUE_ENQ_ROP_PRECEDENCE_MAX);

    /* now we only use profile0 */
    for (mcast = 0; mcast < 2; mcast++)
    {
        index = mcast << 4 | p_queue_pri_map->priority;;
        sal_memset(&erm_prio_sc_tc_map, 0, sizeof(DsErmPrioScTcMap_m));
        cmd = DRV_IOR(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
        SetDsErmPrioScTcMap(V, g2_0_mappedTc_f, &erm_prio_sc_tc_map, p_queue_pri_map->queue_select);
        SetDsErmPrioScTcMap(V, g2_2_mappedTc_f, &erm_prio_sc_tc_map, p_queue_pri_map->queue_select);
        cmd = DRV_IOW(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
    }

    sal_memset(&erm_color_dp_map, 0, sizeof(DsErmColorDpMap_m));
    index = p_queue_pri_map->color;
    cmd = DRV_IOR(DsErmColorDpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_color_dp_map));
    SetDsErmColorDpMap(V, g_0_dropPrecedence_f, &erm_color_dp_map, p_queue_pri_map->drop_precedence);
    SetDsErmColorDpMap(V, g_1_dropPrecedence_f, &erm_color_dp_map, p_queue_pri_map->drop_precedence);
    SetDsErmColorDpMap(V, g_2_dropPrecedence_f, &erm_color_dp_map, p_queue_pri_map->drop_precedence);
    cmd = DRV_IOW(DsErmColorDpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_color_dp_map));

    return CTC_E_NONE;
}

/**
 @brief reserved channel drop init
*/
int32
_sys_usw_rsv_channel_drop_init(uint8 lchip)
{
    uint32 cmd = 0;
    ds_t ds;

    sal_memset(ds, 0, sizeof(ds));

    cmd = DRV_IOR(QWriteRsvChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    SetQWriteRsvChanCtl(V, reservedChannelIdValid_f, ds, 1);
    SetQWriteRsvChanCtl(V, reservedChannelId_f, ds, MCHIP_CAP(SYS_CAP_CHANID_RSV));
    SetQWriteRsvChanCtl(V, g_0_reservedChannelRangeValid_f, ds, 1);
    SetQWriteRsvChanCtl(V, g_0_reservedChannelRangeMax_f, ds, MCHIP_CAP(SYS_CAP_CHANID_DROP));
    SetQWriteRsvChanCtl(V, g_0_reservedChannelRangeMin_f, ds, MCHIP_CAP(SYS_CAP_CHANID_DROP));
    SetQWriteRsvChanCtl(V, g_1_reservedChannelRangeValid_f, ds, 1);
    SetQWriteRsvChanCtl(V, g_1_reservedChannelRangeMax_f, ds, MCHIP_CAP(SYS_CAP_CHANID_RSV));
    SetQWriteRsvChanCtl(V, g_1_reservedChannelRangeMin_f, ds, MCHIP_CAP(SYS_CAP_CHANID_RSV));
    cmd = DRV_IOW(QWriteRsvChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    return CTC_E_NONE;
}

int32
_sys_usw_set_dma_channel_drop_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    uint8 channel_min = 0;
    uint8 channel_max = 0;
    QWriteRsvChanCtl_m qmgr_rsv_channel_range;

    sal_memset(&qmgr_rsv_channel_range, 0, sizeof(qmgr_rsv_channel_range));

    channel_min = enable ? MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0) : MCHIP_CAP(SYS_CAP_CHANID_RSV);
    channel_max = enable ? MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3) : MCHIP_CAP(SYS_CAP_CHANID_RSV);

    cmd = DRV_IOR(QWriteRsvChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qmgr_rsv_channel_range));

    SetQWriteRsvChanCtl(V, g_1_reservedChannelRangeValid_f, &qmgr_rsv_channel_range, 1);
    SetQWriteRsvChanCtl(V, g_1_reservedChannelRangeMax_f, &qmgr_rsv_channel_range, channel_max);
    SetQWriteRsvChanCtl(V, g_1_reservedChannelRangeMin_f, &qmgr_rsv_channel_range, channel_min);
    cmd = DRV_IOW(QWriteRsvChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qmgr_rsv_channel_range));

    return CTC_E_NONE;
}

int32
_sys_usw_qos_set_aqmscan_high_priority_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    value = (enable == 1) ? 0 : 1;
    cmd = DRV_IOW(QMgrEnqMiscCtl_t, QMgrEnqMiscCtl_cfgAqmScanLowPrioEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

int32
sys_usw_queue_set_stats_enable(uint8 lchip, ctc_qos_queue_stats_t* p_stats)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint16 queue_id = 0;
    uint32 stats_offet = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    value = (p_stats->stats_en) ? 1 : 0;
    if(DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_queStatsEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(GlobalStatsUpdateEnCtl_t, GlobalStatsUpdateEnCtl_qMgrGlobalStatsUpdateEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,  &p_stats->queue, &queue_id));
        if (!SYS_IS_EXT_QUEUE(queue_id))
        {
            cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_queStatsEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

            cmd = DRV_IOW(GlobalStatsUpdateEnCtl_t, GlobalStatsUpdateEnCtl_qMgrGlobalStatsUpdateEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            return CTC_E_NONE;
        }
        cmd = DRV_IOR(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &stats_offet));
        if(value == 1 && stats_offet == 0)
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_alloc_statsptr(lchip, CTC_INGRESS, 2, SYS_STATS_TYPE_QUEUE, &stats_offet));
            cmd = DRV_IOW(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &stats_offet));
            value = 0;
            cmd = DRV_IOW(DsErmQueueCfg_t, DsErmQueueCfg_statsMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &value));
        }
        else if(value == 0 && stats_offet != 0)
        {
            CTC_ERROR_RETURN(sys_usw_flow_stats_free_statsptr(lchip, CTC_INGRESS, 2, SYS_STATS_TYPE_QUEUE, &stats_offet));
            value = 0;
            cmd = DRV_IOW(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &value));
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_get_stats_enable(uint8 lchip, uint16 queue_id, uint8* p_enable)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    if (!DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &value));
        cmd = DRV_IOR(QWriteCtl_t, QWriteCtl_queStatsEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1));
        *p_enable = (value && value1) ? 1 : 0;
    }
    else
    {
        cmd = DRV_IOR(QWriteCtl_t, QWriteCtl_queStatsEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        *p_enable = value;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_alloc_ext_group(uint8 lchip, uint32 num, uint32* group_id)
{
    uint32 grp_id = 0;
    sys_usw_opf_t opf;

    /*alloc group id*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_group_id;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, num, &grp_id));

    *group_id = grp_id;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_free_ext_group(uint8 lchip, uint32 num, uint32 group_id)
{
    sys_usw_opf_t opf;

    /*free group id*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_group_id;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, num, group_id));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_alloc_key_index(uint8 lchip, uint32 num, uint32* key_index)
{
    uint32 index = 0;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    /*alloc key index*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_tcam_index;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, num, &index));

    *key_index = index;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_free_key_index(uint8 lchip, uint32 num, uint32 key_index)
{
    sys_usw_opf_t opf;

    /*free key index*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_tcam_index;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, num, key_index));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_add_for_stacking(uint8 lchip, uint32 gport)
{
    int32  ret = 0;
    uint32 group_id = 0;
    uint32 key_index = 0;
    uint32 cmd = 0;
    uint8  chan_id = 0;
    DsQueueMapTcamData_m  QueueMapTcamData;
    DsQueueMapTcamKey_m   QueueMapTcamKey;
    DsQueueMapTcamKey_m   QueueMapTcamMask;
    DsQMgrGrpChanMap_m    grp_chan_map;
    tbl_entry_t           tcam_key;
    sys_extend_que_node_t c2c_que_node;
    sys_extend_que_node_t* p_c2c_que_node = NULL;
    sys_queue_group_node_t *p_sys_group_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&QueueMapTcamData, 0, sizeof(DsQueueMapTcamData_m));
    sal_memset(&QueueMapTcamKey, 0, sizeof(DsQueueMapTcamKey_m));
    sal_memset(&QueueMapTcamMask, 0, sizeof(DsQueueMapTcamKey_m));
    sal_memset(&grp_chan_map, 0, sizeof(DsQMgrGrpChanMap_m));
    sal_memset(&c2c_que_node, 0, sizeof(sys_extend_que_node_t));

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    c2c_que_node.type = SYS_EXTEND_QUE_TYPE_C2C;
    c2c_que_node.lchip = lchip;
    c2c_que_node.channel = chan_id;
    p_c2c_que_node = sys_usw_port_extender_lookup(lchip, &c2c_que_node);
    if (NULL != p_c2c_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
        return CTC_E_EXIST;
    }

    p_c2c_que_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_extend_que_node_t));
    if (NULL == p_c2c_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_c2c_que_node, 0, sizeof(sys_extend_que_node_t));

    ret = _sys_usw_queue_alloc_ext_group(lchip, 1, &group_id);
    if (ret < 0)
    {
        mem_free(p_c2c_que_node);
        return ret;
    }
    CTC_ERROR_GOTO(_sys_usw_queue_alloc_key_index(lchip, 1, &key_index), ret, error0);

    /*write tcam ad*/
    SetDsQueueMapTcamData(V, extGroupId_f, &QueueMapTcamData, group_id );
    cmd = DRV_IOW(DsQueueMapTcamData_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index , cmd, &QueueMapTcamData), ret, error1);

    /*bind extGroup to channel*/
    SetDsQMgrGrpChanMap(V, channelId_f, &grp_chan_map, chan_id);
    cmd = DRV_IOW(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, group_id , cmd, &grp_chan_map), ret, error1);

    SetDsQueueMapTcamKey(V, c2cPacket_f, &QueueMapTcamKey, 1);
    SetDsQueueMapTcamKey(V, c2cPacket_f, &QueueMapTcamMask, 1);
    SetDsQueueMapTcamKey(V, channelId_f, &QueueMapTcamKey, chan_id);
    SetDsQueueMapTcamKey(V, channelId_f, &QueueMapTcamMask, 0x3f);
    cmd = DRV_IOW(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    tcam_key.data_entry = (uint32*)(&QueueMapTcamKey);
    tcam_key.mask_entry = (uint32*)(&QueueMapTcamMask);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &tcam_key), ret, error1);
    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        ret = CTC_E_NOT_EXIST;
        goto error2;
    }
    p_sys_group_node->chan_id  = chan_id;  /*0~63*/

    p_c2c_que_node->lchip = lchip;
    p_c2c_que_node->type = SYS_EXTEND_QUE_TYPE_C2C;
    p_c2c_que_node->channel = chan_id;
    p_c2c_que_node->group_id = group_id;
    p_c2c_que_node->key_index = key_index;
    CTC_ERROR_GOTO(sys_usw_port_extender_add(lchip, p_c2c_que_node), ret, error2);

    /*network base queue*/
    CTC_ERROR_GOTO(sys_usw_queue_sch_set_c2c_group_sched(lchip, (chan_id * 8), 1), ret, error3);

    /*extend queue, 1 ext grp*/
    CTC_ERROR_GOTO(sys_usw_queue_sch_set_c2c_group_sched(lchip,
                   (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT) + group_id*MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)), 3), ret, error3);



    return CTC_E_NONE;

error3:
    c2c_que_node.type = SYS_EXTEND_QUE_TYPE_C2C;
    c2c_que_node.lchip = lchip;
    c2c_que_node.channel = chan_id;
    p_c2c_que_node = sys_usw_port_extender_lookup(lchip, &c2c_que_node);
    if (NULL == p_c2c_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    sys_usw_port_extender_remove(lchip, p_c2c_que_node);
error2:
    cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index , cmd, &cmd));
    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        ret = CTC_E_NOT_EXIST;
    }
    p_sys_group_node->chan_id  = 0xFF;

error1:
    _sys_usw_queue_free_key_index(lchip, 1, key_index);
error0:
    _sys_usw_queue_free_ext_group(lchip, 1, group_id);
    mem_free(p_c2c_que_node);
    return ret;
}

uint32
_sys_usw_qos_set_group_default(uint8 lchip,uint16 group_id)
{
    uint32 cmd = 0;
    uint8  queue_offset = 0;
    uint32 field_id = 0;
    uint32 field_val = 0;
    /*set default group sch class*/
    for(queue_offset = 0; queue_offset < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); queue_offset++)
    {
        /* set default queue weight*/
        field_id = DsQMgrExtGrpWfqWeight_que_0_weight_f + queue_offset;
        field_val = 1;
        cmd = DRV_IOW(DsQMgrExtGrpWfqWeight_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        /* set default group weight*/
        field_id = DsQMgrNetGrpSchWeight_node_0_weight_f + queue_offset;
        field_val = 1;
        cmd = DRV_IOW(DsQMgrNetGrpSchWeight_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id + MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM), cmd, &field_val));
        /* set default group sch*/
        field_id = DsQMgrExtSchMap_grpNode_0_chanNodeSel_f + queue_offset;
        field_val = 2;
        cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        field_val = queue_offset;
        field_id = DsQMgrExtSchMap_que_0_grpNodeSel_f + queue_offset*2;
        cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        field_id = DsQMgrExtSchMap_que_0_meterSel_f + queue_offset*2;
        cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_check_queue_status(uint8 lchip, uint16 que_id)
{
    uint32 depth                   = 0;
    uint32 index                   = 0;
    uint32 cmd = 0;
    /*check queue flush clear*/
    cmd = DRV_IOR(DsErmQueueCnt_t,DsErmQueueCnt_queueCnt_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, que_id, cmd, &depth));
    while (depth)
    {
        sal_task_sleep(20);
        if ((index++) > 1000)
        {
            return CTC_E_INVALID_PARAM;
        }
         cmd = DRV_IOR(DsErmQueueCnt_t,DsErmQueueCnt_queueCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, que_id, cmd, &depth));
    }
    return CTC_E_NONE;
}

uint32
_sys_usw_qos_flush_queue(uint8 lchip,uint16 chan_id,uint16 group_id)
{
    uint32 cmd = 0;
    uint8  queue_offset = 0;
    uint32 tokenThrd = 0;
    uint32 tokenThrdShift = 0;
    uint32 tokenRate = 0;
    uint32 field_val = 0;
    uint32 field_id = 0;
    int32 ret = 0;
    DsQMgrNetChanShpProfile_m net_chan_shp_profile;

    sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));
    /*High group sch class*/
    for(queue_offset = 0; queue_offset < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); queue_offset++)
    {
        field_id = DsQMgrExtSchMap_grpNode_0_chanNodeSel_f + queue_offset;
        field_val = 3;
        cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
    }
    /*Disable chan shaping*/
    cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile),ret,error0);
    tokenThrd = GetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile);
    tokenThrdShift = GetDsQMgrNetChanShpProfile(V, tokenThrdShift_f, &net_chan_shp_profile);
    tokenRate = GetDsQMgrNetChanShpProfile(V, tokenRate_f, &net_chan_shp_profile);
    SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
    SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
    SetDsQMgrNetChanShpProfile(V, tokenRate_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE));
    cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile),ret,error0);
    /*Disable extend group/queue shaping*/
    field_val = 0;
    cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profId_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, group_id, cmd, &field_val),ret,error1);
    for (queue_offset = 0; queue_offset < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); queue_offset++)
    {
        cmd = DRV_IOW(DsQMgrExtQueShpProfId_t, DsQMgrExtQueShpProfId_profId_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, (group_id*MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP) + queue_offset), cmd, &field_val),ret,error1);
        CTC_ERROR_GOTO(_sys_usw_check_queue_status(lchip, (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)+group_id*MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)+ queue_offset)),ret,error1);
    }
    /*Enable chan shaping*/
    cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile),ret,error1);
    SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, tokenThrd);
    SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, tokenThrdShift);
    SetDsQMgrNetChanShpProfile(V, tokenRate_f  , &net_chan_shp_profile, tokenRate);
    cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile),ret,error1);
    /*set default group sch class*/
    CTC_ERROR_GOTO(_sys_usw_qos_set_group_default(lchip, group_id),ret,error0);

    return CTC_E_NONE;
error1:
    cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile);
    SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, tokenThrd);
    SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, tokenThrdShift);
    SetDsQMgrNetChanShpProfile(V, tokenRate_f  , &net_chan_shp_profile, tokenRate);
    cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile);
error0:
    _sys_usw_qos_set_group_default(lchip, group_id);

    return ret;
}
int32
_sys_usw_queue_remove_for_stacking(uint8 lchip, uint32 gport)
{
        uint32 cmd = 0;
    uint8  chan_id = 0;
    DsQMgrGrpChanMap_m    grp_chan_map;
    sys_extend_que_node_t c2c_que_node;
    sys_extend_que_node_t* p_c2c_que_node = NULL;
    sys_queue_group_node_t *p_sys_group_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&grp_chan_map, 0, sizeof(DsQMgrGrpChanMap_m));
    sal_memset(&c2c_que_node, 0, sizeof(sys_extend_que_node_t));

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    c2c_que_node.type = SYS_EXTEND_QUE_TYPE_C2C;
    c2c_que_node.lchip = lchip;
    c2c_que_node.channel = chan_id;
    p_c2c_que_node = sys_usw_port_extender_lookup(lchip, &c2c_que_node);
    if (NULL == p_c2c_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    sys_usw_port_extender_remove(lchip, p_c2c_que_node);

    CTC_ERROR_RETURN(_sys_usw_queue_free_key_index(lchip, 1, p_c2c_que_node->key_index));
    CTC_ERROR_RETURN(_sys_usw_queue_free_ext_group(lchip, 1, p_c2c_que_node->group_id));


    cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_c2c_que_node->key_index , cmd, &cmd));

    SetDsQMgrGrpChanMap(V, channelId_f, &grp_chan_map, 0);
    cmd = DRV_IOW(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_c2c_que_node->group_id, cmd, &grp_chan_map));

    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, p_c2c_que_node->group_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    p_sys_group_node->chan_id  = 0xFF;

    /*network base queue*/
    sys_usw_queue_sch_set_c2c_group_sched(lchip, (chan_id * 8), 2);

    /*extend queue, 1 ext grp*/
    sys_usw_queue_sch_set_c2c_group_sched(lchip, (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT) + p_c2c_que_node->group_id*MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP)), 2);

    mem_free(p_c2c_que_node);

    return CTC_E_NONE;
}

uint32
_sys_usw_qos_write_queue_key(uint8 lchip, uint32 service_id, uint16 dest_port,
                                       uint16 logic_port,uint16 group_id,uint8 type,uint32* p_key_index,uint8 dir)
{
    uint32 key_index = 0;
    tbl_entry_t   tcam_key;
    uint32 dest_map = 0;
    uint8  gchip = 0;
    uint32 cmd = 0;
    int32 ret = 0;
    uint16 lport = 0;
    uint32 nhp_id = 0;
    uint8 xgpon_en = 0;
    sys_nh_info_dsnh_t nhinfo;
    DsQueueMapTcamData_m  QueueMapTcamData;
    DsQueueMapTcamKey_m   QueueMapTcamKey;
    DsQueueMapTcamKey_m   QueueMapTcamMask;

    sal_memset(&QueueMapTcamData, 0, sizeof(DsQueueMapTcamData_m));
    sal_memset(&QueueMapTcamKey, 0, sizeof(DsQueueMapTcamKey_m));
    sal_memset(&QueueMapTcamMask, 0, sizeof(DsQueueMapTcamKey_m));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    CTC_ERROR_RETURN(_sys_usw_queue_alloc_key_index(lchip, 1, &key_index));

    SetDsQueueMapTcamData(V, extGroupId_f, &QueueMapTcamData, group_id);
    cmd = DRV_IOW(DsQueueMapTcamData_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &QueueMapTcamData), ret, error0);

    /*write tcam key*/
    sys_usw_get_gchip_id(lchip, &gchip);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(dest_port, lchip, lport);
    dest_map = SYS_ENCODE_DESTMAP(gchip, lport);
    SetDsQueueMapTcamKey(V, mcastIdValid_f, &QueueMapTcamKey, 0);
    SetDsQueueMapTcamKey(V, mcastIdValid_f, &QueueMapTcamMask, 1);
    SetDsQueueMapTcamKey(V, destChipIdMatch_f, &QueueMapTcamKey, 1);
    SetDsQueueMapTcamKey(V, destChipIdMatch_f, &QueueMapTcamMask, 1);
    SetDsQueueMapTcamKey(V, destMap_f, &QueueMapTcamKey, dest_map);
    SetDsQueueMapTcamKey(V, destMap_f, &QueueMapTcamMask, 0x1ff);
    SetDsQueueMapTcamKey(V, logicSrcPort_f, &QueueMapTcamKey, logic_port);
    SetDsQueueMapTcamKey(V, logicSrcPort_f, &QueueMapTcamMask, type ? 0 : 0xFFFF);
    if(dir == CTC_EGRESS)
    {
        CTC_ERROR_GOTO(sys_usw_l2_get_nhid_by_logic_port(lchip, service_id, &nhp_id), ret, error0);
        CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo(lchip, nhp_id, &nhinfo, 1), ret, error0);
        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        service_id = xgpon_en ? service_id : nhinfo.dsnh_offset;
        SetDsQueueMapTcamKey(V, nexthopPtr_f, &QueueMapTcamKey, service_id);
        SetDsQueueMapTcamKey(V, nexthopPtr_f, &QueueMapTcamMask, 0xffff);
    }
    else
    {
        SetDsQueueMapTcamKey(V, serviceId_f, &QueueMapTcamKey, service_id);
        SetDsQueueMapTcamKey(V, serviceId_f, &QueueMapTcamMask, type ? 0x1ff : 0);

    }

    cmd = DRV_IOW(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    tcam_key.data_entry = (uint32*)(&QueueMapTcamKey);
    tcam_key.mask_entry = (uint32*)(&QueueMapTcamMask);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &tcam_key), ret, error0);
    *p_key_index = key_index;

    return CTC_E_NONE;

error0:
    _sys_usw_queue_free_key_index(lchip, 1, key_index);

    return ret;
}

uint32
_sys_usw_qos_bind_service_dstport(uint8 lchip, uint32 service_id, uint16 dest_port,uint16 logic_port,uint8 dir)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    int32  ret = 0;
    uint32 group_id = 0;
    uint32 key_index = 0;
    uint32 cmd = 0;
    uint32  chan_id = 0;
    uint16 lport = 0;
    uint8  gchip = 0;
    uint8  first_add = 0;
    uint32 logic_dst_port = 0;
    uint16 service_id_tmp = 0;
    uint8 xgpon_en = 0;
    sys_service_node_t* p_service = NULL;
    sys_service_node_t service;
    sys_qos_destport_t* p_dest_port_node = NULL;
    sys_qos_logicport_t* p_logic_port_node = NULL;
    DsQMgrGrpChanMap_m    grp_chan_map;
    sys_extend_que_node_t service_que_node;
    sys_extend_que_node_t* p_service_que_node, *p_logice_que_node ;
    sys_queue_group_node_t *p_sys_group_node = NULL;
    sys_extend_que_node_t* p_service_que_logic_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&grp_chan_map, 0, sizeof(DsQMgrGrpChanMap_m));
    sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));
    sal_memset(&service, 0, sizeof(sys_service_node_t));

    CTC_MIN_VALUE_CHECK(service_id, 1);
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(dest_port, lchip, lport);
    chan_id = SYS_GET_CHANNEL_ID(lchip, dest_port);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }
    if(dir == CTC_INGRESS && p_usw_queue_master[lchip]->service_queue_mode == 1)
    {
        CTC_VALUE_RANGE_CHECK(service_id, 1, MCHIP_CAP(SYS_CAP_SERVICE_ID_NUM) - 1);
    }

    service_que_node.type = SYS_EXTEND_QUE_TYPE_SERVICE;
    service_que_node.service_id = service_id;
    service_que_node.lchip = lchip;
    if(dir == CTC_EGRESS)
    {
        CTC_MIN_VALUE_CHECK(service_id, 1);
        if(p_usw_queue_master[lchip]->service_queue_mode != 2 && xgpon_en != 1)
        {
            return CTC_E_NOT_SUPPORT;
        }
        service_que_node.lport = 0xffff;
        p_service_que_logic_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
        if (NULL == p_service_que_logic_node)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
            return CTC_E_NOT_EXIST;
        }
        logic_dst_port = p_service_que_logic_node->logic_dst_port;
    }
    service_que_node.lport = lport;
    p_service_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
    if (NULL == p_service_que_node)
    {
        first_add = 1;
        p_service_que_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_extend_que_node_t));
        if (NULL == p_service_que_node)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_service_que_node, 0, sizeof(sys_extend_que_node_t));

        ret = _sys_usw_queue_alloc_ext_group(lchip, 1, &group_id);
        if (ret < 0)
        {
            mem_free(p_service_que_node);
            return ret;
        }
        cmd = DRV_IOW(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, group_id, cmd, &chan_id), ret, error0);

        p_service_que_node->service_id = service_id;
        p_service_que_node->lchip = lchip;
        p_service_que_node->lport = lport;
        p_service_que_node->type = SYS_EXTEND_QUE_TYPE_SERVICE;
        p_service_que_node->group_id = group_id;
        p_service_que_node->logic_dst_port = logic_dst_port;
        CTC_ERROR_GOTO(sys_usw_port_extender_add(lchip, p_service_que_node), ret, error0);

        p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
        if (NULL == p_sys_group_node)
        {
             SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
             ret = CTC_E_NOT_EXIST;
             goto error1;
        }
        p_sys_group_node->chan_id  = chan_id;  /*0~63*/
    }
    else
    {
        if(p_usw_queue_master[lchip]->service_queue_mode == 1 ||(p_usw_queue_master[lchip]->service_queue_mode != 1 && logic_port == 0))
        { /*service id based queue*/
            return CTC_E_EXIST;
        }
    }
    service_id_tmp = dir ? logic_dst_port : service_id;
    if(p_usw_queue_master[lchip]->service_queue_mode == 1 || dir == CTC_EGRESS)
    {
        CTC_ERROR_GOTO(_sys_usw_qos_write_queue_key(lchip, service_id_tmp,dest_port,logic_port,group_id,1,&key_index, dir),ret,error2);
        p_service_que_node->key_index = key_index;
    }
    else
    {
        service.service_id = service_id;
        p_service = sys_usw_service_lookup(lchip, &service);
        if (NULL == p_service)
        {
            ret = CTC_E_NOT_EXIST;
            goto error2;
        }
        /*new logic*/
        CTC_SLIST_LOOP(p_service->p_logic_port_list, ctc_slistnode)
        {
            p_logic_port_node = _ctc_container_of(ctc_slistnode, sys_qos_logicport_t, head);
            if(logic_port == 0 || logic_port == p_logic_port_node->logic_port)
            {
                p_logice_que_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_extend_que_node_t));
                if (NULL == p_logice_que_node)
                {
                    ret = CTC_E_NO_MEMORY;
                    goto error3;
                }
                sal_memset(p_logice_que_node, 0, sizeof(sys_extend_que_node_t));
                CTC_ERROR_GOTO(_sys_usw_qos_write_queue_key(lchip, service_id_tmp,dest_port,p_logic_port_node->logic_port,p_service_que_node->group_id,0,&key_index, dir),ret,error3);
                p_logice_que_node->logic_src_port = p_logic_port_node->logic_port;
                p_logice_que_node->service_id = service_id;
                p_logice_que_node->lport = lport;
                p_logice_que_node->lchip = lchip;
                p_logice_que_node->type = SYS_EXTEND_QUE_TYPE_LOGIC_PORT;
                p_logice_que_node->key_index = key_index;
                sys_usw_port_extender_add(lchip, p_logice_que_node);
            }
        }

        if(logic_port == 0)
        {
            /*bind new dest port */
            p_dest_port_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_destport_t));
            if (NULL == p_dest_port_node)
            {
                ret = CTC_E_NO_MEMORY;
                goto error3;
            }
            sal_memset(p_dest_port_node, 0, sizeof(sys_qos_destport_t));
            p_dest_port_node->lchip= lchip;
            p_dest_port_node->lport = lport;
            ctc_slist_add_tail(p_service->p_dest_port_list, &p_dest_port_node->head);
        }
    }
    return CTC_E_NONE;

error3:
    CTC_SLIST_LOOP(p_service->p_logic_port_list, ctc_slistnode)
    {
        p_logic_port_node = _ctc_container_of(ctc_slistnode, sys_qos_logicport_t, head);
        if(logic_port == 0 || logic_port == p_logic_port_node->logic_port)
        {
            sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));
            service_que_node.type = SYS_EXTEND_QUE_TYPE_LOGIC_PORT;
            service_que_node.logic_src_port = p_logic_port_node->logic_port;
            service_que_node.service_id = service_id;
            service_que_node.lchip = lchip;
            service_que_node.lport = lport;
            p_logice_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
            if(p_logice_que_node)
            {
                sys_usw_port_extender_remove(lchip, p_logice_que_node);
                _sys_usw_queue_free_key_index(lchip, 1, p_logice_que_node->key_index);
                cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_logice_que_node->key_index, cmd, &cmd);
                mem_free(p_logice_que_node);
            }
        }
    }
error2:
    if(first_add)
    {
        p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
        if (NULL == p_sys_group_node)
        {
             SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
             ret = CTC_E_NOT_EXIST;
        }
        p_sys_group_node->chan_id  = 0xff;  /*0~63*/
    }
error1:
    if(first_add)
    {
        sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));
        service_que_node.type = SYS_EXTEND_QUE_TYPE_SERVICE;
        service_que_node.service_id = service_id;
        service_que_node.lchip = lchip;
        service_que_node.lport = lport;
        p_service_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
        if (NULL == p_service_que_node)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
            ret = CTC_E_NOT_EXIST;
        }
        sys_usw_port_extender_remove(lchip, p_service_que_node);
    }
error0:
    if(first_add)
    {
        _sys_usw_queue_free_ext_group(lchip, 1, group_id);
        mem_free(p_service_que_node);
    }
    return ret;
}

uint32
_sys_usw_qos_unbind_service_dstport(uint8 lchip, uint32 service_id, uint16 dest_port, uint16 logic_port, uint8 dir)
{
    sys_service_node_t* p_service = NULL;
    sys_service_node_t service;
    sys_qos_destport_t* p_dest_port_node = NULL;
    sys_qos_logicport_t* p_logic_port_node = NULL;
    ctc_slistnode_t* node , *next_node;
    uint32 cmd = 0;
    uint8  chan_id = 0;
    uint16 lport = 0;
    uint8  gchip = 0;
    uint8 xgpon_en = 0;
    DsQMgrGrpChanMap_m    grp_chan_map;
    sys_extend_que_node_t service_que_node;
    sys_extend_que_node_t* p_service_que_node ,*p_logice_que_node;
    sys_queue_group_node_t *p_sys_group_node = NULL;
    uint32 value = 0;
    tbl_entry_t   tcam_key;
    DsQueueMapTcamKey_m   QueueMapTcamKey;
    DsQueueMapTcamKey_m   QueueMapTcamMask;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&grp_chan_map, 0, sizeof(DsQMgrGrpChanMap_m));
    sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(dest_port, lchip, lport);
    chan_id = SYS_GET_CHANNEL_ID(lchip, dest_port);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }
    if(p_usw_queue_master[lchip]->service_queue_mode != 2&& dir == CTC_EGRESS && xgpon_en != 1)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if(p_usw_queue_master[lchip]->service_queue_mode == 0 || (p_usw_queue_master[lchip]->service_queue_mode == 2 && dir == CTC_INGRESS))
    {
        sal_memset(&service, 0, sizeof(service));
        service.service_id = service_id;
        p_service = sys_usw_service_lookup(lchip, &service);
        if (NULL == p_service)
        {
            return CTC_E_NOT_EXIST;
        }
        CTC_SLIST_LOOP_DEL(p_service->p_logic_port_list, node, next_node)
        {/*service_queue_mode == 0*/
            p_logic_port_node = _ctc_container_of(node, sys_qos_logicport_t, head);
            if(logic_port == 0 || logic_port == p_logic_port_node->logic_port)
            {
                sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));
                service_que_node.service_id = service_id;
                service_que_node.logic_src_port = p_logic_port_node->logic_port;
                service_que_node.lport = lport;
                service_que_node.lchip = lchip;
                service_que_node.type = SYS_EXTEND_QUE_TYPE_LOGIC_PORT;
                p_logice_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
                if (!p_logice_que_node)
                {
                    continue;
                }
                _sys_usw_queue_free_key_index(lchip, 1, p_logice_que_node->key_index);
                cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, p_logice_que_node->key_index, cmd, &cmd);
                sys_usw_port_extender_remove(lchip, p_logice_que_node);
                mem_free(p_logice_que_node);
            }
        }
    }
    if( logic_port != 0)
    { /*service_queue_mode == 0*/
       return CTC_E_NONE;
    }
    sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));
    service_que_node.type = SYS_EXTEND_QUE_TYPE_SERVICE;
    service_que_node.service_id = service_id;
    service_que_node.lchip = lchip;
    service_que_node.lport = lport;
    p_service_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
    if (NULL == p_service_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    tcam_key.data_entry = (uint32*)(&QueueMapTcamKey);
    tcam_key.mask_entry = (uint32*)(&QueueMapTcamMask);
    cmd = DRV_IOR(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_service_que_node->key_index, cmd, &tcam_key));
    value = GetDsQueueMapTcamKey(V, nexthopPtr_f, &QueueMapTcamKey);
    if(value && dir != CTC_EGRESS)
    {
        return CTC_E_INVALID_PARAM;
    }
    sys_usw_port_extender_remove(lchip, p_service_que_node);
    if(p_usw_queue_master[lchip]->service_queue_mode == 1 || dir == CTC_EGRESS)
    {
        CTC_ERROR_RETURN(_sys_usw_queue_free_key_index(lchip, 1, p_service_que_node->key_index));
        cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_service_que_node->key_index, cmd, &cmd));
    }
    else
    {
        CTC_SLIST_LOOP(p_service->p_dest_port_list, node)
        {
            p_dest_port_node = _ctc_container_of(node, sys_qos_destport_t, head);
            if (p_dest_port_node->lchip == lchip && p_dest_port_node->lport == lport)
            {
                break;
            }
        }
        ctc_slist_delete_node(p_service->p_dest_port_list, &p_dest_port_node->head);
        mem_free(p_dest_port_node);
    }
    _sys_usw_qos_flush_queue(lchip,chan_id,p_service_que_node->group_id);
    _sys_usw_queue_free_ext_group(lchip, 1, p_service_que_node->group_id);
    chan_id = 0;
    SetDsQMgrGrpChanMap(V, channelId_f, &grp_chan_map, chan_id);
    cmd = DRV_IOW(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_service_que_node->group_id, cmd, &grp_chan_map));
    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, p_service_que_node->group_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    p_sys_group_node->chan_id  = 0xFF;
    mem_free(p_service_que_node);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_bind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port)
{
    uint8  gchip = 0;
    int32  ret = 0;
    uint8 xgpon_en = 0;
    sys_extend_que_node_t service_que_node;
    sys_extend_que_node_t* p_service_que_logic_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    if(p_usw_queue_master[lchip]->service_queue_mode != 2 && xgpon_en != 1)
    {
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));

    CTC_MIN_VALUE_CHECK(service_id, 1);
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    /* look up service id + dstport*/
    service_que_node.type = SYS_EXTEND_QUE_TYPE_SERVICE;
    service_que_node.service_id = service_id;
    service_que_node.lchip = lchip;
    service_que_node.lport = 0xffff;

    p_service_que_logic_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
    if (NULL != p_service_que_logic_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
        return CTC_E_EXIST;
    }

    p_service_que_logic_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_extend_que_node_t));
    if (NULL == p_service_que_logic_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_service_que_logic_node, 0, sizeof(sys_extend_que_node_t));

    p_service_que_logic_node->service_id = service_id;
    p_service_que_logic_node->lchip = lchip;
    p_service_que_logic_node->type = SYS_EXTEND_QUE_TYPE_SERVICE;
    p_service_que_logic_node->lport = 0xffff;
    p_service_que_logic_node->logic_dst_port = logic_dst_port;
    CTC_ERROR_GOTO(sys_usw_port_extender_add(lchip, p_service_que_logic_node), ret, error0);

    return CTC_E_NONE;

error0:
    mem_free(p_service_que_logic_node);

    return ret;
}

int32
_sys_usw_qos_unbind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port)
{
    uint8  gchip = 0;
    uint8 xgpon_en = 0;
    sys_extend_que_node_t service_que_node;
    sys_extend_que_node_t* p_service_que_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    if(p_usw_queue_master[lchip]->service_queue_mode != 2 && xgpon_en != 1)
    {
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&service_que_node, 0, sizeof(sys_extend_que_node_t));

    CTC_MIN_VALUE_CHECK(service_id, 1);
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    service_que_node.type = SYS_EXTEND_QUE_TYPE_SERVICE;
    service_que_node.service_id = service_id;
    service_que_node.lport = 0xffff;
    service_que_node.lchip = lchip;
    p_service_que_node = sys_usw_port_extender_lookup(lchip, &service_que_node);
    if (NULL == p_service_que_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    sys_usw_port_extender_remove(lchip, p_service_que_node);
    mem_free(p_service_que_node);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_bind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port)
{
    uint8 update_flag = 0;
    uint16 old_serviceid = 0;
    uint16 dest_port = 0;
    int32 ret = 0;
    uint8 gchip = 0;
    ctc_slistnode_t *node, *node2, *next_node;
    sys_service_node_t* p_service = NULL;
    sys_service_node_t service;
    sys_qos_logicport_t* p_logic_port_node = NULL;
    sys_qos_logicport_service_t logicport_que_node;
    sys_qos_logicport_service_t* p_logicport_que_node = NULL;
    sys_qos_destport_t* p_dest_port_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*1.Service Queue based on Sevice ID retun none*/
    if(p_usw_queue_master[lchip]->service_queue_mode == 1)
    {
        return CTC_E_NONE;
    }
    if(logic_port == 0 || service_id == 0)
    {
        return CTC_E_INVALID_PARAM;
    }
    sal_memset(&logicport_que_node, 0, sizeof(sys_qos_logicport_service_t));
    logicport_que_node.logic_src_port = logic_port;
    sys_usw_get_gchip_id(lchip, &gchip);
    /*2.lookup by logic_port*/
    p_logicport_que_node = sys_usw_logicport_service_lookup(lchip, &logicport_que_node);
    if(p_logicport_que_node != NULL)
    {
        if(p_logicport_que_node->service_id == service_id)
        {
            return CTC_E_NONE;
        }
        old_serviceid = p_logicport_que_node->service_id;
        p_logicport_que_node->service_id = service_id;
        update_flag = 1;
    }
    else
    {
        p_logicport_que_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_logicport_service_t));
        if (NULL == p_logicport_que_node)
        {
            return CTC_E_NO_MEMORY;
        }
        p_logicport_que_node->logic_src_port = logic_port;
        p_logicport_que_node->service_id = service_id;
        sys_usw_logicport_service_add(lchip, p_logicport_que_node);
    }

    sal_memset(&service, 0, sizeof(service));
    service.service_id = service_id;
    p_service = sys_usw_service_lookup(lchip, &service);
    if (NULL == p_service)
    {
        ret = CTC_E_NOT_EXIST;
        goto error1;
    }

    /*bind new logic port */
    p_logic_port_node = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_logicport_t));
    if (NULL == p_logic_port_node)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }
    p_logic_port_node->logic_port = logic_port;
    ctc_slist_add_tail(p_service->p_logic_port_list, &p_logic_port_node->head);

    CTC_SLIST_LOOP(p_service->p_dest_port_list, node)
    {
        p_dest_port_node = _ctc_container_of(node, sys_qos_destport_t, head);
        dest_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip,p_dest_port_node->lport);
        CTC_ERROR_GOTO(_sys_usw_qos_bind_service_dstport(lchip, service_id,  dest_port, logic_port,CTC_INGRESS),ret,error2);

    }
    if(!update_flag )
    {
        return CTC_E_NONE;
    }
    service.service_id = old_serviceid;
    p_service = sys_usw_service_lookup(lchip, &service);
    if (NULL == p_service)
    {
       return CTC_E_NONE;
    }
    CTC_SLIST_LOOP_DEL(p_service->p_logic_port_list, node,next_node)
    {
        p_logic_port_node = _ctc_container_of(node, sys_qos_logicport_t, head);
        if (p_logic_port_node->logic_port != logic_port)
        {
            continue;
        }
        CTC_SLIST_LOOP(p_service->p_dest_port_list, node2)
        {
            p_dest_port_node = _ctc_container_of(node2, sys_qos_destport_t, head);
            dest_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip,p_dest_port_node->lport);
            _sys_usw_qos_unbind_service_dstport(lchip, old_serviceid, dest_port, logic_port,CTC_INGRESS);
        }
        ctc_slist_delete_node(p_service->p_logic_port_list, &p_logic_port_node->head);
        mem_free(p_logic_port_node);
        break;
    }

    return CTC_E_NONE;
error2:
    CTC_SLIST_LOOP(p_service->p_dest_port_list, node)
    {
        p_dest_port_node = _ctc_container_of(node, sys_qos_destport_t, head);
        dest_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip,p_dest_port_node->lport);
        _sys_usw_qos_unbind_service_dstport(lchip, service_id,  dest_port, logic_port,CTC_INGRESS);
    }
    ctc_slist_delete_node(p_service->p_logic_port_list, &p_logic_port_node->head);
    mem_free(p_logic_port_node);

error1:
    if(update_flag == 0)
    {
        logicport_que_node.logic_src_port = logic_port;
        p_logicport_que_node = sys_usw_logicport_service_lookup(lchip, &logicport_que_node);
        if (NULL == p_logicport_que_node)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
            return CTC_E_NOT_EXIST;
        }
        sys_usw_logicport_service_remove(lchip, p_logicport_que_node);
        mem_free(p_logicport_que_node);
    }
    else
    {
        p_logicport_que_node->service_id = old_serviceid;
    }
    return ret;
}

int32
_sys_usw_qos_unbind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port)
{
    uint8 gchip = 0;
    uint16 dest_port = 0;
    sys_service_node_t* p_service = NULL;
    sys_service_node_t service;
    sys_qos_logicport_t* p_logic_port_node = NULL;
    sys_qos_destport_t* p_dest_port_node = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;
    sys_qos_logicport_service_t logicport_que_node;
    sys_qos_logicport_service_t* p_logicport_que_node = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(p_usw_queue_master[lchip]->service_queue_mode == 1)
    {
        return CTC_E_NONE;
    }
    if(logic_port == 0 || service_id == 0)
    {
        return CTC_E_INVALID_PARAM;
    }
    sal_memset(&logicport_que_node, 0, sizeof(sys_qos_logicport_service_t));
    logicport_que_node.logic_src_port = logic_port;
    /*2.lookup by logic_port*/
    p_logicport_que_node = sys_usw_logicport_service_lookup(lchip, &logicport_que_node);
    if(p_logicport_que_node != NULL && p_logicport_que_node->service_id == service_id)
    {
        sys_usw_logicport_service_remove(lchip, p_logicport_que_node);
        mem_free(p_logicport_que_node);
    }
    else
    {
        return CTC_E_NOT_EXIST;
    }
    sal_memset(&service, 0, sizeof(service));
    service.service_id = service_id;
    p_service = sys_usw_service_lookup(lchip, &service);
    if (NULL == p_service)
    {
        return CTC_E_NOT_EXIST;
    }
    CTC_SLIST_LOOP(p_service->p_logic_port_list, ctc_slistnode)
    {
        p_logic_port_node = _ctc_container_of(ctc_slistnode, sys_qos_logicport_t, head);
        if (p_logic_port_node->logic_port == logic_port)
        {
            break;
        }
    }
    sys_usw_get_gchip_id(lchip, &gchip);
    /*unbind logic port */
    CTC_SLIST_LOOP(p_service->p_dest_port_list, ctc_slistnode)
    {
        p_dest_port_node = _ctc_container_of(ctc_slistnode, sys_qos_destport_t, head);
        dest_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip,p_dest_port_node->lport);
        _sys_usw_qos_unbind_service_dstport(lchip,service_id,dest_port,logic_port,CTC_INGRESS);
    }

    ctc_slist_delete_node(p_service->p_logic_port_list, &p_logic_port_node->head);
    mem_free(p_logic_port_node);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_qos_create_port_extender_queue(uint8 lchip, uint16 lport, uint8 channel, uint16* grp_id, uint8 type)
{
    int32  ret = 0;
    uint32 group_id = 0;
    uint32 key_index = 0;
    uint32 cmd = 0;
    uint32 dest_map = 0;
    uint8  gchip = 0;
    sys_extend_que_node_t port_extender_node;
    sys_extend_que_node_t* p_port_extender_node = NULL;
    DsQueueMapTcamData_m  QueueMapTcamData;
    DsQueueMapTcamKey_m   QueueMapTcamKey;
    DsQueueMapTcamKey_m   QueueMapTcamMask;
    tbl_entry_t   tcam_key;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&QueueMapTcamData, 0 , sizeof(QueueMapTcamData));
    sal_memset(&QueueMapTcamKey, 0 , sizeof(QueueMapTcamKey));
    sal_memset(&QueueMapTcamMask, 0 , sizeof(QueueMapTcamMask));
    sal_memset(&port_extender_node, 0, sizeof(port_extender_node));

    port_extender_node.channel = type ? 0 : channel;
    port_extender_node.type = type ? SYS_EXTEND_QUE_TYPE_MCAST : SYS_EXTEND_QUE_TYPE_BPE;
    port_extender_node.lchip = lchip;
    port_extender_node.lport = lport;
    p_port_extender_node = sys_usw_port_extender_lookup(lchip, &port_extender_node);
    if (NULL != p_port_extender_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
        return CTC_E_EXIST;
    }

    p_port_extender_node =  mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_extend_que_node_t));
    if (NULL == p_port_extender_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_port_extender_node, 0, sizeof(sys_extend_que_node_t));

    ret = _sys_usw_queue_alloc_ext_group(lchip, 1, &group_id);
    if (ret < 0)
    {
        mem_free(p_port_extender_node);
        return ret;
    }
    CTC_ERROR_GOTO(_sys_usw_queue_alloc_key_index(lchip, 1, &key_index), ret, error0);


    /*write tcam ad*/
    SetDsQueueMapTcamData(V, extGroupId_f, &QueueMapTcamData, group_id);
    cmd = DRV_IOW(DsQueueMapTcamData_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &QueueMapTcamData), ret, error1);

    /*write tcam key*/
    sys_usw_get_gchip_id(lchip, &gchip);
    dest_map = lport&0x1FF;
    dest_map = type ? (dest_map | 0x40000): dest_map;
    if(type == 0)
    {
        SetDsQueueMapTcamKey(V, channelId_f, &QueueMapTcamKey, channel);
        SetDsQueueMapTcamKey(V, channelId_f, &QueueMapTcamMask, 0x3f);
    }
    else
    {
        SetDsQueueMapTcamKey(V, macKnown_f, &QueueMapTcamKey, 0);
        SetDsQueueMapTcamKey(V, macKnown_f, &QueueMapTcamMask, 1);
    }
    SetDsQueueMapTcamKey(V, mcastIdValid_f, &QueueMapTcamKey, 0);
    SetDsQueueMapTcamKey(V, mcastIdValid_f, &QueueMapTcamMask, 1);
    SetDsQueueMapTcamKey(V, destChipIdMatch_f, &QueueMapTcamKey, 1);
    SetDsQueueMapTcamKey(V, destChipIdMatch_f, &QueueMapTcamMask, 1);
    SetDsQueueMapTcamKey(V, destMap_f, &QueueMapTcamKey, dest_map);
    SetDsQueueMapTcamKey(V, destMap_f, &QueueMapTcamMask, type ? 0x401ff : 0x1ff);
    cmd = DRV_IOW(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    tcam_key.data_entry = (uint32*)(&QueueMapTcamKey);
    tcam_key.mask_entry = (uint32*)(&QueueMapTcamMask);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &tcam_key), ret, error1);

    p_port_extender_node->lchip = lchip;
    p_port_extender_node->lport = lport;
    p_port_extender_node->type = type ? SYS_EXTEND_QUE_TYPE_MCAST : SYS_EXTEND_QUE_TYPE_BPE;;
    p_port_extender_node->channel = type ? 0 : channel;
    p_port_extender_node->group_id = group_id;
    p_port_extender_node->key_index = key_index;
    CTC_ERROR_GOTO(sys_usw_port_extender_add(lchip, p_port_extender_node), ret, error2);

    *grp_id = p_port_extender_node->group_id;

    return CTC_E_NONE;

error2:
    cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &cmd));

error1:
    _sys_usw_queue_free_key_index(lchip, 1, key_index);

error0:
    _sys_usw_queue_free_ext_group(lchip, 1, group_id);
    mem_free(p_port_extender_node);

    return ret;
}

STATIC int32
_sys_usw_qos_destroy_port_extender_queue(uint8 lchip, uint16 dest_port, uint8 channel, uint16* grp_id, uint8 type)
{
    sys_extend_que_node_t port_extender_node;
    sys_extend_que_node_t* p_port_extender_node = NULL;
    uint16 lport = 0;
    uint32 cmd = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&port_extender_node, 0, sizeof(port_extender_node));

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(dest_port, lchip, lport);
    port_extender_node.channel = type ? 0 : channel;
    port_extender_node.type = type ? SYS_EXTEND_QUE_TYPE_MCAST : SYS_EXTEND_QUE_TYPE_BPE;
    port_extender_node.lchip = lchip;
    port_extender_node.lport = lport;
    p_port_extender_node = sys_usw_port_extender_lookup(lchip, &port_extender_node);
    if (NULL == p_port_extender_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    sys_usw_port_extender_remove(lchip, p_port_extender_node);

    cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_port_extender_node->key_index, cmd, &cmd));

    CTC_ERROR_RETURN(_sys_usw_queue_free_key_index(lchip, 1, p_port_extender_node->key_index));
    CTC_ERROR_RETURN(_sys_usw_queue_free_ext_group(lchip, 1, p_port_extender_node->group_id));

    *grp_id = p_port_extender_node->group_id;

    mem_free(p_port_extender_node);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_add_update_group_id_to_channel(uint8 lchip, uint16 group_id, uint8 channel, uint8 flag)
{
    DsQMgrGrpChanMap_m grp_chan_map;
    uint32 cmd = 0;
    sys_queue_group_node_t *p_sys_group_node = NULL;

    sal_memset(&grp_chan_map, 0, sizeof(DsQMgrGrpChanMap_m));
    cmd = DRV_IOR(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &grp_chan_map));
    SetDsQMgrGrpChanMap(V, channelId_f, &grp_chan_map, channel);
    cmd = DRV_IOW(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &grp_chan_map));

    p_sys_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
    if (NULL == p_sys_group_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    p_sys_group_node->chan_id = flag ? channel : 0xff;  /*0~63*/

    return CTC_E_NONE;
}
/**
 @brief Mapping a extend port to channel.
*/
int32
_sys_usw_qos_add_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    uint16 group_id = 0;
    /* enq_mode == 1 not support bpe*/
    if (p_usw_queue_master[lchip]->queue_num_per_extend_port == 0 || (p_usw_queue_master[lchip]->queue_num_per_extend_port != 0 &&
        p_usw_queue_master[lchip]->enq_mode == 1 && DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_qos_create_port_extender_queue(lchip, lport, channel, &group_id,0));
    CTC_ERROR_RETURN(_sys_usw_qos_add_update_group_id_to_channel(lchip, group_id, channel, 1));

    return CTC_E_NONE;
}

int32
_sys_usw_qos_remove_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    uint16 group_id = 0;

    /* enq_mode == 1 not support bpe*/
    if (p_usw_queue_master[lchip]->queue_num_per_extend_port == 0 || (p_usw_queue_master[lchip]->queue_num_per_extend_port != 0 &&
        p_usw_queue_master[lchip]->enq_mode == 1 && DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_usw_qos_destroy_port_extender_queue(lchip, lport, channel, &group_id,0));
    CTC_ERROR_RETURN(_sys_usw_qos_add_update_group_id_to_channel(lchip, group_id, 0, 0));
    CTC_ERROR_RETURN(_sys_usw_qos_flush_queue(lchip, channel, group_id));

    return CTC_E_NONE;
}

int32
_sys_usw_qos_add_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    uint16 group_id = 0;
    uint16 offset = 0;
    uint32 field_val = 0;
    uint32 field_id = 0;
    uint32 cmd = 0;

    if(p_usw_queue_master[lchip]->enq_mode != 1)
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_usw_qos_create_port_extender_queue(lchip, lport, channel, &group_id, 1));
    CTC_ERROR_RETURN(_sys_usw_qos_add_update_group_id_to_channel(lchip, group_id, channel, 1));
    for(offset = 0; offset < 4; offset++)
    {
        field_id = DsQMgrExtSchMap_grpNode_0_chanNodeSel_f + offset;
        cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_remove_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel)
{
    uint16 group_id = 0;
    uint16 offset = 0;
    uint32 field_val = 2;
    uint32 field_id = 0;
    uint32 cmd = 0;

    if(p_usw_queue_master[lchip]->enq_mode != 1)
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_usw_qos_destroy_port_extender_queue(lchip, lport, channel, &group_id,1));
    CTC_ERROR_RETURN(_sys_usw_qos_add_update_group_id_to_channel(lchip, group_id, 0, 0));
    for(offset = 0; offset < 4; offset++)
    {
        field_id = DsQMgrExtSchMap_grpNode_0_chanNodeSel_f + offset;
        cmd = DRV_IOW(DsQMgrExtSchMap_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_check_pkt_adjust_profile(uint8 lchip, uint8 channel)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint8 profile_id = 0;
    uint8 index = 0;

    cmd = DRV_IOR(DsErmChannel_t, DsErmChannel_packetLengthAdjustProfId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &field_val));
    profile_id = field_val;
    for (index = 0; index < MCHIP_CAP(SYS_CAP_CHANID_MAX); index++)
    {
        cmd = DRV_IOR(DsErmChannel_t, DsErmChannel_packetLengthAdjustProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        if(field_val == profile_id && index != channel)
        {
            return CTC_E_NONE;
        }
    }

    field_val = 0;
    cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjust_f + profile_id * 2);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjustType_f + profile_id * 2);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}
int32
sys_usw_queue_set_queue_pkt_adjust(uint8 lchip, ctc_qos_queue_pkt_adjust_t* p_pkt)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 field_len = 0;
    uint32 field_type = 0;
    uint8 index = 0;
    uint8 new_index = 0xFF;
    uint8 old_index = 0;
    uint8 exist = 0;
    uint8 channel = 0;
    uint16 queue_id = 0;
    uint8 adjust_len = 0;
    uint8 adjust_type = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,  &p_pkt->queue, &queue_id));

    CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip, queue_id, &channel));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "channel id = %d\n", channel);

    adjust_len = (p_pkt->adjust_len) & 0x7F;
    adjust_type = (p_pkt->adjust_len) >> 7;
    adjust_type = adjust_len ? adjust_type : 0;
    if(adjust_len == 0 && adjust_type == 0 )
    {
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjust_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjustType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        CTC_ERROR_RETURN(_sys_usw_qos_check_pkt_adjust_profile(lchip, channel));

        cmd = DRV_IOW(DsErmChannel_t, DsErmChannel_packetLengthAdjustProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &field_val));
        return CTC_E_NONE;
    }

    for (index = 0; index < 4; index++)
    {
        cmd = DRV_IOR(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjust_f + index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_len));

        cmd = DRV_IOR(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjustType_f + index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_type));

        if ((field_len == adjust_len) && (field_type == adjust_type))
        {
            new_index = index;
            exist = 1;
            break;
        }

        if (0 == field_len && index != 0)
        {
            new_index = index;
        }
    }

    if (new_index == 0xFF)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;
    }

    cmd = DRV_IOR(DsErmChannel_t, DsErmChannel_packetLengthAdjustProfId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &field_val));
    old_index = field_val;

    if (new_index == old_index)
    {
        return CTC_E_NONE;
    }

    /*new index*/
    if (!exist)
    {
        CTC_ERROR_RETURN(_sys_usw_qos_check_pkt_adjust_profile(lchip, channel));
        field_val = adjust_len;
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjust_f + new_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        field_val = adjust_type;
        cmd = DRV_IOW(QWritePktLenCtl_t, QWritePktLenCtl_g1_0_packetLengthAdjustType_f + new_index * 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    field_val = new_index;
    cmd = DRV_IOW(DsErmChannel_t, DsErmChannel_packetLengthAdjustProfId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &field_val));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "channel id = %d, NetPktAdjIndex:%d, value:%d\n", channel,  new_index, p_pkt->adjust_len);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_service_proc(uint8 lchip, ctc_qos_service_info_t *p_service_para)
{
    uint32 service_id = p_service_para->service_id;
    if(p_service_para->dir == CTC_EGRESS && (DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NOT_SUPPORT;
    }
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(p_service_para->opcode)
    {
    case CTC_QOS_SERVICE_ID_CREATE:
        CTC_ERROR_RETURN(
        _sys_usw_qos_create_service(lchip, service_id));
        break;

    case CTC_QOS_SERVICE_ID_DESTROY:
        CTC_ERROR_RETURN(
        _sys_usw_qos_destroy_service(lchip, service_id));
        break;

    case CTC_QOS_SERVICE_ID_BIND_DESTPORT:
        CTC_ERROR_RETURN(
        _sys_usw_qos_bind_service_dstport(lchip, service_id, p_service_para->gport,0,p_service_para->dir));

        break;

    case CTC_QOS_SERVICE_ID_UNBIND_DESTPORT:
        CTC_ERROR_RETURN(
        _sys_usw_qos_unbind_service_dstport(lchip, service_id, p_service_para->gport,0,p_service_para->dir));

        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_queue_set(uint8 lchip, ctc_qos_queue_cfg_t* p_que_cfg)
{
    uint8 sub_queue_id = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (p_que_cfg->type)
    {
    case CTC_QOS_QUEUE_CFG_PRI_MAP:
        CTC_ERROR_RETURN(
            sys_usw_set_priority_queue_map(lchip, &p_que_cfg->value.pri_map));
        break;

    case CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN:
        CTC_ERROR_RETURN(
            sys_usw_queue_set_stats_enable(lchip, &p_que_cfg->value.stats));
        break;

    case CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU:
        CTC_VALUE_RANGE_CHECK(p_que_cfg->value.value_32, 1, 0xFFFF);
        p_usw_queue_master[lchip]->wrr_weight_mtu = p_que_cfg->value.value_32;
        break;

    case CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP:
        {
            ctc_qos_queue_cpu_reason_map_t* p_reason_map = &p_que_cfg->value.reason_map;

            sub_queue_id = p_reason_map->reason_group * SYS_USW_MAX_QNUM_PER_GROUP + p_reason_map->queue_id;
            if((p_usw_queue_master[lchip]->monitor_drop_en == 1) &&
                p_usw_queue_master[lchip]->cpu_reason[CTC_PKT_CPU_REASON_QUEUE_DROP_PKT].sub_queue_id == sub_queue_id)
            {
                return CTC_E_INVALID_PARAM;
            }
            if (sub_queue_id >= p_usw_queue_master[lchip]->queue_num_for_cpu_reason)
            { /*Exceed Max Cpu Reason Group*/
                return CTC_E_INVALID_PARAM;
            }
            CTC_ERROR_RETURN(sys_usw_cpu_reason_set_map(lchip,
                                                          p_reason_map->cpu_reason,
                                                          p_reason_map->queue_id,
                                                          p_reason_map->reason_group));
        }
        break;

    case CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST:
        {
            ctc_qos_queue_cpu_reason_dest_t* p_reason_dest = &p_que_cfg->value.reason_dest;

            if (CTC_PKT_CPU_REASON_TO_NHID == p_reason_dest->dest_type)
            {
                CTC_ERROR_RETURN(sys_usw_cpu_reason_set_dest(lchip, p_reason_dest->cpu_reason,
                                                                    p_reason_dest->dest_type,
                                                                    p_reason_dest->nhid));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_cpu_reason_set_dest(lchip, p_reason_dest->cpu_reason,
                                                                    p_reason_dest->dest_type,
                                                                    p_reason_dest->dest_port));
            }
        }
        break;
    case CTC_QOS_QUEUE_CFG_LENGTH_ADJUST:  /*GG will use nexthop adjust length index*/
        CTC_ERROR_RETURN(sys_usw_queue_set_queue_pkt_adjust(lchip, &p_que_cfg->value.pkt));
        break;

    case CTC_QOS_QUEUE_CFG_SERVICE_BIND:
        CTC_ERROR_RETURN(
        _sys_usw_qos_service_proc(lchip, &p_que_cfg->value.srv_queue_info));
        break;

    case CTC_QOS_QUEUE_CFG_QUEUE_NUM:
    case CTC_QOS_QUEUE_CFG_QUEUE_REASON_MODE:
    case CTC_QOS_QUEUE_CFG_QUEUE_REASON_PRIORITY:
    case CTC_QOS_QUEUE_CFG_QUEUE_REASON_MISC:
    default:
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

extern int32
sys_usw_qos_set_monitor_drop_queue_id(uint8 lchip,  ctc_qos_queue_drop_monitor_t* drop_monitor)
{
    uint32 cmd = 0;
    ErmSpanOnDropCtl_m span_on_drop_ctl;
    uint16 lport = 0;
    uint8 src_chan_id = 0;
    uint32 dst_chan_id = 0;
    uint32 array_val[2] = {0};
    uint32 src_gport = 0;
    uint32 dst_gport = 0;
    uint8 is_cpu_port = 0;
    uint16 reason = 0;
    uint16 sub_queue_id = 0;
    uint8 group_id = 0;
    uint8 queue_id = 0;
    uint32 field_val = 0;
    uint32 field_id = 0;
    uint16 queue_index = 0;
    uint32 nexthop_ptr = 0;
    DsQMgrGrpChanMap_m grp_chan_map;
    BufRetrvSpanCtl_m buffer_retrieve_ctl;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&span_on_drop_ctl, 0, sizeof(span_on_drop_ctl));
    sal_memset(&buffer_retrieve_ctl, 0, sizeof(buffer_retrieve_ctl));
    sal_memset(&grp_chan_map, 0, sizeof(grp_chan_map));

    src_gport = drop_monitor->src_gport;
    dst_gport = drop_monitor->dst_gport;
    if(src_gport == dst_gport)
    {
        return CTC_E_INVALID_PARAM;
    }
    if (CTC_IS_CPU_PORT(dst_gport))
    {
        is_cpu_port = 1;
        dst_chan_id = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);
    }
    else
    {
        if(p_usw_queue_master[lchip]->enq_mode == 1 && DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(dst_gport, lchip, lport);
        dst_chan_id = SYS_GET_CHANNEL_ID(lchip, dst_gport);
        if (dst_chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
            return CTC_E_INVALID_PORT;
        }
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(src_gport, lchip, lport);

    src_chan_id = SYS_GET_CHANNEL_ID(lchip, src_gport);
    if (src_chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }
    cmd = DRV_IOR(ErmSpanOnDropCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &span_on_drop_ctl));

    if(GetErmSpanOnDropCtl(V, spanOnDropEn_f, &span_on_drop_ctl))
    {
        if(dst_chan_id != GetErmSpanOnDropCtl(V, spanOnDropChannelId_f, &span_on_drop_ctl))
        {
            return CTC_E_INVALID_PORT;
        }
    }

    if (drop_monitor->enable >= 1)
    {
        if(is_cpu_port == 1)
        {
            uint8  valid = 0;
            uint16 max_queue_num = p_usw_queue_master[lchip]->queue_num_for_cpu_reason;
            sub_queue_id = p_usw_queue_master[lchip]->cpu_reason[CTC_PKT_CPU_REASON_QUEUE_DROP_PKT].sub_queue_id ;
            if(sub_queue_id != CTC_MAX_UINT16_VALUE)
            {
                valid = 1;
            }
            else
            {
                sub_queue_id = 0;
            }
            for(; !valid &&(sub_queue_id < max_queue_num) ; sub_queue_id ++)
            {
                for(reason = 1; reason < CTC_PKT_CPU_REASON_MAX_COUNT; reason++)
                {
                    if(p_usw_queue_master[lchip]->cpu_reason[reason].sub_queue_id == sub_queue_id)
                    {/*in-use*/
                        break;
                    }
                }
                if(reason == CTC_PKT_CPU_REASON_MAX_COUNT)
                {
                    break;
                }
            }
            if(sub_queue_id == max_queue_num)
            {
                return CTC_E_INVALID_PARAM;
            }

            reason = CTC_PKT_CPU_REASON_QUEUE_DROP_PKT;
            group_id = sub_queue_id / SYS_USW_MAX_QNUM_PER_GROUP;
            queue_id = sub_queue_id % SYS_USW_MAX_QNUM_PER_GROUP;
            CTC_ERROR_RETURN(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id));

            sub_queue_id = sub_queue_id + MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);
            nexthop_ptr = CTC_PKT_CPU_REASON_QUEUE_DROP_PKT << 4;

            queue_index = sub_queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP);
            field_val = 0;
            field_id = DsQMgrDmaSchMap_chanNodeSel_f;
            cmd = DRV_IOW(DsQMgrDmaSchMap_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_index, cmd, &field_val));
        }
        else
        {
            cmd = DRV_IOR(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM) - 1, cmd, &grp_chan_map));
            SetDsQMgrGrpChanMap(V, channelId_f, &grp_chan_map, dst_chan_id);
            cmd = DRV_IOW(DsQMgrGrpChanMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM) - 1, cmd, &grp_chan_map));
            sub_queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM) - 1;

            CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, &nexthop_ptr));
        }

        cmd = DRV_IOR(BufRetrvSpanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_retrieve_ctl))
        SetBufRetrvSpanCtl(V, spanOnDropRsvQueId_f , &buffer_retrieve_ctl, sub_queue_id );
        SetBufRetrvSpanCtl(V, spanOnDropEn_f , &buffer_retrieve_ctl, 1);
        SetBufRetrvSpanCtl(V, spanOnDropNextHopPtr_f , &buffer_retrieve_ctl, nexthop_ptr);
        cmd = DRV_IOW(BufRetrvSpanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_retrieve_ctl));

        p_usw_queue_master[lchip]->monitor_drop_en = 1;
        field_val = 1;
        cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_spanOnDropEnable_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOR(ErmSpanOnDropCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &span_on_drop_ctl));

        GetErmSpanOnDropCtl(A, spanOnDropChannelEn_f, &span_on_drop_ctl, array_val);
        CTC_BIT_SET(array_val[src_chan_id / 32], src_chan_id&0x1F);
        SetErmSpanOnDropCtl(V, spanOnDropEn_f , &span_on_drop_ctl, 1);
        SetErmSpanOnDropCtl(V, spanOnDropTrafficClass_f , &span_on_drop_ctl, 0xFF);
        SetErmSpanOnDropCtl(A, spanOnDropChannelEn_f, &span_on_drop_ctl, array_val);
        SetErmSpanOnDropCtl(V, spanOnDropQueueId_f , &span_on_drop_ctl, sub_queue_id);
        SetErmSpanOnDropCtl(V, spanOnDropChannelId_f , &span_on_drop_ctl, dst_chan_id);
        cmd = DRV_IOW(ErmSpanOnDropCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &span_on_drop_ctl));
    }
    else
    {
        p_usw_queue_master[lchip]->monitor_drop_en = 0;
        cmd = DRV_IOR(ErmSpanOnDropCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &span_on_drop_ctl));

        /*1.delete span src*/
        GetErmSpanOnDropCtl(A, spanOnDropChannelEn_f, &span_on_drop_ctl, array_val);
        CTC_BIT_UNSET(array_val[src_chan_id / 32], src_chan_id&0x1F);
        SetErmSpanOnDropCtl(V, spanOnDropTrafficClass_f , &span_on_drop_ctl, 0xFF);
        SetErmSpanOnDropCtl(A, spanOnDropChannelEn_f, &span_on_drop_ctl, array_val);
        cmd = DRV_IOW(ErmSpanOnDropCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &span_on_drop_ctl));
        if(array_val[0] != 0 || array_val[1] != 0)
        {
            return CTC_E_NONE;
        }

        /*2.delete span dest */
        if(is_cpu_port == 0)
        {
            cmd = DRV_IOW(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
            dst_chan_id = 0;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM) - 1, cmd, &dst_chan_id));
        }

        cmd = DRV_IOR(BufRetrvSpanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_retrieve_ctl))
        SetBufRetrvSpanCtl(V, spanOnDropEn_f , &buffer_retrieve_ctl, 0);
        cmd = DRV_IOW(BufRetrvSpanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_retrieve_ctl));

        field_val = 0;
        cmd = DRV_IOW(ErmSpanOnDropCtl_t, ErmSpanOnDropCtl_spanOnDropEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(ErmSpanOnDropCtl_t, ErmSpanOnDropCtl_spanOnDropChannelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_spanOnDropEnable_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    return CTC_E_NONE;
}

extern int32
sys_usw_qos_get_monitor_drop_queue_id(uint8 lchip,  ctc_qos_queue_drop_monitor_t* drop_monitor)
{
    uint32 array_val[2] = {0};
    uint32 cmd = 0;
    uint8 src_chan_id = 0;
    uint8 dst_chan_id = 0;
    uint32 src_gport = 0;
    uint16 lport = 0;
    uint8 gchip = 0;
    ErmSpanOnDropCtl_m span_on_drop_ctl;

    sal_memset(&span_on_drop_ctl, 0, sizeof(span_on_drop_ctl));
    src_gport = drop_monitor->src_gport;
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(src_gport, lchip, lport);

    src_chan_id = SYS_GET_CHANNEL_ID(lchip, src_gport);
    if (src_chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }
    cmd = DRV_IOR(ErmSpanOnDropCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &span_on_drop_ctl));

    GetErmSpanOnDropCtl(A, spanOnDropChannelEn_f, &span_on_drop_ctl, array_val);
    drop_monitor->enable = CTC_IS_BIT_SET(array_val[src_chan_id / 32], src_chan_id&0x1F);
    if(drop_monitor->enable == 0)
    {
        return CTC_E_NONE;
    }
    dst_chan_id = GetErmSpanOnDropCtl(V, spanOnDropChannelId_f , &span_on_drop_ctl);
    if(dst_chan_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0))
    {
        drop_monitor->dst_gport = 0x10000000;
    }
    else
    {
        if(p_usw_queue_master[lchip]->enq_mode == 1 && DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }
        lport = SYS_GET_LPORT_ID_WITH_CHAN(lchip, dst_chan_id);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        drop_monitor->dst_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    }

    return CTC_E_NONE;
}
/**
 @brief Get service queue statistics.
*/
int32
sys_usw_queue_stats_query(uint8 lchip, ctc_qos_queue_stats_t* p_stats_param)
{
    sys_stats_queue_t stats;
    ctc_qos_queue_stats_info_t* p_stats = NULL;
    uint16 queue_id = 0;
    uint32 stats_ptr = 0;
    uint32 cmd = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,
                                                       &p_stats_param->queue,
                                                       &queue_id));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Queue id = %d\n", queue_id);

    p_stats = &p_stats_param->stats;
    if (DRV_IS_DUET2(lchip))
    {
        stats_ptr = queue_id << 1;
    }
    else
    {
        cmd = DRV_IOR(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &stats_ptr));
    }
    CTC_ERROR_RETURN(sys_usw_flow_stats_get_queue_stats(lchip, stats_ptr+1, &stats));
    p_stats->drop_packets = stats.packets;
    p_stats->drop_bytes   = stats.bytes;

    CTC_ERROR_RETURN(sys_usw_flow_stats_get_queue_stats(lchip, stats_ptr, &stats));
    p_stats->deq_packets  = stats.packets;
    p_stats->deq_bytes    = stats.bytes;

    return CTC_E_NONE;
}

/**
 @brief Get service queue statistics.
*/
int32
sys_usw_queue_stats_clear(uint8 lchip, ctc_qos_queue_stats_t* p_stats_param)
{
    uint16 queue_id = 0;
    uint32 stats_ptr = 0;
    uint32 cmd = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &p_stats_param->queue,
                                                       &queue_id));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Queue id = %d\n", queue_id);

    if (DRV_IS_DUET2(lchip))
    {
        stats_ptr = queue_id << 1;
    }
    else
    {
        cmd = DRV_IOR(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &stats_ptr));
    }
    CTC_ERROR_RETURN(sys_usw_flow_stats_clear_queue_stats(lchip, stats_ptr+1));
    CTC_ERROR_RETURN(sys_usw_flow_stats_clear_queue_stats(lchip, stats_ptr));

    return CTC_E_NONE;
}

int32
sys_usw_qos_queue_dump_status(uint8 lchip)
{

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "---------------------Queue allocation---------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Queue number per port", p_usw_queue_master[lchip]->queue_num_per_chanel);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "----------------------------------------------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10s %-8s\n", "type", "port-id", "Queue Range", "Qbase", "SubQNum");

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-----------------------------------------------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "network port", "0~63", "0~511",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NORMAL), p_usw_queue_master[lchip]->queue_num_per_chanel);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "iloop", "alloc", "512~639",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC), 8);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "eloop", "alloc", "512~639",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC), 8);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10d %-10s %-10d %8d\n", "Rx OAM", SYS_RSV_PORT_OAM_CPU_ID, "512~639",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC), 8);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "Local CPU(DMA)", "-", "640~767",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP), 8);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10s %8d\n", "Local CPU(ETH)", "specific", "0",
                       "-",
                       p_usw_queue_master[lchip]->queue_num_per_chanel);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "ctl queue", "0~63", "768~1023",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC), 4);
    if (DRV_IS_DUET2(lchip))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "extend queue", "0~63", "1024~1279",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT), MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP));
    }
    else
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-15s %-10s %-10s %-10d %8d\n", "extend queue", "0~63", "1024~4095",
                       MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT), MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP));
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\n");

    return CTC_E_NONE;
}

int32
_sys_usw_qos_dump_status(uint8 lchip)
{
    sys_usw_qos_policer_dump_status(lchip);
    sys_usw_qos_queue_dump_status(lchip);
    sys_usw_qos_shape_dump_status(lchip);
    sys_usw_qos_sched_dump_status(lchip);
    sys_usw_qos_drop_dump_status(lchip);

    return CTC_E_NONE;
}

int32
_sys_usw_qos_queue_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    uint16 queue_id = 0;
    uint8 que_offset = 0;
    uint32 cmd = 0;
    uint8 channel = 0;
    uint8 stats_en = 0;
    uint32 token_thrd = 0;
    uint32 token_rate = 0;
    uint8 is_pps = 0;
    uint16 ext_grp_id = 0;
    sys_queue_node_t* p_queue_node = NULL;
    sys_queue_group_node_t* p_group_node = NULL;
    sys_group_meter_profile_t* p_meter_profile = NULL;

    CTC_MAX_VALUE_CHECK(start, MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM) - 1);
    CTC_MAX_VALUE_CHECK(end, MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM) - 1);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\nQueue information:\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-7s %-7s %-7s %-7s %-8s %-7s\n", "queue", "offset", "channel", "group","stats-en","shp-en");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "---------------------------------------------------------\n");

    for (queue_id = start; queue_id <= end && queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM); queue_id++)
    {
        p_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec,queue_id);
        if (NULL == p_queue_node)
        {
            que_offset = queue_id % 8;
            continue;
        }
        else
        {
            que_offset = p_queue_node->offset;
        }

        CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip, queue_id, &channel));

        CTC_ERROR_RETURN(_sys_usw_queue_get_stats_enable(lchip, queue_id, &stats_en));

        if (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id))
        {
            if (SYS_IS_EXT_QUEUE(queue_id))
            {
                SYS_EXT_GROUP_ID(queue_id, ext_grp_id);
                p_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, ext_grp_id);
                if (NULL == p_group_node)
                {
                    continue;
                }
                if (p_group_node->chan_id == 0xFF)
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-7d %-7d %-7s %-7d %-8d %-7d\n", queue_id, que_offset, "-",
                                          SYS_GROUP_ID(queue_id), stats_en, p_queue_node->shp_en);
                }
                else
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-7d %-7d %-7d %-7d %-8d %-7d\n", queue_id, que_offset, p_group_node->chan_id,
                                          SYS_GROUP_ID(queue_id), stats_en, p_queue_node->shp_en);
                }
            }
            else
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-7d %-7d %-7d %-7d %-8d %-7d\n", queue_id, que_offset, channel,
                                          SYS_GROUP_ID(queue_id), stats_en, p_queue_node->shp_en);
            }
        }
        else
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-7d %-7d %-7d %-7s %-8d %-7d\n", queue_id, que_offset, channel, "-",
                                  stats_en, p_queue_node->shp_en);
        }
    }

    if (start == end)
    {
        queue_id = start;
    }

    if ((start == end) && p_queue_node)
    {
        is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\nQueue shape:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------------\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %s\n", "Shape mode", is_pps?"PPS":"BPS");

        if (SYS_IS_EXT_QUEUE(queue_id))
        {
            SYS_EXT_GROUP_ID(queue_id, ext_grp_id);
            p_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, ext_grp_id);
            if (NULL == p_group_node)
            {
                return CTC_E_NONE;
            }

            p_meter_profile = p_group_node->p_meter_profile;

            if (!p_queue_node->shp_en || (p_meter_profile == NULL) || p_meter_profile->c_bucket[que_offset].cir_type == SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_FAIL)
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "No CIR Bucket, all packet always mark as yellow!\n");
            }
            else if (p_meter_profile->c_bucket[que_offset].cir_type == SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER)
            {
                sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_meter_profile->c_bucket[que_offset].rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                      p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
                _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_meter_profile->c_bucket[que_offset].thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
                SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d (kbps/pps)\n", "CIR", token_rate);
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d (kb/pkt)\n", "CBS", token_thrd);
            }
        }

        if (p_queue_node->shp_en && p_queue_node->p_shp_profile)
        {
            if (SYS_IS_NETWORK_BASE_QUEUE(queue_id) && p_queue_node->p_meter_profile)
            {
                sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_queue_node->p_meter_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                      p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
                _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_queue_node->p_meter_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
                SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
                if (token_rate)
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d (kbps/pps)\n", "CIR", token_rate);
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d (kb/pkt)\n", "CBS", token_thrd);
                }
                else
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "No CIR Bucket, all packet always mark as yellow!\n");
                }
            }
            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_queue_node->p_shp_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                    p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
            _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_queue_node->p_shp_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d (kbps/pps)\n", "PIR", token_rate);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d (kb/pkt)\n", "PBS", token_thrd);
        }
        else
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "No PIR Bucket\n");
        }
    }

    if (start == end)
    {
        ctc_qos_sched_group_t grp_sched;
        ctc_qos_sched_queue_t  queue_sched;

        sal_memset(&queue_sched, 0,  sizeof(queue_sched));
        sal_memset(&grp_sched, 0, sizeof(grp_sched));
        queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
        CTC_ERROR_RETURN(sys_usw_queue_sch_get_queue_sched(lchip, queue_id , &queue_sched));

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\nQueue Scheduler:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------------\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d \n", "Queue Class(Yellow)", queue_sched.exceed_class);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d \n", "Weight", queue_sched.exceed_weight);

        if (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id))
        {
            grp_sched.queue_class = queue_sched.exceed_class;
            CTC_ERROR_RETURN(sys_usw_queue_sch_get_group_sched(lchip, SYS_GROUP_ID(queue_id), &grp_sched));
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "Queue Class(Yellow) %d priority: %d\n", queue_sched.exceed_class, grp_sched.class_priority);
        }
    }

    if (start == end)
    {
        uint8 wred_profile = 0;
        uint8 wred = 0;
        uint8 wtd  = 0;
        uint8 value = 0;
        uint32 index = 0;
        uint16 wredMaxThrd[4];
        uint16 wredMinThrd[4];
        uint16 ecnthreshold[4];
        uint16 drop_prob[4];
        DsErmAqmQueueCfg_m erm_aqm_queue_cfg;
        DsErmQueueCfg_m erm_queue_cfg;
        DsErmQueueLimitedThrdProfile_m erm_queue_thrd_profile;
        DsErmAqmThrdProfile_m erm_aqm_thrd_profile;
        uint8 cngest = 0;
        uint8 sc = 0;
        uint8 tc = 0;
        uint32 field_value = 0;
        uint8 drop_factor[16] = {7,8,9,10,0,0,0,0,0,6,5,4,3,2,1,0};
        uint8 cngest_level = 0;
        uint8 temp_value = 0;

        sal_memset(&erm_aqm_queue_cfg, 0, sizeof(DsErmAqmQueueCfg_m));
        sal_memset(&erm_queue_cfg, 0, sizeof(DsErmQueueCfg_m));
        sal_memset(&erm_queue_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));
        sal_memset(&erm_aqm_thrd_profile, 0, sizeof(DsErmAqmThrdProfile_m));

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\nQueue Drop:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------------\n");

        if (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id))
        {
            index = (queue_id >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))
                    ? ((queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) + MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC))
                    : queue_id;
            cmd = DRV_IOR(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_queue_cfg));

            wred_profile = GetDsErmAqmQueueCfg(V, queueThrdProfId_f , &erm_aqm_queue_cfg);
            wred = (wred_profile != MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM));
        }

        sys_usw_queue_get_sc_tc(lchip, p_queue_node, &sc, &tc);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Mapped tc", tc);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Mapped sc", sc);
        cngest_level = p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num;

        if (wred)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %s\n", "Drop Mode", "WRED");
            cmd = DRV_IOR(DsErmAqmThrdProfile_t, DRV_ENTRY_FLAG);
            index = wred_profile;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_thrd_profile));

            wredMaxThrd[0] = GetDsErmAqmThrdProfile(V, g_0_wredAvgMaxLen_f , &erm_aqm_thrd_profile);
            wredMaxThrd[1] = GetDsErmAqmThrdProfile(V, g_1_wredAvgMaxLen_f , &erm_aqm_thrd_profile);
            wredMaxThrd[2] = GetDsErmAqmThrdProfile(V, g_2_wredAvgMaxLen_f , &erm_aqm_thrd_profile);
            wredMaxThrd[3] = GetDsErmAqmThrdProfile(V, g_3_wredAvgMaxLen_f , &erm_aqm_thrd_profile);
            wredMinThrd[0] = GetDsErmAqmThrdProfile(V, g_0_wredAvgMinLen_f , &erm_aqm_thrd_profile);
            wredMinThrd[1] = GetDsErmAqmThrdProfile(V, g_1_wredAvgMinLen_f , &erm_aqm_thrd_profile);
            wredMinThrd[2] = GetDsErmAqmThrdProfile(V, g_2_wredAvgMinLen_f , &erm_aqm_thrd_profile);
            wredMinThrd[3] = GetDsErmAqmThrdProfile(V, g_3_wredAvgMinLen_f , &erm_aqm_thrd_profile);
            drop_prob[0] = GetDsErmAqmThrdProfile(V, g_0_wredProbFactor_f , &erm_aqm_thrd_profile);
            drop_prob[1] = GetDsErmAqmThrdProfile(V, g_1_wredProbFactor_f , &erm_aqm_thrd_profile);
            drop_prob[2] = GetDsErmAqmThrdProfile(V, g_2_wredProbFactor_f , &erm_aqm_thrd_profile);
            drop_prob[3] = GetDsErmAqmThrdProfile(V, g_3_wredProbFactor_f , &erm_aqm_thrd_profile);

            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-29s : /%-8u/%-8u/%-8u\n", "MaxThrd",  wredMaxThrd[0] * 8, wredMaxThrd[1] * 8, wredMaxThrd[2] * 8);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-29s : /%-8u/%-8u/%-8u\n", "MinThrd",  wredMinThrd[0] * 8, wredMinThrd[1] * 8, wredMinThrd[2] * 8);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-29s : /%-8u/%-8u/%-8u \n", "Drop_prob", drop_prob[0], drop_prob[1], drop_prob[2]);
        }

        index = queue_id;
        cmd = DRV_IOR(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
        value = GetDsErmQueueCfg(V, queueLimitedThrdProfId_f , &erm_queue_cfg);
        wtd = (value != MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WTD_PROFILE_NUM));

        if (wtd)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %s\n", "Drop Mode", "WTD");
            cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DsErmQueueLimitedThrdProfile_g_0_queueLimitedAutoEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (value << 2), cmd, &field_value));
            if(field_value == 1)
            {
                cngest_level = 1;
                cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (value << 2), cmd, &erm_queue_thrd_profile));
                wredMaxThrd[0] = GetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedFactor_f , &erm_queue_thrd_profile);
                wredMaxThrd[1] = GetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedFactor_f , &erm_queue_thrd_profile);
                wredMaxThrd[2] = GetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedFactor_f , &erm_queue_thrd_profile);
                wredMaxThrd[3] = GetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedFactor_f , &erm_queue_thrd_profile);

                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: /%-8u/%-8u/%-8u\n",
                "DropFactor", drop_factor[wredMaxThrd[0]], drop_factor[wredMaxThrd[1]], drop_factor[wredMaxThrd[2]]);
            }
            else if(cngest_level > 0)
            {
                for (cngest = 0; cngest < cngest_level; cngest++)
                {
                    temp_value = (p_usw_queue_master[lchip]->resrc_check_mode != 0) ? (cngest + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-cngest_level)):0;
                    cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (value << 2) + temp_value, cmd, &erm_queue_thrd_profile));

                    wredMaxThrd[0] = GetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedThrd_f , &erm_queue_thrd_profile);
                    wredMaxThrd[1] = GetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedThrd_f , &erm_queue_thrd_profile);
                    wredMaxThrd[2] = GetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedThrd_f , &erm_queue_thrd_profile);
                    wredMaxThrd[3] = GetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedThrd_f , &erm_queue_thrd_profile);

                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "Congest:%d %-19s : /%-8u/%-8u/%-8u\n",
                                                    cngest, "DropThrd", wredMaxThrd[0] * 8, wredMaxThrd[1] * 8, wredMaxThrd[2] * 8);
                }

            }
            if(cngest_level > 0)
            {
                for (cngest = 0; cngest < cngest_level; cngest++)
                {
                    temp_value = (p_usw_queue_master[lchip]->resrc_check_mode != 0) ? (cngest + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-cngest_level)):0;
                    cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (value << 2) + temp_value, cmd, &erm_queue_thrd_profile));

                    ecnthreshold[0] = GetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &erm_queue_thrd_profile);
                    ecnthreshold[1] = GetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &erm_queue_thrd_profile);
                    ecnthreshold[2] = GetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &erm_queue_thrd_profile);
                    ecnthreshold[3] = GetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &erm_queue_thrd_profile);

                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "Congest:%d %-19s : /%-8u/%-8u/%-8u\n",
                                                    cngest, "EcnThrd", ecnthreshold[0] * 8, ecnthreshold[1] * 8, ecnthreshold[2] * 8);
                }
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_group_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    uint16 group_id = 0;
    uint32 token_thrd = 0;
    uint32 token_rate = 0;
    uint8 loop = 0;
    uint8 loop_num = 0;
    uint8 is_pps = 0;
    sys_queue_group_node_t  * p_group_node = NULL;

    CTC_MAX_VALUE_CHECK(start, (MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)+ MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM)) - 1);
    CTC_MAX_VALUE_CHECK(end, (MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)+ MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM)) - 1);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "Group information:\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-10s %-10s", "group", "channel");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-8s %-15s %-15s ", "shp_en", "pir(kbps/pps)", "pbs(kb/pkt)");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "---------------------------------------------------------------\n");

    for (group_id = start; group_id <= end && group_id < MCHIP_CAP(SYS_CAP_QOS_GROUP_NUM); group_id++)
    {
        token_thrd = 0;
        token_rate = 0;
        if (group_id < MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM))
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-10d %-10d %-8s %-15s %-15s\n", group_id,
                                  group_id, "-", "-", "-");
        }
        else
        {
            p_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, (group_id - MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)));
            if (NULL == p_group_node)
            {
                continue;
            }
            is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, p_group_node->chan_id);
            if (p_group_node->shp_bitmap && p_group_node->p_shp_profile)
            {
                sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_group_node->p_shp_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                      p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
                _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd, p_group_node->p_shp_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            }
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            if (p_group_node->chan_id == 0xFF)
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-10d %-10s %-8d %-15u %-15u\n", group_id,
                                      "-", CTC_IS_BIT_SET(p_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET)), token_rate, token_thrd);
            }
            else
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-10d %-10d %-8d %-15u %-15u\n", group_id,
                                      p_group_node->chan_id, CTC_IS_BIT_SET(p_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET)), token_rate, token_thrd);
            }
        }
    }

    if (start == end)
    {
        ctc_qos_sched_group_t  group_sched;
        sal_memset(&group_sched, 0, sizeof(ctc_qos_sched_group_t));
        group_id = start;

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\nGroup Scheduler:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------------\n");

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-12s %-10s %-6s\n", "Queue-Class", "Class-PRI", "Weight");

        loop_num = (group_id >= MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)) ? MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP) : MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_CHAN) ;

        for (loop = 0;  loop < loop_num; loop++)
        {
            group_sched.queue_class = loop;
            CTC_ERROR_RETURN(sys_usw_queue_sch_get_group_sched(lchip, group_id, &group_sched));
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-12d %-10d %-6d\n", loop, group_sched.class_priority, group_sched.weight);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_port_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    uint8  channel = 0;
    uint16 port = 0;
    uint16 lport = 0;
    uint32 cmd;
    uint8  shp_en = 0;
    uint32 token_thrd = 0;
    uint32 token_thrd_shift = 0;
    uint32 token_rate = 0;
    uint32 token_rate_frac = 0;
    uint8 is_pps = 0;
    uint16 lport_start = 0;
    uint16 lport_end = 0;
    uint8 gchip = 0;
    uint16 group_id = 0;
    uint32 index = 0;
    uint8 bpe_en = 0;
    uint32 gport = 0;
    sys_queue_group_node_t* p_group_node = NULL;

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(start);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(start, lchip, lport_start);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(end, lchip, lport_end);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "Port information:\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-10s %-10s %-10s %-15s %-15s %-7s\n","port","channel", "shp-en", "pir(kbps/pps)", "pbs(kb/pkt)","shp_mode");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-----------------------------------------------------------------\n");

    for (port = lport_start; port <= lport_end && port < SYS_USW_MAX_PORT_NUM_PER_CHIP; port++)
    {
        lport = port;

        gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
        sys_usw_qos_get_mux_port_enable(lchip, gport, &bpe_en);
        _sys_usw_get_channel_by_port(lchip, gport, &channel);
        if (bpe_en)
        {
            sys_extend_que_node_t port_extender_node;
            sys_extend_que_node_t* p_port_extender_node = NULL;
            sal_memset(&port_extender_node, 0, sizeof(port_extender_node));
            port_extender_node.type = SYS_EXTEND_QUE_TYPE_BPE;
            port_extender_node.lchip = lchip;
            port_extender_node.lport = lport;
            port_extender_node.channel = channel;
            p_port_extender_node = sys_usw_port_extender_lookup(lchip, &port_extender_node);
            if (NULL == p_port_extender_node)
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
                return CTC_E_NOT_EXIST;
            }

            group_id = p_port_extender_node->group_id;
            p_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, group_id);
            if (NULL == p_group_node)
            {
                continue;
            }

            is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, p_group_node->chan_id);

            /*default value for token_rate and token_thrd*/
            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, (0x3FFFFF << 8 | 0xFF), &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                    p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
            _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd ,
                                                    (0xFF << MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH) | 0x1F), MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));

            if (p_group_node->shp_bitmap && p_group_node->p_shp_profile)
            {
                sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_group_node->p_shp_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                        p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
                _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_group_node->p_shp_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            }
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "0x%04x     %-10d %-10d %-15u %-15u %-7s\n",
                                                    SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport), p_group_node->chan_id,
            CTC_IS_BIT_SET(p_group_node->shp_bitmap, MCHIP_CAP(SYS_CAP_QOS_QUEUE_PIR_BUCKET)), token_rate, token_thrd, is_pps ? "PPS" : "BPS");
        }
        else
        {
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
            else if (channel < MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0))
            {
                DsQMgrMiscChanShpProfile_m  misc_chan_shp_profile;
                sal_memset(&misc_chan_shp_profile, 0, sizeof(DsQMgrMiscChanShpProfile_m));
                cmd = DRV_IOR(DsQMgrMiscChanShpProfile_t, DRV_ENTRY_FLAG);
                index = channel - MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &misc_chan_shp_profile));

                token_thrd = GetDsQMgrMiscChanShpProfile(V, tokenThrd_f, &misc_chan_shp_profile);
                token_thrd_shift = GetDsQMgrMiscChanShpProfile(V, tokenThrdShift_f, &misc_chan_shp_profile);
                token_rate_frac = GetDsQMgrMiscChanShpProfile(V, tokenRateFrac_f, &misc_chan_shp_profile);
                token_rate = GetDsQMgrMiscChanShpProfile(V, tokenRate_f, &misc_chan_shp_profile);
            }
            else if (channel >= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0) && channel <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3))
            {
                QMgrDmaChanShpProfile_m  dma_chan_shp_profile;
                sal_memset(&dma_chan_shp_profile, 0, sizeof(QMgrDmaChanShpProfile_m));

                cmd = DRV_IOR(QMgrDmaChanShpProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_chan_shp_profile));

                token_thrd = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrd_f, &dma_chan_shp_profile);
                token_thrd_shift = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrdShift_f, &dma_chan_shp_profile);
                token_rate_frac = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenRateFrac_f, &dma_chan_shp_profile);
                token_rate = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenRate_f, &dma_chan_shp_profile);
            }
            else
            {
                token_thrd = MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD);
                token_thrd_shift = MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT);
                token_rate_frac = MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_FRAC);
                token_rate = MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE);
            }

            is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);
            shp_en = (token_thrd_shift == MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT)) ? 0 : 1;

            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, (token_rate << 8 | token_rate_frac)  , &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                    p_usw_queue_master[lchip]->chan_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
            _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd ,
                                                    (token_thrd << MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH) | token_thrd_shift), MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "0x%04x     %-10d %-10d %-10u %-10u %-7s\n",
                                                    SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, port), channel, shp_en, token_rate, token_thrd, is_pps ? "PPS" : "BPS");
        }
    }

    if (start == end)
    {
        uint16 queue_id = 0;
        uint16 queue_index = 0;
        uint32 queue_depth = 0;
        uint8  queue_num = 0;
        uint8  stacking_queue_en = 0;
        ctc_qos_queue_id_t queue;
        sal_memset(&queue, 0, sizeof(ctc_qos_queue_id_t));

        sys_usw_qos_get_mux_port_enable(lchip, gport, &bpe_en);
        _sys_usw_queue_get_stacking_enable(lchip, channel, &stacking_queue_en);
        if (stacking_queue_en)
        {
            queue_num = p_usw_queue_master[lchip]->queue_num_per_chanel + SYS_C2C_QUEUE_NUM;
        }
        else
        {
            queue_num = bpe_en ? p_usw_queue_master[lchip]->queue_num_per_extend_port : p_usw_queue_master[lchip]->queue_num_per_chanel;
        }

        queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        queue.gport = start;

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-10s %-10s\n","queue-id","queue-depth");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");

        for (queue_index = 0; queue_index < queue_num; queue_index++)
        {
            queue.queue_id = queue_index;
            CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &queue, &queue_id));

            cmd = DRV_IOR(DsErmQueueCnt_t, DsErmQueueCnt_queueCnt_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &queue_depth));

            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d %-10d\n", queue_id, queue_depth);

        }
    }

    return CTC_E_NONE;
}
int32
_sys_usw_qos_service_dump(uint8 lchip, uint16 service_id, uint32 gport)
{
    uint16 queue_id = 0;
    uint16 queue_index = 0;
    uint32 queue_depth = 0;
    uint32 cmd = 0;
    ctc_qos_queue_id_t queue;
    sal_memset(&queue, 0, sizeof(ctc_qos_queue_id_t));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"show service information:\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"============================================================\n");

    queue.queue_type = CTC_QUEUE_TYPE_SERVICE_INGRESS;
    queue.gport = gport;
    queue.service_id = service_id;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %-10s\n", "queue-id", "queue-depth");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------\n");

    for (queue_index = 0; queue_index < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); queue_index++)
    {
        queue.queue_id = queue_index;
        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &queue, &queue_id));

        cmd = DRV_IOR(DsErmQueueCnt_t, DsErmQueueCnt_queueCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &queue_depth));

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10d %-10d\n", queue_id, queue_depth);

    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"============================================================\n");

    return CTC_E_NONE;

}

/*for sdk cosim pass*/
int32
sys_usw_queue_init_qmgr(uint8 lchip)
{
    uint32 cmd = 0;
    /*uint8 gchip = 0;*/
    uint32 value = 0;
    uint16 chan_num = 0;
    QWriteCtl_m  q_write_ctl;
    QMgrCtl_m qmgr_ctl;
    /*BufferRetrieveCtl_m buffer_retrieve_ctl;*/
    QWriteOamMiscCtl_m q_oam_ctl;

    sal_memset(&q_write_ctl, 0, sizeof(QWriteCtl_m));
    sal_memset(&qmgr_ctl, 0, sizeof(QMgrCtl_m));
    sal_memset(&q_oam_ctl, 0, sizeof(QWriteOamMiscCtl_m));

    cmd = DRV_IOR(QWriteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_write_ctl));

    cmd = DRV_IOR(QMgrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qmgr_ctl));

    sys_usw_dma_get_packet_rx_chan(lchip, &chan_num);

    if (chan_num == 4)
    {
        SetQWriteCtl(V, cpuQueueMode_f, &q_write_ctl, 1);
        SetQMgrCtl(V, cpuQueueMode_f  , &qmgr_ctl, 1);
    }
    else
    {
        SetQWriteCtl(V, cpuQueueMode_f, &q_write_ctl, 0);
        SetQMgrCtl(V, cpuQueueMode_f  , &qmgr_ctl, 0);
    }

    /*normal stats mode*/
    SetQWriteCtl(V, statsMode_f, &q_write_ctl, 0);
    if (DRV_IS_DUET2(lchip))
    {
        SetQWriteCtl(V, oamFromCpu_f, &q_write_ctl, 1);
    }
    cmd = DRV_IOW(QWriteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_write_ctl));

    cmd = DRV_IOW(QMgrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qmgr_ctl));
/*
    sal_memset(&buffer_retrieve_ctl, 0xFF, sizeof(BufferRetrieveCtl_m));
    sys_usw_get_gchip_id(lchip, &gchip);
    SetBufferRetrieveCtl(V, truncationLenShift_f  , &buffer_retrieve_ctl, 4);
    cmd = DRV_IOW(BufferRetrieveCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_retrieve_ctl));
*/
    /*QueueAssignmentHyperMode*/
    value = 0;
    cmd = DRV_IOW(FwdLatencyCtl_t, FwdLatencyCtl_queueAssignmentHyperModeDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOR(QWriteOamMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_oam_ctl));

    SetQWriteOamMiscCtl(V, g_1_oamMappedTc_f, &q_oam_ctl, 0);
    SetQWriteOamMiscCtl(V, g_1_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_2_oamMappedTc_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_2_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_6_oamMappedTc_f, &q_oam_ctl, 2);
    SetQWriteOamMiscCtl(V, g_6_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_7_oamMappedTc_f, &q_oam_ctl, 3);
    SetQWriteOamMiscCtl(V, g_7_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_8_oamMappedTc_f, &q_oam_ctl, 4);
    SetQWriteOamMiscCtl(V, g_8_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_10_oamMappedTc_f, &q_oam_ctl, 5);
    SetQWriteOamMiscCtl(V, g_10_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_11_oamMappedTc_f, &q_oam_ctl, 6);
    SetQWriteOamMiscCtl(V, g_11_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, g_12_oamMappedTc_f, &q_oam_ctl, 7);
    SetQWriteOamMiscCtl(V, g_12_oamRemapEn_f, &q_oam_ctl, 1);
    SetQWriteOamMiscCtl(V, ignoreNetChannel_f, &q_oam_ctl, 0);
    cmd = DRV_IOW(QWriteOamMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_oam_ctl));
    /* aqm scan pri is high, affect performance */
    cmd = DRV_IOW(QMgrEnqMiscCtl_t, QMgrEnqMiscCtl_cfgAqmScanLowPrioEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* fix tm gg stacking bug 110006*/
    cmd = DRV_IOR(EpeHdrEditReserved_t, EpeHdrEditReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    SET_BIT(value, 15);
    value &= 0xffff8fff;
    value |= 0x6000;
    cmd = DRV_IOW(EpeHdrEditReserved_t, EpeHdrEditReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_queue_traverse_fprintf_pool(void *node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    uint32*                     cnt         = (uint32 *)(user_date->value1);
    uint32*                     mode        = (uint32 *)user_date->value2;
    uint8                       i           = 0;

    if(*mode == SYS_QUEUE_PROFILE)
    {
        sys_queue_shp_profile_t *node_profile = (sys_queue_shp_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-15u%-15u%-15u%-7u\n",*cnt,node_profile->queue_type,node_profile->bucket_thrd,node_profile->bucket_rate,((ctc_spool_node_t*)node)->ref_cnt);
    }
    else if(*mode == SYS_QUEUE_METER_PROFILE)
    {
        sys_queue_meter_profile_t *node_profile = (sys_queue_meter_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-15u%-15u%-7u\n",*cnt,node_profile->bucket_thrd,node_profile->bucket_rate,((ctc_spool_node_t*)node)->ref_cnt);
    }
    else if(*mode == SYS_QUEUE_DROP_WTD_PROFILE)
    {
        sys_queue_drop_wtd_profile_t *node_profile =  (sys_queue_drop_wtd_profile_t *)(((ctc_spool_node_t*)node)->data);
        if(((ctc_spool_node_t*)node)->ref_cnt == 0xFFFFFFFF)
        {
            SYS_DUMP_DB_LOG(p_file, "%-7u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-8ustatic\n",*cnt,node_profile->profile[0].ecn_mark_thrd0,node_profile->profile[0].ecn_mark_thrd1,node_profile->profile[0].ecn_mark_thrd2,
                    node_profile->profile[0].ecn_mark_thrd3,node_profile->profile[0].wred_max_thrd3,node_profile->profile[0].wred_max_thrd2,node_profile->profile[0].wred_max_thrd1,node_profile->profile[0].wred_max_thrd0,
                    node_profile->profile[0].drop_thrd_factor3,node_profile->profile[0].drop_thrd_factor2,node_profile->profile[0].drop_thrd_factor1,node_profile->profile[0].drop_thrd_factor0);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_file, "%-7u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-8u%-6u\n",*cnt,node_profile->profile[0].ecn_mark_thrd0,node_profile->profile[0].ecn_mark_thrd1,node_profile->profile[0].ecn_mark_thrd2,
                    node_profile->profile[0].ecn_mark_thrd3,node_profile->profile[0].wred_max_thrd3,node_profile->profile[0].wred_max_thrd2,node_profile->profile[0].wred_max_thrd1,node_profile->profile[0].wred_max_thrd0,
                    node_profile->profile[0].drop_thrd_factor3,node_profile->profile[0].drop_thrd_factor2,node_profile->profile[0].drop_thrd_factor1,node_profile->profile[0].drop_thrd_factor0,((ctc_spool_node_t*)node)->ref_cnt);
        }
    }
    else if(*mode == SYS_QUEUE_DROP_WRED_PROFILE)
    {
        sys_queue_drop_wred_profile_t *node_profile =  (sys_queue_drop_wred_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-7u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-8u%-6u\n",*cnt,node_profile->wred_min_thrd3,node_profile->wred_min_thrd2,node_profile->wred_min_thrd1,
                    node_profile->wred_min_thrd0,node_profile->wred_max_thrd3,node_profile->wred_max_thrd2,node_profile->wred_max_thrd1,node_profile->wred_max_thrd0,
                    node_profile->factor0,node_profile->factor1,node_profile->factor2,node_profile->factor3,((ctc_spool_node_t*)node)->ref_cnt);
    }
    else if(*mode == SYS_QUEUE_GROUP_PROFILE)
    {
        sys_group_shp_profile_t *node_profile = (sys_group_shp_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-15u%-15u%-7u\n",*cnt,node_profile->bucket_thrd,node_profile->bucket_rate,((ctc_spool_node_t*)node)->ref_cnt);
    }
    else if(*mode == SYS_QUEUE_GROUP_METER_PROFILE)
    {
        sys_group_meter_profile_t *node_profile = (sys_group_meter_profile_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file,"%-5s:%u  refcnt:%u\n","Node",*cnt,((ctc_spool_node_t*)node)->ref_cnt);
        for(i = 0; i < SYS_GRP_SHP_CBUCKET_NUM;i++)
        {
            SYS_DUMP_DB_LOG(p_file,"  rate %u:%u\n",i,node_profile->c_bucket[i].rate);
            SYS_DUMP_DB_LOG(p_file,"  thrd %u:%u\n",i,node_profile->c_bucket[i].thrd);
            SYS_DUMP_DB_LOG(p_file,"  type %u:%u\n",i,node_profile->c_bucket[i].cir_type);
        }
        SYS_DUMP_DB_LOG(p_file, "%s\n", "----------------------------------------------------");
    }
    else if(*mode == SYS_QUEUE_FC_PROFILE || *mode == SYS_QUEUE_PFC_PROFILE || *mode == SYS_QUEUE_FC_DROP_PROFILE|| *mode == SYS_QUEUE_PFC_DROP_PROFILE)
    {
        sys_qos_fc_profile_t *node_profile = (sys_qos_fc_profile_t *)(((ctc_spool_node_t*)node)->data);
        if(((ctc_spool_node_t*)node)->ref_cnt == 0xFFFFFFFF)
        {
            SYS_DUMP_DB_LOG(p_file, "%-7u%-7u%-7u%-7ustatic\n",*cnt,node_profile->xon,node_profile->xoff,node_profile->thrd);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_file, "%-7u%-7u%-7u%-7u%-7u\n",*cnt,node_profile->xon,node_profile->xoff,node_profile->thrd,((ctc_spool_node_t*)node)->ref_cnt);
        }
    }
    (*cnt)++;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_queue_node_wb_sync_func(void* array_data, uint32 index, void* user_data)
{
    uint16 max_entry_cnt = 0;
    uint8 lchip = 0;
    sys_queue_node_t *p_queue_node = (sys_queue_node_t *)array_data;
    sys_wb_qos_queue_queue_node_t *p_wb_queue_node;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    lchip = traversal_data->value1;

    if(traversal_data->value2 == 3 && p_queue_node->queue_id >= MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_NUM))
    {
        return CTC_E_NONE;
    }
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_queue_node = (sys_wb_qos_queue_queue_node_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_queue_node, 0, sizeof(sys_wb_qos_queue_queue_node_t));

    p_wb_queue_node->queue_id = p_queue_node->queue_id;

    if (NULL != p_queue_node->p_shp_profile)
    {
        p_wb_queue_node->is_shp_profile_valid = 1;
        p_wb_queue_node->shp_profile_id = p_queue_node->p_shp_profile->profile_id;
    }
    else
    {
        p_wb_queue_node->is_shp_profile_valid = 0;
        p_wb_queue_node->shp_profile_id = 0;
        p_wb_queue_node->shp_profile_queue_type = 0;
    }

    if (NULL != p_queue_node->p_meter_profile)
    {
        p_wb_queue_node->is_meter_profile_valid = 1;
        p_wb_queue_node->meter_profile_id = p_queue_node->p_meter_profile->profile_id;
    }
    else
    {
        p_wb_queue_node->is_meter_profile_valid = 0;
        p_wb_queue_node->meter_profile_id = 0;
    }

    if (NULL != p_queue_node->drop_profile.p_drop_wtd_profile)
    {
        p_wb_queue_node->is_drop_wtd_profile_valid = 1;
        p_wb_queue_node->drop_wtd_profile_id = p_queue_node->drop_profile.p_drop_wtd_profile->profile_id;
    }
    else
    {
        p_wb_queue_node->is_drop_wtd_profile_valid = 0;
        p_wb_queue_node->drop_wtd_profile_id = 0;
    }

    if (NULL != p_queue_node->drop_profile.p_drop_wred_profile)
    {
        p_wb_queue_node->is_drop_wred_profile_valid = 1;
        p_wb_queue_node->drop_wred_profile_id = p_queue_node->drop_profile.p_drop_wred_profile->profile_id;
    }
    else
    {
        p_wb_queue_node->is_drop_wred_profile_valid = 0;
        p_wb_queue_node->drop_wred_profile_id = 0;
    }

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_group_node_wb_sync_func(void* array_data, uint32 index, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_queue_group_node_t *p_group_node = (sys_queue_group_node_t *)array_data;
    sys_wb_qos_queue_group_node_t *p_wb_group_node;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_group_node = (sys_wb_qos_queue_group_node_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_group_node, 0, sizeof(sys_wb_qos_queue_group_node_t));

    p_wb_group_node->group_id = p_group_node->group_id;
    p_wb_group_node->chan_id = p_group_node->chan_id;
    p_wb_group_node->shp_bitmap = p_group_node->shp_bitmap;

    if (NULL != p_group_node->p_shp_profile)
    {
        p_wb_group_node->is_shp_profile_valid = 1;
        p_wb_group_node->shp_profile_id = p_group_node->p_shp_profile->profile_id;
    }
    else
    {
        p_wb_group_node->is_shp_profile_valid = 0;
        p_wb_group_node->shp_profile_id = 0;
    }

    if (NULL != p_group_node->p_meter_profile)
    {
        p_wb_group_node->is_meter_profile_valid = 1;
        p_wb_group_node->meter_profile_id = p_group_node->p_meter_profile->profile_id;
    }
    else
    {
        p_wb_group_node->is_meter_profile_valid = 0;
        p_wb_group_node->meter_profile_id = 0;
    }

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_port_extender_wb_sync_func(void* bucket_data, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_extend_que_node_t *p_port_extender = (sys_extend_que_node_t *)bucket_data;
    sys_wb_qos_queue_port_extender_t *p_wb_port_extender;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    if(p_port_extender->type == SYS_EXTEND_QUE_TYPE_MCAST)
    {
        return CTC_E_NONE;
    }
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_port_extender = (sys_wb_qos_queue_port_extender_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_port_extender, 0, sizeof(sys_wb_qos_queue_port_extender_t));
    p_wb_port_extender->lchip = p_port_extender->lchip;
    p_wb_port_extender->lport = p_port_extender->lport;
    p_wb_port_extender->channel = p_port_extender->channel;
    p_wb_port_extender->type = p_port_extender->type;
    p_wb_port_extender->logic_src_port = p_port_extender->logic_src_port;

    switch (p_port_extender->type)
    {
        case SYS_EXTEND_QUE_TYPE_SERVICE:
        {
            p_wb_port_extender->type = SYS_WB_QOS_EXTEND_QUE_TYPE_SERVICE;
            break;
        }
        case SYS_EXTEND_QUE_TYPE_BPE:
        {
            p_wb_port_extender->type = SYS_WB_QOS_EXTEND_QUE_TYPE_BPE;
            break;
        }
        case SYS_EXTEND_QUE_TYPE_C2C:
        {
            p_wb_port_extender->type = SYS_WB_QOS_EXTEND_QUE_TYPE_C2C;
            break;
        }
        case SYS_EXTEND_QUE_TYPE_LOGIC_PORT:
        {
            p_wb_port_extender->type = SYS_WB_QOS_EXTEND_QUE_TYPE_LOGIC_PORT;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync queue port extender type failed, type = %d.\n", p_port_extender->type);
            return CTC_E_NONE;
        }
    }

    p_wb_port_extender->service_id = p_port_extender->service_id;
    p_wb_port_extender->group_id = p_port_extender->group_id;
    p_wb_port_extender->key_index = p_port_extender->key_index;
    p_wb_port_extender->logic_dst_port = p_port_extender->logic_dst_port;

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_logic_src_port_wb_sync_func(void* bucket_data, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_service_node_t *p_service = (sys_service_node_t *)bucket_data;
    sys_wb_qos_logicsrc_port_t *p_wb_logicsrc_port = NULL;
    sys_qos_logicport_t *p_logic_port_node = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_logicsrc_port = (sys_wb_qos_logicsrc_port_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_logicsrc_port, 0, sizeof(sys_wb_qos_logicsrc_port_t));
    CTC_SLIST_LOOP(p_service->p_logic_port_list, ctc_slistnode)
    {
        p_wb_logicsrc_port = (sys_wb_qos_logicsrc_port_t *)wb_data->buffer + wb_data->valid_cnt;
        p_logic_port_node = _ctc_container_of(ctc_slistnode, sys_qos_logicport_t, head);
        p_wb_logicsrc_port->logic_src_port = p_logic_port_node->logic_port;
        p_wb_logicsrc_port->service_id = p_service->service_id;

        if (++wb_data->valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
            wb_data->valid_cnt = 0;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_dest_port_wb_sync_func(void* bucket_data, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_service_node_t *p_service = (sys_service_node_t *)bucket_data;
    sys_wb_qos_destport_t *p_wb_destport = NULL;
    sys_qos_destport_t *p_destport_node = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_destport = (sys_wb_qos_destport_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_destport, 0, sizeof(sys_wb_qos_destport_t));

    CTC_SLIST_LOOP(p_service->p_dest_port_list, ctc_slistnode)
    {
        p_wb_destport = (sys_wb_qos_destport_t *)wb_data->buffer + wb_data->valid_cnt;
        p_destport_node = _ctc_container_of(ctc_slistnode, sys_qos_destport_t, head);
        p_wb_destport->lchip = p_destport_node->lchip;
        p_wb_destport->lport = p_destport_node->lport;
        p_wb_destport->service_id = p_service->service_id;

        if (++wb_data->valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
            wb_data->valid_cnt = 0;
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_wb_sync(uint8 lchip, ctc_wb_data_t *p_wb_data)
{
    int32 ret = CTC_E_NONE;
    sys_traverse_t user_data;
    uint16 max_entry_cnt = 0;
    sys_wb_qos_queue_master_t *p_wb_qos_queue_master;
    sys_wb_qos_queue_fc_profile_t *p_wb_queue_fc_profile;
    sys_wb_qos_queue_pfc_profile_t *p_wb_queue_pfc_profile;
    sys_wb_qos_queue_fc_profile_t *p_wb_queue_fc_dropthrd_profile;
    sys_wb_qos_queue_pfc_profile_t *p_wb_queue_pfc_dropthrd_profile;
    sys_wb_qos_cpu_reason_t *p_wb_cpu_reason;
    int32 i;
    int32 j;
    uint8 is_unknown = 0;
    uint16 max_num = 0;
    uint8 work_status = 0;

    user_data.data = p_wb_data;
    user_data.value1 = lchip;
    CTC_ERROR_GOTO(sys_usw_ftm_get_working_status(lchip, &work_status), ret, done);
    user_data.value2 = work_status;

    /* queue matser */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_master_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_MASTER);

    p_wb_qos_queue_master = (sys_wb_qos_queue_master_t *)p_wb_data->buffer;
    sal_memset(p_wb_qos_queue_master, 0, sizeof(sys_wb_qos_queue_master_t));
    p_wb_qos_queue_master->lchip = lchip;
    p_wb_qos_queue_master->store_chan_shp_en = p_usw_queue_master[lchip]->store_chan_shp_en;
    p_wb_qos_queue_master->store_que_shp_en = p_usw_queue_master[lchip]->store_que_shp_en;
    p_wb_qos_queue_master->monitor_drop_en = p_usw_queue_master[lchip]->monitor_drop_en;
    p_wb_qos_queue_master->shp_pps_en = p_usw_queue_master[lchip]->shp_pps_en;
    p_wb_qos_queue_master->queue_thrd_factor = p_usw_queue_master[lchip]->queue_thrd_factor;

    p_wb_data->valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);

    /* queue queue node */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_queue_node_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_QUEUE_NODE);

    CTC_ERROR_GOTO(ctc_vector_traverse2(p_usw_queue_master[lchip]->queue_vec, 0, _sys_usw_queue_queue_node_wb_sync_func, (void *)&user_data), ret, done);
    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }
    if(work_status != 3)
    {
        /* queue group node */
        CTC_WB_INIT_DATA_T(p_wb_data,sys_wb_qos_queue_group_node_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_GROUP_NODE);

        CTC_ERROR_GOTO(ctc_vector_traverse2(p_usw_queue_master[lchip]->group_vec, 0, _sys_usw_queue_group_node_wb_sync_func, (void *)&user_data), ret, done);
        if (p_wb_data->valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
    }
    /* queue fc profile */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_fc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_FC_PROFILE);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (p_wb_data->key_len + p_wb_data->data_len);

    for (i = 0; i < MCHIP_CAP(SYS_CAP_CHANID_MAX); i++)
    {
        if (NULL == p_usw_queue_master[lchip]->p_fc_profile[i])
        {
            continue;
        }

        if (ctc_spool_is_static(p_usw_queue_master[lchip]->p_fc_profile_pool, p_usw_queue_master[lchip]->p_fc_profile[i]))
        {
            continue;
        }

        p_wb_queue_fc_profile = (sys_wb_qos_queue_fc_profile_t *)p_wb_data->buffer + p_wb_data->valid_cnt;

        sal_memset(p_wb_queue_fc_profile, 0, sizeof(sys_wb_qos_queue_fc_profile_t));

        p_wb_queue_fc_profile->chan_id = i;
        p_wb_queue_fc_profile->fc_profile_id = p_usw_queue_master[lchip]->p_fc_profile[i]->profile_id;

        p_wb_data->valid_cnt++;
        if (p_wb_data->valid_cnt >= max_entry_cnt)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
    }

    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }

    /* queue pfc profile */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_pfc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_PFC_PROFILE);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (p_wb_data->key_len + p_wb_data->data_len);

    for (i = 0; i < MCHIP_CAP(SYS_CAP_CHANID_MAX); i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (NULL == p_usw_queue_master[lchip]->p_pfc_profile[i][j])
            {
                continue;
            }

            if (ctc_spool_is_static(p_usw_queue_master[lchip]->p_pfc_profile_pool, p_usw_queue_master[lchip]->p_pfc_profile[i][j]))
            {
                continue;
            }

            p_wb_queue_pfc_profile = (sys_wb_qos_queue_pfc_profile_t *)p_wb_data->buffer + p_wb_data->valid_cnt;

            sal_memset(p_wb_queue_pfc_profile, 0, sizeof(sys_wb_qos_queue_pfc_profile_t));

            p_wb_queue_pfc_profile->chan_id = i;
            p_wb_queue_pfc_profile->priority_class = j;
            p_wb_queue_pfc_profile->pfc_profile_id = p_usw_queue_master[lchip]->p_pfc_profile[i][j]->profile_id;

            p_wb_data->valid_cnt++;
            if (p_wb_data->valid_cnt >= max_entry_cnt)
            {
                CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
                p_wb_data->valid_cnt = 0;
            }
        }
    }

    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }

    /* queue dropth fc profile */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_fc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_FC_DROPTH_PROFILE);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (p_wb_data->key_len + p_wb_data->data_len);

    for (i = 0; i < MCHIP_CAP(SYS_CAP_CHANID_MAX); i++)
    {
        if (NULL == p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i])
        {
            continue;
        }

        p_wb_queue_fc_dropthrd_profile = (sys_wb_qos_queue_fc_profile_t *)p_wb_data->buffer + p_wb_data->valid_cnt;

        sal_memset(p_wb_queue_fc_dropthrd_profile, 0, sizeof(sys_wb_qos_queue_fc_profile_t));

        p_wb_queue_fc_dropthrd_profile->chan_id = i;
        p_wb_queue_fc_dropthrd_profile->fc_profile_id = p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i]->profile_id;

        p_wb_data->valid_cnt++;
        if (p_wb_data->valid_cnt >= max_entry_cnt)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
    }

    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }

    /* queue droth pfc profile */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_pfc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_PFC_DROPTH_PROFILE);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (p_wb_data->key_len + p_wb_data->data_len);

    for (i = 0; i < MCHIP_CAP(SYS_CAP_CHANID_MAX); i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (NULL == p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j])
            {
                continue;
            }

            p_wb_queue_pfc_dropthrd_profile = (sys_wb_qos_queue_pfc_profile_t *)p_wb_data->buffer + p_wb_data->valid_cnt;

            sal_memset(p_wb_queue_pfc_dropthrd_profile, 0, sizeof(sys_wb_qos_queue_pfc_profile_t));

            p_wb_queue_pfc_dropthrd_profile->chan_id = i;
            p_wb_queue_pfc_dropthrd_profile->priority_class = j;
            p_wb_queue_pfc_dropthrd_profile->pfc_profile_id = p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j]->profile_id;

            p_wb_data->valid_cnt++;
            if (p_wb_data->valid_cnt >= max_entry_cnt)
            {
                CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
                p_wb_data->valid_cnt = 0;
            }
        }
    }

    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }
    if(work_status != 3)
    {
        /* queue port extender */
        CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_queue_port_extender_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_PORT_EXTENDER);

        CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_queue_master[lchip]->p_port_extender_hash,
                                                        _sys_usw_queue_port_extender_wb_sync_func, (void *)&user_data), ret, done);
        if (p_wb_data->valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
        /* service logic src port */
        CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_logicsrc_port_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_LOGIC_SRC_PORT);

        CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_queue_master[lchip]->p_service_hash,
                                                        _sys_usw_queue_logic_src_port_wb_sync_func, (void *)&user_data), ret, done);
        if (p_wb_data->valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
        /* service dst port */
        CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_destport_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_DESTPORT);

        CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_queue_master[lchip]->p_service_hash,
                                                        _sys_usw_queue_dest_port_wb_sync_func, (void *)&user_data), ret, done);
        if (p_wb_data->valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
    }
    /* cpu reason */
    CTC_WB_INIT_DATA_T(p_wb_data, sys_wb_qos_cpu_reason_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_CPU_REASON);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (p_wb_data->key_len + p_wb_data->data_len);
    max_num = (work_status == CTC_FTM_MEM_CHANGE_RECOVER) ? CTC_PKT_CPU_REASON_CUSTOM_BASE : CTC_PKT_CPU_REASON_MAX_COUNT;
    for (i = 0; i < max_num; i++)
    {
        p_wb_cpu_reason = (sys_wb_qos_cpu_reason_t *)p_wb_data->buffer + p_wb_data->valid_cnt;

        sal_memset(p_wb_cpu_reason, 0, sizeof(sys_wb_qos_cpu_reason_t));

        p_wb_cpu_reason->reason_id = i;

        switch (p_usw_queue_master[lchip]->cpu_reason[i].dest_type)
        {
            case CTC_PKT_CPU_REASON_TO_LOCAL_CPU:
            {
                p_wb_cpu_reason->dest_type = SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_CPU;
                break;
            }
            case CTC_PKT_CPU_REASON_TO_LOCAL_PORT:
            {
                p_wb_cpu_reason->dest_type = SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_PORT;
                break;
            }
            case CTC_PKT_CPU_REASON_TO_REMOTE_CPU:
            {
                p_wb_cpu_reason->dest_type = SYS_WB_QOS_PKT_CPU_REASON_TO_REMOTE_CPU;
                break;
            }
            case CTC_PKT_CPU_REASON_TO_DROP:
            {
                p_wb_cpu_reason->dest_type = SYS_WB_QOS_PKT_CPU_REASON_TO_DROP;
                break;
            }
            case CTC_PKT_CPU_REASON_TO_NHID:
            {
                p_wb_cpu_reason->dest_type = SYS_WB_QOS_PKT_CPU_REASON_TO_NHID;
                break;
            }
            case CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH:
            {
                p_wb_cpu_reason->dest_type = SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_CPU_ETH;
                break;
            }
            default:
            {
                CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sync cpu reason dest_type failed, dest_type = %d.\n", p_usw_queue_master[lchip]->cpu_reason[i].dest_type);
                is_unknown = 1;
                break;
            }
        }

        if (0 != is_unknown)
        {
            is_unknown = 0;
            continue;
        }

        p_wb_cpu_reason->user_define_mode = p_usw_queue_master[lchip]->cpu_reason[i].user_define_mode;
        p_wb_cpu_reason->excp_id = p_usw_queue_master[lchip]->cpu_reason[i].excp_id;
        p_wb_cpu_reason->dsfwd_offset = p_usw_queue_master[lchip]->cpu_reason[i].dsfwd_offset;
        p_wb_cpu_reason->sub_queue_id = p_usw_queue_master[lchip]->cpu_reason[i].sub_queue_id;
        p_wb_cpu_reason->ref_cnt = p_usw_queue_master[lchip]->cpu_reason[i].ref_cnt;
        p_wb_cpu_reason->dest_port = p_usw_queue_master[lchip]->cpu_reason[i].dest_port;
        p_wb_cpu_reason->dest_map = p_usw_queue_master[lchip]->cpu_reason[i].dest_map;
        p_wb_cpu_reason->dir = p_usw_queue_master[lchip]->cpu_reason[i].dir;

        p_wb_data->valid_cnt++;
        if (p_wb_data->valid_cnt >= max_entry_cnt)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
            p_wb_data->valid_cnt = 0;
        }
    }

    if (p_wb_data->valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(p_wb_data), ret, done);
        p_wb_data->valid_cnt = 0;
    }

done:
    return ret;
}

int32
sys_usw_queue_wb_restore(uint8 lchip, ctc_wb_query_t *p_wb_query)
{
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    sys_usw_opf_t opf;
    sys_wb_qos_queue_master_t *p_wb_qos_queue_master;
    sys_wb_qos_queue_master_t wb_qos_queue_master;
    sys_wb_qos_queue_queue_node_t *p_wb_qos_queue_queue_node;
    sys_wb_qos_queue_queue_node_t wb_qos_queue_queue_node;
    sys_queue_node_t *p_queue_node;
    sys_queue_shp_profile_t queue_shp_profile;
    sys_queue_shp_profile_t *p_queue_shp_profile_get;
    sys_queue_meter_profile_t queue_meter_profile;
    sys_queue_meter_profile_t *p_queue_meter_profile_get;
    sys_queue_drop_profile_t queue_drop_profile;
    sys_queue_drop_wtd_profile_t queue_drop_wtd_profile;
    sys_queue_drop_wtd_profile_t *p_queue_drop_wtd_profile_get;
    sys_queue_drop_wred_profile_t queue_drop_wred_profile;
    sys_queue_drop_wred_profile_t *p_queue_drop_wred_profile_get;
    sys_wb_qos_queue_group_node_t *p_wb_qos_queue_group_node;
    sys_wb_qos_queue_group_node_t wb_qos_queue_group_node;
    sys_queue_group_node_t *p_queue_group_node;
    sys_group_shp_profile_t group_shp_profile;
    sys_group_shp_profile_t *p_group_shp_profile_get;
    sys_group_meter_profile_t group_meter_profile;
    sys_group_meter_profile_t *p_group_meter_profile_get;
    sys_wb_qos_queue_fc_profile_t *p_wb_queue_fc_profile;
    sys_wb_qos_queue_fc_profile_t wb_queue_fc_profile;
    sys_qos_fc_profile_t queue_fc_profile;
    sys_qos_fc_profile_t *p_queue_fc_profile_get;
    sys_wb_qos_queue_pfc_profile_t *p_wb_queue_pfc_profile;
    sys_wb_qos_queue_pfc_profile_t wb_queue_pfc_profile;
    sys_qos_fc_profile_t queue_pfc_profile;
    sys_qos_fc_profile_t *p_queue_pfc_profile_get;
    sys_wb_qos_queue_port_extender_t *p_wb_port_extender;
    sys_wb_qos_queue_port_extender_t wb_port_extender;
    sys_wb_qos_logicsrc_port_t *p_wb_logicsrc_port;
    sys_wb_qos_logicsrc_port_t wb_logicsrc_port;
    sys_wb_qos_destport_t *p_wb_dest_port;
    sys_wb_qos_destport_t wb_dest_port;
    sys_qos_logicport_t *p_logicsrc_port = NULL;
    sys_qos_destport_t *p_dest_port = NULL;
    sys_extend_que_node_t *p_port_extender;
    sys_qos_logicport_service_t *p_logicport_service;
    sys_service_node_t *p_service;
    sys_service_node_t service;
    sys_wb_qos_cpu_reason_t *p_wb_cpu_reason;
    sys_wb_qos_cpu_reason_t wb_cpu_reason;
    sys_wb_qos_queue_fc_profile_t *p_wb_queue_fc_dropthrd_profile;
    sys_wb_qos_queue_fc_profile_t wb_queue_fc_dropthrd_profile;
    sys_qos_fc_profile_t queue_fc_dropthrd_profile;
    sys_qos_fc_profile_t *p_queue_fc_dropthrd_profile_get;
    sys_wb_qos_queue_pfc_profile_t *p_wb_queue_pfc_dropthrd_profile;
    sys_wb_qos_queue_pfc_profile_t wb_queue_pfc_dropthrd_profile;
    sys_qos_fc_profile_t queue_pfc_dropthrd_profile;
    sys_qos_fc_profile_t *p_queue_pfc_dropthrd_profile_get;
    uint8 opf_num = 0;
    uint8 is_unknown = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 flag = 3;

    CTC_ERROR_GOTO(sys_usw_ftm_get_working_status(lchip, &flag), ret, done);
    /* queue master */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_master_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_MASTER);

    sal_memset(&wb_qos_queue_master, 0, sizeof(sys_wb_qos_queue_master_t));
    p_wb_qos_queue_master = &wb_qos_queue_master;

    CTC_ERROR_GOTO(ctc_wb_query_entry(p_wb_query), ret, done);

    if (p_wb_query->valid_cnt != 1 || p_wb_query->is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query qos queue master error! valid_cnt: %d, is_end: %d.\n", p_wb_query->valid_cnt, p_wb_query->is_end);
        goto done;
    }

    sal_memcpy((uint8 *)&wb_qos_queue_master, (uint8 *)p_wb_query->buffer, (p_wb_query->key_len + p_wb_query->data_len));

    p_usw_queue_master[lchip]->store_chan_shp_en = p_wb_qos_queue_master->store_chan_shp_en;
    p_usw_queue_master[lchip]->store_que_shp_en = p_wb_qos_queue_master->store_que_shp_en;
    p_usw_queue_master[lchip]->monitor_drop_en = p_wb_qos_queue_master->monitor_drop_en;
    p_usw_queue_master[lchip]->shp_pps_en = p_wb_qos_queue_master->shp_pps_en;
    p_usw_queue_master[lchip]->queue_thrd_factor = p_wb_qos_queue_master->queue_thrd_factor;

    /* queue queue node */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_queue_node_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_QUEUE_NODE);

    entry_cnt = 0;

    sal_memset(&wb_qos_queue_queue_node, 0, sizeof(sys_wb_qos_queue_queue_node_t));
    p_wb_qos_queue_queue_node = &wb_qos_queue_queue_node;

    p_usw_queue_master[lchip]->p_queue_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_queue_shp_restore_profileId;
    p_usw_queue_master[lchip]->p_queue_meter_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_queue_meter_restore_profileId;
    p_usw_queue_master[lchip]->p_drop_wred_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_queue_drop_wred_restore_profileId;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_qos_queue_queue_node, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));

    entry_cnt++;
    p_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, p_wb_qos_queue_queue_node->queue_id);
    if (NULL == p_queue_node)
    {
        p_queue_node = mem_malloc(MEM_QUEUE_MODULE,  sizeof(sys_queue_node_t));
        if (NULL == p_queue_node)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }

        sal_memset(p_queue_node, 0, sizeof(sys_queue_node_t));

        p_queue_node->queue_id = p_wb_qos_queue_queue_node->queue_id;
        if (p_wb_qos_queue_queue_node->queue_id < MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_NUM))
        {
            p_queue_node->offset = p_wb_qos_queue_queue_node->queue_id % 8;
        }
        else
        {
            p_queue_node->offset = p_wb_qos_queue_queue_node->queue_id % MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        }
        CTC_ERROR_GOTO(ctc_vector_add(p_usw_queue_master[lchip]->queue_vec,
                                                            p_queue_node->queue_id, p_queue_node), ret, done);
    }

    if (0 != p_wb_qos_queue_queue_node->is_shp_profile_valid)
    {
        sal_memset(&queue_shp_profile, 0, sizeof(sys_queue_shp_profile_t));
        queue_shp_profile.profile_id = p_wb_qos_queue_queue_node->shp_profile_id;

        CTC_ERROR_GOTO(sys_usw_queue_get_queue_type_by_queue_id(lchip, p_wb_qos_queue_queue_node->queue_id,
                                                                    &(queue_shp_profile.queue_type)), ret, done);

        CTC_ERROR_GOTO(_sys_usw_queue_shp_get_queue_shape_profile_from_asic(lchip, &queue_shp_profile), ret, done);

        CTC_ERROR_GOTO(ctc_spool_add(p_usw_queue_master[lchip]->p_queue_profile_pool,
                                                               &queue_shp_profile,
                                                               NULL,
                                                               &p_queue_shp_profile_get), ret, done);

        p_queue_node->p_shp_profile = p_queue_shp_profile_get;

        p_queue_node->shp_en = 1;
        p_queue_node->type = queue_shp_profile.queue_type;
    }
    else
    {
        p_queue_node->shp_en = 0;
    }

    if (0 != p_wb_qos_queue_queue_node->is_meter_profile_valid)
    {
        sal_memset(&queue_meter_profile, 0, sizeof(sys_queue_meter_profile_t));
        queue_meter_profile.profile_id = p_wb_qos_queue_queue_node->meter_profile_id;

        CTC_ERROR_GOTO(_sys_usw_queue_shp_get_queue_meter_profile_from_asic(lchip, &queue_meter_profile), ret, done);

        CTC_ERROR_GOTO(ctc_spool_add(p_usw_queue_master[lchip]->p_queue_meter_profile_pool,
                                                               &queue_meter_profile,
                                                               NULL,
                                                               &p_queue_meter_profile_get), ret, done);

        p_queue_node->p_meter_profile = p_queue_meter_profile_get;
    }

    if (0 != p_wb_qos_queue_queue_node->is_drop_wtd_profile_valid)
    {
        sal_memset(&queue_drop_wtd_profile, 0, sizeof(sys_queue_drop_wtd_profile_t));
        queue_drop_wtd_profile.profile_id = p_wb_qos_queue_queue_node->drop_wtd_profile_id;

        sal_memset(&queue_drop_profile, 0 , sizeof(sys_queue_drop_profile_t));
        queue_drop_profile.p_drop_wtd_profile = &queue_drop_wtd_profile;

        CTC_ERROR_GOTO(_sys_usw_queue_drop_read_profile_from_asic(lchip, 0, p_queue_node, &queue_drop_profile), ret, done);

        CTC_ERROR_GOTO(_sys_usw_queue_drop_wtd_profile_add_spool(lchip, p_queue_node->drop_profile.p_drop_wtd_profile,
                                                                                                            &queue_drop_wtd_profile, &p_queue_drop_wtd_profile_get), ret, done);

        p_queue_node->drop_profile.p_drop_wtd_profile = p_queue_drop_wtd_profile_get;
    }

    if (0 != p_wb_qos_queue_queue_node->is_drop_wred_profile_valid)
    {
        sal_memset(&queue_drop_wred_profile, 0, sizeof(sys_queue_drop_wred_profile_t));
        queue_drop_wred_profile.profile_id = p_wb_qos_queue_queue_node->drop_wred_profile_id;

        sal_memset(&queue_drop_profile, 0 , sizeof(sys_queue_drop_profile_t));
        queue_drop_profile.p_drop_wred_profile = &queue_drop_wred_profile;

        CTC_ERROR_GOTO(_sys_usw_queue_drop_read_profile_from_asic(lchip, 1, p_queue_node, &queue_drop_profile), ret, done);

        CTC_ERROR_GOTO(_sys_usw_queue_drop_wred_profile_add_spool(lchip, p_queue_node->drop_profile.p_drop_wred_profile,
                                                                                                            &queue_drop_wred_profile, &p_queue_drop_wred_profile_get), ret, done);

        p_queue_node->drop_profile.p_drop_wred_profile = p_queue_drop_wred_profile_get;
    }

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    p_usw_queue_master[lchip]->p_queue_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_queue_shp_alloc_profileId;
    p_usw_queue_master[lchip]->p_queue_meter_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_queue_meter_alloc_profileId;
    p_usw_queue_master[lchip]->p_drop_wred_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_queue_drop_wred_alloc_profileId;

    /* queue group node */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_group_node_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_GROUP_NODE);

    entry_cnt = 0;

    sal_memset(&wb_qos_queue_group_node, 0, sizeof(sys_wb_qos_queue_group_node_t));
    p_wb_qos_queue_group_node = &wb_qos_queue_group_node;

    p_usw_queue_master[lchip]->p_group_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_group_shp_restore_profileId;
    p_usw_queue_master[lchip]->p_group_profile_meter_pool->spool_alloc = (spool_alloc_fn)_sys_usw_group_meter_restore_profileId;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_qos_queue_group_node, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    p_queue_group_node = ctc_vector_get(p_usw_queue_master[lchip]->group_vec, p_wb_qos_queue_group_node->group_id);
    if (NULL == p_queue_group_node)
    {
        p_queue_group_node = mem_malloc(MEM_QUEUE_MODULE,  sizeof(sys_queue_group_node_t));
        if (NULL == p_queue_group_node)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }

        sal_memset(p_queue_group_node, 0, sizeof(sys_queue_group_node_t));
        p_queue_group_node->group_id = p_wb_qos_queue_group_node->group_id;
        p_queue_group_node->chan_id = p_wb_qos_queue_group_node->chan_id;
        p_queue_group_node->shp_bitmap = p_wb_qos_queue_group_node->shp_bitmap;

        CTC_ERROR_GOTO(ctc_vector_add(p_usw_queue_master[lchip]->group_vec,
                                                            p_queue_group_node->group_id, p_queue_group_node), ret, done);
    }
    else
    {
        p_queue_group_node->chan_id = p_wb_qos_queue_group_node->chan_id;
        p_queue_group_node->shp_bitmap = p_wb_qos_queue_group_node->shp_bitmap;
    }

    if (0 != p_wb_qos_queue_group_node->is_shp_profile_valid)
    {
        sal_memset(&group_shp_profile, 0, sizeof(sys_group_shp_profile_t));
        group_shp_profile.profile_id = p_wb_qos_queue_group_node->shp_profile_id;

        CTC_ERROR_GOTO(_sys_usw_queue_shp_get_group_shape_profile_from_asic(lchip, &group_shp_profile), ret, done);

        CTC_ERROR_GOTO(ctc_spool_add(p_usw_queue_master[lchip]->p_group_profile_pool,
                                                               &group_shp_profile,
                                                               NULL,
                                                               &p_group_shp_profile_get), ret, done);

        p_queue_group_node->p_shp_profile = p_group_shp_profile_get;
    }

    if (0 != p_wb_qos_queue_group_node->is_meter_profile_valid)
    {
        sal_memset(&group_meter_profile, 0, sizeof(sys_group_meter_profile_t));
        group_meter_profile.profile_id = p_wb_qos_queue_group_node->meter_profile_id;

        CTC_ERROR_GOTO(_sys_usw_queue_shp_get_group_meter_profile_from_asic(lchip, &group_meter_profile), ret, done);

        CTC_ERROR_GOTO(ctc_spool_add(p_usw_queue_master[lchip]->p_group_profile_meter_pool,
                                                               &group_meter_profile,
                                                               NULL,
                                                               &p_group_meter_profile_get), ret, done);

        p_queue_group_node->p_meter_profile = p_group_meter_profile_get;
    }

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    p_usw_queue_master[lchip]->p_group_profile_pool->spool_alloc = (spool_alloc_fn)_sys_usw_group_shp_alloc_profileId;
    p_usw_queue_master[lchip]->p_group_profile_meter_pool->spool_alloc = (spool_alloc_fn)_sys_usw_group_meter_alloc_profileId;

    /* queue fc profile */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_fc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_FC_PROFILE);

    entry_cnt = 0;

    sal_memset(&wb_queue_fc_profile, 0, sizeof(sys_wb_qos_queue_fc_profile_t));
    p_wb_queue_fc_profile = &wb_queue_fc_profile;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_queue_fc_profile, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    if (p_wb_queue_fc_profile->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAX))
    {
        continue;
    }

    sal_memset(&queue_fc_profile, 0, sizeof(sys_qos_fc_profile_t));
    queue_fc_profile.profile_id = p_wb_queue_fc_profile->fc_profile_id;

    CTC_ERROR_GOTO(sys_usw_qos_fc_get_profile_from_asic(lchip, 0, &queue_fc_profile), ret, done);

    CTC_ERROR_GOTO(_sys_usw_qos_fc_profile_add_spool(lchip, 0, 0,p_usw_queue_master[lchip]->p_fc_profile[p_wb_queue_fc_profile->chan_id],
                                                                                    &queue_fc_profile, &p_queue_fc_profile_get), ret, done);

    p_usw_queue_master[lchip]->p_fc_profile[p_wb_queue_fc_profile->chan_id] = p_queue_fc_profile_get;

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    /* queue pfc profile */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_pfc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_PFC_PROFILE);

    entry_cnt = 0;

    sal_memset(&wb_queue_pfc_profile, 0, sizeof(sys_wb_qos_queue_pfc_profile_t));
    p_wb_queue_pfc_profile = &wb_queue_pfc_profile;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_queue_pfc_profile, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    if ((p_wb_queue_pfc_profile->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAX)) || (p_wb_queue_pfc_profile->priority_class >= 8))
    {
        continue;
    }

    sal_memset(&queue_pfc_profile, 0, sizeof(sys_qos_fc_profile_t));
    queue_pfc_profile.profile_id = p_wb_queue_pfc_profile->pfc_profile_id;

    CTC_ERROR_GOTO(sys_usw_qos_fc_get_profile_from_asic(lchip, 1, &queue_pfc_profile), ret, done);

    CTC_ERROR_GOTO(_sys_usw_qos_fc_profile_add_spool(lchip, 1, 0,p_usw_queue_master[lchip]->p_pfc_profile[p_wb_queue_pfc_profile->chan_id][p_wb_queue_pfc_profile->priority_class],
                                                                                    &queue_pfc_profile, &p_queue_pfc_profile_get), ret, done);

    p_usw_queue_master[lchip]->p_pfc_profile[p_wb_queue_pfc_profile->chan_id][p_wb_queue_pfc_profile->priority_class] = p_queue_pfc_profile_get;

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    /* queue dropth fc profile */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_fc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_FC_DROPTH_PROFILE);

    entry_cnt = 0;

    sal_memset(&wb_queue_fc_dropthrd_profile, 0, sizeof(sys_wb_qos_queue_fc_profile_t));
    p_wb_queue_fc_dropthrd_profile = &wb_queue_fc_dropthrd_profile;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_queue_fc_dropthrd_profile, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    if (p_wb_queue_fc_dropthrd_profile->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAX))
    {
        continue;
    }

    sal_memset(&queue_fc_dropthrd_profile, 0, sizeof(queue_fc_dropthrd_profile));
    queue_fc_dropthrd_profile.profile_id = p_wb_queue_fc_dropthrd_profile->fc_profile_id;

    CTC_ERROR_GOTO(sys_usw_qos_fc_get_dropthrd_profile_from_asic(lchip, 0, &queue_fc_dropthrd_profile), ret, done);

    CTC_ERROR_GOTO(_sys_usw_qos_fc_profile_add_spool(lchip, 0, 1,p_usw_queue_master[lchip]->p_dropthrd_fc_profile[p_wb_queue_fc_dropthrd_profile->chan_id],
                                                                                    &queue_fc_dropthrd_profile, &p_queue_fc_dropthrd_profile_get), ret, done);

    p_usw_queue_master[lchip]->p_dropthrd_fc_profile[p_wb_queue_fc_dropthrd_profile->chan_id] = p_queue_fc_dropthrd_profile_get;

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    /* queue dropth pfc profile */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_pfc_profile_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_PFC_DROPTH_PROFILE);

    entry_cnt = 0;

    sal_memset(&wb_queue_pfc_dropthrd_profile, 0, sizeof(sys_wb_qos_queue_pfc_profile_t));
    p_wb_queue_pfc_dropthrd_profile = &wb_queue_pfc_dropthrd_profile;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_queue_pfc_dropthrd_profile, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    if ((p_wb_queue_pfc_dropthrd_profile->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAX)) || (p_wb_queue_pfc_dropthrd_profile->priority_class >= 8))
    {
        continue;
    }

    sal_memset(&queue_pfc_dropthrd_profile, 0, sizeof(sys_qos_fc_profile_t));
    queue_pfc_dropthrd_profile.profile_id = p_wb_queue_pfc_dropthrd_profile->pfc_profile_id;

    CTC_ERROR_GOTO(sys_usw_qos_fc_get_dropthrd_profile_from_asic(lchip, 1, &queue_pfc_dropthrd_profile), ret, done);

    CTC_ERROR_GOTO(_sys_usw_qos_fc_profile_add_spool(lchip, 1, 1,p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[p_wb_queue_pfc_dropthrd_profile->chan_id][p_wb_queue_pfc_dropthrd_profile->priority_class],
                                                                                    &queue_pfc_dropthrd_profile, &p_queue_pfc_dropthrd_profile_get), ret, done);

    p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[p_wb_queue_pfc_dropthrd_profile->chan_id][p_wb_queue_pfc_dropthrd_profile->priority_class] = p_queue_pfc_dropthrd_profile_get;

    CTC_WB_QUERY_ENTRY_END(p_wb_query);
    /* queue port extender */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_queue_port_extender_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_PORT_EXTENDER);

    entry_cnt = 0;

    sal_memset(&wb_port_extender, 0, sizeof(sys_wb_qos_queue_port_extender_t));
    p_wb_port_extender = &wb_port_extender;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_port_extender, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    p_port_extender = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_extend_que_node_t));
    if (NULL == p_port_extender)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }

    sal_memset(p_port_extender, 0, sizeof(sys_extend_que_node_t));
    p_port_extender->lchip = p_wb_port_extender->lchip;
    p_port_extender->lport = p_wb_port_extender->lport;
    p_port_extender->channel = p_wb_port_extender->channel;

    switch (p_wb_port_extender->type)
    {
        case SYS_WB_QOS_EXTEND_QUE_TYPE_SERVICE:
        {
            p_port_extender->type = SYS_EXTEND_QUE_TYPE_SERVICE;
            break;
        }
        case SYS_WB_QOS_EXTEND_QUE_TYPE_BPE:
        {
            p_port_extender->type = SYS_EXTEND_QUE_TYPE_BPE;
            break;
        }
        case SYS_WB_QOS_EXTEND_QUE_TYPE_C2C:
        {
            p_port_extender->type = SYS_EXTEND_QUE_TYPE_C2C;
            break;
        }
        case SYS_WB_QOS_EXTEND_QUE_TYPE_LOGIC_PORT:
        {
            p_port_extender->type = SYS_EXTEND_QUE_TYPE_LOGIC_PORT;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore queue port extender type failed, type = %d.\n", p_wb_port_extender->type);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        mem_free(p_port_extender);
        is_unknown = 0;
        continue;
    }

    p_port_extender->service_id = p_wb_port_extender->service_id;
    p_port_extender->group_id = p_wb_port_extender->group_id;
    p_port_extender->key_index = p_wb_port_extender->key_index;
    p_port_extender->logic_dst_port = p_wb_port_extender->logic_dst_port;
    p_port_extender->logic_src_port = p_wb_port_extender->logic_src_port;

    ctc_hash_insert(p_usw_queue_master[lchip]->p_port_extender_hash, p_port_extender);
    opf_num = 1;
    if(p_port_extender->type != SYS_EXTEND_QUE_TYPE_LOGIC_PORT && p_port_extender->lport != 0xffff)
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = p_usw_queue_master[lchip]->opf_type_group_id;
        opf.pool_index = 0;
        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, opf_num, p_port_extender->group_id), ret, done);
    }
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = p_usw_queue_master[lchip]->opf_type_tcam_index;
    opf.pool_index = 0;
    if((p_port_extender->type != SYS_EXTEND_QUE_TYPE_SERVICE || p_usw_queue_master[lchip]->service_queue_mode == 1)&& p_port_extender->lport != 0xffff)
    {
        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, opf_num, p_port_extender->key_index), ret, done);
    }

    CTC_WB_QUERY_ENTRY_END(p_wb_query);
    /* service logic src port */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_logicsrc_port_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_LOGIC_SRC_PORT);

    entry_cnt = 0;

    sal_memset(&wb_logicsrc_port, 0, sizeof(sys_wb_qos_logicsrc_port_t));
    p_wb_logicsrc_port = &wb_logicsrc_port;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_logicsrc_port, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    sal_memset(&service, 0, sizeof(service));
    service.service_id = p_wb_logicsrc_port->service_id;
    p_service = sys_usw_service_lookup(lchip, &service);
    if (NULL == p_service)
    {
        p_service = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_service_node_t));
        if (NULL == p_service)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_service, 0, sizeof(sys_service_node_t));
        p_service->service_id = p_wb_logicsrc_port->service_id;
        p_service->p_logic_port_list = ctc_slist_new();
        if (p_service->p_logic_port_list == NULL)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        p_service->p_dest_port_list = ctc_slist_new();
        if (p_service->p_dest_port_list == NULL)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sys_usw_service_add(lchip, p_service);
    }

    p_logicsrc_port = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_logicport_t));
    if (NULL == p_logicsrc_port)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_logicsrc_port, 0, sizeof(sys_qos_logicport_t));
    p_logicsrc_port->logic_port = p_wb_logicsrc_port->logic_src_port;
    ctc_slist_add_tail(p_service->p_logic_port_list, &p_logicsrc_port->head);

    p_logicport_service = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_logicport_service_t));
    if (NULL == p_logicport_service)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_logicport_service, 0, sizeof(sys_qos_logicport_service_t));
    p_logicport_service->logic_src_port = p_wb_logicsrc_port->logic_src_port;
    p_logicport_service->service_id = p_wb_logicsrc_port->service_id;
    ctc_hash_insert(p_usw_queue_master[lchip]->p_logicport_service_hash, p_logicport_service);

    CTC_WB_QUERY_ENTRY_END(p_wb_query);
    /* service dest port */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_destport_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_DESTPORT);

    entry_cnt = 0;

    sal_memset(&wb_dest_port, 0, sizeof(sys_wb_qos_destport_t));
    p_wb_dest_port = &wb_dest_port;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_dest_port, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                        (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;
    sal_memset(&service, 0, sizeof(service));
    service.service_id = p_wb_dest_port->service_id;
    p_service = sys_usw_service_lookup(lchip, &service);
    if (NULL == p_service)
    {
        p_service = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_service_node_t));
        if (NULL == p_service)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_service, 0, sizeof(sys_service_node_t));
        p_service->service_id = p_wb_dest_port->service_id;
        p_service->p_logic_port_list = ctc_slist_new();
        if (p_service->p_logic_port_list == NULL)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        p_service->p_dest_port_list = ctc_slist_new();
        if (p_service->p_dest_port_list == NULL)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sys_usw_service_add(lchip, p_service);
    }

    p_dest_port = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_qos_destport_t));
    if (NULL == p_dest_port)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }

    p_dest_port->lchip = p_wb_dest_port->lchip;
    p_dest_port->lport = p_wb_dest_port->lport;
    ctc_slist_add_tail(p_service->p_dest_port_list, &p_dest_port->head);

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

    /* cpu reason */
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_qos_cpu_reason_t, CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_CPU_REASON);

    entry_cnt = 0;

    sal_memset(&wb_cpu_reason, 0, sizeof(sys_wb_qos_cpu_reason_t));
    p_wb_cpu_reason = &wb_cpu_reason;

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);

    sal_memcpy((uint8 *)&wb_cpu_reason, (uint8 *)p_wb_query->buffer + entry_cnt * (p_wb_query->key_len + p_wb_query->data_len),
                                    (p_wb_query->key_len + p_wb_query->data_len));
    entry_cnt++;

    if (p_wb_cpu_reason->reason_id >= CTC_PKT_CPU_REASON_MAX_COUNT)
    {
        continue;
    }

    switch (p_wb_cpu_reason->dest_type)
    {
        case SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_CPU:
        {
            p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
            break;
        }
        case SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_PORT:
        {
            p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_PORT;
            break;
        }
        case SYS_WB_QOS_PKT_CPU_REASON_TO_REMOTE_CPU:
        {
            p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_type = CTC_PKT_CPU_REASON_TO_REMOTE_CPU;
            break;
        }
        case SYS_WB_QOS_PKT_CPU_REASON_TO_DROP:
        {
            p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_type = CTC_PKT_CPU_REASON_TO_DROP;
            break;
        }
        case SYS_WB_QOS_PKT_CPU_REASON_TO_NHID:
        {
            p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_type = CTC_PKT_CPU_REASON_TO_NHID;
            break;
        }
        case SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_CPU_ETH:
        {
            p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH;
            break;
        }
        default:
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "restore cpu reason dest_type failed, dest_type = %d.\n", p_wb_cpu_reason->dest_type);
            is_unknown = 1;
            break;
        }
    }

    if (0 != is_unknown)
    {
        is_unknown = 0;
        continue;
    }

    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].user_define_mode = p_wb_cpu_reason->user_define_mode;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].excp_id = p_wb_cpu_reason->excp_id;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dsfwd_offset = p_wb_cpu_reason->dsfwd_offset;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].sub_queue_id = p_wb_cpu_reason->sub_queue_id;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].ref_cnt = p_wb_cpu_reason->ref_cnt;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_port = p_wb_cpu_reason->dest_port;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dest_map = p_wb_cpu_reason->dest_map;
    p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dir = p_wb_cpu_reason->dir;
    if(p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dsfwd_offset)
    {
        CTC_ERROR_GOTO(sys_usw_nh_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD, 1, p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].dsfwd_offset), ret, done);
    }

    cmd = DRV_IOR(ErmMiscCtl_t, ErmMiscCtl_resourceCheckMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
     p_usw_queue_master[lchip]->resrc_check_mode = value;

    if ((2 == p_wb_cpu_reason->user_define_mode) && (0 != p_wb_cpu_reason->excp_id))
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = p_usw_queue_master[lchip]->opf_type_excp_index;
        opf.pool_index = p_wb_cpu_reason->dir;
        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_wb_cpu_reason->excp_id), ret, done);
    }
    if(flag == 3 && p_wb_cpu_reason->reason_id < CTC_PKT_CPU_REASON_CUSTOM_BASE && p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].sub_queue_id != CTC_MAX_UINT16_VALUE)
    {
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
        sys_usw_cpu_reason_set_map(lchip,  p_wb_cpu_reason->reason_id, p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].sub_queue_id%SYS_USW_MAX_QNUM_PER_GROUP,
                       p_usw_queue_master[lchip]->cpu_reason[p_wb_cpu_reason->reason_id].sub_queue_id/SYS_USW_MAX_QNUM_PER_GROUP);
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

done:
    return ret;
}

int32
sys_usw_qos_queue_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    sys_dump_db_traverse_param_t    cb_data;
    uint32 num_cnt = 1;
    uint32 mode = 0;
    uint8 i = 0;
    uint8 j = 0;
    uint8 count = 0;
    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "##Queue");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:","p_fc_profile");
    for(i = 0; i < SYS_MAX_CHANNEL_NUM; i ++)
    {
        if(p_usw_queue_master[lchip]->p_fc_profile[i]&&p_usw_queue_master[lchip]->p_fc_profile[i]->xon == 256&&p_usw_queue_master[lchip]->p_fc_profile[i]->xoff == 224&&p_usw_queue_master[lchip]->p_fc_profile[i]->thrd == 10576 &&
            p_usw_queue_master[lchip]->p_fc_profile[i]->profile_id == 0&&p_usw_queue_master[lchip]->p_fc_profile[i]->type == 0)
        {
            continue;
        }
        if(p_usw_queue_master[lchip]->p_fc_profile[i])
        {
            count ++;
            SYS_DUMP_DB_LOG(dump_db_fp,"[%u:%u,%u,%u,%u,%u]",i,p_usw_queue_master[lchip]->p_fc_profile[i]->xon,p_usw_queue_master[lchip]->p_fc_profile[i]->xoff,p_usw_queue_master[lchip]->p_fc_profile[i]->thrd,
            p_usw_queue_master[lchip]->p_fc_profile[i]->profile_id,p_usw_queue_master[lchip]->p_fc_profile[i]->type);
        }
        if(count%4 == 3 && p_usw_queue_master[lchip]->p_fc_profile[i])
        {
            SYS_DUMP_DB_LOG(dump_db_fp, "\n%31s"," ");
        }
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:","p_pfc_profile");
    count = 0;
    for(i = 0; i < SYS_MAX_CHANNEL_NUM; i ++)
    {
        for(j = 0; j < 8; j ++)
        {
            if(p_usw_queue_master[lchip]->p_pfc_profile[i][j]&&p_usw_queue_master[lchip]->p_pfc_profile[i][j]->xon == 256&&p_usw_queue_master[lchip]->p_pfc_profile[i][j]->xoff == 224&&p_usw_queue_master[lchip]->p_pfc_profile[i][j]->thrd == 10576 &&
                p_usw_queue_master[lchip]->p_pfc_profile[i][j]->profile_id == 0&&p_usw_queue_master[lchip]->p_pfc_profile[i][j]->type == 0)
            {
                continue;
            }
            if(p_usw_queue_master[lchip]->p_pfc_profile[i][j])
            {
                count ++;
                SYS_DUMP_DB_LOG(dump_db_fp,"[%u-%u:%u,%u,%u,%u,%u]",i,j,p_usw_queue_master[lchip]->p_pfc_profile[i][j]->xon,p_usw_queue_master[lchip]->p_pfc_profile[i][j]->xoff,p_usw_queue_master[lchip]->p_pfc_profile[i][j]->thrd,
                p_usw_queue_master[lchip]->p_pfc_profile[i][j]->profile_id,p_usw_queue_master[lchip]->p_pfc_profile[i][j]->type);
            }
            if(count%4 == 3 && p_usw_queue_master[lchip]->p_pfc_profile[i][j])
            {
                SYS_DUMP_DB_LOG(dump_db_fp, "\n%31s"," ");
            }
        }
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","igs_resrc_mode",p_usw_queue_master[lchip]->igs_resrc_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","egs_resrc_mode",p_usw_queue_master[lchip]->egs_resrc_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","store_chan_shp_en",p_usw_queue_master[lchip]->store_chan_shp_en);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","store_que_shp_en",p_usw_queue_master[lchip]->store_que_shp_en);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","monitor_drop_en",p_usw_queue_master[lchip]->monitor_drop_en);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","shp_pps_en",p_usw_queue_master[lchip]->shp_pps_en);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","have_lcpu_by_eth",p_usw_queue_master[lchip]->have_lcpu_by_eth);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","queue_num_per_chanel",p_usw_queue_master[lchip]->queue_num_per_chanel);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","queue_num_per_extend_port",p_usw_queue_master[lchip]->queue_num_per_extend_port);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","max_chan_per_slice",p_usw_queue_master[lchip]->max_chan_per_slice);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","cpu_eth_chan_id",p_usw_queue_master[lchip]->cpu_eth_chan_id);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","max_dma_rx_num",p_usw_queue_master[lchip]->max_dma_rx_num);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","chan_shp_update_freq",p_usw_queue_master[lchip]->chan_shp_update_freq);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","que_shp_update_freq",p_usw_queue_master[lchip]->que_shp_update_freq);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:","que_shp_gran");
    for(i = 0; i < SYS_SHP_GRAN_RANAGE_NUM; i ++)
    {
        SYS_DUMP_DB_LOG(dump_db_fp, "[%u,%u]",p_usw_queue_master[lchip]->que_shp_gran[i].max_rate,p_usw_queue_master[lchip]->que_shp_gran[i].granularity);
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","queue_num_for_cpu_reason",p_usw_queue_master[lchip]->queue_num_for_cpu_reason);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","wrr_weight_mtu",p_usw_queue_master[lchip]->wrr_weight_mtu);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_group_id",p_usw_queue_master[lchip]->opf_type_group_id);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_tcam_index",p_usw_queue_master[lchip]->opf_type_tcam_index);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_queue_shape",p_usw_queue_master[lchip]->opf_type_queue_shape);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_queue_meter",p_usw_queue_master[lchip]->opf_type_queue_meter);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_group_shape",p_usw_queue_master[lchip]->opf_type_group_shape);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_group_meter",p_usw_queue_master[lchip]->opf_type_group_meter);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_queue_drop_wtd",p_usw_queue_master[lchip]->opf_type_queue_drop_wtd);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_queue_drop_wred",p_usw_queue_master[lchip]->opf_type_queue_drop_wred);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_resrc_fc",p_usw_queue_master[lchip]->opf_type_resrc_fc);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_resrc_pfc",p_usw_queue_master[lchip]->opf_type_resrc_pfc);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_excp_index",p_usw_queue_master[lchip]->opf_type_excp_index);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:","egs_pool");
    for(i = 0; i < CTC_QOS_EGS_RESRC_POOL_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(dump_db_fp, "[%u,%u]",p_usw_queue_master[lchip]->egs_pool[i].egs_congest_level_num,p_usw_queue_master[lchip]->egs_pool[i].default_profile_id);
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_resrc_fc_dropthrd",p_usw_queue_master[lchip]->opf_type_resrc_fc_dropthrd);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_resrc_pfc_dropthrd",p_usw_queue_master[lchip]->opf_type_resrc_pfc_dropthrd);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:","p_dropthrd_fc_profile");
    count = 0;
    for(i = 0; i < SYS_MAX_CHANNEL_NUM; i ++)
    {
        if(p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i])
        {
            count ++;
            SYS_DUMP_DB_LOG(dump_db_fp,"[%u:%u,%u,%u,%u,%u]",i,p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i]->xon,p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i]->xoff,p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i]->thrd,
            p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i]->profile_id,p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i]->type);
        }
        if(count%4 == 3 && p_usw_queue_master[lchip]->p_dropthrd_fc_profile[i])
        {
            SYS_DUMP_DB_LOG(dump_db_fp, "\n%31s"," ");
        }
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:","p_dropthrd_pfc_profile");
    count = 0;
    for(i = 0; i < SYS_MAX_CHANNEL_NUM; i ++)
    {
        for(j = 0; j < 8; j ++)
        {
            if(p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j])
            {
                count ++;
                SYS_DUMP_DB_LOG(dump_db_fp,"[%u-%u:%u,%u,%u,%u,%u]",i,j,p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j]->xon,p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j]->xoff,p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j]->thrd,
                p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j]->profile_id,p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j]->type);
            }
            if(count%4 == 3 && p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[i][j])
            {
                SYS_DUMP_DB_LOG(dump_db_fp, "\n%31s"," ");
            }
        }
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","resrc_check_mode",p_usw_queue_master[lchip]->resrc_check_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","service_queue_mode",p_usw_queue_master[lchip]->service_queue_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_group_id, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_tcam_index, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_queue_shape, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_queue_meter, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_group_shape, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_group_meter, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_queue_drop_wtd, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_queue_drop_wred, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_resrc_fc, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_resrc_pfc, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_excp_index, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_resrc_fc_dropthrd, dump_db_fp);
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_queue_master[lchip]->opf_type_resrc_pfc_dropthrd, dump_db_fp);
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_queue_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-15s%-15s%-15s%-7s\n","Node","queue_type","bucket_thrd","bucket_rate","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------");
    cb_data.value0 = dump_db_fp;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_PROFILE;
    cb_data.value2 = &mode;
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_queue_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_METER_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_queue_meter_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-15s%-15s%-7s\n","Node","bucket_thrd","bucket_rate","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_queue_meter_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_DROP_WTD_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_drop_wtd_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-5s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-8s%-6s\n","Node","ecn_td0","ecn_td1","ecn_td2","ecn_td3","wtd_td3","wtd_td2","wtd_td1",
                     "wtd_td0","factor3","factor2","factor1","factor0","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_drop_wtd_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_DROP_WRED_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_drop_wred_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-5s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-8s%-6s\n","Node","min_td3","min_td2","min_td1","min_td0","max_td3","max_td2","max_td1",
                     "max_td0","factor0","factor1","factor2","factor3","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_drop_wred_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_GROUP_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_group_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-15s%-15s%-7s\n","Node","bucket_thrd","bucket_rate","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_group_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_GROUP_METER_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_group_profile_meter_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_group_profile_meter_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_FC_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_fc_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-7s%-7s%-7s%-7s\n","Node","xon","xoff","thrd","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_fc_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_PFC_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_pfc_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-7s%-7s%-7s%-7s\n","Node","xon","xoff","thrd","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_pfc_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_FC_DROP_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_fc_dropthrd_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-7s%-7s%-7s%-7s\n","Node","xon","xoff","thrd","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = SYS_QUEUE_PFC_DROP_PROFILE;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","p_pfc_dropthrd_profile_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-7s%-7s%-7s%-7s\n","Node","xon","xoff","thrd","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    ctc_spool_traverse(p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool, (spool_traversal_fn)_sys_usw_qos_queue_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "}");

    return CTC_E_NONE;
}

/**
 @brief En-queue component initialization.
*/
int32
sys_usw_queue_enq_init(uint8 lchip, void *p_glb_parm)
{
    int32 ret = CTC_E_NONE;
    uint8 gchip = 0;
    uint16 lport = 0;
    uint32 gport = 0;
    uint32 port_type = 0;
    uint16 loop = 0;
    uint8 channel = 0;
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint16 group_num = 0;
    sys_queue_node_t  *p_queue_node = NULL;
    sys_queue_group_node_t *p_sys_group_node = NULL;
    sys_usw_opf_t  opf;
    ctc_qos_global_cfg_t * p_glb_cfg = NULL;
    uint16 chan_num = 0;
    uint32 max_size = 0;
    p_glb_cfg = (ctc_qos_global_cfg_t *)p_glb_parm;

    if (NULL != p_usw_queue_master[lchip])
    {
        return CTC_E_NONE;
    }
    /*check queue mode*/
    if ((CTC_QOS_PORT_QUEUE_NUM_8 != p_glb_cfg->queue_num_per_network_port)
        && (CTC_QOS_PORT_QUEUE_NUM_16 != p_glb_cfg->queue_num_per_network_port))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((128 != p_glb_cfg->queue_num_for_cpu_reason)
        && (64 != p_glb_cfg->queue_num_for_cpu_reason)
        && (32 != p_glb_cfg->queue_num_for_cpu_reason))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_glb_cfg->queue_num_per_internal_port)
    {
        p_glb_cfg->queue_num_per_internal_port = MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
    }

    p_usw_queue_master[lchip] = mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_queue_master_t));
    if (NULL == p_usw_queue_master[lchip])
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    sal_memset(p_usw_queue_master[lchip], 0, sizeof(sys_queue_master_t));
    p_usw_queue_master[lchip]->service_queue_mode = p_glb_cfg->service_queue_mode;
    if (p_usw_queue_master[lchip]->service_queue_mode == 2 && DRV_IS_DUET2(lchip))
    {
        return CTC_E_INVALID_PARAM;
    }
    if (p_usw_queue_master[lchip]->service_queue_mode == 2)
    {
        field_val = 1;
        cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_useNextHopPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    if (CTC_QOS_PORT_QUEUE_NUM_16 == p_glb_cfg->queue_num_per_network_port)
    {
        p_usw_queue_master[lchip]->queue_num_per_chanel = DRV_IS_DUET2(lchip) ? 13 :10;
        p_usw_queue_master[lchip]->enq_mode = 1;
    }
    else
    {
        p_usw_queue_master[lchip]->queue_num_per_chanel = 8;
        p_usw_queue_master[lchip]->enq_mode = 0;
    }
    p_usw_queue_master[lchip]->wrr_weight_mtu = (SYS_WFQ_DEFAULT_WEIGHT << SYS_WFQ_DEFAULT_SHIFT);
    /*1280 queue,80 * 16 = 1280 queue*/
    if (DRV_IS_DUET2(lchip))
    {
        p_usw_queue_master[lchip]->queue_vec = ctc_vector_init(80, 16);
    }
    else if(DRV_IS_TSINGMA(lchip))
    {
        p_usw_queue_master[lchip]->queue_vec = ctc_vector_init(128, 32);
    }
    if (NULL == p_usw_queue_master[lchip]->queue_vec)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    for (loop = 0; loop < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM); loop++)
    {
        p_queue_node = (sys_queue_node_t*)mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_queue_node_t));
        if (!p_queue_node)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            ret = CTC_E_NO_MEMORY;
            goto error2;
        }

        sal_memset(p_queue_node, 0, sizeof(sys_queue_node_t));
        p_queue_node->queue_id = loop;
        if(loop < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC))
        {
            p_queue_node->offset = loop % 8;
        }
        else if(loop >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC) &&
                loop < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))
        {
            p_queue_node->offset = loop % 4;
        }
        else
        {
            p_queue_node->offset = loop % MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        }
        (void)ctc_vector_add(p_usw_queue_master[lchip]->queue_vec, loop, p_queue_node);
    }

    /*64 ext group*/
    p_usw_queue_master[lchip]->group_vec = ctc_vector_init(1, MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM));
    if (NULL == p_usw_queue_master[lchip]->group_vec)
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }

    for (loop = 0; loop < MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); loop++)
    {
        p_sys_group_node = (sys_queue_group_node_t*)mem_malloc(MEM_QUEUE_MODULE, sizeof(sys_queue_group_node_t));
        if (!p_sys_group_node)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            ret = CTC_E_NO_MEMORY;
            goto error3;
        }

        sal_memset(p_sys_group_node, 0, sizeof(sys_queue_group_node_t));
        p_sys_group_node->group_id = loop;
        p_sys_group_node->chan_id = 0xFF;
        (void)ctc_vector_add(p_usw_queue_master[lchip]->group_vec, loop, p_sys_group_node);
    }

    p_usw_queue_master[lchip]->p_port_extender_hash = ctc_hash_create(
                                                  SYS_SERVICE_HASH_BLOCK_NUM,
                                                  SYS_SERVICE_HASH_BLOCK_SIZE,
                                                  (hash_key_fn)sys_usw_port_extender_hash_make,
                                                  (hash_cmp_fn)sys_usw_port_extender_hash_cmp);
    if (NULL == p_usw_queue_master[lchip]->p_port_extender_hash)
    {
        ret = CTC_E_NO_MEMORY;
        goto error3;
    }

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_group_id,
                                                                    1, "opf-queue-group-id"), ret, error4);

    opf.pool_index = 0;
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_group_id;
    if (DRV_IS_TSINGMA(lchip))
    {
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM)-1), ret, error5);
    }
    else
    {
        /* D2 enq_mode == 1 not support span on drop*/
        group_num = p_usw_queue_master[lchip]->enq_mode ? MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM) : MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM)-1;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, group_num), ret, error5);
    }
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_tcam_index,
                                                                    1, "opf-queue-tcam-index"), ret, error5);

    opf.pool_index = 0;
    opf.pool_type  = p_usw_queue_master[lchip]->opf_type_tcam_index;
    sys_usw_ftm_query_table_entry_num(lchip, DsQueueMapTcamKey_t, &max_size);
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, max_size), ret, error6);

    /*************************************************
    *init drop channel
    *************************************************/
    CTC_ERROR_GOTO(_sys_usw_rsv_channel_drop_init(lchip), ret, error6);

    /*************************************************
    *init queue stats base
    *************************************************/
    field_val = 0;
    cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_queStatsEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error6);

    cmd = DRV_IOW(GlobalStatsUpdateEnCtl_t, GlobalStatsUpdateEnCtl_qMgrGlobalStatsUpdateEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error6);
    CTC_ERROR_GOTO(sys_usw_queue_init_qmgr(lchip), ret, error6);

   /*************************************************
    *init queue alloc */
    CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip), ret, error6);

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PORT_NUM); lport++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
#if (SDK_WORK_PLATFORM == 0) && (0 == SDK_WORK_ENV)
        sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type);
        if (SYS_DMPS_RSV_PORT == port_type)
        {
            channel = MCHIP_CAP(SYS_CAP_CHANID_DROP);
        }
        else
        {
            channel = SYS_GET_CHANNEL_ID(lchip, gport);
        }
        if(DRV_IS_DUET2(lchip) && SYS_DMPS_NETWORK_PORT == port_type &&  p_usw_queue_master[lchip]->enq_mode == 1)
        {
            channel = SYS_GET_CHANNEL_ID(lchip, gport);
            CTC_ERROR_GOTO(_sys_usw_qos_add_mcast_queue_to_channel(lchip, lport, channel), ret, error6);
        }
#else
        port_type = port_type;
        if (lport < SYS_RSV_PORT_START)
        {
            channel = lport;
        }
        else
        {
            channel = SYS_GET_CHANNEL_ID(lchip, gport);
        }
        if(DRV_IS_DUET2(lchip) && lport < 40 &&  p_usw_queue_master[lchip]->enq_mode == 1)
        {
            channel = SYS_GET_CHANNEL_ID(lchip, gport);
            CTC_ERROR_GOTO(_sys_usw_qos_add_mcast_queue_to_channel(lchip, lport, channel), ret, error6);
        }
#endif
        _sys_usw_add_port_to_channel(lchip, lport, channel);
    }

    sys_usw_dma_get_packet_rx_chan(lchip, &chan_num);

    p_usw_queue_master[lchip]->max_dma_rx_num = chan_num;  /* must get from DMA module */

    p_usw_queue_master[lchip]->queue_num_per_extend_port = p_glb_cfg->queue_num_per_internal_port;

    if (0 == p_usw_queue_master[lchip]->enq_mode)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); lport++)
        {
            /*only use from cpu queue of ctl que*/
            /* SYSTEM MODIFIED by hansf 2018/12/3 for: cpu send packet into port 8 queue, not use NetCtl queue */
            //field_val = 0xB;
            field_val = 0xF;
            cmd = DRV_IOW(QWriteNetChannel_t, QWriteNetChannel_miscQueueDisable_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &field_val), ret, error6);
        }
    }
    else if(2 == p_usw_queue_master[lchip]->enq_mode || (1 == p_usw_queue_master[lchip]->enq_mode && DRV_IS_TSINGMA(lchip)))
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); lport++)
        {
            /*only use mcast and log que*/
            field_val = 0xC;
            cmd = DRV_IOW(QWriteNetChannel_t, QWriteNetChannel_miscQueueDisable_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &field_val), ret, error6);
        }
    }
    else if(1 == p_usw_queue_master[lchip]->enq_mode && DRV_IS_DUET2(lchip))
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); lport++)
        {
            /*only use log que*/
            field_val = 0xE;
            cmd = DRV_IOW(QWriteNetChannel_t, QWriteNetChannel_miscQueueDisable_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &field_val), ret, error6);
        }
    }

    p_usw_queue_master[lchip]->queue_num_for_cpu_reason = p_glb_cfg->queue_num_for_cpu_reason;
    if ((chan_num == 1) && (p_glb_cfg->queue_num_for_cpu_reason > 64))
    {
        p_usw_queue_master[lchip]->queue_num_for_cpu_reason = 64;
    }

    p_usw_queue_master[lchip]->max_chan_per_slice = MCHIP_CAP(SYS_CAP_CHANID_DROP); /*channel enq*/

    CTC_ERROR_GOTO(sys_usw_get_chip_cpu_eth_en(lchip, &p_usw_queue_master[lchip]->have_lcpu_by_eth,
                                                        &p_usw_queue_master[lchip]->cpu_eth_chan_id), ret, error6);

    if (p_usw_queue_master[lchip]->have_lcpu_by_eth)
    {
        CTC_ERROR_GOTO(sys_usw_chip_set_eth_cpu_cfg(lchip), ret, error6);
    }
    p_usw_queue_master[lchip]->p_service_hash =
    ctc_hash_create(
            SYS_SERVICE_HASH_BLOCK_NUM,
            SYS_SERVICE_HASH_BLOCK_SIZE,
            (hash_key_fn)sys_usw_service_hash_make,
            (hash_cmp_fn)sys_usw_service_hash_cmp);
    if (NULL == p_usw_queue_master[lchip]->p_service_hash)
    {
        ret = CTC_E_NO_MEMORY;
        goto error6;
    }

    p_usw_queue_master[lchip]->p_logicport_service_hash =
    ctc_hash_create(
            SYS_SERVICE_HASH_BLOCK_NUM,
            SYS_SERVICE_HASH_BLOCK_SIZE,
            (hash_key_fn)sys_usw_logicport_service_hash_make,
            (hash_cmp_fn)sys_usw_logicport_service_hash_cmp);
    if (NULL == p_usw_queue_master[lchip]->p_logicport_service_hash)
    {
        ret = CTC_E_NO_MEMORY;
        goto error7;
    }
    return CTC_E_NONE;
error7:
    ctc_hash_traverse(p_usw_queue_master[lchip]->p_service_hash, (hash_traversal_fn)_sys_usw_queue_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_queue_master[lchip]->p_service_hash);
error6:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_tcam_index);
error5:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_group_id);
error4:
    ctc_hash_traverse(p_usw_queue_master[lchip]->p_port_extender_hash, (hash_traversal_fn)_sys_usw_queue_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_queue_master[lchip]->p_port_extender_hash);
error3:
    ctc_vector_traverse(p_usw_queue_master[lchip]->group_vec, (vector_traversal_fn)_sys_usw_queue_vec_free_node_data, NULL);
    ctc_vector_release(p_usw_queue_master[lchip]->group_vec);
error2:
    ctc_vector_traverse(p_usw_queue_master[lchip]->queue_vec, (vector_traversal_fn)_sys_usw_queue_vec_free_node_data, NULL);
    ctc_vector_release(p_usw_queue_master[lchip]->queue_vec);
error1:
    mem_free(p_usw_queue_master[lchip]);
error0:
    return ret;
}

int32
sys_usw_queue_enq_deinit(uint8 lchip)
{
    if (NULL == p_usw_queue_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_tcam_index);

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_group_id);

    ctc_hash_traverse(p_usw_queue_master[lchip]->p_port_extender_hash, (hash_traversal_fn)_sys_usw_queue_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_queue_master[lchip]->p_port_extender_hash);

    ctc_vector_traverse(p_usw_queue_master[lchip]->group_vec, (vector_traversal_fn)_sys_usw_queue_vec_free_node_data, NULL);
    ctc_vector_release(p_usw_queue_master[lchip]->group_vec);

    ctc_vector_traverse(p_usw_queue_master[lchip]->queue_vec, (vector_traversal_fn)_sys_usw_queue_vec_free_node_data, NULL);
    ctc_vector_release(p_usw_queue_master[lchip]->queue_vec);

    ctc_hash_traverse(p_usw_queue_master[lchip]->p_service_hash, (vector_traversal_fn)_sys_usw_queue_hash_free_list_node_data, NULL);
    ctc_hash_free(p_usw_queue_master[lchip]->p_service_hash);

    ctc_hash_traverse(p_usw_queue_master[lchip]->p_logicport_service_hash, (vector_traversal_fn)_sys_usw_queue_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_queue_master[lchip]->p_logicport_service_hash);

    mem_free(p_usw_queue_master[lchip]);

    return CTC_E_NONE;
}

