
#include "sdb.h"
#include "gen/stbl_nexthop_group_define.h"
#include "gen/stbl_nexthop_group.h"
#include "gen/stbl_nexthop_define.h"
#include "gen/stbl_nexthop.h"
#include "sdb_data_cmp.h"

stbl_nexthop_group_master_t *_g_p_stbl_nexthop_group_master = NULL;

int32
stbl_nexthop_group_add_nexthop_group_sync(stbl_nexthop_group_t *p_nh_group, uint32 sync)
{
    stbl_nexthop_group_master_t *p_master = _g_p_stbl_nexthop_group_master;
    stbl_nexthop_group_t *p_db_nh_group = NULL;

    /* 1. lookup entry exist */
    if (stbl_nexthop_group_get_nexthop_group(&p_nh_group->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nh_group = XCALLOC(MEM_STBL_NEXTHOP_GROUP, sizeof(stbl_nexthop_group_t));
    if (NULL == p_db_nh_group)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nh_group, p_nh_group, sizeof(stbl_nexthop_group_t));
    sdb_reference_list_init(&p_db_nh_group->nh_list, SDB_NODE_TBL, STBL_NEXTHOP, stbl_nexthop_cmp);

    /* 4. add to db */
    p_master->array[p_nh_group->key.group_id] = p_db_nh_group;

    return SDB_E_NONE;
}

int32
stbl_nexthop_group_del_nexthop_group_sync(stbl_nexthop_group_key_t *p_nh_group_key, uint32 sync)
{
    stbl_nexthop_group_master_t *p_master = _g_p_stbl_nexthop_group_master;
    stbl_nexthop_group_t *p_db_nh_group = NULL;

    /* 1. lookup entry exist */
    p_db_nh_group = stbl_nexthop_group_get_nexthop_group(p_nh_group_key);
    if (NULL == p_db_nh_group)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_nh_group_key->group_id] = NULL;
    sdb_reference_list_deinit(&p_db_nh_group->nh_list);

    /* 4. free db entry */
    XFREE(MEM_STBL_NEXTHOP_GROUP, p_db_nh_group);

    return SDB_E_NONE;
}

int32
stbl_nexthop_group_set_nexthop_group_field_sync(stbl_nexthop_group_t *p_nh_group, stbl_nexthop_group_field_id_t field_id, uint32 sync)
{
    stbl_nexthop_group_t *p_db_nh_group = NULL;

    /* 1. lookup entry exist */
    p_db_nh_group = stbl_nexthop_group_get_nexthop_group(&p_nh_group->key);
    if (NULL == p_db_nh_group)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_NEXTHOP_GROUP_FLD_TYPE:
        p_db_nh_group->type = p_nh_group->type;
        break;

    case STBL_NEXTHOP_GROUP_FLD_IPMC_GROUP_ID:
        p_db_nh_group->ipmc_group_id = p_nh_group->ipmc_group_id;
        break;

    case STBL_NEXTHOP_GROUP_FLD_ECMP_TYPE:
        p_db_nh_group->ecmp_type = p_nh_group->ecmp_type;
        break;

    case STBL_NEXTHOP_GROUP_FLD_FAILOVER_EN:
        p_db_nh_group->failover_en = p_nh_group->failover_en;
        break;

    case STBL_NEXTHOP_GROUP_FLD_MAX:
        sal_memcpy(p_db_nh_group, p_nh_group, sizeof(stbl_nexthop_group_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_nexthop_group_add_nexthop_group(stbl_nexthop_group_t *p_nh_group)
{
    return stbl_nexthop_group_add_nexthop_group_sync(p_nh_group, TRUE);
}

int32
stbl_nexthop_group_del_nexthop_group(stbl_nexthop_group_key_t *p_nh_group_key)
{
    return stbl_nexthop_group_del_nexthop_group_sync(p_nh_group_key, TRUE);
}

int32
stbl_nexthop_group_set_nexthop_group_field(stbl_nexthop_group_t *p_nh_group, stbl_nexthop_group_field_id_t field_id)
{
    return stbl_nexthop_group_set_nexthop_group_field_sync(p_nh_group, field_id, TRUE);
}

stbl_nexthop_group_t*
stbl_nexthop_group_get_nexthop_group(stbl_nexthop_group_key_t *p_nh_group_key)
{
    stbl_nexthop_group_master_t *p_master = _g_p_stbl_nexthop_group_master;

    if (GLB_MAX_CTC_NEXTHOP_GROUP_ID+1 <= p_nh_group_key->group_id)
    {
        return NULL;
    }
    return (p_master->array[p_nh_group_key->group_id]);
}

char*
stbl_nexthop_group_key_val2str(stbl_nexthop_group_t *p_nh_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_nh_group->key.group_id);
    return str;
}

int32
stbl_nexthop_group_dump_one(stbl_nexthop_group_t *p_nh_group, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_NEXTHOP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_NEXTHOP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NEXTHOP_GROUP_FLD_KEY].name,
            p_nh_group->key.group_id);
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_GROUP_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NEXTHOP_GROUP_FLD_TYPE].name,
            p_nh_group->type);
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_GROUP_FLD_IPMC_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NEXTHOP_GROUP_FLD_IPMC_GROUP_ID].name,
            p_nh_group->ipmc_group_id);
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_GROUP_FLD_ECMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NEXTHOP_GROUP_FLD_ECMP_TYPE].name,
            p_nh_group->ecmp_type);
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_GROUP_FLD_FAILOVER_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NEXTHOP_GROUP_FLD_FAILOVER_EN].name,
            p_nh_group->failover_en);
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_GROUP_FLD_NH_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[STBL_NEXTHOP_GROUP_FLD_NH_LIST].name,
            sdb_reference_list_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_nh_group->nh_list));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_nexthop_group_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_nexthop_group_master_t *p_master = _g_p_stbl_nexthop_group_master;
    stbl_nexthop_group_t *p_db_nh_group = NULL;
    uint32 group_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (group_id = 0; group_id < GLB_MAX_CTC_NEXTHOP_GROUP_ID+1; group_id++)
    {
        p_db_nh_group = p_master->array[group_id];
        if (NULL == p_db_nh_group)
        {
            continue;
        }
        rc |= fn(p_db_nh_group, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_nexthop_group_master_t*
stbl_nexthop_group_get_master()
{
    return _g_p_stbl_nexthop_group_master;
}

stbl_nexthop_group_master_t*
stbl_nexthop_group_init_nexthop_group()
{
    _g_p_stbl_nexthop_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_nexthop_group_master_t));
    return _g_p_stbl_nexthop_group_master;
}

int32
stbl_nexthop_group_set_nh_list_sync(stbl_nexthop_group_t *p_nh_group, uint32 add, void *p_object, uint32 sync)
{
    stbl_nexthop_group_t *p_db_nh_group = NULL;

    /* 1. lookup entry exist */
    p_db_nh_group = stbl_nexthop_group_get_nexthop_group(&p_nh_group->key);
    if (NULL == p_db_nh_group)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        sdb_reference_list_add_object(&p_db_nh_group->nh_list, p_object);
    }
    else
    {
        sdb_reference_list_del_object(&p_db_nh_group->nh_list, p_object);
    }

    return SDB_E_NONE;
}

int32
stbl_nexthop_group_set_nh_list(stbl_nexthop_group_t *p_nh_group, uint32 add, void *p_object)
{
    return stbl_nexthop_group_set_nh_list_sync(p_nh_group, add, p_object, TRUE);
}

