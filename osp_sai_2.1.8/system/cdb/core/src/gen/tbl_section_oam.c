
#include "proto.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "cdb_data_cmp.h"

tbl_section_oam_master_t *_g_p_tbl_section_oam_master = NULL;

static uint32
_tbl_section_oam_hash_make(tbl_section_oam_t *p_section_oam)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_section_oam->key;
    for (index = 0; index < sizeof(p_section_oam->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_section_oam_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_section_oam_t *p_section_oam1 = (tbl_section_oam_t*)p_arg1;
    tbl_section_oam_t *p_section_oam2 = (tbl_section_oam_t*)p_arg2;

    if (0 == sal_memcmp(&p_section_oam1->key, &p_section_oam2->key, sizeof(tbl_section_oam_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_section_oam_add_section_oam_sync(tbl_section_oam_t *p_section_oam, uint32 sync)
{
    tbl_section_oam_master_t *p_master = _g_p_tbl_section_oam_master;
    tbl_section_oam_t *p_db_section_oam = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_section_oam_get_section_oam(&p_section_oam->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_section_oam = XCALLOC(MEM_TBL_SECTION_OAM, sizeof(tbl_section_oam_t));
    if (NULL == p_db_section_oam)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_section_oam, p_section_oam, sizeof(tbl_section_oam_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->section_oam_hash, (void*)p_db_section_oam, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->section_oam_list, p_db_section_oam);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SECTION_OAM, p_db_section_oam);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SECTION_OAM, p_db_section_oam);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_section_oam_del_section_oam_sync(tbl_section_oam_key_t *p_section_oam_key, uint32 sync)
{
    tbl_section_oam_master_t *p_master = _g_p_tbl_section_oam_master;
    tbl_section_oam_t *p_db_section_oam = NULL;

    /* 1. lookup entry exist */
    p_db_section_oam = tbl_section_oam_get_section_oam(p_section_oam_key);
    if (NULL == p_db_section_oam)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SECTION_OAM, p_db_section_oam);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SECTION_OAM, p_db_section_oam);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->section_oam_hash, (void*)p_db_section_oam);
    ctclib_slistnode_delete(p_master->section_oam_list, p_db_section_oam);

    /* 4. free db entry */
    XFREE(MEM_TBL_SECTION_OAM, p_db_section_oam);

    return PM_E_NONE;
}

int32
tbl_section_oam_set_section_oam_field_sync(tbl_section_oam_t *p_section_oam, tbl_section_oam_field_id_t field_id, uint32 sync)
{
    tbl_section_oam_t *p_db_section_oam = NULL;

    /* 1. lookup entry exist */
    p_db_section_oam = tbl_section_oam_get_section_oam(&p_section_oam->key);
    if (NULL == p_db_section_oam)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SECTION_OAM_FLD_OAM_SESSION_ID:
        p_db_section_oam->oam_session_id = p_section_oam->oam_session_id;
        break;

    case TBL_SECTION_OAM_FLD_NEXTHOP_MAC:
        sal_memcpy(p_db_section_oam->nexthop_mac, p_section_oam->nexthop_mac, sizeof(p_section_oam->nexthop_mac));
        break;

    case TBL_SECTION_OAM_FLD_MAX:
        sal_memcpy(p_db_section_oam, p_section_oam, sizeof(tbl_section_oam_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SECTION_OAM, field_id, p_db_section_oam);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SECTION_OAM, field_id, p_db_section_oam);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_section_oam_add_section_oam(tbl_section_oam_t *p_section_oam)
{
    return tbl_section_oam_add_section_oam_sync(p_section_oam, TRUE);
}

int32
tbl_section_oam_del_section_oam(tbl_section_oam_key_t *p_section_oam_key)
{
    return tbl_section_oam_del_section_oam_sync(p_section_oam_key, TRUE);
}

int32
tbl_section_oam_set_section_oam_field(tbl_section_oam_t *p_section_oam, tbl_section_oam_field_id_t field_id)
{
    return tbl_section_oam_set_section_oam_field_sync(p_section_oam, field_id, TRUE);
}

tbl_section_oam_t*
tbl_section_oam_get_section_oam(tbl_section_oam_key_t *p_section_oam_key)
{
    tbl_section_oam_master_t *p_master = _g_p_tbl_section_oam_master;
    tbl_section_oam_t lkp;

    sal_memcpy(&lkp.key, p_section_oam_key, sizeof(tbl_section_oam_key_t));
    return ctclib_hash_lookup(p_master->section_oam_hash, &lkp);
}

char*
tbl_section_oam_key_val2str(tbl_section_oam_t *p_section_oam, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_section_oam->key.ifindex);
    return str;
}

int32
tbl_section_oam_key_str2val(char *str, tbl_section_oam_t *p_section_oam)
{
    int32 ret = 0;

    p_section_oam->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_section_oam_key_name_dump(tbl_section_oam_t *p_section_oam, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SECTION_OAM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_section_oam->key.ifindex);
    return str;
}

char*
tbl_section_oam_key_value_dump(tbl_section_oam_t *p_section_oam, char *str)
{
    sal_sprintf(str, "%u", p_section_oam->key.ifindex);
    return str;
}

char*
tbl_section_oam_field_name_dump(tbl_section_oam_t *p_section_oam, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SECTION_OAM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SECTION_OAM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_section_oam_field_value_dump(tbl_section_oam_t *p_section_oam, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_section_oam->key.ifindex);
    }
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_OAM_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_section_oam->oam_session_id);
    }
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_section_oam->nexthop_mac);
    }
    return str;
}

