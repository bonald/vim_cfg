#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "ivykis/src/include/iv.h"
#include "hiredis.h"
#include "async.h"
#include "rdb_hiredis.h"

#include "sal.h"
#include "proto.h"

static char* g_p_data[CMD_BUF_512];
#define RDB_PORT 6379

//#define CONSOLE_DEBUG_RDB 1
#if 0
#define DB_PRINT_CON(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}
#else
#define DB_PRINT_CON(fmt, args...)                      \
{                                                            \
                                    \
}
#endif

typedef struct redisIvykisEvents {
    redisAsyncContext *context;
    struct iv_fd fd;
} redisIvykisEvents;

static redisAsyncContext *con_context = NULL;
static redisAsyncContext *con_context_pubsub[REDIS_CHANNEL_MAX_NUMBER];
static redisContext *con_context_sync_mode = NULL;

static void redisIvykisReadEvent(void *arg) {
    redisAsyncContext *context = (redisAsyncContext *)arg;
    redisAsyncHandleRead(context);
}

static void redisIvykisWriteEvent(void *arg) {
    redisAsyncContext *context = (redisAsyncContext *)arg;
    redisAsyncHandleWrite(context);
}

static void redisIvykisAddRead(void *privdata) {
    redisIvykisEvents *e = (redisIvykisEvents*)privdata;
    iv_fd_set_handler_in(&e->fd, redisIvykisReadEvent);
}

static void redisIvykisDelRead(void *privdata) {
    redisIvykisEvents *e = (redisIvykisEvents*)privdata;
    iv_fd_set_handler_in(&e->fd, NULL);
}

static void redisIvykisAddWrite(void *privdata) {
    redisIvykisEvents *e = (redisIvykisEvents*)privdata;
    iv_fd_set_handler_out(&e->fd, redisIvykisWriteEvent);
}

static void redisIvykisDelWrite(void *privdata) {
    redisIvykisEvents *e = (redisIvykisEvents*)privdata;
    iv_fd_set_handler_out(&e->fd, NULL);
}

static void redisIvykisCleanup(void *privdata) {
    redisIvykisEvents *e = (redisIvykisEvents*)privdata;

    iv_fd_unregister(&e->fd);
    free(e);
}


void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    DB_PRINT_CON("argv[%s]: %s\n", (char*)privdata, reply->str);

    /* Disconnect after receiving the reply to GET */
    redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        DB_PRINT_CON("Error: %s\n", c->errstr);
        return;
    }
    DB_PRINT_CON("RDB Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    DB_PRINT_CON("RDB Disconnected...\n");
}

static int redisIvykisAttach(redisAsyncContext *ac) {
    redisContext *c = &(ac->c);
    redisIvykisEvents *e;

    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Create container for context and r/w events */
    e = (redisIvykisEvents*)malloc(sizeof(*e));
    e->context = ac;

    /* Register functions to start/stop listening for events */
    ac->ev.addRead = redisIvykisAddRead;
    ac->ev.delRead = redisIvykisDelRead;
    ac->ev.addWrite = redisIvykisAddWrite;
    ac->ev.delWrite = redisIvykisDelWrite;
    ac->ev.cleanup = redisIvykisCleanup;
    ac->ev.data = e;

    /* Initialize and install read/write events */
    IV_FD_INIT(&e->fd);
    e->fd.fd = c->fd;
    e->fd.handler_in = redisIvykisReadEvent;
    e->fd.handler_out = redisIvykisWriteEvent;
    e->fd.handler_err = NULL;
    e->fd.cookie = e->context;

    iv_fd_register(&e->fd);

    return REDIS_OK;
}

