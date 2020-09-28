/**
 @file sys_usw_chip.c

 @date 2009-10-19

 @version v2.0

 The file define APIs of chip of sys layer
*/
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctc_error.h"
#include "ctc_register.h"
#include "ctc_warmboot.h"
#include "ctc_hash.h"
#include "sys_usw_register.h"
#include "sys_usw_chip.h"
#include "sys_usw_mchip.h"
#include "sys_usw_dmps.h"
#include "sys_usw_ftm.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_dma.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"


/****************************************************************************
 *
 * Global and static
 *
 *****************************************************************************/
sys_chip_master_t* p_usw_chip_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
extern uint8 g_lchip_num;
extern int32 dal_get_chip_dev_id(uint8 lchip, uint32* dev_id);

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
STATIC int32 _sys_usw_chip_init_ser(uint8 lchip, ctc_chip_global_cfg_t* p_cfg);
/**
 @brief The function is to initialize the chip module and set the local chip number of the linecard
*/

int32
sys_usw_chip_wb_sync(uint8 lchip)
{
    int32 ret  = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_chip_master_t* p_chip_wb_master = NULL;


    /* sync up chip matser */
    wb_data.buffer = mem_malloc(MEM_SYSTEM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    p_chip_wb_master = (sys_wb_chip_master_t*)wb_data.buffer;

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_chip_master_t, CTC_FEATURE_CHIP, SYS_WB_APPID_CHIP_SUBID_MASTER);


    p_chip_wb_master->lchip = lchip;
    p_chip_wb_master->version = SYS_WB_VERSION_CHIP;
    dal_get_chip_dev_id(lchip, &(p_chip_wb_master->dev_id));
    wb_data.valid_cnt = 1;

    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);


done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}
STATIC int32
_sys_usw_chip_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    uint8 i = 0;
    uint8 j = 0;

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Chip");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","g_chip_id",p_usw_chip_master[lchip]->g_chip_id);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","cut_through_en",p_usw_chip_master[lchip]->cut_through_en);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","tpid",p_usw_chip_master[lchip]->tpid);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","cpu_eth_en",p_usw_chip_master[lchip]->cpu_eth_en);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","cpu_channel",p_usw_chip_master[lchip]->cpu_channel);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","vlan_id",p_usw_chip_master[lchip]->vlan_id);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%"PRIu64"\n","alpm_affinity_mask",p_usw_chip_master[lchip]->alpm_affinity_mask);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%"PRIu64"\n","normal_affinity_mask",p_usw_chip_master[lchip]->normal_affinity_mask);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u%u%u%u%u%u\n","cpu_mac_sa",p_usw_chip_master[lchip]->cpu_mac_sa[0],p_usw_chip_master[lchip]->cpu_mac_sa[1],p_usw_chip_master[lchip]->cpu_mac_sa[2],p_usw_chip_master[lchip]->cpu_mac_sa[3],
                                                   p_usw_chip_master[lchip]->cpu_mac_sa[4],p_usw_chip_master[lchip]->cpu_mac_sa[5]);
    SYS_DUMP_DB_LOG(p_f, "%-30s:","cut_through_speed");
    for(i = 0; i < CTC_PORT_SPEED_MAX; i ++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u,%u]", p_usw_chip_master[lchip]->cut_through_speed[i].port_speed,p_usw_chip_master[lchip]->cut_through_speed[i].enable);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "Cpu mac da:");
    SYS_DUMP_DB_LOG(p_f, "%-15s%-10s\n", "Index", "Cpu mac da");
    for(i = 0; i < SYS_USW_MAX_CPU_MACDA_NUM; i++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-15u", i);
        for (j = 0; j < 5; j++)
        {
            SYS_DUMP_DB_LOG(p_f, "%02x:", p_usw_chip_master[lchip]->cpu_mac_da[i][j]);
        }
        SYS_DUMP_DB_LOG(p_f, "%02x\n", p_usw_chip_master[lchip]->cpu_mac_da[i][5]);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    return CTC_E_NONE;
}
int32
sys_usw_chip_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 dev_id = 0;
    ctc_wb_query_t wb_query;
    sys_wb_chip_master_t chip_wb_master;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_SYSTEM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*restore master*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(&chip_wb_master, 0, sizeof(sys_wb_chip_master_t));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_chip_master_t, CTC_FEATURE_CHIP, SYS_WB_APPID_CHIP_SUBID_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query chip master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }


    sal_memcpy(&chip_wb_master, wb_query.buffer, wb_query.key_len + wb_query.data_len);

    dal_get_chip_dev_id(lchip, &dev_id);
    if(chip_wb_master.dev_id != dev_id)
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_CHIP, chip_wb_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

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
sys_usw_chip_init(uint8 lchip, uint8 lchip_num)
{
    int32 ret;

    if ((lchip >= CTC_MAX_LOCAL_CHIP_NUM)
        || (lchip_num > SYS_USW_MAX_LOCAL_CHIP_NUM)
        || (lchip_num > CTC_MAX_LOCAL_CHIP_NUM))
    {
        return CTC_E_INVALID_CHIP_NUM;
    }

    /* check init */
    if (NULL != p_usw_chip_master[lchip])
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_chip_set_active(lchip, TRUE));

    p_usw_chip_master[lchip] = (sys_chip_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_chip_master_t));

    if (NULL == p_usw_chip_master[lchip])
    {
        SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_chip_master[lchip], 0, sizeof(sys_chip_master_t));

    /* create mutex for chip module */
    ret = sal_mutex_create(&(p_usw_chip_master[lchip]->p_chip_mutex));
    if (ret || (!p_usw_chip_master[lchip]->p_chip_mutex))
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    g_lchip_num = lchip_num;

    /*
        drv-io ,which inits in datapath, depends drv-init
    */
    ret = drv_init(lchip, 0);
    if (ret < 0)
    {
        ret = CTC_E_INIT_FAIL;
        goto error0;
    }

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_chip_wb_restore(lchip), ret, error0);
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }


    CTC_ERROR_GOTO(sys_usw_mchip_init(lchip), ret, error0);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_CHIP, _sys_usw_chip_dump_db), ret, error0);

    return ret;
