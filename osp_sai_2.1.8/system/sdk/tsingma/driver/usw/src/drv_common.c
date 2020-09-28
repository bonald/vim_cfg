/**
  @file drv_tbl_reg.c

  @date 2010-07-22

  @version v5.1

  The file implement driver IOCTL defines and macros
*/

#include "sal.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_error.h"
#include "usw/include/drv_chip_ctrl.h"
/**********************************************************************************
              Define Global declarations, Typedef, define and Data Structure
***********************************************************************************/

dup_address_offset_type_t duplicate_addr_type = SLICE_Addr_All;

/**
 @brief
*/
int32
drv_usw_get_tbl_index_base(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint8 *addr_offset)
{
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);
    DRV_INIT_CHECK(lchip);
    if (TABLE_ENTRY_TYPE(lchip, tbl_id) == SLICE_Cascade)
    {
        if (index >= (TABLE_MAX_INDEX(lchip, tbl_id)/2))
        {
            *addr_offset = 1;
        }
        else
        {
            *addr_offset = 0;
        }
    }
#if (SDK_WORK_PLATFORM == 0)
    else if (TABLE_ENTRY_TYPE(lchip, tbl_id) == SLICE_Duplicated)
    {
        if (duplicate_addr_type == SLICE_Addr_0)
        {
            *addr_offset = 1;
        }
        else if (duplicate_addr_type == SLICE_Addr_1)
        {
            *addr_offset = 2;
        }
    }
#endif
    else
    {
        *addr_offset = 0;
    }

    return DRV_E_NONE;
}

