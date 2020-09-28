/****************************************************************************
 * sram_model.c  :      sram operation interface
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   V1.0
 * Author   :   Zhouw
 * Date     :   2010-02-24
 * Reason   :   First create
 *
 * Revision :   V1.0
 * Author   :   JiangJf
 * Date     :   2010-12-03
 * Reason   :   Modified according to Mem allocation
 ****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#if !defined(SDK_IN_KERNEL)
#include "greatbelt/include/drv_lib.h"
#include "sram_model.h"

/****************************************************************************
 *
* Type definitions and macros
*
****************************************************************************/

/****************************************************************************
 *
* Global and Declarations
*
****************************************************************************/
cm_sram_info_t sram_info[MAX_LOCAL_CHIP_NUM];

/* Record Each Reg and Tbl's software address */
cmodel_tbl_info_t cmodel_tbls_info[MAX_LOCAL_CHIP_NUM][MaxTblId_t];

/**************************************************************************
*
*Fuction Declarations
*
**************************************************************************/

/***************************************************************************
**  Functions
***************************************************************************/
/****************************************************************************
 * Name       : sram_model_initialize
 * Purpose    : The function initialize chip's sram.
 * Input      : chip_id_offset - chip id information
 * Output     : N/A
 * Return     : DRV_E_NONE
 *              Other = ErrCode
 * Note:      : N/A
****************************************************************************/
int32
sram_model_initialize(uint8 chip_id_offset)
{
    int32 ret = DRV_E_NONE;
    tbls_id_t tbl_id = 0;
    uint32 tbl_mem_size = 0;

    /* Cmodel only support up to 2 same chip */
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* check whether the chip's sram has been malloc or not */
    if (sram_info[chip_id_offset].sram_model_initialized_flag)
    {
        return DRV_E_NONE;
    }

/* --- memory allocation for dynamic tables starts --- */
    /*Malloc one block memory for all hash/hash ad/dynamic tables*/
    sram_info[chip_id_offset].dynamic_mem_base
        = (uintptr)sal_malloc(DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    if (!sram_info[chip_id_offset].dynamic_mem_base)
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset((void*)sram_info[chip_id_offset].dynamic_mem_base, 0, DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

    /*Malloc one block memory for dynamic tables wbit*/
    sram_info[chip_id_offset].dynamic_mem_wbit = (uint8*)sal_malloc(DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM);
    if (!sram_info[chip_id_offset].dynamic_mem_wbit)
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset((void*)sram_info[chip_id_offset].dynamic_mem_wbit, 0, DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM);

    /*Malloc one block memory for tcam ad tables wbit*/
    sram_info[chip_id_offset].tcam_ad_mem_base
        = (uintptr)sal_malloc(DRV_INT_TCAM_AD_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    if (!sram_info[chip_id_offset].tcam_ad_mem_base)
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset((void*)sram_info[chip_id_offset].tcam_ad_mem_base, 0, DRV_INT_TCAM_AD_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);

    sram_info[chip_id_offset].tcam_ad_mem_wbit = (uint8*)sal_malloc(DRV_INT_TCAM_AD_MAX_ENTRY_NUM);
    if (!sram_info[chip_id_offset].tcam_ad_mem_wbit)
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset((void*)sram_info[chip_id_offset].tcam_ad_mem_wbit, 0, DRV_INT_TCAM_AD_MAX_ENTRY_NUM);

    sram_info[chip_id_offset].lpm_tcam_ad_mem_base
        = (uintptr)sal_malloc(DRV_INT_LPM_TCAM_AD_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    if (!sram_info[chip_id_offset].lpm_tcam_ad_mem_base)
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset((void*)sram_info[chip_id_offset].lpm_tcam_ad_mem_base, 0, DRV_INT_LPM_TCAM_AD_MAX_ENTRY_NUM * DRV_BYTES_PER_ENTRY);
    sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit = (uint8*)sal_malloc(DRV_INT_LPM_TCAM_AD_MAX_ENTRY_NUM);
    if (!sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit)
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset((void*)sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit, 0, DRV_INT_LPM_TCAM_AD_MAX_ENTRY_NUM);
/* --- memory allocation for dynamic tables ends --- */

    /* Malloc each table Memory */
    for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
    {
        /* here allocate the static tbl sram info, not include the dynamic and tcam ad info
           dynamic and tcam ad info is set by memory allocation */

        /* those dynamic tables not malloc */
        if (!TABLE_MAX_INDEX(tbl_id))
        {
            continue;
        }

        /* allocate memory for all static table */
        cmodel_tbls_info[chip_id_offset][tbl_id].sram_tbl_type = SRAM_TBL_TYPE_NORMAL;
        tbl_mem_size = TABLE_MAX_INDEX(tbl_id) * TABLE_ENTRY_SIZE(tbl_id);

        cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base = (uintptr)sal_malloc(tbl_mem_size);
        if (!cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base)
        {
            return DRV_E_NO_MEMORY;
        }

        sal_memset((uint32*)cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base, 0, tbl_mem_size);

        sram_info[chip_id_offset].sram_model_wbit[tbl_id] = sal_malloc(TABLE_MAX_INDEX(tbl_id));
        if (!sram_info[chip_id_offset].sram_model_wbit[tbl_id])
        {
            return DRV_E_NO_MEMORY;
        }

        sal_memset(sram_info[chip_id_offset].sram_model_wbit[tbl_id], 0, TABLE_MAX_INDEX(tbl_id));

    }

    sram_info[chip_id_offset].sram_model_initialized_flag = TRUE;

    return ret;
}

/****************************************************************************
 * Name        : sram_model_release
 * Purpose     : The function release sram model's memory.
 * Input       : chip_id_offset - chip id information.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
sram_model_release(uint8 chip_id_offset)
{
    uint32 tbl_id = 0;
    uint32* sw_mem_ptr = NULL;

    /* Cmodel only support up to 2 same chip */
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* release each tbl's memory */
    for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
    {
        if (!cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base)
        {
            sw_mem_ptr = (uint32*)cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base;
            sal_free(sw_mem_ptr);
            cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base = 0;
        }

        if (!cmodel_tbls_info[chip_id_offset][tbl_id].sram_dynamic_info_ptr)
        {
            sal_free(cmodel_tbls_info[chip_id_offset][tbl_id].sram_dynamic_info_ptr);
            cmodel_tbls_info[chip_id_offset][tbl_id].sram_dynamic_info_ptr = NULL;
        }

        if (!sram_info[chip_id_offset].sram_model_wbit[tbl_id])
        {
            sal_free(sram_info[chip_id_offset].sram_model_wbit[tbl_id]);
            sram_info[chip_id_offset].sram_model_wbit[tbl_id] = NULL;
        }
    }

    if (!sram_info[chip_id_offset].dynamic_mem_wbit)
    {
        sal_free(sram_info[chip_id_offset].dynamic_mem_wbit);
        sram_info[chip_id_offset].dynamic_mem_wbit = NULL;
    }

    if (!sram_info[chip_id_offset].tcam_ad_mem_wbit)
    {
        sal_free(sram_info[chip_id_offset].tcam_ad_mem_wbit);
        sram_info[chip_id_offset].tcam_ad_mem_wbit = NULL;
    }

    if (!sram_info[chip_id_offset].dynamic_mem_base)
    {
        sw_mem_ptr = (uint32*)sram_info[chip_id_offset].dynamic_mem_base;
        sal_free(sw_mem_ptr);
        sram_info[chip_id_offset].dynamic_mem_base = 0;
    }

    if (!sram_info[chip_id_offset].tcam_ad_mem_base)
    {
        sw_mem_ptr = (uint32*)sram_info[chip_id_offset].tcam_ad_mem_base;
        sal_free(sw_mem_ptr);
        sram_info[chip_id_offset].tcam_ad_mem_base = 0;
    }

    return DRV_E_NONE;
}

/****************************************************************************
 * Name        : sram_model_read
 * Purpose     : The function read data from sram to controller.
 * Input       : chip_id_offset - chip id information.
                 sw_model_addr - memory model's address.
                 data - the read data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
sram_model_read(uint8  chip_id_offset,
                uintptr sw_model_addr,
                uint32* data,
                int32  len)
{
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* May be add code to check whether the address is valid or not */

    SRAM_MODEL_PTR_VALID_CHECK(data);

    sal_memcpy(data, (uint32*)sw_model_addr, len);

    return DRV_E_NONE;
}

/****************************************************************************
 * Name        : sram_model_write
 * Purpose     : The function wirte data from sram to controller.
 * Input       : chip_id_offset - chip id information.
                 sw_model_addr - memory model's address.
                 data - the writed data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32
sram_model_write(uint8  chip_id_offset,
                 uintptr sw_model_addr,
                 uint32* data,
                 int32  len)
{
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* May be add code to check whether the address is valid or not */

    SRAM_MODEL_PTR_VALID_CHECK(data);

    sal_memcpy((uint32*)sw_model_addr, data,  len);

    /* set wbit for dump after write action(move to drv) */

    return DRV_E_NONE;
}

/****************************************************************************
 * Name         : sram_model_reset
 * Purpose      : reset the sram's value.
 * Input        : chip_id_offset - chip id information.
 * Output       : N/A.
 * Return       : SUCCESS.
 *                Other = ErrCode.
 * Note         : N/A.
****************************************************************************/
int32
sram_model_reset(uint8 chip_id_offset)
{
    uint32 tbl_id = 0;

    for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
    {
        uint32 tbl_mem_size = TABLE_MAX_INDEX(tbl_id) * TABLE_ENTRY_SIZE(tbl_id);

        if (cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base)
        {
            sal_memset((uint32*)cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base, 0, tbl_mem_size);
        }

        if (sram_info[chip_id_offset].sram_model_wbit[tbl_id])
        {
            sal_memset(sram_info[chip_id_offset].sram_model_wbit[tbl_id], 0, tbl_mem_size / DRV_BYTES_PER_ENTRY);
        }
    }

    if (!sram_info[chip_id_offset].dynamic_mem_wbit)
    {
        sal_memset(sram_info[chip_id_offset].dynamic_mem_wbit, 0, DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM);
    }

    if (!sram_info[chip_id_offset].tcam_ad_mem_wbit)
    {
        sal_memset(sram_info[chip_id_offset].tcam_ad_mem_wbit, 0, DRV_INT_TCAM_AD_MAX_ENTRY_NUM);
    }

    return DRV_E_NONE;
}

int32
sram_model_get_sw_address(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index, uint32* sw_model_addr)
{
    if (drv_greatbelt_table_is_tcam_key(tbl_id)) /* isTcamKeyTable */
    {
        return DRV_E_INVALID_PARAMETER;
    }

    if (drv_greatbelt_table_is_dynamic_table(tbl_id))
    {

        if (DYNAMIC_ACCESS_MODE(tbl_id) == DYNAMIC_8W_MODE) /* special process, 8word table but index is per-80bits */
        {
            *sw_model_addr = cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base + index * DRV_BYTES_PER_ENTRY;
            return DRV_E_NONE;
        }
    }

    *sw_model_addr = cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base + index * TABLE_ENTRY_SIZE(tbl_id);

    return DRV_E_NONE;
}

int8
sram_model_is_tcam_ad_table(uint8 chip_id_offset, tbls_id_t tbl_id)
{

    if ((cmodel_tbls_info[chip_id_offset][tbl_id].sram_tbl_type) == SRAM_TBL_TYPE_TCAM_AD)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}

int32
sram_model_get_sw_address_by_tbl_id_and_index(uint8 chip_id_offset, tbls_id_t tbl_id,
                                              uint32 index, uintptr* start_data_addr)
{
    uintptr sw_data_base = 0;
    uint8 blk_id = 0;

    DRV_TBL_ID_VALID_CHECK(tbl_id);
    DRV_TBL_INDEX_VALID_CHECK(tbl_id, index);

    if (drv_greatbelt_table_is_tcam_key(tbl_id))
    {
        DRV_DBG_INFO("ERROR (sram_model_get_sw_address_by_tbl_id_and_index): chip-0x%x, tbl-%d, tcam key call the function only for non tcam key.\n",
                     chip_id_offset + drv_gb_init_chip_info.drv_init_chipid_base, tbl_id);
        return DRV_E_INVALID_TBL;
    }

    if (drv_greatbelt_table_is_dynamic_table(tbl_id))
    {
        for (blk_id = 0; blk_id < MAX_DRV_BLOCK_NUM; blk_id++)
        {
            if (!IS_BIT_SET(DYNAMIC_BITMAP(tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= DYNAMIC_START_INDEX(tbl_id, blk_id)) && (index <= DYNAMIC_END_INDEX(tbl_id, blk_id)))
            {
                sw_data_base = SRAM_DATA_BASE(chip_id_offset, tbl_id, blk_id);
                break;
            }
        }

        if(MAX_DRV_BLOCK_NUM == blk_id)
        {
            return DRV_E_INVALID_INDEX;
        }

        if (DYNAMIC_ACCESS_MODE(tbl_id) == DYNAMIC_8W_MODE)
        {
            if (index % 2)
            {
                DRV_DBG_INFO("ERROR!! get tbl_id %d index must be even! now index = %d\n", tbl_id, index);
                return DRV_E_INVALID_INDEX;
            }

            *start_data_addr = sw_data_base + (index - DYNAMIC_START_INDEX(tbl_id, blk_id)) * DRV_BYTES_PER_ENTRY;
        }
        else
        {
            *start_data_addr = sw_data_base + (index - DYNAMIC_START_INDEX(tbl_id, blk_id)) * TABLE_ENTRY_SIZE(tbl_id);
        }
    }
    else
    {
        sw_data_base = SRAM_SW_DATA_BASE(chip_id_offset, tbl_id);
        *start_data_addr = sw_data_base + index * TABLE_ENTRY_SIZE(tbl_id);
    }

    return DRV_E_NONE;
}

STATIC bool
sram_model_is_tcam_ad_tbl(tbls_id_t tbl_id)
{
    uint32 idx = 0, len = 0;

    tbls_id_t tcam_ad_list[] =   /* all tcam ad */
    {
        DsIpv6Acl0Tcam_t,
        DsIpv6Acl1Tcam_t,
        DsIpv4Acl0Tcam_t,
        DsIpv4Acl1Tcam_t,
        DsIpv4Acl2Tcam_t,
        DsIpv4Acl3Tcam_t,
        DsMacAcl0Tcam_t,
        DsMacAcl1Tcam_t,
        DsMacAcl2Tcam_t,
        DsMacAcl3Tcam_t,
        DsIpv4UcastDaTcam_t,
        DsIpv4McastDaTcam_t,
        DsIpv6UcastDaTcam_t,
        DsIpv6McastDaTcam_t,
        DsIpv4SaNatTcam_t,
        DsIpv6SaNatTcam_t,
        DsIpv4UcastPbrDualDaTcam_t,
        DsIpv6UcastPbrDualDaTcam_t,
        DsMacTcam_t,
        DsFcoeDaTcam_t,
        DsFcoeSaTcam_t,
        /* DsTrillDaTcam_t,*/
        DsTrillDaUcastTcam_t,
        DsTrillDaMcastTcam_t,
        DsUserIdMacTcam_t,
        DsUserIdIpv6Tcam_t,
        DsUserIdIpv4Tcam_t,
        DsUserIdVlanTcam_t,
        DsTunnelIdIpv6Tcam_t,
        DsTunnelIdIpv4Tcam_t,
        DsTunnelIdPbbTcam_t,
        DsTunnelIdCapwapTcam_t,
        DsTunnelIdTrillTcam_t,

        DsEthOamTcamChan_t,
        DsBfdOamChanTcam_t,
        DsMplsOamChanTcam_t,
        DsPbtOamChanTcam_t,
        DsEthRmepChan_t,

        /* DsMacIpv4Tcam and DsMacIpv6Tcam */
        DsMacIpv4Tcam_t,
        DsMacIpv6Tcam_t,
    };

    len = sizeof(tcam_ad_list) / sizeof(tbls_id_t);

    for (idx = 0; idx < len; idx++)
    {
        if (tcam_ad_list[idx] == tbl_id)
        {
            return TRUE;
        }
    }

    return FALSE;

}

STATIC bool
sram_model_is_lpm_tcam_ad_tbl(tbls_id_t tbl_id)
{
    uint32 idx = 0, len = 0;

    tbls_id_t lpm_tcam_ad[] =
    {
        LpmTcamAdMem_t,
        DsUserIdConflictTcam_t,
        DsTunnelIdConflictTcam_t,
        DsVlanXlateConflictTcam_t,
    };

    len = sizeof(lpm_tcam_ad) / sizeof(tbls_id_t);

    for (idx = 0; idx < len; idx++)
    {
        if (lpm_tcam_ad[idx] == tbl_id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************
 * Name:       cm_sim_cfg_kit_allocate_sram_tbl_info
 * Purpose:    write dynamic and tcam ad table info to sram model, only for sram model exist
 * Parameters:
 * Input:      void
 * Output:     None
 * Return:     DRV_E_NONE = success.
 *             Other = Error, please refer to DRV_E_XXX.
 * Note:       none.
 ****************************************************************************/
int32
sram_model_allocate_sram_tbl_info(void)
{
    uint8 chipid = 0;
    uint8 chip_num = 0;
    uint8 chip_id_base = 0;
    uint8 chip_id_offset = 0;
    tbls_id_t tbl_id = MaxTblId_t, oam_stats_static_tbl_id = MaxTblId_t;
    sram_dynamic_info_t* sram_dynamic_info_ptr = NULL;
    uint8 blk_id = 0;
    uint32 memory_base_addr = 0;
    uint8 oam_lm_stats_num = 0, oam_lm_stats_static_tbl_index = 0;

    DRV_IF_ERROR_RETURN(drv_greatbelt_get_chipnum(&chip_num));
    DRV_IF_ERROR_RETURN(drv_greatbelt_get_chipid_base(&chip_id_base));

    /* for cmodel support multi chip */
    for (chipid = chip_id_base; chipid < chip_id_base + chip_num; chipid++)
    {
        chip_id_offset = chipid - chip_id_base;

        oam_lm_stats_num = 0;
#if 0
        /* alloc SRAM table does not depend on SRAM model alloced */
        if (SRAM_INFO_MODEL_INIT_FLAG(chip_id_offset) == FALSE)
        {
            DRV_DBG_INFO("CModelAlloc ERROR! Sram is not initilize, can't allocate sram model dynamic info.\n");
            return DRV_E_INVALID_DYNIC_TBL_ALLOC;
        }

#endif

        for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
        {
            if (TABLE_MAX_INDEX(tbl_id) == 0)
            {
                continue;
            }

            /* do tcam ad table sram model operation */
            if (sram_model_is_tcam_ad_tbl(tbl_id))
            {
                memory_base_addr = (TABLE_ENTRY_SIZE(tbl_id) == DRV_BYTES_PER_ENTRY) ?
                    DRV_INT_TCAM_AD_MEM_4W_BASE : DRV_INT_TCAM_AD_MEM_8W_BASE;

                SRAM_SW_DATA_BASE(chip_id_offset, tbl_id) = SRAM_INFO_TCAM_AD_MEM_BASE(chip_id_offset)
                    + (TABLE_DATA_BASE(tbl_id) - memory_base_addr) / DRV_ADDR_BYTES_PER_ENTRY * DRV_BYTES_PER_ENTRY;

                /* set tcam ad wbit info to sram_model_wbit[MaxTblId_t],
                   in driver io , can use tbl_id to get wbit of tcam ad, same of statis table */
                SRAM_INFO_MODEL_TBL_WBIT_PTR(chip_id_offset, tbl_id) = SRAM_INFO_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset)
                    + (TABLE_DATA_BASE(tbl_id) - memory_base_addr) / DRV_ADDR_BYTES_PER_ENTRY;

                /* set flag to indicated table is tcam ad,
                   because dump tcam ad is per 80 bit, so dump tcam ad is different of STATIC table */
                SRAM_TBL_TYPE(chip_id_offset, tbl_id) = SRAM_TBL_TYPE_TCAM_AD;

                continue;
            }

            /* do lpm tcam table sram model operation */
            if (sram_model_is_lpm_tcam_ad_tbl(tbl_id))
            {
                SRAM_SW_DATA_BASE(chip_id_offset, tbl_id) = SRAM_INFO_LPM_TCAM_AD_MEM_BASE(chip_id_offset);

                /* set lpm tcam ad wbit info to sram_model_wbit[MaxTblId_t],
                   in driver io , can use tbl_id to get wbit of tcam ad, same of statis table */
                SRAM_INFO_MODEL_TBL_WBIT_PTR(chip_id_offset, tbl_id) = SRAM_INFO_LPM_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset);

                /* set flag to indicated table is tcam ad,
                   because dump tcam ad is per 80 bit, so dump tcam ad is different of STATIC table */
                SRAM_TBL_TYPE(chip_id_offset, tbl_id) = SRAM_TBL_TYPE_TCAM_AD;

                continue;
            }

            /* if driver ptr ext info is NULL, static table, so continue */
            if (TABLE_EXT_INFO_PTR(tbl_id) == NULL)
            {
                continue;
            }

            /* continue if table is not dynamic table */
            if (TABLE_EXT_INFO_TYPE(tbl_id) != EXT_INFO_TYPE_DYNAMIC)
            {
                continue;
            }

            /* continue if table is dynamic ,but not allocate resource */
            if (DYNAMIC_EXT_INFO_PTR(tbl_id) == NULL)
            {
                continue;
            }

            sram_dynamic_info_ptr = (sram_dynamic_info_t*)sal_malloc(sizeof(sram_dynamic_info_t));
            if (sram_dynamic_info_ptr == NULL)
            {
                SAL_LOG_FATAL("%s:%d ******** malloc sram dynamic table error, no enough memory!!********\n",
                              __FUNCTION__, __LINE__);
                exit(1);
            }

            sal_memset(sram_dynamic_info_ptr, 0, sizeof(sram_dynamic_info_t));

            sram_dynamic_info_ptr->sram_dynamic_bitmap = DYNAMIC_BITMAP(tbl_id);

            for (blk_id = 0; blk_id < MAX_MEMORY_BLOCK_NUM; blk_id++)
            {
                if (!IS_BIT_SET(sram_dynamic_info_ptr->sram_dynamic_bitmap, blk_id))
                {
                    continue;
                }

                sram_dynamic_info_ptr->sram_dynamic_entry_num[blk_id] = DYNAMIC_ENTRY_NUM(tbl_id, blk_id);
                sram_dynamic_info_ptr->sram_dynamic_entry_start_index[blk_id] = DYNAMIC_START_INDEX(tbl_id, blk_id);
                sram_dynamic_info_ptr->sram_dynamic_entry_end_index[blk_id] = DYNAMIC_END_INDEX(tbl_id, blk_id);

                memory_base_addr = (TABLE_ENTRY_SIZE(tbl_id) == DRV_BYTES_PER_ENTRY) ?
                    DRV_MEMORY0_BASE_4W : DRV_MEMORY0_BASE_8W;

                sram_dynamic_info_ptr->sram_dynamic_sw_data_base[blk_id] = sram_info[chip_id_offset].dynamic_mem_base
                    + (DYNAMIC_DATA_BASE(tbl_id, blk_id) - memory_base_addr) / DRV_ADDR_BYTES_PER_ENTRY * DRV_BYTES_PER_ENTRY;

                sram_dynamic_info_ptr->sram_dynamic_wbit_sw_data_base[blk_id] = (uintptr)sram_info[chip_id_offset].dynamic_mem_wbit
                    + (DYNAMIC_DATA_BASE(tbl_id, blk_id) - memory_base_addr) / DRV_ADDR_BYTES_PER_ENTRY;
                sram_dynamic_info_ptr->sram_access_mode = DYNAMIC_ACCESS_MODE(tbl_id);

            }

            /* special action for DsOamLmStats_t, can't get DsOamLmStats0~2_t sw address according drv info
               so the Lm_ext_block info should special code to implement */
            if (tbl_id == DsOamLmStats_t)
            {
                /* in sram model, should add the static DsOamLmStats0~2 info to dynamic table DsOamLmStats */
                /* add the static DsOamLmStats0 info to DsOamLmStats */

                for (oam_lm_stats_static_tbl_index = 0; oam_lm_stats_static_tbl_index < LM_EXT_BLOCK_NUM; oam_lm_stats_static_tbl_index++)
                {
                    switch (oam_lm_stats_static_tbl_index)
                    {
                    case 0:
                        oam_stats_static_tbl_id = DsOamLmStats0_t;
                        break;

                    case 1:
                        oam_stats_static_tbl_id = DsOamLmStats1_t;
                        break;

                    default:
                        break;

                    }

                    blk_id = MAX_MEMORY_BLOCK_NUM + (oam_lm_stats_num++);

                    sram_dynamic_info_ptr->sram_dynamic_entry_num[blk_id] = DYNAMIC_ENTRY_NUM(tbl_id, blk_id);
                    sram_dynamic_info_ptr->sram_dynamic_entry_start_index[blk_id] = DYNAMIC_START_INDEX(tbl_id, blk_id);
                    sram_dynamic_info_ptr->sram_dynamic_entry_end_index[blk_id] = DYNAMIC_END_INDEX(tbl_id, blk_id);

                    sram_dynamic_info_ptr->sram_dynamic_sw_data_base[blk_id] = SRAM_SW_DATA_BASE(chip_id_offset, oam_stats_static_tbl_id);

                    sram_dynamic_info_ptr->sram_dynamic_wbit_sw_data_base[blk_id] =
                        (uintptr)sram_info[chip_id_offset].sram_model_wbit[oam_stats_static_tbl_id];
                    sram_dynamic_info_ptr->sram_access_mode = DYNAMIC_ACCESS_MODE(tbl_id);

                }
            }

            /* assigne the pointer to sram model */
            SRAM_DYNAMIC_INFO_PTR(chip_id_offset, tbl_id) = sram_dynamic_info_ptr;
            sram_dynamic_info_ptr = NULL;

        }
    }

    return DRV_E_NONE;
}

int32
sram_model_update_tbl_info(uint32 tbl_id, uint8 block_id, uint32 offset, uint32 size)
{
    uint8 blk_id    = 0;
    uint8 blk_num   = 0;
    uint8 chip_num  = 0;
    uint8 chipid    = 0;
    uint8 chip_id_base  = 0;
    uint8 chip_id_offset = 0;
    uint32 entry_num = 0;

    DRV_TBL_ID_VALID_CHECK(tbl_id);

    DRV_IF_ERROR_RETURN(drv_greatbelt_get_chipnum(&chip_num));
    DRV_IF_ERROR_RETURN(drv_greatbelt_get_chipid_base(&chip_id_base));


    if (drv_greatbelt_table_is_dynamic_table(tbl_id))
    {
        for (blk_id = 0; blk_id < MAX_DRV_BLOCK_NUM; blk_id++)
        {
            if (!IS_BIT_SET(DYNAMIC_BITMAP(tbl_id), blk_id))
            {
                continue;
            }
            else
            {
                if(blk_num == block_id)
                {
                    /* for cmodel support multi chip */
                    for (chipid = chip_id_base; chipid < chip_id_base + chip_num; chipid++)
                    {
                        chip_id_offset = chipid - chip_id_base;
                        //entry_num = SRAM_END_INDEX(chip_id_offset, tbl_id, blk_id) - SRAM_START_INDEX(chip_id_offset, tbl_id, blk_id);
                        entry_num = size - 1;
                        SRAM_START_INDEX(chip_id_offset, tbl_id, blk_id) = offset;
                        SRAM_END_INDEX(chip_id_offset, tbl_id, blk_id)   = entry_num + offset;
                    }

                }
                blk_num ++;
            }
        }
    }

    return DRV_E_NONE;
}

#endif

