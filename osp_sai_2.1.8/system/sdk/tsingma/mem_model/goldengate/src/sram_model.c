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
#include "goldengate/include/drv_lib.h"
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

/* Hardware mem chip information, driver IO may user this to map hardware address for Hash key.
Hash key uses universal addressing, we shall map different index to different hardware address. */
sram_mem_chip_allocation_info_t model_mem_chip_info[CM_MAX_MEM_CHIP_ID] =
{
{"ShareRam0",0, DRV_SHARE_RAM0_MAX_ENTRY_NUM, DRV_SHARE_RAM0_BASE_4W, DRV_SHARE_RAM0_BASE_6W, DRV_SHARE_RAM0_BASE_12W},/*CM_MEMORY0*/
{"ShareRam1",0, DRV_SHARE_RAM1_MAX_ENTRY_NUM, DRV_SHARE_RAM1_BASE_4W, DRV_SHARE_RAM1_BASE_6W, DRV_SHARE_RAM1_BASE_12W},/*CM_MEMORY1*/
{"ShareRam2",0, DRV_SHARE_RAM2_MAX_ENTRY_NUM, DRV_SHARE_RAM2_BASE_4W, DRV_SHARE_RAM2_BASE_6W, DRV_SHARE_RAM2_BASE_12W},/*CM_MEMORY2*/
{"ShareRam3",0, DRV_SHARE_RAM3_MAX_ENTRY_NUM, DRV_SHARE_RAM3_BASE_4W, DRV_SHARE_RAM3_BASE_6W, DRV_SHARE_RAM3_BASE_12W},/*CM_MEMORY3*/
{"ShareRam4",0, DRV_SHARE_RAM4_MAX_ENTRY_NUM, DRV_SHARE_RAM4_BASE_4W, DRV_SHARE_RAM4_BASE_6W, DRV_SHARE_RAM4_BASE_12W},/*CM_MEMORY4*/
{"ShareRam5",0, DRV_SHARE_RAM5_MAX_ENTRY_NUM, DRV_SHARE_RAM5_BASE_4W, DRV_SHARE_RAM5_BASE_6W, DRV_SHARE_RAM5_BASE_12W},/*CM_MEMORY5*/
{"ShareRam6",0, DRV_SHARE_RAM6_MAX_ENTRY_NUM, DRV_SHARE_RAM6_BASE_4W, DRV_SHARE_RAM6_BASE_6W, DRV_SHARE_RAM6_BASE_12W},/*CM_MEMORY5*/
{"IpMacRam0",0, DRV_DS_IPMAC_RAM0_MAX_ENTRY_NUM, DRV_DS_IPMAC_RAM0_BASE_4W, DRV_DS_IPMAC_RAM0_BASE_6W, DRV_DS_IPMAC_RAM0_BASE_12W},/*CM_MEMORY6*/
{"IpMacRam1",0, DRV_DS_IPMAC_RAM1_MAX_ENTRY_NUM, DRV_DS_IPMAC_RAM1_BASE_4W, DRV_DS_IPMAC_RAM1_BASE_6W, DRV_DS_IPMAC_RAM1_BASE_12W},/*CM_MEMORY7*/
{"IpMacRam2",0, DRV_DS_IPMAC_RAM2_MAX_ENTRY_NUM, DRV_DS_IPMAC_RAM2_BASE_4W, DRV_DS_IPMAC_RAM2_BASE_6W, DRV_DS_IPMAC_RAM2_BASE_12W},/*CM_MEMORY8*/
{"IpMacRam3",0, DRV_DS_IPMAC_RAM3_MAX_ENTRY_NUM, DRV_DS_IPMAC_RAM3_BASE_4W, DRV_DS_IPMAC_RAM3_BASE_6W, DRV_DS_IPMAC_RAM3_BASE_12W},/*CM_MEMORY8*/

{"UseridHashKeyRam0",0, DRV_USERIDHASHKEY_RAM0_MAX_ENTRY_NUM, DRV_USERIDHASHKEY_RAM0_BASE_4W, DRV_USERIDHASHKEY_RAM0_BASE_6W, DRV_USERIDHASHKEY_RAM0_BASE_12W},/*CM_MEMORY8*/
{"UseridHashKeyRam1",0, DRV_USERIDHASHKEY_RAM1_MAX_ENTRY_NUM, DRV_USERIDHASHKEY_RAM1_BASE_4W, DRV_USERIDHASHKEY_RAM1_BASE_6W, DRV_USERIDHASHKEY_RAM1_BASE_12W},/*CM_MEMORY8*/
{"UseridHashAdRam",0, DRV_USERIDHASHAD_RAM_MAX_ENTRY_NUM, DRV_USERIDHASHAD_RAM_BASE_4W, DRV_USERIDHASHAD_RAM_BASE_6W, DRV_USERIDHASHAD_RAM_BASE_12W},/*CM_MEMORY8*/
{"L23EditRam0",0, DRV_L23EDITRAM0_MAX_ENTRY_NUM, DRV_L23EDITRAM0_BASE_4W, DRV_L23EDITRAM0_BASE_6W, DRV_L23EDITRAM0_BASE_12W},/*CM_MEMORY8*/
{"L23EditRam1",0, DRV_L23EDITRAM1_MAX_ENTRY_NUM, DRV_L23EDITRAM1_BASE_4W, DRV_L23EDITRAM1_BASE_6W, DRV_L23EDITRAM1_BASE_12W},/*CM_MEMORY8*/
{"NexthopMetRam0",0, DRV_NEXTHOPMET_RAM0_MAX_ENTRY_NUM, DRV_NEXTHOPMET_RAM0_BASE_4W, DRV_NEXTHOPMET_RAM0_BASE_6W, DRV_NEXTHOPMET_RAM0_BASE_12W},/*CM_MEMORY8*/
{"NexthopMetRam1",0, DRV_NEXTHOPMET_RAM1_MAX_ENTRY_NUM, DRV_NEXTHOPMET_RAM1_BASE_4W, DRV_NEXTHOPMET_RAM1_BASE_6W, DRV_NEXTHOPMET_RAM1_BASE_12W},/*CM_MEMORY8*/
//{"DsFwdRam",0, DRV_DSFWD_RAM_MAX_ENTRY_NUM, DRV_DSFWD_RAM_BASE_4W, DRV_DSFWD_RAM_BASE_4W},/*CM_MEMORY8*/
{"StatsRam",0, DRV_STATS_RAM_MAX_ENTRY_NUM, DRV_STATS_RAM_BASE_4W, DRV_STATS_RAM_BASE_6W, DRV_STATS_RAM_BASE_12W},/*CM_MEMORY8*/
{"PolicerCounterRam0",0, DRV_POLICERCOUNTERRAM0_MAX_ENTRY_NUM, DRV_POLICERCOUNTERRAM0_BASE_4W, DRV_POLICERCOUNTERRAM0_BASE_6W, DRV_POLICERCOUNTERRAM0_BASE_12W},/*CM_MEMORY8*/
{"PolicerCounterRam1",0, DRV_POLICERCOUNTERRAM1_MAX_ENTRY_NUM, DRV_POLICERCOUNTERRAM1_BASE_4W, DRV_POLICERCOUNTERRAM1_BASE_6W, DRV_POLICERCOUNTERRAM1_BASE_12W},/*CM_MEMORY8*/


{"Tcam key0", 0, DRV_TCAM_KEY0_MAX_ENTRY_NUM, DRV_TCAM_KEY0_BASE_4W, 0, 0},/*CM_TCAM_KEY*/
{"Tcam key1", 0, DRV_TCAM_KEY1_MAX_ENTRY_NUM, DRV_TCAM_KEY1_BASE_4W, 0, 0},/*CM_TCAM_KEY*/
{"Tcam key2", 0, DRV_TCAM_KEY2_MAX_ENTRY_NUM, DRV_TCAM_KEY2_BASE_4W, 0, 0},/*CM_TCAM_KEY*/
{"Tcam key3", 0, DRV_TCAM_KEY3_MAX_ENTRY_NUM, DRV_TCAM_KEY3_BASE_4W, 0, 0},/*CM_TCAM_KEY*/
{"Tcam key4", 0, DRV_TCAM_KEY4_MAX_ENTRY_NUM, DRV_TCAM_KEY4_BASE_4W, 0, 0},/*CM_TCAM_KEY*/
{"Tcam key5", 0, DRV_TCAM_KEY5_MAX_ENTRY_NUM, DRV_TCAM_KEY5_BASE_4W, 0, 0},/*CM_TCAM_KEY*/
{"Tcam key6", 0, DRV_TCAM_KEY6_MAX_ENTRY_NUM, DRV_TCAM_KEY6_BASE_4W, 0, 0},/*CM_TCAM_KEY*/

{"Tcam AD0",  0, DRV_TCAM_AD0_MAX_ENTRY_NUM, DRV_TCAM_AD0_BASE_4W, DRV_TCAM_AD0_BASE_4W, DRV_TCAM_AD0_BASE_4W},/*CM_TCAM_AD*/
{"Tcam AD1",  0, DRV_TCAM_AD1_MAX_ENTRY_NUM, DRV_TCAM_AD1_BASE_4W, DRV_TCAM_AD1_BASE_4W, DRV_TCAM_AD1_BASE_4W},/*CM_TCAM_AD*/
{"Tcam AD2",  0, DRV_TCAM_AD2_MAX_ENTRY_NUM, DRV_TCAM_AD2_BASE_4W, DRV_TCAM_AD2_BASE_4W, DRV_TCAM_AD2_BASE_4W},/*CM_TCAM_AD*/
{"Tcam AD3",  0, DRV_TCAM_AD3_MAX_ENTRY_NUM, DRV_TCAM_AD3_BASE_4W, DRV_TCAM_AD3_BASE_4W, DRV_TCAM_AD3_BASE_4W},/*CM_TCAM_AD*/
{"Tcam AD4",  0, DRV_TCAM_AD4_MAX_ENTRY_NUM, DRV_TCAM_AD4_BASE_4W, DRV_TCAM_AD4_BASE_4W, DRV_TCAM_AD4_BASE_4W},/*CM_TCAM_AD*/
{"Tcam AD5",  0, DRV_TCAM_AD5_MAX_ENTRY_NUM, DRV_TCAM_AD5_BASE_4W, DRV_TCAM_AD5_BASE_4W, DRV_TCAM_AD5_BASE_4W},/*CM_TCAM_AD*/
{"Tcam AD6",  0, DRV_TCAM_AD6_MAX_ENTRY_NUM, DRV_TCAM_AD6_BASE_4W, DRV_TCAM_AD6_BASE_4W, DRV_TCAM_AD6_BASE_4W},/*CM_TCAM_AD*/

{"LPM Tcam key0", 0, DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM, DRV_LPM_TCAM_DATA_ASIC_BASE0, 0, 0},/* LPM TCAM KEY */
{"LPM Tcam key1", 0, DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM, DRV_LPM_TCAM_DATA_ASIC_BASE1, 0, 0},/* LPM TCAM KEY */

{"LPM Tcam Ad0", 0, DRV_LPM_TCAM_AD0_MAX_ENTRY_NUM, DRV_LPM_TCAM_AD_ASIC_BASE0, 0, 0},/* LPM TCAM AD */
{"LPM Tcam Ad1", 0, DRV_LPM_TCAM_AD1_MAX_ENTRY_NUM, DRV_LPM_TCAM_AD_ASIC_BASE1, 0, 0},/* LPM TCAM AD */

}; /*all mem chips' info*/

