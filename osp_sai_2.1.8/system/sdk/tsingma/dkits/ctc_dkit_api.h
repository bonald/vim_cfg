/**********************************************************
 * ctc_dkit_api.h
 * Date:
 * Author: auto generate from include file
 **********************************************************/
#ifndef _CTC_DKIT_API_H
#define _CTC_DKIT_API_H
#ifdef __cplusplus
extern "C" {
#endif
/**********************************************************
 *
 * Header file
 *
 **********************************************************/
#include "sal_types.h"

/**********************************************************
 *
 * Defines and macros
 *
 **********************************************************/
#define CTC_MAX_TABLE_DATA_LEN      128             /**< [GB.GG] register max word size*/

struct ctc_dkit_tbl_reg_wr_s
{
    uint8*  table_name;                             /**<[GB.GG] pointer to table name*/
    uint8*  field_name;                             /**<[GB.GG] pointer to field name*/
    uint32  table_index;                            /**<[GB.GG] table index */

    uint32  value[CTC_MAX_TABLE_DATA_LEN];          /**<[GB.GG] The table or register value */
    uint32  mask[CTC_MAX_TABLE_DATA_LEN];           /**<[GB.GG] The table mask*/
};
typedef struct ctc_dkit_tbl_reg_wr_s ctc_dkit_tbl_reg_wr_t;

/**********************************************************
 *
 * Functions
 *
 **********************************************************/
/**
 @brief read chip table and register

 @param[in] lchip_id    local chip id

 @param[in|out]  tbl_info refer to ctc_dkit_tbl_reg_wr_t

 @remark read chip register

 @return CTC_E_XXX

*/
extern int32
ctc_dkit_read_table(uint8 lchip_id,  ctc_dkit_tbl_reg_wr_t* tbl_info);

/**
 @brief write chip table and register

 @param[in] lchip_id    local chip id

 @param[in]  tbl_info refer to ctc_dkit_tbl_reg_wr_t

 @remark write chip register

 @return CTC_E_XXX

*/
extern int32
ctc_dkit_write_table(uint8 lchip_id,  ctc_dkit_tbl_reg_wr_t* tbl_info);

#ifdef __cplusplus
}
#endif
#endif

