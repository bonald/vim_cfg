/**
 @file drv_api.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2015-10-09

 @version v1.0

 The file contains all chip APIs of Driver layer
*/
#include "sal.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_error.h"
#include "dal.h"

extern int32 drv_chip_common_init(uint8 lchip);
extern int32 drv_model_common_init(uint8 lchip);
extern int32 dal_get_chip_number(uint8* p_num);

/* use for testing cpu endian */
struct endian_test_e
{
    uint32 test1    : 1;
    uint32 test2    : 31;
};
typedef struct endian_test_e endian_test_t;
drv_master_t g_drv_master[DRV_MAX_CHIP_NUM];
drv_master_t* p_drv_master[DRV_MAX_CHIP_NUM] = {NULL};


#define __DRV_INTERNAL_FUNCTION_

#if defined CHIP_AGENT && defined _SAL_LINUX_UM

extern uint8 g_usw_chip_agent_init_done;
extern int32 drv_usw_chip_agent_mode(void);

#endif
/**
 @brief get host type automatically. Little-Endian or Big-Endian.
*/
STATIC void
_drv_get_host_type(uint8 lchip)
{
    endian_test_t test;
    uint32* p_test = NULL;

    p_test = (uint32 *)&test;
    *p_test = 0;
    test.test1 = 1;

    if (*p_test == 0x01)
    {
        p_drv_master[lchip]->host_type = HOST_LE;
    }
    else
    {
        p_drv_master[lchip]->host_type = HOST_BE;
    }


    return;
}


/**
 @brief register a Memory field to the register field directory
*/
fields_t *
_drv_find_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id)
{
    fields_t *p_fld = NULL;
	uint32 i = 0;

    if (!(CHK_TABLE_ID_VALID(lchip, tbl_id)))
    {
        DRV_DBG_INFO("ERROR! INVALID TblID or fieldID! TblID: %d, fieldID: %d\n", tbl_id, field_id);
        if (TABLE_NAME(lchip, tbl_id))
        {
            DRV_DBG_INFO("ERROR! INVALID TblName:%s\n", TABLE_NAME(lchip, tbl_id));
        }
        return NULL;
    }

    for (i = 0; i < TABLE_FIELD_NUM(lchip, tbl_id); i++)
    {
         p_fld = TABLE_FIELD_INFO_PTR(lchip, tbl_id) + i;

         if (p_fld && p_fld->field_id == field_id)
         {
             return p_fld;
         }
    }

    return NULL;
}

