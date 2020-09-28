/**
 @file sys_usw_mac_stats.c

 @date 2009-12-22

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_stats.h"
#include "ctc_hash.h"
#include "sys_usw_common.h"
#include "sys_usw_stats.h"
#include "sys_usw_ftm.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_dma.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_vlan.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "sys_usw_dmps.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
sys_mac_stats_master_t* usw_mac_stats_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
static sys_stats_mac_throughput_t mac_throughput[CTC_MAX_LOCAL_CHIP_NUM];

#define ___________MAC_STATS_COMMON___________

#define SYS_STATS_INIT_CHECK() \
    do { \
        SYS_LCHIP_CHECK_ACTIVE(lchip); \
        if (usw_mac_stats_master[lchip] == NULL){ \
            SYS_STATS_DBG_ERROR(" Feature not initialized \n"); \
			return CTC_E_NOT_INIT; \
        } \
    } while (0)

#define STATS_LOCK() \
    do { \
        if (usw_mac_stats_master[lchip]->p_stats_mutex) sal_mutex_lock(usw_mac_stats_master[lchip]->p_stats_mutex); \
    } while (0)

#define STATS_UNLOCK() \
    do { \
        if (usw_mac_stats_master[lchip]->p_stats_mutex) sal_mutex_unlock(usw_mac_stats_master[lchip]->p_stats_mutex); \
    } while (0)

#define CTC_ERROR_RETURN_UNLOCK(op) \
    do { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            sal_mutex_unlock(usw_mac_stats_master[lchip]->p_stats_mutex); \
            return (rv); \
        } \
    } while (0)

#define RX_TYPE(_TYPE_) (_TYPE_)
#define TX_TYPE(_TYPE_) (_TYPE_ - SYS_STATS_MAC_SEND_UCAST)

/***************************************************************
 *
 * Function
 *
 ***************************************************************/

#define ___________MAC_STATS_WB________________________

int32
sys_usw_mac_stats_wb_sync(uint8 lchip)
{
    uint8 gchip = 0;
    uint16 index = 0;
    uint16 max_entry_cnt = 0;
    uint32 gport = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_mac_stats_t* p_wb_mac_stats = NULL;
    ctc_mac_stats_t mac_stats;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }
    sal_memset(&mac_stats, 0, sizeof(ctc_mac_stats_t));
    sys_usw_get_gchip_id(lchip, &gchip);
    for (index = 0; index < SYS_USW_MAX_PORT_NUM_PER_CHIP; index++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(index));

        if ((index & 0xFF) < SYS_PHY_PORT_NUM_PER_SLICE)
        {
            /*befor warmboot, read and clear mac stats*/
            sys_usw_mac_stats_get_rx_stats(lchip, gport, &mac_stats);
            sys_usw_mac_stats_get_tx_stats(lchip, gport, &mac_stats);
        }
    }

    /*syncup mac stats*/
    sal_memset(&wb_data, 0, sizeof(wb_data));
    wb_data.buffer = mem_malloc(MEM_STATS_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_mac_stats_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_MAC_STATS);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);

    STATS_LOCK();
    for (index = 0; index < SYS_STATS_XQMAC_RAM_MAX; index++)
    {
        p_wb_mac_stats = (sys_wb_mac_stats_t *)wb_data.buffer + wb_data.valid_cnt;
        p_wb_mac_stats->index = index;
        sal_memcpy(&p_wb_mac_stats->qmac_stats_table, &usw_mac_stats_master[lchip]->qmac_stats_table[index], sizeof(sys_wb_qmac_stats_t));

        if (++wb_data.valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
            wb_data.valid_cnt = 0;
        }
    }
    STATS_UNLOCK();

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

    return ret;
}

int32
_sys_usw_mac_stats_wb_restore(uint8 lchip)
{
    uint16 index = 0;
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_mac_stats_t* p_wb_mac_stats = NULL;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    p_wb_mac_stats = (sys_wb_mac_stats_t*)mem_malloc(MEM_STATS_MODULE, sizeof(sys_wb_mac_stats_t));
    wb_query.buffer = mem_malloc(MEM_STATS_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if ((NULL == wb_query.buffer) || (NULL == p_wb_mac_stats))
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(p_wb_mac_stats, 0, sizeof(sys_wb_mac_stats_t));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_mac_stats_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_MAC_STATS);

    STATS_LOCK();
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_mac_stats, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        index = p_wb_mac_stats->index;
        sal_memcpy(&usw_mac_stats_master[lchip]->qmac_stats_table[index], &p_wb_mac_stats->qmac_stats_table, sizeof(sys_wb_qmac_stats_t));
    CTC_WB_QUERY_ENTRY_END((&wb_query));
    STATS_UNLOCK();

done:

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    if (p_wb_mac_stats)
    {
        mem_free(p_wb_mac_stats);
    }

    return ret;
}

#define ___________MAC_STATS_FUNCTION________________________