char**
tbl_section_oam_table_dump(tbl_section_oam_t *p_section_oam, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_section_oam_key_name_dump(p_section_oam, buf));
    for(i=1; i<TBL_SECTION_OAM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_section_oam_field_name_dump(p_section_oam, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_section_oam_field_value_dump(p_section_oam, i, buf));
    }
    return str;
}

int32
tbl_section_oam_field_value_parser(char *str, int32 field_id, tbl_section_oam_t *p_section_oam)
{
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_KEY, field_id))
    {
        int32 ret;
        p_section_oam->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_OAM_SESSION_ID, field_id))
    {
        int32 ret;
        p_section_oam->oam_session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_section_oam->nexthop_mac, str);
    }
    return PM_E_NONE;
}

int32
tbl_section_oam_table_parser(char** array, char* key_value,tbl_section_oam_t *p_section_oam)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SECTION_OAM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_section_oam_key_str2val(key_value, p_section_oam));

    for(i=1; i<TBL_SECTION_OAM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_section_oam_field_value_parser( array[j++], i, p_section_oam));
    }

    return PM_E_NONE;
}

int32
tbl_section_oam_dump_one(tbl_section_oam_t *p_section_oam, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SECTION_OAM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SECTION_OAM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SECTION_OAM_FLD_KEY].name,
            p_section_oam->key.ifindex);
    }
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_OAM_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SECTION_OAM_FLD_OAM_SESSION_ID].name,
            p_section_oam->oam_session_id);
    }
    if (FLD_MATCH(TBL_SECTION_OAM_FLD_NEXTHOP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SECTION_OAM_FLD_NEXTHOP_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_section_oam->nexthop_mac));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_section_oam_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_section_oam_master_t *p_master = _g_p_tbl_section_oam_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_section_oam_t *p_db_section_oam = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->section_oam_list, p_db_section_oam, listnode, next)
    {
        rc |= fn(p_db_section_oam, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_section_oam_master_t*
tbl_section_oam_get_master()
{
    return _g_p_tbl_section_oam_master;
}

tbl_section_oam_master_t*
tbl_section_oam_init_section_oam()
{
    _g_p_tbl_section_oam_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_section_oam_master_t));
    _g_p_tbl_section_oam_master->section_oam_hash = ctclib_hash_create(_tbl_section_oam_hash_make, _tbl_section_oam_hash_cmp);
    _g_p_tbl_section_oam_master->section_oam_list = ctclib_slist_create(tbl_section_oam_cmp, NULL);
    return _g_p_tbl_section_oam_master;
}

