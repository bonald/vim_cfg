
#include "proto.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_mirror.h"
#include "cdb_data_cmp.h"

tbl_mirror_master_t *_g_p_tbl_mirror_master = NULL;

#include "mirror_define.h"

const char *mirror_dest_type_strs[MIRROR_DEST_TYPE_MAX] =
{
    "MIRROR_DEST_NONE",
    "MIRROR_DEST_VLAN",
    "MIRROR_DEST_PORT",
    "MIRROR_DEST_GROUP",
    "MIRROR_DEST_CPU"
};


int32
tbl_mirror_add_mirror_sync(tbl_mirror_t *p_mirror, uint32 sync)
{
    tbl_mirror_master_t *p_master = _g_p_tbl_mirror_master;
    tbl_mirror_t *p_db_mirror = NULL;

    /* 1. lookup entry exist */
    if (tbl_mirror_get_mirror(&p_mirror->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_mirror = XCALLOC(MEM_TBL_MIRROR, sizeof(tbl_mirror_t));
    if (NULL == p_db_mirror)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_mirror, p_mirror, sizeof(tbl_mirror_t));

    /* 4. add to db */
    p_master->mirror_array[p_mirror->key.id] = p_db_mirror;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MIRROR, p_db_mirror);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MIRROR, p_db_mirror);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mirror_del_mirror_sync(tbl_mirror_key_t *p_mirror_key, uint32 sync)
{
    tbl_mirror_master_t *p_master = _g_p_tbl_mirror_master;
    tbl_mirror_t *p_db_mirror = NULL;

    /* 1. lookup entry exist */
    p_db_mirror = tbl_mirror_get_mirror(p_mirror_key);
    if (NULL == p_db_mirror)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MIRROR, p_db_mirror);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MIRROR, p_db_mirror);
        #endif 
    }

    /* 3. delete from db */
    p_master->mirror_array[p_mirror_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_MIRROR, p_db_mirror);

    return PM_E_NONE;
}

int32
tbl_mirror_set_mirror_field_sync(tbl_mirror_t *p_mirror, tbl_mirror_field_id_t field_id, uint32 sync)
{
    tbl_mirror_t *p_db_mirror = NULL;

    /* 1. lookup entry exist */
    p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key);
    if (NULL == p_db_mirror)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP:
        sal_memcpy(p_db_mirror->source_igs_port_bmp, p_mirror->source_igs_port_bmp, sizeof(p_mirror->source_igs_port_bmp));
        break;

    case TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP:
        sal_memcpy(p_db_mirror->source_egs_port_bmp, p_mirror->source_egs_port_bmp, sizeof(p_mirror->source_egs_port_bmp));
        break;

    case TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP:
        sal_memcpy(p_db_mirror->source_igs_vlan_bmp, p_mirror->source_igs_vlan_bmp, sizeof(p_mirror->source_igs_vlan_bmp));
        break;

    case TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP:
        sal_memcpy(p_db_mirror->source_egs_vlan_bmp, p_mirror->source_egs_vlan_bmp, sizeof(p_mirror->source_egs_vlan_bmp));
        break;

    case TBL_MIRROR_FLD_CPU_SOURCE_DIR:
        p_db_mirror->cpu_source_dir = p_mirror->cpu_source_dir;
        break;

    case TBL_MIRROR_FLD_DEST_TYPE:
        p_db_mirror->dest_type = p_mirror->dest_type;
        break;

    case TBL_MIRROR_FLD_DEST_PORT_BMP:
        sal_memcpy(p_db_mirror->dest_port_bmp, p_mirror->dest_port_bmp, sizeof(p_mirror->dest_port_bmp));
        break;

    case TBL_MIRROR_FLD_DEST_PORT:
        p_db_mirror->dest_port = p_mirror->dest_port;
        break;

    case TBL_MIRROR_FLD_DEST_GROUP:
        p_db_mirror->dest_group = p_mirror->dest_group;
        break;

    case TBL_MIRROR_FLD_DEST_VLAN:
        p_db_mirror->dest_vlan = p_mirror->dest_vlan;
        break;

    case TBL_MIRROR_FLD_MAX:
        sal_memcpy(p_db_mirror, p_mirror, sizeof(tbl_mirror_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MIRROR, field_id, p_db_mirror);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MIRROR, field_id, p_db_mirror);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mirror_add_mirror(tbl_mirror_t *p_mirror)
{
    return tbl_mirror_add_mirror_sync(p_mirror, TRUE);
}

int32
tbl_mirror_del_mirror(tbl_mirror_key_t *p_mirror_key)
{
    return tbl_mirror_del_mirror_sync(p_mirror_key, TRUE);
}

int32
tbl_mirror_set_mirror_field(tbl_mirror_t *p_mirror, tbl_mirror_field_id_t field_id)
{
    return tbl_mirror_set_mirror_field_sync(p_mirror, field_id, TRUE);
}

tbl_mirror_t*
tbl_mirror_get_mirror(tbl_mirror_key_t *p_mirror_key)
{
    tbl_mirror_master_t *p_master = _g_p_tbl_mirror_master;

    if (GLB_MIRROR_MAX_SESSION_ID+1 <= p_mirror_key->id)
    {
        return NULL;
    }
    return (p_master->mirror_array[p_mirror_key->id]);
}

char*
tbl_mirror_key_val2str(tbl_mirror_t *p_mirror, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_mirror->key.id);
    return str;
}

int32
tbl_mirror_key_str2val(char *str, tbl_mirror_t *p_mirror)
{
    int32 ret = 0;

    p_mirror->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_mirror_key_name_dump(tbl_mirror_t *p_mirror, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_mirror->key.id);
    return str;
}

