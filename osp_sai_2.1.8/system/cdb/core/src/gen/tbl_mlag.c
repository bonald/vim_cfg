
#include "proto.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "cdb_data_cmp.h"

tbl_mlag_t *_g_p_tbl_mlag = NULL;

int32
tbl_mlag_set_mlag_field_sync(tbl_mlag_t *p_mlag, tbl_mlag_field_id_t field_id, uint32 sync)
{
    tbl_mlag_t *p_db_mlag = _g_p_tbl_mlag;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MLAG_FLD_ENABLE:
        p_db_mlag->enable = p_mlag->enable;
        break;

    case TBL_MLAG_FLD_LOCAL_ADDR:
        sal_memcpy(&p_db_mlag->local_addr, &p_mlag->local_addr, sizeof(p_mlag->local_addr));
        break;

    case TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL:
        p_db_mlag->reload_delay_interval = p_mlag->reload_delay_interval;
        break;

    case TBL_MLAG_FLD_RELOAD_DELAY_AUTO:
        p_db_mlag->reload_delay_auto = p_mlag->reload_delay_auto;
        break;

    case TBL_MLAG_FLD_ROLE:
        p_db_mlag->role = p_mlag->role;
        break;

    case TBL_MLAG_FLD_LOCAL_SYSID:
        sal_memcpy(p_db_mlag->local_sysid, p_mlag->local_sysid, sizeof(p_mlag->local_sysid));
        break;

    case TBL_MLAG_FLD_REMOTE_SYSID:
        sal_memcpy(p_db_mlag->remote_sysid, p_mlag->remote_sysid, sizeof(p_mlag->remote_sysid));
        break;

    case TBL_MLAG_FLD_MLAG_SYSID:
        sal_memcpy(p_db_mlag->mlag_sysid, p_mlag->mlag_sysid, sizeof(p_mlag->mlag_sysid));
        break;

    case TBL_MLAG_FLD_PEER_ROUTEMAC:
        sal_memcpy(p_db_mlag->peer_routemac, p_mlag->peer_routemac, sizeof(p_mlag->peer_routemac));
        break;

    case TBL_MLAG_FLD_LOCAL_SYSPRI:
        p_db_mlag->local_syspri = p_mlag->local_syspri;
        break;

    case TBL_MLAG_FLD_REMOTE_SYSPRI:
        p_db_mlag->remote_syspri = p_mlag->remote_syspri;
        break;

    case TBL_MLAG_FLD_MLAG_SYSPRI:
        p_db_mlag->mlag_syspri = p_mlag->mlag_syspri;
        break;

    case TBL_MLAG_FLD_PEER_VLANIF_BMP:
        sal_memcpy(p_db_mlag->peer_vlanif_bmp, p_mlag->peer_vlanif_bmp, sizeof(p_mlag->peer_vlanif_bmp));
        break;

    case TBL_MLAG_FLD_PEER_FDB_BMP:
        sal_memcpy(p_db_mlag->peer_fdb_bmp, p_mlag->peer_fdb_bmp, sizeof(p_mlag->peer_fdb_bmp));
        break;

    case TBL_MLAG_FLD_PEER_LINK_IFINDEX:
        p_db_mlag->peer_link_ifindex = p_mlag->peer_link_ifindex;
        break;

    case TBL_MLAG_FLD_FLUSH_FDB_MI:
        p_db_mlag->flush_fdb_mi = p_mlag->flush_fdb_mi;
        break;

    case TBL_MLAG_FLD_RELOAD_DELAY_TIMER:
        p_db_mlag->reload_delay_timer = p_mlag->reload_delay_timer;
        break;

    case TBL_MLAG_FLD_MAX:
        sal_memcpy(p_db_mlag, p_mlag, sizeof(tbl_mlag_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MLAG, field_id, p_db_mlag);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MLAG, field_id, p_db_mlag);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mlag_set_mlag_field(tbl_mlag_t *p_mlag, tbl_mlag_field_id_t field_id)
{
    return tbl_mlag_set_mlag_field_sync(p_mlag, field_id, TRUE);
}

tbl_mlag_t*
tbl_mlag_get_mlag()
{
    return _g_p_tbl_mlag;
}

int32
tbl_mlag_key_str2val(char *str, tbl_mlag_t *p_mlag)
{
    return PM_E_NONE;
}

char*
tbl_mlag_key_name_dump(tbl_mlag_t *p_mlag, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_mlag_key_value_dump(tbl_mlag_t *p_mlag, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_mlag_field_name_dump(tbl_mlag_t *p_mlag, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MLAG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mlag_field_value_dump(tbl_mlag_t *p_mlag, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MLAG_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->enable);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_ADDR, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_mlag->local_addr);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->reload_delay_interval);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_AUTO, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->reload_delay_auto);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_ROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mlag_role_strs, MLAG_ROLE_MAX, p_mlag->role));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_SYSID, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_mlag->local_sysid);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_REMOTE_SYSID, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_mlag->remote_sysid);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_MLAG_SYSID, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_mlag->mlag_sysid);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_ROUTEMAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_mlag->peer_routemac);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_SYSPRI, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->local_syspri);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_REMOTE_SYSPRI, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->remote_syspri);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_MLAG_SYSPRI, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->mlag_syspri);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_VLANIF_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mlag->peer_vlanif_bmp, sizeof(p_mlag->peer_vlanif_bmp)/4);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_FDB_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mlag->peer_fdb_bmp, sizeof(p_mlag->peer_fdb_bmp)/4);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_LINK_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->peer_link_ifindex);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_FLUSH_FDB_MI, field_id))
    {
        sal_sprintf(str, "%u", p_mlag->flush_fdb_mi);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mlag->reload_delay_timer);
    }
    return str;
}

