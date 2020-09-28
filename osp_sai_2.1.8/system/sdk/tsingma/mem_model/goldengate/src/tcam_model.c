/****************************************************************************
 * tcam_model.c: Tcam model interfaces and database
 *
 * (C) Copyright Centec Networks Inc. All rights reserved.
 *
 * Revision     : V1.0
 * Author       : ZhouW
 * Date         : 2011-03-15
 ****************************************************************************/
#include "tcam_model.h"

cm_tcam_info_t tcam_info;

/* ########## internal utility defined here ############################ */
STATIC bool
_tcam_model_addr_check(uint8 chip_id_offset, uintptr model_addr,
                                  uint8* key_type, bool* is_data_addr)
{
    uintptr max_model_dataddr = 0, max_model_maskaddr = 0;
    uintptr max_lpm_model_data_addr = 0;
    uintptr max_lpm_model_mask_addr = 0;
    uint32 couple_mode = 0;

    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    TCAM_MODEL_PTR_VALID_CHECK(key_type);
    TCAM_MODEL_PTR_VALID_CHECK(is_data_addr);

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    max_model_dataddr = (uintptr)tcam_info.int_tcam_data_base[chip_id_offset]
                        + ((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    max_model_maskaddr = (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset]
                        + ((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);

    max_lpm_model_data_addr = (uintptr)tcam_info.int_lpm_tcam_data_base[chip_id_offset]
                            + ((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) * DRV_LPM_KEY_BYTES_PER_ENTRY
                                + ((DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY));

    max_lpm_model_mask_addr = (uintptr)tcam_info.int_lpm_tcam_mask_base[chip_id_offset]
                            + ((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) * DRV_LPM_KEY_BYTES_PER_ENTRY
                                + ((DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY));

    if ((model_addr >= (uintptr)tcam_info.int_tcam_data_base[chip_id_offset])
        && (model_addr < max_model_dataddr))
    {
        *key_type = 0; /* internal tcam */
        *is_data_addr = TRUE;
        return TRUE;
    }

    if ((model_addr >= (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset])
        && (model_addr < max_model_maskaddr))
    {
        *key_type = 0;
        *is_data_addr = FALSE;
        return TRUE;
    }

    /* Add LPM Tcam key check */
    if ((model_addr >= (uintptr)tcam_info.int_lpm_tcam_data_base[chip_id_offset])
        && (model_addr < max_lpm_model_data_addr))
    {
        *key_type = 2; /* lpm tcam */
        *is_data_addr = TRUE;
        return TRUE;
    }

    if ((model_addr >= (uintptr)tcam_info.int_lpm_tcam_mask_base[chip_id_offset])
        && (model_addr < max_lpm_model_mask_addr))
    {
        *key_type = 2;
        *is_data_addr = FALSE;
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
 * Name       : tcam_model_initialize
 * Purpose    : The function initialize chip's tcam.
 * Input      : chip_id_offset - chip id infomation
 * Output     : N/A
 * Return     : DRV_E_NONE
 *              Other = ErrCode
 * Note:      : N/A
****************************************************************************/
int32 tcam_model_initialize(uint8 chip_id_offset)
{
    uint32 couple_mode = 0;

    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    tcam_info.max_int_tcam_data_base = DRV_TCAM_KEY0_BASE_4W
                             + ((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    tcam_info.max_int_tcam_mask_base = DRV_TCAM_MASK0_BASE_4W
                             + ((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    tcam_info.max_int_lpm_tcam_data_base = DRV_LPM_TCAM_DATA_ASIC_BASE0
                             + ((DRV_LPM_TCAM_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    tcam_info.max_int_lpm_tcam_mask_base = DRV_LPM_TCAM_MASK_ASIC_BASE0
                             + ((DRV_LPM_TCAM_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);

    if (tcam_info.tcam_model_inited[chip_id_offset])
    {
        return DRV_E_NONE;
    }

    /* Malloc Tcam software memory process */
    tcam_info.int_tcam_data_base[chip_id_offset] = (uint32 *)sal_malloc((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    tcam_info.int_tcam_mask_base[chip_id_offset] = (uint32 *)sal_malloc((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    /* Each tcam entry use 1 wbit */
    tcam_info.int_tcam_wbit[chip_id_offset] = (uint32 *)sal_malloc((DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) / 8);

    tcam_info.int_lpm_tcam_data_base[chip_id_offset] = (uint32 *)sal_malloc((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) * DRV_LPM_KEY_BYTES_PER_ENTRY
                                                                            + (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM ) * DRV_BYTES_PER_ENTRY);
    tcam_info.int_lpm_tcam_mask_base[chip_id_offset] = (uint32 *)sal_malloc((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) * DRV_LPM_KEY_BYTES_PER_ENTRY
                                                                            + (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM ) * DRV_BYTES_PER_ENTRY);
    tcam_info.int_lpm_tcam_wbit[chip_id_offset] = (uint32 *)sal_malloc(((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) + (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM )) / 8);


    if (!tcam_info.int_tcam_data_base[chip_id_offset]
       || !tcam_info.int_tcam_mask_base[chip_id_offset]
       || !tcam_info.int_tcam_wbit[chip_id_offset]
       || !tcam_info.int_lpm_tcam_data_base[chip_id_offset]
       || !tcam_info.int_lpm_tcam_mask_base[chip_id_offset]
       || !tcam_info.int_lpm_tcam_wbit[chip_id_offset])
    {
        SAL_LOG_FATAL("******** When Tcam allocation, no enough memory!!********\n");
        exit(1);
    }

    sal_memset(tcam_info.int_tcam_data_base[chip_id_offset], 0, (DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.int_tcam_mask_base[chip_id_offset], 0, (DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.int_tcam_wbit[chip_id_offset], 0,(DRV_TCAM_KEY_MAX_ENTRY_NUM << couple_mode) / 8);
    sal_memset(tcam_info.int_lpm_tcam_data_base[chip_id_offset], 0, ((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) * DRV_LPM_KEY_BYTES_PER_ENTRY
                                                                            + (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM ) * DRV_BYTES_PER_ENTRY));
    sal_memset(tcam_info.int_lpm_tcam_mask_base[chip_id_offset], 0, ((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) * DRV_LPM_KEY_BYTES_PER_ENTRY
                                                                            + (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM ) * DRV_BYTES_PER_ENTRY));
    sal_memset(tcam_info.int_lpm_tcam_wbit[chip_id_offset], 0,((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode) + (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM )) / 8);

    tcam_info.tcam_block_entry_num[chip_id_offset][0] = (DRV_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode);
    tcam_info.tcam_block_entry_num[chip_id_offset][1] = (DRV_TCAM_KEY1_MAX_ENTRY_NUM << couple_mode);
    tcam_info.tcam_block_entry_num[chip_id_offset][2] = (DRV_TCAM_KEY2_MAX_ENTRY_NUM << couple_mode);
    tcam_info.tcam_block_entry_num[chip_id_offset][3] = (DRV_TCAM_KEY3_MAX_ENTRY_NUM << couple_mode);
    tcam_info.tcam_block_entry_num[chip_id_offset][4] = (DRV_TCAM_KEY4_MAX_ENTRY_NUM << couple_mode);
    tcam_info.tcam_block_entry_num[chip_id_offset][5] = (DRV_TCAM_KEY5_MAX_ENTRY_NUM << couple_mode);
    tcam_info.tcam_block_entry_num[chip_id_offset][6] = (DRV_TCAM_KEY6_MAX_ENTRY_NUM << couple_mode);

    tcam_info.lpm_tcam_block_entry_num[chip_id_offset][0] = (DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode);
    tcam_info.lpm_tcam_block_entry_num[chip_id_offset][1] = (DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM );

    tcam_info.tcam_model_inited[chip_id_offset] = TRUE;

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : tcam_model_release
 * Purpose     : The function release tcam model's memory.
 * Input       : chip_id_offset - chip id infomation.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 tcam_model_release(uint8 chip_id_offset)
{
    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    sal_free(tcam_info.int_tcam_data_base[chip_id_offset]);
    tcam_info.int_tcam_data_base[chip_id_offset] = NULL;

    sal_free(tcam_info.int_tcam_mask_base[chip_id_offset]);
    tcam_info.int_tcam_mask_base[chip_id_offset] = NULL;

    sal_free(tcam_info.int_tcam_wbit[chip_id_offset]);
    tcam_info.int_tcam_wbit[chip_id_offset] = NULL;

    sal_free(tcam_info.int_lpm_tcam_data_base[chip_id_offset]);
    tcam_info.int_lpm_tcam_data_base[chip_id_offset] = NULL;

    sal_free(tcam_info.int_lpm_tcam_mask_base[chip_id_offset]);
    tcam_info.int_lpm_tcam_mask_base[chip_id_offset] = NULL;

    sal_free(tcam_info.int_lpm_tcam_wbit[chip_id_offset]);
    tcam_info.int_lpm_tcam_wbit[chip_id_offset] = NULL;

    tcam_info.tcam_model_inited[chip_id_offset] = FALSE;

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : tcam_model_write
 * Purpose     : The function read data from tcam to controller.
 * Input       : chip_id_offset - chip id infomation.
                 sw_model_addr - tcam memory model's address.
                 data - the writed data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 tcam_model_write(uint8 chip_id_offset, uintptr sw_model_addr, uint32* data, int32 len)
{
    uint8 key_type = 0;
    bool is_data_addr = TRUE;

    /* check para */
    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    TCAM_MODEL_PTR_VALID_CHECK(data);

    /* Before write tcam, check whether the tcam model is inited or not */
    if(!tcam_info.tcam_model_inited[chip_id_offset])
    {
        DRV_DBG_INFO(" \n You can not write tcam model before initialized it.\n");
        return DRV_E_TCAM_RESET;
    }

    if (!_tcam_model_addr_check(chip_id_offset, sw_model_addr,
                               &key_type, &is_data_addr))
    {
        DRV_DBG_INFO(" \n Check the address ERROR when write tcam! \n");
        return DRV_E_INVALID_ADDR;
    }


    /* write data in the tcam model address */
    sal_memcpy((uint8*)sw_model_addr, data, len);

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : tcam_model_read
 * Purpose     : The function read data from tcam to controller.
 * Input       : chip_id_offset - chip id infomation.
                 sw_model_addr - tcam memory model's address.
                 data - the read data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 tcam_model_read(uint8 chip_id_offset, uintptr sw_model_addr, uint32* data, int32 len)
{
    uint8 key_type = TRUE;
    bool is_data_addr = TRUE;

    /* check para */
    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    TCAM_MODEL_PTR_VALID_CHECK(data);

    /* Before read tcam, check whether the tcam model is inited or not */
    if (!tcam_info.tcam_model_inited[chip_id_offset])
    {
        DRV_DBG_INFO(" \n You cant to write sim tcam before initialized it.\n");
        return DRV_E_TCAM_RESET;
    }

    if (!_tcam_model_addr_check(chip_id_offset, sw_model_addr,
                               &key_type, &is_data_addr))
    {
        DRV_DBG_INFO(" \n Check the address ERROR when Read tcam! \n");
        return DRV_E_INVALID_ADDR;
    }

    sal_memcpy(data, (uint8*)sw_model_addr, len );

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : tcam_model_remove
 * Purpose     : The function remove data from tcam to controller.
 * Input       : chip_id_offset - chip id infomation.
                 sw_model_addr - tcam memory model's address.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
tcam_model_remove(uint8 chip_id_offset, uintptr sw_model_addr, uint32 entry_word_len)
{
    bool is_data_addr;
    uint8 key_type = 0;
    //uintptr model_data_base, model_mask_base, offset;

    if (!_tcam_model_addr_check(chip_id_offset, sw_model_addr,
                               &key_type, &is_data_addr))
    {
        DRV_DBG_INFO(" \n Check the address ERROR when remove tcam entry! \n");
        return DRV_E_INVALID_ADDR;
    }
    sal_memset((uint32 *)sw_model_addr, 0, entry_word_len);

#if 0
    switch(key_type)
    {
        case 0:
            model_data_base = (uintptr)tcam_info.int_tcam_data_base[chip_id_offset];
            model_mask_base = (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset];
            break;
        case 1:
            model_data_base = (uintptr)tcam_info.ext_tcam_data_base[chip_id_offset];
            model_mask_base = (uintptr)tcam_info.ext_tcam_mask_base[chip_id_offset];
            break;
        case 2:
            model_data_base = (uintptr)tcam_info.int_lpm_tcam_data_base[chip_id_offset];
            model_mask_base = (uintptr)tcam_info.int_lpm_tcam_mask_base[chip_id_offset];
            break;
        default:
            break;
    }

    if (is_data_addr)
    {
        offset = sw_model_addr - model_data_base;
    }
    else
    {
        offset = sw_model_addr - model_mask_base;
    }

    if (is_data_addr)
    {
        sal_memset((uint32 *)sw_model_addr, 0, entry_word_len);
        sal_memset((uint32 *)(model_mask_base+offset), 0, entry_word_len);
    }
    else
    {
        sal_memset((uint32 *)(model_data_base+offset), 0, entry_word_len);
        sal_memset((uint32 *)sw_model_addr, 0, entry_word_len);
    }
#endif

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : tcam_model_lookup
 * Purpose     : The function lookup operation in tcam model.
 * Input       : chip_id_offset - chip id infomation.
                 tbl_id - tcam tabel ID.
                 data - the lookup data.
                 tcam_index - return hited tcam's index.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 tcam_model_lookup(uint8 chip_id_offset, uint32 tbl_id, uint32* data, uint32* tcam_index)
{
    uint32  *sw_vbit_base = NULL;//*sw_mask_base = NULL,
    uint32 *sw_data_base = NULL;
    uint32 *key_data_base = NULL, *key_mask_base = NULL;
    uintptr key_data_base_addr = 0, key_mask_base_addr = 0;
    uint32 tcam_entry_num = 0;
    uint32 tmp_tbl_index = 0, tmp_word_offset = 0;
    uint32 tmp_tcam_data = 0, tmp_tcam_mask = 0, tmp_key = 0;
    uint8 tcam_type = 0;
    uint32 per_index_addr_offset = 0, per_index_entry_offset = 0;
    uint32 tbl_entry_num_offset = 0, tcam_640_offset = 0;
    uint32 blk_id = 0, start_index = 0, end_index = 0, index_id = 0;
    uint8 key_matched = FALSE;
    uint8 flow_tcam_line_bitmap = 0;
    char key_name[100]={0};
    uint32 couple_mode = 0;

    /* check para */
    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    TCAM_MODEL_PTR_VALID_CHECK(data);
    TCAM_MODEL_PTR_VALID_CHECK(tcam_index);

    if (!tcam_info.tcam_model_inited[chip_id_offset])
    {
        DRV_DBG_INFO("@@ TCAM model do not initilized!\n");
        return DRV_E_NO_MEMORY;
    }

    if (TABLE_MAX_INDEX(tbl_id) == 0)
    {
        *tcam_index = DRV_TCAM_INVALID_INDEX;
        return DRV_E_NONE;
    }

    if (drv_goldengate_table_is_tcam_key(tbl_id))
    {
        sw_data_base = tcam_info.int_tcam_data_base[chip_id_offset];
        sw_vbit_base = tcam_info.int_tcam_wbit[chip_id_offset];
        tcam_type = 1;
        drv_goldengate_get_tcam_640_offset(tbl_id,&tcam_640_offset);
    }
    else if(drv_goldengate_table_is_lpm_tcam_key(tbl_id))
    {
        sw_data_base = tcam_info.int_lpm_tcam_data_base[chip_id_offset];
        sw_vbit_base = tcam_info.int_lpm_tcam_wbit[chip_id_offset];
        tcam_type = 3;
    }
    else
    {
        tcam_type = 0;
        DRV_DBG_INFO("@@ Check address ERROR when Lookup TCAM!\n");
        return DRV_E_INVALID_ADDR;
    }

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    if(3 == tcam_type)
    {
        per_index_addr_offset = TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_WORD;

        if(drv_goldengate_table_is_lpm_tcam_key_ip_prefix(tbl_id))
        {
            per_index_entry_offset = TCAM_KEY_SIZE(tbl_id)/DRV_LPM_KEY_BYTES_PER_ENTRY;
        }
        else
        {
            per_index_entry_offset = TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_ENTRY;
        }
    }
    else
    {
    #ifdef TCAM640
        if (TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_ENTRY == 8)
        {
            per_index_addr_offset = TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_WORD/2;
            per_index_entry_offset = 4;
        }
        else
    #endif
        {
            per_index_addr_offset = TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_WORD;
            per_index_entry_offset = TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_ENTRY;
        }
    }

    for (blk_id = 0,index_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
    {
        if ((tcam_type!=3) && (!IS_BIT_SET(TCAM_BITMAP(tbl_id), blk_id)))
        {
            /* If this tcam can be used for the tbl_id, but not enabled,
            increase the index_id, for use in tcam result merge */
            flow_tcam_line_bitmap = drv_goldengate_get_flow_tcam_line_bitmap(tbl_id);
            if(IS_BIT_SET(flow_tcam_line_bitmap,blk_id))
            {
                tcam_index[index_id] = DRV_TCAM_INVALID_INDEX;
                index_id ++;
            }

            continue;
        }

        if (tcam_type != 3)
        {
            start_index = TCAM_START_INDEX(tbl_id, blk_id);
            end_index = TCAM_END_INDEX(tbl_id, blk_id) + 1;
        }
        else
        {
            start_index = 0;
            end_index = TABLE_MAX_INDEX(tbl_id);
        }

        /* Get the table's software data and mask address */
        tcam_model_get_sw_address(chip_id_offset, tbl_id,start_index,&key_data_base_addr, TRUE);
        tcam_model_get_sw_address(chip_id_offset, tbl_id,start_index,&key_mask_base_addr, FALSE);
        key_data_base = (uint32 *)key_data_base_addr;
        key_mask_base = (uint32 *)key_mask_base_addr;
/*SDK Modify {*/
        if(3 == tcam_type)
        {
            if(drv_goldengate_table_is_lpm_tcam_key_ip_prefix(tbl_id))
            {
                tbl_entry_num_offset = ((uint8*)key_data_base - (uint8*)sw_data_base)/DRV_LPM_KEY_BYTES_PER_ENTRY;
            }
            else
            {
                tbl_entry_num_offset = (DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM << couple_mode)
                              + (key_data_base - (sw_data_base+ ((DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM <<couple_mode) *DRV_LPM_KEY_BYTES_PER_ENTRY)/4))*4/DRV_BYTES_PER_ENTRY;
            }
        }
        else
        {
            tbl_entry_num_offset = (key_data_base - sw_data_base)*4/DRV_BYTES_PER_ENTRY;
        }

        /* Search in the table range */
        for (tmp_tbl_index = 0; tmp_tbl_index < (end_index - start_index); ++tmp_tbl_index)
        {
            tcam_entry_num = tmp_tbl_index * per_index_entry_offset + tbl_entry_num_offset;

            /* Check the tcam entry's Wbit */
            if (!(IS_BIT_SET(*(sw_vbit_base + tcam_entry_num/DRV_BITS_PER_WORD), tcam_entry_num%DRV_BITS_PER_WORD)))
            {
                continue;
            }

            /* Compare key according to the word unit */
            for (tmp_word_offset = 0; tmp_word_offset < TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_WORD; ++tmp_word_offset)
            {

                tmp_tcam_mask = *(key_mask_base + tmp_tbl_index * per_index_addr_offset + tmp_word_offset);
                tmp_tcam_data = *(key_data_base + tmp_tbl_index * per_index_addr_offset + tmp_word_offset);
                tmp_key = *(data + tmp_word_offset);

#if 0
                sal_printf("tbl:%d, index:%d, tmp_word_offset:%d, data:0x%08x, key:0x%08x, mask:0x%08x\n",
                    tbl_id, tmp_tbl_index, tmp_word_offset, tmp_tcam_data, tmp_key, tmp_tcam_mask);
#endif
                if ((tmp_tcam_data & tmp_tcam_mask) != (tmp_key & tmp_tcam_mask))
                {
                    break;
                }

                if (tmp_word_offset == TCAM_KEY_SIZE(tbl_id)/DRV_BYTES_PER_WORD -1)
                {
                    if (3 == tcam_type)
                    {
                        /* lpm tcam, return the index according to tcam entry unit(40bits) */
                        if (TABLE_EXT_INFO_TYPE(tbl_id) == EXT_INFO_TYPE_LPM_TCAM_NAT)
                        {
                            tcam_index[index_id] =  (tmp_tbl_index + start_index);
                        }
                        else
                        {
                            tcam_index[index_id] =  (tmp_tbl_index + start_index)*per_index_entry_offset;
                        }

                        drv_goldengate_get_tbl_string_by_id(tbl_id, key_name);
                        sal_printf("+++++++++++ Tcam Hit lpm tcam[%s] index %d\n", key_name, tcam_index[index_id]  );
                        return DRV_E_NONE;
                    }
                    else
                    {
                        /* Match, return the index according to tcam entry unit(80bits) in the whole Tcam */
                        tcam_index[index_id] =  (tmp_tbl_index + start_index)*per_index_entry_offset/2 ;
                        drv_goldengate_get_tbl_string_by_id(tbl_id, key_name);
                        sal_printf("+++++++++++ Tcam Hit  normal tcam[%s] block %d, index %d\n",key_name, index_id, tcam_index[index_id]  );
                    }

                    key_matched = TRUE;
                    break;
                    //return DRV_E_NONE;
                }
            }
            if (key_matched)
            {
                break;
            }
            else
            {
                tcam_index[index_id] = DRV_TCAM_INVALID_INDEX;
            }
        }
        index_id ++;
    }
    //*tcam_index = DRV_TCAM_INVALID_INDEX; /* Can not match in tcam*/
/*SDK Modify }*/
    return DRV_E_NONE;
}

/*SDK Modify {*/
int32
tcam_model_get_sw_address(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index,
                                        uintptr* sw_model_addr,bool is_data)
{
    uintptr sw_base_addr = 0, hw_base_addr = 0, key_base_addr = 0, hw_addr_offset_num = 0;
    uint8 blk_id = 0;
    uint8 addr_offset = 0;
    uint32 lpm_key_size = 0;
    uint32 offset_index = 0;
    uint32 offset = 0;

    if (drv_goldengate_table_is_tcam_key(tbl_id))
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {
            if (!IS_BIT_SET(TCAM_BITMAP(tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(tbl_id, blk_id)) && (index <=TCAM_END_INDEX(tbl_id, blk_id)))
            {
                if (is_data)
                {
                    hw_base_addr = DRV_TCAM_KEY0_BASE_4W;
                    key_base_addr = TCAM_DATA_BASE(tbl_id, blk_id, 0);
                    sw_base_addr = (uintptr)tcam_info.int_tcam_data_base[chip_id_offset];
                }
                else
                {
                    hw_base_addr = DRV_TCAM_MASK0_BASE_4W;
                    key_base_addr = TCAM_MASK_BASE(tbl_id, blk_id, 0);
                    sw_base_addr = (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset];
                }
                offset_index = index - TCAM_START_INDEX(tbl_id, blk_id);
                 offset = (( key_base_addr - hw_base_addr) / DRV_ADDR_BYTES_PER_ENTRY)*DRV_BYTES_PER_ENTRY ;
                *sw_model_addr = sw_base_addr + offset   + offset_index*TCAM_KEY_SIZE(tbl_id);

                break;
            }

        }

    }
    else if(drv_goldengate_table_is_lpm_tcam_key(tbl_id))
    {
        if (is_data)
        {
            sw_base_addr = (uintptr)tcam_info.int_lpm_tcam_data_base[chip_id_offset];
        }
        else
        {
            sw_base_addr = (uintptr)tcam_info.int_lpm_tcam_mask_base[chip_id_offset];
        }

        if (TABLE_EXT_INFO_TYPE(tbl_id) == EXT_INFO_TYPE_LPM_TCAM_NAT)
        {
            hw_addr_offset_num = tcam_info.lpm_tcam_block_entry_num[chip_id_offset][0];
            offset =  ((TABLE_DATA_BASE(tbl_id, addr_offset) - DRV_LPM_TCAM_DATA_ASIC_BASE1) / DRV_ADDR_BYTES_PER_ENTRY)*DRV_BYTES_PER_ENTRY;
        }
        else
        {
            offset =  ((TABLE_DATA_BASE(tbl_id, addr_offset) - DRV_LPM_TCAM_DATA_ASIC_BASE0) / DRV_LPM_KEY_BYTES_PER_ENTRY)*DRV_LPM_KEY_BYTES_PER_ENTRY;
        }

        lpm_key_size = TCAM_KEY_SIZE(tbl_id);
        *sw_model_addr = sw_base_addr+ offset  + hw_addr_offset_num*DRV_LPM_KEY_BYTES_PER_ENTRY  + index*lpm_key_size;
    }
    else
    {
        sal_printf("%% ERROR! Invalid Tcam Key = %d !!\n", tbl_id);
        return DRV_E_INVALID_ADDR;
    }

    return DRV_E_NONE;
}

int32
tcam_model_get_blk_sw_addr_start_index(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index,
                                        uintptr*sw_model_addr,uint32 *start_index, bool is_data)
{
    uintptr sw_base_addr = 0, hw_base_addr = 0, key_base_addr = 0;
    uint8 blk_id = 0;
    uint32 offset_index = 0;
    uint32 offset = 0;

    if (drv_goldengate_table_is_tcam_key(tbl_id))
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {

            if (!IS_BIT_SET(TCAM_BITMAP(tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(tbl_id, blk_id)) && (index <=TCAM_END_INDEX(tbl_id, blk_id)))
            {
                if (is_data)
                {
                    hw_base_addr = DRV_TCAM_KEY0_BASE_4W;
                    key_base_addr = TCAM_DATA_BASE(tbl_id, blk_id, 0);
                    sw_base_addr = (uintptr)tcam_info.int_tcam_data_base[chip_id_offset];
                }
                else
                {
                    hw_base_addr = DRV_TCAM_MASK0_BASE_4W;
                    key_base_addr = TCAM_MASK_BASE(tbl_id, blk_id, 0);
                    sw_base_addr = (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset];
                }

                offset_index = index - TCAM_START_INDEX(tbl_id, blk_id);
                offset = (( key_base_addr - hw_base_addr) / DRV_ADDR_BYTES_PER_ENTRY)*DRV_BYTES_PER_ENTRY ;
                *sw_model_addr = sw_base_addr + offset   + offset_index*TCAM_KEY_SIZE(tbl_id);

                *start_index = TCAM_START_INDEX(tbl_id, blk_id);

                break;
            }

        }
    }
    else
    {
        sal_printf("%% ERROR! Invalid Tcam Key = %d !!\n", tbl_id);
        return DRV_E_INVALID_ADDR;
    }

    return DRV_E_NONE;
}

/*SDK Modify }*/


