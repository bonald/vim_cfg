/****************************************************************************
 * @date 2018-02-23  The file contains all ecc soft error  recover realization.
 *
 * Copyright:    (c)2017 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     v0.1
 * Date:         2018-02-23
 * Reason:       Create for USW v3.0
 ****************************************************************************/
#include "sal.h"
#include "usw/include/drv_io.h"
#include "usw/include/drv_app.h"
#include "usw/include/drv_ser.h"
#include "usw/include/drv_ser_db.h"
#include "usw/include/drv_ftm.h"

#define DRV_ECC_LPM_AD_BLOCK_NUM 2
#define DRV_ECC_LPM_KEY_BLOCK_NUM 4
#define DRV_ECC_MPLS_HS_KEY_ECC_SIZE  64
#define DRV_ECC_SHARE_BUFFER_ECC_SIZE 48
#define DRV_ECC_ACC_TBL_MIN_SIZE      12
#define DRV_UINT32_BITS  32
#define DRV_SER_GRANULARITY   10000   /* ns*/
#define _DRV_BMP_OP(_bmp, _offset, _op)     \
    (_bmp[(_offset) / DRV_UINT32_BITS] _op(1U << ((_offset) % DRV_UINT32_BITS)))

#define DRV_BMP_ISSET(bmp, offset)          \
    _DRV_BMP_OP((bmp), (offset), &)

extern int32 dal_get_chip_number(uint8* p_num);

uint8 g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_NUM];

enum drv_ecc_tbl_type_e
{
    DRV_ECC_TBL_TYPE_UNKNOWN = 0,
    DRV_ECC_TBL_TYPE_STATIC,
    DRV_ECC_TBL_TYPE_HASH_KEY,
    DRV_ECC_TBL_TYPE_DYNAMIC_SRAM = 3,
    DRV_ECC_TBL_TYPE_NUM
};
typedef enum drv_ecc_tbl_type_e drv_ecc_tbl_type_t;


enum drv_ecc_recover_action_e
{
    DRV_ECC_RECOVER_ACTION_NONE,
    DRV_ECC_RECOVER_ACTION_REMOVE,
    DRV_ECC_RECOVER_ACTION_OVERWRITE,
    DRV_ECC_RECOVER_ACTION_NUM
};
typedef enum drv_ecc_recover_action_e drv_ecc_recover_action_t;


struct drv_ser_scan_thread_s
{
    uint16      prio;
    sal_task_t* p_scan_task;
};
typedef struct drv_ser_scan_thread_s drv_ser_scan_thread_t;

struct drv_ser_master_s
{
    uint32                 recover_static_cnt;
    uint32                 recover_dynamic_cnt;
    uint32                 recover_tcam_key;
    uint32                 recover_sbe_cnt;
    uint32                 ignore_cnt;

    drv_ser_event_fn       ser_event_cb;
    uint32                 sbe_scan_en    :1;
    uint32                 tcam_scan_en   :1;
    uint32                 ecc_recover_en :1;
    uint32                 remove_free_tcam_en :1; /**< if set,will remove free tcam entry in a scan period  */
    uint32                 simulation_en :1;
    uint32                 err_rec_offset :18;     /*if simulation_en set 1, ECC table pointer to err_rec_offset */
    uint32                 rsv :8;

    uint32                 tcam_scan_burst_interval;             /* unit is ms */
    uint32                 tcam_scan_burst_entry_num;            /* acl/scl/ip unit is 160bit, nat/pbr unit is 40bit */
    uint32                 scan_interval[DRV_SER_SCAN_TYPE_NUM]; /* unit is ms, 0 means disable scan thread*/
    uint8                  scan_doing[DRV_SER_SCAN_TYPE_NUM];
    drv_ser_scan_thread_t  scan_thread[DRV_SER_SCAN_TYPE_NUM];
};
typedef struct drv_ser_master_s drv_ser_master_t;


STATIC uint32
_drv_ser_get_sharebuf_stats(uint8 lchip)
{
    uint32    cmd = 0;
    uint32    dynamic_used = 0;

    cmd = DRV_IOR(ShareBufferCtl_t, ShareBufferCtl_cfgShareTableEn_f);
    DRV_IOCTL(lchip, 0, cmd, &dynamic_used);

    return dynamic_used;
}

STATIC void
drv_ser_report_ser_event_cb(uint8 lchip, drv_ser_cb_info_t* p_ecc_cb)
{
    uint8 gchip = 0;
    drv_ser_master_t* p_drv_ser_master = NULL;
    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);

    drv_get_gchip_id(lchip, &gchip);

    if (NULL != p_drv_ser_master->ser_event_cb)
    {
        p_drv_ser_master->ser_event_cb(gchip, p_ecc_cb);
    }

    return;
}

STATIC int32
_drv_ser_get_dyn_entry_size(uint8 lchip, uint8 ram_type, uint8* p_entry_size)
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

STATIC int32
_drv_ecc_map_lpm_ad(uint8 lchip, uint8* p_mem_id, uint8* p_mem_order, uint16* p_entry_idx)
{
    uint8 per_blk_entry_size = 0;
    uint32  per_blk_mem_size = 0;
    uint32 per_blk_entry_index = 0;
    uint8 i = 0;
    uint8 mem_id = *p_mem_id;
    uint8 mem_order = *p_mem_order;
    uint8 stat_mem_id = mem_id - mem_order;
    uint32 entry_index_all = 0;

    for (i = 0; i < DRV_ECC_LPM_AD_BLOCK_NUM; i++)
    {
        drv_usw_get_memory_size(lchip, stat_mem_id + DRV_ECC_LPM_AD_BLOCK_NUM*mem_order + i, &per_blk_mem_size);
        _drv_ser_get_dyn_entry_size(lchip, DRV_FTM_MEM_TCAM_LPM_AD, &per_blk_entry_size);
        if ((per_blk_mem_size == 0) || (per_blk_entry_size == 0))
        {
            return DRV_E_INVALID_MEM;
        }

        per_blk_entry_index = per_blk_mem_size / per_blk_entry_size;

        if (*p_entry_idx < entry_index_all + per_blk_entry_index)
        {
            *p_mem_id  =  stat_mem_id + DRV_ECC_LPM_AD_BLOCK_NUM*mem_order + i;
            *p_mem_order = DRV_ECC_LPM_AD_BLOCK_NUM*mem_order + i;
            *p_entry_idx = (*p_entry_idx) - entry_index_all;
            return  DRV_E_NONE;
        }
        entry_index_all += per_blk_entry_index;
    }

    return  DRV_E_NONE;
}

