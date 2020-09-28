
#ifndef __CDB_CLIENT_SUBSCRIBE_H__
#define __CDB_CLIENT_SUBSCRIBE_H__

/* subscribe cdb */
int32
cdbclt_subscribe_register_user_cb(CDB_SUBSCRIBE_USER_FUNC fn);

int32
cdbclt_subscribe_register_action_cb(CDB_SUBSCRIBE_ACTION_FUNC fn);

int32
cdbclt_subscribe_init_cdb(char *path, cdb_subscribe_format_t format);

int32
cdbclt_subscribe_init_tbl(char *tables, cdb_subscribe_format_t format, uint32 sync);

int32
cdbclt_subscribe_init_tbl_new(char *tables, cdb_subscribe_format_t format, uint32 sync);

int32
cdbclient_execvp(char **argv);

int32
get_last_exec_result();

#endif /* !__CDB_CLIENT_SUBSCRIBE_H__ */

