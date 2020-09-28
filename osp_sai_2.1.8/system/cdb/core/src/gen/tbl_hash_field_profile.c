
#include "proto.h"
#include "gen/tbl_hash_field_profile_define.h"
#include "gen/tbl_hash_field_profile.h"
#include "cdb_data_cmp.h"

tbl_hash_field_profile_master_t *_g_p_tbl_hash_field_profile_master = NULL;

static uint32
_tbl_hash_field_profile_hash_make(tbl_hash_field_profile_t *p_hash_field_profile)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_hash_field_profile->key;
    for (index = 0; index < sizeof(p_hash_field_profile->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_hash_field_profile_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_hash_field_profile_t *p_hash_field_profile1 = (tbl_hash_field_profile_t*)p_arg1;
    tbl_hash_field_profile_t *p_hash_field_profile2 = (tbl_hash_field_profile_t*)p_arg2;

    if (0 == sal_memcmp(&p_hash_field_profile1->key, &p_hash_field_profile2->key, sizeof(tbl_hash_field_profile_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_hash_field_profile_add_hash_field_profile_sync(tbl_hash_field_profile_t *p_hash_field_profile, uint32 sync)
{
    tbl_hash_field_profile_master_t *p_master = _g_p_tbl_hash_field_profile_master;
    tbl_hash_field_profile_t *p_db_hash_field_profile = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_hash_field_profile_get_hash_field_profile(&p_hash_field_profile->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_hash_field_profile = XCALLOC(MEM_TBL_HASH_FIELD_PROFILE, sizeof(tbl_hash_field_profile_t));
    if (NULL == p_db_hash_field_profile)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_hash_field_profile, p_hash_field_profile, sizeof(tbl_hash_field_profile_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->hash_field_profile_hash, (void*)p_db_hash_field_profile, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->hash_field_profile_list, p_db_hash_field_profile);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_HASH_FIELD_PROFILE, p_db_hash_field_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_HASH_FIELD_PROFILE, p_db_hash_field_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_hash_field_profile_del_hash_field_profile_sync(tbl_hash_field_profile_key_t *p_hash_field_profile_key, uint32 sync)
{
    tbl_hash_field_profile_master_t *p_master = _g_p_tbl_hash_field_profile_master;
    tbl_hash_field_profile_t *p_db_hash_field_profile = NULL;

    /* 1. lookup entry exist */
    p_db_hash_field_profile = tbl_hash_field_profile_get_hash_field_profile(p_hash_field_profile_key);
    if (NULL == p_db_hash_field_profile)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_HASH_FIELD_PROFILE, p_db_hash_field_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_HASH_FIELD_PROFILE, p_db_hash_field_profile);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->hash_field_profile_hash, (void*)p_db_hash_field_profile);
    ctclib_slistnode_delete(p_master->hash_field_profile_list, p_db_hash_field_profile);

    /* 4. free db entry */
    XFREE(MEM_TBL_HASH_FIELD_PROFILE, p_db_hash_field_profile);

    return PM_E_NONE;
}

int32
tbl_hash_field_profile_set_hash_field_profile_field_sync(tbl_hash_field_profile_t *p_hash_field_profile, tbl_hash_field_profile_field_id_t field_id, uint32 sync)
{
    tbl_hash_field_profile_t *p_db_hash_field_profile = NULL;

    /* 1. lookup entry exist */
    p_db_hash_field_profile = tbl_hash_field_profile_get_hash_field_profile(&p_hash_field_profile->key);
    if (NULL == p_db_hash_field_profile)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION:
        sal_memcpy(p_db_hash_field_profile->description, p_hash_field_profile->description, sizeof(p_hash_field_profile->description));
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DATA:
        p_db_hash_field_profile->bitmap_data = p_hash_field_profile->bitmap_data;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_FLAG:
        p_db_hash_field_profile->flag = p_hash_field_profile->flag;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_INDEX:
        p_db_hash_field_profile->index = p_hash_field_profile->index;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_L2_SET:
        p_db_hash_field_profile->l2_set = p_hash_field_profile->l2_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_IP_SET:
        p_db_hash_field_profile->ip_set = p_hash_field_profile->ip_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET:
        p_db_hash_field_profile->ipv6_set = p_hash_field_profile->ipv6_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_GRE_SET:
        p_db_hash_field_profile->gre_set = p_hash_field_profile->gre_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET:
        p_db_hash_field_profile->nvgre_set = p_hash_field_profile->nvgre_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET:
        p_db_hash_field_profile->mpls_set = p_hash_field_profile->mpls_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET:
        p_db_hash_field_profile->vpws_set = p_hash_field_profile->vpws_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET:
        p_db_hash_field_profile->l3vpn_set = p_hash_field_profile->l3vpn_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET:
        p_db_hash_field_profile->vxlan_set = p_hash_field_profile->vxlan_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET:
        p_db_hash_field_profile->vpls_set = p_hash_field_profile->vpls_set;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC:
        p_db_hash_field_profile->hash_arithmetic = p_hash_field_profile->hash_arithmetic;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_HASH_SEED_VALUE:
        p_db_hash_field_profile->hash_seed_value = p_hash_field_profile->hash_seed_value;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY:
        p_db_hash_field_profile->mode_symmetry = p_hash_field_profile->mode_symmetry;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE:
        p_db_hash_field_profile->bitmap_disable = p_hash_field_profile->bitmap_disable;
        break;

    case TBL_HASH_FIELD_PROFILE_FLD_MAX:
        sal_memcpy(p_db_hash_field_profile, p_hash_field_profile, sizeof(tbl_hash_field_profile_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_HASH_FIELD_PROFILE, field_id, p_db_hash_field_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_HASH_FIELD_PROFILE, field_id, p_db_hash_field_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_hash_field_profile_add_hash_field_profile(tbl_hash_field_profile_t *p_hash_field_profile)
{
    return tbl_hash_field_profile_add_hash_field_profile_sync(p_hash_field_profile, TRUE);
}

int32
tbl_hash_field_profile_del_hash_field_profile(tbl_hash_field_profile_key_t *p_hash_field_profile_key)
{
    return tbl_hash_field_profile_del_hash_field_profile_sync(p_hash_field_profile_key, TRUE);
}

int32
tbl_hash_field_profile_set_hash_field_profile_field(tbl_hash_field_profile_t *p_hash_field_profile, tbl_hash_field_profile_field_id_t field_id)
{
    return tbl_hash_field_profile_set_hash_field_profile_field_sync(p_hash_field_profile, field_id, TRUE);
}

tbl_hash_field_profile_t*
tbl_hash_field_profile_get_hash_field_profile(tbl_hash_field_profile_key_t *p_hash_field_profile_key)
{
    tbl_hash_field_profile_master_t *p_master = _g_p_tbl_hash_field_profile_master;
    tbl_hash_field_profile_t lkp;

    sal_memcpy(&lkp.key, p_hash_field_profile_key, sizeof(tbl_hash_field_profile_key_t));
    return ctclib_hash_lookup(p_master->hash_field_profile_hash, &lkp);
}

char*
tbl_hash_field_profile_key_val2str(tbl_hash_field_profile_t *p_hash_field_profile, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_hash_field_profile->key.name);
    return str;
}

int32
tbl_hash_field_profile_key_str2val(char *str, tbl_hash_field_profile_t *p_hash_field_profile)
{
    int32 ret = 0;

    sal_strcpy(p_hash_field_profile->key.name, str);
    return ret;
}

char*
tbl_hash_field_profile_key_name_dump(tbl_hash_field_profile_t *p_hash_field_profile, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_FIELD_PROFILE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_hash_field_profile->key.name);
    return str;
}

char*
tbl_hash_field_profile_key_value_dump(tbl_hash_field_profile_t *p_hash_field_profile, char *str)
{
    sal_sprintf(str, "%s", p_hash_field_profile->key.name);
    return str;
}

char*
tbl_hash_field_profile_field_name_dump(tbl_hash_field_profile_t *p_hash_field_profile, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_FIELD_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_HASH_FIELD_PROFILE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_hash_field_profile_field_value_dump(tbl_hash_field_profile_t *p_hash_field_profile, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_hash_field_profile->key.name);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION, field_id))
    {
        sal_sprintf(str, "%s", p_hash_field_profile->description);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DATA, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->bitmap_data);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->flag);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->index);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_L2_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->l2_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_IP_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->ip_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->ipv6_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_GRE_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->gre_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->nvgre_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->mpls_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->vpws_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->l3vpn_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->vxlan_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->vpls_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->hash_arithmetic);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_HASH_SEED_VALUE, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->hash_seed_value);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->mode_symmetry);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_hash_field_profile->bitmap_disable);
    }
    return str;
}