STATIC int32
_drv_ser_restore_static_tbl(uint8 lchip ,drv_ecc_intr_tbl_t* p_intr_info, drv_ser_cb_info_t* p_ecc_cb)
{
    uint32    data[32] = {0};
    uint32    cmd = 0;
    uint32    err_rec = 0;
    uint32    rec_idx_f = 0;
    uint16    err_tbl_id = 0;
    uint8*    p_bucket_data = NULL;
    uint32    ecc_tbl_idx = 0;
    drv_ser_master_t* p_drv_ser_master = NULL;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    p_drv_ser_master = p_drv_master[lchip]->p_ser_master;
    err_tbl_id  =   p_intr_info->mem_id;
    err_rec     =   p_intr_info->rec_id;
    rec_idx_f   =   p_intr_info->rec_id_fld;

    /*get the err_table_index*/
    cmd = DRV_IOR(err_rec, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, data);
    DRV_GET_FIELD(lchip,err_rec, rec_idx_f, data, &ecc_tbl_idx);

    /*for test*/
    if (p_drv_ser_master->simulation_en)
    {
        ecc_tbl_idx = p_drv_ser_master->err_rec_offset;
    }

    /*deal with no-backup-tbl*/
    if (!DRV_GET_TBL_INFO_ECC(err_tbl_id))
    {
        p_ecc_cb->recover  = 0;
        (p_drv_ser_master->ignore_cnt)++;
    }
    else
    {
        p_bucket_data = (p_ser_db->lookup_cb)(p_ser_db->p_static_tbl_hash, &err_tbl_id);
        if (p_bucket_data)
        {
            /*recover*/
            p_bucket_data += DRV_MEM_DB_TBL_ID_SIZE + ecc_tbl_idx*(TABLE_ENTRY_SIZE(lchip, err_tbl_id));
            drv_usw_chip_sram_tbl_write(lchip, (tbls_id_t)err_tbl_id, ecc_tbl_idx, (uint32*)p_bucket_data);
            p_ecc_cb->recover = 1;
            (p_drv_ser_master->recover_static_cnt)++;
        }
        else
        {
            /*if can not get this tbl in backup-data*/
            p_ecc_cb->recover  = 0;
            (p_drv_ser_master->ignore_cnt)++;
        }

    }

    p_ecc_cb->tbl_id     = p_intr_info->mem_id;
    p_ecc_cb->tbl_index  = ecc_tbl_idx;
    p_ecc_cb->type       = p_intr_info->err_type;
    p_ecc_cb->action     = p_intr_info->action;

    return DRV_E_NONE;

}

STATIC int32
_drv_ecc_restore_dynamic_sram(uint8 lchip ,drv_ecc_intr_tbl_t* p_intr_info, drv_ser_cb_info_t* p_ecc_cb)
{
    drv_ser_master_t* p_drv_ser_master = NULL;
    uint8*    p_mem_addr               = NULL;
    uint32    cmd             = 0;
    uint32    err_rec         = 0;
    uint32    rec_idx_f       = 0;
    uint8     err_mem_id      = 0;
    uint8     err_mem_type    = 0;
    uint8     err_mem_order   = 0;
    uint32    err_entry_index = 0;
    uint8     err_entry_size  = 0;
    uint32    err_mem_offset  = 0;
    uint8     buffer1[128]    = {0};
    uint8     buffer2[128]    = {0};
    drv_ftm_mem_info_t mem_info;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    p_drv_ser_master = p_drv_master[lchip]->p_ser_master;
    err_mem_id  =   p_intr_info->mem_id;
    err_rec     =   p_intr_info->rec_id;
    rec_idx_f   =   p_intr_info->rec_id_fld;

    if (!p_drv_ser_master->simulation_en)
    {
        /*get the err_table_index*/
        cmd = DRV_IOR(err_rec, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, buffer2);
        DRV_GET_FIELD(lchip,err_rec, rec_idx_f, buffer2, &err_entry_index);
    }
    else
    {
         err_entry_index = p_drv_ser_master->err_rec_offset;
    }
    sal_memset(buffer2, 0, 128);
    sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));
    mem_info.info_type = DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL;
    mem_info.mem_id    = err_mem_id;
    DRV_IF_ERROR_RETURN(drv_ftm_get_mem_info(lchip, &mem_info));
    err_mem_type = mem_info.mem_type;
    err_mem_order = mem_info.mem_sub_id;

    /*sharebuffer not used for dynamic will not recover*/
    if ((err_mem_type == DRV_FTM_MEM_SHARE_MEM) && (0 == _drv_ser_get_sharebuf_stats(lchip)))
    {
        p_ecc_cb->tbl_id     = 0xFF000000 + err_mem_id;
        p_ecc_cb->tbl_index  = err_mem_offset;
        p_ecc_cb->type       = p_intr_info->err_type;
        p_ecc_cb->action     = p_intr_info->action;
        p_ecc_cb->tbl_id |= (err_entry_size) << 16;
        return DRV_E_NONE;   
    }
    
    if (err_mem_type == DRV_FTM_MEM_TCAM_LPM_AD)/*lpm ad need map mem_id and entry index*/
    {
        _drv_ecc_map_lpm_ad(lchip, &err_mem_id, &err_mem_order, (uint16*)&err_entry_index);
    }

    /*get backup data*/
    p_mem_addr = p_ser_db->dynamic_tbl[err_mem_id];
    DRV_IF_ERROR_RETURN(_drv_ser_get_dyn_entry_size(lchip, err_mem_type, &err_entry_size));
    err_mem_offset = err_entry_size * err_entry_index;
    sal_memcpy(buffer1, p_mem_addr + err_mem_offset, err_entry_size);

    /*Invalid tbl,such as aging/ipfix tbl:do not recover*/
    if (sal_memcmp(buffer1, buffer2, err_entry_size) == 0)
    {
        p_ecc_cb->recover = 0;
        (p_drv_ser_master->ignore_cnt)++;
    }
    else/*rewrite*/
    {
        DRV_IF_ERROR_RETURN(drv_ser_db_rewrite_dyn(lchip, err_mem_id, err_mem_offset, err_entry_size, buffer1, 0, 0));
        p_ecc_cb->recover = 1;
        (p_drv_ser_master->recover_dynamic_cnt)++;
    }

    p_ecc_cb->tbl_id     = 0xFF000000 + err_mem_id;
    p_ecc_cb->tbl_index  = err_mem_offset;
    p_ecc_cb->type       = p_intr_info->err_type;
    p_ecc_cb->action     = p_intr_info->action;
    p_ecc_cb->tbl_id |= (err_entry_size) << 16;
    return DRV_E_NONE;

}


#define ___ECC_RECOVER_SCAN___