/*mep table need mask cfg, TBD*/
STATIC int32
_drv_ioctl(uint8 lchip, int32 index, uint32 cmd, void* val)
{
    int32 action;
    tbls_id_t tbl_id;
    uint16 field_id;
    uint32* p_data_entry = NULL;
    uint32* p_mask_entry = NULL;
    tbl_entry_t entry;
    drv_io_callback_fun_t* drv_io_api;
    int32 ret = DRV_E_NONE;

    DRV_PTR_VALID_CHECK(val);

    DRV_INIT_CHECK(lchip);

    action = DRV_IOC_OP(cmd);
    tbl_id = DRV_IOC_MEMID(cmd);
    field_id = DRV_IOC_FIELDID(cmd);

    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    sal_memset(&entry, 0, sizeof(entry));

    p_data_entry = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    p_mask_entry = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    if ((NULL == p_data_entry) || (NULL == p_mask_entry))
    {
        ret = DRV_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_data_entry, 0, sizeof(uint32)*MAX_ENTRY_WORD);
    sal_memset(p_mask_entry, 0, sizeof(uint32)*MAX_ENTRY_WORD);

    entry.data_entry = p_data_entry;
    entry.mask_entry = p_mask_entry;

    drv_io_api= &(p_drv_master[lchip]->drv_io_api);

    switch (action)
    {
    case DRV_IOC_WRITE:
        if ((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_TCAM)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_STATIC_TCAM_KEY))
        {
            /* Write Sram one field of the entry */
            if ((drv_io_api->drv_sram_tbl_read) && (DRV_ENTRY_FLAG != field_id))
            {
                DRV_IF_ERROR_GOTO(drv_io_api->drv_sram_tbl_read(lchip, tbl_id, index, p_data_entry), ret, out);
                DRV_IF_ERROR_GOTO(drv_set_field(lchip, tbl_id, field_id, p_data_entry, (uint32*)val), ret, out);
            }
            else
            {
                sal_memcpy(p_data_entry, (uint32*)val, TABLE_ENTRY_SIZE(lchip, tbl_id));
            }

            if (drv_io_api->drv_sram_tbl_write)
            {
                DRV_IF_ERROR_GOTO(drv_io_api->drv_sram_tbl_write(lchip, tbl_id, index, p_data_entry), ret, out);
            }
        }
        else
        {
            if ((drv_io_api->drv_tcam_tbl_read) && (DRV_ENTRY_FLAG != field_id))
            {
                DRV_IF_ERROR_GOTO(drv_io_api->drv_tcam_tbl_read(lchip, tbl_id, index, &entry), ret, out);
                DRV_IF_ERROR_GOTO(drv_set_field(lchip, tbl_id, field_id, entry.data_entry, (uint32 *)((tbl_entry_t*)val)->data_entry), ret, out);
                DRV_IF_ERROR_GOTO(drv_set_field(lchip, tbl_id, field_id, entry.mask_entry, (uint32 *)((tbl_entry_t*)val)->mask_entry), ret, out);
            }
            else
            {
                sal_memcpy(entry.data_entry, (uint32 *)((tbl_entry_t*)val)->data_entry, TABLE_ENTRY_SIZE(lchip, tbl_id));
                sal_memcpy(entry.mask_entry, (uint32 *)((tbl_entry_t*)val)->mask_entry, TABLE_ENTRY_SIZE(lchip, tbl_id));
            }

            if (drv_io_api->drv_tcam_tbl_write)
            {
                DRV_IF_ERROR_GOTO(drv_io_api->drv_tcam_tbl_write(lchip, tbl_id, index, &entry), ret, out);
            }
        }
        break;
    case DRV_IOC_READ:
        if ((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_TCAM)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_STATIC_TCAM_KEY))
        {
            /* Read Sram one field of the entry */
            if (drv_io_api->drv_sram_tbl_read)
            {
                DRV_IF_ERROR_GOTO(drv_io_api->drv_sram_tbl_read(lchip, tbl_id, index, p_data_entry), ret, out);
            }

            if (DRV_ENTRY_FLAG != field_id)
            {
                DRV_IF_ERROR_GOTO(drv_get_field(lchip, tbl_id, field_id, p_data_entry, (uint32*)val), ret, out);
            }
            else
            {
                sal_memcpy((uint32*)val, p_data_entry, TABLE_ENTRY_SIZE(lchip, tbl_id));
            }
        }
        else
        {
            /* Read Tcam one field of the entry */
            /* For Tcam entry, we could only operate its data's filed, app should not opreate tcam field */
            if (drv_io_api->drv_tcam_tbl_read)
            {
                DRV_IF_ERROR_GOTO(drv_io_api->drv_tcam_tbl_read(lchip, tbl_id, index, &entry), ret, out);
            }

            if (DRV_ENTRY_FLAG != field_id)
            {
                DRV_IF_ERROR_GOTO(drv_get_field(lchip, tbl_id, field_id, (entry.data_entry), (uint32 *)((tbl_entry_t*)val)->data_entry), ret, out);
                DRV_IF_ERROR_GOTO(drv_get_field(lchip, tbl_id, field_id, (entry.mask_entry), (uint32 *)((tbl_entry_t*)val)->mask_entry), ret, out);
            }
            else
            {
                sal_memcpy((uint32 *)((tbl_entry_t*)val)->data_entry, entry.data_entry, TABLE_ENTRY_SIZE(lchip, tbl_id));
                sal_memcpy((uint32 *)((tbl_entry_t*)val)->mask_entry, entry.mask_entry, TABLE_ENTRY_SIZE(lchip, tbl_id));
            }
        }
        break;
    case DRV_IOC_REMOVE:
    /*just for tcam remove*/
        if ((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_TCAM)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT)
            && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_STATIC_TCAM_KEY))
        {
            ret = DRV_E_INVALID_TCAM_INDEX;
            goto out;
        }

        if (drv_io_api->drv_tcam_tbl_remove)
        {
            DRV_IF_ERROR_GOTO(drv_io_api->drv_tcam_tbl_remove(lchip, tbl_id, index), ret, out);
        }

    default:
        break;
    }

