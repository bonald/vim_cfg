
/**
 @file ctc_warmboot.h

 @date 2016-04-13

 @version v5.0

 The file defines warmboot api
*/

#ifndef _CTC_WARMBOOT_H_
#define _CTC_WARMBOOT_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_mix.h"
#include "common/include/ctc_error.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define CTC_WB_DATA_BUFFER_LENGTH    102400

#define CTC_WB_STATUS_DONE       1
#define CTC_WB_STATUS_RELOADING     2
#define CTC_WB_STATUS_SYNC               3

#define CTC_WB_STATUS(lchip)       (g_wb_master ? g_wb_master->wb_status[lchip] : 0)

#define CTC_WB_ENABLE       (g_wb_master ? g_wb_master->enable : 0)

#define CTC_WB_VERSION(V_Release, V_Build)  (((V_Release)<<16)|V_Build)
#define CTC_WB_R_VERSION(V)                         (((V)&0xFFFF0000)>>16)
#define CTC_WB_B_VERSION(V)                         ((V)&0x0000FFFF)

#define CTC_WB_VERSION_CHECK(version, wb_version)\
    (CTC_WB_R_VERSION(version) != CTC_WB_R_VERSION(wb_version))

#define CTC_WB_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(wb, wb, WB_CTC, level, FMT, ## __VA_ARGS__);  \
    } while (0)

struct ctc_wb_key_data_s
{
    uint32   key[0];    /**< [D2.TM]Key of struct */
    uint32   data[0];   /**< [D2.TM]Data of struct */
};
typedef struct ctc_wb_key_data_s  ctc_wb_key_data_t;

struct ctc_wb_data_s
{
    uint32  app_id;  /**< [D2.TM]The table id of struct */
    uint32  key_len;   /**< [D2.TM]The length of struct key, key length of all entry must equal */
    uint32  data_len;   /**< [D2.TM]The length of struct data, data length of all entry maybe not equal */
    uint32  valid_cnt;   /**< [D2.TM]Operated count every time */

    uint32  buffer_len; /**< [D2.TM]The max length of data buffer */
    ctc_wb_key_data_t  *buffer; /**< [D2.TM]Buffer storage of all data*/
};
typedef struct ctc_wb_data_s  ctc_wb_data_t;


struct ctc_wb_query_s
{
    uint8    query_type; /**< [D2.TM]Query type, if set 0, means query all, if set 1, means query by key */
    uint8    rsv0[3];
    uint32   app_id;  /**< [D2.TM]The table id of struct */
    uint32   key_len; /**< [D2.TM]The length of struct key, key length of all entry must equal */
    uint8    *key;  /**< [D2.TM]Data buffer for query key, If query type equal to 1 */

   /*query_rst*/
    uint32    cursor; /**< [D2.TM]If it is first query, it is equal to 0, else using result cursor */
    uint32    data_len; /**< [D2.TM] Return the data length by query */
    uint8     is_end; /**< [D2.TM]The query flag, if set 1, means the end of query */
    uint8     rsv1;
    uint16    valid_cnt; /**< [D2.TM]Return the count of entry in data buffer */
    uint32    buffer_len;  /**< [D2.TM]The max length of query data buffer */
    ctc_wb_key_data_t  *buffer ; /**< [D2.TM]The query data buffer */
};
typedef struct ctc_wb_query_s ctc_wb_query_t;

typedef int32 (* ctc_wb_init_fn)(uint8 lchip, uint8 reloading);
typedef int32 (* ctc_wb_init_done_fn)(uint8 lchip);
typedef int32 (* ctc_wb_sync_fn)(uint8 lchip);
typedef int32 (* ctc_wb_sync_done_fn)(uint8 lchip, int32 result);
typedef int32 (* ctc_wb_add_entry_fn)(ctc_wb_data_t *data);
typedef int32 (* ctc_wb_query_entry_fn)(ctc_wb_query_t *query);


struct ctc_wb_api_s
{
    uint8  enable;                      /**< [D2.TM]Enable or disable warmboot */
    uint8  reloading;                   /**< [D2.TM]Whether or not init with reloading */
    ctc_wb_init_fn               init;  /**< [D2.TM]Callback of init */
    ctc_wb_init_done_fn       init_done;    /**< [D2.TM]Callback of init done */
    ctc_wb_sync_fn              sync;   /**< [D2.TM]Callback of sync */
    ctc_wb_sync_done_fn     sync_done;  /**< [D2.TM]Callback of sync done */
    ctc_wb_add_entry_fn       add_entry;    /**< [D2.TM]Callback of add entry */
    ctc_wb_query_entry_fn     query_entry;  /**< [D2.TM]Callback of query entry */
};
typedef struct ctc_wb_api_s ctc_wb_api_t;


