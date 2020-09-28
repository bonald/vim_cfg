#ifndef __HIREDIS_IVYKIS_H__
#define __HIREDIS_IVYKIS_H__

#include "sal.h"
#include "proto.h"

#define REDIS_PUBLISH_ACTION_ADD "ADD"
#define REDIS_PUBLISH_ACTION_SET "SET"
#define REDIS_PUBLISH_ACTION_GET "GET"
#define REDIS_PUBLISH_ACTION_DEL "DEL"

#define REDIS_CHANNEL_MESSAGE_STR_NUM 4
#define REDIS_CHANNEL_MESSAGE_STR 0
#define REDIS_CHANNEL_NAME_STAR_STR 1
#define REDIS_CHANNEL_NAME_STR 2
#define REDIS_CHANNEL_PUBLISH_STR 3
#define REDIS_CHANNEL_MAX_NUMBER 8192

typedef int (*rdb_subscribe_callback)(char* channel, char* str, char** data);

int32
rdb_connect (void);

int32 
rdb_sync_get_table(int32 region, char* key_name, char* data[]);

int32
rdb_subscribe_channel(char* channel_name, void* func);

int32
rdb_publish_add_table(char* channel_name, char* table_name, char* key_name, char* key_value, 
        int field_num, char** field_array);
int32 
rdb_publish_set_table(char* channel_name, char* table_name, char* key_name, char* key_value, 
        char* field_name, char* field_value);
int32 
rdb_publish_del_table(char* channel_name, char* table_name, char* key_name, char* key_value);

#endif