error0:
    ctc_sal_mutex_destroy(p_usw_chip_master[lchip]->p_chip_mutex);
error1:
    mem_free(p_usw_chip_master[lchip]);

    return ret;
}

int32
sys_usw_chip_deinit(uint8 lchip)
{
    CTC_ERROR_RETURN(sys_usw_chip_set_active(lchip, FALSE));

    /* during warmboot, the failover must close */
    sys_usw_global_failover_en(lchip, FALSE);

    sys_usw_interrupt_set_group_en(lchip, FALSE);

    if (!p_usw_chip_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_task_sleep(1);

    sal_mutex_destroy(p_usw_chip_master[lchip]->p_chip_mutex);
    mem_free(p_usw_chip_master[lchip]);
    return 0;
}

/**
 @brief The function is to get the local chip num
*/
uint8
sys_usw_get_local_chip_num()
{
    return g_lchip_num;
}

/**
 @brief The function is to set chip's global chip id
*/
int32
sys_usw_set_gchip_id(uint8 lchip, uint8 gchip_id)
{
    if (NULL == p_usw_chip_master[lchip])
    {
        SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    if (lchip >= g_lchip_num)
    {
        SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");
			return CTC_E_INVALID_CHIP_ID;

    }

    SYS_GLOBAL_CHIPID_CHECK(gchip_id);

    CTC_ERROR_RETURN(drv_set_gchip_id(lchip, gchip_id));

    return CTC_E_NONE;
}

/**
 @brief The function is to get chip's global chip id
*/
int32
sys_usw_get_gchip_id(uint8 lchip, uint8* gchip_id)
{
    if (NULL == p_usw_chip_master[lchip])
    {
        SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    CTC_PTR_VALID_CHECK(gchip_id);

    CTC_ERROR_RETURN(drv_get_gchip_id(lchip, gchip_id));

    return CTC_E_NONE;
}

/**
 @brief The function is to judge whether the chip is local
*/
bool
sys_usw_chip_is_local(uint8 lchip, uint8 gchip_id)
{
    bool ret        = FALSE;
    uint8 temp_gchip = 0;

    drv_get_gchip_id(lchip, &temp_gchip);

    if ((NULL != p_usw_chip_master[lchip])
        && (temp_gchip == gchip_id))
    {
        ret     = TRUE;
    }

    return ret;
}

/* function that call sys_usw_get_local_chip_id needs to check return */
int32
sys_usw_get_local_chip_id(uint8 gchip_id, uint8* lchip_id)
{
    uint8 i = 0;
    uint8 is_find = 0;
    uint8 temp_gchip = 0;

    CTC_PTR_VALID_CHECK(lchip_id);

    if (CTC_LINKAGG_CHIPID == gchip_id)
    {
        return CTC_E_NONE;
    }

    /* different chips have same lchip num */
    for (i = 0; i < g_lchip_num; i++)
    {
        temp_gchip = 0;
        if(drv_get_gchip_id(i, &temp_gchip))
        {
            return CTC_E_INVALID_PARAM;
        }

        if ((NULL != p_usw_chip_master[i])
            && (temp_gchip == gchip_id))
        {
            is_find = 1;
            break;
        }
    }

    if (is_find)
    {
        *lchip_id = i;
        return CTC_E_NONE;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
}

int32
sys_usw_chip_set_eth_cpu_cfg(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 chan_id = 0;
    uint8 lchan_id = 0;

    uint8 slice_id = 0;
    uint32 tocpu_bmp[4] = {0};
    ds_t ds;
    hw_mac_addr_t hw_mac_da;
    hw_mac_addr_t hw_mac_sa;
    IpeHeaderAdjustCtl_m ipe_header_adjust_ctl;
    EpeNextHopCtl_m epe_nhop_ctl;
    uint32 field_value = 0;

    chan_id =  p_usw_chip_master[lchip]->cpu_channel;
    if (chan_id >= 128)
    {
        return CTC_E_INVALID_PARAM;
    }

    slice_id = SYS_MAP_CHANID_TO_SLICE(chan_id);
    lchan_id = chan_id&0x3F;

    CTC_BIT_SET(tocpu_bmp[lchan_id >> 5], (lchan_id&0x1F));

    sal_memset(&ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeHeaderAdjustCpuChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));
    SetIpeHeaderAdjustCpuChanCtl(A, fromCpuEn_f, &ds, tocpu_bmp);
    cmd = DRV_IOW(IpeHeaderAdjustCpuChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));

    sal_memset(&ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeFwdCpuChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));
    SetIpeFwdCpuChanCtl(A, fromCpuEn_f, &ds, tocpu_bmp);
    cmd = DRV_IOW(IpeFwdCpuChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));

    sal_memset(&ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));
    SetEpeHdrAdjustChanCtl(A, toCpuEn_f, &ds, tocpu_bmp);
    cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));

    sal_memset(&ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeHeaderEditCpuChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));
    SetEpeHeaderEditCpuChanCtl(A, toCpuEn_f, &ds, tocpu_bmp);
    cmd = DRV_IOW(EpeHeaderEditCpuChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, slice_id, cmd, &ds));

    sal_memset(tocpu_bmp, 0, sizeof(tocpu_bmp));
    CTC_BIT_SET(tocpu_bmp[chan_id >> 5], (chan_id&0x1F));
    sal_memset(&ds, 0, sizeof(ds));
    cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));
    SetBufferStoreCtl(A, fromCpuEn_f, &ds, tocpu_bmp);
    cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

    SYS_USW_SET_HW_MAC(hw_mac_da, p_usw_chip_master[lchip]->cpu_mac_da[0]);
    SYS_USW_SET_HW_MAC(hw_mac_sa, p_usw_chip_master[lchip]->cpu_mac_sa);

    sal_memset(&ds, 0, sizeof(ds));
    cmd = DRV_IOR(DsPacketHeaderEditTunnel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ds));
    SetDsPacketHeaderEditTunnel(A, macSa_f, &ds, hw_mac_sa);
    SetDsPacketHeaderEditTunnel(V, packetHeaderL2En_f, &ds, 1);
    SetDsPacketHeaderEditTunnel(V, vlanIdValid_f, &ds, 1);
    SetDsPacketHeaderEditTunnel(V, vlanId_f, &ds, p_usw_chip_master[lchip]->vlan_id);
    SetDsPacketHeaderEditTunnel(A, macDa_f, &ds, hw_mac_da);
    cmd = DRV_IOW(DsPacketHeaderEditTunnel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ds));

    field_value = p_usw_chip_master[lchip]->tpid;
    cmd = DRV_IOW(EpePktHdrCtl_t, EpePktHdrCtl_tpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    field_value = 0x55aa;
    cmd = DRV_IOW(EpePktHdrCtl_t, EpePktHdrCtl_headerEtherType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

#if 0
    field_value = 47;
    cmd = DRV_IOW(BufRetrvChanIdCfg_t, BufRetrvChanIdCfg_cfgCpuChanId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
#endif


    field_value = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2);
    cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f );
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_value));

    sal_memset(&ipe_header_adjust_ctl, 0, sizeof(ipe_header_adjust_ctl));
    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_header_adjust_ctl));
    SetIpeHeaderAdjustCtl(V, headerMacDaCheckEn_f, &ipe_header_adjust_ctl,0);
    SetIpeHeaderAdjustCtl(V, headerTpid_f, &ipe_header_adjust_ctl, p_usw_chip_master[lchip]->tpid);
    SetIpeHeaderAdjustCtl(V, headerEtherTypeCheckDisable_f, &ipe_header_adjust_ctl, 1);
    SetIpeHeaderAdjustCtl(V, headerEtherType_f, &ipe_header_adjust_ctl,0x55aa);
    SetIpeHeaderAdjustCtl(V, packetHeaderBypassAll_f, &ipe_header_adjust_ctl,1);
    SetIpeHeaderAdjustCtl(V, fromCpuBypassAll_f , &ipe_header_adjust_ctl,1);
    cmd = DRV_IOW(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_header_adjust_ctl));

    sal_memset(&epe_nhop_ctl, 0, sizeof(epe_nhop_ctl));
    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nhop_ctl));
    SetEpeNextHopCtl(V, toCpuPktDoNotBypassNhp_f, &epe_nhop_ctl, 1);
    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nhop_ctl));

    return CTC_E_NONE;
}

