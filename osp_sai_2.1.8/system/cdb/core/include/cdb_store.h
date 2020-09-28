
#ifndef __CDB_STORE_H__
#define __CDB_STORE_H__

typedef int32 (*CDB_STORE_TBL_FUNC)(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len);

typedef int32 (*CDB_STORE_DS_FUNC)(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, 
                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len);

int32
cdb_store_tbl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len);

int32
cdb_store_ds(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,
                void *p_data, uint32 data_len);

int32
cdb_store_ds_refer(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,
                void *p_data, uint32 data_len);
int32
cdb_store_tbl_refer(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, 
                void *p_data, uint32 data_len);
            
int32
cdb_format_tbl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len);

int32
cdb_format_ds(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,
                void *p_data, uint32 data_len);
#endif /* !__CDB_STORE_H__ */