STATIC int32
_sys_usw_mac_stats_get_property_field(uint8 lchip, sys_mac_stats_property_t property, uint32* p_filed_id)
{
    switch (property)
    {
    case SYS_MAC_STATS_PROPERTY_CLEAR:
        *p_filed_id = QuadSgmacStatsCfg0_clearOnRead_f;
        break;

    case SYS_MAC_STATS_PROPERTY_HOLD:
        *p_filed_id = QuadSgmacStatsCfg0_incrHold_f;
        break;

    case SYS_MAC_STATS_PROPERTY_SATURATE:
        *p_filed_id = QuadSgmacStatsCfg0_incrSaturate_f;
        break;

    case SYS_MAC_STATS_PROPERTY_MTU1:
        *p_filed_id = QuadSgmacStatsCfg0_packetLenMtu1_f;
        break;

    case SYS_MAC_STATS_PROPERTY_MTU2:
        *p_filed_id = QuadSgmacStatsCfg0_packetLenMtu2_f;
        break;

    case SYS_MAC_STATS_PROPERTY_PFC:
        *p_filed_id = QuadSgmacStatsCfg0_pfcPriStatsEnable_f;
        break;

    default:
        return CTC_E_INVALID_PARAM;

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_get_property_table(uint8 lchip, uint16 lport, uint32* p_tbl_id)
{
    uint32 mac_id = 0;
    uint8 gchip = 0;
    uint32 gport = 0;
    uint32 port_type = 0;

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_ID, (void *)&mac_id));
    *p_tbl_id = QuadSgmacStatsCfg0_t + ((mac_id >> 2)&0xF);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_set_global_property(uint8 lchip, sys_mac_stats_property_t property, uint32 value)
{
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    uint32 cmd = 0;

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property_field(lchip, property, &field_id));

    for (tbl_id = QuadSgmacStatsCfg0_t; tbl_id <= QuadSgmacStatsCfg15_t; tbl_id++)
    {
        cmd = DRV_IOW(tbl_id, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_stats_get_global_property(uint8 lchip, sys_mac_stats_property_t property, uint32* p_value)
{
    uint32 tbl_id = QuadSgmacStatsCfg0_t;
    uint32 field_id = 0;
    uint32 cmd = 0;

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property_field(lchip, property, &field_id));

    cmd = DRV_IOR(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_set_property(uint8 lchip, uint32 gport, sys_mac_stats_property_t property, uint32 value)
{
    uint16 lport = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    uint32 cmd = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property_table(lchip, lport, &tbl_id));
    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property_field(lchip, property, &field_id));

    cmd = DRV_IOW(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_get_property(uint8 lchip, uint32 gport, sys_mac_stats_property_t property, uint32* p_value)
{
    uint16 lport = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    uint32 cmd = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property_table(lchip, lport, &tbl_id));
    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property_field(lchip, property, &field_id));

    cmd = DRV_IOR(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_value));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_mac_stats_stats_to_basic(uint8 lchip, void* p_stats_ram, ctc_stats_basic_t* p_basic_stats)
{
    uint64 tmp = 0;
    QuadSgmacStatsRam0_m* p_quadmac_stats_ram = NULL;


    CTC_PTR_VALID_CHECK(p_basic_stats);

    p_quadmac_stats_ram = (QuadSgmacStatsRam0_m*)p_stats_ram;

    tmp = GetQuadSgmacStatsRam0(V, frameCntDataHi_f, p_quadmac_stats_ram);
    tmp <<= 32;
    tmp |= GetQuadSgmacStatsRam0(V, frameCntDataLo_f, p_quadmac_stats_ram);

    p_basic_stats->packet_count = tmp;

    tmp = GetQuadSgmacStatsRam0(V, byteCntDataHi_f, p_quadmac_stats_ram);
    tmp <<= 32;
    tmp |= GetQuadSgmacStatsRam0(V, byteCntDataLo_f, p_quadmac_stats_ram);

    p_basic_stats->byte_count = tmp;


    return CTC_E_NONE;

}

STATIC int32
_sys_usw_mac_stats_write_sys_mac_stats(uint8 lchip, uint8 stats_type, void* p_mac_stats, ctc_stats_basic_t* p_stats_basic)
{
    sys_mac_stats_rx_t* p_mac_stats_rx = NULL;
    sys_mac_stats_tx_t* p_mac_stats_tx = NULL;

    if (stats_type < SYS_STATS_MAC_RCV_NUM)
    {
        p_mac_stats_rx = (sys_mac_stats_rx_t*)p_mac_stats;

        p_mac_stats_rx->mac_stats_rx_pkts[stats_type] += p_stats_basic->packet_count;
        p_mac_stats_rx->mac_stats_rx_bytes[stats_type] += p_stats_basic->byte_count;
    }
    else
    {
        p_mac_stats_tx = (sys_mac_stats_tx_t*)p_mac_stats;
        p_mac_stats_tx->mac_stats_tx_pkts[stats_type - SYS_STATS_MAC_SEND_UCAST] += p_stats_basic->packet_count;
        p_mac_stats_tx->mac_stats_tx_bytes[stats_type - SYS_STATS_MAC_SEND_UCAST] += p_stats_basic->byte_count;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_get_stats_tbl_info(uint8 lchip, uint16 lport, uint32* p_tbl_id, uint8* p_tbl_base)
{
    uint32 mac_stats_id = 0;
    uint8 gchip = 0;
    uint32 gport = 0;
    uint32 port_type = 0;
    uint32 tbl_step = 0;

    if(p_tbl_base == NULL || p_tbl_id == NULL)
    {
        SYS_STATS_DBG_ERROR(" Invalid pointer \n");
        return CTC_E_INVALID_PTR;
    }

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_STATS_ID, (void *)&mac_stats_id));

    SYS_STATS_DBG_INFO("Mac stats id is %u\n", mac_stats_id);
    tbl_step = QuadSgmacStatsRam1_t - QuadSgmacStatsRam0_t;
    *p_tbl_id = QuadSgmacStatsRam0_t + (mac_stats_id >> 2)* tbl_step;
    *p_tbl_base = (mac_stats_id & 0x3) * (SYS_STATS_MAC_BASED_STATS_XQMAC_RAM_DEPTH / 4);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_get_hw_stats(uint8 lchip, uint16 lport, uint8 stats_type, void* p_mac_stats)
{
    uint8  tbl_base = 0;
    uint32 cmdr = 0;
    uint32 cmdw = 0;
    uint32 tbl_id = 0;
    QuadSgmacStatsRam0_m mac_stats;
    ctc_stats_basic_t stats_basic;
    QuadSgmacStatsRam0_m zero;
    drv_work_platform_type_t platform_type = MAX_WORK_PLATFORM;

    sal_memset(&mac_stats, 0, sizeof(QuadSgmacStatsRam0_m));
    sal_memset(&zero, 0, sizeof(QuadSgmacStatsRam0_m));
    sal_memset(&stats_basic, 0, sizeof(ctc_stats_basic_t));

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &tbl_base));
    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));

    cmdr = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);

    SYS_STATS_DBG_INFO("read mac stats tbl %s[%d] \n", TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, stats_type + tbl_base, cmdr, &mac_stats));

    CTC_ERROR_RETURN(_sys_usw_mac_stats_stats_to_basic(lchip,  &mac_stats, &stats_basic));
    SYS_STATS_DBG_INFO("pkts:%"PRId64" bytes: %"PRId64"\n", stats_basic.packet_count, stats_basic.byte_count);

    /* for uml, add this code to support clear after read */
    if (SW_SIM_PLATFORM == platform_type)
    {
        SYS_STATS_DBG_INFO("clear %s[%d] by software in uml\n", TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, stats_type + tbl_base, cmdw, &zero));
    }

    CTC_ERROR_RETURN(_sys_usw_mac_stats_write_sys_mac_stats(lchip, stats_type, p_mac_stats, &stats_basic));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_mac_stats_get_sw_stats(uint8 lchip, uint8 mac_type, uint32 gport, uint8 stats_type, void* p_mac_stats)
{
    sys_mac_stats_rx_t* p_stats_rx = (sys_mac_stats_rx_t*)p_mac_stats;
    sys_mac_stats_tx_t* p_stats_tx = (sys_mac_stats_tx_t*)p_mac_stats;
    uint32 mac_stats_id = 0;
    sys_mac_stats_rx_t* p_stats_db_rx = NULL;
    sys_mac_stats_tx_t* p_stats_db_tx = NULL;

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_STATS_ID, (void *)&mac_stats_id));
    p_stats_db_rx = &usw_mac_stats_master[lchip]->qmac_stats_table[mac_type].mac_stats_rx[mac_stats_id%4];
    p_stats_db_tx = &usw_mac_stats_master[lchip]->qmac_stats_table[mac_type].mac_stats_tx[mac_stats_id%4];

    if (stats_type < SYS_STATS_MAC_RCV_NUM)
    {
        p_stats_rx->mac_stats_rx_pkts[RX_TYPE(stats_type)] += p_stats_db_rx->mac_stats_rx_pkts[stats_type];
        p_stats_rx->mac_stats_rx_bytes[RX_TYPE(stats_type)] += p_stats_db_rx->mac_stats_rx_bytes[stats_type];
    }
    else
    {
        p_stats_tx->mac_stats_tx_pkts[TX_TYPE(stats_type)] += p_stats_db_tx->mac_stats_tx_pkts[stats_type - SYS_STATS_MAC_SEND_UCAST];
        p_stats_tx->mac_stats_tx_bytes[TX_TYPE(stats_type)] += p_stats_db_tx->mac_stats_tx_bytes[stats_type - SYS_STATS_MAC_SEND_UCAST];
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_get_stats_db(uint8 lchip, uint8 mac_type, uint8 stats_type, uint32 gport, void** pp_mac_stats)
{
    uint32 mac_stats_id = 0;
    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_STATS_ID, (void *)&mac_stats_id));

    if (stats_type < SYS_STATS_MAC_RCV_NUM)
    {
        *pp_mac_stats = &usw_mac_stats_master[lchip]->qmac_stats_table[mac_type].mac_stats_rx[mac_stats_id % 4];
    }
    else
    {
        *pp_mac_stats = &usw_mac_stats_master[lchip]->qmac_stats_table[mac_type].mac_stats_tx[mac_stats_id% 4];
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_get_throughput(uint8 lchip, uint32 gport, uint64* p_throughput, sal_systime_t* p_systime)
{
    uint16 drv_lport = 0, sys_lport = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, drv_lport);
    sys_lport = SYS_MAP_DRV_LPORT_TO_SYS_LPORT(drv_lport);

    p_throughput[CTC_STATS_MAC_STATS_RX] = mac_throughput[lchip].bytes[sys_lport][CTC_STATS_MAC_STATS_RX];
    p_throughput[CTC_STATS_MAC_STATS_TX] = mac_throughput[lchip].bytes[sys_lport][CTC_STATS_MAC_STATS_TX];
    sal_memcpy(p_systime, &(mac_throughput[lchip].timestamp[sys_lport]), sizeof(sal_systime_t));

    SYS_STATS_DBG_INFO("%s %d, tv_sec:%u, tv_usec:%u ms.\n",
                       __FUNCTION__, __LINE__,
                       mac_throughput[lchip].timestamp[sys_lport].tv_sec,
                       mac_throughput[lchip].timestamp[sys_lport].tv_usec);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_stats_sync_throughput(uint8 lchip, uint16 lport, uint8* p_addr)
{
    int32  ret = CTC_E_NONE;
    void*  p_mac_stats = NULL;
    uint8  gchip_id = 0;
    uint8  stats_type = 0;
    uint16 stats_offset = 0;
    uint8  tbl_base = 0;
    uint32 cmdr = 0;
    uint32 mac_ipg = 0;
    uint32 tbl_id = 0;
    uint32 tbl_id_base = QuadSgmacStatsRam0_t;
    uint32 gport = 0;
    ctc_mac_stats_dir_t dir = CTC_STATS_MAC_STATS_RX;
    QuadSgmacStatsRam0_m mac_stats;
    ctc_stats_basic_t stats_basic;

    SYS_STATS_INIT_CHECK();

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &tbl_base));
    cmdr = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);

    sys_usw_get_gchip_id(lchip, &gchip_id);
    gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, lport);

    for (stats_type = SYS_STATS_MAC_RCV_GOOD_UCAST; stats_type < SYS_STATS_MAC_STATS_TYPE_NUM; stats_type++)
    {
        if (!((SYS_STATS_MAC_RCV_GOOD_UCAST == stats_type) || (SYS_STATS_MAC_RCV_GOOD_MCAST == stats_type)
           || (SYS_STATS_MAC_RCV_GOOD_BCAST == stats_type) || (SYS_STATS_MAC_SEND_UCAST == stats_type)
           || (SYS_STATS_MAC_SEND_MCAST == stats_type) || (SYS_STATS_MAC_SEND_BCAST == stats_type)))
        {
            continue;
        }

        sal_memset(&mac_stats, 0, sizeof(mac_stats));
        sal_memset(&stats_basic, 0, sizeof(ctc_stats_basic_t));

        if (NULL != p_addr)
        {
            stats_offset = stats_type * DRV_ADDR_BYTES_PER_ENTRY;
            sal_memcpy(&mac_stats, (uint8*)p_addr + stats_offset, sizeof(mac_stats));
            CTC_ERROR_RETURN(_sys_usw_mac_stats_stats_to_basic(lchip, &mac_stats, &stats_basic));
        }
        else
        {
            SYS_STATS_DBG_INFO("read mac stats tbl %s[%d] \n", TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, stats_type + tbl_base, cmdr, &mac_stats));
            CTC_ERROR_RETURN(_sys_usw_mac_stats_stats_to_basic(lchip, &mac_stats, &stats_basic));
            CTC_ERROR_RETURN(_sys_usw_mac_stats_get_stats_db(lchip, tbl_id-tbl_id_base, stats_type, gport, (void**)(&p_mac_stats)));
            CTC_ERROR_RETURN(_sys_usw_mac_stats_write_sys_mac_stats(lchip, stats_type, p_mac_stats, &stats_basic));
        }

        dir = (stats_type < SYS_STATS_MAC_SEND_UCAST) ? CTC_STATS_MAC_STATS_RX : CTC_STATS_MAC_STATS_TX;
        ret = sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_TX_IPG, &mac_ipg);
        if (ret < 0)
        {
            mac_ipg = 12;
        }
        mac_throughput[lchip].bytes[lport][dir] += (stats_basic.byte_count + stats_basic.packet_count*(8+mac_ipg));
    }
    sal_gettime(&(mac_throughput[lchip].timestamp[lport]));

    return CTC_E_NONE;
}