char*
tbl_mirror_key_value_dump(tbl_mirror_t *p_mirror, char *str)
{
    sal_sprintf(str, "%u", p_mirror->key.id);
    return str;
}

char*
tbl_mirror_field_name_dump(tbl_mirror_t *p_mirror, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MIRROR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mirror_field_value_dump(tbl_mirror_t *p_mirror, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MIRROR_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_mirror->key.id);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mirror->source_igs_port_bmp, sizeof(p_mirror->source_igs_port_bmp)/4);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mirror->source_egs_port_bmp, sizeof(p_mirror->source_egs_port_bmp)/4);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mirror->source_igs_vlan_bmp, sizeof(p_mirror->source_igs_vlan_bmp)/4);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mirror->source_egs_vlan_bmp, sizeof(p_mirror->source_egs_vlan_bmp)/4);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_CPU_SOURCE_DIR, field_id))
    {
        sal_sprintf(str, "%u", p_mirror->cpu_source_dir);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mirror_dest_type_strs, MIRROR_DEST_TYPE_MAX, p_mirror->dest_type));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_PORT_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mirror->dest_port_bmp, sizeof(p_mirror->dest_port_bmp)/4);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_mirror->dest_port);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_GROUP, field_id))
    {
        sal_sprintf(str, "%u", p_mirror->dest_group);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_mirror->dest_vlan);
    }
    return str;
}

char**
tbl_mirror_table_dump(tbl_mirror_t *p_mirror, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mirror_key_name_dump(p_mirror, buf));
    for(i=1; i<TBL_MIRROR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mirror_field_name_dump(p_mirror, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mirror_field_value_dump(p_mirror, i, buf));
    }
    return str;
}

int32
tbl_mirror_field_value_parser(char *str, int32 field_id, tbl_mirror_t *p_mirror)
{
    if (FLD_MATCH(TBL_MIRROR_FLD_KEY, field_id))
    {
        int32 ret;
        p_mirror->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mirror->source_igs_port_bmp, sizeof(p_mirror->source_igs_port_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mirror->source_egs_port_bmp, sizeof(p_mirror->source_egs_port_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mirror->source_igs_vlan_bmp, sizeof(p_mirror->source_igs_vlan_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mirror->source_egs_vlan_bmp, sizeof(p_mirror->source_egs_vlan_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_CPU_SOURCE_DIR, field_id))
    {
        int32 ret;
        p_mirror->cpu_source_dir = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_TYPE, field_id))
    {
        cdb_enum_str2val(mirror_dest_type_strs, MIRROR_DEST_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_PORT_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mirror->dest_port_bmp, sizeof(p_mirror->dest_port_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_PORT, field_id))
    {
        int32 ret;
        p_mirror->dest_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_GROUP, field_id))
    {
        int32 ret;
        p_mirror->dest_group = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_VLAN, field_id))
    {
        int32 ret;
        p_mirror->dest_vlan = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_mirror_table_parser(char** array, char* key_value,tbl_mirror_t *p_mirror)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mirror_key_str2val(key_value, p_mirror));

    for(i=1; i<TBL_MIRROR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mirror_field_value_parser( array[j++], i, p_mirror));
    }

    return PM_E_NONE;
}

int32
tbl_mirror_dump_one(tbl_mirror_t *p_mirror, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MIRROR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MIRROR_FLD_KEY].name,
            p_mirror->key.id);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mirror->source_igs_port_bmp, sizeof(p_mirror->source_igs_port_bmp)/4));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mirror->source_egs_port_bmp, sizeof(p_mirror->source_egs_port_bmp)/4));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mirror->source_igs_vlan_bmp, sizeof(p_mirror->source_igs_vlan_bmp)/4));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mirror->source_egs_vlan_bmp, sizeof(p_mirror->source_egs_vlan_bmp)/4));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_CPU_SOURCE_DIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MIRROR_FLD_CPU_SOURCE_DIR].name,
            p_mirror->cpu_source_dir);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_FLD_DEST_TYPE].name, 
            cdb_enum_val2str(mirror_dest_type_strs, MIRROR_DEST_TYPE_MAX, p_mirror->dest_type));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_PORT_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_FLD_DEST_PORT_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mirror->dest_port_bmp, sizeof(p_mirror->dest_port_bmp)/4));
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MIRROR_FLD_DEST_PORT].name,
            p_mirror->dest_port);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_GROUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MIRROR_FLD_DEST_GROUP].name,
            p_mirror->dest_group);
    }
    if (FLD_MATCH(TBL_MIRROR_FLD_DEST_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MIRROR_FLD_DEST_VLAN].name,
            p_mirror->dest_vlan);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mirror_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mirror_master_t *p_master = _g_p_tbl_mirror_master;
    tbl_mirror_t *p_db_mirror = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_MIRROR_MAX_SESSION_ID+1; id++)
    {
        p_db_mirror = p_master->mirror_array[id];
        if (NULL == p_db_mirror)
        {
            continue;
        }
        rc |= fn(p_db_mirror, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mirror_master_t*
tbl_mirror_get_master()
{
    return _g_p_tbl_mirror_master;
}

tbl_mirror_master_t*
tbl_mirror_init_mirror()
{
    _g_p_tbl_mirror_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_mirror_master_t));
    return _g_p_tbl_mirror_master;
}


tbl_mirror_t*
mirror_get_session(uint32 sessionid)
{
    tbl_mirror_key_t mirror_key;

    mirror_key.id = sessionid;
    return tbl_mirror_get_mirror(&mirror_key);
}

