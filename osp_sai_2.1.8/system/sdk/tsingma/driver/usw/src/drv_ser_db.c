#include "sal.h"
#include "dal.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_common.h"
#include "drv_api.h"
#include "usw/include/drv_io.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_app.h"
#include "usw/include/drv_ser_db.h"

enum drv_ser_db_tcam_key_action_e
{
    DRV_MEM_DB_TCAM_KEY_REMOVE,
    DRV_MEM_DB_TCAM_KEY_ADD,
    DRV_MEM_DB_TCAM_KEY_ACTION_MAX
};
typedef enum drv_ser_db_tcam_key_action_e drv_ser_db_tcam_key_action_t;
struct drv_ser_db_reset_hw_register_s
{
    uint32 tbl_id;
    uint32 fld_id;
    uint8 value;
    uint8 old_value;
    uint8  type;  /*0:set before reset hw; 1:set before reset hw but recover after dynamic tbl restore;*/
};
typedef struct drv_ser_db_reset_hw_register_s drv_ser_db_reset_hw_register_t;

drv_ser_db_reset_hw_register_t drv_ser_db_reset_hw_register[] =
{
    {MucSupMiscCtl0_t, MucSupMiscCtl0_mcuSupDataMemEccCorrectDis_f, 1, 0, 0},
    {MucSupMiscCtl0_t, MucSupMiscCtl0_mcuSupDataMemEccDetectDis_f,  1, 0, 0},
    {MucSupMiscCtl0_t, MucSupMiscCtl0_mcuSupProgMemEccCorrectDis_f, 1, 0, 0},
    {MucSupMiscCtl0_t, MucSupMiscCtl0_mcuSupProgMemEccDetectDis_f,  1, 0, 0},
    {MucSupMiscCtl1_t, MucSupMiscCtl1_mcuSupDataMemEccCorrectDis_f, 1, 0, 0},
    {MucSupMiscCtl1_t, MucSupMiscCtl1_mcuSupDataMemEccDetectDis_f,  1, 0, 0},
    {MucSupMiscCtl1_t, MucSupMiscCtl1_mcuSupProgMemEccCorrectDis_f, 1, 0, 0},
    {MucSupMiscCtl1_t, MucSupMiscCtl1_mcuSupProgMemEccDetectDis_f,  1, 0, 0},

    {OamUpdateCtl_t, OamUpdateCtl_updEn_f,           0, 0, 1},
    {OamUpdateCtl_t, OamUpdateCtl_bfdUpdEn_f,        0, 0, 1},
    {IpeAgingCtl_t,  IpeAgingCtl_hwAgingEn_f,        0, 0, 1},
    {IpeAgingCtl_t,  IpeAgingCtl_gTimmer_2_scanEn_f, 0, 0, 1},
    {IpePolicing0EngineCtl_t,  IpePolicing0EngineCtl_ipePolicing0EngineCtlRefreshEn_f, 0, 0, 1},
    {IpePolicing1EngineCtl_t,  IpePolicing1EngineCtl_ipePolicing1EngineCtlRefreshEn_f, 0, 0, 1},
    {EpePolicing0EngineCtl_t,  EpePolicing0EngineCtl_epePolicing0EngineCtlRefreshEn_f, 0, 0, 1},
    {EpePolicing1EngineCtl_t,  EpePolicing1EngineCtl_epePolicing1EngineCtlRefreshEn_f, 0, 0, 1},
    {IpfixAgingTimerCtl_t,     IpfixAgingTimerCtl_agingUpdEn_f,                        0, 0, 1},
    {IpfixAgingTimerCtl_t,     IpfixAgingTimerCtl_cpuAgingEn_f,                        0, 0, 1},
    {SupIntrCtl_t,  SupIntrCtl_funcIntrEn_f,  0, 0, 1},
    {SupIntrCtl_t,  SupIntrCtl_interruptEn_f, 0, 0, 1},
    {SupIntrCtl_t,  SupIntrCtl_padIntrEn_f,   0, 0, 1},
    {SupIntrCtl_t,  SupIntrCtl_pcieIntrEn_f,  0, 0, 1},
    {SupIntrCtl_t,  SupIntrCtl_socIntrEn_f,   0, 0, 1}
};

#define DRV_MEM_DB_ACC_TBL_MIN_SIZE          12
#define DRV_MEM_DB_MALLOC_CPU_MEM_SIZE   (24*1024)  /*byte,must 2^n aline*/
#define DRV_MEM_DB_MALLOC_DMA_MEM_SIZE   (48*1024)  /*byte*/

extern dal_op_t g_dal_op;
extern int32 dal_get_chip_number(uint8* p_num);
extern int32 drv_usw_ftm_get_mplshash_mode(uint8 lchip, uint8* is_mpls);

/*rewrite only rewrite to phy-memory.It is
unnessary to rewrite backup-data,because that
backup-data only can be rewrite when drv-io be called*/
int32
drv_ser_db_rewrite_tcam(uint8 lchip, uint8 mem_id, uint8 mem_type, uint8 mem_order, uint32 entry_idx, void* p_data)
{
    uint8 action = 0;
    /*uint8 start_mem_id = 0;*/
    tbl_entry_t* p_tbl_entry = (tbl_entry_t*)p_data;

    action = (p_tbl_entry->data_entry == NULL) ? DRV_MEM_DB_TCAM_KEY_REMOVE : DRV_MEM_DB_TCAM_KEY_ADD;
    /*start_mem_id = mem_id - mem_order;*/

    if (DRV_MEM_DB_TCAM_KEY_REMOVE == action)
    {
        if (mem_type == DRV_FTM_MEM_TCAM_FLOW_KEY)
        {
            drv_usw_chip_remove_flow_tcam_entry(lchip, mem_order, entry_idx);
        }
        /*lpm tcam*/
        else if (mem_type == DRV_FTM_MEM_TCAM_LPM_KEY)
        {
            drv_ser_db_unmap_lpm_tcam(lchip, &mem_id, &mem_order, (uint16*)&entry_idx);/*12 to 3 map*/
            drv_usw_chip_remove_lpm_tcam_ip_entry(lchip, mem_order, entry_idx);

        }
        /*cid tcam*/
        else if (mem_type == DRV_FTM_MEM_TCAM_CID_KEY)
        {
            drv_usw_chip_remove_cid_tcam_entry(lchip, entry_idx);

        }
        /*enque tcam*/
        else if (mem_type == DRV_FTM_MEM_TCAM_QUEUE_KEY)
        {
            drv_usw_chip_remove_enque_tcam_entry(lchip, entry_idx);
        }
        else
        {
            return DRV_E_INVAILD_TYPE;
        }

    }
    else/*add*/
    {
        if (mem_type == DRV_FTM_MEM_TCAM_FLOW_KEY)
        {
            drv_usw_chip_write_flow_tcam_data_mask(lchip, mem_order, entry_idx, p_tbl_entry->data_entry, p_tbl_entry->mask_entry);
        }
        /*lpm tcam*/
        else if (mem_type == DRV_FTM_MEM_TCAM_LPM_KEY)
        {
            drv_ser_db_unmap_lpm_tcam(lchip, &mem_id, &mem_order, (uint16*)&entry_idx);/*12 to 3 map*/
            drv_usw_chip_write_lpm_tcam_ip_data_mask(lchip, mem_order, entry_idx, p_tbl_entry->data_entry, p_tbl_entry->mask_entry);
        }
        /*cid tcam*/
        else if (mem_type == DRV_FTM_MEM_TCAM_CID_KEY)
        {
            drv_usw_chip_write_cid_tcam_data_mask(lchip, entry_idx, p_tbl_entry->data_entry, p_tbl_entry->mask_entry);
        }
        /*enque tcam*/
        else if (mem_type == DRV_FTM_MEM_TCAM_QUEUE_KEY)
        {
            drv_usw_chip_write_enque_tcam_data_mask(lchip, entry_idx, p_tbl_entry->data_entry, p_tbl_entry->mask_entry);
        }
        else
        {
            return DRV_E_INVAILD_TYPE;
        }
    }
    return DRV_E_NONE;
}

int32
drv_ser_db_get_mem_id_info(uint8 lchip, uint8 mem_id, uint8* p_order , uint8* p_type)
{
    int32 ret = 0;
    drv_ftm_mem_info_t mem_info;

    sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));

    mem_info.info_type = DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL;
    mem_info.mem_id    = mem_id;
    ret = drv_ftm_get_mem_info(lchip, &mem_info);
    *p_type = mem_info.mem_type;
    *p_order = mem_info.mem_sub_id;
    return  ret;
}