STATIC void
_drv_ser_scan_sbe_thread(void* param)
{
    uintptr lchip = (uintptr)param;
    drv_ser_cb_info_t  ser_cb_info;
    uint32 i = 0;
    uint32 cmd = 0;
    uint32 val = 0;
    uint32 data[32] = {0};
    drv_ecc_sbe_cnt_t*  p_sbe_cnt = NULL;
    /*char* p_time_str = NULL;
    sal_time_t tm;*/
    drv_ser_master_t* p_drv_ser_master = NULL;

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    sal_task_set_priority(p_drv_ser_master->scan_thread[DRV_SER_SCAN_TYPE_SBE].prio);

    /*sal_time(&tm);
    p_time_str = sal_ctime(&tm);
    DRV_DBG_INFO("\nStart single bit error scan thread time: %s", p_time_str);*/

    p_sbe_cnt = p_drv_master[lchip]->drv_ecc_data.p_sbe_cnt;
    do
    {
        for (i = 0; (MaxTblId_t != p_sbe_cnt[i].tblid) || (NULL != p_sbe_cnt[i].p_tbl_name); i++)
        {
            if (g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_SBE] == 2)
            {
                  /*stop scan break the thread*/
                  p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_SBE]  = 0;
                  p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_SBE] = 0;
                  /*DRV_DBG_INFO("\nStop single bit error scan ,scan stop time: %s", p_time_str);*/
                  return;
            }

            p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_SBE] = 1;

            cmd = DRV_IOR(p_sbe_cnt[i].rec, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, data);
            DRV_GET_FIELD(lchip,p_sbe_cnt[i].rec, p_sbe_cnt[i].fld, data, &val);
            /*DRV_DBG_INFO("\nscan sbe tbl:%d,fld:%d,cnt:%d\n", p_sbe_cnt[i].tblid, p_sbe_cnt[i].fld,val);*/
            if (val)
            {
                ser_cb_info.tbl_id = p_sbe_cnt[i].tblid;
                ser_cb_info.type = DRV_SER_TYPE_SBE;
                ser_cb_info.tbl_index = 0xFFFFFFFF;
                ser_cb_info.recover = 1;
                ser_cb_info.action = DRV_SER_ACTION_LOG;
                drv_ser_report_ser_event_cb(lchip, &ser_cb_info);
                (p_drv_ser_master->recover_sbe_cnt)++;
            }
        }

        p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_SBE] = 0;

        if (g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_SBE] == 1)
        {
            sal_task_sleep(p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_SBE]* 60 *1000);
        }
        else
        {
            break;
        }
    }while(1);
    p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_SBE] = 0;

    /*sal_time(&tm);
    p_time_str = sal_ctime(&tm);
    DRV_DBG_INFO("\nSbe scan thread exist time: %s", p_time_str);*/

    return;
}
#if 0
STATIC void
_drv_ser_scan_tcam_thread(void* param)
{
    uintptr lchip = (uintptr)param;
    uint32 i = 0, j = 0, key_index = 0;
    uint32 cmd = 0, per_entry_bytes = 0;
    uint32 entries_interval = 0;
    uint32 data[24] = {0};
    uint32 mask[24] = {0};
    tbl_entry_t tcam_key;
    char* p_time_str = NULL;
    sal_time_t tm;
    drv_ser_master_t* p_drv_ser_master = NULL;
    uint16  (*p_scan_tcam_tbl)[5] = NULL;

    p_scan_tcam_tbl  = p_drv_master[lchip]->drv_ecc_data.p_scan_tcam_tbl;
    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    tcam_key.data_entry = data;
    tcam_key.mask_entry = mask;

    sal_time(&tm);
    p_time_str = sal_ctime(&tm);
    DRV_DBG_INFO("\nStart tcam scan thread time: %s", p_time_str);
    do
    {
        entries_interval = 0;
        for (i = 0; p_scan_tcam_tbl[i][0] != MaxTblId_t; i++)
        {
            drv_ser_db_get_tcam_local_entry_size(lchip, p_scan_tcam_tbl[i][0], &per_entry_bytes);

            for (j = 1; (j < 5) && (MaxTblId_t != p_scan_tcam_tbl[i][j]); j++)
            {
                cmd = DRV_IOR(p_scan_tcam_tbl[i][j], DRV_ENTRY_FLAG);
                for (key_index = 0; key_index < TABLE_MAX_INDEX(lchip, p_scan_tcam_tbl[i][j]); key_index++)
                {
                    if (g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_TCAM] == 2)
                    {
                        /*stop scan break the thread*/
                        p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_TCAM]  = 0;
                        p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_TCAM] = 0;
                        DRV_DBG_INFO("\nStop tcam bit error scan ,scan stop time: %s", p_time_str);
                        return;
                    }

                    p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_TCAM] = 1;
                    DRV_IOCTL(lchip, key_index, cmd, &tcam_key);
                    entries_interval += (TCAM_KEY_SIZE(lchip, p_scan_tcam_tbl[i][j]) / per_entry_bytes);
                    /*DRV_DBG_INFO("\nscan tcam tbl:%d,index:%d\n", p_scan_tcam_tbl[i][j],key_index);*/
                    if (entries_interval >= p_drv_ser_master->tcam_scan_burst_entry_num)
                    {
                        p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_TCAM] = 0;
                        sal_task_sleep(p_drv_ser_master->tcam_scan_burst_interval);
                        entries_interval = 0;
                    }
                }
            }
        }
        p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_TCAM] = 0;
        if (g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_TCAM] == 1)
        {
            sal_task_sleep(p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_TCAM]* 60 *1000);
        }
        else
        {
            break;
        }
    }while(1);
    p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_TCAM] = 0;

    sal_time(&tm);
    p_time_str = sal_ctime(&tm);
    DRV_DBG_INFO("\nTcam scan thread exist time: %s", p_time_str);

    return;
}
#endif
/**
*mode   *scan_interval * meanings                                *
*0      *-            * scan once                                *
*1      *n             * loop scan,interval time:n (mins)        *
*2      *-             * stop scan or no scan enable             *
**/
int32
drv_ser_set_mem_scan_mode(uint8 lchip, drv_ser_scan_type_t type,  uint8 mode, uint32 scan_interval, uint64 cpu_mask)
{
    drv_ser_master_t* p_drv_ser_master = NULL;
    uint8 chip_num = 0;
    uintptr chip_id = lchip;
    sal_task_t tmp_task;

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);

    if ((!p_drv_ser_master) || (SDK_WORK_PLATFORM == 1)
         || ((DRV_SER_SCAN_TYPE_TCAM == type) && (0 == p_drv_ser_master->tcam_scan_en))
         || ((DRV_SER_SCAN_TYPE_SBE == type) && (0 == p_drv_ser_master->sbe_scan_en)))
    {
        return DRV_E_NONE;
    }

    dal_get_chip_number(&chip_num);

    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }

    if (mode > 2)
    {
        return DRV_E_INVALID_PARAMETER;
    }

    if (mode != 1)
    {
       scan_interval = (mode == 0)?1:0;
    }

    g_drv_ecc_scan_mode[type] = mode;

    if(DRV_SER_SCAN_TYPE_TCAM == type)
    {
        p_drv_ser_master->scan_interval[type]= scan_interval;
        return 0;
    }
    if (scan_interval && ((p_drv_ser_master->scan_interval[type]) == 0 ))
    {
        p_drv_ser_master->scan_interval[type]= scan_interval;

        sal_memset(&tmp_task, 0, sizeof(sal_task_t));
        tmp_task.cpu_mask = cpu_mask;
        tmp_task.task_type = SAL_TASK_TYPE_ECC_SCAN;
        sal_sprintf(tmp_task.name, "%d %s", lchip,  "DrvSbeScan");
        p_drv_ser_master->scan_thread[type].p_scan_task = &tmp_task;

        sal_task_create(&p_drv_ser_master->scan_thread[type].p_scan_task,
                        tmp_task.name,
                        SAL_DEF_TASK_STACK_SIZE,
                        SAL_TASK_PRIO_DEF,
                        _drv_ser_scan_sbe_thread,
                        (void*)chip_id);
    }

    return DRV_E_NONE;
}

