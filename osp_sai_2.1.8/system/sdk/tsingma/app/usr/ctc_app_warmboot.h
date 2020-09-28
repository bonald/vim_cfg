

/**
 @file ctc_warmboot.h

 @date 2016-04-13

 @version v5.0

 The file defines warmboot api
*/
#ifndef _CTC_APP_WARMBOOT_H_
#define _CTC_APP_WARMBOOT_H_
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define LOAD_FILE_TO_MEMORY     0
#define LOAD_MEMORY_TO_FILE     1


/**********************************************************************************
                      Define API function interfaces
 ***********************************************************************************/
extern int32
ctc_app_wb_init(uint8 lchip, uint8 reloading);
extern int32
ctc_app_wb_init_done(uint8 lchip);
extern int32
ctc_app_wb_sync(uint8 lchip);
extern int32
ctc_app_wb_sync_done(uint8 lchip, int32 result);
extern int32
ctc_app_wb_add_entry(ctc_wb_data_t *data);
extern int32
ctc_app_wb_query_entry(ctc_wb_query_t *query);



#ifdef __cplusplus
}
#endif

#endif  /* _CTC_APP_WARMBOOT_H_*/

