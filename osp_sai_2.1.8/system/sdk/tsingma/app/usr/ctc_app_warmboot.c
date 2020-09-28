#include "sal.h"
#include "api/include/ctc_api.h"
#include "ctc_app.h"
#include "ctc_app_warmboot.h"

#ifdef SDK_IN_USERMODE
#include "sqlite3.h"

sqlite3 *g_db = NULL;
char *warmboot_db_path = "warmboot.db";

uint8 g_wb_status[CTC_MAX_LOCAL_CHIP_NUM] = {0};

enum ctc_app_error_code
{
    CTC_APP_OK = 0,
    CTC_APP_TABLE_NOT_EXIST = 0,
    CTC_APP_DB_SYNC_FAILED = -100,
    CTC_APP_DB_CLOSE_UNEXPECTED,
    CTC_APP_DB_QUERY_TYPE_UNEXPECTED,
    CTC_APP_COUNT_CHECK_ERROR,
    CTC_APP_LENGTH_CHECK_ERROR
};

#define CHECK_FIRST_WB_STATUS(ret, status)     \
    do                                                     \
    {       \
        ret = 1;    \
        uint8 lchip = 0;\
        for (lchip = 0; lchip < CTC_MAX_LOCAL_CHIP_NUM; lchip++)    \
        {   \
            if (g_wb_status[lchip] == status)   \
            {   \
                ret = 0;   \
                break;  \
            }   \
        }  \
    } while (0)

#define CHECK_LAST_WB_STATUS(ret, status)     \
    do                                                     \
    {       \
        ret = 1;    \
        uint8 lchip = 0, count = 0;\
        for (lchip = 0; lchip < CTC_MAX_LOCAL_CHIP_NUM; lchip++)    \
        {   \
            if (g_wb_status[lchip] == status)   \
            {   \
                if (++count > 1)    \
                {   \
                    ret = 0;    \
                    break;  \
                }   \
            }   \
        }  \
    } while (0)

int32 _ctc_app_wb_sort_store()
{
    uint16 loop = 0;
    uint16 table_cnt = 0;
    char sql[256];
    int32 ret = CTC_APP_OK;
    uint16 table_name[128];
    sqlite3_stmt *stmt = NULL;

    if (g_db == NULL)
    {
        return CTC_APP_DB_CLOSE_UNEXPECTED;
    }

    sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    ret = sqlite3_prepare_v2(g_db, "select name from sqlite_master where type = 'table';", -1, &stmt, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "BEGIN TRANSACTION error! error code: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));

        goto done;
    }

    while ( sqlite3_step(stmt) == SQLITE_ROW )
    {
        table_name[table_cnt++] = sqlite3_column_int(stmt, 0);
    }

    for(loop = 0; loop < table_cnt; loop++)
    {
        sal_sprintf(sql, "CREATE TABLE '%d_sort' AS select * from '%d' order by key ASC;", table_name[loop], table_name[loop]);
        ret = sqlite3_exec(g_db, sql, NULL, NULL, NULL);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Create table_sort %d error ret: %d! error message: %s\n", table_name[loop], ret, sqlite3_errmsg(g_db));

            goto done;
        }
    }

    ret = sqlite3_exec(g_db, "COMMIT TRANSACTION;", NULL, NULL, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "COMMIT TRANSACTION error! error code: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));

        goto done;
    }

    for(loop = 0; loop < table_cnt; loop++)
    {
        sal_sprintf(sql, "DROP TABLE '%d';", table_name[loop]);
        ret = sqlite3_exec(g_db, sql, NULL, NULL, NULL);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Create table_sort %d error ret: %d! error message: %s\n", table_name[loop], ret, sqlite3_errmsg(g_db));

            goto done;
        }

        sal_sprintf(sql, "ALTER TABLE '%d_sort' RENAME to '%d';", table_name[loop], table_name[loop]);
        ret = sqlite3_exec(g_db, sql, NULL, NULL, NULL);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ALTER table %d error ret: %d! error message: %s\n", table_name[loop], ret, sqlite3_errmsg(g_db));

            goto done;
        }
    }

    ret = sqlite3_exec(g_db, "vacuum;", NULL, NULL, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Vacuum data error! error message: %s\n", sqlite3_errmsg(g_db));

        goto done;
    }

done:
    if ( stmt )
    {
        sqlite3_finalize( stmt );
    }

    return ret;
}

