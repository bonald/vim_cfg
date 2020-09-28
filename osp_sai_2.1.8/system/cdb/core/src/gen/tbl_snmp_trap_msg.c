
#include "proto.h"
#include "gen/tbl_snmp_trap_msg_define.h"
#include "gen/tbl_snmp_trap_msg.h"
#include "cdb_data_cmp.h"

tbl_snmp_trap_msg_t *_g_p_tbl_snmp_trap_msg = NULL;

int32
tbl_snmp_trap_msg_set_snmp_trap_msg_field_sync(tbl_snmp_trap_msg_t *p_trap, tbl_snmp_trap_msg_field_id_t field_id, uint32 sync)
{
    tbl_snmp_trap_msg_t *p_db_trap = _g_p_tbl_snmp_trap_msg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_TRAP_MSG_FLD_MSG:
        sal_memcpy(p_db_trap->msg, p_trap->msg, sizeof(p_trap->msg));
        break;

    case TBL_SNMP_TRAP_MSG_FLD_MAX:
        sal_memcpy(p_db_trap, p_trap, sizeof(tbl_snmp_trap_msg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_TRAP_MSG, field_id, p_db_trap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_TRAP_MSG, field_id, p_db_trap);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_trap_msg_set_snmp_trap_msg_field(tbl_snmp_trap_msg_t *p_trap, tbl_snmp_trap_msg_field_id_t field_id)
{
    return tbl_snmp_trap_msg_set_snmp_trap_msg_field_sync(p_trap, field_id, TRUE);
}

tbl_snmp_trap_msg_t*
tbl_snmp_trap_msg_get_snmp_trap_msg()
{
    return _g_p_tbl_snmp_trap_msg;
}

int32
tbl_snmp_trap_msg_key_str2val(char *str, tbl_snmp_trap_msg_t *p_trap)
{
    return PM_E_NONE;
}

char*
tbl_snmp_trap_msg_key_name_dump(tbl_snmp_trap_msg_t *p_trap, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP_MSG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_snmp_trap_msg_key_value_dump(tbl_snmp_trap_msg_t *p_trap, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_snmp_trap_msg_field_name_dump(tbl_snmp_trap_msg_t *p_trap, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP_MSG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_TRAP_MSG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_trap_msg_field_value_dump(tbl_snmp_trap_msg_t *p_trap, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_TRAP_MSG_FLD_MSG, field_id))
    {
        sal_sprintf(str, "%s", p_trap->msg);
    }
    return str;
}

char**
tbl_snmp_trap_msg_table_dump(tbl_snmp_trap_msg_t *p_trap, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_trap_msg_key_name_dump(p_trap, buf));
    for(i=1; i<TBL_SNMP_TRAP_MSG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_trap_msg_field_name_dump(p_trap, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_trap_msg_field_value_dump(p_trap, i, buf));
    }
    return str;
}

int32
tbl_snmp_trap_msg_field_value_parser(char *str, int32 field_id, tbl_snmp_trap_msg_t *p_trap)
{
    if (FLD_MATCH(TBL_SNMP_TRAP_MSG_FLD_MSG, field_id))
    {
        sal_strcpy(p_trap->msg, str);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_trap_msg_table_parser(char** array, char* key_value,tbl_snmp_trap_msg_t *p_trap)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP_MSG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_trap_msg_key_str2val(key_value, p_trap));

    for(i=1; i<TBL_SNMP_TRAP_MSG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_trap_msg_field_value_parser( array[j++], i, p_trap));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_trap_msg_dump_one(tbl_snmp_trap_msg_t *p_trap, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP_MSG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SNMP_TRAP_MSG_FLD_MSG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TRAP_MSG_FLD_MSG].name,
            p_trap->msg);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_trap_msg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_trap_msg_t *p_db_trap = _g_p_tbl_snmp_trap_msg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_trap)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_trap, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_trap_msg_t*
tbl_snmp_trap_msg_init_snmp_trap_msg()
{
    _g_p_tbl_snmp_trap_msg = XCALLOC(MEM_TBL_SNMP_TRAP_MSG, sizeof(tbl_snmp_trap_msg_t));
    return _g_p_tbl_snmp_trap_msg;
}