/**
 @brief The function is to set chip's global cfg
*/
int32
sys_usw_set_chip_global_cfg(uint8 lchip, ctc_chip_global_cfg_t* chip_cfg)
{
    BufferStoreCtl_m buf_stro_ctl;
    IpeFwdCtl_m      ipe_fwd_ctl;
    uint32 cmd = 0;
    uint8 gchip = 0;
    uint16 lport = 0;
    uint8 loop = 0, loop2 = 0;

    SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(chip_cfg);

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(chip_cfg->cpu_port);
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(chip_cfg->cpu_port);
    if (chip_cfg->cpu_port_en && !sys_usw_chip_is_local(lchip, gchip))
    {
         return CTC_E_INVALID_PORT;
    }
    p_usw_chip_master[lchip]->cut_through_en       = chip_cfg->cut_through_en;


#ifdef EMULATION_ENV

#else
{
    EpePktProcCtl_m EpePktProcCtl;
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &EpePktProcCtl));

    if (chip_cfg->cut_through_en == 0)
    {
        uint32 field_val = 0;
        field_val = 0;
        cmd = DRV_IOW(QMgrEnqReserved_t, QMgrEnqReserved_reserved_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        SetEpePktProcCtl(V, mtuCheckSelectBits_f, &EpePktProcCtl, 0x06);
    }
    else
    {
        uint32 field_val = 0;
        field_val = 0xf;
        cmd = DRV_IOW(QMgrEnqReserved_t, QMgrEnqReserved_reserved_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        SetEpePktProcCtl(V, mtuCheckSelectBits_f, &EpePktProcCtl, 0x03);
    }
    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &EpePktProcCtl));
}
#endif

   /*support cut through speed*/
    p_usw_chip_master[lchip]->cut_through_speed[0].port_speed = CTC_PORT_SPEED_1G;
    p_usw_chip_master[lchip]->cut_through_speed[1].port_speed = CTC_PORT_SPEED_2G5;
    p_usw_chip_master[lchip]->cut_through_speed[2].port_speed = CTC_PORT_SPEED_5G;
    p_usw_chip_master[lchip]->cut_through_speed[3].port_speed = CTC_PORT_SPEED_10G;
    p_usw_chip_master[lchip]->cut_through_speed[4].port_speed = CTC_PORT_SPEED_20G;
    p_usw_chip_master[lchip]->cut_through_speed[5].port_speed = CTC_PORT_SPEED_25G;
    p_usw_chip_master[lchip]->cut_through_speed[6].port_speed = CTC_PORT_SPEED_40G;
    p_usw_chip_master[lchip]->cut_through_speed[7].port_speed = CTC_PORT_SPEED_50G;
    p_usw_chip_master[lchip]->cut_through_speed[8].port_speed = CTC_PORT_SPEED_100G;

    /**<
      [GG] 1:10/40/100G mixed;2:1/10/100G mixed 3:1/10/40G mixed, else none */

    if(chip_cfg->cut_through_speed == 1)
    {
       p_usw_chip_master[lchip]->cut_through_speed[3].enable = 1;
       p_usw_chip_master[lchip]->cut_through_speed[6].enable = 1;
       p_usw_chip_master[lchip]->cut_through_speed[8].enable = 1;
    }
    else if(chip_cfg->cut_through_speed == 2)
    {
       p_usw_chip_master[lchip]->cut_through_speed[0].enable = 1;
       p_usw_chip_master[lchip]->cut_through_speed[3].enable = 1;
       p_usw_chip_master[lchip]->cut_through_speed[8].enable = 1;
    }
    else if(chip_cfg->cut_through_speed == 3)
    {
        p_usw_chip_master[lchip]->cut_through_speed[0].enable = 1;
        p_usw_chip_master[lchip]->cut_through_speed[3].enable = 1;
        p_usw_chip_master[lchip]->cut_through_speed[6].enable = 1;
    }

    if(chip_cfg->cut_through_speed_bitmap !=0)
     {
        for(loop =0 ; loop < CTC_PORT_SPEED_MAX;loop++)
        {
          if(!CTC_IS_BIT_SET(chip_cfg->cut_through_speed_bitmap,loop)
            || (loop == CTC_PORT_SPEED_10M) || (loop == CTC_PORT_SPEED_100M))
          {
            continue;
          }
          for(loop2 =0 ; loop2 < CTC_PORT_SPEED_MAX;loop2++)
          {
            if(p_usw_chip_master[lchip]->cut_through_speed[loop2].port_speed == loop)
            {
              p_usw_chip_master[lchip]->cut_through_speed[loop2].enable = 1;
              break;
            }
          }
        }
     }

    p_usw_chip_master[lchip]->cpu_eth_en = chip_cfg->cpu_port_en;
    p_usw_chip_master[lchip]->cpu_channel = SYS_GET_CHANNEL_ID(lchip, chip_cfg->cpu_port);
    if ((0xFF ==  p_usw_chip_master[lchip]->cpu_channel) && (chip_cfg->cpu_port_en))
    {
        return CTC_E_INVALID_PARAM;
    }
    sal_memset(&ipe_fwd_ctl, 0, sizeof(ipe_fwd_ctl));
    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
    SetIpeFwdCtl(V, cutThroughEn_f, &ipe_fwd_ctl, chip_cfg->cut_through_en);
    SetIpeFwdCtl(V, cutThroughDontCareException_f, &ipe_fwd_ctl, chip_cfg->cut_through_en);
    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));


    p_usw_chip_master[lchip]->tpid = chip_cfg->tpid;
    p_usw_chip_master[lchip]->vlan_id = chip_cfg->vlanid;
    sal_memcpy(p_usw_chip_master[lchip]->cpu_mac_sa, chip_cfg->cpu_mac_sa, sizeof(mac_addr_t));
    sal_memcpy(p_usw_chip_master[lchip]->cpu_mac_da[0], chip_cfg->cpu_mac_da[0], sizeof(mac_addr_t));
    cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_stro_ctl));
    SetBufferStoreCtl(V, cpuRxPortModeEn_f, &buf_stro_ctl,chip_cfg->cpu_port_en);
    SetBufferStoreCtl(V, g0_0_cpuPort_f, &buf_stro_ctl, lport);
    SetBufferStoreCtl(V, g0_1_cpuPort_f, &buf_stro_ctl, lport);
    SetBufferStoreCtl(V, g0_2_cpuPort_f, &buf_stro_ctl, lport);
    SetBufferStoreCtl(V, g0_3_cpuPort_f, &buf_stro_ctl, lport);
    cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_stro_ctl));

    p_usw_chip_master[lchip]->alpm_affinity_mask = chip_cfg->alpm_affinity_mask;
  p_usw_chip_master[lchip]->normal_affinity_mask = chip_cfg->normal_affinity_mask;
  p_usw_chip_master[lchip]->rchain_en = chip_cfg->rchain_en;
  p_usw_chip_master[lchip]->rchain_gchip= chip_cfg->rchain_gchip;


    CTC_ERROR_RETURN(_sys_usw_chip_init_ser(lchip ,chip_cfg));

    return CTC_E_NONE;
}