int32 ctc_app_wb_load(uint8 load_flag)
{
    int32 ret = CTC_APP_OK;
    uint32 flags = 0;
    sqlite3 *pFile;
    sqlite3 *pTo;
    sqlite3 *pFrom;
    sqlite3_backup *pBackup;

    if (g_db == NULL)
    {
        return CTC_APP_DB_CLOSE_UNEXPECTED;
    }

    if (load_flag == LOAD_FILE_TO_MEMORY)
    {
        flags = SQLITE_OPEN_READONLY;
    }
    else if (load_flag == LOAD_MEMORY_TO_FILE)
    {
        flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
    }

    ret = sqlite3_open_v2(warmboot_db_path, &pFile, flags, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Can't open database: %s\n", sqlite3_errmsg(pFile));
        sqlite3_close(pFile);

        return (ret > 0) ? -ret : ret;
    }

    if (load_flag == LOAD_FILE_TO_MEMORY)
    {
        pFrom = pFile;
        pTo = g_db;
    }
    else if (load_flag == LOAD_MEMORY_TO_FILE)
    {
        pFrom = g_db;
        pTo = pFile;
    }
    else
    {
        sqlite3_close(pFile);

        return CTC_E_INVALID_PARAM;
    }

    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if( pBackup ){
        sqlite3_backup_step(pBackup, -1);
        sqlite3_backup_finish(pBackup);
    }
    ret = sqlite3_errcode(pTo);

    sqlite3_close(pFile);

    return (ret > 0) ? -ret : ret;
}

int32 ctc_app_wb_init(uint8 lchip, uint8 reloading)
{
    int32 ret = CTC_APP_OK;
    uint8 is_frist = 0;

    if ( reloading )
    {
        CHECK_FIRST_WB_STATUS(is_frist, CTC_WB_STATUS_RELOADING);
        if (!is_frist)
        {
            return CTC_APP_OK;
        }

        ret = sqlite3_open(":memory:", &g_db);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Can't open database: %s\n", sqlite3_errmsg(g_db));
            sqlite3_close(g_db);
            g_db = NULL;

            return (ret > 0) ? -ret : ret;
        }

        ret = ctc_app_wb_load(LOAD_FILE_TO_MEMORY);
        if ( ret != CTC_E_NONE )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ctc_app_wb_load error, ret: %d\n", ret);
            sqlite3_close(g_db);
            g_db = NULL;

            return (ret > 0) ? -ret : ret;
        }
        g_wb_status[lchip] = CTC_WB_STATUS_RELOADING;
    }

    return ret;
}

int32 ctc_app_wb_init_done(uint8 lchip)
{
    uint8 is_last = 0;

    CHECK_LAST_WB_STATUS(is_last, CTC_WB_STATUS_RELOADING);
    if (!is_last)
    {
        return CTC_APP_OK;
    }

    if (g_db)
    {
        sqlite3_close(g_db);
        g_db = NULL;
    }
    g_wb_status[lchip] = CTC_WB_STATUS_DONE;

    return CTC_APP_OK;
}

int32 ctc_app_wb_sync(uint8 lchip)
{
    int32 ret = CTC_APP_OK;
    uint8 is_frist = 0;

    CHECK_FIRST_WB_STATUS(is_frist, CTC_WB_STATUS_SYNC);
    if (!is_frist)
    {
        return CTC_APP_OK;
    }

    if ( g_db )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Database is not closed!!\n");

        return CTC_APP_OK;
    }

    ret = sqlite3_open(":memory:", &g_db);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Can't open database: %s\n", sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        g_db = NULL;

        return (ret > 0) ? -ret : ret;
    }
    g_wb_status[lchip] = CTC_WB_STATUS_SYNC;

    return CTC_APP_OK;
}

int32 ctc_app_wb_sync_done(uint8 lchip, int32 result)
{
    int32 ret = CTC_APP_OK;
    uint8 is_last = 0;

    if ( !g_db )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "database close unexpect!\n");

        return CTC_APP_DB_CLOSE_UNEXPECTED;
    }

    CHECK_LAST_WB_STATUS(is_last, CTC_WB_STATUS_SYNC);
    if (!is_last)
    {
        return CTC_APP_OK;
    }

    if (result != CTC_APP_OK)
    {
        if ( g_db )
        {
            sqlite3_close(g_db);
            g_db = NULL;
        }

        return CTC_APP_DB_SYNC_FAILED;
    }

     /*-_ctc_app_wb_sort_store();*/

    ret = ctc_app_wb_load(LOAD_MEMORY_TO_FILE);

    if ( g_db )
    {
        sqlite3_close(g_db);
        g_db = NULL;
    }
    g_wb_status[lchip] = CTC_WB_STATUS_DONE;

    return (ret > 0) ? -ret : ret;
}