/****************************************************************************************************
||tbl-name                || D2-tbl num   || TM-tbl num  || D2-ramsize(tbl-num/ram)|| D2-ramsize    ||
|| DynamicKeyShareRam12W  || 128k         || 128k        || 8k                     || 8K            ||
|| DynamicAdShareRam12W   || 64k          || 64k         || 8K                     || 8K            ||
|| DynamicEditShareRam12W || 32k          || 64k         || 4K                     || 8K            ||
|| LpmTcamAdMem           || 12k          || 24k         || 2k(if lpm-ad ram is 6) || 4k            ||
|| FlowTcamAdMem          || 16k          || 19k         || 1k                     || 1k            ||
*****************************************************************************************************/
int32
drv_ser_db_get_dyn_unite_tbl_info(uint8 lchip ,drv_ser_db_err_mem_info_t err_mem_info, uint32* p_rw_tbl, uint32* p_rw_idx, uint8* p_loop_tm)
{
   uint8  loop_time = 0;
   uint32 rewrite_tbl = 0;
   uint32 rewrite_idx = 0;
   uint16 lpm_bk_start_idx  = 0;
   uint8  err_mem_type   = err_mem_info.err_mem_type;
   uint8  err_mem_order  = err_mem_info.err_mem_order;
   uint32 err_mem_offset = err_mem_info.err_mem_offset;
   uint32 err_entry_size = err_mem_info.err_entry_size;
   uint8  is_mpls = 0;

   loop_time = 1;
   switch (err_mem_type)
   {
       case  DRV_FTM_MEM_DYNAMIC_KEY:
           rewrite_tbl = DynamicKeyShareRam12W_t;
           rewrite_idx = err_mem_order*8*1024 + err_mem_offset/err_entry_size;
           break;
       case  DRV_FTM_MEM_DYNAMIC_AD:
           rewrite_tbl = DynamicAdShareRam12W_t;
           rewrite_idx = err_mem_order*8*1024 + err_mem_offset/err_entry_size;
           break;
       case  DRV_FTM_MEM_DYNAMIC_EDIT:
           rewrite_tbl = DynamicEditShareRam12W_t;
           if (DRV_IS_DUET2(lchip))
           {
               rewrite_idx = err_mem_order*4*1024 + err_mem_offset/err_entry_size;
           }
           else
           {
               rewrite_idx = err_mem_order*8*1024 + err_mem_offset/err_entry_size;
           }
           break;
       case  DRV_FTM_MEM_TCAM_FLOW_AD:
           rewrite_tbl = FlowTcamAdMem_t;
           rewrite_idx = err_mem_order*1*1024 + err_mem_offset/err_entry_size;
           break;
       case  DRV_FTM_MEM_TCAM_LPM_AD:/*RTL Special proc*/
           rewrite_tbl = LpmTcamAdMem_t;
           if (err_mem_order < DRV_MEM_DB_LPM_KEY_BLOCK_NUM*2)
           {
               lpm_bk_start_idx = (err_mem_order % DRV_MEM_DB_LPM_KEY_BLOCK_NUM )*512;
           }
           else
           {
               lpm_bk_start_idx = (err_mem_order % (DRV_MEM_DB_LPM_KEY_BLOCK_NUM*2))*1024;
           }
           if (DRV_IS_DUET2(lchip))
           {
               rewrite_idx = (err_mem_order/DRV_MEM_DB_LPM_KEY_BLOCK_NUM)*4*1024 + lpm_bk_start_idx + err_mem_offset/err_entry_size;
           }
           else
           {
               rewrite_idx = (err_mem_order/DRV_MEM_DB_LPM_KEY_BLOCK_NUM)*4*1024 + lpm_bk_start_idx + (err_mem_offset/err_entry_size)/2;
               rewrite_idx = (rewrite_idx*2) + (err_mem_offset/err_entry_size)%2;
           }
           break;
       case DRV_FTM_MEM_MIXED_KEY:/*TM*/
           drv_usw_ftm_get_mplshash_mode(lchip, &is_mpls);
           if (is_mpls)
           {
               rewrite_tbl = MplsHashKey2w_t;
               loop_time   = DRV_MEM_DB_MPLS_HS_KEY_ECC_SIZE/TABLE_ENTRY_SIZE(lchip, rewrite_tbl);
               rewrite_idx = err_mem_order*8*1024 + (err_mem_offset/err_entry_size)*loop_time;
           }
           else/*xgpon*/
           {
               rewrite_tbl = MplsHashGemPortKey2w_t;
               loop_time   = DRV_MEM_DB_MPLS_HS_KEY_ECC_SIZE/TABLE_ENTRY_SIZE(lchip, rewrite_tbl);
               rewrite_idx = err_mem_order*6*1024 + (err_mem_offset/err_entry_size)*loop_time;
           }
           break;
       case  DRV_FTM_MEM_MIXED_AD:/*TM*/
           drv_usw_ftm_get_mplshash_mode(lchip, &is_mpls);
           if (is_mpls)
           {
               rewrite_tbl = MplsHashAd6w_t;
               rewrite_idx = err_mem_offset/err_entry_size;
           }
           else/*xgpon*/
           {
               rewrite_tbl = MplsHashAd2w_t;
               loop_time   = TABLE_ENTRY_SIZE(lchip, MplsHashAd6w_t)/TABLE_ENTRY_SIZE(lchip, MplsHashAd2w_t);
               rewrite_idx = (err_mem_offset/err_entry_size)*loop_time;  
           }
           break;
       case DRV_FTM_MEM_SHARE_MEM:/*TM*/
           if (err_mem_order < 4)
           {
               rewrite_tbl = ShareBufferDsMacKey_t;/*sharebuffer0-3*/
               loop_time   = DRV_MEM_DB_SHARE_BUFFER_ECC_SIZE/TABLE_ENTRY_SIZE(lchip, rewrite_tbl);
               rewrite_idx = err_mem_order*64*1024 + (err_mem_offset/err_entry_size)*loop_time;
           }
           else
           {
               rewrite_tbl = ShareBufferRam3W_t;/*sharebuffer4-6*/
               loop_time   = DRV_MEM_DB_SHARE_BUFFER_ECC_SIZE/TABLE_ENTRY_SIZE(lchip, rewrite_tbl);
               rewrite_idx = (err_mem_order-4)*64*1024 + (err_mem_offset/err_entry_size)*loop_time;
           }



           break;
       default:
           return DRV_E_INVALID_PARAMETER;
           break;
   }

   *p_loop_tm = loop_time;
   *p_rw_tbl  = rewrite_tbl;
   *p_rw_idx  = rewrite_idx;

    return DRV_E_NONE;
}

int32
drv_ser_db_rewrite_acc_tbl(uint8 lchip, uint16 tbl_id, uint16 tbl_idx,uint8* p_data)
{
    uint8     cam_type  = 0;
    uint8     cam_num   = 0;
    int32     ret       = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.tbl_id = tbl_id;
    in.index  = tbl_idx;
    in.data   = (void*)p_data;

    ret += drv_usw_ftm_get_cam_by_tbl_id(in.tbl_id, &cam_type, &cam_num);
    if (!cam_num)
    {
        DRV_DBG_INFO("DRV_mem_db:can not get this tbl cam_num.\n");
    }
    in.index += cam_num;
    ret += drv_usw_acc(lchip, (void*)&in, (void*)&out);

    return ret;
}



int32 drv_ser_db_get_tcam_local_entry_size(uint8 lchip, uint8 mem_id, uint32*p_entry_size)
{
    drv_ftm_mem_info_t mem_info;

    sal_memset(&mem_info, 0 ,sizeof(drv_ftm_mem_info_t));
    mem_info.mem_id = mem_id;
    mem_info.info_type = DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL;
    drv_ftm_get_mem_info(lchip, &mem_info);

    switch (mem_info.mem_type)
    {
        case DRV_FTM_MEM_TCAM_FLOW_KEY:
            *p_entry_size = DRV_BYTES_PER_ENTRY;
            break;
        case DRV_FTM_MEM_TCAM_LPM_KEY:
            *p_entry_size = DRV_LPM_KEY_BYTES_PER_ENTRY;
            break;
        case DRV_FTM_MEM_TCAM_CID_KEY:
            *p_entry_size = 1*4;
            break;
        case DRV_FTM_MEM_TCAM_QUEUE_KEY:
            *p_entry_size = (DRV_IS_DUET2(lchip) ? (2*4):(3*4));
            break;
        default:
            *p_entry_size = 0;
            return  DRV_E_INVALID_PARAMETER;
            break;
    }

    return  DRV_E_NONE;
}

int32
drv_ser_db_map_lpm_tcam(uint8 lchip, uint8* p_mem_id, uint8* p_mem_order, uint16* p_entry_idx)
{
    uint32 per_blk_entry_size = 0;
    uint32  per_blk_mem_size = 0;
    uint32 per_blk_entry_index = 0;
    uint8 i = 0;
    uint8 mem_id = *p_mem_id;
    uint8 mem_order = *p_mem_order;
    uint8  start_mem_id = 0;
    uint32 entry_index_all = 0;

    start_mem_id = mem_id - mem_order;
    for (i = 0; i < DRV_MEM_DB_LPM_TCAM_BLOCK_NUM; i++)
    {
        drv_usw_get_memory_size(lchip, start_mem_id + DRV_MEM_DB_LPM_TCAM_BLOCK_NUM*mem_order + i, &per_blk_mem_size);
        drv_ser_db_get_tcam_local_entry_size(lchip, start_mem_id + DRV_MEM_DB_LPM_TCAM_BLOCK_NUM*mem_order + i, &per_blk_entry_size);
        if ((per_blk_mem_size == 0) || (per_blk_entry_size == 0))
        {
            return DRV_E_INVALID_MEM;
        }

        per_blk_entry_index = per_blk_mem_size / per_blk_entry_size;

        if (*p_entry_idx < entry_index_all + per_blk_entry_index)
        {
            *p_mem_id  =  start_mem_id + DRV_MEM_DB_LPM_TCAM_BLOCK_NUM*mem_order + i;
            *p_mem_order = DRV_MEM_DB_LPM_TCAM_BLOCK_NUM*mem_order + i;
            *p_entry_idx = (*p_entry_idx) - entry_index_all;
            return  DRV_E_NONE;
        }
        entry_index_all += per_blk_entry_index;
    }

    return  DRV_E_NONE;
}

