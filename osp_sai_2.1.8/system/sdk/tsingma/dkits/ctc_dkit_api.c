/**********************************************************
 * ctc_dkit_api.c
 * Date:
 * Author: auto generate from include file
 **********************************************************/
/**********************************************************
 *
 * Header file
 *
 **********************************************************/
#include "ctc_dkit.h"
#include "ctc_dkit_api.h"
/**********************************************************
 *
 * Defines and macros
 *
 **********************************************************/
/**********************************************************
 *
 * Global and Declaration
 *
 **********************************************************/
extern ctc_dkit_api_t* g_dkit_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];
/**********************************************************
 *
 * Functions
 *
 **********************************************************/
/**
 @brief read chip register
*/
int32
ctc_dkit_read_table(uint8 lchip_id,  ctc_dkit_tbl_reg_wr_t* tbl_info)
{
    CTC_DKIT_LCHIP_CHECK(lchip_id);

    CTC_DKIT_PTR_VALID_CHECK(g_dkit_api[lchip_id]);
    CTC_DKIT_API_ERROR_RETURN(g_dkit_api[lchip_id]->read_table, lchip_id, (void*)tbl_info);
}

/**
 @brief write chip register
*/
int32
ctc_dkit_write_table(uint8 lchip_id,  ctc_dkit_tbl_reg_wr_t* tbl_info)
{
    CTC_DKIT_LCHIP_CHECK(lchip_id);

    CTC_DKIT_PTR_VALID_CHECK(g_dkit_api[lchip_id]);
    CTC_DKIT_API_ERROR_RETURN(g_dkit_api[lchip_id]->write_table, lchip_id, (void*)tbl_info);
}

