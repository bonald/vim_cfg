
#ifndef __LIB_LOAD_H__
#define __LIB_LOAD_H__

typedef int32 (*CDS_SYNC_LOCAL_TBL_SET_CB)(uint32 tbl_id, uint32 field_id, void *p_db_tbl);

int32
lib_load_set_cds_sync_set_func(CDS_SYNC_LOCAL_TBL_SET_CB fn);

int32
lib_load_init();

int32
lib_load_fini();

#endif /* !__LIB_LOAD_H__ */

