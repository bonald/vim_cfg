
#include "proto.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_brg_if_define.h"
#include "gen/tbl_fea_brg_if.h"
#include "cdb_data_cmp.h"

tbl_fea_brg_if_master_t *_g_p_tbl_fea_brg_if_master = NULL;

static uint32
_tbl_fea_brg_if_hash_make(tbl_fea_brg_if_t *p_brgif)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_brgif->key;
    for (index = 0; index < sizeof(p_brgif->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_brg_if_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_brg_if_t *p_brgif1 = (tbl_fea_brg_if_t*)p_arg1;
    tbl_fea_brg_if_t *p_brgif2 = (tbl_fea_brg_if_t*)p_arg2;

    if (0 == sal_memcmp(&p_brgif1->key, &p_brgif2->key, sizeof(tbl_fea_brg_if_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_brg_if_add_fea_brg_if_sync(tbl_fea_brg_if_t *p_brgif, uint32 sync)
{
    tbl_fea_brg_if_master_t *p_master = _g_p_tbl_fea_brg_if_master;
    tbl_fea_brg_if_t *p_db_brgif = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_brg_if_get_fea_brg_if(&p_brgif->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_brgif = XCALLOC(MEM_TBL_FEA_BRG_IF, sizeof(tbl_fea_brg_if_t));
    if (NULL == p_db_brgif)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_brgif, p_brgif, sizeof(tbl_fea_brg_if_t));
    CTCLIB_TAILQ_INIT(&p_db_brgif->fdb_queue);

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->brgif_hash, (void*)p_db_brgif, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_BRG_IF, p_db_brgif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_BRG_IF, p_db_brgif);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_brg_if_del_fea_brg_if_sync(tbl_fea_brg_if_key_t *p_brgif_key, uint32 sync)
{
    tbl_fea_brg_if_master_t *p_master = _g_p_tbl_fea_brg_if_master;
    tbl_fea_brg_if_t *p_db_brgif = NULL;

    /* 1. lookup entry exist */
    p_db_brgif = tbl_fea_brg_if_get_fea_brg_if(p_brgif_key);
    if (NULL == p_db_brgif)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_BRG_IF, p_db_brgif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_BRG_IF, p_db_brgif);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->brgif_hash, (void*)p_db_brgif);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_BRG_IF, p_db_brgif);

    return PM_E_NONE;
}

int32
tbl_fea_brg_if_set_fea_brg_if_field_sync(tbl_fea_brg_if_t *p_brgif, tbl_fea_brg_if_field_id_t field_id, uint32 sync)
{
    tbl_fea_brg_if_t *p_db_brgif = NULL;

    /* 1. lookup entry exist */
    p_db_brgif = tbl_fea_brg_if_get_fea_brg_if(&p_brgif->key);
    if (NULL == p_db_brgif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_BRG_IF_FLD_FLAGS:
        p_db_brgif->flags = p_brgif->flags;
        break;

    case TBL_FEA_BRG_IF_FLD_LAG_ID:
        p_db_brgif->lag_id = p_brgif->lag_id;
        break;

    case TBL_FEA_BRG_IF_FLD_MAC_LEARNING_EN:
        p_db_brgif->mac_learning_en = p_brgif->mac_learning_en;
        break;

    case TBL_FEA_BRG_IF_FLD_PORT_SECURITY_EN:
        p_db_brgif->port_security_en = p_brgif->port_security_en;
        break;

    case TBL_FEA_BRG_IF_FLD_MAX_MAC:
        p_db_brgif->max_mac = p_brgif->max_mac;
        break;

    case TBL_FEA_BRG_IF_FLD_CURR_MAC:
        p_db_brgif->curr_mac = p_brgif->curr_mac;
        break;

    case TBL_FEA_BRG_IF_FLD_CURR_MAC_STATIC:
        p_db_brgif->curr_mac_static = p_brgif->curr_mac_static;
        break;

    case TBL_FEA_BRG_IF_FLD_MLAG_ID:
        p_db_brgif->mlag_id = p_brgif->mlag_id;
        break;

    case TBL_FEA_BRG_IF_FLD_MLAG_PEER_CONF:
        p_db_brgif->mlag_peer_conf = p_brgif->mlag_peer_conf;
        break;

    case TBL_FEA_BRG_IF_FLD_MLAG_PEER_IF_UP:
        p_db_brgif->mlag_peer_if_up = p_brgif->mlag_peer_if_up;
        break;

    case TBL_FEA_BRG_IF_FLD_MLAG_IS_GROUP:
        p_db_brgif->mlag_is_group = p_brgif->mlag_is_group;
        break;

    case TBL_FEA_BRG_IF_FLD_IS_MIRROR_DEST:
        p_db_brgif->is_mirror_dest = p_brgif->is_mirror_dest;
        break;

    case TBL_FEA_BRG_IF_FLD_HW_TYPE:
        p_db_brgif->hw_type = p_brgif->hw_type;
        break;

    case TBL_FEA_BRG_IF_FLD_PORTID:
        p_db_brgif->portid = p_brgif->portid;
        break;

    case TBL_FEA_BRG_IF_FLD_DISABLE_LEARN:
        sal_memcpy(p_db_brgif->disable_learn, p_brgif->disable_learn, sizeof(p_brgif->disable_learn));
        break;

    case TBL_FEA_BRG_IF_FLD_MAX:
        sal_memcpy(p_db_brgif, p_brgif, sizeof(tbl_fea_brg_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_BRG_IF, field_id, p_db_brgif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_BRG_IF, field_id, p_db_brgif);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_brg_if_add_fea_brg_if(tbl_fea_brg_if_t *p_brgif)
{
    return tbl_fea_brg_if_add_fea_brg_if_sync(p_brgif, TRUE);
}

int32
tbl_fea_brg_if_del_fea_brg_if(tbl_fea_brg_if_key_t *p_brgif_key)
{
    return tbl_fea_brg_if_del_fea_brg_if_sync(p_brgif_key, TRUE);
}

int32
tbl_fea_brg_if_set_fea_brg_if_field(tbl_fea_brg_if_t *p_brgif, tbl_fea_brg_if_field_id_t field_id)
{
    return tbl_fea_brg_if_set_fea_brg_if_field_sync(p_brgif, field_id, TRUE);
}

tbl_fea_brg_if_t*
tbl_fea_brg_if_get_fea_brg_if(tbl_fea_brg_if_key_t *p_brgif_key)
{
    tbl_fea_brg_if_master_t *p_master = _g_p_tbl_fea_brg_if_master;
    tbl_fea_brg_if_t lkp;

    sal_memcpy(&lkp.key, p_brgif_key, sizeof(tbl_fea_brg_if_key_t));
    return ctclib_hash_lookup(p_master->brgif_hash, &lkp);
}

char*
tbl_fea_brg_if_key_val2str(tbl_fea_brg_if_t *p_brgif, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_brgif->key.ifindex);
    return str;
}

int32
tbl_fea_brg_if_key_str2val(char *str, tbl_fea_brg_if_t *p_brgif)
{
    int32 ret = 0;

    p_brgif->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_brg_if_key_name_dump(tbl_fea_brg_if_t *p_brgif, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_BRG_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_brgif->key.ifindex);
    return str;
}

char*
tbl_fea_brg_if_key_value_dump(tbl_fea_brg_if_t *p_brgif, char *str)
{
    sal_sprintf(str, "%u", p_brgif->key.ifindex);
    return str;
}

char*
tbl_fea_brg_if_field_name_dump(tbl_fea_brg_if_t *p_brgif, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_BRG_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_BRG_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_brg_if_field_value_dump(tbl_fea_brg_if_t *p_brgif, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->key.ifindex);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->flags);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_LAG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->lag_id);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MAC_LEARNING_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mac_learning_en);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_PORT_SECURITY_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->port_security_en);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MAX_MAC, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->max_mac);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_CURR_MAC, field_id))
    {
        sal_sprintf(str, "%d", p_brgif->curr_mac);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_CURR_MAC_STATIC, field_id))
    {
        sal_sprintf(str, "%d", p_brgif->curr_mac_static);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_id);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_PEER_CONF, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_peer_conf);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_PEER_IF_UP, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_peer_if_up);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_IS_GROUP, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_is_group);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_IS_MIRROR_DEST, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->is_mirror_dest);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_HW_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_brgif->hw_type));
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_PORTID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_brgif->portid);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_DISABLE_LEARN, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_brgif->disable_learn, sizeof(p_brgif->disable_learn));
    }
    return str;
}

