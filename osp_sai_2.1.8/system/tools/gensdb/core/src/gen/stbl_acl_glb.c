
#include "sdb.h"
#include "gen/stbl_acl_glb_define.h"
#include "gen/stbl_acl_glb.h"
#include "sdb_data_cmp.h"

stbl_acl_glb_t *_g_p_stbl_acl_glb = NULL;

int32
stbl_acl_glb_set_acl_glb_field_sync(stbl_acl_glb_t *p_acl_glb, stbl_acl_glb_field_id_t field_id, uint32 sync)
{
    stbl_acl_glb_t *p_db_acl_glb = _g_p_stbl_acl_glb;

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ACL_GLB_FLD_ACL_TBL_MAX_COUNT:
        p_db_acl_glb->acl_tbl_max_count = p_acl_glb->acl_tbl_max_count;
        break;

    case STBL_ACL_GLB_FLD_ACL_ENTRY_MAX_COUNT:
        p_db_acl_glb->acl_entry_max_count = p_acl_glb->acl_entry_max_count;
        break;

    case STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_MAX_COUNT:
        p_db_acl_glb->sdk_acl_entry_max_count = p_acl_glb->sdk_acl_entry_max_count;
        break;

    case STBL_ACL_GLB_FLD_TUNNEL_MAX_COUNT:
        p_db_acl_glb->tunnel_max_count = p_acl_glb->tunnel_max_count;
        break;

    case STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB_MAX_COUNT:
        p_db_acl_glb->erspan_service_opb_max_count = p_acl_glb->erspan_service_opb_max_count;
        break;

    case STBL_ACL_GLB_FLD_ACL_TBL_OPB:
        sal_memcpy(&p_db_acl_glb->acl_tbl_opb, &p_acl_glb->acl_tbl_opb, sizeof(p_acl_glb->acl_tbl_opb));
        break;

    case STBL_ACL_GLB_FLD_ACL_ENTRY_OPB:
        sal_memcpy(&p_db_acl_glb->acl_entry_opb, &p_acl_glb->acl_entry_opb, sizeof(p_acl_glb->acl_entry_opb));
        break;

    case STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_OPB:
        sal_memcpy(&p_db_acl_glb->sdk_acl_entry_opb, &p_acl_glb->sdk_acl_entry_opb, sizeof(p_acl_glb->sdk_acl_entry_opb));
        break;

    case STBL_ACL_GLB_FLD_TUNNEL_OPB:
        sal_memcpy(&p_db_acl_glb->tunnel_opb, &p_acl_glb->tunnel_opb, sizeof(p_acl_glb->tunnel_opb));
        break;

    case STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB:
        sal_memcpy(&p_db_acl_glb->erspan_service_opb, &p_acl_glb->erspan_service_opb, sizeof(p_acl_glb->erspan_service_opb));
        break;

    case STBL_ACL_GLB_FLD_MAX:
        sal_memcpy(p_db_acl_glb, p_acl_glb, sizeof(stbl_acl_glb_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_glb_set_acl_glb_field(stbl_acl_glb_t *p_acl_glb, stbl_acl_glb_field_id_t field_id)
{
    return stbl_acl_glb_set_acl_glb_field_sync(p_acl_glb, field_id, TRUE);
}

stbl_acl_glb_t*
stbl_acl_glb_get_acl_glb()
{
    return _g_p_stbl_acl_glb;
}

int32
stbl_acl_glb_key_str2val(char *str, stbl_acl_glb_t *p_acl_glb)
{
    return SDB_E_NONE;
}

int32
stbl_acl_glb_dump_one(stbl_acl_glb_t *p_acl_glb, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ACL_GLB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_ACL_TBL_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_GLB_FLD_ACL_TBL_MAX_COUNT].name,
            p_acl_glb->acl_tbl_max_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_ACL_ENTRY_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_GLB_FLD_ACL_ENTRY_MAX_COUNT].name,
            p_acl_glb->acl_entry_max_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_MAX_COUNT].name,
            p_acl_glb->sdk_acl_entry_max_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_TUNNEL_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_GLB_FLD_TUNNEL_MAX_COUNT].name,
            p_acl_glb->tunnel_max_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB_MAX_COUNT].name,
            p_acl_glb->erspan_service_opb_max_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_ACL_TBL_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_GLB_FLD_ACL_TBL_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_acl_glb->acl_tbl_opb));
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_ACL_ENTRY_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_GLB_FLD_ACL_ENTRY_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_acl_glb->acl_entry_opb));
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_GLB_FLD_SDK_ACL_ENTRY_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_acl_glb->sdk_acl_entry_opb));
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_TUNNEL_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_GLB_FLD_TUNNEL_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_acl_glb->tunnel_opb));
    }
    if (SDB_FLD_MATCH(STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_GLB_FLD_ERSPAN_SERVICE_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_acl_glb->erspan_service_opb));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_acl_glb_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_acl_glb_t *p_db_acl_glb = _g_p_stbl_acl_glb;
    int32 rc = SDB_E_NONE;

    if (NULL == p_db_acl_glb)
    {
        return SDB_E_NONE;
    }

    rc = fn(p_db_acl_glb, pargs);
    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_acl_glb_t*
stbl_acl_glb_init_acl_glb()
{
    _g_p_stbl_acl_glb = XCALLOC(MEM_STBL_ACL_GLB, sizeof(stbl_acl_glb_t));
    return _g_p_stbl_acl_glb;
}