uint64
sys_usw_chip_get_affinity(uint8 lchip, uint8 is_alpm)
{
    SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    return is_alpm ? p_usw_chip_master[lchip]->alpm_affinity_mask : p_usw_chip_master[lchip]->normal_affinity_mask;
}

int32
sys_usw_get_chip_clock(uint8 lchip, uint16* freq)
{
    SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    *freq = sys_usw_get_core_freq(lchip, 0);

    return CTC_E_NONE;
}

int32
sys_usw_get_chip_cpumac(uint8 lchip, uint8* mac_sa, uint8* mac_da)
{
    sal_memcpy(mac_sa, p_usw_chip_master[lchip]->cpu_mac_sa, sizeof(mac_addr_t));
    sal_memcpy(mac_da, p_usw_chip_master[lchip]->cpu_mac_da[0], sizeof(mac_addr_t));

    return CTC_E_NONE;
}
bool
sys_usw_chip_is_eth_cpu_port(uint8 lchip, uint16 lport)
{
    uint16 sys_port     = 0;

    if (p_usw_chip_master[lchip]->cpu_eth_en)
    {
        sys_port = SYS_GET_LPORT_ID_WITH_CHAN(lchip, p_usw_chip_master[lchip]->cpu_channel);
        if (lport == SYS_MAP_SYS_LPORT_TO_DRV_LPORT(sys_port))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

int32
sys_usw_get_chip_cpu_eth_en(uint8 lchip, uint8 *enable, uint8* cpu_eth_chan)
{
    *enable = p_usw_chip_master[lchip]->cpu_eth_en;
    *cpu_eth_chan = p_usw_chip_master[lchip]->cpu_channel;

    return CTC_E_NONE;
}

uint8
sys_usw_chip_get_rchain_en()
{
    if (!p_usw_chip_master[0])
    {
        return 0;
    }
    return p_usw_chip_master[0]->rchain_en;
}

uint8
sys_usw_chip_get_rchain_gchip()
{
    if (!p_usw_chip_master[0])
    {
        return 0;
    }
    return p_usw_chip_master[0]->rchain_gchip;
}
#define __CUT_THROUGH__
uint8
sys_usw_chip_get_cut_through_en(uint8 lchip)
{
    return p_usw_chip_master[lchip]->cut_through_en ?1:0;
}

uint8
sys_usw_chip_get_cut_through_speed(uint8 lchip, uint32 gport)
{
    uint8  cut_through_speed = 0;
    uint8  found = 0;
    uint8  loop = 0;
    uint32 port_type = 0;
    uint32 speed_mode = 0;

    if (!p_usw_chip_master[lchip]->cut_through_en)
    {
        return 0;
    }
    sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type);
    sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_SPEED_MODE, (void *)&speed_mode);
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        return 0;
    }

    for (loop = 0; loop < CTC_PORT_SPEED_MAX; loop++)
    {

        if ( p_usw_chip_master[lchip]->cut_through_speed[loop].enable)
        {
            cut_through_speed += 1;
        }
        if (p_usw_chip_master[lchip]->cut_through_speed[loop].enable &&
            (p_usw_chip_master[lchip]->cut_through_speed[loop].port_speed == speed_mode ))
        {
            found = 1;
            break;
        }
    }
    /*cut_through only support 7 speed*/
    return (found && cut_through_speed <= 7) ? cut_through_speed : 0;

}