struct ctc_wb_master_s
{
    uint8 enable;                       /**< [D2.TM]Enable or disable warmboot */
    uint8 wb_status[CTC_MAX_LOCAL_CHIP_NUM];/**< [D2.TM]The status of warmboot, refer to CTC_WB_STATUS(lchip)_XXX */
    ctc_wb_init_fn               init;  /**< [D2.TM]Callback of init */
    ctc_wb_init_done_fn       init_done;    /**< [D2.TM]Callback of init done */
    ctc_wb_sync_fn              sync;   /**< [D2.TM]Callback of sync */
    ctc_wb_sync_done_fn     sync_done;  /**< [D2.TM]Callback of sync done */
    ctc_wb_add_entry_fn      add_entry;    /**< [D2.TM]Callback of add entry */
    ctc_wb_query_entry_fn     query_entry;  /**< [D2.TM]Callback of query entry */
};
typedef struct ctc_wb_master_s ctc_wb_master_t;

extern ctc_wb_master_t *g_wb_master;

#define CTC_WB_INIT_DATA_T(p_data,T, module_id, subid)   \
    p_data->app_id = (lchip << 16) | ( module_id << 8) | subid;  \
    p_data->key_len = CTC_OFFSET_OF(T, calc_key_len);     \
    p_data->data_len = sizeof(T) - p_data->key_len;          \
    p_data->valid_cnt = 0;     \
    p_data->buffer_len = CTC_WB_DATA_BUFFER_LENGTH;     \

#define CTC_WB_INIT_QUERY_T(p_query, T, module_id, subid)   \
    p_query->query_type = 0;   \
    p_query->app_id = (lchip << 16) | ( module_id << 8) | subid;   \
    p_query->key_len = CTC_OFFSET_OF(T, calc_key_len);     \
    p_query->data_len = sizeof(T) - p_query->key_len;          \
    p_query->key = NULL;   \
    p_query->cursor = 0;   \
    p_query->valid_cnt = 0;    \
    p_query->buffer_len = CTC_WB_DATA_BUFFER_LENGTH;\

#define CTC_WB_QUERY_ENTRY_BEGIN(p_query)   \
   do   \
   {        \
        entry_cnt = 0;  \
        CTC_ERROR_GOTO(ctc_wb_query_entry(p_query), ret, done);   \
        while (entry_cnt < p_query->valid_cnt)     \
        {

#define CTC_WB_QUERY_ENTRY_END(p_query)   \
        };  \
   }    \
   while (!p_query->is_end);


/**********************************************************************************
                      Define API function interfaces
 ***********************************************************************************/
/**
 @brief Init warmboot module

 @param[in] lchip    lchip valid if ctcs api enable

 @param[in] wb_api warmboot api struct

 @remark
        Init warmboot resource.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_init(uint8 lchip, ctc_wb_api_t* wb_api);

/**
 @brief Init warmboot done

 @param[in] lchip    lchip valid if ctcs api enable

 @remark
        Recover the resource of warmboot, which alloc by warmboot init.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_init_done(uint8 lchip);

/**
 @brief Sync up all module

 @param[in] lchip    lchip valid if ctcs api enable

 @remark
        Sync up the data of all module to external storage file.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_sync(uint8 lchip);

/**
 @brief Sync up done

 @param[in] lchip    lchip valid if ctcs api enable

 @param[in] result the result of sync up

 @remark
        Recover the resource by sync up if necessary.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_sync_done(uint8 lchip, int32 result);

/**
 @brief Add multiple or one entry

 @param[in] data the data struct

 @remark
        Add multiple entry to external storage file, each key length must equal.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_add_entry(ctc_wb_data_t* data);

/**
 @brief Query all or one entry

 @param[in] query the query struct

 @remark
        Query all entry if query type is 0, query one entry by key if query type is 1.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_query_entry(ctc_wb_query_t* query);

/**
 @brief Set cpu rx status

 @param[in] lchip    lchip valid if ctcs api enable

 @param[in] enable 1 or 0

 @remark
        Set cpu rx to enable or disable.

 @return CTC_E_XXX

*/
extern int32
ctc_wb_set_cpu_rx_en(uint8 lchip, bool enable);

/**
 @brief Get cpu rx status

 @param[in] lchip    lchip valid if ctcs api enable

 @param[out] enable cpu rx status

 @remark
        Get cpu rx status

 @return CTC_E_XXX

*/
extern int32
ctc_wb_get_cpu_rx_en(uint8 lchip, bool* enable);

/**
 @brief Deinit warmboot module

 @param[in] lchip    lchip valid if ctcs api enable

 @remark
        Deinit warmboot resource.

 @return CTC_E_XXX

*/
extern int32 ctc_wb_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif  /* _CTC_WARMBOOT_H_*/