int32
rdb_connect (void) 
{
    int i;
    
    con_context = redisAsyncConnect("127.0.0.1", RDB_PORT);
    DB_PRINT_CON("Connect to localhost\n");
    if (con_context->err) 
    {
        /* Let *c leak for now... */
        DB_PRINT_CON("RDB connection Error: %s\n", con_context->errstr);
        return 1;
    }


    con_context_sync_mode = redisConnect("127.0.0.1", RDB_PORT);
    DB_PRINT_CON("RDB Connect to localhost with sync mode for get\n");
    if (con_context_sync_mode->err) 
    {
        /* Let *c leak for now... */
        DB_PRINT_CON("RDB connection sync mode Error: %s\n", con_context_sync_mode->errstr);
        return 1;
    }    

    redisIvykisAttach(con_context);
    redisAsyncSetConnectCallback(con_context,connectCallback);
    redisAsyncSetDisconnectCallback(con_context,disconnectCallback);

    for(i =0; i<CMD_BUF_512; i++)
    {
        g_p_data[i] = sal_calloc(CMD_BUF_4096);
    }
    return 0;
}


int32 
rdb_setup_subscribe_connection (redisAsyncContext ** context) 
{
    static int subscribe_count = 0;
    
    con_context_pubsub[subscribe_count] = redisAsyncConnect("127.0.0.1", RDB_PORT);
    DB_PRINT_CON("Connect to localhost\n");
    if (con_context_pubsub[subscribe_count]->err) 
    {
        /* Let *c leak for now... */
        DB_PRINT_CON("RDB connection for subpub Error: %d\n", con_context_pubsub[subscribe_count]->err);
        return 1;
    }

    redisIvykisAttach(con_context_pubsub[subscribe_count]);
    redisAsyncSetConnectCallback(con_context_pubsub[subscribe_count],connectCallback);
    redisAsyncSetDisconnectCallback(con_context_pubsub[subscribe_count],disconnectCallback);

    (*context) = con_context_pubsub[subscribe_count];

    subscribe_count ++;
    
    return 0;
}


int32 
rdb_sync_get_table(int32 region, char* key_name, char* data[])
{
    redisReply *reply;
    int i;
    
    /*insert the key:value to HASH*/
    reply = redisCommand(con_context_sync_mode, "HGETALL %s", key_name); 

    if(reply->type == REDIS_REPLY_ARRAY)
    {
        for(i=0; i<reply->elements; i++)
        {
            DB_PRINT_CON("%s \n", reply->element[i]->str);
            sal_sprintf(data[i], "%s", reply->element[i]->str);
        }
    }

    if(!reply)
    {
        DB_PRINT_CON("HGETALL NULL: %s\n", con_context_sync_mode->errstr);
        return -1;
    }
    else
    {
        freeReplyObject(reply);
    }
    
    return 0;
}


/* note how to call callback func in redis
        fn = rdb_subscribe_channel_callback 
        cb->privdata = func
*/
void rdb_subscribe_channel_callback(struct redisAsyncContext* c, void* re,  void* func)
{
    int i;
    struct redisReply* reply = re;
    rdb_subscribe_callback call;


    DB_PRINT_CON("redis subscribe Callback \n");
    if(NULL == reply)
    {
        DB_PRINT_CON("subscribe reply is NULL \n"); 
        return;
    }
    if(reply->type == REDIS_REPLY_ARRAY)
    {
        for(i=0; i<reply->elements; i++)
        {
            DB_PRINT_CON("%s \n", reply->element[i]->str);
            if(reply->element[i]->str)
            {
                sal_sprintf(g_p_data[i], "%s", reply->element[i]->str);
            }
            
        }

        if(sal_strcmp(reply->element[REDIS_CHANNEL_MESSAGE_STR]->str,"pmessage"))
        {
            /*in this function, we only process channel subscribe message, else return*/
            return;
        }
        
        if(func && reply->elements == REDIS_CHANNEL_MESSAGE_STR_NUM && reply->element[REDIS_CHANNEL_PUBLISH_STR]->str)
        {
            /*it is a callback regestered by subscribe channel*/
            call = (rdb_subscribe_callback)func;
            call(reply->element[REDIS_CHANNEL_NAME_STR]->str, reply->element[REDIS_CHANNEL_PUBLISH_STR]->str, g_p_data);
        }
    }

    return;
}