STATIC int32
sys_usw_mac_stats_sync_dma_stats(uint8 lchip, void* p_data)
{
    void*  p_mac_stats = NULL;
    uint8  stats_type = 0;
    uint16 stats_offset = 0;
    uint16 lport = 0;
    uint32 gport = 0;
    uint32 tbl_id = 0;
    uint32 tbl_id_base = QuadSgmacStatsRam0_t;
    uint8  tbl_base = 0;
    uint8  mac_id = 0;
    uint8  gchip_id = 0;
    uint8* p_addr = NULL;
    QuadSgmacStatsRam0_m stats_ram;
    ctc_stats_basic_t stats_basic;
    sys_dma_reg_t* p_dma_reg = (sys_dma_reg_t*)p_data;
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_data);

    mac_id = *((uint8*)p_dma_reg->p_ext);
    p_addr = p_dma_reg->p_data;

    STATS_LOCK();

    if (DRV_IS_DUET2(lchip))
    {
        lport = sys_usw_dmps_get_lport_with_mac(lchip, mac_id);
    }
    else
    {
        lport = sys_usw_dmps_get_lport_with_mac_tbl_id(lchip, mac_id);
    }
    if (SYS_COMMON_USELESS_MAC == lport)
    {
        STATS_UNLOCK();
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN_UNLOCK(sys_usw_get_gchip_id(lchip, &gchip_id));
    gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, lport);
    /*1. get table id in order to get db */
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &tbl_base));

    sal_memset(&stats_ram, 0, sizeof(QuadSgmacStatsRam0_m));

    /*2. get detail info from dma data & store in db*/
    for (stats_type = 0; stats_type < SYS_STATS_MAC_STATS_TYPE_NUM; stats_type++)
    {
        _sys_usw_mac_stats_get_stats_db(lchip, tbl_id-tbl_id_base, stats_type, gport, (void**)(&p_mac_stats));
        stats_offset = stats_type * DRV_ADDR_BYTES_PER_ENTRY;

        sal_memcpy(&stats_ram, (uint8*)p_addr + stats_offset, sizeof(stats_ram));
        sal_memset(&stats_basic, 0, sizeof(ctc_stats_basic_t));

        _sys_usw_mac_stats_stats_to_basic(lchip, &stats_ram, &stats_basic);
        _sys_usw_mac_stats_write_sys_mac_stats(lchip, stats_type, p_mac_stats, &stats_basic);
    }

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_sync_throughput(lchip, lport, p_addr));
    STATS_UNLOCK();

    return CTC_E_NONE;
}