/* SDK Modify  {*/
uint32 model_lpm_tcam_ad_share_table_list[8] =
{
    DsLpmTcamAd0_t,
    DsLpmTcamIpv4Route40Ad_t,
    DsLpmTcamIpv4Route160Ad_t,
    DsLpmTcamAd1_t,
    DsLpmTcamIpv4Pbr160Ad_t,
    DsLpmTcamIpv6Pbr320Ad_t,
    DsLpmTcamIpv4Nat160Ad_t,
    DsLpmTcamIpv6Nat160Ad_t,
};
/* SDK Modify  }*/

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
 * Input      : chip_id_offset - chip id infomation
 * Output     : N/A
 * Return     : DRV_E_NONE
 *              Other = ErrCode
 * Note:      : N/A
****************************************************************************/
int32 sram_model_initialize(uint8 chip_id_offset)
{
    int32 ret = DRV_E_NONE;
    tbls_id_t tbl_id = 0;
    uint32 tbl_mem_size = 0;
    uint32 couple_mode = 0;
    uint32 entry_num = 0;


    /* Cmodel only support up to 2 same chip */
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);
    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    /* check whether the chip's sram has been malloc or not */
    if (sram_info[chip_id_offset].sram_model_initialized_flag)
    {
        return DRV_E_NONE;
    }

    if (couple_mode)
    {
        entry_num = (DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM << couple_mode);
    }
    else
    {
        //if ()
        {
            entry_num = DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM + DRV_SHARE_RAM3_MAX_ENTRY_NUM + DRV_DS_IPMAC_RAM0_MAX_ENTRY_NUM;
        }
        //else
        {
            //entry_num = DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM;
        }
    }