#define __ECC_RECOVER__


int32
sys_usw_chip_get_reset_hw_en(uint8 lchip)
{

   uint32 hw_reset_en;

   drv_ser_get_cfg(lchip, DRV_SER_CFG_TYPE_HW_RESER_EN, &hw_reset_en);

   return hw_reset_en;
}

STATIC int32
_sys_usw_chip_init_ser(uint8 lchip, ctc_chip_global_cfg_t* p_cfg)
{

    drv_ser_global_cfg_t  ser_cfg;

    sal_memset(&ser_cfg,0,sizeof(ser_cfg));
    ser_cfg.ecc_recover_en     = p_cfg->ecc_recover_en;
    ser_cfg.tcam_scan_en       = p_cfg->tcam_scan_en;
    ser_cfg.sbe_scan_en        = p_cfg->tcam_scan_en;
    ser_cfg.hw_reset_en            = p_cfg->ecc_recover_en;
    ser_cfg.tcam_scan_burst_entry_num = 4096;
    ser_cfg.tcam_scan_interval = 0; /* Not start tcam key scan thread when init */
    ser_cfg.sbe_scan_interval  = 0; /* Not start single bit error scan thread when init */
    ser_cfg.cpu_mask = p_usw_chip_master[lchip]->normal_affinity_mask;


    ser_cfg.create_cb   =  ctc_hash_create;
    ser_cfg.insert_cb   =  ctc_hash_insert;
    ser_cfg.free_cb     =  ctc_hash_free;
    ser_cfg.lookup_cb   =  ctc_hash_lookup;
    ser_cfg.travese_cb  =  ctc_hash_traverse;
    ser_cfg.dma_rw_cb   =  sys_usw_dma_write_dynamic_table;

    CTC_ERROR_RETURN(drv_ser_init(lchip, &ser_cfg));

    drv_ser_register_hw_reset_cb(lchip, DRV_SER_HW_RESET_CB_TYPE_DATAPATH, sys_usw_datapath_chip_reset_recover_proc);
    return CTC_E_NONE;
}