#define ___________MAC_STATS_API________________________
int32
sys_usw_mac_stats_set_global_property_hold(uint8 lchip, bool enable)
{
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    tmp = (FALSE == enable) ? 0 : 1;

    CTC_ERROR_RETURN(_sys_usw_mac_stats_set_global_property(lchip, SYS_MAC_STATS_PROPERTY_HOLD, tmp));

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_global_property_hold(uint8 lchip, bool* p_enable)
{
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_global_property(lchip, SYS_MAC_STATS_PROPERTY_HOLD, &tmp));

    *p_enable = (tmp>0 ? TRUE: FALSE);

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_set_global_property_saturate(uint8 lchip, bool enable)
{
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    tmp = (FALSE == enable) ? 0 : 1;

    CTC_ERROR_RETURN(_sys_usw_mac_stats_set_global_property(lchip, SYS_MAC_STATS_PROPERTY_SATURATE, tmp));

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_global_property_saturate(uint8 lchip, bool* p_enable)
{
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_global_property(lchip, SYS_MAC_STATS_PROPERTY_SATURATE, &tmp));

    *p_enable = (tmp>0 ? TRUE: FALSE);

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_set_global_property_clear_after_read(uint8 lchip, bool enable)
{
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    tmp = (FALSE == enable) ? 0 : 1;

    CTC_ERROR_RETURN(_sys_usw_mac_stats_set_global_property(lchip, SYS_MAC_STATS_PROPERTY_CLEAR, tmp));

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_global_property_clear_after_read(uint8 lchip, bool* p_enable)
{
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_global_property(lchip, SYS_MAC_STATS_PROPERTY_CLEAR, &tmp));

    *p_enable = (tmp>0 ? TRUE: FALSE);

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_set_property_mtu1(uint8 lchip, uint32 gport, uint16 length)
{
    uint32 mtu2_len = 0;
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    tmp = length;

    CTC_VALUE_RANGE_CHECK(tmp, SYS_STATS_MTU_PKT_MIN_LENGTH, SYS_STATS_MTU_PKT_MAX_LENGTH);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property(lchip, gport, SYS_MAC_STATS_PROPERTY_MTU2, &mtu2_len));
    if ((tmp >= mtu2_len) && (0 != mtu2_len))
    {
        return CTC_E_INVALID_PARAM;
    }
    CTC_ERROR_RETURN(_sys_usw_mac_stats_set_property(lchip, gport, SYS_MAC_STATS_PROPERTY_MTU1, length));

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_property_mtu1(uint8 lchip, uint32 gport, uint16* p_length)
{
    uint32 value = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_length);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property(lchip, gport, SYS_MAC_STATS_PROPERTY_MTU1, &value));
    *p_length = (uint16)value;

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_set_property_mtu2(uint8 lchip, uint32 gport, uint16 length)
{
    uint32 mtu1_len = 0;
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    tmp = length;

    CTC_MIN_VALUE_CHECK(tmp, SYS_STATS_MTU_PKT_MIN_LENGTH);
    CTC_MAX_VALUE_CHECK(tmp, SYS_STATS_MTU_PKT_MAX_LENGTH);

    /*mtu2 length must greater than mtu1 length*/
    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property(lchip, gport, SYS_MAC_STATS_PROPERTY_MTU1, &mtu1_len));
    if (tmp <= mtu1_len)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_MAX_VALUE_CHECK(tmp, SYS_STATS_MTU_PKT_MAX_LENGTH);
    CTC_ERROR_RETURN(_sys_usw_mac_stats_set_property(lchip, gport, SYS_MAC_STATS_PROPERTY_MTU2, tmp));

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_property_mtu2(uint8 lchip, uint32 gport, uint16* p_length)
{
    uint32 value = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_length);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property(lchip, gport, SYS_MAC_STATS_PROPERTY_MTU2, &value));
    *p_length = (uint16)value;

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_set_property_pfc(uint8 lchip, uint32 gport, uint8 enable)
{
    uint32 tmp = 0;

    SYS_STATS_INIT_CHECK();
    tmp = enable;

    CTC_MAX_VALUE_CHECK(tmp, 1);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_set_property(lchip, gport, SYS_MAC_STATS_PROPERTY_PFC, tmp));

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_property_pfc(uint8 lchip, uint32 gport, uint8* p_enable)
{
    uint32 value = 0;

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);

    CTC_ERROR_RETURN(_sys_usw_mac_stats_get_property(lchip, gport, SYS_MAC_STATS_PROPERTY_PFC, &value));
    *p_enable = (value > 0 ? 1: 0);

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_rx_stats(uint8 lchip, uint32 gport, ctc_mac_stats_t* p_stats)
{
    uint8  stats_type = 0;
    uint16 lport = 0;
    uint8  tbl_base = 0;
    uint32 tbl_id = 0;
    uint32 tbl_id_base = QuadSgmacStatsRam0_t;
    sys_mac_stats_rx_t* p_mac_stats = NULL;
    sys_mac_stats_rx_t tmp_mac_stats;
    ctc_stats_mac_rec_t* p_stats_mac_rec = NULL;

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&tmp_mac_stats, 0, sizeof(tmp_mac_stats));

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &tbl_base));

    SYS_STATS_DBG_FUNC();
    SYS_STATS_DBG_INFO("tbl_id:%d drv_port:%d, base:%d\n", tbl_id, lport, tbl_base);

    sal_memset(&p_stats->u, 0, sizeof(p_stats->u));
    p_stats_mac_rec = &p_stats->u.stats_detail.stats.rx_stats;

    for (stats_type = SYS_STATS_MAC_RCV_GOOD_UCAST; stats_type < SYS_STATS_MAC_RCV_NUM; stats_type++)
    {
        if (SYS_MAC_STATS_STORE_SW == usw_mac_stats_master[lchip]->store_mode)
        {
            switch (usw_mac_stats_master[lchip]->query_mode)
            {
                case CTC_STATS_QUERY_MODE_IO:
                {
                    /*
                     * SDK maintain mac stats db and use dma callback,
                     * user app do not need db to stroe every io result.
                     */

                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_db(lchip, tbl_id-tbl_id_base, stats_type, gport, (void**)(&p_mac_stats)));
                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_hw_stats(lchip, lport, stats_type, p_mac_stats));
                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_sw_stats(lchip, tbl_id-tbl_id_base, gport, stats_type, &tmp_mac_stats));
                    break;
                }
                case CTC_STATS_QUERY_MODE_POLL:
                {
                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_sw_stats(lchip, tbl_id-tbl_id_base, gport, stats_type, &tmp_mac_stats));
                    break;
                }
                default :
                    break;
            }
        }
        else
        {
            /*
             *  SDK do not maintain mac stats db and no dma callback,
             *  the internal time of user app read stats should less than counter overflow.
             */

            CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_hw_stats(lchip, lport, stats_type, &tmp_mac_stats));
        }
    }

    if (CTC_STATS_MODE_PLUS == p_stats->stats_mode)
    {
        ctc_stats_mac_rec_plus_t* p_rx_stats_plus = &(p_stats->u.stats_plus.stats.rx_stats_plus);
        p_rx_stats_plus->ucast_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_UCAST)];
        p_rx_stats_plus->all_octets = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_63B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_BAD_63B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_64B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_127B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_255B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_511B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_1023B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_1518B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_2047B)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_MTU1)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_MTU1_TO_MTU2)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_BAD_MTU1_TO_MTU2)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_JUMBO)] + \
                                      tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_BAD_JUMBO)];
        p_rx_stats_plus->all_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_64B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_63B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_63B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_127B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_255B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_511B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_1023B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_1518B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_2047B)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_MTU1)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_MTU1_TO_MTU2)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_MTU1_TO_MTU2)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_JUMBO)] + \
                                    tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_JUMBO)];
        p_rx_stats_plus->bcast_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_BCAST)];
        p_rx_stats_plus->crc_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_FCS_ERROR)];
        p_rx_stats_plus->drop_events = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_MAC_OVERRUN)];
        p_rx_stats_plus->error_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_FCS_ERROR)] + \
                                      tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_MAC_OVERRUN)];
        p_rx_stats_plus->fragments_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_63B)] + tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_63B)];
        p_rx_stats_plus->giants_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_JUMBO)];
        p_rx_stats_plus->jumbo_events = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_JUMBO)] + tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_JUMBO)];
        p_rx_stats_plus->mcast_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_MCAST)];
        p_rx_stats_plus->overrun_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_MAC_OVERRUN)];
        p_rx_stats_plus->pause_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_NORMAL_PAUSE)] + tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_PFC_PAUSE)];
        p_rx_stats_plus->runts_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_63B)];
    }
    else
    {
        p_stats_mac_rec->good_ucast_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_UCAST)];
        p_stats_mac_rec->good_ucast_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_UCAST)];
        p_stats_mac_rec->good_mcast_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_MCAST)];
        p_stats_mac_rec->good_mcast_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_MCAST)];
        p_stats_mac_rec->good_bcast_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_BCAST)];
        p_stats_mac_rec->good_bcast_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_BCAST)];
        p_stats_mac_rec->good_normal_pause_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_NORMAL_PAUSE)];
        p_stats_mac_rec->good_normal_pause_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_NORMAL_PAUSE)];
        p_stats_mac_rec->good_pfc_pause_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_PFC_PAUSE)];
        p_stats_mac_rec->good_pfc_pause_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_PFC_PAUSE)];
        p_stats_mac_rec->good_control_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_CONTROL)];
        p_stats_mac_rec->good_control_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_CONTROL)];
        p_stats_mac_rec->fcs_error_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_FCS_ERROR)];
        p_stats_mac_rec->fcs_error_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_FCS_ERROR)];
        p_stats_mac_rec->mac_overrun_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_MAC_OVERRUN)];
        p_stats_mac_rec->mac_overrun_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_MAC_OVERRUN)];

        p_stats_mac_rec->good_63_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_63B)];
        p_stats_mac_rec->good_63_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_63B)];
        p_stats_mac_rec->bad_63_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_63B)];
        p_stats_mac_rec->bad_63_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_BAD_63B)];
        p_stats_mac_rec->good_jumbo_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_JUMBO)];
        p_stats_mac_rec->good_jumbo_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_GOOD_JUMBO)];
        p_stats_mac_rec->bad_jumbo_pkts = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_BAD_JUMBO)];
        p_stats_mac_rec->bad_jumbo_bytes = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_BAD_JUMBO)];
        p_stats_mac_rec->pkts_64 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_64B)];
        p_stats_mac_rec->bytes_64 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_64B)];
        p_stats_mac_rec->pkts_65_to_127 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_127B)];
        p_stats_mac_rec->bytes_65_to_127 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_127B)];
        p_stats_mac_rec->pkts_128_to_255 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_255B)];
        p_stats_mac_rec->bytes_128_to_255 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_255B)];
        p_stats_mac_rec->pkts_256_to_511 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_511B)];
        p_stats_mac_rec->bytes_256_to_511 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_511B)];
        p_stats_mac_rec->pkts_512_to_1023 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_1023B)];
        p_stats_mac_rec->bytes_512_to_1023 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_1023B)];
        p_stats_mac_rec->pkts_1024_to_1518 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_1518B)];
        p_stats_mac_rec->bytes_1024_to_1518 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_1518B)];
        p_stats_mac_rec->pkts_1519_to_2047 = tmp_mac_stats.mac_stats_rx_pkts[RX_TYPE(SYS_STATS_MAC_RCV_2047B)];
        p_stats_mac_rec->bytes_1519_to_2047 = tmp_mac_stats.mac_stats_rx_bytes[RX_TYPE(SYS_STATS_MAC_RCV_2047B)];

        p_stats_mac_rec->good_oversize_pkts = p_stats_mac_rec->good_1519_pkts + p_stats_mac_rec->good_jumbo_pkts;
        p_stats_mac_rec->good_oversize_bytes = p_stats_mac_rec->good_1519_bytes + p_stats_mac_rec->good_jumbo_bytes;
        p_stats_mac_rec->good_undersize_pkts = p_stats_mac_rec->good_63_pkts;
        p_stats_mac_rec->good_undersize_bytes = p_stats_mac_rec->good_63_bytes;
        p_stats_mac_rec->good_pause_pkts = p_stats_mac_rec->good_normal_pause_pkts + p_stats_mac_rec->good_pfc_pause_pkts;
        p_stats_mac_rec->good_pause_bytes = p_stats_mac_rec->good_normal_pause_bytes + p_stats_mac_rec->good_pfc_pause_bytes;
    }

    STATS_UNLOCK();

    return CTC_E_NONE;

}