char**
tbl_fea_brg_if_table_dump(tbl_fea_brg_if_t *p_brgif, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_brg_if_key_name_dump(p_brgif, buf));
    for(i=1; i<TBL_FEA_BRG_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_brg_if_field_name_dump(p_brgif, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_brg_if_field_value_dump(p_brgif, i, buf));
    }
    return str;
}

int32
tbl_fea_brg_if_field_value_parser(char *str, int32 field_id, tbl_fea_brg_if_t *p_brgif)
{
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_KEY, field_id))
    {
        int32 ret;
        p_brgif->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_brgif->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_LAG_ID, field_id))
    {
        int32 ret;
        p_brgif->lag_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MAC_LEARNING_EN, field_id))
    {
        int32 ret;
        p_brgif->mac_learning_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_PORT_SECURITY_EN, field_id))
    {
        int32 ret;
        p_brgif->port_security_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MAX_MAC, field_id))
    {
        int32 ret;
        p_brgif->max_mac = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_CURR_MAC, field_id))
    {
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_CURR_MAC_STATIC, field_id))
    {
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_ID, field_id))
    {
        int32 ret;
        p_brgif->mlag_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_PEER_CONF, field_id))
    {
        int32 ret;
        p_brgif->mlag_peer_conf = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_PEER_IF_UP, field_id))
    {
        int32 ret;
        p_brgif->mlag_peer_if_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_IS_GROUP, field_id))
    {
        int32 ret;
        p_brgif->mlag_is_group = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_IS_MIRROR_DEST, field_id))
    {
        int32 ret;
        p_brgif->is_mirror_dest = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_HW_TYPE, field_id))
    {
        cdb_enum_str2val(glb_if_type_strs, GLB_IF_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_PORTID, field_id))
    {
        int32 ret;
        p_brgif->portid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_DISABLE_LEARN, field_id))
    {
        cdb_uint8_array_str2val(str, p_brgif->disable_learn);
    }
    return PM_E_NONE;
}

