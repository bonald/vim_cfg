
#include "proto.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/ds_pvlan_community_define.h"
#include "gen/ds_pvlan_community.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

tbl_pvlan_master_t *_g_p_tbl_pvlan_master = NULL;

int32
tbl_pvlan_add_pvlan_sync(tbl_pvlan_t *p_pvlan, uint32 sync)
{
    tbl_pvlan_master_t *p_master = _g_p_tbl_pvlan_master;
    tbl_pvlan_t *p_db_pvlan = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_pvlan_get_pvlan(&p_pvlan->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pvlan = XCALLOC(MEM_TBL_PVLAN, sizeof(tbl_pvlan_t));
    if (NULL == p_db_pvlan)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pvlan, p_pvlan, sizeof(tbl_pvlan_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_pvlan->community_groups, CDB_NODE_DS, DS_PVLAN_COMMUNITY, ds_pvlan_community_cmp));
    PM_E_RETURN(cdb_reference_list_init(&p_db_pvlan->isloate_ports, CDB_NODE_TBL, TBL_INTERFACE, tbl_interface_cmp));
    PM_E_RETURN(cdb_reference_list_init(&p_db_pvlan->promiscuous_ports, CDB_NODE_TBL, TBL_INTERFACE, tbl_interface_cmp));

    /* 4. add to db */
    p_master->pvlan_array[p_pvlan->key.primary_vid] = p_db_pvlan;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PVLAN, p_db_pvlan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PVLAN, p_db_pvlan);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pvlan_del_pvlan_sync(tbl_pvlan_key_t *p_pvlan_key, uint32 sync)
{
    tbl_pvlan_master_t *p_master = _g_p_tbl_pvlan_master;
    tbl_pvlan_t *p_db_pvlan = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(p_pvlan_key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PVLAN, p_db_pvlan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PVLAN, p_db_pvlan);
        #endif 
    }

    /* 3. delete from db */
    p_master->pvlan_array[p_pvlan_key->primary_vid] = NULL;
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_pvlan->community_groups));
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_pvlan->isloate_ports));
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_pvlan->promiscuous_ports));

    /* 4. free db entry */
    XFREE(MEM_TBL_PVLAN, p_db_pvlan);

    return PM_E_NONE;
}

int32
tbl_pvlan_set_pvlan_field_sync(tbl_pvlan_t *p_pvlan, tbl_pvlan_field_id_t field_id, uint32 sync)
{
    tbl_pvlan_t *p_db_pvlan = NULL;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PVLAN_FLD_COMMUNITY_GROUP_ID:
        sal_memcpy(p_db_pvlan->community_group_id, p_pvlan->community_group_id, sizeof(p_pvlan->community_group_id));
        break;

    case TBL_PVLAN_FLD_MAX:
        sal_memcpy(p_db_pvlan, p_pvlan, sizeof(tbl_pvlan_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PVLAN, field_id, p_db_pvlan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PVLAN, field_id, p_db_pvlan);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pvlan_add_pvlan(tbl_pvlan_t *p_pvlan)
{
    return tbl_pvlan_add_pvlan_sync(p_pvlan, TRUE);
}

int32
tbl_pvlan_del_pvlan(tbl_pvlan_key_t *p_pvlan_key)
{
    return tbl_pvlan_del_pvlan_sync(p_pvlan_key, TRUE);
}

int32
tbl_pvlan_set_pvlan_field(tbl_pvlan_t *p_pvlan, tbl_pvlan_field_id_t field_id)
{
    return tbl_pvlan_set_pvlan_field_sync(p_pvlan, field_id, TRUE);
}

tbl_pvlan_t*
tbl_pvlan_get_pvlan(tbl_pvlan_key_t *p_pvlan_key)
{
    tbl_pvlan_master_t *p_master = _g_p_tbl_pvlan_master;

    if (GLB_MAX_VLAN_ID <= p_pvlan_key->primary_vid)
    {
        return NULL;
    }
    return (p_master->pvlan_array[p_pvlan_key->primary_vid]);
}

char*
tbl_pvlan_key_val2str(tbl_pvlan_t *p_pvlan, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_pvlan->key.primary_vid);
    return str;
}

int32
tbl_pvlan_key_str2val(char *str, tbl_pvlan_t *p_pvlan)
{
    int32 ret = 0;

    p_pvlan->key.primary_vid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_pvlan_key_name_dump(tbl_pvlan_t *p_pvlan, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PVLAN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_pvlan->key.primary_vid);
    return str;
}

char*
tbl_pvlan_key_value_dump(tbl_pvlan_t *p_pvlan, char *str)
{
    sal_sprintf(str, "%u", p_pvlan->key.primary_vid);
    return str;
}

char*
tbl_pvlan_field_name_dump(tbl_pvlan_t *p_pvlan, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PVLAN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PVLAN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pvlan_field_value_dump(tbl_pvlan_t *p_pvlan, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PVLAN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_pvlan->key.primary_vid);
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_COMMUNITY_GROUP_ID, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_pvlan->community_group_id, sizeof(p_pvlan->community_group_id)/4);
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_COMMUNITY_GROUPS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_ISLOATE_PORTS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_PROMISCUOUS_PORTS, field_id))
    {
    }
    return str;
}