int32
drv_ser_get_mem_scan_mode(uint8 lchip ,drv_ser_scan_type_t type, uint8* p_mode, uint32* p_scan_interval)
{
    drv_ser_master_t* p_drv_ser_master = NULL;

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);

    /*uml do not init ecc*/
    if (SDK_WORK_PLATFORM == 1)
    {
        return DRV_E_NONE;
    }

    DRV_PTR_VALID_CHECK(p_mode);
    DRV_PTR_VALID_CHECK(p_scan_interval);

    if (NULL == p_drv_ser_master)
    {
        return DRV_E_NONE;
    }

    if (DRV_SER_SCAN_TYPE_TCAM == type)
    {
        *p_scan_interval = p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_TCAM];
        *p_mode = g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_TCAM];
    }

    if (DRV_SER_SCAN_TYPE_SBE == type)
    {
        *p_scan_interval = p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_SBE];
        *p_mode = g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_SBE];
    }

    return DRV_E_NONE;
}

uint8
drv_ser_get_tcam_scan_enable(uint8 lchip)
{
    drv_ser_master_t* p_drv_ser_master = NULL;

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);

    if ((!p_drv_ser_master) || (SDK_WORK_PLATFORM == 1))
    {
        return 0;
    }

    if(p_drv_ser_master->tcam_scan_en)
    {
        return 1;
    }

    return 0;
}

#define ___ECC_RECOVER_INTERFACE___
/**
 @brief The function get interrupt bits's field id and action type
*/
int32
drv_ser_get_intr_info(uint8 lchip, drv_ecc_intr_tbl_t* p_tbl_db_info, uint8 mode)
{
    uint16 tbl_id = 0;
    drv_ecc_intr_tbl_t* p_intr_tbl = NULL;

    DRV_PTR_VALID_CHECK(p_tbl_db_info);

    p_intr_tbl = p_drv_master[lchip]->drv_ecc_data.p_intr_tbl;
    if (!p_intr_tbl)
    {
        return DRV_E_NONE;
    }

    if (mode == 0)
    {
        for (tbl_id = 0; p_intr_tbl[tbl_id].intr_tblid != MaxTblId_t; tbl_id++)
        {
            if ((p_intr_tbl[tbl_id].intr_tblid == p_tbl_db_info->intr_tblid)
                && (p_intr_tbl[tbl_id].intr_bit == p_tbl_db_info->intr_bit))
            {
                p_tbl_db_info->action = p_intr_tbl[tbl_id].action;
                p_tbl_db_info->err_type = p_intr_tbl[tbl_id].err_type;
                p_tbl_db_info->rec_id = p_intr_tbl[tbl_id].rec_id;
                p_tbl_db_info->rec_id_fld = p_intr_tbl[tbl_id].rec_id_fld;
                p_tbl_db_info->mem_id = p_intr_tbl[tbl_id].mem_id;
                p_tbl_db_info->tbl_type = p_intr_tbl[tbl_id].tbl_type;

                return DRV_E_NONE;
            }
            continue;
        }

        p_tbl_db_info->action = 0;
        p_tbl_db_info->tbl_type = 0;
        return DRV_E_NONE;
    }
    else
    {
        for (tbl_id = 0; p_intr_tbl[tbl_id].intr_tblid != MaxTblId_t; tbl_id++)
        {
            if ((p_intr_tbl[tbl_id].mem_id == p_tbl_db_info->mem_id) &&
                (p_intr_tbl[tbl_id].tbl_type ==  p_tbl_db_info->tbl_type))
            {
                p_tbl_db_info->intr_tblid  = p_intr_tbl[tbl_id].intr_tblid;
                p_tbl_db_info->intr_fld    = p_intr_tbl[tbl_id].intr_fld;
                return DRV_E_NONE;
            }
            continue;
        }
        return DRV_E_NONE;
    }
}