int32
sys_usw_mac_stats_clear_rx_stats(uint8 lchip, uint32 gport)
{
    uint8  base = 0;
    uint16 lport = 0;
    uint16 index = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 tbl_id_base = QuadSgmacStatsRam0_t;
    QuadSgmacStatsRam0_m qmac_stats_ram;
    uint32 mac_stats_id = 0;
    drv_work_platform_type_t platform_type = MAX_WORK_PLATFORM;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&qmac_stats_ram, 0, sizeof(QuadSgmacStatsRam0_m));

    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &base));

    SYS_STATS_DBG_INFO("drv_port:%d, base:%d\n", lport, base);

    if (CTC_STATS_QUERY_MODE_IO == usw_mac_stats_master[lchip]->query_mode)
    {
        /* for uml, add this code to support clear after read */
        if (SW_SIM_PLATFORM == platform_type)
        {
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        }
        else
        {
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        }
        for (index = SYS_STATS_MAC_RCV_GOOD_UCAST + base; index < SYS_STATS_MAC_RCV_NUM + base; index++)
        {
            CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &qmac_stats_ram));
        }
    }

    CTC_ERROR_RETURN_UNLOCK(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_STATS_ID, (void *)&mac_stats_id));
    sal_memset(&usw_mac_stats_master[lchip]->qmac_stats_table[tbl_id-tbl_id_base].mac_stats_rx[mac_stats_id% 4], 0, sizeof(sys_mac_stats_rx_t));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_get_tx_stats(uint8 lchip, uint32 gport, ctc_mac_stats_t* p_stats)
{
    uint8  stats_type = 0;
    uint16 lport = 0;
    uint8  tbl_base = 0;
    uint32 tbl_id = 0;
    uint32 tbl_id_base = QuadSgmacStatsRam0_t;
    sys_mac_stats_tx_t* p_mac_stats = NULL;
    sys_mac_stats_tx_t tmp_mac_stats;
    ctc_stats_mac_snd_t* p_stats_mac_snd = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&tmp_mac_stats, 0, sizeof(tmp_mac_stats));

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &tbl_base));

    SYS_STATS_DBG_INFO("drv_port:%d, base:%d\n",lport, tbl_base);

    sal_memset(&p_stats->u, 0, sizeof(p_stats->u));
    p_stats_mac_snd = &p_stats->u.stats_detail.stats.tx_stats;

    for (stats_type = SYS_STATS_MAC_SEND_UCAST; stats_type < SYS_STATS_MAC_STATS_TYPE_NUM; stats_type++)
    {
        if (SYS_MAC_STATS_STORE_SW == usw_mac_stats_master[lchip]->store_mode)
        {
            switch (usw_mac_stats_master[lchip]->query_mode)
            {
                case CTC_STATS_QUERY_MODE_IO:
                {
                    /*
                     * SDK maintain mac stats db and use dma callback,
                     * user app do not need db to stroe every io result.
                     */

                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_db(lchip, tbl_id-tbl_id_base, stats_type, gport, (void**)(&p_mac_stats)));
                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_hw_stats(lchip, lport, stats_type, p_mac_stats));
                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_sw_stats(lchip, tbl_id-tbl_id_base, gport, stats_type, &tmp_mac_stats));
                    break;
                }
                case CTC_STATS_QUERY_MODE_POLL:
                {
                    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_sw_stats(lchip, tbl_id-tbl_id_base, gport, stats_type, &tmp_mac_stats));
                    break;
                }
                default :
                    break;
            }
        }
        else
        {
            /*
             *  SDK do not maintain mac stats db and no dma callback,
             *  the internal time of user app read stats should less than counter overflow.
             */

            CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_hw_stats(lchip, lport, stats_type, &tmp_mac_stats));
        }
    }

    if (CTC_STATS_MODE_PLUS == p_stats->stats_mode)
    {
        ctc_stats_mac_snd_plus_t* p_tx_stats_plus = &(p_stats->u.stats_plus.stats.tx_stats_plus);
        p_tx_stats_plus->all_octets = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_63B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_64B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_127B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_255B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_511B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_1023B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_1518B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_2047B)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_MTU1)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_MTU2)] + \
                                      tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_JUMBO)];
        p_tx_stats_plus->all_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_63B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_64B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_127B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_255B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_511B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_1023B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_1518B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_2047B)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MTU1)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MTU2)] + \
                                    tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_JUMBO)];
        p_tx_stats_plus->bcast_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_BCAST)];
        p_tx_stats_plus->error_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_FCS_ERROR)];
        p_tx_stats_plus->jumbo_events = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_JUMBO)];
        p_tx_stats_plus->mcast_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MCAST)];
        p_tx_stats_plus->ucast_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_UCAST)];
        p_tx_stats_plus->underruns_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MAC_UNDERRUN)];
    }
    else
    {
        p_stats_mac_snd->good_ucast_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_UCAST)];
        p_stats_mac_snd->good_ucast_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_UCAST)];
        p_stats_mac_snd->good_mcast_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MCAST)];
        p_stats_mac_snd->good_mcast_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_MCAST)];
        p_stats_mac_snd->good_bcast_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_BCAST)];
        p_stats_mac_snd->good_bcast_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_BCAST)];
        p_stats_mac_snd->good_pause_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_PAUSE)];
        p_stats_mac_snd->good_pause_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_PAUSE)];
        p_stats_mac_snd->good_control_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_CONTROL)];
        p_stats_mac_snd->good_control_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_CONTROL)];
        p_stats_mac_snd->pkts_63 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_63B)];
        p_stats_mac_snd->bytes_63 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_63B)];
        p_stats_mac_snd->pkts_64 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_64B)];
        p_stats_mac_snd->bytes_64 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_64B)];
        p_stats_mac_snd->pkts_65_to_127 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_127B)];
        p_stats_mac_snd->bytes_65_to_127 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_127B)];
        p_stats_mac_snd->pkts_128_to_255 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_255B)];
        p_stats_mac_snd->bytes_128_to_255 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_255B)];
        p_stats_mac_snd->pkts_256_to_511 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_511B)];
        p_stats_mac_snd->bytes_256_to_511 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_511B)];
        p_stats_mac_snd->pkts_512_to_1023 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_1023B)];
        p_stats_mac_snd->bytes_512_to_1023 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_1023B)];
        p_stats_mac_snd->pkts_1024_to_1518 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_1518B)];
        p_stats_mac_snd->bytes_1024_to_1518 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_1518B)];
        p_stats_mac_snd->pkts_1519_to_2047 = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_2047B)];
        p_stats_mac_snd->bytes_1519_to_2047 = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_2047B)];

        p_stats_mac_snd->jumbo_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_JUMBO)] + tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_2047B)] + \
                                        tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MTU1)] + tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MTU2)];
        p_stats_mac_snd->jumbo_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_JUMBO)] + tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_2047B)] + \
                                        tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_MTU1)] + tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_MTU2)];
        p_stats_mac_snd->mac_underrun_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_MAC_UNDERRUN)];
        p_stats_mac_snd->mac_underrun_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_MAC_UNDERRUN)];
        p_stats_mac_snd->fcs_error_pkts = tmp_mac_stats.mac_stats_tx_pkts[TX_TYPE(SYS_STATS_MAC_SEND_FCS_ERROR)];
        p_stats_mac_snd->fcs_error_bytes = tmp_mac_stats.mac_stats_tx_bytes[TX_TYPE(SYS_STATS_MAC_SEND_FCS_ERROR)];

    }

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_clear_tx_stats(uint8 lchip, uint32 gport)
{
    uint8  base = 0;
    uint16 lport = 0;
    uint16 index = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 tbl_id_base = QuadSgmacStatsRam0_t;
    uint32 mac_stats_id = 0;
    QuadSgmacStatsRam0_m qmac_stats_ram;
    drv_work_platform_type_t platform_type = MAX_WORK_PLATFORM;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&qmac_stats_ram, 0, sizeof(QuadSgmacStatsRam0_m));

    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_mac_stats_get_stats_tbl_info(lchip, lport, &tbl_id, &base));

    SYS_STATS_DBG_INFO("drv_port:%d, base:%d\n", lport, base);

    if (CTC_STATS_QUERY_MODE_IO == usw_mac_stats_master[lchip]->query_mode)
    {
        /* for uml, add this code to support clear after read */
        if (SW_SIM_PLATFORM == platform_type)
        {
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        }
        else
        {
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        }
        for (index = SYS_STATS_MAC_SEND_UCAST + base; index < SYS_STATS_MAC_STATS_TYPE_NUM + base; index++)
        {
            CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, index, cmd, &qmac_stats_ram));
        }
    }
    CTC_ERROR_RETURN_UNLOCK(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_STATS_ID, (void *)&mac_stats_id));
    sal_memset(&usw_mac_stats_master[lchip]->qmac_stats_table[tbl_id-tbl_id_base].mac_stats_tx[mac_stats_id% 4], 0, sizeof(sys_mac_stats_tx_t));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

