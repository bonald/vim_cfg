
#ifndef __CDB_H__
#define __CDB_H__

#include "gen/cdb_define.h"

#define INVALID_UINT32_ID   0xFFFFFFFF
#define INVALID_CDB_ID      0xFFFF

#define CDB_FLAG_CHECK_SET(PDB, PPARAM, FIELD, FLAG) \
if (GLB_FLAG_ISSET((PPARAM)->FIELD, (FLAG))) \
{ \
    GLB_SET_FLAG((PDB)->FIELD, (FLAG)); \
} \
else \
{ \
    GLB_UNSET_FLAG((PDB)->FIELD, (FLAG)); \
}

#define CDB_FLAG_CHECK_UNSET(PDB, PPARAM, FIELD, FLAG) \
if (GLB_FLAG_ISSET((PPARAM)->FIELD, (FLAG))) \
{ \
    GLB_UNSET_FLAG((PDB)->FIELD, (FLAG)); \
} \
else \
{ \
    GLB_SET_FLAG((PDB)->FIELD, (FLAG)); \
}

typedef enum
{
    CDB_SUB_CDB    = 0,
    CDB_SUB_TBL    = 1,
    CDB_SUB_MAX    = 2
} cdb_subscribe_type_t;

typedef enum
{
    CDB_SUB_FORMAT_RAW = 0,
    CDB_SUB_FORMAT_PROTOBUF,
    CDB_SUB_FORMAT_RAWSNMP,
    CDB_SUB_FORMAT_MAX
} cdb_subscribe_format_t;

typedef enum
{
    CDB_NODE_DIR    = 0,
    CDB_NODE_TBL    = 1,
    CDB_NODE_DS     = 2,
    CDB_NODE_ACT    = 3,
    CDB_NODE_BAT    = 4,
    CDB_NODE_MAX    = 5
} cdb_node_type_t;

typedef enum
{
    CDB_PRODUCT_ALL      = 0,
    CDB_PRODUCT_L2PLUS   = 1,
    CDB_PRODUCT_OPENFLOW = 2,
    CDB_PRODUCT_TAP      = 3,
    CDB_PRODUCT_MAX      = 4
} cdb_product_type_t;

#define CDB_DS_INVALID  ((uint16)-1)

typedef enum
{
    CDB_DT_UINT         = 0,
    CDB_DT_UINT64       = 1,
    CDB_DT_INT          = 2,
    CDB_DT_INT64        = 3,
    CDB_DT_STRING       = 4,
    CDB_DT_ENUM8        = 5,
    CDB_DT_ENUM32       = 6,
    CDB_DT_FLAGS8       = 7,
    CDB_DT_FLAGS32      = 8,
    CDB_DT_NONE         = 9,
    CDB_DT_DS           = 10,
    CDB_DT_REFERLIST    = 11,
    CDB_DT_COMPOSE      = 12,
    CDB_DT_VOID         = 13,
    CDB_DT_DOUBLE      = 14,
    CDB_DT_MAX          = 15
} cdb_dt_type_t;

typedef enum
{
    CDB_ACCESS_RD       = 0,
    CDB_ACCESS_RW       = 1,
    CDB_ACCESS_SUB      = 2,
    CDB_ACCESS_MAX      = 3
} cdb_access_t;

typedef struct
{
    uint8    access;        /* cdb_access_t */
    uint8    dt_type;       /* cdb_dt_type_t */
    uint16   ds_type;       /* cdb_ds_t */
    char*    name;
} cdb_field_t;

#define TBL_ITERATE_ARGS_NUM    8
#define CDB_GET_COUNT           1024

typedef struct
{
    void *node_array[CDB_GET_COUNT];
    uint32 index;    
} cdb_get_node_t;

typedef struct
{
    void  *argv[TBL_ITERATE_ARGS_NUM];
} tbl_iter_args_t;

typedef int32 (*TBL_ITER_CB_FUNC)(void *obj, tbl_iter_args_t *pargs);