/**
 @brief The function compare cached tcam data/mask with data/mask read from hw,
        if compare failed, recover from cached data/mask and send tcam error event log.
*/
int32
drv_ser_dma_recover_tcam(uint8 lchip, drv_ser_dma_tcam_param_t* p_param)
{
    uint32 entry_num = 0;
    uint8* p_dma_data = NULL;
    uint8* p_dma_mask = NULL;
    uint32* time_stamp = NULL;
    uint32* p_memory = NULL;
    /*uint32  dma_entry_size = 0;*/
    uint32  mem_id = 0;
    uint16  sub_mem_id = 0;
    uint8* p_mem_addr = NULL;
    uint8* p_cpu_data     = NULL;
    uint8* p_cpu_mask     = NULL;
    uint32 blk_entry_size  = 0;
    uint32 buf_bk_x[8]     = {0};
    uint32 buf_bk_y[8]     = {0};
    uint8  entry_len_aline = 0;
    uint32 entry_idx       = 0;
    uint32 ram_data_size = 0;
    uint8  bk_empty = 0;
    tbl_entry_t buffer_entry;
    drv_ftm_mem_info_t mem_info;
    drv_ser_cb_info_t  ser_cb_info;
    drv_ser_db_mask_tcam_fld_t mask_tcam_data;
    drv_ser_master_t* p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    if ((0 == drv_ser_get_tcam_scan_enable(lchip)) || (p_ser_db == NULL) || (p_drv_ser_master == NULL))
    {
        return DRV_E_NONE;
    }
    if(!p_param)
    {
        return DRV_E_INVALID_POINT;
    }

    mem_id = p_param->mem_id;
    time_stamp = p_param->time_stamp;
    p_memory = p_param->p_memory;
    /*dma_entry_size = p_param->entry_size;*/
    sub_mem_id = p_param->sub_mem_id;

    if((time_stamp[1] <= p_ser_db->dma_scan_timestamp[mem_id][1]) || \
        ((time_stamp[1] == p_ser_db->dma_scan_timestamp[mem_id][1]) && (time_stamp[0]-DRV_SER_GRANULARITY) <= p_ser_db->dma_scan_timestamp[mem_id][0]))
    {
        return 0;
    }

    sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));
    sal_memset(&ser_cb_info, 0, sizeof(drv_ser_cb_info_t));
    sal_memset(&mask_tcam_data, 0, sizeof(drv_ser_db_mask_tcam_fld_t));
    drv_usw_get_memory_size(lchip, mem_id, &ram_data_size);
    drv_ser_db_get_tcam_local_entry_size(lchip, mem_id, &blk_entry_size);
    entry_num = sub_mem_id ? (ram_data_size/blk_entry_size)/2 : (ram_data_size/blk_entry_size);
    entry_len_aline = drv_ser_db_get_tcam_entry_size_aline(lchip, mem_id);
    p_mem_addr = p_ser_db->dynamic_tbl[mem_id];
    for (entry_idx = 0; entry_idx < entry_num; entry_idx++)
    {
        p_cpu_data = p_mem_addr + entry_len_aline*entry_idx + sub_mem_id*entry_len_aline*entry_num;
        p_cpu_mask = p_cpu_data + blk_entry_size;
        sal_memcpy((uint8*)buf_bk_x, p_cpu_data, blk_entry_size);
        sal_memcpy((uint8*)buf_bk_y, p_cpu_mask, blk_entry_size);

        /*mask useless part*/
        sal_memset(&mask_tcam_data, 0, sizeof(drv_ser_db_mask_tcam_fld_t));
        mask_tcam_data.type = DRV_MEM_DB_MASK_TCAM_BY_READ;
        mask_tcam_data.p_data = buf_bk_x;
        mask_tcam_data.p_mask = buf_bk_y;
        mask_tcam_data.ram_id = mem_id;
        drv_ser_db_set_tcam_mask_fld(lchip, &mask_tcam_data);

        p_dma_data = (uint8*)p_memory+entry_idx*entry_len_aline;
        p_dma_mask = p_dma_data + blk_entry_size;

        /*COMPARE*/
        if ((sal_memcmp(p_dma_data, buf_bk_x, blk_entry_size) == 0)
        && (sal_memcmp(p_dma_mask, buf_bk_y, blk_entry_size) == 0))
        {
            continue;
        }
        /*RECOVER*/

        /*backup-memory: (Y,X) transfer to (data,mask)*/
        drv_usw_chip_convert_tcam_dump_content(lchip, blk_entry_size*8, buf_bk_x, buf_bk_y, &bk_empty);

        buffer_entry.data_entry = (uint32*)buf_bk_x;
        buffer_entry.mask_entry = (uint32*)buf_bk_y;

        ser_cb_info.type = DRV_SER_TYPE_TCAM_ERROR;
        ser_cb_info.tbl_id = 0xFE000000 + mem_id;
        ser_cb_info.tbl_index =  blk_entry_size*entry_idx;
        ser_cb_info.tbl_id |= blk_entry_size << 20;

        sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));
        mem_info.info_type = DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL;
        mem_info.mem_id    = mem_id;
        drv_ftm_get_mem_info(lchip, &mem_info);
        drv_ser_db_rewrite_tcam(lchip, mem_id, mem_info.mem_type, mem_info.mem_sub_id, entry_idx, (void*)&buffer_entry);
        ser_cb_info.recover = 1;
        (p_drv_ser_master->recover_tcam_key)++;
        ser_cb_info.action = DRV_SER_ACTION_LOG;
        drv_ser_report_ser_event_cb(lchip, &ser_cb_info);

    }

    return DRV_E_NONE;
}

/**
 @brief The function compare cached tcam data/mask with data/mask read from hw,
        if compare failed, recover from cached data/mask and send tcam error event log.
*/
int32
drv_ser_check_correct_tcam(uint8 lchip, uint32 mem_id, uint32 entry_idx, void* p_void, uint8  rd_empty)
{
    drv_ser_master_t* p_drv_ser_master = NULL;
    drv_ser_cb_info_t  ser_cb_info;
    uint8* p_mem_addr = NULL;
    uint8* p_data     = NULL;
    uint8* p_mask     = NULL;
    tbl_entry_t buffer_entry;
    uint32 block_size =  0;
    uint8  mem_type   =  0;
    uint8  mem_order  =  0;
    uint32 blk_entry_size  = 0;
    uint32 buf_bk_x[8]      = {0};
    uint32 buf_bk_y[8]      = {0};
    uint32 buf_read_mask[8] = {0};
    uint32 buf_read_data[8] = {0};
    uint8  entry_len_aline = 0;
    drv_ser_db_mask_tcam_fld_t mask_tcam_data;
    uint8  bk_empty = 0;
    tbl_entry_t* p_tbl_entry = NULL;
    drv_ftm_mem_info_t mem_info;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    DRV_PTR_VALID_CHECK(p_void);
    p_tbl_entry = (tbl_entry_t*)p_void;
    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    if ((SDK_WORK_PLATFORM == 1) || (!p_drv_ser_master) || (!p_drv_ser_master->tcam_scan_en) || p_ser_db->db_reset_hw_doing)
    {
        return DRV_E_NONE;
    }

    sal_memset(&mem_info, 0, sizeof(drv_ftm_mem_info_t));
    mem_info.info_type = DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL;
    mem_info.mem_id    = mem_id;
    drv_ftm_get_mem_info(lchip, &mem_info);
    mem_type = mem_info.mem_type;
    mem_order = mem_info.mem_sub_id;

    if (mem_type == DRV_FTM_MEM_TCAM_LPM_KEY)
    {
        drv_ser_db_map_lpm_tcam(lchip, (uint8*)&mem_id, &mem_order, (uint16*)&entry_idx);
    }
    drv_usw_get_memory_size(lchip, mem_id, &block_size);
    drv_ser_db_get_tcam_local_entry_size(lchip, mem_id, &blk_entry_size);
    entry_len_aline = drv_ser_db_get_tcam_entry_size_aline(lchip, mem_id);

    p_mem_addr = p_ser_db->dynamic_tbl[mem_id];
    p_data = p_mem_addr + entry_len_aline*entry_idx;
    p_mask = p_mem_addr + entry_len_aline*entry_idx + blk_entry_size;
    sal_memcpy((uint8*)buf_bk_x, p_data, blk_entry_size);
    sal_memcpy((uint8*)buf_bk_y, p_mask, blk_entry_size);

    mask_tcam_data.type = DRV_MEM_DB_MASK_TCAM_BY_READ;
    mask_tcam_data.p_data = buf_bk_x;
    mask_tcam_data.p_mask = buf_bk_y;
    mask_tcam_data.ram_id = mem_id;
    drv_ser_db_set_tcam_mask_fld(lchip, &mask_tcam_data);
    /*backup-memory: (Y,X) transfer to (data,mask)*/
    drv_usw_chip_convert_tcam_dump_content(lchip, blk_entry_size*8, buf_bk_x, buf_bk_y, &bk_empty);

    if (0 != rd_empty)
    {
        sal_memset((uint8*)buf_read_data, 0, blk_entry_size);
        sal_memset((uint8*)buf_read_mask, 0, blk_entry_size);
    }
    else
    {
        sal_memcpy((uint8*)buf_read_data, (uint8*)(p_tbl_entry->data_entry), blk_entry_size);
        sal_memcpy((uint8*)buf_read_mask, (uint8*)(p_tbl_entry->mask_entry), blk_entry_size);
    }

    mask_tcam_data.type = DRV_MEM_DB_MASK_TCAM_BY_READ;
    mask_tcam_data.p_data = buf_read_data;
    mask_tcam_data.p_mask = buf_read_mask;
    mask_tcam_data.ram_id = mem_id;
    drv_ser_db_set_tcam_mask_fld(lchip, &mask_tcam_data);

    /*Compare: info 1.entry_valid  2.data_entry 3.mask_entry*/
    /*if entry_valid == 0,chip will auto set all X and Y to 1.
    In this case,RTL can only be read,empty == 1, data == mask == 0*/
    if ((rd_empty == 1) && (bk_empty == 1))
    {
        return DRV_E_NONE;
    }
    else if (0 == ((rd_empty ^ bk_empty)
        || (sal_memcmp(buf_bk_x, buf_read_data, blk_entry_size) != 0)
        || (sal_memcmp(buf_bk_y, buf_read_mask, blk_entry_size) != 0)))
    {
        return DRV_E_NONE;
    }

    buffer_entry.data_entry = (uint32*)buf_bk_x;
    buffer_entry.mask_entry = (uint32*)buf_bk_y;

    ser_cb_info.type = DRV_SER_TYPE_TCAM_ERROR;
    ser_cb_info.tbl_id = 0xFE000000 + mem_id;
    ser_cb_info.tbl_index =  blk_entry_size*entry_idx;
    ser_cb_info.tbl_id |= blk_entry_size << 20;


    drv_ser_db_rewrite_tcam(lchip, mem_id, mem_type, mem_order, entry_idx, (void*)&buffer_entry);
    ser_cb_info.recover = 1;
    (p_drv_ser_master->recover_tcam_key)++;
    ser_cb_info.action = DRV_SER_ACTION_LOG;

    sal_memcpy(p_tbl_entry->data_entry, buf_bk_x, blk_entry_size);
    sal_memcpy(p_tbl_entry->mask_entry, buf_bk_y, blk_entry_size);

    drv_ser_report_ser_event_cb(lchip, &ser_cb_info);

    return DRV_E_NONE;
}


