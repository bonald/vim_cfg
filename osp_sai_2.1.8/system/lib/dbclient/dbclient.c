
#include "sal.h"
#include "ctclib_memory.h"
#include "hiredis.h"
#include "dbclient.h"
#include "genlog.h"

typedef struct
{
    char                    address[64];
    uint32                  port;
    uint32                  count;
    redisContext           *p_context;
} ctc_dbclient_master_t;

ctc_dbclient_master_t *g_db_master = NULL;

#define CONFIG_DBCLIENT

#define DB_SIZE_32      32
#define DB_SIZE_64      64
#define DB_SIZE_256     256
#define DB_SIZE_1024    1024
#define DB_SIZE_4096    4096

#define DB_ARGC_2   2
#define DB_ARGC_3   3
#define DB_ARGC_4   4

uint32 DB_PORT = 6379;
char* DB_HOSTNAME = "127.0.0.1";

int32
_dbclient_connect(char *address, uint32 port)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    p_master->p_context = redisConnect(address, port);

    sal_strncpy(p_master->address, address, 64);
    if (NULL == p_master->p_context)
    {
        return -1;
    }
#endif
    return 0;
}

int32
_dbclient_select()
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    char cmd[256];
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    sal_snprintf(cmd, 256, "SELECT 1");
    redisAppendCommand(p_master->p_context, cmd);
    redisGetReply(p_master->p_context, (void **)&reply);
    freeReplyObject(reply);

    return rc;
#else
    return 0;
#endif
}

int32
dbclient_init()
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = NULL;
    int32 rc = 0;
    
    p_master = XCALLOC(MEM_TEMP, sizeof(ctc_dbclient_master_t));
    if (NULL == p_master)
    {
        return -1;
    }
    g_db_master = p_master;

    rc = _dbclient_connect(DB_HOSTNAME, DB_PORT);
    if (rc < 0)
    {
        return -1;
    }

    _dbclient_select();
#endif
    return 0;
}

int32
dbclient_deinit()
{
#ifdef CONFIG_DBCLIENT
    if (g_db_master)
    {
        if (g_db_master->p_context)
        {
            redisFree(g_db_master->p_context);
        }
        XFREE(MEM_TEMP, g_db_master);
        g_db_master = NULL;
    }
#endif
    return 0;
}

int32
dbclient_flushdb()
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    char cmd[256];
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    sal_snprintf(cmd, 256, "FLUSHDB");
    redisAppendCommand(p_master->p_context, cmd);
    redisGetReply(p_master->p_context, (void **)&reply);
    freeReplyObject(reply);

    return rc;
#else
    return 0;
#endif
}

int32
dbclient_save()
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    char cmd[256];
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    sal_snprintf(cmd, 256, "SAVE");
    redisAppendCommand(p_master->p_context, cmd);
    redisGetReply(p_master->p_context, (void **)&reply);
    freeReplyObject(reply);

    return rc;
#else
    return 0;
#endif
}

int32
dbclient_hdel(char *key, char *field)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    char cmd[256];
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    sal_snprintf(cmd, 256, "HDEL %s %s", key, field);
    redisAppendCommand(p_master->p_context, cmd);
    redisGetReply(p_master->p_context, (void **)&reply);
    freeReplyObject(reply);

    return rc;
#else
    return 0;
#endif
}

int32
dbclient_hset(char *key, char *field, char *value)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    char cmd[256];
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }

    sal_snprintf(cmd, 256, "HSET %s %s %s", key, field, value);
    redisAppendCommand(p_master->p_context, cmd);
    redisGetReply(p_master->p_context, (void **)&reply);
    freeReplyObject(reply);

    return rc;
#else
    return 0;
#endif
}

int32
dbclient_hdel_binary(char *key, char *field)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    const char* argv[DB_ARGC_3];
    size_t argvlen[DB_ARGC_3];

    if (NULL == p_master)
    {
        return -1;
    }
    
    argv[0] = "HDEL";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = sal_strlen(key);
    argv[2] = field;
    argvlen[2] = sal_strlen(field);
    
    reply =(redisReply*)redisCommandArgv(p_master->p_context, DB_ARGC_3, argv, argvlen);
    if (NULL == reply)
    {
        return -1;
    }
    
    freeReplyObject(reply);
#endif
    return 0;
}

int32
dbclient_hset_binary(char *key, char *field, char *value)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    const char* argv[DB_ARGC_4];
    size_t argvlen[DB_ARGC_4];

    if (NULL == p_master)
    {
        return -1;
    }
    
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = sal_strlen(key);
    argv[2] = field;
    argvlen[2] = sal_strlen(field);
    argv[3] = value;
    argvlen[3] = sal_strlen(value);
    
    reply =(redisReply*)redisCommandArgv(p_master->p_context, DB_ARGC_4, argv, argvlen);
    if (NULL == reply)
    {
        return -1;
    }
    
    freeReplyObject(reply);
