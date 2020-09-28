
#ifndef __CDB_SYNC_H__
#define __CDB_SYNC_H__

#include "gen/cdb_define.h"
#include "cdb.h"


int32
cdb_sync_tbl_add(uint32 tbl_id, void *p_db_tbl);

int32
cdb_sync_tbl_del(uint32 tbl_id, void *p_db_tbl);

int32
cdb_sync_tbl_set(uint32 tbl_id, uint32 field_id, void *p_db_tbl);

int32
cdb_sync_ds_add(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void *p_db_ds);

int32
cdb_sync_ds_del(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void *p_db_ds);

int32
cdb_sync_ds_set(uint32 tbl_id, uint32 ds_id, uint32 field_id, void *p_db_tbl, void *p_db_ds);

int32
cdb_sync_ds2_add(uint32 tbl_id, uint32 ds_id1, uint32 ds_id2, void *p_db_tbl, void *p_db_ds1, void *p_db_ds2);

int32
cdb_sync_ds2_del(uint32 tbl_id, uint32 ds_id1, uint32 ds_id2, void *p_db_tbl, void *p_db_ds1, void *p_db_ds2);

int32
cdb_sync_ds2_set(uint32 tbl_id, uint32 ds_id1, uint32 ds_id2, uint32 field_id, void *p_db_tbl, void *p_db_ds1, void *p_db_ds2);

int32
cdb_sync_tbl_set_refer(uint32 tbl_id, uint32 field_id, void *p_db_tbl, 
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj);

int32
cdb_sync_ds_set_refer(uint32 tbl_id, uint32 ds_id, uint32 field_id, void *p_db_tbl, void *p_db_ds, 
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj);

/*here is to publish table to out db*/
int32
cdb_sync_subscribe_channel(cdb_tbl_t tbl_id, CDB_SUBSCRIBE_TBL_FUNC before_fn, CDB_SUBSCRIBE_TBL_FUNC after_fn);
int32
cdb_sync_publish_tbl_add(uint32 tbl_id, void *p_db_tbl);
int32
cdb_sync_publish_ds_add(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void* p_db_ds);
int32
cdb_sync_publish_tbl_set(uint32 tbl_id, uint32 field_id, void *p_db_tbl);
int32
cdb_sync_publish_ds_set(uint32 tbl_id, uint32 ds_id, uint32 field_id, void *p_db_tbl, void *p_db_ds);
int32
cdb_sync_publish_tbl_del(uint32 tbl_id, void *p_db_tbl);
int32
cdb_sync_publish_ds_del(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void* p_db_ds);

int32
cdb_sync_publish_init();


#endif /* !__CDB_SYNC_H__ */