fields_t *
drv_usw_find_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id)
{
    fields_t *p_fld = NULL;
	uint32 i = 0;
    if (!p_drv_master[lchip]->init_done)
    {
        return NULL;
    }
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

/**
 @brief Get a field of  word & bit offset
*/
int32
drv_usw_get_field_offset(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, uint32* w_offset, uint32 *b_offset)
{
#ifdef SHANZ_NOTE

    fields_t* field = NULL;
    DRV_INIT_CHECK(lchip);
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    field = drv_find_field(lchip, tbl_id, field_id);
    if (field == NULL)
    {
        DRV_DBG_INFO("ERROR! (drv_usw_mem_get_field): memID-%d, field-%d is not supported\n", tbl_id, field_id);
        return DRV_E_INVALID_FLD;
    }

    if(NULL != w_offset)
    {
        *w_offset = field->word_offset;
    }

    if(NULL != b_offset)
    {
        *b_offset = field->bit_offset;
    }
#endif
    return DRV_E_NONE;
}

/**
 @brief
*/
int32
drv_usw_get_tbl_string_by_id(uint8 lchip, tbls_id_t tbl_id, char* name)
{
    DRV_INIT_CHECK(lchip);
    if (!name)
    {
        return DRV_E_INVALID_PARAMETER;
    }

    if ((tbl_id >= 0) && (tbl_id < MaxTblId_t))
    {
         sal_strcpy(name, TABLE_NAME(lchip, tbl_id));
         return DRV_E_NONE;
    }

    return DRV_E_INVALID_TBL;
}

/**
 @brief
*/
int32
drv_usw_get_field_string_by_id(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, char* name)
{
    fields_t* ptr_field_info = NULL;

    if (!name)
    {
        return DRV_E_INVALID_PARAMETER;
    }
    DRV_INIT_CHECK(lchip);
    ptr_field_info = drv_usw_find_field(lchip, tbl_id, field_id);
    if (NULL != ptr_field_info)
    {
        sal_strcpy(name, ptr_field_info->ptr_field_name);
    }
    else
    {
        return DRV_E_INVALID_FLD;
    }


    return DRV_E_INVALID_TBL;
}

/**
 @brief
*/
int32
drv_usw_get_tbl_id_by_string(uint8 lchip, tbls_id_t* tbl_id, char* name)
{
    tbls_id_t tmp_tableid = 0;
    DRV_INIT_CHECK(lchip);

    /* This is so inefficient Code, need to consider to optimize it, add by zhouw??? */
    for (tmp_tableid = 0; tmp_tableid < MaxTblId_t; tmp_tableid++)
    {
        if (0 == sal_strcasecmp(name, TABLE_NAME(lchip, tmp_tableid)))
        {
            *tbl_id = tmp_tableid;
            return DRV_E_NONE;
        }
    }

    DRV_DBG_INFO("%% Not Find the TableID!! tableName: %s\n", name);

    return DRV_E_INVALID_TBL;
}

/**
 @brief
*/
int32
drv_usw_get_field_id_by_string(uint8 lchip, tbls_id_t tbl_id, fld_id_t* field_id, char* name)
{
    fields_t* ptr_field_info = NULL;
    uint32 tmp_index = 0;
    DRV_INIT_CHECK(lchip);

    if (!(CHK_TABLE_ID_VALID(lchip, tbl_id)))
    {
        DRV_DBG_INFO("ERROR! INVALID TblID or fieldID! TblID: %d\n", tbl_id);
        if (TABLE_NAME(lchip, tbl_id))
        {
            DRV_DBG_INFO("ERROR! INVALID TblName:%s\n", TABLE_NAME(lchip, tbl_id));
        }
        return DRV_E_INVALID_TBL;
    }

    ptr_field_info = TABLE_FIELD_INFO_PTR(lchip, tbl_id);

    for (tmp_index = 0; tmp_index < TABLE_FIELD_NUM(lchip, tbl_id); tmp_index++)
    {
        if (0 == sal_strcasecmp(name, ptr_field_info[tmp_index].ptr_field_name))
        {
            *field_id = ptr_field_info[tmp_index].field_id;
            return DRV_E_NONE;
        }
    }

    DRV_DBG_INFO("%% Not Find the FieldID!! tableId: %d; Fieldname: %s\n", tbl_id, name);

    return DRV_E_INVALID_FLD;
}

int32
drv_usw_get_field_id_by_sub_string(uint8 lchip, tbls_id_t tbl_id, fld_id_t* field_id, char* name)
{
    fields_t* ptr_field_info = NULL;
    uint32 tmp_index = 0;
    DRV_INIT_CHECK(lchip);

    if (!(CHK_TABLE_ID_VALID(lchip, tbl_id)))
    {
        DRV_DBG_INFO("ERROR! INVALID TblID or fieldID! TblID: %d\n", tbl_id);
        if (TABLE_NAME(lchip, tbl_id))
        {
            DRV_DBG_INFO("ERROR! INVALID TblName:%s\n", TABLE_NAME(lchip, tbl_id));
        }
        return DRV_E_INVALID_TBL;
    }

    ptr_field_info = TABLE_FIELD_INFO_PTR(lchip, tbl_id);
    for (tmp_index = 0; tmp_index < TABLE_FIELD_NUM(lchip, tbl_id); tmp_index++)
    {
        if (0 == sal_strcasecmp(name, ptr_field_info[tmp_index].ptr_field_name))
        {
            *field_id = ptr_field_info[tmp_index].field_id;
          /*   DRV_DBG_INFO("%% The Fieldname is found by accuracy matching !! tableId: %d; Fieldname: % s \n", tbl_id, name);*/
            return DRV_E_NONE;
        }
    }

    for (tmp_index = 0; tmp_index < TABLE_FIELD_NUM(lchip, tbl_id); tmp_index++)
    {
        if (NULL != sal_strstr((ptr_field_info[tmp_index].ptr_field_name), name))
        {
            *field_id = ptr_field_info[tmp_index].field_id;
          /*   DRV_DBG_INFO("%% The Fieldname is found by fuzzy matching !! tableId: %d; Fieldname: % s \n", tbl_id, name);*/
            return DRV_E_NONE;
        }
    }

    DRV_DBG_INFO("%% Not Find the FieldID!! tableId: %d; Fieldname: %s\n", tbl_id, name);

    return DRV_E_INVALID_FLD;
}

int32
drv_usw_table_consum_hw_addr_size_per_index(uint8 lchip, tbls_id_t tbl_id, uint32 *hw_addr_size)
{
    uint32 entry_size = TABLE_ENTRY_SIZE(lchip, tbl_id);
    uint32 word_num = 0;

    if ((!entry_size)
        || (entry_size%DRV_BYTES_PER_WORD))
    {
        DRV_DBG_INFO("%% ERROR! tbl_id = %d 's entrySize(%d Bytes) is unreasonable in driver dataBase!!\n", tbl_id, entry_size);
        return DRV_E_INVALID_PARAMETER;
    }

    *hw_addr_size = 0;
    word_num = entry_size/DRV_BYTES_PER_WORD;

    switch(word_num) /* wordNum */
    {
        case 1:     /* 1 word */
            *hw_addr_size = 1;
            break;
        case 2:     /* 2 words */
            *hw_addr_size = 2;
            break;
        case 3:
        case 4:
            *hw_addr_size = 4;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            *hw_addr_size = 8;
            break;
        default:           /* addr_unit(=2^n) >= wordNum > 2^(n-1) */
            if ((word_num <= 16) && (word_num >= 9))
            {
                *hw_addr_size = 16;
            }
            else if ((word_num >= 17) && (word_num <= 32))
            {
                *hw_addr_size = 32;
            }
            else if ((word_num >= 33) && (word_num <= 64))
            {
                *hw_addr_size = 64;
            }
            /* hold on by zhouw???? Pay attention !! */
           /*  *addr_unit = 2^(ceil(M_LOG2E *(entry_size/DRV_BYTES_PER_WORD)));*/
            break;
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_table_get_sram_hw_addr(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32 *hw_addr, uint32 is_dump_cfg)
{
    uint8 blk_id = 0;
    uint8 blk_id_tmp = 0;
    uint8 addr_offset = 0;
    uint32 hw_data_base = 0;
    uint32 hw_addr_size_per_idx = 0; /* unit: word, per-index consume hw address size */
    uint32 map_index = 0;

    DRV_TBL_INDEX_VALID_CHECK(lchip, tbl_id, index);

    DRV_IF_ERROR_RETURN(drv_usw_table_consum_hw_addr_size_per_index(lchip, tbl_id, &hw_addr_size_per_idx));

    if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_DYNAMIC
        || drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_MIXED)
    {
        for (blk_id = 0; blk_id < MAX_DRV_BLOCK_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(DYNAMIC_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_MIXED)
            {
                blk_id_tmp = blk_id+DRV_FTM_MIXED0;
            }
            else
            {
                blk_id_tmp = blk_id;
            }
            if ((index >= DYNAMIC_START_INDEX(lchip, tbl_id, blk_id_tmp)) && (index <= DYNAMIC_END_INDEX(lchip, tbl_id, blk_id_tmp)))
            {
                hw_data_base = DYNAMIC_DATA_BASE(lchip, tbl_id, blk_id_tmp, addr_offset);
                break;
            }
        }

        if (blk_id == MAX_DRV_BLOCK_NUM)
        {
            DRV_DBG_INFO("ERROR!! get tbl_id %d \n", index);
            return DRV_E_INVALID_INDEX;
        }

        if (DYNAMIC_ACCESS_MODE(lchip, tbl_id) == DYNAMIC_8W_MODE)
        {
            if ((index%2) && (!is_dump_cfg))
            {
                DRV_DBG_INFO("ERROR!! get tbl_id %d index must be even! now index = %d\n", tbl_id, index);
                return DRV_E_INVALID_INDEX;
            }

            *hw_addr = hw_data_base + (index - DYNAMIC_START_INDEX(lchip, tbl_id, blk_id_tmp)) * DRV_ADDR_BYTES_PER_ENTRY;
        }
        else if (DYNAMIC_ACCESS_MODE(lchip, tbl_id) == DYNAMIC_16W_MODE)
        {
            if ((index%4) && (!is_dump_cfg))
            {
                DRV_DBG_INFO("ERROR!! get tbl_id %d index must be times of 4! now index = %d\n", tbl_id, index);
                return DRV_E_INVALID_INDEX;
            }

            *hw_addr = hw_data_base + (index - DYNAMIC_START_INDEX(lchip, tbl_id, blk_id_tmp)) * DRV_ADDR_BYTES_PER_ENTRY;
        }
        else
        {
            *hw_addr = hw_data_base + (index - DYNAMIC_START_INDEX(lchip, tbl_id, blk_id_tmp)) * hw_addr_size_per_idx * DRV_BYTES_PER_WORD;
        }
    }
    else if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_AD)
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                hw_data_base = TCAM_DATA_BASE(lchip, tbl_id, blk_id, addr_offset);
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                break;
            }
        }
        *hw_addr = hw_data_base + map_index * hw_addr_size_per_idx * DRV_BYTES_PER_WORD;
    }
    else if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_LPM_AD || drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_NAT_AD)
    {
        for (blk_id = 0; blk_id < MAX_LPM_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                hw_data_base = TCAM_DATA_BASE(lchip, tbl_id, blk_id, addr_offset);
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                break;
            }
        }
        *hw_addr = hw_data_base + map_index * hw_addr_size_per_idx * DRV_BYTES_PER_WORD;
    }
    else
    {
        DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));
        hw_data_base = TABLE_DATA_BASE(lchip, tbl_id, addr_offset);

        if (TABLE_ENTRY_TYPE(lchip, tbl_id) == SLICE_Cascade)
        {
            if (index >= (TABLE_MAX_INDEX(lchip, tbl_id)/2))
            {
                map_index = index - (TABLE_MAX_INDEX(lchip, tbl_id)/2);
            }
            else
            {
                map_index = index;
            }
        }
        else
        {
            map_index = index;
        }

        *hw_addr = hw_data_base + map_index *hw_addr_size_per_idx*DRV_BYTES_PER_WORD;
    }

    return DRV_E_NONE;
}

