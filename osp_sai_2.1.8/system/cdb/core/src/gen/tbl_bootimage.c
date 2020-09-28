
#include "proto.h"
#include "gen/tbl_bootimage_define.h"
#include "gen/tbl_bootimage.h"
#include "cdb_data_cmp.h"

tbl_bootimage_t *_g_p_tbl_bootimage = NULL;

int32
tbl_bootimage_set_bootimage_field_sync(tbl_bootimage_t *p_boot, tbl_bootimage_field_id_t field_id, uint32 sync)
{
    tbl_bootimage_t *p_db_boot = _g_p_tbl_bootimage;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_BOOTIMAGE_FLD_MODE:
        p_db_boot->mode = p_boot->mode;
        break;

    case TBL_BOOTIMAGE_FLD_FAST:
        p_db_boot->fast = p_boot->fast;
        break;

    case TBL_BOOTIMAGE_FLD_IMAGE:
        sal_memcpy(p_db_boot->image, p_boot->image, sizeof(p_boot->image));
        break;

    case TBL_BOOTIMAGE_FLD_SERVERIP:
        sal_memcpy(p_db_boot->serverip, p_boot->serverip, sizeof(p_boot->serverip));
        break;

    case TBL_BOOTIMAGE_FLD_IPADDR:
        sal_memcpy(p_db_boot->ipaddr, p_boot->ipaddr, sizeof(p_boot->ipaddr));
        break;

    case TBL_BOOTIMAGE_FLD_BOOTCMD:
        sal_memcpy(p_db_boot->bootcmd, p_boot->bootcmd, sizeof(p_boot->bootcmd));
        break;

    case TBL_BOOTIMAGE_FLD_CURRENT_IMAGE:
        sal_memcpy(p_db_boot->current_image, p_boot->current_image, sizeof(p_boot->current_image));
        break;

    case TBL_BOOTIMAGE_FLD_MAX:
        sal_memcpy(p_db_boot, p_boot, sizeof(tbl_bootimage_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_BOOTIMAGE, field_id, p_db_boot);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_BOOTIMAGE, field_id, p_db_boot);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bootimage_set_bootimage_field(tbl_bootimage_t *p_boot, tbl_bootimage_field_id_t field_id)
{
    return tbl_bootimage_set_bootimage_field_sync(p_boot, field_id, TRUE);
}

tbl_bootimage_t*
tbl_bootimage_get_bootimage()
{
    return _g_p_tbl_bootimage;
}

int32
tbl_bootimage_key_str2val(char *str, tbl_bootimage_t *p_boot)
{
    return PM_E_NONE;
}

char*
tbl_bootimage_key_name_dump(tbl_bootimage_t *p_boot, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BOOTIMAGE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_bootimage_key_value_dump(tbl_bootimage_t *p_boot, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_bootimage_field_name_dump(tbl_bootimage_t *p_boot, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_BOOTIMAGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_BOOTIMAGE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_bootimage_field_value_dump(tbl_bootimage_t *p_boot, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(boot_mode_strs, BOOT_MODE_MAX, p_boot->mode));
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_FAST, field_id))
    {
        sal_sprintf(str, "%u", p_boot->fast);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_IMAGE, field_id))
    {
        sal_sprintf(str, "%s", p_boot->image);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_SERVERIP, field_id))
    {
        sal_sprintf(str, "%s", p_boot->serverip);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_IPADDR, field_id))
    {
        sal_sprintf(str, "%s", p_boot->ipaddr);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_BOOTCMD, field_id))
    {
        sal_sprintf(str, "%s", p_boot->bootcmd);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_CURRENT_IMAGE, field_id))
    {
        sal_sprintf(str, "%s", p_boot->current_image);
    }
    return str;
}

char**
tbl_bootimage_table_dump(tbl_bootimage_t *p_boot, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_bootimage_key_name_dump(p_boot, buf));
    for(i=1; i<TBL_BOOTIMAGE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_bootimage_field_name_dump(p_boot, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_bootimage_field_value_dump(p_boot, i, buf));
    }
    return str;
}

int32
tbl_bootimage_field_value_parser(char *str, int32 field_id, tbl_bootimage_t *p_boot)
{
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_MODE, field_id))
    {
        cdb_enum_str2val(boot_mode_strs, BOOT_MODE_MAX, str);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_FAST, field_id))
    {
        int32 ret;
        p_boot->fast = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_IMAGE, field_id))
    {
        sal_strcpy(p_boot->image, str);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_SERVERIP, field_id))
    {
        sal_strcpy(p_boot->serverip, str);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_IPADDR, field_id))
    {
        sal_strcpy(p_boot->ipaddr, str);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_BOOTCMD, field_id))
    {
        sal_strcpy(p_boot->bootcmd, str);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_CURRENT_IMAGE, field_id))
    {
        sal_strcpy(p_boot->current_image, str);
    }
    return PM_E_NONE;
}

int32
tbl_bootimage_table_parser(char** array, char* key_value,tbl_bootimage_t *p_boot)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_BOOTIMAGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_bootimage_key_str2val(key_value, p_boot));

    for(i=1; i<TBL_BOOTIMAGE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_bootimage_field_value_parser( array[j++], i, p_boot));
    }

    return PM_E_NONE;
}

int32
tbl_bootimage_dump_one(tbl_bootimage_t *p_boot, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BOOTIMAGE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BOOTIMAGE_FLD_MODE].name, 
            cdb_enum_val2str(boot_mode_strs, BOOT_MODE_MAX, p_boot->mode));
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_FAST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BOOTIMAGE_FLD_FAST].name,
            p_boot->fast);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_IMAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BOOTIMAGE_FLD_IMAGE].name,
            p_boot->image);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_SERVERIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BOOTIMAGE_FLD_SERVERIP].name,
            p_boot->serverip);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_IPADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BOOTIMAGE_FLD_IPADDR].name,
            p_boot->ipaddr);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_BOOTCMD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BOOTIMAGE_FLD_BOOTCMD].name,
            p_boot->bootcmd);
    }
    if (FLD_MATCH(TBL_BOOTIMAGE_FLD_CURRENT_IMAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BOOTIMAGE_FLD_CURRENT_IMAGE].name,
            p_boot->current_image);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_bootimage_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_bootimage_t *p_db_boot = _g_p_tbl_bootimage;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_boot)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_boot, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_bootimage_t*
tbl_bootimage_init_bootimage()
{
    _g_p_tbl_bootimage = XCALLOC(MEM_TBL_BOOTIMAGE, sizeof(tbl_bootimage_t));
    return _g_p_tbl_bootimage;
}