int32
sys_usw_chip_show_ecc_recover_status(uint8 lchip)
{

    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(  drv_ser_get_cfg( lchip, DRV_SER_CFG_TYPE_SHOW_ECC_STAUTS, NULL));

    return CTC_E_NONE;
}

int32
sys_usw_chip_set_tcam_scan_cfg(uint8 lchip, uint32 burst_entry_num, uint32 burst_interval)
{
    drv_ser_scan_info_t ecc_cfg;

    sal_memset(&ecc_cfg,0,sizeof(ecc_cfg));
    ecc_cfg.burst_entry_num = burst_entry_num;
    ecc_cfg.burst_interval  = burst_interval;
    CTC_ERROR_RETURN(  drv_ser_set_cfg( lchip, DRV_SER_CFG_TYPE_TCAM_SCAN_INFO, &ecc_cfg));

    return CTC_E_NONE;
}

/**
 @brief access type switch interface
*/
int32
sys_usw_chip_set_access_type(uint8 lchip, ctc_chip_access_type_t access_type)
{

    SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set access, access_type:%d\n", access_type);

    if (access_type >= CTC_CHIP_MAX_ACCESS_TYPE)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(drv_set_access_type(lchip, (drv_access_type_t)access_type));
    return CTC_E_NONE;
}