/**
 @brief The function recover the ecc error memory with handle info
*/
int32
drv_ser_restore(uint8 lchip, drv_ecc_intr_param_t* p_intr_param)
{
    uint8 intr_bit = 0;
    uint8 valid_cnt = 0;
    drv_ser_cb_info_t  ser_cb_info;
    drv_ser_master_t* p_drv_ser_master = NULL;
    drv_ecc_intr_tbl_t table_info_intr;
    uint8 ecc_tbl_type = 0;
    drv_ser_db_t* p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
    DRV_PTR_VALID_CHECK(p_intr_param);

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);

    if ((!p_drv_ser_master) || (!p_drv_ser_master->ecc_recover_en) || (NULL == p_ser_db->p_static_tbl_hash))
    {
        return DRV_E_NONE;
    }

    for (intr_bit = 0; (valid_cnt < p_intr_param->valid_bit_count) && (intr_bit < p_intr_param->total_bit_num); intr_bit++)
    {
        if (!DRV_BMP_ISSET(p_intr_param->p_bmp, intr_bit))
        {
            continue;
        }

        /*get intr db info*/
        table_info_intr.intr_tblid = p_intr_param->intr_tblid;
        table_info_intr.intr_bit = intr_bit;
        DRV_IF_ERROR_RETURN(drv_ser_get_intr_info(lchip, &table_info_intr, 0));
        ecc_tbl_type = table_info_intr.tbl_type;

        if (ecc_tbl_type == DRV_ECC_TBL_TYPE_STATIC)
        {
            DRV_IF_ERROR_RETURN(_drv_ser_restore_static_tbl(lchip, &table_info_intr, &ser_cb_info));
        }
        else if ((ecc_tbl_type == DRV_ECC_TBL_TYPE_HASH_KEY) || (ecc_tbl_type == DRV_ECC_TBL_TYPE_DYNAMIC_SRAM))
        {
            DRV_IF_ERROR_RETURN(_drv_ecc_restore_dynamic_sram(lchip , &table_info_intr ,&ser_cb_info));
        }
        else
        {
             /*DRV_DBG_INFO("[DRV_ECC]:unknown table[tblid:%d,fld:%d] ,can't do ecc recover\n",p_intr_param->intr_tblid,intr_bit);*/
            return DRV_E_NONE;
        }

        valid_cnt++;
        drv_ser_report_ser_event_cb(lchip, &ser_cb_info);


    }
    return DRV_E_NONE;
}

#define ___ECC_RECOVER_CFG___

/**
 @brief Set scan burst interval entry num and time(ms)
*/
int32
drv_ser_set_tcam_scan_cfg(uint8 lchip,uint32 burst_entry_num, uint32 burst_interval)
{
    uint8 chip_num = 0;
    drv_ser_master_t* p_ser_master = NULL;

    dal_get_chip_number(&chip_num);

    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }

    p_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    if (p_ser_master == NULL)
    {
        return DRV_E_FATAL_EXCEP;
    }

    if (burst_interval && (0 == burst_entry_num))
    {
        return DRV_E_INVALID_PARAMETER;
    }

    p_ser_master->tcam_scan_burst_entry_num = burst_interval ? burst_entry_num : 0xFFFFFFFF;
    p_ser_master->tcam_scan_burst_interval = burst_interval;

    return DRV_E_NONE;
}

/**
 @brief The function simulate ecc error-tbl/sram'index/offset
*/
int32
drv_ser_set_ecc_simulation_idx(uint8 lchip, uint8 mode, uint32 tbl_idx)
{
    uint8 chip_num = 0;
    drv_ser_master_t* p_drv_ser_master = NULL;

    dal_get_chip_number(&chip_num);

    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    if (!p_drv_ser_master)
    {
        return DRV_E_NONE;

    }
    else
    {
        p_drv_ser_master->err_rec_offset = tbl_idx;
        p_drv_ser_master->simulation_en = mode;
    }
    return DRV_E_NONE;
}


