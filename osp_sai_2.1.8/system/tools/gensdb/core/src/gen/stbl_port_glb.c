
#include "sdb.h"
#include "gen/stbl_port_glb_define.h"
#include "gen/stbl_port_glb.h"
#include "sdb_data_cmp.h"

stbl_port_glb_t *_g_p_stbl_port_glb = NULL;

int32
stbl_port_glb_set_port_glb_field_sync(stbl_port_glb_t *p_port_glb, stbl_port_glb_field_id_t field_id, uint32 sync)
{
    stbl_port_glb_t *p_db_port_glb = _g_p_stbl_port_glb;

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_PORT_GLB_FLD_GROUP_ID_IGS_OPB:
        sal_memcpy(&p_db_port_glb->group_id_igs_opb, &p_port_glb->group_id_igs_opb, sizeof(p_port_glb->group_id_igs_opb));
        break;

    case STBL_PORT_GLB_FLD_GROUP_ID_EGS_OPB:
        sal_memcpy(&p_db_port_glb->group_id_egs_opb, &p_port_glb->group_id_egs_opb, sizeof(p_port_glb->group_id_egs_opb));
        break;

    case STBL_PORT_GLB_FLD_TTP_GROUP_ID_IGS_OPB:
        sal_memcpy(&p_db_port_glb->ttp_group_id_igs_opb, &p_port_glb->ttp_group_id_igs_opb, sizeof(p_port_glb->ttp_group_id_igs_opb));
        break;

    case STBL_PORT_GLB_FLD_PORT_CLASS_ID_OPB:
        sal_memcpy(&p_db_port_glb->port_class_id_opb, &p_port_glb->port_class_id_opb, sizeof(p_port_glb->port_class_id_opb));
        break;

    case STBL_PORT_GLB_FLD_HOSTIF_ID_OPB:
        sal_memcpy(&p_db_port_glb->hostif_id_opb, &p_port_glb->hostif_id_opb, sizeof(p_port_glb->hostif_id_opb));
        break;

    case STBL_PORT_GLB_FLD_HOSTIF_GROUP_ID_OPB:
        sal_memcpy(&p_db_port_glb->hostif_group_id_opb, &p_port_glb->hostif_group_id_opb, sizeof(p_port_glb->hostif_group_id_opb));
        break;

    case STBL_PORT_GLB_FLD_ROUTE_IF_OPB:
        sal_memcpy(&p_db_port_glb->route_if_opb, &p_port_glb->route_if_opb, sizeof(p_port_glb->route_if_opb));
        break;

    case STBL_PORT_GLB_FLD_MAX:
        sal_memcpy(p_db_port_glb, p_port_glb, sizeof(stbl_port_glb_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_port_glb_set_port_glb_field(stbl_port_glb_t *p_port_glb, stbl_port_glb_field_id_t field_id)
{
    return stbl_port_glb_set_port_glb_field_sync(p_port_glb, field_id, TRUE);
}

stbl_port_glb_t*
stbl_port_glb_get_port_glb()
{
    return _g_p_stbl_port_glb;
}

int32
stbl_port_glb_key_str2val(char *str, stbl_port_glb_t *p_port_glb)
{
    return SDB_E_NONE;
}

int32
stbl_port_glb_dump_one(stbl_port_glb_t *p_port_glb, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_PORT_GLB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_GROUP_ID_IGS_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_GROUP_ID_IGS_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->group_id_igs_opb));
    }
    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_GROUP_ID_EGS_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_GROUP_ID_EGS_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->group_id_egs_opb));
    }
    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_TTP_GROUP_ID_IGS_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_TTP_GROUP_ID_IGS_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->ttp_group_id_igs_opb));
    }
    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_PORT_CLASS_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_PORT_CLASS_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->port_class_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_HOSTIF_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_HOSTIF_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->hostif_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_HOSTIF_GROUP_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_HOSTIF_GROUP_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->hostif_group_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_PORT_GLB_FLD_ROUTE_IF_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_GLB_FLD_ROUTE_IF_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port_glb->route_if_opb));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_port_glb_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_port_glb_t *p_db_port_glb = _g_p_stbl_port_glb;
    int32 rc = SDB_E_NONE;

    if (NULL == p_db_port_glb)
    {
        return SDB_E_NONE;
    }

    rc = fn(p_db_port_glb, pargs);
    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_port_glb_t*
stbl_port_glb_init_port_glb()
{
    _g_p_stbl_port_glb = XCALLOC(MEM_STBL_PORT_GLB, sizeof(stbl_port_glb_t));
    return _g_p_stbl_port_glb;
}

