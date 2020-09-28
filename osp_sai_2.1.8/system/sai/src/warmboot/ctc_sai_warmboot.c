
#include "sal.h"
#include "ctc_warmboot.h"
#include "ctc_sai_warmboot.h"
#include "ctc_sai_debug.h"

extern int32
dbclient_lpush_binary(char *key, uint8 *value, uint32 value_len);

extern int32
dbclient_lrange_binary(char *key, int32 start, int32 end, uint8 *out_buf, uint32 *buf_len, uint32 *count);

#define DB_SIZE_32      32
#define DB_SIZE_20K     20480

/* refer ctc_app_wb_init */
#ifdef USW
int32 
ctc_sai_wb_init(uint8 lchip, uint8 status)
{
    int32 rc = 0;
    if (status == CTC_WB_STATUS_RELOADING)
    {
        log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_init()");
    }

    return rc;
}
#else
int32 
ctc_sai_wb_init(uint8 status)
{
    int32 rc = 0;
    if (status == CTC_WB_STATUS_RELOADING)
    {
        log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_init()");
    }

    return rc;
}
#endif

/* refer ctc_app_wb_init_done */
#ifdef USW
int32
ctc_sai_wb_init_done(uint8 lchip)
{
    int32 rc = 0;
    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_init_done()");
        
    return rc;
}
#else
int32
ctc_sai_wb_init_done(void)
{
    int32 rc = 0;
    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_init_done()");
        
    return rc;
}
#endif

/* refer ctc_app_wb_sync */
#ifdef USW
int32
ctc_sai_wb_sync(uint8 lchip)
{
    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_sync()");

    return 0;
}
#else
int32
ctc_sai_wb_sync(void)
{
    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_sync()");

    return 0;
}
#endif


/* refer ctc_app_wb_sync_done */
#ifdef USW
int32
ctc_sai_wb_sync_done(uint8 lchip, int32 result)
{
    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_sync_done()");

    return 0;
}
#else
int32
ctc_sai_wb_sync_done(int32 result)
{
    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_sync_done()");

    return 0;
}
#endif

/* refer ctc_app_wb_add_entry */
int32
ctc_sai_wb_add_entry(ctc_wb_data_t *data)
{
    char key[DB_SIZE_32];
    uint32 offset = 0;
    uint8 buffer[DB_SIZE_20K];
    uint32 len = 0;
    uint32 i = 0;

    if (DB_SIZE_20K < data->key_len + data->data_len)
    {
        log_sys(M_MOD_SAI, E_ERROR, "SDK wb data %u is too large", (data->key_len + data->data_len));
        return -1;
    }
    
    sal_snprintf(key, DB_SIZE_32, "SDK_%u", data->app_id);
    len = data->key_len + data->data_len;
    for (i = 0; i < data->valid_cnt; i++)
    {
        sal_memcpy(buffer, ((uint8*)data->buffer) + offset, data->key_len);
        offset += data->key_len;
        sal_memcpy((buffer+data->key_len), ((uint8*)data->buffer) + offset, data->data_len);
        offset += data->data_len;
        dbclient_lpush_binary(key, buffer, len);
    }
    
    return 0;
}

/* refer ctc_app_wb_query_entry */
int32
ctc_sai_wb_query_entry(ctc_wb_query_t *query)
{
    char key[DB_SIZE_32];
    int32 rc = 0;
    uint32 query_cnt = 0;
    uint32 return_cnt = 0;
    uint32 buffer_len = 0;
    
    query->valid_cnt = 0;
    query->is_end = 1;
    buffer_len = query->buffer_len;

    if ((0 == query->key_len) || (0 == query->buffer_len))
    {
        return -1;
    }

    /* only support query all */
    if (0 != query->query_type)
    {
        return -1;
    }

    sal_snprintf(key, DB_SIZE_32, "SDK_%u", query->app_id);
    query_cnt = query->buffer_len / (query->key_len + query->data_len);

    rc = dbclient_lrange_binary(key, query->cursor, (query->cursor + query_cnt -1), (uint8*)query->buffer, &buffer_len, &return_cnt);
    query->valid_cnt = return_cnt;
    if (query_cnt > return_cnt)
    {
        query->cursor = 0;
        query->is_end = 1;
    }
    else
    {
        query->cursor += return_cnt;
        query->is_end = 0;
    }

    log_sys(M_MOD_SAI, E_ERROR, "ctc_sai_wb_query_entry query %s cursor %u, return_cnt %u, is_end %u", key, query->cursor, return_cnt, query->is_end);

    return rc;
}

uint32 wb_en = FALSE;
uint32 wb_restoring = FALSE;
uint32 wb_restoring_execute_count = 0;

int32
ctc_sai_set_wb_en(uint32 en)
{
    wb_en = en;
    return 0;
}

int32
ctc_sai_set_wb_restoring(uint32 restoring)
{
    wb_restoring = restoring;
    return 0;
}

uint32
ctc_sai_get_wb_restoring()
{
    return wb_restoring;
}

int32
ctc_sai_warmboot_init()
{
    ctc_wb_api_t wb_api;
    int32 rc = 0;
    #ifdef USW
    uint8 lchip = 0;
    #endif
    
    sal_memset(&wb_api, 0, sizeof(wb_api));

    if (wb_en)
    {
        wb_api.enable = TRUE;
        wb_api.reloading = wb_restoring;
        /* for after reloading, restore data from DB */
        wb_api.init = &ctc_sai_wb_init;
        wb_api.init_done = &ctc_sai_wb_init_done;
        
        /* for before reloading, sync data to DB */
        wb_api.sync = &ctc_sai_wb_sync;
        wb_api.sync_done = &ctc_sai_wb_sync_done;
        wb_api.add_entry = &ctc_sai_wb_add_entry;
        
        /* for after reloading, restore data from DB */
        wb_api.query_entry = &ctc_sai_wb_query_entry;
    }
    #ifdef USW
    rc = ctc_wb_init(lchip, &wb_api);
    #else
    rc = ctc_wb_init(&wb_api);
    #endif
    if (rc != 0)
    {
        return rc;
    }

    return rc;
}