/* Get hardware address according to tablid + index + data/mask flag (only tcam key) */
STATIC int32
_drv_usw_table_get_tcam_hw_addr(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32 *hw_addr, uint32 is_data)
{
    uint8 addr_offset = 0;
    uint32 blk_id = 0;
    uint32 addr_base = 0;
    uint32 map_index = 0;

    if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM)
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                addr_base = is_data ? TCAM_DATA_BASE(lchip, tbl_id, blk_id, addr_offset) : TCAM_MASK_BASE(lchip, tbl_id, blk_id, addr_offset);
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *hw_addr = addr_base + TCAM_KEY_SIZE(lchip, tbl_id)/DRV_BYTES_PER_ENTRY*DRV_ADDR_BYTES_PER_ENTRY* map_index;
                break;
            }
        }
    }
    else if ((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_IP) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_NAT))
    {
        for (blk_id = 0; blk_id < MAX_LPM_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                addr_base = is_data ? TCAM_DATA_BASE(lchip, tbl_id, blk_id, addr_offset) : TCAM_MASK_BASE(lchip, tbl_id, blk_id, addr_offset);
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *hw_addr = addr_base + TCAM_KEY_SIZE(lchip, tbl_id) * map_index;
                break;
            }
        }
    }
    else if(drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_STATIC_TCAM_KEY)
    {
        if ((index >= TCAM_START_INDEX(lchip, tbl_id, 0)) && (index <= TCAM_END_INDEX(lchip, tbl_id, 0)))
        {
            addr_base = is_data ? TCAM_DATA_BASE(lchip, tbl_id, 0, 0) : TCAM_MASK_BASE(lchip, tbl_id, 0, 0);
            map_index = index;

            *hw_addr = addr_base + TCAM_KEY_SIZE(lchip, tbl_id) * map_index;
        }
    }
    else
    {
        DRV_DBG_INFO("ERROR!! tbl_id %d is not tcam key!\n", tbl_id);
        return DRV_E_INVALID_INDEX;
    }

    return DRV_E_NONE;
}