int32
tbl_fea_brg_if_table_parser(char** array, char* key_value,tbl_fea_brg_if_t *p_brgif)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_BRG_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_brg_if_key_str2val(key_value, p_brgif));

    for(i=1; i<TBL_FEA_BRG_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_brg_if_field_value_parser( array[j++], i, p_brgif));
    }

    return PM_E_NONE;
}

int32
tbl_fea_brg_if_dump_one(tbl_fea_brg_if_t *p_brgif, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_BRG_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_KEY].name,
            p_brgif->key.ifindex);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_FLAGS].name,
            p_brgif->flags);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_LAG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_LAG_ID].name,
            p_brgif->lag_id);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MAC_LEARNING_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_MAC_LEARNING_EN].name,
            p_brgif->mac_learning_en);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_PORT_SECURITY_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_PORT_SECURITY_EN].name,
            p_brgif->port_security_en);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MAX_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_MAX_MAC].name,
            p_brgif->max_mac);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_CURR_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_CURR_MAC].name,
            p_brgif->curr_mac);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_CURR_MAC_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_CURR_MAC_STATIC].name,
            p_brgif->curr_mac_static);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_MLAG_ID].name,
            p_brgif->mlag_id);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_PEER_CONF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_MLAG_PEER_CONF].name,
            p_brgif->mlag_peer_conf);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_PEER_IF_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_MLAG_PEER_IF_UP].name,
            p_brgif->mlag_peer_if_up);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_MLAG_IS_GROUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_MLAG_IS_GROUP].name,
            p_brgif->mlag_is_group);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_IS_MIRROR_DEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_IS_MIRROR_DEST].name,
            p_brgif->is_mirror_dest);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_HW_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_HW_TYPE].name, 
            cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_brgif->hw_type));
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_BRG_IF_FLD_PORTID].name,
            p_brgif->portid);
    }
    if (FLD_MATCH(TBL_FEA_BRG_IF_FLD_DISABLE_LEARN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_BRG_IF_FLD_DISABLE_LEARN].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_brgif->disable_learn, sizeof(p_brgif->disable_learn)));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_brg_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_brg_if_master_t *p_master = _g_p_tbl_fea_brg_if_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->brgif_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_fea_brg_if_master_t*
tbl_fea_brg_if_get_master()
{
    return _g_p_tbl_fea_brg_if_master;
}

tbl_fea_brg_if_master_t*
tbl_fea_brg_if_init_fea_brg_if()
{
    _g_p_tbl_fea_brg_if_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_brg_if_master_t));
    _g_p_tbl_fea_brg_if_master->brgif_hash = ctclib_hash_create(_tbl_fea_brg_if_hash_make, _tbl_fea_brg_if_hash_cmp);
    return _g_p_tbl_fea_brg_if_master;
}