/* iterate instance function used for fulldump */
typedef int32 (*TBL_ITER_FUNC)(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

typedef int32 (*TBL_DUMP_FUNC)(void *obj, tbl_iter_args_t *pargs);

typedef char* (*TBL_KEY_V2S_FUNC)(void *obj, char *str, uint32 str_len);

typedef char* (*TBL_KEY_NAME_DUMP_FUNC)(void *p_tbl, char* str);
typedef char* (*TBL_KEY_VALUE_DUMP_FUNC)(void *p_tbl, char* str);
typedef char* (*TBL_FIELD_NAME_DUMP_FUNC)(void *p_tbl, int32 field_id, char* str);
typedef char* (*TBL_FIELD_VALUE_DUMP_FUNC)(void *p_tbl, int32 field_id, char* str);
typedef char** (*TBL_ALL_DUMP_FUNC)(void *p_tbl, char** str);
typedef void* (*TBL_PARSER_FUNC)(char** str, char* key_value, void *p_tbl);

typedef char* (*DS_KEY_NAME_DUMP_FUNC)(void *p_tbl, void* p_ds,  char* str);
typedef char* (*DS_KEY_VALUE_DUMP_FUNC)(void *p_tbl, void* p_ds,  char* str);
typedef char* (*DS_FIELD_NAME_DUMP_FUNC)(void *p_tbl, void* p_ds, int32 field_id, char* str);
typedef char* (*DS_FIELD_VALUE_DUMP_FUNC)(void *p_tbl, void* p_ds, int32 field_id, char* str);
typedef char** (*DS_DUMP_FUNC)(void *p_tbl, void* p_ds, char** str);
typedef void* (*DS_PARSER_FUNC)(char** str, char* key_value, void *p_tbl, void* p_ds);

typedef struct cdb_sync_tbl_func_
{
    TBL_KEY_NAME_DUMP_FUNC  dump_key_name;
    TBL_KEY_VALUE_DUMP_FUNC dump_key_value;
    TBL_FIELD_NAME_DUMP_FUNC dump_field_name;
    TBL_FIELD_VALUE_DUMP_FUNC dump_field_value;
    TBL_ALL_DUMP_FUNC dump_table;
    TBL_PARSER_FUNC parser_table;
}cdb_sync_tbl_func_t;

typedef struct cdb_sync_ds_func_
{
    DS_KEY_NAME_DUMP_FUNC dump_key_name;
    DS_KEY_VALUE_DUMP_FUNC dump_key_value;
    DS_FIELD_NAME_DUMP_FUNC dump_field_name;
    DS_FIELD_VALUE_DUMP_FUNC dump_field_value;
    DS_DUMP_FUNC dump_table;
    DS_PARSER_FUNC parser_table;
}cdb_sync_ds_func_t;

typedef struct
{
    TBL_ITER_FUNC       iter_fn;
    TBL_DUMP_FUNC       dump_fn;
    TBL_KEY_V2S_FUNC    key_v2s_fn;
    CMD_FUNC            cmd_fn;
} cdb_tbl_ops_t;

typedef struct
{
    uint16          key_size;
    uint16          data_size;
    uint32          field_num;
    cdb_field_t    *field;
    cdb_tbl_ops_t   ops;
    cdb_sync_tbl_func_t tbl_op_db; 
    cdb_sync_ds_func_t ds_op_db;
} cdb_tbl_info_t;

typedef struct cdb_node_s
{
    uint16              id;         /* cdb_dir_t / cdb_tbl_t / cdb_ds_t / cdb_bat_t */
    uint16              type;       /* cdb_node_type_t */
    uint32              product;    /* cdb_product_type_t */
    char                *name;
    char                *full_name;
    struct cdb_node_s   *parent;
    cdb_tbl_info_t      *tbl_info;
    uint32              inited;     /* only for TBL */
    ctclib_list_t       sub;
    ctclib_list_node_t  node;
} cdb_node_t;

#define CDB_REF_LIST_COUNT(RL)      CTCLIB_SLISTCOUNT((RL)->obj_list)
#define CDB_REF_LIST_ISEMPTY(RL)    CTCLIB_SLIST_ISEMPTY((RL)->obj_list)

typedef int32 (*CDB_SUBSCRIBE_TBL_FUNC)(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);

typedef int32 (*CDB_SUBSCRIBE_TBL_REFER_FUNC)(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds,
                cdb_sync_refer_t *p_refer);
    
char*
str_tolower(char *str);

char*
str_toupper(char *str);

int32
cdb_check_product(cdb_node_t* p_node);

int32
cdb_check_product_out(cdb_node_t* p_node, FILE *fp);

int32
call_show_func(cfg_cmd_para_t *para, void (*func)(), void* args);

int32
cdb_init(pm_id_t pm_id);

cdb_node_t*
cdb_get_dir(uint16 id);

cdb_node_t*
cdb_get_tbl(uint16 id);

cdb_node_t*
cdb_get_ds(uint16 id);

cdb_node_t*
cdb_get_act(uint16 id);

cdb_node_t*
cdb_get_tbl_by_name(char *name);

cdb_node_t*
cdb_get_ds_by_name(char *name);

const char*
cdb_node_type_str(uint32 type);

const char*
cdb_product_type_str(uint32 type);

const char*
cdb_ds_val2str(uint16 ds_type);

int32
cdb_get_field_id(char *field_name, cdb_node_t* p_node, cdb_field_t **pp_field);

cdb_node_t*
cdb_get_node_by_id(uint16 type, uint16 id);

cdb_node_t*
cdb_get_node_by_name(const char *path);

int32
cdb_register_cfg_tbl_cb(uint16 tbl_id, CMD_FUNC cb);

int32
cdb_register_cfg_act_cb(uint16 act_id, CMD_FUNC cb);

int32
tbl_hash_iter_adpt_fn(ctclib_hash_backet_t* phb, void *parg1, void *p_arg2);

int32
tbl_hash_iter_adpt_fn_cont(ctclib_hash_backet_t* phb, int32 cur_index, void *parg1, void *p_arg2);

int32
cfg_cmd_parse_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key);