/* Get hardware address according to tablid + index
    flag: for tcam, 0-mask, 1-data
          for sram, 0-not dump, 1-dump
 */
int32
drv_usw_table_get_hw_addr(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32 *hw_addr, uint32 flag)
{
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);
    DRV_INIT_CHECK(lchip);
    if ((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM) ||
        (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_IP) ||
        (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_NAT) ||
        (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_STATIC_TCAM_KEY))
    {
        DRV_IF_ERROR_RETURN(_drv_usw_table_get_tcam_hw_addr(lchip, tbl_id, index, hw_addr, flag));
    }
    else
    {
        DRV_IF_ERROR_RETURN(_drv_usw_table_get_sram_hw_addr(lchip, tbl_id, index, hw_addr, flag));
    }

    return DRV_E_NONE;
}

/**
  @driver get table type interface
*/
uint32
drv_usw_get_table_type (uint8 lchip, tbls_id_t tb_id)
{
    uint32 type = 0;

    if (TABLE_EXT_INFO_PTR(lchip, tb_id))
    {
        type = TABLE_EXT_INFO_TYPE(lchip, tb_id);
    }
    else
    {
        type = DRV_TABLE_TYPE_NORMAL;
    }
    return type;
}

int8 drv_usw_table_is_slice1(uint8 lchip, tbls_id_t tbl_id)
{
    /*No use for usw*/
    return FALSE;
}

