#include "sal.h"
#include "ctc_warmboot.h"

#include "ctc_register.h"
#include "ctcs_api.h"
#include "ctc_api.h"

extern uint8 g_lchip_num;
ctc_wb_master_t *g_wb_master;

#define CTC_WB_CHIPID(table_id)       (uint8)(table_id >> 16)
#define CTC_WB_MODID(table_id)        (uint8)(table_id>> 8)
#define CTC_WB_SUBID(table_id)         table_id & 0xFF

int32 ctc_wb_init(uint8 lchip, ctc_wb_api_t *wb_api)
{
    int32 ret = CTC_E_NONE;
    uint8 status = 0;

    lchip = g_ctcs_api_en ? lchip : 0;

    if (!wb_api
	 || !wb_api->init
	 || !wb_api->init_done
	 || !wb_api->sync
	 || !wb_api->add_entry
	 || !wb_api->sync_done
	 || !wb_api->query_entry)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "warmboot is not support!\n");
        return CTC_E_NONE;
    }

    if (!g_wb_master)
    {
        g_wb_master = sal_malloc(sizeof(ctc_wb_master_t));
        g_wb_master->init = wb_api->init;
        g_wb_master->init_done = wb_api->init_done;
        g_wb_master->sync = wb_api->sync;
        g_wb_master->sync_done = wb_api->sync_done;
        g_wb_master->add_entry = wb_api->add_entry;
        g_wb_master->query_entry = wb_api->query_entry;
        g_wb_master->enable  = wb_api->enable;
    }

    ret = g_wb_master->init(lchip, wb_api->reloading);
    if( wb_api->enable && wb_api->reloading && ret == CTC_E_NONE)
    { /*warmboot*/
        status = CTC_WB_STATUS_RELOADING;   /*reloading*/
    }
    else
    {
        status = CTC_WB_STATUS_DONE;  /*done*/
    }

    if (g_ctcs_api_en)
    {
        g_wb_master->wb_status[lchip] = status;
    }
    else
    {
        for (lchip = 0; lchip < CTC_MAX_LOCAL_CHIP_NUM; lchip++)
        {
            g_wb_master->wb_status[lchip] = status;
        }
    }

    if (ret != CTC_E_NONE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init error, ret = %d\n", ret);
    }

    return CTC_E_NONE;
 }

int32 ctc_wb_init_done(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 status = CTC_WB_STATUS_DONE;

    lchip = g_ctcs_api_en ? lchip : 0;

    if (!CTC_WB_ENABLE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "warmboot is not enable!\n");
        return CTC_E_NOT_READY;
    }

    if (g_wb_master->wb_status[lchip] == CTC_WB_STATUS_RELOADING)
    {
        ret = g_wb_master->init_done(lchip);
        if (ret != CTC_E_NONE)
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_wb_init_done error, ret = %d\n", ret);
            return CTC_E_INVALID_PARAM;
        }

        if (g_ctcs_api_en)
        {
            CTC_ERROR_RETURN(ctcs_global_ctl_set(lchip, CTC_GLOBAL_WARMBOOT_STATUS, &status));
            g_wb_master->wb_status[lchip] = status;/*done*/
        }
        else
        {
            CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_WARMBOOT_STATUS, &status));
            for (lchip = 0; lchip < CTC_MAX_LOCAL_CHIP_NUM; lchip++)
            {
                g_wb_master->wb_status[lchip] = status;/*done*/
            }
        }
    }

    return CTC_E_NONE;
}

int32 ctc_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 status = CTC_WB_STATUS_SYNC;

    lchip = g_ctcs_api_en ? lchip : 0;

    if (!CTC_WB_ENABLE)
    {
        return CTC_E_NOT_READY;
    }

    ret = g_wb_master->sync(lchip);
    if (ret != CTC_E_NONE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ctc_wb_sync error, ret = %d\n", ret);
        return CTC_E_INVALID_PARAM;
    }

    if (g_ctcs_api_en)
    {
        CTC_ERROR_RETURN(ctcs_global_ctl_set(lchip, CTC_GLOBAL_WARMBOOT_STATUS, &status));
        g_wb_master->wb_status[lchip] = status;/*sync*/
    }
    else
    {
        CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_WARMBOOT_STATUS, &status));
        for (lchip = 0; lchip < g_lchip_num; lchip++)
        {
            g_wb_master->wb_status[lchip] = status;/*sync*/
        }
    }

    return CTC_E_NONE;
}