char**
tbl_mlag_table_dump(tbl_mlag_t *p_mlag, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mlag_key_name_dump(p_mlag, buf));
    for(i=1; i<TBL_MLAG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mlag_field_name_dump(p_mlag, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mlag_field_value_dump(p_mlag, i, buf));
    }
    return str;
}

int32
tbl_mlag_field_value_parser(char *str, int32 field_id, tbl_mlag_t *p_mlag)
{
    if (FLD_MATCH(TBL_MLAG_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_mlag->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_ADDR, field_id))
    {
        cdb_addr_str2val(&p_mlag->local_addr, str, 0);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL, field_id))
    {
        int32 ret;
        p_mlag->reload_delay_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_AUTO, field_id))
    {
        int32 ret;
        p_mlag->reload_delay_auto = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_ROLE, field_id))
    {
        cdb_enum_str2val(mlag_role_strs, MLAG_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_SYSID, field_id))
    {
        cdb_mac_addr_str2val(p_mlag->local_sysid, str);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_REMOTE_SYSID, field_id))
    {
        cdb_mac_addr_str2val(p_mlag->remote_sysid, str);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_MLAG_SYSID, field_id))
    {
        cdb_mac_addr_str2val(p_mlag->mlag_sysid, str);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_ROUTEMAC, field_id))
    {
        cdb_mac_addr_str2val(p_mlag->peer_routemac, str);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_SYSPRI, field_id))
    {
        int32 ret;
        p_mlag->local_syspri = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_REMOTE_SYSPRI, field_id))
    {
        int32 ret;
        p_mlag->remote_syspri = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_MLAG_SYSPRI, field_id))
    {
        int32 ret;
        p_mlag->mlag_syspri = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_VLANIF_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mlag->peer_vlanif_bmp, sizeof(p_mlag->peer_vlanif_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_FDB_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mlag->peer_fdb_bmp, sizeof(p_mlag->peer_fdb_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_LINK_IFINDEX, field_id))
    {
        int32 ret;
        p_mlag->peer_link_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_FLUSH_FDB_MI, field_id))
    {
        int32 ret;
        p_mlag->flush_fdb_mi = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mlag->reload_delay_timer);
    }
    return PM_E_NONE;
}

int32
tbl_mlag_table_parser(char** array, char* key_value,tbl_mlag_t *p_mlag)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mlag_key_str2val(key_value, p_mlag));

    for(i=1; i<TBL_MLAG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mlag_field_value_parser( array[j++], i, p_mlag));
    }

    return PM_E_NONE;
}

int32
tbl_mlag_dump_one(tbl_mlag_t *p_mlag, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MLAG_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_ENABLE].name,
            p_mlag->enable);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_LOCAL_ADDR].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_mlag->local_addr));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL].name,
            p_mlag->reload_delay_interval);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_AUTO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_RELOAD_DELAY_AUTO].name,
            p_mlag->reload_delay_auto);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_ROLE].name, 
            cdb_enum_val2str(mlag_role_strs, MLAG_ROLE_MAX, p_mlag->role));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_SYSID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_LOCAL_SYSID].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_mlag->local_sysid));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_REMOTE_SYSID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_REMOTE_SYSID].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_mlag->remote_sysid));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_MLAG_SYSID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_MLAG_SYSID].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_mlag->mlag_sysid));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_ROUTEMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_PEER_ROUTEMAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_mlag->peer_routemac));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_LOCAL_SYSPRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_LOCAL_SYSPRI].name,
            p_mlag->local_syspri);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_REMOTE_SYSPRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_REMOTE_SYSPRI].name,
            p_mlag->remote_syspri);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_MLAG_SYSPRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_MLAG_SYSPRI].name,
            p_mlag->mlag_syspri);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_VLANIF_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_PEER_VLANIF_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mlag->peer_vlanif_bmp, sizeof(p_mlag->peer_vlanif_bmp)/4));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_FDB_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_PEER_FDB_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mlag->peer_fdb_bmp, sizeof(p_mlag->peer_fdb_bmp)/4));
    }
    if (FLD_MATCH(TBL_MLAG_FLD_PEER_LINK_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_PEER_LINK_IFINDEX].name,
            p_mlag->peer_link_ifindex);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_FLUSH_FDB_MI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_FLD_FLUSH_FDB_MI].name,
            p_mlag->flush_fdb_mi);
    }
    if (FLD_MATCH(TBL_MLAG_FLD_RELOAD_DELAY_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_FLD_RELOAD_DELAY_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mlag->reload_delay_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mlag_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mlag_t *p_db_mlag = _g_p_tbl_mlag;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_mlag)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_mlag, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mlag_t*
tbl_mlag_init_mlag()
{
    _g_p_tbl_mlag = XCALLOC(MEM_TBL_MLAG, sizeof(tbl_mlag_t));
    return _g_p_tbl_mlag;
}