int32
drv_ser_db_unmap_lpm_tcam(uint8 lchip, uint8* p_mem_id, uint8* p_mem_order, uint16* p_entry_idx)
{
    uint32 per_blk_entry_index = 0;
    uint32 per_blk_entry_size  = 0;
    uint32 per_blk_size        = 0;
    uint8 mem_id = *p_mem_id;
    uint8 mem_order = *p_mem_order;
    uint32 entry_index_all = 0;
    uint8 loop_time    = 0;
    uint8 start_mem_id = 0;
    uint8 i = 0;

    start_mem_id = mem_id - mem_order;
    *p_mem_order = mem_order / DRV_MEM_DB_LPM_TCAM_BLOCK_NUM;
    *p_mem_id    = start_mem_id + *p_mem_order;
    loop_time    = mem_order % DRV_MEM_DB_LPM_TCAM_BLOCK_NUM;

    for (i = 0; i < loop_time; i++)
    {
        drv_ser_db_get_tcam_local_entry_size(lchip, mem_id -i -1, &per_blk_entry_size);
        drv_usw_get_memory_size(lchip, mem_id, &per_blk_size);
        per_blk_entry_index = per_blk_size / per_blk_entry_size;
        entry_index_all += per_blk_entry_index;
    }
    *p_entry_idx = (*p_entry_idx) + entry_index_all;

    return  DRV_E_NONE;
}

STATIC int32
_drv_ser_db_write_tbl_by_dma(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* p_data, uint32 entry_num, uint8 mode)
{
    uint32 tbl_hw_addr  = 0;
    int32  ret =  0;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    drv_usw_table_get_hw_addr(lchip, tbl_id, index, &tbl_hw_addr, FALSE);
    ret =  p_ser_db->dma_rw_cb(lchip, tbl_hw_addr, p_data, entry_num, TABLE_ENTRY_SIZE(lchip, tbl_id), mode);
    if (ret)
    {
        DRV_DBG_INFO("[Reset Hardware]DMA Rewrite failed!!!\n");
        return ret;
    }
    return  DRV_E_NONE;
}

/**mode-0:ecc;mode-1:reset-chip*/
int32
drv_ser_db_rewrite_dyn(uint8 lchip, uint8 err_mem_id, uint32 err_mem_offset, uint32 err_entry_size, uint8* p_data, uint8 mode, uint32 ram_size)
{
   uint8 err_mem_type = 0;
   uint8 err_mem_order = 0;
   uint8 loop_time    = 0;
   uint32 rewrite_tbl = 0;
   uint32 rewrite_idx = 0;
   uint8  i           = 0;
   uint8* p_temp      = NULL;
   uint8  hash_module = 0;
   uint8  is_recover  = 0;
   int32  ret         = 0;
   drv_ftm_mem_info_t mem_info;
   drv_ser_db_err_mem_info_t err_mem_info;
   uint32 temp_index_size   = 0;

   /*acc-tbl special proc*/
   if (mode == 0)
   {
        loop_time = err_entry_size/DRV_MEM_DB_ACC_TBL_MIN_SIZE;
        for (i = 0; i < loop_time; i++)
        {
           sal_memset(&mem_info, 0 ,sizeof(drv_ftm_mem_info_t));
           mem_info.info_type = DRV_FTM_MEM_INFO_TBL_INDEX;
           mem_info.mem_id = err_mem_id;
           mem_info.mem_offset = err_mem_offset + i*DRV_MEM_DB_ACC_TBL_MIN_SIZE;
           DRV_IF_ERROR_RETURN(drv_ftm_get_mem_info(lchip, &mem_info));
           ret = drv_usw_acc_get_hash_module_from_tblid(lchip, mem_info.table_id, &hash_module);
           if ((mem_info.table_id == 0) || (ret != 0))
           {
               break;
           }

           if ((DRV_IS_DUET2(lchip) && (DRV_ACC_HASH_MODULE_FIB_HOST0 == hash_module))
               || (DRV_ACC_HASH_MODULE_FDB == hash_module))
           {
               drv_ser_db_rewrite_acc_tbl(lchip, mem_info.table_id, mem_info.index, p_data + i*DRV_MEM_DB_ACC_TBL_MIN_SIZE);
               is_recover ++;
           }
        }
        if (is_recover)
        {
           return DRV_E_NONE;
        }
   }
   /*common dynamic-tbl recover proc*/
   drv_ser_db_get_mem_id_info(lchip, err_mem_id,&err_mem_order,&err_mem_type);

   err_mem_info.err_mem_type   = err_mem_type;
   err_mem_info.err_mem_order  = err_mem_order;
   err_mem_info.err_mem_offset = err_mem_offset;
   err_mem_info.err_entry_size = err_entry_size;
   drv_ser_db_get_dyn_unite_tbl_info(lchip, err_mem_info, &rewrite_tbl,&rewrite_idx, &loop_time);

   if (mode == 0)
   {
       for (i = 0 ; i < loop_time ; i++)
       {
           p_temp = (uint8*)p_data + i*TABLE_ENTRY_SIZE(lchip, rewrite_tbl);
           drv_usw_chip_sram_tbl_write(lchip, rewrite_tbl, (rewrite_idx + i), (uint32*)p_temp);
       }
   }
   else
   {
       if ((rewrite_tbl == MplsHashKey2w_t) ||  (rewrite_tbl == MplsHashAd2w_t) || (rewrite_tbl == MplsHashGemPortKey2w_t))/*special proc*/
       {
           temp_index_size = ram_size/TABLE_ENTRY_SIZE(lchip, rewrite_tbl);
       }
       else
       {
           temp_index_size = ram_size/err_entry_size;
       }
       _drv_ser_db_write_tbl_by_dma(lchip, rewrite_tbl, rewrite_idx, (uint32*)p_data, temp_index_size, 0);
   }
   return DRV_E_NONE;
}

/*X = ~data & mask; Y = data & Mask*/
int32
drv_ser_db_convert_tcam_wr_content(uint8 lchip, uint32 tcam_entry_width, uint32 *data, uint32 *mask)
{
    uint32 bit_pos = 0;
    uint32 index = 0;
    uint32 bit_offset = 0;

    if ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV))
    {
        return DRV_E_NONE;
    }

    for (bit_pos = 0; bit_pos < tcam_entry_width; bit_pos++)
    {
        index = bit_pos / 32;
        bit_offset = bit_pos % 32;

        if ((!IS_BIT_SET(data[index], bit_offset))
            && IS_BIT_SET(mask[index], bit_offset))    /* data = 0; mask = 1 */
        {
            SET_BIT(mask[index], bit_offset);   /*X*/
            CLEAR_BIT(data[index], bit_offset); /*Y*/
        }
        else if (IS_BIT_SET(data[index], bit_offset)
            && (!IS_BIT_SET(mask[index], bit_offset))) /* data = 1; mask = 0 */
        {
            CLEAR_BIT(mask[index], bit_offset);
            CLEAR_BIT(data[index], bit_offset);
        }
        else if ((!IS_BIT_SET(data[index], bit_offset))
            && (!IS_BIT_SET(mask[index], bit_offset))) /* data = 0; mask = 0 */
        {
            CLEAR_BIT(mask[index], bit_offset);
            CLEAR_BIT(data[index], bit_offset);
        }
        else                                           /* data = 1; mask = 1 */
        {
            CLEAR_BIT(mask[index], bit_offset);    /* Return_Data = 0; Return_Mask = 0 */
            SET_BIT(data[index], bit_offset);
        }
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_get_ram_entry_size(uint8 lchip, uint8 ram_type, uint8* p_entry_size)
{
    if (p_entry_size == NULL)
    {
        return DRV_E_NONE;
    }
    switch(ram_type)
    {
    case DRV_FTM_MEM_DYNAMIC_KEY:
    case DRV_FTM_MEM_DYNAMIC_AD:
    case DRV_FTM_MEM_DYNAMIC_EDIT:
    case DRV_FTM_MEM_SHARE_MEM:
        *p_entry_size = 48; /*byte*/
        break;
    case DRV_FTM_MEM_TCAM_FLOW_AD:
    case DRV_FTM_MEM_MIXED_AD:
        *p_entry_size = 24; /*byte*/
        break;
    case DRV_FTM_MEM_TCAM_LPM_AD:
        *p_entry_size = 8; /*byte*/
        break;
    case DRV_FTM_MEM_MIXED_KEY:
        *p_entry_size = 64; /*byte*/
        break;
    default:
        break;
    }
    return DRV_E_NONE;

}

STATIC uint8
_drv_ser_db_get_ram_type(uint8 lchip, uint8 ram_id, uint8 is_detail)
{
    drv_ftm_mem_info_t mem_info;
    int32 ret = 0;

    sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));
    mem_info.info_type = is_detail ? DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL : DRV_FTM_MEM_INFO_MEM_TYPE;
    mem_info.mem_id = ram_id;
    ret = drv_ftm_get_mem_info(lchip, &mem_info);

    if (ret == 0)
    {
        return mem_info.mem_type;
    }
    else
    {
        return 0xff;
    }
}

STATIC uint8
_drv_ser_db_get_tbl_type(uint8 lchip, tbls_id_t tb_id)

{
    int32 ret = 0;
    drv_ftm_tbl_detail_t tbl_info;

    sal_memset(&tbl_info, 0, sizeof(drv_ftm_tbl_detail_t));
    tbl_info.type = DRV_FTM_TBL_TYPE;
    tbl_info.tbl_id = tb_id;
    ret = drv_ftm_get_tbl_detail(lchip, &tbl_info);

    if (ret == 0)
    {
        return tbl_info.tbl_type;
    }
    else
    {
        return 0xff;
    }
}

