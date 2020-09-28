#include "proto.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"

static int32
_cdb_sync_tbl_full_add_common_one(void *obj, tbl_iter_args_t *pargs)
{
    cdb_node_t* p_node = pargs->argv[0];
    return ipc_pm_to_cds_data(CDB_OPER_ADD, CDB_NODE_TBL, p_node->id, 0, obj, p_node->tbl_info->data_size);
}

static int32
_cdb_sync_tbl_full_set_common_one(void *obj, tbl_iter_args_t *pargs)
{
    cdb_node_t* p_node = pargs->argv[0];
    uint32 field_id = *(uint32*)pargs->argv[1];
    return ipc_pm_to_cds_data(CDB_OPER_SET, CDB_NODE_TBL, p_node->id, field_id, obj, p_node->tbl_info->data_size);
}

int32
cdb_sync_tbl_full_add_common(uint32 tbl_id)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    tbl_iter_args_t args;
    args.argv[0] = p_node;

    p_node->tbl_info->ops.iter_fn(_cdb_sync_tbl_full_add_common_one, &args);

    return PM_E_NONE;
}

int32
cdb_sync_tbl_full_set_common(uint32 tbl_id)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    tbl_iter_args_t args;
    uint32 field_id = 0;
    
    args.argv[0] = p_node;
    args.argv[1] = &field_id;

    for (field_id = 0; field_id < p_node->tbl_info->field_num; field_id++)
    {
        p_node->tbl_info->ops.iter_fn(_cdb_sync_tbl_full_set_common_one, &args);
    }

    return PM_E_NONE;
}

int32
cdb_sync_tbl_full(uint32 tbl_id)
{
    switch (tbl_id)
    {
    case TBL_SYS_SPEC:
        return cdb_sync_tbl_full_add_common(tbl_id);

    default:
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}