char**
tbl_hash_field_profile_table_dump(tbl_hash_field_profile_t *p_hash_field_profile, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_hash_field_profile_key_name_dump(p_hash_field_profile, buf));
    for(i=1; i<TBL_HASH_FIELD_PROFILE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_hash_field_profile_field_name_dump(p_hash_field_profile, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_hash_field_profile_field_value_dump(p_hash_field_profile, i, buf));
    }
    return str;
}

int32
tbl_hash_field_profile_field_value_parser(char *str, int32 field_id, tbl_hash_field_profile_t *p_hash_field_profile)
{
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_KEY, field_id))
    {
        sal_strcpy(p_hash_field_profile->key.name, str);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION, field_id))
    {
        sal_strcpy(p_hash_field_profile->description, str);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DATA, field_id))
    {
        int32 ret;
        p_hash_field_profile->bitmap_data = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_FLAG, field_id))
    {
        int32 ret;
        p_hash_field_profile->flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_INDEX, field_id))
    {
        int32 ret;
        p_hash_field_profile->index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_L2_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->l2_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_IP_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->ip_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->ipv6_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_GRE_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->gre_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->nvgre_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->mpls_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->vpws_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->l3vpn_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->vxlan_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET, field_id))
    {
        int32 ret;
        p_hash_field_profile->vpls_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC, field_id))
    {
        int32 ret;
        p_hash_field_profile->hash_arithmetic = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_HASH_SEED_VALUE, field_id))
    {
        int32 ret;
        p_hash_field_profile->hash_seed_value = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY, field_id))
    {
        int32 ret;
        p_hash_field_profile->mode_symmetry = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE, field_id))
    {
        int32 ret;
        p_hash_field_profile->bitmap_disable = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_hash_field_profile_table_parser(char** array, char* key_value,tbl_hash_field_profile_t *p_hash_field_profile)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_FIELD_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_hash_field_profile_key_str2val(key_value, p_hash_field_profile));

    for(i=1; i<TBL_HASH_FIELD_PROFILE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_hash_field_profile_field_value_parser( array[j++], i, p_hash_field_profile));
    }

    return PM_E_NONE;
}