int32 ctc_app_wb_add_entry(ctc_wb_data_t *data)
{
    uint32 loop = 0;
    int32 ret = CTC_APP_OK;
    uint32 exist_flag = 0;
    int insert_cnt = 0;
    char sql[256];
    uint32 buffer_pos = 0;
    sqlite3_stmt *stmt1 = NULL, *stmt2 = NULL;

    if (!g_db)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Database is closed unexpected!\n");
        return CTC_APP_DB_CLOSE_UNEXPECTED;
    }

    if ( data->buffer_len < (data->key_len + data->data_len) * data->valid_cnt )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Buffer length error! key length: %d, data length: %d, valid_cnt: %d, buffer_len: %d\n",
            data->key_len, data->data_len, data->valid_cnt, data->buffer_len);

        ret = CTC_APP_LENGTH_CHECK_ERROR;
        goto done;
    }

    ret = sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "BEGIN TRANSACTION error! error code: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));

        goto done;
    }

    sal_sprintf(sql, "select count(*) from sqlite_master where type='table' and name='%d';", data->app_id);
    ret = sqlite3_prepare_v2(g_db, sql, -1, &stmt1, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Get table %d count error! error code: %d, error message: %s\n", data->app_id, ret, sqlite3_errmsg(g_db));

        goto done;
    }

    if ( (ret = sqlite3_step(stmt1)) == SQLITE_ROW )
    {
        exist_flag = sqlite3_column_int(stmt1, 0);
        if ( exist_flag == 0 )
        {
            sal_sprintf(sql, "CREATE TABLE '%d'(key BLOB PRIMARY KEY NOT NULL, key_len int, data BLOB, data_len int);", data->app_id);
             /*CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "create table %d\n", data->app_id);*/
            ret = sqlite3_exec(g_db, sql, NULL, NULL, NULL);
            if ( ret != SQLITE_OK )
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Create table %d error ret: %d, error message: %s\n", data->app_id, ret, sqlite3_errmsg(g_db));

                goto done;
            }
        }
    }
    else
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sqlite3_step(select) error ret: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));

        goto done;
    }

    sal_sprintf(sql, "INSERT INTO '%d' VALUES(?, %d, ?, %d);", data->app_id, data->key_len, data->data_len);
    sqlite3_prepare_v2(g_db, sql, -1, &stmt2, NULL);

    for( loop = 0; loop < data->valid_cnt; loop++)
    {
        sqlite3_bind_blob(stmt2, 1, (uint8 *)data->buffer + buffer_pos, data->key_len, NULL);
        buffer_pos += data->key_len;
        sqlite3_bind_blob(stmt2, 2, (uint8 *)data->buffer + buffer_pos, data->data_len, NULL);
        buffer_pos += data->data_len;

        ret = sqlite3_step(stmt2);
        if ( ret != SQLITE_DONE )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sqlite3_step(add) error ret: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));

            sqlite3_reset(stmt2);

            continue;
        }
        insert_cnt++;

        sqlite3_reset(stmt2);
    }

     /*CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "+++insert %d entry to table %d+++\n", insert_cnt, data->app_id);*/
    ret = sqlite3_exec(g_db, "COMMIT TRANSACTION;", NULL, NULL, NULL);
    if ( ret != SQLITE_OK )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "COMMIT TRANSACTION error! error code: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));

        goto done;
    }

done:
    if ( stmt1 )
    {
        sqlite3_finalize( stmt1 );
    }

    if ( stmt2 )
    {
        sqlite3_finalize( stmt2 );
    }

    return (ret > 0) ? -ret : ret;
}

