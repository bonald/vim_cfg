
#include "proto.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "cdb_data_cmp.h"

tbl_chip_t *_g_p_tbl_chip = NULL;

int32
tbl_chip_set_chip_field_sync(tbl_chip_t *p_chip, tbl_chip_field_id_t field_id, uint32 sync)
{
    tbl_chip_t *p_db_chip = _g_p_tbl_chip;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CHIP_FLD_TYPE:
        p_db_chip->type = p_chip->type;
        break;

    case TBL_CHIP_FLD_MAX:
        sal_memcpy(p_db_chip, p_chip, sizeof(tbl_chip_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CHIP, field_id, p_db_chip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CHIP, field_id, p_db_chip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_chip_set_chip_field(tbl_chip_t *p_chip, tbl_chip_field_id_t field_id)
{
    return tbl_chip_set_chip_field_sync(p_chip, field_id, TRUE);
}

tbl_chip_t*
tbl_chip_get_chip()
{
    return _g_p_tbl_chip;
}

int32
tbl_chip_key_str2val(char *str, tbl_chip_t *p_chip)
{
    return PM_E_NONE;
}

char*
tbl_chip_key_name_dump(tbl_chip_t *p_chip, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHIP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_chip_key_value_dump(tbl_chip_t *p_chip, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_chip_field_name_dump(tbl_chip_t *p_chip, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CHIP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CHIP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_chip_field_value_dump(tbl_chip_t *p_chip, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CHIP_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_chip->type));
    }
    return str;
}

char**
tbl_chip_table_dump(tbl_chip_t *p_chip, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_chip_key_name_dump(p_chip, buf));
    for(i=1; i<TBL_CHIP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_chip_field_name_dump(p_chip, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_chip_field_value_dump(p_chip, i, buf));
    }
    return str;
}

int32
tbl_chip_field_value_parser(char *str, int32 field_id, tbl_chip_t *p_chip)
{
    if (FLD_MATCH(TBL_CHIP_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, str);
    }
    return PM_E_NONE;
}

int32
tbl_chip_table_parser(char** array, char* key_value,tbl_chip_t *p_chip)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHIP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_chip_key_str2val(key_value, p_chip));

    for(i=1; i<TBL_CHIP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_chip_field_value_parser( array[j++], i, p_chip));
    }

    return PM_E_NONE;
}

int32
tbl_chip_dump_one(tbl_chip_t *p_chip, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHIP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CHIP_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CHIP_FLD_TYPE].name, 
            cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_chip->type));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_chip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_chip_t *p_db_chip = _g_p_tbl_chip;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_chip)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_chip, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_chip_t*
tbl_chip_init_chip()
{
    _g_p_tbl_chip = XCALLOC(MEM_TBL_CHIP, sizeof(tbl_chip_t));
    return _g_p_tbl_chip;
}

uint32
tbl_chip_get_type()
{
    return _g_p_tbl_chip->type;
}