STATIC int32
 _drv_ser_db_tcam_get_mem_size(uint8 lchip, uint8 ram_id, uint32 *p_mem_size)
{
    uint32 mem_size = 0;
    if (NULL == p_mem_size)
    {
        return DRV_E_NONE;
    }

    mem_size = *p_mem_size * 2;/*byte*/

    if (DRV_FTM_MEM_TCAM_FLOW_KEY == _drv_ser_db_get_ram_type(lchip, ram_id, 1))
    {
        *p_mem_size = mem_size/12*16;/*for 2power-aline: entrysize:8w = data3w + mask3w*/
    }
    else if (DRV_FTM_MEM_TCAM_LPM_KEY == _drv_ser_db_get_ram_type(lchip, ram_id, 1))
    {
        *p_mem_size = mem_size/8*8;/*for 2power-aline: entrysize:4w = data2w + mask2w*/
    }
    else if (DRV_FTM_MEM_TCAM_CID_KEY == _drv_ser_db_get_ram_type(lchip, ram_id, 1))
    {
        *p_mem_size = mem_size/4*4;/*for 2power-aline: entrysize:2w = data1w + mask1w*/
    }
    else if (DRV_FTM_MEM_TCAM_QUEUE_KEY == _drv_ser_db_get_ram_type(lchip, ram_id, 1))
    {
        if (DRV_IS_DUET2(lchip))
        {
            *p_mem_size = mem_size/8*16;/*for 2power-aline: entrysize:8w = data2w + mask2w + entryvalid*/
        }
        else
        {
            *p_mem_size = mem_size/12*16;/*for 2power-aline: entrysize:8w = data3w + mask3w + entryvalid*/
        }
    }
    else
    {
        return DRV_E_NONE;
    }

    return DRV_E_NONE;

}


int32
drv_ser_db_set_tcam_mask_fld(uint8 lchip, drv_ser_db_mask_tcam_fld_t * p_void)
{
    uint8 tcam_type = 0;
    if (NULL == p_void)
    {
        return DRV_E_NONE;
    }

    tcam_type = _drv_ser_db_get_ram_type(lchip, p_void->ram_id, 1);

    if ((DRV_MEM_DB_MASK_TCAM_BY_STORE ==  p_void->type) || (DRV_MEM_DB_MASK_TCAM_BY_READ ==  p_void->type))
    {
        if ((NULL == p_void->p_data) || (NULL == p_void->p_mask))
        {
            return DRV_E_NONE;
        }
    }
    else if (DRV_MEM_DB_MASK_TCAM_BY_INIT ==  p_void->type)
    {
        if (NULL == p_void->p_data)
        {
            return DRV_E_NONE;
        }
    }
    switch (p_void->type)
    {
        case DRV_MEM_DB_MASK_TCAM_BY_STORE:
            if (tcam_type == DRV_FTM_MEM_TCAM_FLOW_KEY)
            {
                p_void->p_data[2] |= 0xffff0000;
                p_void->p_mask[2] |= 0xffff0000;
                p_void->p_mask[3] |= (p_void->entry_valid ? 0xffffffff:0x7fffffff);
                p_void->p_mask[4] |= 0xffffffff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_LPM_KEY)
            {
                p_void->p_data[1] |= 0xffffc000;
                p_void->p_mask[1] |= (p_void->entry_valid ? 0xffffc000:0x7fffc000);
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_CID_KEY)
            {
                p_void->p_data[0] |= 0xfffc0000;
                p_void->p_mask[0] |= (p_void->entry_valid ? 0xfffc0000:0x7ffc0000);
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_QUEUE_KEY)
            {
                if (DRV_IS_DUET2(lchip))
                {
                    p_void->p_data[1] |= 0xffc00000;
                    p_void->p_mask[1] |= 0xffc00000;
                    p_void->p_mask[2] |= (p_void->entry_valid ? 0xffffffff:0x7fffffff);
                    p_void->p_mask[3] |= 0xffffffff;
                    p_void->p_mask[4] |= 0xffffffff;
                    p_void->p_mask[5] |= 0xffffffff;
                }
                else
                {
                    p_void->p_data[2] |= 0xffff0000;
                    p_void->p_mask[2] |= 0xffff0000;
                    p_void->p_mask[3] |= (p_void->entry_valid ? 0xffffffff:0x7fffffff);
                    p_void->p_mask[4] |= 0xffffffff;
                }
            }
            break;
        case DRV_MEM_DB_MASK_TCAM_BY_READ:
            if (tcam_type == DRV_FTM_MEM_TCAM_FLOW_KEY)
            {
                p_void->p_data[2] &= 0x0000ffff;
                p_void->p_mask[2] &= 0x0000ffff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_LPM_KEY)
            {
                p_void->p_data[1] &= 0x00003fff;
                p_void->p_mask[1] &= 0x00003fff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_CID_KEY)
            {
                p_void->p_data[0] &= 0x0003ffff;
                p_void->p_mask[0] &= 0x0003ffff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_QUEUE_KEY)
            {
                if (DRV_IS_DUET2(lchip))
                {
                    p_void->p_data[1] &= 0x003fffff;
                    p_void->p_mask[1] &= 0x003fffff;
                }
                else
                {
                    p_void->p_data[2] &= 0x0000ffff;
                    p_void->p_mask[2] &= 0x0000ffff;
                }
            }
            break;
        case DRV_MEM_DB_MASK_TCAM_BY_INIT:
            if (tcam_type == DRV_FTM_MEM_TCAM_FLOW_KEY)
            {
                p_void->p_data[6] &= 0x7fffffff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_LPM_KEY)
            {
                p_void->p_data[3] &= 0x7fffffff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_CID_KEY)
            {
                p_void->p_data[1] &= 0x7fffffff;
            }
            else if (tcam_type == DRV_FTM_MEM_TCAM_QUEUE_KEY)
            {
                if (DRV_IS_DUET2(lchip))
                {
                    p_void->p_data[4] &= 0x7fffffff;
                }
                else
                {
                    p_void->p_data[6] &= 0x7fffffff;
                }
            }
            break;
    }

    return DRV_E_NONE;
}
uint8
drv_ser_db_get_tcam_entry_size_aline(uint8 lchip, uint8 ram_id)
{
    uint8 tcam_type = 0;

    tcam_type = _drv_ser_db_get_ram_type(lchip, ram_id, 1);

    if (tcam_type == DRV_FTM_MEM_TCAM_FLOW_KEY)
    {
        return 8*4;
    }
    else if (tcam_type == DRV_FTM_MEM_TCAM_LPM_KEY)
    {
        return 4*4;
    }
    else if (tcam_type == DRV_FTM_MEM_TCAM_CID_KEY)
    {
        return 2*4;
    }
    else if (tcam_type == DRV_FTM_MEM_TCAM_QUEUE_KEY)
    {
        return 8*4;
    }

    return 0;
}

STATIC bool
_drv_ser_db_invalid_hash_key_tbl(uint8 lchip, uint32 tbl_id)
{
    uint8 hash_mod = 0;
    int32 ret      = 0;

    ret = drv_usw_acc_get_hash_module_from_tblid(lchip, tbl_id, &hash_mod);
    if (((DRV_ACC_HASH_MODULE_MAC_LIMIT == hash_mod)
        || (DRV_ACC_HASH_MODULE_IPFIX == hash_mod)
        || (DRV_ACC_HASH_MODULE_AGING == hash_mod))
        && (ret == DRV_E_NONE))
    {
        return TRUE;
    }
    return FALSE;
}

STATIC int32
_drv_ser_db_rollback_dynamic_tbl(uint8 lchip, uint8 ramid_end)
{
    uint8 i = 0;
    uint8* p_dynamic_info = NULL;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    for (i = 0 ; i < ramid_end; i++)
    {
        p_dynamic_info = p_ser_db->dynamic_tbl[i];
        if (p_dynamic_info)
        {
            mem_free(p_dynamic_info);
            p_dynamic_info = NULL;
        }
        else
        {
            continue;
        }
    }

    p_ser_db->catch_mem_dynamic_size = 0;
    p_ser_db->catch_mem_tcam_key_size = 0;

    return  DRV_E_NONE;
}


#define DRV_HASH_MIX(a, b, c)        \
    {                                         \
        a -= b; a -= c; a ^= (c >> 13);           \
        b -= c; b -= a; b ^= (a << 8);            \
        c -= a; c -= b; c ^= (b >> 13);           \
        a -= b; a -= c; a ^= (c >> 12);           \
        b -= c; b -= a; b ^= (a << 16);           \
        c -= a; c -= b; c ^= (b >> 5);            \
        a -= b; a -= c; a ^= (c >> 3);            \
        b -= c; b -= a; b ^= (a << 10);           \
        c -= a; c -= b; c ^= (b >> 15);           \
    }                                         \