void
sys_usw_mac_stats_get_port_rate(uint8 lchip, uint32 gport, uint64* p_rate)
{

    int32       ret = CTC_E_NONE;
    uint32      enable = 0;
    uint64      pre_throughput[2] = {0};
    uint64      cur_throughput[2] = {0};
    uint64      intv = 0;
    uint16      lport = 0;
    sal_systime_t  pre_timestamp;
    sal_systime_t  cur_timestamp;

    sal_memset(&pre_timestamp,  0, sizeof(sal_systime_t));
    sal_memset(&cur_timestamp,  0, sizeof(sal_systime_t));

    STATS_LOCK();

    p_rate[CTC_STATS_MAC_STATS_RX] = 0;
    p_rate[CTC_STATS_MAC_STATS_TX] = 0;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    ret = _sys_usw_mac_stats_get_throughput(lchip, gport, pre_throughput, &pre_timestamp);

    ret = ret ? ret : _sys_usw_mac_stats_sync_throughput(lchip, lport, NULL);

    ret = ret ? ret : _sys_usw_mac_stats_get_throughput(lchip, gport, cur_throughput, &cur_timestamp);

    sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_EN, (void *)&enable);
    intv = (((uint64)cur_timestamp.tv_sec * 1000000 + cur_timestamp.tv_usec) - ((uint64)pre_timestamp.tv_sec * 1000000 + pre_timestamp.tv_usec)) / 1000;

    if ((CTC_E_NONE != ret) || (0 == enable) || (0 == intv))
    {
        SYS_STATS_DBG_INFO("%s %d, ret:%d, enable:%u, intv:%"PRIu64" ms.\n", __FUNCTION__, __LINE__, ret, enable, intv);
        STATS_UNLOCK();
        return;
    }

    p_rate[CTC_STATS_MAC_STATS_RX] = ((cur_throughput[CTC_STATS_MAC_STATS_RX] - pre_throughput[CTC_STATS_MAC_STATS_RX])
                                      / intv) * 1000;
    p_rate[CTC_STATS_MAC_STATS_TX] = ((cur_throughput[CTC_STATS_MAC_STATS_TX] - pre_throughput[CTC_STATS_MAC_STATS_TX])
                                      / intv) * 1000;
    SYS_STATS_DBG_INFO("%s gport:0x%04X, time-interval:%"PRIu64" ms.\n", __FUNCTION__, gport, intv);

    STATS_UNLOCK();
    return;
}