int32 ctc_app_wb_query_entry(ctc_wb_query_t *query)
{
    int ret = CTC_APP_OK;
    uint16 key_len = 0;
    uint16 data_len = 0;
    uint32 query_cnt = 0;
    uint32 table_cnt = 0;
    uint32 select_cnt = 0;
    uint32 buffer_pos = 0;
    uint8 table_exist = 0;
    uint8 check_flag = 0;
    char sql[256];
    sqlite3_stmt *stmt1 = NULL, *stmt2 = NULL;

    if (!g_db)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Database is closed unexpected!\n");
        return CTC_APP_DB_CLOSE_UNEXPECTED;
    }

    query->valid_cnt = 0;
    query->is_end = 1;

    if ( query->key_len == 0 || query->buffer_len == 0 )
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Length error! key len: %d, data len: %d, buffer len: %d\n",
            query->key_len, query->data_len, query->buffer_len);

        ret = CTC_APP_LENGTH_CHECK_ERROR;
        goto done;
    }

    query_cnt = query->buffer_len / (query->key_len + query->data_len);

    if ( query->query_type == 0 )
    {
        sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        sal_sprintf(sql, "select count(*) from sqlite_master where type='table' and name='%d';", query->app_id);
        ret = sqlite3_prepare_v2(g_db, sql, -1, &stmt1, NULL);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Get table %d error! error message: %s\n", query->app_id, sqlite3_errmsg(g_db));

            ret = CTC_APP_OK;
            goto done;
        }

        if ( sqlite3_step(stmt1) == SQLITE_ROW )
        {
            table_exist = sqlite3_column_int(stmt1, 0);
            if (table_exist == 0)
            {
                ret = CTC_APP_TABLE_NOT_EXIST;
                goto done;
            }
        }

        sal_sprintf(sql, "select count(*) from '%d';", query->app_id);
        ret = sqlite3_prepare_v2(g_db, sql, -1, &stmt1, NULL);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Get table %d count error! error message: %s\n", query->app_id, sqlite3_errmsg(g_db));

            goto done;
        }

        if ( sqlite3_step(stmt1) == SQLITE_ROW )
        {
            table_cnt = sqlite3_column_int(stmt1, 0);
        }

        sal_sprintf(sql, "select * from '%d' limit %d offset %d;", query->app_id, query_cnt, query->cursor);
        ret = sqlite3_prepare_v2(g_db, sql, -1, &stmt2, NULL);
        if ( ret != SQLITE_OK )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sqlite3 prepare error ret: %d, error message: %s!\n", ret, sqlite3_errmsg(g_db));

            goto done;
        }

        while ((ret = sqlite3_step( stmt2 )) == SQLITE_ROW)
        {
            if ( check_flag == 0 )
            {
                key_len = sqlite3_column_int(stmt2, 1);
                data_len = sqlite3_column_int(stmt2, 3);

                if ( key_len != query->key_len )
                {
                    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Key length error! query key_len: %d, data_len: %d; db key_len: %d, data_len: %d\n",
                        query->key_len, query->data_len, key_len, data_len);

                    ret = CTC_APP_LENGTH_CHECK_ERROR;
                    goto done;
                }

                query->data_len = data_len;
                check_flag = 1;
            }

            if ( buffer_pos + key_len + data_len > query->buffer_len )
            {
                break;
            }

            sal_memcpy((uint8 *)query->buffer + buffer_pos, (uint8 *)sqlite3_column_blob(stmt2, 0), key_len);
            buffer_pos += key_len;
            sal_memcpy((uint8 *)query->buffer + buffer_pos, (uint8 *)sqlite3_column_blob(stmt2, 2), data_len);
            buffer_pos += data_len;

            select_cnt++;
        }
        sqlite3_exec(g_db, "COMMIT TRANSACTION;", NULL, NULL, NULL);

        if ( query->cursor + select_cnt > table_cnt )
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Select count error! table_count: %d, select_count: %d\n", table_cnt, select_cnt);

            ret = CTC_APP_LENGTH_CHECK_ERROR;
            goto done;
        }
        else
        {
            query->valid_cnt = select_cnt;
            if (query->cursor + select_cnt == table_cnt)
            {
                query->cursor = 0;
                query->is_end = 1;
            }
            else
            {
                query->cursor += select_cnt;
                query->is_end = 0;
            }
        }

        ret = CTC_APP_OK;
        goto done;
    }
    else if ( query->query_type == 1 )
    {
        sal_sprintf(sql, "select * from '%d' where key = ?;", query->app_id);
        sqlite3_prepare_v2(g_db, sql, -1, &stmt1, NULL);
        sqlite3_bind_blob(stmt1, 1, query->key, query->key_len, NULL);

        if ( (ret = sqlite3_step(stmt1)) == SQLITE_ROW )
        {
            key_len = sqlite3_column_int(stmt1, 1);
            data_len = sqlite3_column_int(stmt1, 3);

            if ( key_len != query->key_len )
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Key length error!query key_len: %d, data_len: %d; db key_len: %d, data_len: %d\n",    \
                    query->key_len, query->data_len, key_len, data_len);

                ret = CTC_APP_LENGTH_CHECK_ERROR;
                goto done;
            }

            sal_memcpy(query->buffer, (uint8 *)sqlite3_column_blob(stmt1, 0), key_len);
            buffer_pos += key_len;
            sal_memcpy((uint8 *)query->buffer + buffer_pos, (uint8 *)sqlite3_column_blob(stmt1, 2), data_len);
            buffer_pos += data_len;

            query->cursor = 0;
            query->valid_cnt = 1;
            query->is_end = 1;

            ret = CTC_E_NONE;
            goto done;
        }
        else
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sqlite3_step(select) error ret: %d, error message: %s\n", ret, sqlite3_errmsg(g_db));
            query->cursor = 0;
            query->valid_cnt = 0;
            query->is_end = 1;

            goto done;
        }
    }
    else
    {
        query->valid_cnt = 0;
        query->is_end = 1;
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Query type error, type: %d\n", query->query_type);

        ret = CTC_APP_DB_QUERY_TYPE_UNEXPECTED;
        goto done;
    }

done:
    if ( stmt1 )
    {
        sqlite3_finalize(stmt1);
    }

    if ( stmt2 )
    {
        sqlite3_finalize(stmt2);
    }

    return (ret > 0) ? -ret : ret;
}

#endif