out:
    if (p_data_entry)
    {
        mem_free(p_data_entry);
    }

    if (p_mask_entry)
    {
        mem_free(p_mask_entry);
    }

    return ret;
}


STATIC int32
_drv_oam_mask_ioctl(uint8 lchip, int32 index, uint32 cmd, void* val)
{
#if(SDK_WORK_PLATFORM == 0)
    uint32 cmd_oam = 0;
#endif
    uint32* value = NULL;

#if(SDK_WORK_PLATFORM == 0)
    /*write mask*/
    value = (uint32 *)(((tbl_entry_t*)val)->mask_entry);
    cmd_oam = DRV_IOW(OamDsMpDataMask_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, 0, cmd_oam, value));
#endif

    /*write data*/
    value = (uint32 *)(((tbl_entry_t*)val)->data_entry);
    DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, index, cmd, value));

    return DRV_E_NONE;
}

#if(SDK_WORK_PLATFORM == 0)
STATIC int32
_drv_aps_mask_ioctl(uint8 lchip, int32 index, uint32 cmd, void* val)
{
    uint32 cmd_aps = 0;
    DsApsBridge_m data_mask;
    uint32 field_id = 0;
    uint32 mask_value = 0;

    field_id = DRV_IOC_FIELDID(cmd);

    if (DRV_ENTRY_FLAG == field_id)
    {
        /*per entry write, mask should be all 0*/
        sal_memset(&data_mask, 0, sizeof(DsApsBridge_m));
    }
    else
    {
        sal_memset(&data_mask, 0xff, sizeof(DsApsBridge_m));
        DRV_IOW_FIELD(lchip, DsApsBridge_t, field_id, &mask_value, &data_mask);
    }

    if (DRV_IS_DUET2(lchip))
    {
        /*write mask*/
        cmd_aps = DRV_IOW(DsApsBridgeMask_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, 0, cmd_aps, &data_mask));

        /*mapping data*/
        cmd = DRV_IOW(DsApsBridgeRtl_t, field_id);

        /*write data*/
        DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, index, cmd, val));
    }
    else
    {
        uint32 field_val = 0;
        /*write mask*/
        cmd_aps = DRV_IOW(MetFifoDsApsMask_t, MetFifoDsApsMask_dsApsMemMask_f);
        DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, 0, cmd_aps, &data_mask));

        field_val = 1;
        cmd_aps = DRV_IOW(MetFifoDsApsMask_t, MetFifoDsApsMask_dsApsMemMaskEn_f);
        DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, 0, cmd_aps, &field_val));

        DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, index, cmd, val));
    }

    return DRV_E_NONE;
}
#endif

#define __DRV_API__

/**
  @driver set gchip id for lchip
*/
int32
drv_set_gchip_id(uint8 lchip, uint8 gchip_id)
{
    DRV_INIT_CHECK(lchip);
    p_drv_master[lchip]->g_lchip = gchip_id;

    return DRV_E_NONE;
}

/**
  @driver get gchip id for lchip
*/
int32
drv_get_gchip_id(uint8 lchip, uint8* gchip_id)
{
    DRV_INIT_CHECK(lchip);
    *gchip_id = p_drv_master[lchip]->g_lchip;
    return DRV_E_NONE;
}

/**
  @driver get environmnet type interface
*/
int32
drv_get_platform_type(uint8 lchip, drv_work_platform_type_t *plaform_type)
{
    DRV_INIT_CHECK(lchip);

    if (p_drv_master[lchip])
    {
       *plaform_type = p_drv_master[lchip]->plaform_type;
        return DRV_E_NONE;
    }
    else
    {
        DRV_DBG_INFO("@@@ERROR, Driver is not initialized!\n");
        return  DRV_E_NOT_INIT;
    }
}

/**
  @driver get host type interface
*/
int32
drv_get_host_type (uint8 lchip)
{
    DRV_INIT_CHECK(lchip);

    return p_drv_master[lchip]->host_type;
}