int32
sys_usw_mac_stats_show_status(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    SYS_STATS_INIT_CHECK();

    SYS_STATS_DBG_DUMP("\n");
    SYS_STATS_DBG_DUMP("--------------------------Mac stats status-------------------------\n");

    STATS_LOCK();
    SYS_STATS_DBG_DUMP("%-30s:%10s \n", "Mac stats mode", usw_mac_stats_master[lchip]->stats_mode?"sdk config":"user config");
    SYS_STATS_DBG_DUMP("%-30s:%10s \n", "Query mode", usw_mac_stats_master[lchip]->query_mode?"poll":"io");
    SYS_STATS_DBG_DUMP("%-30s:%10s \n", "Store mode", usw_mac_stats_master[lchip]->store_mode?"software":"hardware");
    SYS_STATS_DBG_DUMP("%-30s:%10u \n", "Poll timer", usw_mac_stats_master[lchip]->mac_timer);
    STATS_UNLOCK();

    SYS_STATS_DBG_DUMP("------------------------------------------------------------------\n");
    SYS_STATS_DBG_DUMP("\n");

    return CTC_E_NONE;
}

int32
sys_usw_mac_stats_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;

    SYS_STATS_INIT_CHECK();
    STATS_LOCK();
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# MAC STATS");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%10s \n", "Mac stats mode", usw_mac_stats_master[lchip]->stats_mode?"sdk config":"user config");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%10s \n", "Query mode", usw_mac_stats_master[lchip]->query_mode?"poll":"io");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%10s \n", "Store mode", usw_mac_stats_master[lchip]->store_mode?"software":"hardware");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%10u \n", "Poll timer", usw_mac_stats_master[lchip]->mac_timer);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    STATS_UNLOCK();

    return ret;
}