int32
cdb_reference_list_add_object(cdb_reference_list_t *ref_list, void *p_object);

int32
cdb_reference_list_del_object(cdb_reference_list_t *ref_list, void *p_object);

void*
cdb_reference_list_lookup_object(cdb_reference_list_t *ref_list, void *p_object);

int32
cdb_reference_list_init(cdb_reference_list_t *ref_list, uint16 type, uint16 id, CTCLIB_SLIST_CMP_FN cmp);

int32
cdb_reference_list_deinit(cdb_reference_list_t *ref_list);

int32
cdb_reference_init(cdb_reference_t *p_ref, uint16 id);

int32
cdb_reference_deinit(cdb_reference_t *p_ref);

int32
cdb_pm_subscribe_tbl(cdb_tbl_t tbl_id, CDB_SUBSCRIBE_TBL_FUNC before_fn, CDB_SUBSCRIBE_TBL_FUNC after_fn);

int32
cdb_pm_subscribe_tbl_refer(cdb_tbl_t tbl_id, CDB_SUBSCRIBE_TBL_REFER_FUNC before_fn, CDB_SUBSCRIBE_TBL_REFER_FUNC after_fn);

int32
cdb_pm_subscribe_ds_refer(cdb_ds_t ds_id, CDB_SUBSCRIBE_TBL_REFER_FUNC before_fn, CDB_SUBSCRIBE_TBL_REFER_FUNC after_fn);

int32
cdb_store_data(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg);

int32
cdb_build_refer_tbl(uint32 tbl_id, uint32 add, cdb_sync_refer_hdr_t *p_refer);

int32
cdb_get_product_type();

#endif /* !__CDB_H__ */

