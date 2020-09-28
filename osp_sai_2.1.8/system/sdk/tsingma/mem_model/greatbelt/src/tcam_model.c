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

    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    TCAM_MODEL_PTR_VALID_CHECK(key_type);
    TCAM_MODEL_PTR_VALID_CHECK(is_data_addr);

    max_model_dataddr = (uintptr)tcam_info.int_tcam_data_base[chip_id_offset]
        + (DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    max_model_maskaddr = (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset]
        + (DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

    max_lpm_model_data_addr = (uintptr)tcam_info.int_lpm_tcam_data_base[chip_id_offset]
        + (DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

    max_lpm_model_mask_addr = (uintptr)tcam_info.int_lpm_tcam_mask_base[chip_id_offset]
        + (DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

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

    max_model_dataddr = (uintptr)tcam_info.ext_tcam_data_base[chip_id_offset]
        + (DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    max_model_maskaddr = (uintptr)tcam_info.ext_tcam_mask_base[chip_id_offset]
        + (DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

    if ((model_addr >= (uintptr)tcam_info.ext_tcam_data_base[chip_id_offset])
        && (model_addr < max_model_dataddr))
    {
        *key_type = 1; /* external tcam */
        *is_data_addr = TRUE;
        return TRUE;
    }

    if ((model_addr >= (uintptr)tcam_info.ext_tcam_mask_base[chip_id_offset])
        && (model_addr < max_model_maskaddr))
    {
        *key_type = 1;
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
 * Input      : chip_id_offset - chip id information
 * Output     : N/A
 * Return     : DRV_E_NONE
 *              Other = ErrCode
 * Note:      : N/A
****************************************************************************/
int32
tcam_model_initialize(uint8 chip_id_offset)
{
    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    tcam_info.max_int_tcam_data_base = DRV_INT_TCAM_KEY_DATA_ASIC_BASE
        + (DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.max_int_tcam_mask_base = DRV_INT_TCAM_KEY_MASK_ASIC_BASE
        + (DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.max_ext_tcam_data_base = DRV_EXT_TCAM_KEY_DATA_ASIC_BASE
        + (DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.max_ext_tcam_mask_base = DRV_EXT_TCAM_KEY_MASK_ASIC_BASE
        + (DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.max_int_lpm_tcam_data_base = DRV_INT_LPM_TCAM_DATA_ASIC_BASE
        + (DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.max_int_lpm_tcam_mask_base = DRV_INT_LPM_TCAM_MASK_ASIC_BASE
        + (DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

    if (tcam_info.tcam_model_inited[chip_id_offset])
    {
        return DRV_E_NONE;
    }

    /* Malloc Tcam software memory process */
    tcam_info.int_tcam_data_base[chip_id_offset] = sal_malloc(DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.int_tcam_mask_base[chip_id_offset] = sal_malloc(DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    /* Each tcam entry use 1 wbit */
    tcam_info.int_tcam_wbit[chip_id_offset] = sal_malloc(DRV_INT_TCAM_KEY_MAX_ENTRY_NUM / 8);

    tcam_info.int_lpm_tcam_data_base[chip_id_offset] = sal_malloc(DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.int_lpm_tcam_mask_base[chip_id_offset] = sal_malloc(DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.int_lpm_tcam_wbit[chip_id_offset] = sal_malloc(DRV_INT_LPM_TCAM_MAX_ENTRY_NUM / 8);

    tcam_info.ext_tcam_data_base[chip_id_offset] = sal_malloc(DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.ext_tcam_mask_base[chip_id_offset] = sal_malloc(DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    tcam_info.ext_tcam_wbit[chip_id_offset] = sal_malloc(DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM / 8);

    if (!tcam_info.int_tcam_data_base[chip_id_offset]
        || !tcam_info.int_tcam_mask_base[chip_id_offset]
        || !tcam_info.int_tcam_wbit[chip_id_offset]
        || !tcam_info.int_lpm_tcam_data_base[chip_id_offset]
        || !tcam_info.int_lpm_tcam_mask_base[chip_id_offset]
        || !tcam_info.int_lpm_tcam_wbit[chip_id_offset]
        || !tcam_info.ext_tcam_data_base[chip_id_offset]
        || !tcam_info.ext_tcam_mask_base[chip_id_offset]
        || !tcam_info.ext_tcam_wbit[chip_id_offset])
    {
        SAL_LOG_FATAL("******** When Tcam allocation, no enough memory!!********\n");
        exit(1);
    }

    sal_memset(tcam_info.int_tcam_data_base[chip_id_offset], 0, DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.int_tcam_mask_base[chip_id_offset], 0, DRV_INT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.int_tcam_wbit[chip_id_offset], 0, DRV_INT_TCAM_KEY_MAX_ENTRY_NUM / 8);
    sal_memset(tcam_info.int_lpm_tcam_data_base[chip_id_offset], 0, DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.int_lpm_tcam_mask_base[chip_id_offset], 0, DRV_INT_LPM_TCAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.int_lpm_tcam_wbit[chip_id_offset], 0, DRV_INT_LPM_TCAM_MAX_ENTRY_NUM / 8);
    sal_memset(tcam_info.ext_tcam_data_base[chip_id_offset], 0, DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.ext_tcam_mask_base[chip_id_offset], 0, DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sal_memset(tcam_info.ext_tcam_wbit[chip_id_offset], 0, DRV_EXT_TCAM_KEY_MAX_ENTRY_NUM / 8);

    tcam_info.tcam_model_inited[chip_id_offset] = TRUE;

    return DRV_E_NONE;
}

/****************************************************************************
 * Name        : tcam_model_release
 * Purpose     : The function release tcam model's memory.
 * Input       : chip_id_offset - chip id information.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
tcam_model_release(uint8 chip_id_offset)
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

    sal_free(tcam_info.ext_tcam_data_base[chip_id_offset]);
    tcam_info.ext_tcam_data_base[chip_id_offset] = NULL;

    sal_free(tcam_info.ext_tcam_mask_base[chip_id_offset]);
    tcam_info.ext_tcam_mask_base[chip_id_offset] = NULL;

    sal_free(tcam_info.ext_tcam_wbit[chip_id_offset]);
    tcam_info.ext_tcam_wbit[chip_id_offset] = NULL;

    tcam_info.tcam_model_inited[chip_id_offset] = FALSE;

    return DRV_E_NONE;
}

/****************************************************************************
 * Name        : tcam_model_write
 * Purpose     : The function read data from tcam to controller.
 * Input       : chip_id_offset - chip id information.
                 sw_model_addr - tcam memory model's address.
                 data - the writed data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
tcam_model_write(uint8 chip_id_offset, uintptr sw_model_addr, uint32* data, int32 len)
{
    uint8 key_type = 0;
    bool is_data_addr = TRUE;

    /* check para */
    TCAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    TCAM_MODEL_PTR_VALID_CHECK(data);

    /* Before write tcam, check whether the tcam model is inited or not */
    if (!tcam_info.tcam_model_inited[chip_id_offset])
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
 * Input       : chip_id_offset - chip id information.
                 sw_model_addr - tcam memory model's address.
                 data - the read data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
tcam_model_read(uint8 chip_id_offset, uintptr sw_model_addr, uint32* data, int32 len)
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

    sal_memcpy(data, (uint8*)sw_model_addr, len);

    return DRV_E_NONE;
}

/****************************************************************************
 * Name        : tcam_model_remove
 * Purpose     : The function remove data from tcam to controller.
 * Input       : chip_id_offset - chip id information.
                 sw_model_addr - tcam memory model's address.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
tcam_model_remove(uint8 chip_id_offset, uintptr sw_model_addr)
{
    bool is_data_addr;
    uint8 key_type = 0;
    uintptr model_data_base = 0;
    uintptr model_mask_base = 0;
    uint32 offset = 0;

    if (!_tcam_model_addr_check(chip_id_offset, sw_model_addr,
                                &key_type, &is_data_addr))
    {
        DRV_DBG_INFO(" \n Check the address ERROR when remove tcam entry! \n");
        return DRV_E_INVALID_ADDR;
    }

    switch (key_type)
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
        sal_memset((uint32*)sw_model_addr, 0, DRV_BYTES_PER_ENTRY);
        sal_memset((uint32*)(model_mask_base + offset), 0, DRV_BYTES_PER_ENTRY);
    }
    else
    {
        sal_memset((uint32*)(model_data_base + offset), 0, DRV_BYTES_PER_ENTRY);
        sal_memset((uint32*)sw_model_addr, 0, DRV_BYTES_PER_ENTRY);
    }

    return DRV_E_NONE;
}

/****************************************************************************
 * Name        : tcam_model_lookup
 * Purpose     : The function lookup operation in tcam model.
 * Input       : chip_id_offset - chip id information.
                 tbl_id - tcam tabel ID.
                 data - the lookup data.
                 tcam_index - return hited tcam's index.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
tcam_model_lookup(uint8 chip_id_offset, uint32 tbl_id, uint32* data, uint32* tcam_index)
{
    uint32* sw_data_base = NULL, * sw_mask_base = NULL, * sw_vbit_base = NULL;
    uint32 hw_data_base = 0, hw_mask_base = 0;
    uint32* key_data_base = NULL, * key_mask_base = NULL;
    uint32 tcam_entry_num = 0;
    uint32 tmp_tbl_index = 0, tmp_word_offset = 0;
    uint32 tmp_tcam_data = 0, tmp_tcam_mask = 0, tmp_key = 0;
    uint8 tcam_type = 0;
    uint32 per_index_addr_offset = 0, per_index_entry_offset = 0;

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

    /* Get where the table is allocated (the Embeded/External/LPM TCAM) */
    if (((TABLE_DATA_BASE(tbl_id) >= DRV_INT_TCAM_KEY_DATA_ASIC_BASE)
         && (TABLE_DATA_BASE(tbl_id) < tcam_info.max_int_tcam_data_base)))
    {
        sw_data_base = tcam_info.int_tcam_data_base[chip_id_offset];
        sw_mask_base = tcam_info.int_tcam_mask_base[chip_id_offset];
        sw_vbit_base = tcam_info.int_tcam_wbit[chip_id_offset];
        hw_data_base = DRV_INT_TCAM_KEY_DATA_ASIC_BASE;
        hw_mask_base = DRV_INT_TCAM_KEY_MASK_ASIC_BASE;
        tcam_type = 1;
    }
    else if (((TABLE_DATA_BASE(tbl_id) >= DRV_EXT_TCAM_KEY_DATA_ASIC_BASE)
              && (TABLE_DATA_BASE(tbl_id) < tcam_info.max_ext_tcam_data_base)))
    {
        sw_data_base = tcam_info.ext_tcam_data_base[chip_id_offset];
        sw_mask_base = tcam_info.ext_tcam_mask_base[chip_id_offset];
        sw_vbit_base = tcam_info.ext_tcam_wbit[chip_id_offset];
        hw_data_base = DRV_EXT_TCAM_KEY_DATA_ASIC_BASE;
        hw_mask_base = DRV_EXT_TCAM_KEY_MASK_ASIC_BASE;
        tcam_type = 2;
    }
    else if (((TABLE_DATA_BASE(tbl_id) >= DRV_INT_LPM_TCAM_DATA_ASIC_BASE)
              && (TABLE_DATA_BASE(tbl_id) < tcam_info.max_int_lpm_tcam_data_base)))
    {
        sw_data_base = tcam_info.int_lpm_tcam_data_base[chip_id_offset];
        sw_mask_base = tcam_info.int_lpm_tcam_mask_base[chip_id_offset];
        sw_vbit_base = tcam_info.int_lpm_tcam_wbit[chip_id_offset];
        hw_data_base = DRV_INT_LPM_TCAM_DATA_ASIC_BASE;
        hw_mask_base = DRV_INT_LPM_TCAM_MASK_ASIC_BASE;
        tcam_type = 3;
    }
    else
    {
        tcam_type = 0;
        DRV_DBG_INFO("@@ Check address ERROR when Lookup TCAM!\n");
        return DRV_E_INVALID_ADDR;
    }

    /* Get the table's software data and mask address */
    key_data_base = sw_data_base +
        (TABLE_DATA_BASE(tbl_id) - hw_data_base) / DRV_BYTES_PER_WORD;
    key_mask_base = sw_mask_base +
        (TCAM_MASK_BASE(tbl_id) - hw_mask_base) / DRV_BYTES_PER_WORD;

    if (3 == tcam_type)
    {
        per_index_addr_offset = DRV_BYTES_PER_ENTRY / DRV_BYTES_PER_WORD;
        per_index_entry_offset = 1;
    }
    else
    {
        if (TCAM_KEY_SIZE(tbl_id) / DRV_BYTES_PER_ENTRY == 8)
        {
            per_index_addr_offset = TCAM_KEY_SIZE(tbl_id) / DRV_BYTES_PER_WORD / 2;
            per_index_entry_offset = 4;
        }
        else
        {
            per_index_addr_offset = TCAM_KEY_SIZE(tbl_id) / DRV_BYTES_PER_WORD;
            per_index_entry_offset = TCAM_KEY_SIZE(tbl_id) / DRV_BYTES_PER_ENTRY;
        }
    }

    /* Search in the table range */
    for (tmp_tbl_index = 0; tmp_tbl_index < TABLE_MAX_INDEX(tbl_id); ++tmp_tbl_index)
    {
        tcam_entry_num = tmp_tbl_index * per_index_entry_offset
            + (TABLE_DATA_BASE(tbl_id) - hw_data_base) / DRV_BYTES_PER_ENTRY;

        /* Check the tcam entry's Wbit */
        if (!(IS_BIT_SET(*(sw_vbit_base + tcam_entry_num / DRV_BITS_PER_WORD), tcam_entry_num % DRV_BITS_PER_WORD)))
        {
            continue;
        }

        /* Compare key according to the word unit */
        for (tmp_word_offset = 0; tmp_word_offset < TCAM_KEY_SIZE(tbl_id) / DRV_BYTES_PER_WORD; ++tmp_word_offset)
        {
            if (tmp_word_offset < 12)
            {
                tmp_tcam_mask = *(key_mask_base + tmp_tbl_index * per_index_addr_offset + tmp_word_offset);
                tmp_tcam_data = *(key_data_base + tmp_tbl_index * per_index_addr_offset + tmp_word_offset);
            }
            else
            {
                tmp_tcam_mask = *(key_mask_base + tmp_tbl_index * per_index_addr_offset + DRV_ACL_TCAM_640_OFFSET_IDX * DRV_WORDS_PER_ENTRY - per_index_entry_offset * DRV_WORDS_PER_ENTRY + tmp_word_offset);
                tmp_tcam_data = *(key_data_base + tmp_tbl_index * per_index_addr_offset + DRV_ACL_TCAM_640_OFFSET_IDX * DRV_WORDS_PER_ENTRY - per_index_entry_offset * DRV_WORDS_PER_ENTRY + tmp_word_offset);
            }

            tmp_key = *(data + tmp_word_offset);

            if ((tmp_tcam_data & tmp_tcam_mask) != (tmp_key & tmp_tcam_mask))
            {
                break;
            }

            if (tmp_word_offset == TCAM_KEY_SIZE(tbl_id) / DRV_BYTES_PER_WORD - 1)
            {
                /* Match, return the index according to tcam entry unit(80bits) in the whole Tcam */
                *tcam_index = (TABLE_DATA_BASE(tbl_id) - hw_data_base) / DRV_BYTES_PER_ENTRY
                    + tmp_tbl_index * per_index_entry_offset;
                /*printf("+++++++++++ tcam index %d\n", *tcam_index );*/
                return DRV_E_NONE;
            }
        }
    }

    *tcam_index = DRV_TCAM_INVALID_INDEX; /* Can not match in tcam*/

    return DRV_E_NONE;
}

int32
tcam_model_get_sw_address(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index,
                          uint32* sw_model_addr, bool is_data)
{
    uint32 hw_base_addr = 0;
    uintptr sw_base_addr = 0;

    if (!drv_greatbelt_table_is_tcam_key(tbl_id)) /* is not TcamKey Table */
    {
        return DRV_E_INVALID_PARAMETER;
    }

    if (DRV_ADDR_IN_TCAM_DATA_RANGE(TABLE_DATA_BASE(tbl_id)))
    {
        if (is_data)
        {
            hw_base_addr = DRV_INT_TCAM_KEY_DATA_ASIC_BASE;
            sw_base_addr = (uintptr)tcam_info.int_tcam_data_base[chip_id_offset];
        }
        else
        {
            hw_base_addr = DRV_INT_TCAM_KEY_MASK_ASIC_BASE;
            sw_base_addr = (uintptr)tcam_info.int_tcam_mask_base[chip_id_offset];
        }
    }
    else if (DRV_ADDR_IN_LPM_TCAM_DATA_RANGE(TABLE_DATA_BASE(tbl_id)))
    {
        if (is_data)
        {
            hw_base_addr = DRV_INT_LPM_TCAM_DATA_ASIC_BASE;
            sw_base_addr = (uintptr)tcam_info.int_lpm_tcam_data_base[chip_id_offset];
        }
        else
        {
            hw_base_addr = DRV_INT_LPM_TCAM_MASK_ASIC_BASE;
            sw_base_addr = (uintptr)tcam_info.int_lpm_tcam_mask_base[chip_id_offset];
        }
    }
    else
    {
        sal_printf("%% ERROR! Invalid Tcam Key = %d data base address = 0x%8x!!\n", tbl_id, TABLE_DATA_BASE(tbl_id));
        return DRV_E_INVALID_ADDR;
    }

    *sw_model_addr
        = sw_base_addr + (TABLE_DATA_BASE(tbl_id) - hw_base_addr) / DRV_BYTES_PER_ENTRY
            + index * TCAM_KEY_SIZE(tbl_id);

    return DRV_E_NONE;
}