char**
tbl_pvlan_table_dump(tbl_pvlan_t *p_pvlan, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pvlan_key_name_dump(p_pvlan, buf));
    for(i=1; i<TBL_PVLAN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pvlan_field_name_dump(p_pvlan, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pvlan_field_value_dump(p_pvlan, i, buf));
    }
    return str;
}

int32
tbl_pvlan_field_value_parser(char *str, int32 field_id, tbl_pvlan_t *p_pvlan)
{
    if (FLD_MATCH(TBL_PVLAN_FLD_KEY, field_id))
    {
        int32 ret;
        p_pvlan->key.primary_vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_COMMUNITY_GROUP_ID, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_pvlan->community_group_id, sizeof(p_pvlan->community_group_id)/4, err);
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_COMMUNITY_GROUPS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_ISLOATE_PORTS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_PROMISCUOUS_PORTS, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_pvlan_table_parser(char** array, char* key_value,tbl_pvlan_t *p_pvlan)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PVLAN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pvlan_key_str2val(key_value, p_pvlan));

    for(i=1; i<TBL_PVLAN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pvlan_field_value_parser( array[j++], i, p_pvlan));
    }

    return PM_E_NONE;
}

int32
tbl_pvlan_dump_one(tbl_pvlan_t *p_pvlan, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PVLAN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    ds_pvlan_community_t *p_db_pvlan_community = NULL;
    ctclib_slistnode_t *listnode = NULL;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PVLAN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PVLAN_FLD_KEY].name,
            p_pvlan->key.primary_vid);
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_COMMUNITY_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PVLAN_FLD_COMMUNITY_GROUP_ID].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_pvlan->community_group_id, sizeof(p_pvlan->community_group_id)/4));
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_COMMUNITY_GROUPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_PVLAN_FLD_COMMUNITY_GROUPS].name);
        if (p_pvlan->community_groups.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_pvlan->community_groups.obj_list, p_db_pvlan_community, listnode)
            {
                ds_pvlan_community_dump_one(p_db_pvlan_community, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_ISLOATE_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[TBL_PVLAN_FLD_ISLOATE_PORTS].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_pvlan->isloate_ports));
    }
    if (FLD_MATCH(TBL_PVLAN_FLD_PROMISCUOUS_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[TBL_PVLAN_FLD_PROMISCUOUS_PORTS].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_pvlan->promiscuous_ports));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pvlan_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pvlan_master_t *p_master = _g_p_tbl_pvlan_master;
    tbl_pvlan_t *p_db_pvlan = NULL;
    uint32 primary_vid = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (primary_vid = 0; primary_vid < GLB_MAX_VLAN_ID; primary_vid++)
    {
        p_db_pvlan = p_master->pvlan_array[primary_vid];
        if (NULL == p_db_pvlan)
        {
            continue;
        }
        rc |= fn(p_db_pvlan, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pvlan_master_t*
tbl_pvlan_get_master()
{
    return _g_p_tbl_pvlan_master;
}

tbl_pvlan_master_t*
tbl_pvlan_init_pvlan()
{
    _g_p_tbl_pvlan_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pvlan_master_t));
    return _g_p_tbl_pvlan_master;
}

int32
tbl_pvlan_set_isloate_ports_sync(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    tbl_pvlan_t *p_db_pvlan = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_pvlan->isloate_ports, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_pvlan->isloate_ports, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_INTERFACE, add, &refer);
        cdb_sync_tbl_set_refer(TBL_PVLAN, TBL_PVLAN_FLD_ISLOATE_PORTS, p_pvlan, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
tbl_pvlan_set_isloate_ports(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object)
{
    return tbl_pvlan_set_isloate_ports_sync(p_pvlan, add, p_object, TRUE);
}

int32
tbl_pvlan_set_promiscuous_ports_sync(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    tbl_pvlan_t *p_db_pvlan = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_pvlan->promiscuous_ports, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_pvlan->promiscuous_ports, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_INTERFACE, add, &refer);
        cdb_sync_tbl_set_refer(TBL_PVLAN, TBL_PVLAN_FLD_PROMISCUOUS_PORTS, p_pvlan, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
tbl_pvlan_set_promiscuous_ports(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object)
{
    return tbl_pvlan_set_promiscuous_ports_sync(p_pvlan, add, p_object, TRUE);
}