uint32
drv_hash_caculate(uint32 size, void* kk)
{
    uint32 len = 0;
    uint32 a = 0x9e3779b9;
    uint32 b = 0x9e3779b9;
    uint32 c = 0;
    uint8* k = (uint8*) kk;

    len = size;

    while (len >= 12)
    {
        a += (k[0] + ((uint32)k[1] << 8) + ((uint32)k[2] << 16) + ((uint32)k[3] << 24));
        b += (k[4] + ((uint32)k[5] << 8) + ((uint32)k[6] << 16) + ((uint32)k[7] << 24));
        c += (k[8] + ((uint32)k[9] << 8) + ((uint32)k[10] << 16) + ((uint32)k[11] << 24));
        DRV_HASH_MIX(a, b, c);
        k += 12;
        len -= 12;
    }

    /*------------------------------------- handle the last 11 bytes */
    c += size;

    switch (len)            /* all the case statements fall through */
    {
    case 11:
        c += ((uint32)k[10] << 24);

    case 10:
        c += ((uint32)k[9] << 16);

    case 9:
        c += ((uint32)k[8] << 8);

    /* the first byte of c is reserved for the length */
    case 8:
        b += ((uint32)k[7] << 24);

    case 7:
        b += ((uint32)k[6] << 16);

    case 6:
        b += ((uint32)k[5] << 8);

    case 5:
        b += k[4];

    case 4:
        a += ((uint32)k[3] << 24);

    case 3:
        a += ((uint32)k[2] << 16);

    case 2:
        a += ((uint32)k[1] << 8);

    case 1:
        a += k[0];
        /* case 0: nothing left to add */
    }

    DRV_HASH_MIX(a, b, c);
    /*-------------------------------------------- report the result */

    return c;
}


STATIC uint32
_drv_ser_db_static_tbl_hash_make(void* p_data)
{
    uint8 len = DRV_MEM_DB_TBL_ID_SIZE;

    return drv_hash_caculate(len, p_data);
}