#define ___ECC_RECOVER_STATS___
int32
drv_ser_show_status(uint8 lchip)
{
    char str[32] = {0};
    uint8 chip_num = 0;
    drv_ser_master_t* p_drv_ser_master = NULL;
    uint32   static_catch_mem_size   = 0;
    uint32   dynamic_catch_mem_size  = 0;
    uint32   tcam_key_catch_mem_size = 0;
    uint32   ecc_catch_mem_size      = 0;
    drv_ser_db_t* p_ser_db = NULL;

    dal_get_chip_number(&chip_num);
    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }

    p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    if ((!p_drv_ser_master) || (p_ser_db == NULL))
    {
        DRV_DBG_INFO("ECC NOT enable.\n");
        return DRV_E_NONE;
    }

    static_catch_mem_size = p_ser_db->catch_mem_static_size;
    dynamic_catch_mem_size = p_ser_db->catch_mem_dynamic_size;
    tcam_key_catch_mem_size = p_ser_db->catch_mem_tcam_key_size;
    ecc_catch_mem_size = static_catch_mem_size + dynamic_catch_mem_size;

    DRV_DBG_INFO("\n");

    /*multibit/parity*/
    if  (0 == ecc_catch_mem_size)
    {
        sal_sprintf(str, "%s", "0");
    }
    else
    {
        sal_sprintf(str, "%u.%u", ((ecc_catch_mem_size * 100) / (1024 * 1024)) / 100,
            ((ecc_catch_mem_size * 100) / (1024 * 1024)) % 100);
    }
    DRV_DBG_INFO(" %-23s%-10s\n", "EccMultiBit/Parity:",(p_drv_ser_master->ecc_recover_en ? "Yes" : "No"));
    DRV_DBG_INFO(" %-23s%-10s\n", "--Memory Size(M)", str);
    DRV_DBG_INFO("\n");

    /*tcam key scan*/
    if (0 == tcam_key_catch_mem_size)
    {
        sal_sprintf(str, "%s", "0");
    }
    else
    {
        sal_sprintf(str, "%u.%u", ((tcam_key_catch_mem_size * 100) / (1024 * 1024)) / 100, ((tcam_key_catch_mem_size * 100) / (1024 * 1024)) % 100);
    }
    DRV_DBG_INFO(" %-23s%-10s\n", "TcamScan:",(p_drv_ser_master->tcam_scan_en ? "Yes" : "No"));
    DRV_DBG_INFO(" %-23s%-10s\n", "--Memory Size(M)", str);
    DRV_DBG_INFO(" %-23s%-10d\n", "--Scan mode", g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_TCAM]);
    DRV_DBG_INFO(" %-23s%-10d\n", "--Scan interval(minute)", p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_TCAM]);
    DRV_DBG_INFO(" %-23s%-10s\n", "--Scan doing",(p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_TCAM]?"Yes":"No"));
    DRV_DBG_INFO("\n");

    /*sbe scan*/
    DRV_DBG_INFO(" %-23s%-10s\n", "SingleBitScan:",(p_drv_ser_master->tcam_scan_en ? "Yes" : "No"));
    DRV_DBG_INFO(" %-23s%-10d\n", "--Scan mode", g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_SBE]);
    DRV_DBG_INFO(" %-23s%-10d\n", "--Scan interval(minute)", p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_SBE]);
    DRV_DBG_INFO(" %-23s%-10s\n", "--Scan doing",(p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_SBE]?"Yes":"No"));

    /***************************recover cnt status************************/
    DRV_DBG_INFO(" --------------------------------------------\n\n");
    DRV_DBG_INFO(" %-33s%s\n", "RecoverMemory", "RestoreCount");
    DRV_DBG_INFO(" ---------------------------------------------\n");
    /*static tbl*/
    if (p_drv_ser_master->recover_static_cnt)
    {
        sal_memset(&str, 0, sizeof(str));
        sal_sprintf(str, "%s","static_table:");
        DRV_DBG_INFO(" %-33s%u\n", str, p_drv_ser_master->recover_static_cnt);
    }
    /*dynamic tbl*/
    if (p_drv_ser_master->recover_dynamic_cnt)
    {
        sal_memset(&str, 0, sizeof(str));
        sal_sprintf(str, "%s","dynamic_table:");
        DRV_DBG_INFO(" %-33s%u\n", str, p_drv_ser_master->recover_dynamic_cnt);
    }
    /*tcam key*/
    if (p_drv_ser_master->recover_tcam_key)
    {
        sal_memset(&str, 0, sizeof(str));
        sal_sprintf(str, "%s","tcam_key:");
        DRV_DBG_INFO(" %-33s%u\n", str, p_drv_ser_master->recover_tcam_key);
    }
    /*single bit error*/
    if (p_drv_ser_master->recover_sbe_cnt)
    {
        sal_memset(&str, 0, sizeof(str));
        sal_sprintf(str, "%s","single bit:");
        DRV_DBG_INFO(" %-33s%u\n", str, p_drv_ser_master->recover_sbe_cnt);
    }

    /******************************ignore cnt status**********************/
    if (p_drv_ser_master->ignore_cnt)
    {
        DRV_DBG_INFO("\n");
        DRV_DBG_INFO(" ---------------------------------------------\n");
        DRV_DBG_INFO(" %-33s%u\n","Ignore count", p_drv_ser_master->ignore_cnt);
    }

    DRV_DBG_INFO("\n");
    return DRV_E_NONE;
}


#define ___ECC_RECOVER_INIT___

int32
drv_ser_init(uint8 lchip , drv_ser_global_cfg_t* p_cfg)
{
    drv_ser_master_t* p_drv_ser_master = NULL;
    int32 ret = 0;

    DRV_PTR_VALID_CHECK(p_cfg);
    if ((SDK_WORK_PLATFORM == 1) || ( p_drv_master[lchip]->p_ser_master)
        || ((0 == p_cfg->ecc_recover_en) && (0 == p_cfg->tcam_scan_en) && (0 == p_cfg->sbe_scan_en)&& (!p_cfg->hw_reset_en)))
    {
        return DRV_E_NONE;
    }

    drv_ser_db_init( lchip , p_cfg);

    p_drv_ser_master = (drv_ser_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(drv_ser_master_t));
    p_drv_master[lchip]->p_ser_master = p_drv_ser_master;
    if (!p_drv_ser_master)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset(p_drv_ser_master, 0, sizeof(drv_ser_master_t));

    p_drv_ser_master->ecc_recover_en = p_cfg->ecc_recover_en;
    p_drv_ser_master->tcam_scan_en = p_cfg->tcam_scan_en;
    p_drv_ser_master->sbe_scan_en = p_cfg->sbe_scan_en;
    p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_SBE] = 0;
    p_drv_ser_master->scan_interval[DRV_SER_SCAN_TYPE_TCAM] = 0;
    p_drv_ser_master->tcam_scan_burst_interval = 5 * 1000;
    p_drv_ser_master->tcam_scan_burst_entry_num = (0 == p_cfg->tcam_scan_burst_entry_num)
    ? 4096 : p_cfg->tcam_scan_burst_entry_num;

    /*step1: scan task create*/
    if ((p_drv_ser_master->sbe_scan_en) && (p_cfg->sbe_scan_interval))
    {
        ret = drv_ser_set_mem_scan_mode(lchip, DRV_SER_SCAN_TYPE_SBE,
                                                     (p_cfg->sbe_scan_interval != 0),
                                                     p_cfg->sbe_scan_interval,
                                                     p_cfg->cpu_mask);
    }
    else
    {
        g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_SBE] = 2;
    }

    if ((p_drv_ser_master->tcam_scan_en) && (p_cfg->tcam_scan_interval))
    {
        ret = drv_ser_set_mem_scan_mode(lchip, DRV_SER_SCAN_TYPE_TCAM,
                                                     (p_cfg->tcam_scan_interval != 0),
                                                     p_cfg->tcam_scan_interval,
                                                     p_cfg->cpu_mask);
    }
    else
    {
        g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_TCAM] = 2;
    }

    if (ret)
    {
        mem_free(p_drv_ser_master);
        return DRV_E_INIT_FAILED;
    }

    return DRV_E_NONE;
}

