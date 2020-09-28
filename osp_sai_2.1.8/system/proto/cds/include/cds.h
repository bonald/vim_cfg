
#ifndef __CDS_H__
#define __CDS_H__

typedef struct cds_master_s
{
//    void  *db_client;
} cds_master_t;

int32
cds_sync_local_tbl_set(uint32 tbl_id, uint32 field_id, void *p_db_tbl);

int32
cds_init();

#endif /* !__CDS_H__ */