STATIC bool
_drv_ser_db_static_tbl_hash_cmp(uint8* bucket_data , uint16* p_data)
{
    if (sal_memcmp(bucket_data, p_data, DRV_MEM_DB_TBL_ID_SIZE) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_drv_ser_db_static_tbl_travese_fn(void* p_data,  void* p_lchip)
{
    uint16 tbl_index = 0;
    uint16 tbl_id    = 0;
    uint8  lchip     = 0;
    int32  ret       = 0;
    uint8* p_bucket_data = (uint8*)p_data;

    if ((p_data == NULL) || (p_lchip == NULL))
    {
        return DRV_E_NONE;
    }

    lchip = *((uint8*)p_lchip);
    if (p_bucket_data)
    {
        sal_memcpy(&tbl_id, p_bucket_data, DRV_MEM_DB_TBL_ID_SIZE);
        if ((tbl_id == 0) || (DRV_GET_TBL_INFO_DB(tbl_id) == 0))
        {
            return DRV_E_NONE;
        }

        p_bucket_data += DRV_MEM_DB_TBL_ID_SIZE;
        for (tbl_index = 0; tbl_index < TABLE_MAX_INDEX(lchip, tbl_id); tbl_index++)
        {
            ret = drv_usw_chip_sram_tbl_write(lchip, (tbls_id_t)tbl_id, tbl_index, (uint32*)p_bucket_data);
            if (ret)
            {
                DRV_DBG_INFO("\nFAILED! static Table hot recover %d\n",tbl_id);
                break;
            }
            p_bucket_data += TABLE_ENTRY_SIZE(lchip, tbl_id);
        }
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_init_static_tbl(uint8 lchip)
{
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    if  (NULL == p_ser_db->create_cb)
    {
        return DRV_E_NONE;
    }
    p_ser_db->p_static_tbl_hash = (p_ser_db->create_cb)(70,
                                           100,
                                           (mem_db_key_fn)_drv_ser_db_static_tbl_hash_make,
                                           (mem_db_cmp_fn)_drv_ser_db_static_tbl_hash_cmp);

    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_init_dynamic_tbl(uint8 lchip)
{
    uint8 ramid = 0;
    uint32 cur  = 0;
    uint8 entry_size_aline  = 0;
    uint32 offset   = 0;
    uint32 mem_size = 0;
    uint8** p_dynamic_info = NULL;
    drv_ser_db_mask_tcam_fld_t tcam_cfg;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    for (ramid = 0; ramid < DRV_FTM_MAX_ID; ramid++)
    {
        drv_usw_get_memory_size(lchip, ramid, &mem_size);
        if (((_drv_ser_db_get_ram_type(lchip, ramid, 0) == DRV_FTM_MEM_TCAM) && (!p_ser_db->tcam_key_backup_en))
            || ((_drv_ser_db_get_ram_type(lchip, ramid, 0) == DRV_FTM_MEM_DYNAMIC) && (!p_ser_db->dynamic_tbl_backup_en))
            || (!mem_size))
        {
            continue;
        }

        if (_drv_ser_db_get_ram_type(lchip, ramid, 0) == DRV_FTM_MEM_TCAM)
        {
            _drv_ser_db_tcam_get_mem_size(lchip, ramid, &mem_size);
        }

        p_dynamic_info =  &(p_ser_db->dynamic_tbl[ramid]);
        *p_dynamic_info = (uint8*)mem_malloc(MEM_SYSTEM_MODULE, mem_size);
        if (*p_dynamic_info == NULL)
        {
            _drv_ser_db_rollback_dynamic_tbl(lchip, ramid);
            return DRV_E_NO_MEMORY;
        }

        if (_drv_ser_db_get_ram_type(lchip, ramid, 0) != DRV_FTM_MEM_TCAM)/*dynamic tbl*/
        {
            sal_memset(*p_dynamic_info, 0, mem_size);
            p_ser_db->catch_mem_dynamic_size += mem_size;
        }
        else/*tcam key*/
        {
            sal_memset(*p_dynamic_info, 0xff, mem_size);
            entry_size_aline = drv_ser_db_get_tcam_entry_size_aline(lchip, ramid);
            for (cur = 0; cur < (mem_size/entry_size_aline); cur++)
            {
                offset = (cur*entry_size_aline)/4;
                tcam_cfg.type   = DRV_MEM_DB_MASK_TCAM_BY_INIT;
                tcam_cfg.ram_id = ramid;
                tcam_cfg.p_data = ((uint32*)(*p_dynamic_info)) + offset;
                drv_ser_db_set_tcam_mask_fld(lchip, &tcam_cfg);
            }
            p_ser_db->catch_mem_tcam_key_size += mem_size;
        }
    }
    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_store_static_tbl(uint8 lchip, uint16 tbl_id, uint16 entry_idx, tbl_entry_t* p_tbl_entry)
{
    uint8* p_bucket_data = NULL;
    uint8  is_insert     = 0;
    uint32 size          = 0;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

     /*DRV_DBG_INFO("[MEM_DB]STORE static-TBL[%d]ENTRY_INDEX[%d]\r\n",tbl_id,entry_idx);*/
    if (((0 == DRV_GET_TBL_INFO_ECC(tbl_id)) && (0 == DRV_GET_TBL_INFO_DB(tbl_id)))
        || (_drv_ser_db_get_tbl_type(lchip, tbl_id) != DRV_FTM_TABLE_STATIC)
        || (TABLE_MAX_INDEX(lchip, tbl_id) == 0))
    {
        return DRV_E_NONE;
    }

    p_bucket_data = (uint8*)(p_ser_db->lookup_cb)(p_ser_db->p_static_tbl_hash, &tbl_id);
    is_insert = (NULL == p_bucket_data)?1:0;

    /*If tbl first goto:insert*/
    if (is_insert)
    {
        size = DRV_MEM_DB_TBL_ID_SIZE + TABLE_MAX_INDEX(lchip, tbl_id) * TABLE_ENTRY_SIZE(lchip, tbl_id);
        p_bucket_data = (uint8*)mem_malloc(MEM_SYSTEM_MODULE, size);
        if (p_bucket_data == NULL)
        {
            (p_ser_db->free_cb)(p_ser_db->p_static_tbl_hash);
            DRV_DBG_INFO("[MEM_DB]Insert static-table-backup memory failed.\r\n");
            return DRV_E_NO_MEMORY;
        }
        sal_memset(p_bucket_data, 0, size);
        sal_memcpy(p_bucket_data , (uint8*)&tbl_id, DRV_MEM_DB_TBL_ID_SIZE);
        p_ser_db->catch_mem_static_size += size;
        if (NULL == p_ser_db->insert_cb(p_ser_db->p_static_tbl_hash, p_bucket_data))
        {
            if (p_bucket_data)
            {
                mem_free(p_bucket_data);
            }
            (p_ser_db->free_cb)(p_ser_db->p_static_tbl_hash);
            p_ser_db->catch_mem_static_size -= size;
            return DRV_E_NO_MEMORY;
        }
    }

    /*add*/
    if (p_bucket_data)
    {
        sal_memcpy((p_bucket_data + DRV_MEM_DB_TBL_ID_SIZE + entry_idx*TABLE_ENTRY_SIZE(lchip, tbl_id)),
            p_tbl_entry->data_entry, TABLE_ENTRY_SIZE(lchip, tbl_id));
    }

    return DRV_E_NONE;

}

STATIC int32
_drv_ser_db_store_hash_cam(uint8 lchip, uint16 tbl_id, uint16 entry_idx, uint8* p_data, uint8  enable)
{
    uint32 cmd   = 0;
    uint32 index = 0;
    /*uint32 step  = 0;*/
    uint32 cam_key[12] = { 0 };
    tbl_entry_t   entry;

    if (enable != TRUE)
    {
        return DRV_E_NONE;
    }

    index = entry_idx / 4;
    /*step  = entry_idx % 4;*/

    cmd = DRV_IOR(DsFibHost0HashCam_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, cam_key));

    entry.data_entry =  cam_key;
    DRV_IF_ERROR_RETURN(_drv_ser_db_store_static_tbl(lchip, DsFibHost0HashCam_t, index, &entry));

    return DRV_E_NONE;
}




STATIC int32
_drv_ser_db_store_dynamic_tbl(uint8 lchip, uint16 tbl_id, uint16 entry_idx, tbl_entry_t* p_tbl_entry)
{
    uint8  mem_id     = 0xff;
    uint32 offset     = 0;
    uint8  entry_size = 0;
    uint8* p_temp     = NULL;
    uint8  ret        = 0;
    uint8  hash_module = 0;
    uint8  cam_type    =  0;
    uint8  cam_num    =  0;
    uint32 mem_size   = 0;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    if (TRUE == _drv_ser_db_invalid_hash_key_tbl(lchip, tbl_id))
    {
        return DRV_E_NONE;
    }

    if (DsMplsHashCamAd_t == tbl_id)/*DsMplsHashCamAd-process put-in static-tbl*/
    {
        return DRV_E_NONE;
    }

    /*if data get from acc api, need to - cam_num*/
    if (DRV_E_NONE == drv_usw_acc_get_hash_module_from_tblid(lchip, tbl_id, &hash_module))
    {
        if ((DRV_IS_DUET2(lchip) && (DRV_ACC_HASH_MODULE_FIB_HOST0 == hash_module))
            || (DRV_ACC_HASH_MODULE_FDB == hash_module))
        {
            ret += drv_usw_ftm_get_cam_by_tbl_id(tbl_id, &cam_type, &cam_num);
            if (!cam_num)
            {
                DRV_DBG_INFO("MEM_DB:can not get this tbl cam_num.\n");
            }

            if (entry_idx >= cam_num)/*is_hashkey*/
            {
                entry_idx -= cam_num;
            }
            else/*is_cam*/
            {
                DRV_IF_ERROR_RETURN(_drv_ser_db_store_hash_cam(lchip, tbl_id, entry_idx, (uint8*)p_tbl_entry->data_entry, TRUE));
                return DRV_E_NONE;
            }
        }
    }
    ret = drv_usw_ftm_table_id_2_mem_id(lchip, tbl_id, entry_idx, (uint32*)&mem_id, &offset);
    if (0 != ret)
    {
        return DRV_E_NONE;
    }

    entry_size = TABLE_ENTRY_SIZE(lchip, tbl_id);
    p_temp = p_ser_db->dynamic_tbl[mem_id];
 /*ftm api check*/
    drv_usw_get_memory_size(lchip, mem_id, &mem_size);
    if ((offset + entry_size) > mem_size)
    {
        DRV_DBG_INFO("[MEM_DB]ERROR SIZE,tbl_id[%d],tbl_index[%d],mem_size[%d],tbl_mem_offset[%d],entry_size[%d]\r\n",tbl_id,entry_idx,mem_size,offset,entry_size);
        return 0;
    }
 /*test:end*/
    sal_memcpy(p_temp + offset, (uint8*)p_tbl_entry->data_entry, entry_size);

    return DRV_E_NONE;
}

/***********************************
|| action      || p_data || p_mask ||
|| TCAM-ADD    || !NULL  || !NULL  ||
|| TCAM-REMOVE || NULL   || !NULL  ||
************************************/
STATIC int32
_drv_ser_db_store_tcam_key(uint8 lchip, uint8 mem_id, uint16 entry_idx, tbl_entry_t* p_tbl_entry)
{
    uint32 block_size = 0;
    uint32 blk_entry_size = 0;
    uint8* p_temp = NULL;
    uint8  data_entry[MAX_ENTRY_WORD] = {0};
    uint8  mask_entry[MAX_ENTRY_WORD] = {0};
    uint8  mem_type  = 0;
    uint8  mem_order = 0;
    uint8  entry_valid = 0;
    drv_ftm_mem_info_t mem_info;
    uint8 entry_size_aline = 0;
    TsEngineRefRc_m ts_ref_rc;
    uint32 cmd = DRV_IOR(TsEngineRefRc_t, DRV_ENTRY_FLAG);
    uint32 ts_s;
    uint32 ts_ns;
    uint64 total_ns = 0;
    uint32 ts_array[2] = {0};
    drv_ser_db_mask_tcam_fld_t  mask_tcam_fld;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    /*lpm tcam key need  map*/
    sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));
    mem_info.info_type = DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL;
    mem_info.mem_id    = mem_id;
    drv_ftm_get_mem_info(lchip, &mem_info);
    mem_type = mem_info.mem_type;
    mem_order = mem_info.mem_sub_id;
    if (mem_type == DRV_FTM_MEM_TCAM_LPM_KEY)
    {
        drv_ser_db_map_lpm_tcam(lchip, &mem_id, &mem_order, &entry_idx);
    }
    drv_usw_get_memory_size(lchip, mem_id, &block_size);
    drv_ser_db_get_tcam_local_entry_size(lchip, mem_id, &blk_entry_size);

    DRV_IOCTL(lchip, 0, cmd, &ts_ref_rc);
    ts_s = GetTsEngineRefRc(V, rcSecond_f, &ts_ref_rc);
    ts_ns = GetTsEngineRefRc(V, rcNs_f, &ts_ref_rc);

    /*total ns is 60 bits in dma desc, so need to truncate*/
    total_ns = (ts_s*1000LL*1000LL*1000LL+ts_ns);
    ts_array[0] = (total_ns)&0xFFFFFFFF;
    ts_array[1] = (total_ns>>32)&0x3FFFFFFF;

    /*when mask is 0,,do not store backup,but entry valid change to 1*/
    if (sal_memcmp(data_entry,((uint8*)p_tbl_entry->mask_entry),blk_entry_size) == 0)
    {
        entry_valid = 1;
        sal_memset(data_entry, 0, blk_entry_size);
        sal_memset(mask_entry, 0, blk_entry_size);
    }

    p_temp = p_ser_db->dynamic_tbl[mem_id];
    /*tcam-remove*/
    if (p_tbl_entry->data_entry == NULL)
    {
        entry_valid = 0;
        sal_memset(data_entry, 0xff, blk_entry_size);
        sal_memset(mask_entry, 0xff, blk_entry_size);
    }
    /*tcam-add*/
    else
    {
        p_ser_db->dma_scan_timestamp[mem_id][0] = ts_array[0];
        p_ser_db->dma_scan_timestamp[mem_id][1] = (ts_array[1])&0X3FFFFFFF;

        sal_memcpy(data_entry, p_tbl_entry->data_entry, blk_entry_size);
        sal_memcpy(mask_entry, p_tbl_entry->mask_entry, blk_entry_size);
        entry_valid = 1;
        /*data/mask->X/Y*/
        drv_ser_db_convert_tcam_wr_content(lchip, blk_entry_size*8, (uint32*)data_entry, (uint32*)mask_entry);
    }

    /*useless bit process*/
    mask_tcam_fld.type = DRV_MEM_DB_MASK_TCAM_BY_STORE;
    mask_tcam_fld.entry_valid = entry_valid;
    mask_tcam_fld.ram_id =  mem_id;
    mask_tcam_fld.p_data =  (uint32*)data_entry;
    mask_tcam_fld.p_mask =  (uint32*)mask_entry;
    drv_ser_db_set_tcam_mask_fld(lchip, &mask_tcam_fld);
    entry_size_aline = drv_ser_db_get_tcam_entry_size_aline(lchip, mem_id);
    /*X/Y write to backup*/
    sal_memcpy(p_temp + entry_idx*entry_size_aline, data_entry, blk_entry_size);
    sal_memcpy(p_temp + entry_idx*entry_size_aline + blk_entry_size, mask_entry, (entry_size_aline - blk_entry_size));
    
    return DRV_E_NONE;
}

STATIC uint32
_drv_ser_db_get_dma_rw_tcam_info(uint8 lchip, uint32 mem_id, uint32* p_rw_tbl, uint32* p_rw_idx, uint32* p_entry_aline_sz)
{
    uint8  mem_order = 0;
    uint8  mem_type  = 0;
    uint32  ram_entry_sz_max_aline  = 0;
    uint32  lpm_entry_sz_max_aline = 0;
    uint8   double_size = 0;

    *p_entry_aline_sz = drv_ser_db_get_tcam_entry_size_aline(lchip, mem_id);

    double_size = DRV_IS_DUET2(lchip) ? 1 : 2;
    drv_ser_db_get_mem_id_info(lchip, mem_id,&mem_order,&mem_type);

    switch (mem_type)
    {
        case DRV_FTM_MEM_TCAM_FLOW_KEY:
           *p_rw_tbl = FlowTcamTcamMem_t;
           ram_entry_sz_max_aline = 2048;
           break;
        case DRV_FTM_MEM_TCAM_LPM_KEY:
           *p_rw_tbl = LpmTcamTcamMem_t;
            ram_entry_sz_max_aline = 1024 * double_size;
           break;
        case DRV_FTM_MEM_TCAM_CID_KEY:
           *p_rw_tbl = IpeCidTcamMem_t;
            ram_entry_sz_max_aline = 128;
           break;
        case DRV_FTM_MEM_TCAM_QUEUE_KEY:
           *p_rw_tbl = QMgrEnqTcamMem_t;
            ram_entry_sz_max_aline = 256 * double_size;
           break;
        default:
           *p_rw_tbl = 0;
           *p_rw_idx = 0;
            return DRV_E_NONE;
    }
    if (mem_order == 0)
    {
        *p_rw_idx = 0;
        return DRV_E_NONE;
    }

    if (DRV_FTM_MEM_TCAM_LPM_KEY == mem_type)
    {
        lpm_entry_sz_max_aline = (mem_order < 8) ? ram_entry_sz_max_aline/2 : ram_entry_sz_max_aline;
        *p_rw_idx = (mem_order/4)*ram_entry_sz_max_aline*4 + (mem_order%4)*lpm_entry_sz_max_aline;
    }
    else
    {
        *p_rw_idx  = ram_entry_sz_max_aline*mem_order;
    }
    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_reset_register(uint8 lchip, uint8 is_recover)
{
    uint32 cmd      = 0;
    uint32 index    = 0;
    uint32 value    = 0;

    for (index = 0; (index < (sizeof(drv_ser_db_reset_hw_register)/sizeof(drv_ser_db_reset_hw_register_t))); index ++)
    {
        if(drv_ser_db_reset_hw_register[index].type == 1)
        {
            if (!is_recover)/*store*/
            {
                cmd = DRV_IOR(drv_ser_db_reset_hw_register[index].tbl_id, drv_ser_db_reset_hw_register[index].fld_id);
                DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd,  &value));
                drv_ser_db_reset_hw_register[index].old_value = value;
            }
            else/*recover*/
            {
                cmd = DRV_IOW(drv_ser_db_reset_hw_register[index].tbl_id, drv_ser_db_reset_hw_register[index].fld_id);
                value = drv_ser_db_reset_hw_register[index].old_value;
                DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd,  &value));
                continue;
            }
        }
        value = drv_ser_db_reset_hw_register[index].value;
        cmd = DRV_IOW(drv_ser_db_reset_hw_register[index].tbl_id, drv_ser_db_reset_hw_register[index].fld_id);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
     }
     return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_reset_static(uint8 lchip, uint8 enable)
{
    int32  ret           = 0;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    if ((!p_ser_db->static_tbl_backup_en) || (enable == 0))
    {
        return DRV_E_NONE;
    }

    ret = p_ser_db->travese_cb(p_ser_db->p_static_tbl_hash, _drv_ser_db_static_tbl_travese_fn, &lchip);
    if (ret)
    {
        DRV_DBG_INFO("\nFAILED! static Table reset failed!\n");
    }
    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_reset_per_ram(uint8 lchip, uint8 mem_id)
{
    uint32   i              = 0;
    uint16   entry_index    = 0;
    uint32   mem_whole_size = 0;
    uint32   mem_entry_size = 0;
    uint8*   p_mem_addr     = NULL;
    uint32*  p_mem_buffer   = NULL;
    uint32   dma_loop_time      = 0;
    uint8    is_not_dma_mem_aline = 0;
    uint32   dma_offset          = 0;
    uint32   dma_per_size        = 0;
    uint32   already_rewrite_idx = 0;
    uint32   dma_rewrite_tbl     = 0;
    uint32   entry_total_sz_aline = 0;
    uint8    temp_mem_entry_sz = 0;
    uint32   cp_mem_size_max   = 0;
    uint8    is_last_time  = 0;
    uint8 order = 0;
    uint8 type  = 0;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    if (p_ser_db->dynamic_tbl[mem_id] == NULL)
    {
        return DRV_E_NONE;
    }

    /*sharebuffer 0-3 fdb not support recover*/
    drv_ser_db_get_mem_id_info(lchip, mem_id,&order,&type);
    if ((DRV_FTM_MEM_SHARE_MEM == type) && (order < 4))
    {
        return DRV_E_NONE;
    }

    drv_usw_get_memory_size(lchip, mem_id, &mem_whole_size);
    if (mem_whole_size == 0)
    {
        return DRV_E_NONE;
    }
    p_mem_addr = p_ser_db->dynamic_tbl[mem_id];
    p_mem_buffer = (_drv_ser_db_get_ram_type(lchip, mem_id, 0) == DRV_FTM_MEM_DYNAMIC) ? (p_ser_db->p_reset_hw_alloc_mem)[0]:(p_ser_db->p_reset_hw_alloc_mem)[1] ;
    cp_mem_size_max = (_drv_ser_db_get_ram_type(lchip, mem_id, 0) == DRV_FTM_MEM_DYNAMIC) ? DRV_MEM_DB_MALLOC_CPU_MEM_SIZE : DRV_MEM_DB_MALLOC_DMA_MEM_SIZE;

    if  (_drv_ser_db_get_ram_type(lchip, mem_id, 0) == DRV_FTM_MEM_DYNAMIC)
    {
        DRV_IF_ERROR_RETURN(_drv_ser_db_get_ram_entry_size(lchip, _drv_ser_db_get_ram_type(lchip, mem_id, 1), &temp_mem_entry_sz));
        mem_entry_size = temp_mem_entry_sz;
    }
    else
    {
        _drv_ser_db_get_dma_rw_tcam_info(lchip, mem_id, &dma_rewrite_tbl, &already_rewrite_idx, &entry_total_sz_aline);
        drv_ser_db_get_tcam_local_entry_size(lchip, mem_id, &mem_entry_size);
        _drv_ser_db_tcam_get_mem_size(lchip, mem_id, &mem_whole_size);
    }
    is_not_dma_mem_aline = mem_whole_size%cp_mem_size_max ? 1:0;
    dma_loop_time = mem_whole_size/cp_mem_size_max + is_not_dma_mem_aline;
    for (i = 0; i< dma_loop_time; i++)
    {
        dma_per_size   =  0;
        is_last_time =  (i == (dma_loop_time - 1)) ? 1:0;
        dma_offset = i*cp_mem_size_max;
        dma_per_size = ((0 == is_not_dma_mem_aline) ? cp_mem_size_max : cp_mem_size_max*(!is_last_time) +is_last_time*(mem_whole_size%cp_mem_size_max));
        sal_memcpy(p_mem_buffer , p_mem_addr + dma_offset, dma_per_size);

        /*dynamic tbl recover*/
        if (_drv_ser_db_get_ram_type(lchip, mem_id, 0) == DRV_FTM_MEM_DYNAMIC)
        {
            DRV_IF_ERROR_RETURN(drv_ser_db_rewrite_dyn(lchip, mem_id, dma_offset, mem_entry_size, (uint8*)p_mem_buffer, 1, dma_per_size));
        }
        else
        {
            uint8* p_data     = NULL;
            uint8* p_mask     = NULL;
            uint8  is_empty   = 0;
            drv_ser_db_mask_tcam_fld_t mask_tcam_fld;

            /*transfer X/Y to data/mask*/
            for (entry_index = 0; entry_index < (dma_per_size/entry_total_sz_aline); entry_index++)
            {
                p_data = (uint8*)p_mem_buffer + entry_total_sz_aline*entry_index;
                p_mask = (uint8*)p_mem_buffer + entry_total_sz_aline*entry_index + mem_entry_size;
                /*mask unless bit, to get true is_empty*/
                mask_tcam_fld.type = DRV_MEM_DB_MASK_TCAM_BY_READ;
                mask_tcam_fld.p_data = (uint32*)p_data;
                mask_tcam_fld.p_mask = (uint32*)p_mask;
                mask_tcam_fld.ram_id = mem_id;
                drv_ser_db_set_tcam_mask_fld(lchip, &mask_tcam_fld);

                /*backup-memory: (Y,X) transfer to (data,mask)*/
                is_empty = 0;
                DRV_IF_ERROR_RETURN(drv_usw_chip_convert_tcam_dump_content(lchip, mem_entry_size*8, (uint32*)p_data, (uint32*)p_mask, &is_empty));

                /*rewrite is_empty to entry*/
                mask_tcam_fld.type = DRV_MEM_DB_MASK_TCAM_BY_STORE;
                mask_tcam_fld.entry_valid = (!is_empty);
                mask_tcam_fld.ram_id =  mem_id;
                mask_tcam_fld.p_data =  (uint32*)p_data;
                mask_tcam_fld.p_mask =  (uint32*)p_mask;
                drv_ser_db_set_tcam_mask_fld(lchip, &mask_tcam_fld);
            }

            /*rewrite by DMA*/
            DRV_IF_ERROR_RETURN(_drv_ser_db_write_tbl_by_dma(lchip, dma_rewrite_tbl, already_rewrite_idx, (uint32*)((uint8*)p_mem_buffer + dma_offset), dma_per_size/entry_total_sz_aline, 1));
            already_rewrite_idx += dma_per_size/entry_total_sz_aline;
        }
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_ser_db_reset_dynamic(uint8 lchip, uint8 enable)
{
    uint8 mem_id = 0;
    drv_ser_db_t* p_ser_db = NULL;

    p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    if (enable == 0)
    {
        return DRV_E_NONE;
    }


    /*malloc  memory*/
    p_ser_db->p_reset_hw_alloc_mem[0] = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, DRV_MEM_DB_MALLOC_CPU_MEM_SIZE);
    if (p_ser_db->p_reset_hw_alloc_mem[0] == NULL)
    {
        DRV_DBG_INFO("DRV_MEM_DB:No Memory.\n");
        goto FREE_MEMORY;
    }
    p_ser_db->p_reset_hw_alloc_mem[1] = g_dal_op.dma_alloc(lchip, DRV_MEM_DB_MALLOC_DMA_MEM_SIZE, 0);
    if (NULL ==  p_ser_db->p_reset_hw_alloc_mem[1])
    {
        DRV_DBG_INFO("DRV_MEM_DB:No Memory.\n");
        goto FREE_MEMORY;
    }

    sal_memset(p_ser_db->p_reset_hw_alloc_mem[0], 0, DRV_MEM_DB_MALLOC_CPU_MEM_SIZE);
    sal_memset(p_ser_db->p_reset_hw_alloc_mem[1], 0, DRV_MEM_DB_MALLOC_DMA_MEM_SIZE);

    /*reset dynamic*/
    for (mem_id = 0 ; mem_id < DRV_FTM_MAX_ID; mem_id++ )
    {
         DRV_IF_ERROR_RETURN(_drv_ser_db_reset_per_ram(lchip, mem_id));
    }
    goto FREE_MEMORY;

    /*free memory*/
FREE_MEMORY:
    if (p_ser_db->p_reset_hw_alloc_mem[0])
    {
        mem_free(p_ser_db->p_reset_hw_alloc_mem[0]);
    }
    if ((g_dal_op.dma_free) && p_ser_db->p_reset_hw_alloc_mem[1])
    {
        g_dal_op.dma_free(lchip, p_ser_db->p_reset_hw_alloc_mem[1]);
    }
    return DRV_E_NONE;
}

int32
drv_ser_db_set_stop_store(uint8 lchip, uint8 enable)
{
    drv_ser_db_t* p_ser_db = NULL;
    uint8 chip_num = 0;

    dal_get_chip_number(&chip_num);
    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }
    p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    if ((NULL == p_ser_db) || (SDK_WORK_PLATFORM == 1))
    {
        DRV_DBG_INFO("DRV_MEM_DB:can not set simluation-config,becasuse drv_mem_db do not init");
        return DRV_E_NONE;
    }

    p_ser_db->stop_store_en = enable;
    return DRV_E_NONE;
}

#define ___MEM_DB_INTERFACE___
/**
 @brief The function can set ecc do not store data.it just be useful when test.
*/

int32
drv_ser_db_store(uint8 lchip, uint32 mem_id, uint32 entry_idx, void* p_data)
{
    tbl_entry_t* p_tbl_entry = (tbl_entry_t*)p_data;

    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    if ((NULL == p_ser_db) || (SDK_WORK_PLATFORM == 1) || (NULL == p_ser_db->p_static_tbl_hash))
    {
        return DRV_E_NONE;
    }

    if (p_ser_db->stop_store_en)/*for dt2-test-simluation*/
    {
        return DRV_E_NONE;
    }

    if (!p_tbl_entry->mask_entry)
    {
        /*static tbl backup store*/
        if ((_drv_ser_db_get_tbl_type(lchip, mem_id) == DRV_FTM_TABLE_STATIC) && (p_ser_db->static_tbl_backup_en))
        {
            DRV_IF_ERROR_RETURN(_drv_ser_db_store_static_tbl(lchip, mem_id, entry_idx, p_tbl_entry));
        }
        /*dynamic tbl backup store*/
        else if ((_drv_ser_db_get_tbl_type(lchip, mem_id) == DRV_FTM_TABLE_DYNAMIC) && (p_ser_db->dynamic_tbl_backup_en))
        {
            DRV_IF_ERROR_RETURN(_drv_ser_db_store_dynamic_tbl(lchip, mem_id, entry_idx, p_tbl_entry));
        }
    }
    else/*DRV_FTM_TABLE_TCAM_KEY*/
    {
        if (p_ser_db->dynamic_tbl_backup_en)
        {
            DRV_IF_ERROR_RETURN(_drv_ser_db_store_tcam_key(lchip, mem_id, entry_idx, p_tbl_entry));
        }
    }
    return DRV_E_NONE;
}

int32
drv_ser_db_set_hw_reset_en(uint8 lchip, uint8 enable)
{
    uint32 i      = 0;
    uint8 chip_num = 0;
    int32 ret = 0;

    drv_ser_db_t* p_ser_db = NULL;
    dal_get_chip_number(&chip_num);
    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }

    p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    if ((NULL == p_ser_db) || (0 == enable))
    {
        return DRV_E_NONE;
    }

    DRV_IF_ERROR_RETURN(_drv_ser_db_reset_register(lchip, 0));

    p_ser_db->db_reset_hw_doing = enable ? 1:0;
    DRV_IF_ERROR_RETURN(drv_ser_db_set_stop_store(lchip, enable));

    /*module special process*/
    for (i = 0; i< DRV_SER_HW_RESET_CB_TYPE_NUM; i++)
    {
        if (p_ser_db->reset_hw_cb[i] != NULL)
        {
            if ((i == DRV_SER_HW_RESET_CB_TYPE_FDB) || (i == DRV_SER_HW_RESET_CB_TYPE_MAC))/*reset at end*/
            {
                continue;
            }
            ret = (p_ser_db->reset_hw_cb[i](lchip, NULL));
        }
    }


    DRV_DBG_INFO("\n[DB]Start chip tbl recover \n");
    /*static tbl recover: based on tbl_id*/
    ret = _drv_ser_db_reset_static(lchip, enable);

    /*dynamic and tcam tbl recover:based on ramid + offset*/
    ret = _drv_ser_db_reset_dynamic(lchip, enable);

    ret = _drv_ser_db_reset_register(lchip, 1);

    if (p_ser_db->reset_hw_cb[ DRV_SER_HW_RESET_CB_TYPE_FDB])
    {
        ret= p_ser_db->reset_hw_cb[ DRV_SER_HW_RESET_CB_TYPE_FDB](lchip, NULL);
    }

    if (p_ser_db->reset_hw_cb[ DRV_SER_HW_RESET_CB_TYPE_MAC])
    {
        ret = p_ser_db->reset_hw_cb[ DRV_SER_HW_RESET_CB_TYPE_MAC](lchip, NULL);
    }

    p_ser_db->db_reset_hw_doing  = 0;
    DRV_IF_ERROR_RETURN(drv_ser_db_set_stop_store(lchip, 0));

    DRV_DBG_INFO("\n[DB]End chip tbl recover \n");
    return ret;
}

int32
drv_ser_db_init(uint8 lchip ,drv_ser_global_cfg_t* p_cfg)
{
    int ret = 0;
    uint32 cmd = 0;
    uint32 buffer[32] = {0};
    uint16 tbl_id = 0;
    uint32 entry_num = 0;
    uint32 entry_max_size = 0;
    drv_ser_db_t* p_ser_db = NULL;

    DRV_PTR_VALID_CHECK(p_cfg);

    if ((SDK_WORK_PLATFORM == 1) || ( p_drv_master[lchip]->p_ser_db))
    {
        return DRV_E_NONE;
    }

    /*overall control*/
    if (!p_cfg->hw_reset_en)
    {
        return DRV_E_NONE;
    }

    p_ser_db = (drv_ser_db_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(drv_ser_db_t));
    p_drv_master[lchip]->p_ser_db = p_ser_db;
    if (!p_ser_db)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset(p_ser_db, 0, sizeof(drv_ser_db_t));

    p_ser_db->drv_ser_db_en = p_cfg->ecc_recover_en | p_cfg->hw_reset_en;
    p_ser_db->static_tbl_backup_en  =  p_cfg->ecc_recover_en | p_cfg->hw_reset_en;
    p_ser_db->dynamic_tbl_backup_en =  p_cfg->ecc_recover_en | p_cfg->hw_reset_en;
    p_ser_db->tcam_key_backup_en    =  p_cfg->tcam_scan_en | p_cfg->hw_reset_en;

    p_ser_db->dma_rw_cb   =  p_cfg->dma_rw_cb;
    p_ser_db->create_cb   =  p_cfg->create_cb;
    p_ser_db->insert_cb   =  p_cfg->insert_cb;
    p_ser_db->free_cb     =  p_cfg->free_cb;
    p_ser_db->lookup_cb   =  p_cfg->lookup_cb;
    p_ser_db->travese_cb  =  p_cfg->travese_cb;

    /*static tbl*/
    if (p_ser_db->static_tbl_backup_en)
    {
        DRV_IF_ERROR_RETURN(_drv_ser_db_init_static_tbl(lchip));
    }

    /*dynamic tbl & tcam key*/
    if ((p_ser_db->dynamic_tbl_backup_en) || (p_ser_db->tcam_key_backup_en))
    {
        DRV_IF_ERROR_RETURN(_drv_ser_db_init_dynamic_tbl(lchip));
    }

    /*step2: get data of static table and rewrite to sram memory*/
    for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++ )
    {
        if (DRV_GET_TBL_INFO_RWRITE(tbl_id) == 0)
        {
            continue;
        }
        entry_max_size = TABLE_MAX_INDEX(lchip, tbl_id);
        for (entry_num = 0; entry_num < entry_max_size; entry_num++)
        {
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, entry_num, cmd, buffer);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, entry_num, cmd, buffer);
            sal_memset(buffer, 0, 32*sizeof(uint32));
        }
    }
    return ret;
}

int32
drv_ser_db_deinit(uint8 lchip)
{
    drv_ser_db_t* p_ser_db = NULL;

    p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    if ((SDK_WORK_PLATFORM == 1) || (!p_ser_db))
    {
        return DRV_E_NONE;
    }


    /*static tbl free*/
    if (p_ser_db->p_static_tbl_hash)
    {
        (p_ser_db->free_cb)(p_ser_db->p_static_tbl_hash);
    }

    /*dynmic tbl free*/
    _drv_ser_db_rollback_dynamic_tbl(lchip, DRV_FTM_MAX_ID);

    /*master free*/
    mem_free(p_ser_db);
    p_ser_db = NULL;

    return DRV_E_NONE;
}