#define ___________MAC_STATS_INIT________________________

int32
sys_usw_mac_stats_init(uint8 lchip, ctc_stats_global_cfg_t* stats_global_cfg)
{
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 dma_timer = 0;
    tbls_id_t tblid = MaxTblId_t;
    tbls_id_t tblid_end = MaxTblId_t;
    QuadSgmacStatsCfg0_m quad_sgmacStatsCfg;
    uint8 work_status = 0;

    LCHIP_CHECK(lchip);
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    /*init global variable*/
    if (NULL != usw_mac_stats_master[lchip])
    {
        return CTC_E_NONE;
    }

    usw_mac_stats_master[lchip] = (sys_mac_stats_master_t*)mem_malloc(MEM_STATS_MODULE, sizeof(sys_mac_stats_master_t));
    if (NULL == usw_mac_stats_master[lchip])
    {
        SYS_STATS_DBG_ERROR(" No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error_roll;
    }

    sal_memset(usw_mac_stats_master[lchip], 0, sizeof(sys_mac_stats_master_t));
    sal_mutex_create(&(usw_mac_stats_master[lchip]->p_stats_mutex));
    usw_mac_stats_master[lchip]->store_mode = SYS_MAC_STATS_STORE_SW;   /* default store in software db */
    usw_mac_stats_master[lchip]->stats_mode = stats_global_cfg->stats_mode;
    usw_mac_stats_master[lchip]->query_mode = stats_global_cfg->query_mode;

    sal_memset(&quad_sgmacStatsCfg, 0, sizeof(QuadSgmacStatsCfg0_m));
    SetQuadSgmacStatsCfg0(V, clearOnRead_f,          &quad_sgmacStatsCfg, 1);
    SetQuadSgmacStatsCfg0(V, ge64BPktHiPri_f,        &quad_sgmacStatsCfg, 1);
    SetQuadSgmacStatsCfg0(V, incrHold_f,             &quad_sgmacStatsCfg, 0);
    SetQuadSgmacStatsCfg0(V, incrSaturate_f,         &quad_sgmacStatsCfg, 1);
    SetQuadSgmacStatsCfg0(V, packetLenMtu1_f,        &quad_sgmacStatsCfg, 0x5EE);
    SetQuadSgmacStatsCfg0(V, packetLenMtu2_f,        &quad_sgmacStatsCfg, 0x600);
    SetQuadSgmacStatsCfg0(V, statsOverWriteEnable_f, &quad_sgmacStatsCfg, 0);

    tblid_end = DRV_IS_DUET2(lchip)?QuadSgmacStatsCfg15_t:QuadSgmacStatsCfg17_t;
    for (tblid = QuadSgmacStatsCfg0_t; tblid <= tblid_end; tblid++)
    {
        cmd = DRV_IOW(tblid, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &quad_sgmacStatsCfg);
    }

    if (SYS_MAC_STATS_STORE_SW == usw_mac_stats_master[lchip]->store_mode)
    {
        CTC_ERROR_GOTO(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_PORT_STATS, sys_usw_mac_stats_sync_dma_stats), ret, error_roll);
        dma_timer = ((CTC_STATS_QUERY_MODE_POLL == usw_mac_stats_master[lchip]->query_mode) ? stats_global_cfg->mac_timer: SYS_STATS_DMA_IO_TIMER);

        CTC_VALUE_RANGE_CHECK(dma_timer, 30, 600);

        CTC_ERROR_GOTO(sys_usw_dma_set_mac_stats_timer(lchip, dma_timer*1000), ret, error_roll);
        usw_mac_stats_master[lchip]->mac_timer = stats_global_cfg->mac_timer;
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_dma_set_mac_stats_timer(lchip, 0), ret, error_roll);
        usw_mac_stats_master[lchip]->mac_timer = 0;
    }

    /* wormboot data restore */
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(_sys_usw_mac_stats_wb_restore(lchip), ret, error_roll);
    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

error_roll:

    if (NULL != usw_mac_stats_master[lchip])
    {
        if (usw_mac_stats_master[lchip]->p_stats_mutex)
        {
            sal_mutex_destroy(usw_mac_stats_master[lchip]->p_stats_mutex);
        }

        mem_free(usw_mac_stats_master[lchip]);
        usw_mac_stats_master[lchip] = NULL;
    }

    return ret;
}


int32
sys_usw_mac_stats_deinit(uint8 lchip)
{
    if (NULL == usw_mac_stats_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (usw_mac_stats_master[lchip]->p_stats_mutex)
    {
        sal_mutex_destroy(usw_mac_stats_master[lchip]->p_stats_mutex);
    }

    mem_free(usw_mac_stats_master[lchip]);
    usw_mac_stats_master[lchip] = NULL;

    return CTC_E_NONE;
}