int32
rdb_subscribe_channel(char* channel_name, void* func)
{
    /* note how to call callback func in redis
    if (cb->fn != NULL) {
        cb->fn(ac,reply,cb->privdata);
        
        fn = rdb_subscribe_channel_callback 
        cb->privdata = func
    */
    redisAsyncContext * context = NULL;

    if(rdb_setup_subscribe_connection(&context))
    {
        DB_PRINT_CON("Con_context error !\n");
        return -1;
    }
    
    
    redisAsyncCommand(context, rdb_subscribe_channel_callback, func, "PSUBSCRIBE %s%s", channel_name, "*");
    return 0;
}


int32 
rdb_publish_add_table(char* channel_name, char* table_name, char* key_name, char* key_value, 
        int field_num, char** field_array)
{
    int ret;

    if(NULL == con_context)
    {
        DB_PRINT_CON("Con_context is NULL when publish add!\n");
        DB_PRINT_CON("     channel name is %s, key_name is %s, field_num is %d!\n", channel_name, key_name, field_num);
        return -1;
    }
    
    /*insert the keys to keys_list*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "SADD %s %s", table_name, key_value);
    if(ret)
    {
        DB_PRINT_CON("SADD Error: %s\n", con_context->errstr);
    }

    /*insert the key:value to HASH*/
    sal_sprintf(field_array[0],"HMSET");
    ret = redisAsyncCommandArgv(con_context, NULL, NULL, field_num, (const char **)field_array, NULL);
    if(ret)
    {
        DB_PRINT_CON("HMSET Error: %s\n", con_context->errstr);
    }
    /*publish the table event*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "PUBLISH %s %s"RDB_PARTITION_CHAR"%s", 
        channel_name, 
        REDIS_PUBLISH_ACTION_ADD,
        key_name); 
    if(ret)
    {
        DB_PRINT_CON("PUBLISH Error: %s\n", con_context->errstr);
    }

    return ret;
}

int32 
rdb_publish_set_table(char* channel_name, char* table_name, char* key_name, char* key_value, 
        char* field_name, char* field_value)
{
    int ret;

    if(NULL == con_context)
    {
        DB_PRINT_CON("Con_context is NULL when publish set!\n");
        DB_PRINT_CON("     channel name is %s, key_name is %s, field_name is %s!\n", channel_name, key_name, field_name);
        return -1;
    }
    
    /*insert the key:value to HASH*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "HSET %s %s %s", 
        key_name, field_name, field_value); 
    if(ret)
    {
        DB_PRINT_CON("HSET Error: %s\n", con_context->errstr);
    }
    
    /*publish the table event*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "PUBLISH %s %s"RDB_PARTITION_CHAR"%s"RDB_PARTITION_CHAR"%s"RDB_PARTITION_CHAR"%s", 
        channel_name, 
        REDIS_PUBLISH_ACTION_SET,
        key_name,field_name, field_value); 
    if(ret)
    {
        DB_PRINT_CON("PUBLISH Error: %s\n", con_context->errstr);
    }

    return ret;
}

int32 
rdb_publish_del_table(char* channel_name, char* table_name, char* key_name, char* key_value)
{
    int ret;

    if(NULL == con_context)
    {
        DB_PRINT_CON("Con_context is NULL when publish del!\n");
        DB_PRINT_CON("     channel name is %s, key_name is %s!\n", channel_name, key_name);
        return -1;
    }

    /*delete the keys from keys_list*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "SREM %s %s", table_name, key_value);
    if(ret)
    {
        DB_PRINT_CON("SREM Error: %s\n", con_context->errstr);
    }
    
    /*delete the key:value from HASH*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "DEL %s ", key_name); 
    if(ret)
    {
        DB_PRINT_CON("DEL Error: %s\n", con_context->errstr);
    }
    
    /*publish the table event*/
    ret = redisAsyncCommand(con_context, NULL, NULL, "PUBLISH %s %s"RDB_PARTITION_CHAR"%s", 
        channel_name, 
        REDIS_PUBLISH_ACTION_DEL,
        key_name); 
    if(ret)
    {
        DB_PRINT_CON("PUBLISH Error: %s\n", con_context->errstr);
    }

    return ret;
}