int32
drv_ser_deinit(uint8 lchip)
{
    drv_ser_master_t* p_drv_ser_master = NULL;

    p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
    if ((SDK_WORK_PLATFORM == 1) || (!p_drv_ser_master))
    {
        return DRV_E_NONE;
    }

    /*step1: stop all scan task*/
    g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_SBE] = 2;
    g_drv_ecc_scan_mode[DRV_SER_SCAN_TYPE_TCAM] = 2;
    do
    {
        if ((p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_SBE] == 0)
            &&(p_drv_ser_master->scan_doing[DRV_SER_SCAN_TYPE_TCAM] == 0))
        {
            break;
        }
    }
    while (1);

    /*step2: mem free*/
    if (p_drv_ser_master)
    {
        mem_free(p_drv_ser_master);
        p_drv_ser_master = NULL;
    }

    drv_ser_db_deinit( lchip);
    return DRV_E_NONE;
}

int32
drv_ser_set_cfg(uint8 lchip, uint32 type,void * p_cfg)
{
    if (type != DRV_SER_CFG_TYPE_HW_RESER_EN)
    {
        DRV_PTR_VALID_CHECK(p_cfg);
    }
    switch (type)
    {
    case DRV_SER_CFG_TYPE_SCAN_MODE:
    {
        drv_ser_scan_info_t *p_scan_cfg = (drv_ser_scan_info_t*) p_cfg;
        DRV_IF_ERROR_RETURN(drv_ser_set_mem_scan_mode(lchip,p_scan_cfg->type, p_scan_cfg->mode,   p_scan_cfg->scan_interval,0));
    }
        break;
    case DRV_SER_CFG_TYPE_TCAM_SCAN_INFO:
    {
        drv_ser_scan_info_t *p_scan_cfg = (drv_ser_scan_info_t*) p_cfg;
        DRV_IF_ERROR_RETURN(drv_ser_set_tcam_scan_cfg(lchip, p_scan_cfg->burst_entry_num, p_scan_cfg->burst_interval));
    }
        break;
    case DRV_SER_CFG_TYPE_ECC_EVENT_CB:
    {
        drv_ser_master_t* p_drv_ser_master = (drv_ser_master_t*)(p_drv_master[lchip]->p_ser_master);
        if(p_drv_ser_master )
        p_drv_ser_master->ser_event_cb = p_cfg;
    }
        break;

    case DRV_SER_CFG_TYPE_HW_RESER_EN:
        drv_ser_db_set_hw_reset_en( lchip, 1);
        break;
    case DRV_SER_CFG_TYPE_DMA_RECOVER_TCAM:
        drv_ser_dma_recover_tcam(lchip, p_cfg);
        break;
    default:
        return DRV_E_NONE;
    }
    return DRV_E_NONE;
}


int32
drv_ser_get_cfg(uint8 lchip, uint32 type,void * p_cfg)
{
    uint8 lchip_num = 0;
    if ((type != DRV_SER_CFG_TYPE_SHOW_ECC_STAUTS) && (type != DRV_SER_CFG_TYPE_SCAN_MODE))
    {
        DRV_PTR_VALID_CHECK(p_cfg);
    }
    dal_get_chip_number(&lchip_num);
    if (lchip >= lchip_num)
    {
        return DRV_E_NOT_INIT;
    }

    switch (type)
    {
    case DRV_SER_CFG_TYPE_SCAN_MODE:
    {
        if (p_cfg)
        {
            drv_ser_scan_info_t *p_scan_cfg = (drv_ser_scan_info_t*) p_cfg;
            drv_ser_get_mem_scan_mode( lchip , p_scan_cfg->type, &p_scan_cfg->mode, &p_scan_cfg->scan_interval);
        }
        else
        {
            return drv_ser_get_tcam_scan_enable(lchip);
        }
    }
        break;
    case DRV_SER_CFG_TYPE_ECC_INTERRUPT_INFO:
        DRV_IF_ERROR_RETURN(drv_ser_get_intr_info(lchip,p_cfg, 0));
        break;
    case DRV_SER_CFG_TYPE_SHOW_ECC_STAUTS:
        DRV_IF_ERROR_RETURN(drv_ser_show_status(lchip));
        break;
    case DRV_SER_CFG_TYPE_HW_RESER_EN:
    {
        drv_ser_db_t* p_ser_db =  (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);
        if (NULL == p_ser_db)
        {
            *(uint32*)p_cfg = 0;
        }
        else
        {
            *(uint32*)p_cfg = p_ser_db->db_reset_hw_doing;
        }
    }
        break;
    default:
        return DRV_E_NONE;
    }
    return DRV_E_NONE;
}


int32
drv_ser_register_hw_reset_cb(uint8 lchip, uint32 cb_type, drv_ser_hw_reset_cb func)
{
    drv_ser_db_t* p_ser_db = NULL;
    uint8 chip_num = 0;

    dal_get_chip_number(&chip_num);
    if( lchip >= chip_num)
    {
         return DRV_E_NONE;
     }

    p_ser_db = (drv_ser_db_t*)(p_drv_master[lchip]->p_ser_db);

    if ((SDK_WORK_PLATFORM == 1) || (NULL ==  p_drv_master[lchip]->p_ser_db))
    {
        return DRV_E_NONE;
    }

    if ((cb_type >= DRV_SER_HW_RESET_CB_TYPE_NUM) )
    {
        return DRV_E_INVALID_PARAMETER;
    }

    if (p_ser_db->reset_hw_cb[cb_type])/*2nd GO TO */
    {
        return DRV_E_NONE;
    }

    p_ser_db->reset_hw_cb[cb_type] = func;

    return DRV_E_NONE;
}