/* --- memory allocation for dynamic tables starts --- */
    /*Malloc one block memory for all hash/hash ad/dynamic tables*/
    sram_info[chip_id_offset].dynamic_mem_base
                    = (uintptr)sal_malloc(entry_num*DRV_BYTES_PER_ENTRY);
    if (!sram_info[chip_id_offset].dynamic_mem_base)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset((void*)sram_info[chip_id_offset].dynamic_mem_base, 0, entry_num*DRV_BYTES_PER_ENTRY);

    /*Malloc one block memory for dynamic tables wbit*/
    sram_info[chip_id_offset].dynamic_mem_wbit = (uint8*)sal_malloc(entry_num);
    if (!sram_info[chip_id_offset].dynamic_mem_wbit)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset((void*)sram_info[chip_id_offset].dynamic_mem_wbit, 0, entry_num);

    /*Malloc one block memory for tcam ad tables wbit*/
    sram_info[chip_id_offset].tcam_ad_mem_base
                    = (uintptr)sal_malloc((DRV_TCAM_AD_MAX_ENTRY_NUM << couple_mode)*DRV_BYTES_PER_ENTRY);
    if (!sram_info[chip_id_offset].tcam_ad_mem_base)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset((void*)sram_info[chip_id_offset].tcam_ad_mem_base, 0, (DRV_TCAM_AD_MAX_ENTRY_NUM << couple_mode)*DRV_BYTES_PER_ENTRY);

    sram_info[chip_id_offset].tcam_ad_mem_wbit = (uint8*)sal_malloc((DRV_TCAM_AD_MAX_ENTRY_NUM << couple_mode));
    if (!sram_info[chip_id_offset].tcam_ad_mem_wbit)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset((void*)sram_info[chip_id_offset].tcam_ad_mem_wbit, 0, (DRV_TCAM_AD_MAX_ENTRY_NUM << couple_mode));


    /*Malloc one block memory for lpm tcam ad tables wbit*/
    sram_info[chip_id_offset].lpm_tcam_ad_mem_base
                    = (uintptr)sal_malloc((DRV_LPM_TCAM_AD0_MAX_ENTRY_NUM << couple_mode)*DRV_LPM_AD0_BYTE_PER_ENTRY
                                        + (DRV_LPM_TCAM_AD1_MAX_ENTRY_NUM )*DRV_LPM_AD1_BYTE_PER_ENTRY);
    if (!sram_info[chip_id_offset].lpm_tcam_ad_mem_base)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset((void*)sram_info[chip_id_offset].lpm_tcam_ad_mem_base, 0, ((DRV_LPM_TCAM_AD0_MAX_ENTRY_NUM << couple_mode)*DRV_LPM_AD0_BYTE_PER_ENTRY
                                                        + (DRV_LPM_TCAM_AD1_MAX_ENTRY_NUM )*DRV_LPM_AD1_BYTE_PER_ENTRY));

    sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit = (uint8*)sal_malloc((DRV_LPM_TCAM_AD0_MAX_ENTRY_NUM << couple_mode) + DRV_LPM_TCAM_AD1_MAX_ENTRY_NUM);
    if (!sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit)
    {
        return DRV_E_NO_MEMORY;
    }
    sal_memset((void*)sram_info[chip_id_offset].lpm_tcam_ad_mem_wbit, 0, ((DRV_LPM_TCAM_AD0_MAX_ENTRY_NUM << couple_mode) + DRV_LPM_TCAM_AD1_MAX_ENTRY_NUM));

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
        sal_memset((uintptr *)cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base, 0, tbl_mem_size);

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
 * Input       : chip_id_offset - chip id infomation.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 sram_model_release(uint8 chip_id_offset)
{
    uint32 tbl_id = 0;
    uintptr* sw_mem_ptr = NULL;

    /* Cmodel only support up to 2 same chip */
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* release each tbl's memory */
    for(tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
    {
        if (!cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base)
        {
            sw_mem_ptr = (uintptr *)cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base;
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
        sw_mem_ptr = (uintptr *)sram_info[chip_id_offset].dynamic_mem_base;
        sal_free(sw_mem_ptr);
        sram_info[chip_id_offset].dynamic_mem_base = 0;
    }

    if (!sram_info[chip_id_offset].tcam_ad_mem_base)
    {
        sw_mem_ptr = (uintptr *)sram_info[chip_id_offset].tcam_ad_mem_base;
        sal_free(sw_mem_ptr);
        sram_info[chip_id_offset].tcam_ad_mem_base = 0;
    }

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : sram_model_read
 * Purpose     : The function read data from sram to controller.
 * Input       : chip_id_offset - chip id infomation.
                 sw_model_addr - memory model's address.
                 data - the read data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 sram_model_read(uint8  chip_id_offset,
                      uintptr sw_model_addr,
                      uint32 *data,
                      int32  len)
{
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* May be add code to check whether the address is valid or not */

    SRAM_MODEL_PTR_VALID_CHECK(data);

    sal_memcpy(data, (uintptr *)sw_model_addr, len);

    return DRV_E_NONE;
}


/****************************************************************************
 * Name        : sram_model_write
 * Purpose     : The function wirte data from sram to controller.
 * Input       : chip_id_offset - chip id infomation.
                 sw_model_addr - memory model's address.
                 data - the writed data.
                 len - the data length.
 * Output      : N/A.
 * Return      : SUCCESS.
 *               Other = ErrCode.
 * Note        : N/A.
****************************************************************************/
int32 sram_model_write(uint8  chip_id_offset,
                       uintptr sw_model_addr,
                       uint32 *data,
                       int32  len)
{
    SRAM_CHIP_ID_VALID_CHECK(chip_id_offset);

    /* May be add code to check whether the address is valid or not */

    SRAM_MODEL_PTR_VALID_CHECK(data);

    sal_memcpy((uintptr *)sw_model_addr, data,  len);

    /* set wbit for dump after write action(move to drv) */

    return DRV_E_NONE;
}

/****************************************************************************
 * Name         : sram_model_reset
 * Purpose      : reset the sram's value.
 * Input        : chip_id_offset - chip id infomation.
 * Output       : N/A.
 * Return       : SUCCESS.
 *                Other = ErrCode.
 * Note         : N/A.
****************************************************************************/
int32
sram_model_reset(uint8 chip_id_offset)
{
    uint32 tbl_id = 0;
    uint32 couple_mode = 0;

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
    {
        uint32 tbl_mem_size = TABLE_MAX_INDEX(tbl_id) * TABLE_ENTRY_SIZE(tbl_id);

        if (cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base)
        {
            sal_memset((uintptr *)cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base, 0, tbl_mem_size);
        }

        if (sram_info[chip_id_offset].sram_model_wbit[tbl_id])
        {
            sal_memset(sram_info[chip_id_offset].sram_model_wbit[tbl_id], 0, tbl_mem_size/DRV_BYTES_PER_ENTRY);
        }
    }

    if (!sram_info[chip_id_offset].dynamic_mem_wbit)
    {
        sal_memset(sram_info[chip_id_offset].dynamic_mem_wbit, 0, (DRV_DYNAMIC_SRAM_MAX_ENTRY_NUM << couple_mode));
    }

    if (!sram_info[chip_id_offset].tcam_ad_mem_wbit)
    {
        sal_memset(sram_info[chip_id_offset].tcam_ad_mem_wbit, 0, (DRV_TCAM_AD_MAX_ENTRY_NUM << couple_mode));
    }

    return DRV_E_NONE;
}


int32
sram_model_get_sw_address(uint8 chip_id_offset, tbls_id_t tbl_id, uint32 index, uintptr *sw_model_addr)
{
    if (drv_goldengate_table_is_tcam_key(tbl_id)
       || drv_goldengate_table_is_lpm_tcam_key(tbl_id)) /* isTcamKeyTable */
    {
        return DRV_E_INVALID_PARAMETER;
    }

    if (drv_goldengate_table_is_dynamic_table(tbl_id))
    {

        if (DYNAMIC_ACCESS_MODE(tbl_id) == DYNAMIC_8W_MODE) /* special process, 8word table but index is per-80bits */
        {
            *sw_model_addr = cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base + index*DRV_BYTES_PER_ENTRY;
            return DRV_E_NONE;
        }
    }

    *sw_model_addr = cmodel_tbls_info[chip_id_offset][tbl_id].sw_data_base + index*TABLE_ENTRY_SIZE(tbl_id);

    return DRV_E_NONE;
}


int8 sram_model_is_tcam_ad_table(uint8 chip_id_offset, tbls_id_t tbl_id)
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

int32 sram_model_get_sw_address_by_tbl_id_and_index(uint8 chip_id_offset, tbls_id_t tbl_id,
                                                                        uint32 index, uintptr *start_data_addr, uint32 is_dump_cfg )
{
    uintptr sw_data_base = 0;
    uint8 blk_id = 0;

    DRV_TBL_ID_VALID_CHECK(tbl_id);
    DRV_TBL_INDEX_VALID_CHECK(tbl_id, index);

    if (drv_goldengate_table_is_tcam_key(tbl_id)
        || drv_goldengate_table_is_lpm_tcam_key(tbl_id))
    {
        DRV_DBG_INFO("ERROR (sram_model_get_sw_address_by_tbl_id_and_index): chip-0x%x, tbl-%d, tcam key call the function only for non tcam key.\n",
                        chip_id_offset + drv_gg_init_chip_info.drv_init_chipid_base, tbl_id);
        return DRV_E_INVALID_TBL;
    }

    if (drv_goldengate_table_is_dynamic_table(tbl_id))
    {
        for (blk_id = 0; blk_id < MAX_DRV_BLOCK_NUM; blk_id++)
        {
            if (!IS_BIT_SET(DYNAMIC_BITMAP(tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= DYNAMIC_START_INDEX(tbl_id, blk_id)) && (index <=DYNAMIC_END_INDEX(tbl_id, blk_id)))
            {
                sw_data_base = SRAM_DATA_BASE(chip_id_offset, tbl_id, blk_id);
                break;
            }
        }

        if (DYNAMIC_ACCESS_MODE(tbl_id) == DYNAMIC_8W_MODE)
        {
            if ((index%2) && (!is_dump_cfg))
            {
                DRV_DBG_INFO("ERROR!! get tbl_id %d index must be even! now index = %d\n", tbl_id, index);
                return DRV_E_INVALID_INDEX;
            }

            *start_data_addr = sw_data_base + (index - DYNAMIC_START_INDEX(tbl_id, blk_id)) * DRV_BYTES_PER_ENTRY;
        }
        else if (DYNAMIC_ACCESS_MODE(tbl_id) == DYNAMIC_16W_MODE)
        {
            if ((index%4) && (!is_dump_cfg))
            {
                DRV_DBG_INFO("ERROR!! get tbl_id %d index must be times of 4! now index = %d\n", tbl_id, index);
                return DRV_E_INVALID_INDEX;
            }

            *start_data_addr = sw_data_base + (index - DYNAMIC_START_INDEX(tbl_id, blk_id)) * DRV_BYTES_PER_ENTRY;
        }
        else
        {
            *start_data_addr = sw_data_base + (index - DYNAMIC_START_INDEX(tbl_id, blk_id))* TABLE_ENTRY_SIZE(tbl_id);
        }
    }
    else if (drv_goldengate_table_is_tcam_ad(tbl_id))
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {
            if (!IS_BIT_SET(TCAM_BITMAP(tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(tbl_id, blk_id)) && (index <=TCAM_END_INDEX(tbl_id, blk_id)))
            {
                sw_data_base = SRAM_TCAM_DATA_BASE(chip_id_offset, tbl_id, blk_id);
                break;
            }
        }
        *start_data_addr = sw_data_base + (index - TCAM_START_INDEX(tbl_id, blk_id))* TABLE_ENTRY_SIZE(tbl_id);
        //sal_printf("ReadAdrr:Tbl[%d], blk:%d, index:%d, swDataBase:0x%x\n ", tbl_id, blk_id, index, sw_data_base);
    }
    else
    {
        sw_data_base = SRAM_SW_DATA_BASE(chip_id_offset, tbl_id);

        *start_data_addr = sw_data_base + index * TABLE_ENTRY_SIZE(tbl_id);
    }

    return DRV_E_NONE;
}

STATIC uint32
sram_model_get_tcam_ad_addr_offset(uint8 block_num)
{
    uint32 hw_base_addr = 0;

    switch(block_num)
    {
        case 0:
            hw_base_addr = DRV_TCAM_AD0_BASE_4W;
            break;
        case 1:
            hw_base_addr = DRV_TCAM_AD1_BASE_4W;
            break;
        case 2:
            hw_base_addr = DRV_TCAM_AD2_BASE_4W;
            break;
        case 3:
            hw_base_addr = DRV_TCAM_AD3_BASE_4W;
            break;
        case 4:
            hw_base_addr = DRV_TCAM_AD4_BASE_4W;
            break;
        case 5:
            hw_base_addr = DRV_TCAM_AD5_BASE_4W;
            break;
        default:
            hw_base_addr = DRV_TCAM_AD6_BASE_4W;
            break;
    }

    return hw_base_addr;

}

STATIC uint32
sram_model_get_tcam_ad_blk_offset(uint32 blk_id)
{
    uint32 blk_idx = 0;
    uint32 blk_offset = 0;
    uint32 couple_mode = 0;

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    for (blk_idx = CM_TCAM_AD0; blk_idx < CM_LPM_TCAM_KEY0; blk_idx++)
    {
        if (blk_id == (blk_idx - CM_TCAM_AD0))
        {
            break;
        }
        blk_offset += (model_mem_chip_info[blk_idx].max_mem_entry_num << couple_mode);
    }
    return blk_offset;
}

STATIC int8
sram_model_is_lpm_tcam_ad_tbl(uint32 tbl_id)
{
    uint32 index = 0;
    uint32 len = sizeof(model_lpm_tcam_ad_share_table_list)/sizeof(model_lpm_tcam_ad_share_table_list[0]);

    for (index = 0; index < len; index++)
    {
        if (tbl_id == model_lpm_tcam_ad_share_table_list[index])
        {
            return TRUE;
        }

    }

    return FALSE;
}

STATIC uint32
sram_model_get_dynamic_sram_blk_offset(uint32 blk_id)
{
    uint32 blk_idx = 0;
    uint32 blk_offset = 0;
    uint32 couple_mode = 0;

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    for (blk_idx = 0; blk_idx < CM_TCAM_KEY0; blk_idx++)
    {
        if (blk_id == blk_idx)
        {
            break;
        }

        if ((CM_SHARE_RAM3 == blk_idx)
            || (CM_DS_IPMAC_RAM0 == blk_idx))
        {
            /* ipfix is couple mode default */
            blk_offset += (model_mem_chip_info[blk_idx].max_mem_entry_num << 1);
        }
        else
        {
            blk_offset += (model_mem_chip_info[blk_idx].max_mem_entry_num << couple_mode);
        }
    }
    return blk_offset;
}
/****************************************************************************
 * Name:       sram_model_allocate_sram_tbl_info
 * Purpose:    write dynamic and tcam ad table info to sram model, only for sram model exist
 * Parameters:
 * Input:      void
 * Output:     None
 * Return:     DRV_E_NONE = success.
 *             Other = Error, please refer to DRV_E_XXX.
 * Note:       none.
 ****************************************************************************/
/*SDK Modify*/
int32 sram_model_allocate_sram_tbl_info(void)
{
    uint8 chipid = 0;
    uint8 chip_num = 0;
    uint8 addr_offset = 0;
    uint8 chip_id_base = 0;
    uint8 chip_id_offset = 0;
    tbls_id_t tbl_id = MaxTblId_t;
    /*
    oam_stats_static_tbl_id = MaxTblId_t;
    */
    sram_dynamic_info_t *sram_dynamic_info_ptr = NULL;
    sram_tcam_info_t *sram_tcam_ptr = NULL;
    uint32 memory_base_addr = 0;
    uint32 blk_id = 0, blk_addr_offset = 0;
    /*
    uint8 oam_lm_stats_num = 0, oam_lm_stats_static_tbl_index = 0;
    */
    uint32 couple_mode = 0;

    DRV_IF_ERROR_RETURN(drv_goldengate_get_dynamic_ram_couple_mode(&couple_mode));

    DRV_IF_ERROR_RETURN(drv_goldengate_get_chipnum(&chip_num));
    DRV_IF_ERROR_RETURN(drv_goldengate_get_chipid_base(&chip_id_base));

    /* for cmodel support multi chip */
    for (chipid = chip_id_base; chipid < chip_id_base + chip_num; chipid++)
    {
        chip_id_offset = chipid - chip_id_base;

        if (SRAM_INFO_MODEL_INIT_FLAG(chip_id_offset) == FALSE)
        {
            return DRV_E_INVALID_DYNIC_TBL_ALLOC;
        }

        for (tbl_id = 0; tbl_id < MaxTblId_t; tbl_id++)
        {
		/*SDK Modify {*/
            if (TABLE_MAX_INDEX(tbl_id) == 0)
            {
                continue;
            }

            /* do tcam ad table sram model operation */
            if (drv_goldengate_table_is_tcam_ad(tbl_id))
            {
                sram_tcam_ptr = (sram_tcam_info_t *)sal_malloc(sizeof(sram_tcam_info_t));
                if (sram_tcam_ptr == NULL )
                {
                    SAL_LOG_FATAL("%s:%d ******** malloc sram tcam ad table error, no enough memory!!********\n",
                                    __FUNCTION__, __LINE__);
                    exit(1);
                }
                sal_memset(sram_tcam_ptr, 0, sizeof(sram_tcam_info_t));

                sram_tcam_ptr->sram_tcam_bitmap = TCAM_BITMAP(tbl_id);

                for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
                {
                    if (! (TCAM_BITMAP(tbl_id) & (1 << (blk_id))))
                    {
                        continue;
                    }

                    sram_tcam_ptr->sram_tcam_entry_num[blk_id] = TCAM_ENTRY_NUM(tbl_id, blk_id);
                    sram_tcam_ptr->sram_tcam_entry_start_index[blk_id] = TCAM_START_INDEX(tbl_id, blk_id);
                    sram_tcam_ptr->sram_tcam_entry_end_index[blk_id] = TCAM_END_INDEX(tbl_id, blk_id);

                    blk_addr_offset = sram_model_get_tcam_ad_blk_offset(blk_id);
                    memory_base_addr = sram_model_get_tcam_ad_addr_offset(blk_id);

                    /* one tcam can not share by more than one table */
                    //memory_base_addr =  TCAM_DATA_BASE(tbl_id, blk_id);
                    sram_tcam_ptr->sram_tcam_sw_data_base[blk_id] = SRAM_INFO_TCAM_AD_MEM_BASE(chip_id_offset)
                        + blk_addr_offset*DRV_BYTES_PER_ENTRY
                        + (TCAM_DATA_BASE(tbl_id, blk_id, addr_offset) - memory_base_addr)/DRV_ADDR_BYTES_PER_ENTRY*DRV_BYTES_PER_ENTRY;

                    //sal_printf("AD[%d] blk:%d, offset:%d\n", tbl_id, blk_id, (TCAM_DATA_BASE(tbl_id, blk_id, addr_offset) - memory_base_addr)/DRV_ADDR_BYTES_PER_ENTRY);
                    sram_tcam_ptr->sram_tcam_wbit_sw_data_base[blk_id] = (uintptr)SRAM_INFO_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset)
                        + blk_addr_offset+ (TCAM_DATA_BASE(tbl_id, blk_id, addr_offset) - memory_base_addr)/DRV_ADDR_BYTES_PER_ENTRY;

                }
                ((SRAM_TBL_INFO(chip_id_offset, tbl_id).tcam_info_ptr)) = sram_tcam_ptr;
                sram_tcam_ptr = NULL;

                /* set flag to indicated table is tcam ad,
                   because dump tcam ad is per 80 bit, so dump tcam ad is different of STATIC table */
                SRAM_TBL_TYPE(chip_id_offset, tbl_id) = SRAM_TBL_TYPE_TCAM_AD;

                continue;
            }

            /* do lpm tcam table sram model operation */
            if (sram_model_is_lpm_tcam_ad_tbl(tbl_id))
            {
                if (TABLE_DATA_BASE(tbl_id, addr_offset)  <  DRV_LPM_TCAM_AD_ASIC_BASE1)
                {
                    memory_base_addr = DRV_LPM_TCAM_AD_ASIC_BASE0;
                    SRAM_SW_DATA_BASE(chip_id_offset, tbl_id) =
                    SRAM_INFO_LPM_TCAM_AD_MEM_BASE(chip_id_offset)
                     + (TABLE_DATA_BASE(tbl_id, addr_offset) - memory_base_addr) / DRV_LPM_AD0_BYTE_PER_ENTRY*DRV_LPM_AD0_BYTE_PER_ENTRY;

                    /* set lpm tcam ad wbit info to sram_model_wbit[MaxTblId_t],
                    in driver io , can use tbl_id to get wbit of tcam ad, same of statis table */
                    SRAM_INFO_MODEL_TBL_WBIT_PTR(chip_id_offset, tbl_id) =
                    SRAM_INFO_LPM_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset)
                    +  (TABLE_DATA_BASE(tbl_id, addr_offset) - memory_base_addr)/DRV_LPM_AD0_BYTE_PER_ENTRY;
                }
                else
                {
                    memory_base_addr = DRV_LPM_TCAM_AD_ASIC_BASE1;
                    blk_addr_offset = (DRV_LPM_TCAM_AD0_MAX_ENTRY_NUM << couple_mode);
                    SRAM_SW_DATA_BASE(chip_id_offset, tbl_id) =
                    SRAM_INFO_LPM_TCAM_AD_MEM_BASE(chip_id_offset)
                    + blk_addr_offset*DRV_LPM_AD0_BYTE_PER_ENTRY
                    + (TABLE_DATA_BASE(tbl_id, addr_offset) - memory_base_addr) / DRV_LPM_AD1_BYTE_PER_ENTRY*DRV_LPM_AD1_BYTE_PER_ENTRY;

                    /* set lpm tcam ad wbit info to sram_model_wbit[MaxTblId_t],
                    in driver io , can use tbl_id to get wbit of tcam ad, same of statis table */
                    SRAM_INFO_MODEL_TBL_WBIT_PTR(chip_id_offset, tbl_id) =
                    SRAM_INFO_LPM_TCAM_AD_MEM_WBIT_BASE_PTR(chip_id_offset) + blk_addr_offset
                  +  (TABLE_DATA_BASE(tbl_id, addr_offset) - memory_base_addr)/DRV_LPM_AD1_BYTE_PER_ENTRY;

                }
                /* set flag to indicated table is tcam ad,
                   because dump tcam ad is per 80 bit, so dump tcam ad is different of STATIC table */
                SRAM_TBL_TYPE(chip_id_offset, tbl_id) = SRAM_TBL_TYPE_TCAM_AD;

                continue;
            }
      		/*SDK Modify }*/
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

            sram_dynamic_info_ptr = (sram_dynamic_info_t *)sal_malloc(sizeof(sram_dynamic_info_t));
            if (sram_dynamic_info_ptr == NULL )
            {
                SAL_LOG_FATAL("%s:%d ******** malloc sram dynamic table error, no enough memory!!********\n",
                                __FUNCTION__, __LINE__);
                exit(1);
            }
            sal_memset(sram_dynamic_info_ptr, 0, sizeof(sram_dynamic_info_t));

            sram_dynamic_info_ptr->sram_dynamic_bitmap = DYNAMIC_BITMAP(tbl_id);

            for (blk_id = 0; blk_id < MAX_MEMORY_BLOCK_NUM; blk_id++)
            {
                sram_dynamic_info_ptr->sram_dynamic_entry_num[blk_id] = DYNAMIC_ENTRY_NUM(tbl_id, blk_id);;
                sram_dynamic_info_ptr->sram_dynamic_entry_start_index[blk_id] = DYNAMIC_START_INDEX(tbl_id, blk_id);
                sram_dynamic_info_ptr->sram_dynamic_entry_end_index[blk_id] = DYNAMIC_END_INDEX(tbl_id, blk_id);
/*SDK modify {*/
#if (SDK_COSIM == 1)
                    memory_base_addr = model_mem_chip_info[blk_id].hardware_data_base_address;
#else
                if (TABLE_ENTRY_SIZE(tbl_id) == (4 * DRV_BYTES_PER_ENTRY))
                {
                    memory_base_addr = model_mem_chip_info[blk_id].hardware_data_base_address_12w;
                }
                else if (TABLE_ENTRY_SIZE(tbl_id) == (2 * DRV_BYTES_PER_ENTRY))
                {
                    memory_base_addr = model_mem_chip_info[blk_id].hardware_data_base_address_6w;
                }
                else if (TABLE_ENTRY_SIZE(tbl_id) == DRV_BYTES_PER_ENTRY)
                {
                    memory_base_addr = model_mem_chip_info[blk_id].hardware_data_base_address;
                }
#endif
/*SDK modify }*/
                blk_addr_offset = sram_model_get_dynamic_sram_blk_offset(blk_id);

                sram_dynamic_info_ptr->sram_dynamic_sw_data_base[blk_id] = sram_info[chip_id_offset].dynamic_mem_base
                        + blk_addr_offset*DRV_BYTES_PER_ENTRY
                        + (DYNAMIC_DATA_BASE(tbl_id, blk_id, addr_offset) - memory_base_addr)/DRV_ADDR_BYTES_PER_ENTRY*DRV_BYTES_PER_ENTRY;

                //printf("%s:%d,table:%d,sw_data_base:%x,\n",__FUNCTION__,__LINE__,tbl_id, sram_dynamic_info_ptr->sram_dynamic_sw_data_base[0]);

                sram_dynamic_info_ptr->sram_dynamic_wbit_sw_data_base[blk_id] = (uintptr)sram_info[chip_id_offset].dynamic_mem_wbit
                        + blk_addr_offset + (DYNAMIC_DATA_BASE(tbl_id, blk_id, addr_offset) - memory_base_addr)/DRV_ADDR_BYTES_PER_ENTRY;

                sram_dynamic_info_ptr->sram_access_mode = DYNAMIC_ACCESS_MODE(tbl_id);
            }

            (SRAM_TBL_INFO(chip_id_offset, tbl_id).sram_dynamic_info_ptr) = sram_dynamic_info_ptr;
            sram_dynamic_info_ptr = NULL;

        }

    }

    return DRV_E_NONE;
}

#endif