#endif
    return 0;
}

int32
dbclient_lpush_binary(char *key, uint8 *value, uint32 value_len)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    const char* argv[DB_ARGC_3];
    size_t argvlen[DB_ARGC_3];

    if (NULL == p_master)
    {
        return -1;
    }
    
    argv[0] = "LPUSH";
    argvlen[0] = 5;
    argv[1] = key;
    argvlen[1] = sal_strlen(key);
    argv[2] = (char*)value;
    argvlen[2] = value_len;
    
    reply =(redisReply*)redisCommandArgv(p_master->p_context, DB_ARGC_3, argv, argvlen);
    if (NULL == reply)
    {
        return -1;
    }
    
    freeReplyObject(reply);
#endif
    return 0;
}

int32
dbclient_lpop_binary(char *key)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    const char* argv[DB_ARGC_2];
    size_t argvlen[DB_ARGC_2];

    if (NULL == p_master)
    {
        return -1;
    }
    
    argv[0] = "LPOP";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = sal_strlen(key);
    
    reply =(redisReply*)redisCommandArgv(p_master->p_context, DB_ARGC_2, argv, argvlen);
    if (NULL == reply)
    {
        return -1;
    }
    
    freeReplyObject(reply);
#endif
    return 0;
}

int32
dbclient_lrange_binary(char *key, int32 start, int32 end, uint8 *out_buf, uint32 *buf_len, uint32 *count)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    redisReply *element = NULL;
    const char* argv[DB_ARGC_4];
    size_t argvlen[DB_ARGC_4];
    uint32 i = 0;
    char start_str[DB_SIZE_32];
    char end_str[DB_SIZE_32];
    uint32 buffer_len = 0;
    uint32 buffer_pos = 0;
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    if ((NULL == out_buf) || (NULL == buf_len))
    {
        return -1;
    }

    if (0 == buf_len)
    {
        return -1;
    }
    buffer_len = *buf_len;

    *count = 0;
    *buf_len = 0;
    
    sal_snprintf(start_str, DB_SIZE_32, "%d", start);
    sal_snprintf(end_str, DB_SIZE_32, "%d", end);
    
    argv[0] = "LRANGE";
    argvlen[0] = 6;
    argv[1] = key;
    argvlen[1] = sal_strlen(key);
    argv[2] = start_str;
    argvlen[2] = sal_strlen(start_str);
    argv[3] = end_str;
    argvlen[3] = sal_strlen(end_str);
    
    reply =(redisReply*)redisCommandArgv(p_master->p_context, DB_ARGC_4, argv, argvlen);
    if (NULL == reply)
    {
        return -1;
    }

    if (REDIS_REPLY_ARRAY == reply->type)
    {
        for (i = 0; i < reply->elements; i++)
        {
            element = reply->element[i];
            if (REDIS_REPLY_STRING == element->type)
            {
                if ((buffer_pos + element->len) >= buffer_len)
                {
                    rc = -1;
                    goto EXIT;
                }            
                sal_memcpy(out_buf + buffer_pos, (uint8*)element->str, element->len);
                buffer_pos += element->len;
            }
        }
    }
    
    *buf_len = buffer_pos;
    *count = reply->elements;

EXIT:
    freeReplyObject(reply);
    return rc;
#else
    return 0;
#endif
}

int32
dbclient_lrange_handle(char *key, int32 start, int32 end, DBCLIENT_STORE_CB_FUNC fn, void *aux)
{
#ifdef CONFIG_DBCLIENT
    ctc_dbclient_master_t *p_master = g_db_master;
    redisReply *reply = NULL;
    redisReply *element = NULL;
    const char* argv[DB_ARGC_4];
    size_t argvlen[DB_ARGC_4];
    uint32 i = 0;
    char start_str[DB_SIZE_32];
    char end_str[DB_SIZE_32];
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    sal_snprintf(start_str, DB_SIZE_32, "%d", start);
    sal_snprintf(end_str, DB_SIZE_32, "%d", end);
    
    argv[0] = "LRANGE";
    argvlen[0] = 6;
    argv[1] = key;
    argvlen[1] = sal_strlen(key);
    argv[2] = start_str;
    argvlen[2] = sal_strlen(start_str);
    argv[3] = end_str;
    argvlen[3] = sal_strlen(end_str);
    
    reply =(redisReply*)redisCommandArgv(p_master->p_context, DB_ARGC_4, argv, argvlen);
    if (NULL == reply)
    {
        return -1;
    }

    if (REDIS_REPLY_ARRAY == reply->type)
    {
        for (i = 0; i < reply->elements; i++)
        {
            element = reply->element[i];
            if (REDIS_REPLY_STRING == element->type)
            {
                fn(element->str, element->len, aux);
            }
        }
    }

    freeReplyObject(reply);
    return rc;
#else
    return 0;
#endif
}