/**
 @brief access type switch interface
*/
int32
sys_usw_chip_get_access_type(uint8 lchip, ctc_chip_access_type_t* p_access_type)
{
    drv_access_type_t drv_access;

    SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_ERROR_RETURN(drv_get_access_type(lchip, &drv_access));
    *p_access_type = (ctc_chip_access_type_t)drv_access;

    return CTC_E_NONE;

}

uint16
sys_usw_get_core_freq(uint8 lchip, uint8 type)
{
   return sys_usw_dmps_get_core_clock(lchip, type);
}

int32
sys_usw_chip_get_device_info(uint8 lchip, ctc_chip_device_info_t* p_device_info)
{
    uint32      cmd = 0;
    DevId_m    read_device_id;
    uint32      tmp_device_id = 0;
    uint32      get_device_id = 0;
    uint32      get_version_id = 0;

    CTC_PTR_VALID_CHECK(p_device_info);


    cmd = DRV_IOR(DevId_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &read_device_id));

    tmp_device_id = GetDevId(V,deviceId_f,&read_device_id);

    get_version_id = tmp_device_id & 0x0F;
    get_device_id  = (tmp_device_id >> 4) & 0x0F;


    switch(get_device_id)
    {
    case 0x07:
        p_device_info->device_id = SYS_CHIP_DEVICE_ID_USW_CTC7148;
        sal_strcpy(p_device_info->chip_name, "CTC7148");
        break;
    case 0x08:
        p_device_info->device_id = SYS_CHIP_DEVICE_ID_USW_CTC7132;
        sal_strcpy(p_device_info->chip_name, "CTC7132");
        break;

    default:
        p_device_info->device_id  = SYS_CHIP_DEVICE_ID_INVALID;
        sal_strcpy(p_device_info->chip_name, "Not Support Chip");
        break;
    }
    p_device_info->version_id = get_version_id;

    return CTC_E_NONE;
}