int32
tbl_hash_field_profile_dump_one(tbl_hash_field_profile_t *p_hash_field_profile, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_FIELD_PROFILE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_KEY].name,
            p_hash_field_profile->key.name);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION].name,
            p_hash_field_profile->description);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DATA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DATA].name,
            p_hash_field_profile->bitmap_data);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_FLAG].name,
            p_hash_field_profile->flag);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_INDEX].name,
            p_hash_field_profile->index);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_L2_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_L2_SET].name,
            p_hash_field_profile->l2_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_IP_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_IP_SET].name,
            p_hash_field_profile->ip_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET].name,
            p_hash_field_profile->ipv6_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_GRE_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_GRE_SET].name,
            p_hash_field_profile->gre_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET].name,
            p_hash_field_profile->nvgre_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET].name,
            p_hash_field_profile->mpls_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET].name,
            p_hash_field_profile->vpws_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET].name,
            p_hash_field_profile->l3vpn_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET].name,
            p_hash_field_profile->vxlan_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET].name,
            p_hash_field_profile->vpls_set);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC].name,
            p_hash_field_profile->hash_arithmetic);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_HASH_SEED_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_HASH_SEED_VALUE].name,
            p_hash_field_profile->hash_seed_value);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY].name,
            p_hash_field_profile->mode_symmetry);
    }
    if (FLD_MATCH(TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE].name,
            p_hash_field_profile->bitmap_disable);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_hash_field_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_hash_field_profile_master_t *p_master = _g_p_tbl_hash_field_profile_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_hash_field_profile_t *p_db_hash_field_profile = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->hash_field_profile_list, p_db_hash_field_profile, listnode, next)
    {
        rc |= fn(p_db_hash_field_profile, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_hash_field_profile_master_t*
tbl_hash_field_profile_get_master()
{
    return _g_p_tbl_hash_field_profile_master;
}

tbl_hash_field_profile_master_t*
tbl_hash_field_profile_init_hash_field_profile()
{
    _g_p_tbl_hash_field_profile_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_hash_field_profile_master_t));
    _g_p_tbl_hash_field_profile_master->hash_field_profile_hash = ctclib_hash_create(_tbl_hash_field_profile_hash_make, _tbl_hash_field_profile_hash_cmp);
    _g_p_tbl_hash_field_profile_master->hash_field_profile_list = ctclib_slist_create(tbl_hash_field_profile_cmp, NULL);
    return _g_p_tbl_hash_field_profile_master;
}