/**
  @driver get table property
*/
int32
drv_get_table_property(uint8 lchip, uint8 type, tbls_id_t tbl_id, uint32 index, void* value)
{
    int32 ret = 0;

    if (!value)
    {
        return DRV_E_INVALID_ADDR;
    }

    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);
    DRV_INIT_CHECK(lchip);

    switch(type)
    {
        case DRV_TABLE_PROP_TYPE:
            *((uint32*)value) = drv_usw_get_table_type(lchip, tbl_id);

            break;
        case DRV_TABLE_PROP_HW_ADDR:
            ret = drv_usw_table_get_hw_addr(lchip, tbl_id, index, (uint32*)value, 0);
            break;

        case DRV_TABLE_PROP_GET_NAME:
            ret = drv_usw_get_tbl_string_by_id(lchip, tbl_id, (char*)value);
            break;

        case DRV_TABLE_PROP_BITMAP:
            *((uint32*)value) = DYNAMIC_BITMAP(lchip, tbl_id);
            break;

        case DRV_TABLE_PROP_FIELD_NUM:
            *((uint32*)value) = TABLE_FIELD_NUM(lchip, tbl_id);
            break;

        default:
            ret = DRV_E_INVALID_TBL;
            break;
    }

    return ret;
}

/**
  @driver set chip access type
*/
int32
drv_set_access_type(uint8 lchip, drv_access_type_t access_type)
{
    DRV_INIT_CHECK(lchip);
    p_drv_master[lchip]->access_type = access_type;

    return DRV_E_NONE;
}

/**
  @driver get chip access type
*/
int32
drv_get_access_type(uint8 lchip, drv_access_type_t* p_access_type)
{
    DRV_INIT_CHECK(lchip);
    *p_access_type = p_drv_master[lchip]->access_type;

    return DRV_E_NONE;
}
/**
  @driver install drv io/app interface
*/
int32
drv_install_api(uint8 lchip, drv_io_callback_fun_t* cb)
{

    uint8 chip_num = 0;

    dal_get_chip_number(&chip_num);

    if (lchip >= chip_num)
    {
        return DRV_E_NOT_INIT;
    }

    if (NULL == cb)
    {
        return DRV_E_NOT_INIT;
    }

    p_drv_master[lchip]->drv_io_api.drv_sram_tbl_read = cb->drv_sram_tbl_read;
    p_drv_master[lchip]->drv_io_api.drv_sram_tbl_write = cb->drv_sram_tbl_write;
    p_drv_master[lchip]->drv_io_api.drv_tcam_tbl_read = cb->drv_tcam_tbl_read;
    p_drv_master[lchip]->drv_io_api.drv_tcam_tbl_write = cb->drv_tcam_tbl_write;
    p_drv_master[lchip]->drv_io_api.drv_tcam_tbl_remove = cb->drv_tcam_tbl_remove;

    return DRV_E_NONE;
}


int32  drv_field_support_check(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id)
{
    fields_t* field = NULL;

    field = _drv_find_field(lchip, tbl_id, field_id);
    if (field == NULL)
    {
        return DRV_E_INVALID_FLD;
    }

    return DRV_E_NONE;
}

uint8  drv_is_chip(uint8 lchip, drv_chip_type_t chip_type)
{
    uint32 dev_id = 0;

    dal_get_chip_dev_id(0, &dev_id);

    if (chip_type == DRV_DUET2 && dev_id == DAL_DUET2_DEVICE_ID)
    {
       return 1;
    }

    if (chip_type == DRV_TSINGMA && dev_id == DAL_TSINGMA_DEVICE_ID)
    {
       return 1;
    }

    return 0;
}



/**
 @brief Get a field of a memory data entry
*/
int32
drv_get_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id,
                void* ds, uint32* value)
{
    fields_t* field = NULL;
    int32 seg_id;
    segs_t* seg = NULL;
    uint8 uint32_idx  = 0;
    uint16 remain_len  = 0;
    uint16 cross_word  = 0;
    uint32 remain_value = 0;
    uint32 seg_value = 0;
    uint32* entry  = NULL;

    DRV_INIT_CHECK(lchip);

    DRV_PTR_VALID_CHECK(ds)
    DRV_PTR_VALID_CHECK(value)
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    entry = ds;
    field = _drv_find_field(lchip, tbl_id, field_id);
    if (field == NULL)
    {
         /*DRV_DBG_INFO("ERROR! (drv_get_field): %s(%d), field-%d is not supported\n", TABLE_NAME(lchip, tbl_id), tbl_id, field_id);*/
        *value = 0;
        return 0;
    }

    seg_id = (field->seg_num-1);
    while( seg_id >= 0)
    {
        seg = &(field->ptr_seg[seg_id]);
        seg_value = (entry[seg->word_offset] >> seg->start) & SHIFT_LEFT_MINUS_ONE(seg->bits);

        value[uint32_idx] = (seg_value << remain_len) | remain_value;
        if ((seg->bits + remain_len) == 32)
        {
            remain_value = 0;
            cross_word = 0;
            uint32_idx ++;
        }
        else if ((seg->bits + remain_len) > 32)
        {
            /*create new remain_value */
            remain_value = seg_value >> (32 - remain_len);
            cross_word = 1;
            uint32_idx ++;
        }
        else
        {
            /*create new remain_value */
            remain_value = (seg_value << remain_len) | remain_value;
            cross_word = 0;
        }
        /*create new remain_len */
        remain_len   = (seg->bits + remain_len) & 0x1F; /*rule out bits that exceeds 32*/
        seg_id --;
    }

    if(cross_word) /* last seg cross word */
    {
        value[uint32_idx] = remain_value;
    }

    return DRV_E_NONE;
}