int32 sys_usw_wb_show_module_version(uint8 lchip)
{
    char ver_str[20];

    if (!CTC_WB_ENABLE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "warmboot is not enable!\n");
        return CTC_E_NOT_READY;
    }

    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_L2 >> 16, SYS_WB_VERSION_L2 & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "L2", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_SCL >> 16, SYS_WB_VERSION_SCL & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "SCL", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_ACL >> 16, SYS_WB_VERSION_ACL & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "ACL", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_QOS >> 16, SYS_WB_VERSION_QOS & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Qos", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_OAM >> 16, SYS_WB_VERSION_OAM & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "OAM", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_APS >> 16, SYS_WB_VERSION_APS & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "APS", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_PTP >> 16, SYS_WB_VERSION_PTP & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "PTP", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_PORT >> 16, SYS_WB_VERSION_PORT & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Port", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_VLAN >> 16, SYS_WB_VERSION_VLAN & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "Vlan", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_CHIP >> 16, SYS_WB_VERSION_CHIP & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Chip", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_L3IF >> 16, SYS_WB_VERSION_L3IF & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "L3IF", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_IPUC >> 16, SYS_WB_VERSION_IPUC & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "IPUC", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_IPMC >> 16, SYS_WB_VERSION_IPMC & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "IPMC", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_WLAN >> 16, SYS_WB_VERSION_WLAN & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Wlan", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_MPLS >> 16, SYS_WB_VERSION_MPLS & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "MPLS", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_IPFIX >> 16, SYS_WB_VERSION_IPFIX & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Ipfix", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_STATS >> 16, SYS_WB_VERSION_STATS & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Stats", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_DOT1AE >> 16, SYS_WB_VERSION_DOT1AE & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "Dot1ae", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_MIRROR >> 16, SYS_WB_VERSION_MIRROR & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Mirror", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_NEXTHOP >> 16, SYS_WB_VERSION_NEXTHOP & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Nexthop", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_OVERLAY >> 16, SYS_WB_VERSION_OVERLAY & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "Overlay", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_SECURITY >> 16, SYS_WB_VERSION_SECURITY & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Security", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_LINKAGG >> 16, SYS_WB_VERSION_LINKAGG & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Linkagg", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_STACKING >> 16, SYS_WB_VERSION_STACKING & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |\n", "Stacking", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_DATAPATH >> 16, SYS_WB_VERSION_DATAPATH & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| %-8s: %-6s |", "Datapath", ver_str);
    sal_sprintf(ver_str, "v%d.%d", SYS_WB_VERSION_IP_TUNNEL >> 16, SYS_WB_VERSION_IP_TUNNEL & 0xFFFF);
    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%-8s: %-6s |\n", "IP-Tunnel", ver_str);

    return CTC_E_NONE;
}