int32 drv_usw_mutex_set_lock(uint8 lchip, uint32 lock_id, uint32 mutex_data_addr, uint32 mutex_mask_addr)
{
    uint32 tmp_val32[2] = {0, 0xffffffff};

    drv_usw_chip_read(lchip, mutex_data_addr, &tmp_val32[0]);
    tmp_val32[0] |= (1U << lock_id);
    tmp_val32[1] &= ~(1U << lock_id);
    drv_usw_chip_write(lchip, mutex_data_addr, tmp_val32[0]);
    drv_usw_chip_write(lchip, mutex_mask_addr, tmp_val32[1]);

    drv_usw_chip_read(lchip, mutex_data_addr, &tmp_val32[0]);
    if ((tmp_val32[0] >> lock_id) & 0x1)
    {
        return DRV_E_NONE;  /* obtain success */
    }
    else
    {
        return DRV_E_OCCPANCY;
    }
}

int32 drv_usw_mutex_set_unlock(uint8 lchip, uint32 lock_id, uint32 mutex_data_addr, uint32 mutex_mask_addr)
{
    uint32 tmp_val32[2] = {0, 0xffffffff};

    drv_usw_chip_read(lchip, mutex_data_addr, &tmp_val32[0]);
    tmp_val32[0] &= ~(1U << lock_id);
    tmp_val32[1] &= ~(1U << lock_id);
    drv_usw_chip_write(lchip, mutex_data_addr, tmp_val32[0]);
    drv_usw_chip_write(lchip, mutex_mask_addr, tmp_val32[1]);

    drv_usw_chip_read(lchip, mutex_data_addr, &tmp_val32[0]);
    if ((tmp_val32[0] >> lock_id) & 0x1)
    {
        return DRV_E_OCCPANCY;
    }
    else
    {
        return DRV_E_NONE;
    }
}