/**
 @brief Set a field of a memory data entry
*/
int32
drv_set_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id,
                        void* ds, uint32 *value)
{
    fields_t* field = NULL;
    segs_t* seg = NULL;
    int32 seg_id = 0;
    uint8 cut_len =  0;
    uint8 array_idx = 0;
    uint32 seg_value = 0;
    uint32 value_a = 0;
    uint32 value_b = 0;
    uint32* entry  = NULL;

    DRV_INIT_CHECK(lchip);

    DRV_PTR_VALID_CHECK(ds)
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    entry = ds;
    /* register field support check */
    field = _drv_find_field(lchip, tbl_id, field_id);

    if (field == NULL)
    {
         /*DRV_DBG_INFO("ERROR! (drv_set_field): tbl_id-%d, name-%s(%d), field-%d is not supported\n", tbl_id, TABLE_NAME(lchip, tbl_id),tbl_id, field_id);*/
        return DRV_E_NONE;
    }


    seg_id = (field->seg_num-1);
    cut_len = 0;

    array_idx = 0;
    while( seg_id >= 0)
    {
        seg = &(field->ptr_seg[seg_id]);

        if ((cut_len + seg->bits) >= 32)
        {
            value_b = (value[array_idx + 1 ] & SHIFT_LEFT_MINUS_ONE((cut_len + seg->bits - 32)));
            value_a = (value[array_idx] >> cut_len) & SHIFT_LEFT_MINUS_ONE((32- cut_len));
            seg_value = (value_b << (32 - cut_len)) | value_a;

            array_idx++;
        }
        else
        {
            value_a = (value[array_idx] >> cut_len) & SHIFT_LEFT_MINUS_ONE(seg->bits);
            seg_value =  value_a;
        }

        entry[seg->word_offset] &= ~ (SHIFT_LEFT_MINUS_ONE(seg->bits)  << seg->start);
        entry[seg->word_offset] |= (seg_value << seg->start);

        cut_len = (cut_len + seg->bits) & 0x1F; /*rule out bits that exceeds 32*/

        seg_id--;
    }

    return DRV_E_NONE;
}

/**
@brief Get a field  valued of a memory data entry
*/
uint32
drv_get_field_value(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id,  void* ds)
{
    uint32 value = 0;
    DRV_INIT_CHECK(lchip);

    drv_get_field(lchip, tbl_id, field_id,                  ds, &value);
    return value;
}

