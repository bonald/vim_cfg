
#ifndef _CTC_DBCLIENT_H_
#define _CTC_DBCLIENT_H_

typedef int32 (*DBCLIENT_STORE_CB_FUNC)(void *obj, uint32 len, void *aux);

int32
dbclient_init();

int32
dbclient_deinit();

int32
dbclient_flushdb();

int32
dbclient_save();

int32
dbclient_lpush_binary(char *key, uint8 *value, uint32 value_len);

int32
dbclient_lrange_handle(char *key, int32 start, int32 end, DBCLIENT_STORE_CB_FUNC fn, void *aux);

#endif /* !_CTC_DBCLIENT_H_ */