int32 ctc_wb_sync_done(uint8 lchip, int32 result)
{
    int32 ret = CTC_E_NONE;
    uint32 status = CTC_WB_STATUS_DONE;

    lchip = g_ctcs_api_en ? lchip : 0;

    if (!CTC_WB_ENABLE)
    {
        return CTC_E_NOT_READY;
    }

    ret = g_wb_master->sync_done(lchip, result);
    if (ret != CTC_E_NONE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ctc_wb_sync_done error\n");
    }

    if (g_ctcs_api_en)
    {
        g_wb_master->wb_status[lchip] = status;/*done*/
        CTC_ERROR_RETURN(ctcs_global_ctl_set(lchip, CTC_GLOBAL_WARMBOOT_STATUS, &status));
    }
    else
    {
        for (lchip = 0; lchip < g_lchip_num; lchip++)
        {
            g_wb_master->wb_status[lchip] = status;/*done*/
        }
        CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_WARMBOOT_STATUS, &status));
    }

    return ret;
}

int32 ctc_wb_add_entry(ctc_wb_data_t *data)
{
    int32 ret;

    if (!CTC_WB_ENABLE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "warmboot is not enable!\n");
        return CTC_E_NOT_READY;
    }

    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add %4d entry to mod_id: %2d sub_id:%2d (appid: %4d) table on chip %u\n",
        data->valid_cnt, CTC_WB_MODID(data->app_id), CTC_WB_SUBID(data->app_id),
        data->app_id, CTC_WB_CHIPID(data->app_id));

    ret = g_wb_master->add_entry ? g_wb_master->add_entry(data) : CTC_E_NOT_SUPPORT;
    if (ret != CTC_E_NONE && ret != CTC_E_NOT_SUPPORT)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ctc_wb_add_entry error\n");
        ret = CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32 ctc_wb_query_entry(ctc_wb_query_t *query)
{
    int32 ret;

    if (!CTC_WB_ENABLE)
    {
        return CTC_E_NONE;
    }

    ret = g_wb_master->query_entry ? g_wb_master->query_entry(query) : CTC_E_NOT_SUPPORT;

    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Query mod_id: %2d sub_id:%2d (appid: %4d) table on chip %u, entry count: %4d, cursor: %6d, end: %d\n",
            CTC_WB_MODID(query->app_id), CTC_WB_SUBID(query->app_id), query->app_id,
            CTC_WB_CHIPID(query->app_id), query->valid_cnt, query->cursor, query->is_end);

    if (ret != CTC_E_NONE && ret != CTC_E_NOT_SUPPORT)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ctc_wb_query_entry error, ret\n");
        ret = CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32 ctc_wb_set_cpu_rx_en(uint8 lchip, bool enable)
{
    bool value = FALSE;

    if (!CTC_WB_ENABLE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "warmboot is not enable!\n");
        return CTC_E_NOT_READY;
    }

    value = enable;
    if (g_ctcs_api_en)
    {
        CTC_ERROR_RETURN(ctcs_global_ctl_set(lchip, CTC_GLOBAL_WARMBOOT_CPU_RX_EN, &value));
    }
    else
    {
        CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_WARMBOOT_CPU_RX_EN, &value));
    }

    return CTC_E_NONE;
}

int32 ctc_wb_get_cpu_rx_en(uint8 lchip, bool* enable)
{
    uint32 value = FALSE;
    CTC_PTR_VALID_CHECK(enable);

    if (!CTC_WB_ENABLE)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "warmboot is not enable!\n");
        return CTC_E_NOT_READY;
    }

    if (g_ctcs_api_en)
    {
        CTC_ERROR_RETURN(ctcs_global_ctl_get(lchip, CTC_GLOBAL_WARMBOOT_CPU_RX_EN, &value));
    }
    else
    {
        CTC_ERROR_RETURN(ctc_global_ctl_get(CTC_GLOBAL_WARMBOOT_CPU_RX_EN, &value));
    }
    *enable = value ? TRUE : FALSE;

    return CTC_E_NONE;
}

int32 ctc_wb_deinit(uint8 lchip)
{
    uint8 is_free = 1;

    if (!g_wb_master)
    {
        return CTC_E_NONE;
    }

    if (g_ctcs_api_en)
    {
        if (g_wb_master->wb_status[lchip])
        {
            g_wb_master->wb_status[lchip] = 0;
        }

        for (lchip = 0; lchip < g_lchip_num; lchip++)
        {
            if (g_wb_master->wb_status[lchip])
            {
                is_free = 0;
            }
        }
    }

    if (is_free)
    {
        sal_free(g_wb_master);
        g_wb_master = NULL;
    }

    return CTC_E_NONE;
}