int32 drv_usw_mcu_lock(uint8 lchip, uint32 lock_id, uint8 mcu_id)
{
    int32  ret = 0;
    uint32 tmp_val32 = 0;
    uint32 lock_stat = 0;
    uint32 mutex_data_addr = 0;
    uint32 mutex_mask_addr = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;

    /*1. para check*/
    if ((DRV_MCU_LOCK_NONE == lock_id) || (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING))
    {
        return DRV_E_NONE;
    }
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }
    if (DRV_MCU_LOCK_MAX <= lock_id)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /*2. get mutex address*/
    if(DRV_GET_MCU_ADDR)
    {
        DRV_GET_MCU_ADDR(mcu_id, &mutex_data_addr, &mutex_mask_addr);
    }
    else return DRV_E_INVALID_PTR;

    /*3. lock status check*/
    drv_usw_chip_read(lchip, mutex_data_addr, &tmp_val32);
    lock_stat = (tmp_val32 >> lock_id) & 0x1;
    if(lock_stat)
    {
        return DRV_E_NONE;
    }

    /*4. mutex operation*/
    ret = drv_usw_mutex_set_lock(lchip, lock_id, mutex_data_addr, mutex_mask_addr);
    while((DRV_E_OCCPANCY == ret) && (--timeout))
    {
        ret = drv_usw_mutex_set_lock(lchip, lock_id, mutex_data_addr, mutex_mask_addr);
        if (DRV_E_NONE == ret)
        {
            break;
        }
    }

    /*5. success check*/
    if(DRV_E_NONE == ret)
    {
        return DRV_E_NONE;
    }
    else
    {
        return DRV_E_TIME_OUT;
    }
}

int32 drv_usw_mcu_unlock(uint8 lchip, uint32 lock_id, uint8 mcu_id)
{
    int32  ret = 0;
    uint32 tmp_val32 = 0;
    uint32 lock_stat = 0;
    uint32 mutex_data_addr = 0;
    uint32 mutex_mask_addr = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;

    /*1. para check*/
    if ((DRV_MCU_LOCK_NONE == lock_id) || (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING))
    {
        return DRV_E_NONE;
    }
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }
    if (DRV_MCU_LOCK_MAX <= lock_id)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /*2. get mutex address*/
    if(DRV_GET_MCU_ADDR)
    {
        DRV_GET_MCU_ADDR(mcu_id, &mutex_data_addr, &mutex_mask_addr);
    }
    else return DRV_E_INVALID_PTR;

    /*3. lock status check*/
    drv_usw_chip_read(lchip, mutex_data_addr, &tmp_val32);
    lock_stat = (tmp_val32 >> lock_id) & 0x1;
    if(!lock_stat)
    {
        return DRV_E_NONE;
    }

    /*4. mutex operation*/
    ret = drv_usw_mutex_set_unlock(lchip, lock_id, mutex_data_addr, mutex_mask_addr);
    while((DRV_E_OCCPANCY == ret) && (--timeout))
    {
        ret = drv_usw_mutex_set_unlock(lchip, lock_id, mutex_data_addr, mutex_mask_addr);
        if (DRV_E_NONE == ret)
        {
            break;
        }
    }

    /*5. success check*/
    if(DRV_E_NONE == ret)
    {
        return DRV_E_NONE;
    }
    else
    {
        return DRV_E_TIME_OUT;
    }
}

int32 drv_usw_mcu_tbl_lock(uint8 lchip, tbls_id_t tbl_id, uint32 op)
{
    uint8  mcu_id = 0xff;
    uint32 lock   = DRV_MCU_LOCK_NONE;

    if (DRV_IOC_READ == op)
    {
        return DRV_E_NONE;
    }

    if(DRV_GET_MCU_LOCK_ID)
    {
        DRV_GET_MCU_LOCK_ID(lchip, tbl_id, &mcu_id, &lock);
    }
    else return DRV_E_INVALID_PTR;

    DRV_IF_ERROR_RETURN(drv_usw_mcu_lock(lchip, lock, mcu_id));

    return DRV_E_NONE;
}

int32 drv_usw_mcu_tbl_unlock(uint8 lchip, tbls_id_t tbl_id, uint32 op)
{
    uint8  mcu_id = 0xff;
    uint32 lock   = DRV_MCU_LOCK_NONE;

    if (DRV_IOC_READ == op)
    {
        return DRV_E_NONE;
    }

    if(DRV_GET_MCU_LOCK_ID)
    {
        DRV_GET_MCU_LOCK_ID(lchip, tbl_id, &mcu_id, &lock);
    }
    else return DRV_E_INVALID_PTR;

    DRV_IF_ERROR_RETURN(drv_usw_mcu_unlock(lchip, lock, mcu_id));

    return DRV_E_NONE;
}