/**
  @driver Driver IO Api
*/
int32
drv_ioctl_api(uint8 lchip, int32 index, uint32 cmd, void* val)
{
    tbls_id_t tbl_id;
    uint32 action;
    action = DRV_IOC_OP(cmd);
    tbl_id = DRV_IOC_MEMID(cmd);

    DRV_INIT_CHECK(lchip);

    if (0 == TABLE_FIELD_NUM(lchip, tbl_id) || TABLE_MAX_INDEX(lchip, tbl_id) == 0)
    {
          return 0;
    }

    if(action == DRV_IOC_WRITE && (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING))
    {
        return DRV_E_NONE;
    }

#if defined CHIP_AGENT && defined _SAL_LINUX_UM && (SDK_WORK_PLATFORM == 1)
    if (DRV_CHIP_AGT_MODE_CLIENT == drv_usw_chip_agent_mode() && !g_usw_chip_agent_init_done)
    {
        return DRV_E_NONE;
    }
#endif
    if(((tbl_id == DsEthMep_t) || (tbl_id == DsEthRmep_t) || (tbl_id == DsBfdMep_t) || (tbl_id == DsBfdRmep_t)) &&
        (DRV_IOC_WRITE == action))
    {
        DRV_IF_ERROR_RETURN(_drv_oam_mask_ioctl(lchip, index, cmd, val));
    }
#if(SDK_WORK_PLATFORM == 0)
    else if(tbl_id == DsApsBridge_t)
    {
        if (DRV_IOC_WRITE == action)
        {
            DRV_IF_ERROR_RETURN(_drv_aps_mask_ioctl(lchip, index, cmd, val));
        }
        else
        {
            uint32 cmd_aps = 0;
            uint32 field_id = 0;
            field_id = DRV_IOC_FIELDID(cmd);
			if(DRV_IS_DUET2(lchip))
            {
                cmd_aps = DRV_IOR(DsApsBridgeRtl_t, field_id);
            }
            else
            {
                cmd_aps = DRV_IOR(tbl_id, field_id);
            }
            DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, index, cmd_aps, val));
        }
    }
    else if ((tbl_id == DsMac_t) && DRV_IS_DUET2(lchip))
    {
        FibAccDrainEnable_m fib_drain_en;
        int32 ret = DRV_E_NONE;
        uint32 cmd_tmp = 0;

        sal_memset(&fib_drain_en, 0, sizeof(FibAccDrainEnable_m));
        SetFibAccDrainEnable(V, agingReqDrainEnable_f, &fib_drain_en, 0);
        SetFibAccDrainEnable(V, learnReqDrainEnable_f, &fib_drain_en, 0);

        FIB_ACC_LOCK(lchip);
        cmd_tmp = DRV_IOW(FibAccDrainEnable_t, DRV_ENTRY_FLAG);
        _drv_ioctl(lchip, 0, cmd_tmp, &fib_drain_en);

        ret = _drv_ioctl(lchip, index, cmd, val);

        SetFibAccDrainEnable(V, agingReqDrainEnable_f, &fib_drain_en, 1);
        SetFibAccDrainEnable(V, learnReqDrainEnable_f, &fib_drain_en, 1);
        _drv_ioctl(lchip, 0, cmd_tmp, &fib_drain_en);
        FIB_ACC_UNLOCK(lchip);
        DRV_IF_ERROR_RETURN(ret);
    }
#endif
    else
    {
#if(SDK_WORK_PLATFORM == 0)
        DRV_IF_ERROR_RETURN(drv_usw_mcu_tbl_lock(lchip, tbl_id, action));
#endif
        DRV_IF_ERROR_RETURN(_drv_ioctl(lchip, index, cmd, val));
#if(SDK_WORK_PLATFORM == 0)
        DRV_IF_ERROR_RETURN(drv_usw_mcu_tbl_unlock(lchip, tbl_id, action));
#endif
    }

    return DRV_E_NONE;
}

/**
  @driver Driver Acc Api
*/
int32
drv_acc_api(uint8 lchip, void* in, void* out)
{
    DRV_INIT_CHECK(lchip);

#if defined CHIP_AGENT && defined _SAL_LINUX_UM && (SDK_WORK_PLATFORM == 1)
    if (DRV_CHIP_AGT_MODE_CLIENT == drv_usw_chip_agent_mode() && !g_usw_chip_agent_init_done)
        return DRV_E_NONE;
    else
#endif
    DRV_IF_ERROR_RETURN(drv_usw_acc(lchip, in, out));

    return DRV_E_NONE;
}

/**
  @driver Driver Get ChipAgent mode
*/
int32
drv_get_chip_agent_mode(void)
{
    #if defined CHIP_AGENT && defined _SAL_LINUX_UM
        return drv_usw_chip_agent_mode();
    #else
        return 0;
    #endif

}

/**
  @driver Driver Set warmboot status
*/
int32
drv_set_warmboot_status(uint8 lchip, uint32 wb_status)
{
    DRV_INIT_CHECK(lchip);

    p_drv_master[lchip]->wb_status = wb_status;

    return DRV_E_NONE;
}


