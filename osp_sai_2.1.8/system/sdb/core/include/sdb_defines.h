
#ifndef __SDB_DEFINES_H__
#define __SDB_DEFINES_H__

typedef enum
{
    SDB_E_NONE              = 0,

    SDB_E_FAIL              = -1000,
    SDB_E_INVALID_PARAM     = -999,
    SDB_E_NOT_EXIST         = -998,
    SDB_E_EXIST             = -997,
    SDB_E_NOT_INITED        = -996,
    SDB_E_INITED            = -995,
    SDB_E_INIT_FAIL         = -994,
    SDB_E_NO_MEMORY         = -993,
    SDB_E_SOCK              = -992,
    SDB_E_NOT_SUPPORT       = -991,
    SDB_E_NOT_PERMIT        = -990,

    SDB_E_FLOW_EXIST        = -989,
    SDB_E_CFG_PROMPT        = -988
} sdb_error_t;

#define SDB_FLAG_CHECK_SET(PDB, PPARAM, FIELD, FLAG) \
if (GLB_FLAG_ISSET((PPARAM)->FIELD, (FLAG))) \
{ \
    GLB_SET_FLAG((PDB)->FIELD, (FLAG)); \
} \
else \
{ \
    GLB_UNSET_FLAG((PDB)->FIELD, (FLAG)); \
}

#define SDB_FLAG_CHECK_UNSET(PDB, PPARAM, FIELD, FLAG) \
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
    SDB_NODE_DIR    = SDB_NODE_BASE,
    SDB_NODE_TBL    = SDB_NODE_BASE+1,
    SDB_NODE_DS     = SDB_NODE_BASE+2,
    SDB_NODE_ACT    = SDB_NODE_BASE+3,
    SDB_NODE_MAX    = SDB_NODE_BASE+4
} sdb_node_type_t;

typedef enum
{
    SDB_PRODUCT_ALL      = 0,
    SDB_PRODUCT_L2PLUS   = 1,
    SDB_PRODUCT_OPENFLOW = 2,
    SDB_PRODUCT_TAP      = 3,
    SDB_PRODUCT_MAX      = 4
} sdb_product_type_t;

typedef enum
{
    SDB_DT_UINT         = 0,
    SDB_DT_UINT64       = 1,
    SDB_DT_INT          = 2,
    SDB_DT_INT64        = 3,
    SDB_DT_STRING       = 4,
    SDB_DT_ENUM8        = 5,
    SDB_DT_ENUM32       = 6,
    SDB_DT_FLAGS8       = 7,
    SDB_DT_FLAGS32      = 8,
    SDB_DT_NONE         = 9,
    SDB_DT_DS           = 10,
    SDB_DT_REFERLIST    = 11,
    SDB_DT_COMPOSE      = 12,
    SDB_DT_VOID         = 13,
    SDB_DT_DOUBLE       = 14,
    SDB_DT_MAX          = 15
} sdb_dt_type_t;

typedef enum
{
    SDB_ACCESS_RD       = 0,
    SDB_ACCESS_RW       = 1,
    SDB_ACCESS_SUB      = 2,
    SDB_ACCESS_MAX      = 3
} sdb_access_t;

typedef struct
{
    uint8    access;        /* sdb_access_t */
    uint8    dt_type;       /* sdb_dt_type_t */
    uint16   ds_type;       /* sdb_ds_t */
    char*    name;
} sdb_field_t;

#define STBL_ITERATE_ARGS_NUM    8
#define SDB_GET_COUNT           1024

typedef struct
{
    void *node_array[SDB_GET_COUNT];
    uint32 index;    
} sdb_get_node_t;

typedef struct
{
    void  *argv[STBL_ITERATE_ARGS_NUM];
} stbl_iter_args_t;

typedef int32 (*STBL_ITER_CB_FUNC)(void *obj, stbl_iter_args_t *pargs);

/* iterate instance function used for fulldump */
typedef int32 (*STBL_ITER_FUNC)(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);
typedef int32 (*STBL_DUMP_FUNC)(void *obj, stbl_iter_args_t *pargs);
typedef char* (*STBL_KEY_V2S_FUNC)(void *obj, char *str, uint32 str_len);
typedef int32 (*SCMD_FUNC)(char **argv, int32 argc, void *para);

typedef struct
{
    STBL_ITER_FUNC       iter_fn;
    STBL_DUMP_FUNC       dump_fn;
    STBL_KEY_V2S_FUNC    key_v2s_fn;
    SCMD_FUNC            cmd_fn;
} sdb_tbl_ops_t;

typedef struct
{
    uint16          key_size;
    uint16          data_size;
    uint32          field_num;
    sdb_field_t    *field;
    sdb_tbl_ops_t   ops;
} sdb_tbl_info_t;

typedef struct sdb_node_s
{
    uint16              id;         /* sdb_dir_t / sdb_tbl_t */
    uint16              type;       /* sdb_node_type_t */
    uint32              product;    /* sdb_product_type_t */
    char                *name;
    char                *full_name;
    struct sdb_node_s   *parent;
    sdb_tbl_info_t      *tbl_info;
    uint32              inited;     /* only for TBL */
    ctclib_list_t       sub;
    ctclib_list_node_t  node;
} sdb_node_t;

typedef struct
{
    uint16              type;   /* sdb_node_type_t */
    uint16              id;
    ctclib_slist_t      *obj_list;
} sdb_reference_list_t;

typedef struct
{
    void           *p_stats; /* store ptr to SDK define ctc_stats_mac_rec_t */
} sdb_mac_rx_stats_ptr_t;

typedef struct
{
    void           *p_stats; /* store ptr to SDK' define ctc_stats_mac_snd_t */
} sdb_mac_tx_stats_ptr_t;

#define SDB_REF_LIST_COUNT(RL)      CTCLIB_SLISTCOUNT((RL)->obj_list)
#define SDB_REF_LIST_ISEMPTY(RL)    CTCLIB_SLIST_ISEMPTY((RL)->obj_list)

int32
sdb_init(uint32 pm_id);

sdb_node_t*
sdb_get_dir(uint16 id);

sdb_node_t*
sdb_get_tbl(uint16 id);

const char*
sdb_node_type_str(uint32 type);

sdb_node_t*
sdb_get_tbl_by_name(char *name);

sdb_node_t*
sdb_get_node_by_name(const char *path);

sdb_node_t*
sdb_get_node_by_id(uint16 type, uint16 id);

int32
sdb_register_cfg_tbl_cb(uint16 tbl_id, SCMD_FUNC cb);

#endif /* __SDB_DEFINES_H__ */