#ifdef DUET2
extern int32
drv_tbls_list_init_duet2(uint8 lchip);
int32
drv_tbls_list_deinit_duet2(uint8 lchip);
#endif

#ifdef TSINGMA
extern int32
drv_tbls_list_init_tsingma(uint8 lchip);
int32
drv_tbls_list_deinit_tsingma(uint8 lchip);
#endif


int32
drv_tbls_info_init(uint8 lchip)
{

#ifdef DUET2
    if (DRV_IS_DUET2(lchip))
    {
        drv_tbls_list_init_duet2(lchip);
    }
#endif


#ifdef TSINGMA

    if (DRV_IS_TSINGMA(lchip))
    {
        drv_tbls_list_init_tsingma(lchip);
    }
#endif

    return 0;
}

int32
drv_tbls_info_deinit(uint8 lchip)
{

#ifdef DUET2
    if (DRV_IS_DUET2(lchip))
    {
        drv_tbls_list_deinit_duet2(lchip);
    }
#endif


#ifdef TSINGMA

    if (DRV_IS_TSINGMA(lchip))
    {
        drv_tbls_list_deinit_tsingma(lchip);
    }
#endif

    return 0;
}

/**
  @driver Driver Init Api,per chip init TBD
*/
int32
drv_init(uint8 lchip, uint8 base)
{
    uint8 chip_num = 0;

    dal_get_chip_number(&chip_num);
    if(p_drv_master[lchip] == NULL)
    {
        uint16 lchip_idx = 0;
        for(lchip_idx=0; lchip_idx < DRV_MAX_CHIP_NUM; lchip_idx++)
        {
            p_drv_master[lchip_idx] = &(g_drv_master[lchip_idx]);
        }
    }
    sal_memset(p_drv_master[lchip], 0, sizeof(drv_master_t));

    p_drv_master[lchip]->plaform_type = SDK_WORK_PLATFORM;
    p_drv_master[lchip]->workenv_type = SDK_WORK_ENV;
    p_drv_master[lchip]->access_type = DRV_PCI_ACCESS;

    drv_tbls_info_init(lchip);

    _drv_get_host_type(lchip);

    if (0 == SDK_WORK_PLATFORM)
    {
        drv_chip_common_init(lchip);
    }
    else
    {
        drv_model_common_init(lchip);
    }
    p_drv_master[lchip]->init_done = 1;
    return DRV_E_NONE;
}

int32
drv_deinit(uint8 lchip, uint8 base)
{
    drv_ser_deinit(lchip);
    drv_tbls_info_deinit(lchip);

    p_drv_master[lchip]->p_flow_tcam_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_flow_tcam_mutex):0;
    p_drv_master[lchip]->p_lpm_ip_tcam_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_lpm_ip_tcam_mutex):0;
    p_drv_master[lchip]->p_lpm_nat_tcam_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_lpm_nat_tcam_mutex):0;
    p_drv_master[lchip]->p_entry_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_entry_mutex):0;
    p_drv_master[lchip]->fib_acc_mutex ? sal_mutex_destroy(p_drv_master[lchip]->fib_acc_mutex):0;
    p_drv_master[lchip]->cpu_acc_mutex ? sal_mutex_destroy(p_drv_master[lchip]->cpu_acc_mutex):0;
    p_drv_master[lchip]->ipfix_acc_mutex ? sal_mutex_destroy(p_drv_master[lchip]->ipfix_acc_mutex):0;
    p_drv_master[lchip]->cid_acc_mutex ? sal_mutex_destroy(p_drv_master[lchip]->cid_acc_mutex):0;
    p_drv_master[lchip]->p_pci_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_pci_mutex):0;
    p_drv_master[lchip]->p_i2c_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_i2c_mutex):0;
    p_drv_master[lchip]->p_hss_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_hss_mutex):0;
    p_drv_master[lchip]->mpls_acc_mutex ? sal_mutex_destroy(p_drv_master[lchip]->mpls_acc_mutex):0;
    p_drv_master[lchip]->gemport_acc_mutex ? sal_mutex_destroy(p_drv_master[lchip]->gemport_acc_mutex):0;
    p_drv_master[lchip]->p_mep_mutex ? sal_mutex_destroy(p_drv_master[lchip]->p_mep_mutex):0;
    sal_memset(&(g_drv_master[lchip]), 0, sizeof(drv_master_t));
    return 0;
}
