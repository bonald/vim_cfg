
#include "proto.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "cdb_data_cmp.h"

tbl_mstp_instance_master_t *_g_p_tbl_mstp_instance_master = NULL;

int32
tbl_mstp_instance_add_mstp_instance_sync(tbl_mstp_instance_t *p_inst, uint32 sync)
{
    tbl_mstp_instance_master_t *p_master = _g_p_tbl_mstp_instance_master;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_mstp_instance_get_mstp_instance(&p_inst->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_inst = XCALLOC(MEM_TBL_MSTP_INSTANCE, sizeof(tbl_mstp_instance_t));
    if (NULL == p_db_inst)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_inst, p_inst, sizeof(tbl_mstp_instance_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_inst->port_list, CDB_NODE_TBL, TBL_MSTI_PORT, tbl_msti_port_cmp));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->inst_list, p_db_inst);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MSTP_INSTANCE, p_db_inst);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MSTP_INSTANCE, p_db_inst);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mstp_instance_del_mstp_instance_sync(tbl_mstp_instance_key_t *p_inst_key, uint32 sync)
{
    tbl_mstp_instance_master_t *p_master = _g_p_tbl_mstp_instance_master;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_inst = tbl_mstp_instance_get_mstp_instance(p_inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MSTP_INSTANCE, p_db_inst);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MSTP_INSTANCE, p_db_inst);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->inst_list, p_db_inst);
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_inst->port_list));

    /* 4. free db entry */
    XFREE(MEM_TBL_MSTP_INSTANCE, p_db_inst);

    return PM_E_NONE;
}

int32
tbl_mstp_instance_set_mstp_instance_field_sync(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_field_id_t field_id, uint32 sync)
{
    tbl_mstp_instance_t *p_db_inst = NULL;

    /* 1. lookup entry exist */
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&p_inst->key);
    if (NULL == p_db_inst)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MSTP_INSTANCE_FLD_INSTANCE_OID:
        p_db_inst->instance_oid = p_inst->instance_oid;
        break;

    case TBL_MSTP_INSTANCE_FLD_VLAN:
        sal_memcpy(p_db_inst->vlan, p_inst->vlan, sizeof(p_inst->vlan));
        break;

    case TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY:
        p_db_inst->bridge_priority = p_inst->bridge_priority;
        break;

    case TBL_MSTP_INSTANCE_FLD_BRIDGE_ID:
        sal_memcpy(&p_db_inst->bridge_id, &p_inst->bridge_id, sizeof(p_inst->bridge_id));
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_PORTID:
        p_db_inst->root_portId = p_inst->root_portId;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_ROOTID:
        sal_memcpy(&p_db_inst->root_rootId, &p_inst->root_rootId, sizeof(p_inst->root_rootId));
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_ROOTPATHCOST:
        p_db_inst->root_rootPathCost = p_inst->root_rootPathCost;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDBRIDGEID:
        sal_memcpy(&p_db_inst->root_designatedBridgeId, &p_inst->root_designatedBridgeId, sizeof(p_inst->root_designatedBridgeId));
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDPORTID:
        p_db_inst->root_designatedPortId = p_inst->root_designatedPortId;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_BRIDGEPORTID:
        p_db_inst->root_bridgePortId = p_inst->root_bridgePortId;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_MESSAGEAGE:
        p_db_inst->root_messageAge = p_inst->root_messageAge;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_FWDDELAY:
        p_db_inst->root_fwdDelay = p_inst->root_fwdDelay;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_HELLOTIME:
        p_db_inst->root_helloTime = p_inst->root_helloTime;
        break;

    case TBL_MSTP_INSTANCE_FLD_ROOT_MAXAGE:
        p_db_inst->root_maxAge = p_inst->root_maxAge;
        break;

    case TBL_MSTP_INSTANCE_FLD_PRS_STATE:
        p_db_inst->prs_state = p_inst->prs_state;
        break;

    case TBL_MSTP_INSTANCE_FLD_HOPCOUNT:
        p_db_inst->hopCount = p_inst->hopCount;
        break;

    case TBL_MSTP_INSTANCE_FLD_G8032_RING_ID:
        p_db_inst->g8032_ring_id = p_inst->g8032_ring_id;
        break;

    case TBL_MSTP_INSTANCE_FLD_MAX:
        sal_memcpy(p_db_inst, p_inst, sizeof(tbl_mstp_instance_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MSTP_INSTANCE, field_id, p_db_inst);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MSTP_INSTANCE, field_id, p_db_inst);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mstp_instance_add_mstp_instance(tbl_mstp_instance_t *p_inst)
{
    return tbl_mstp_instance_add_mstp_instance_sync(p_inst, TRUE);
}

int32
tbl_mstp_instance_del_mstp_instance(tbl_mstp_instance_key_t *p_inst_key)
{
    return tbl_mstp_instance_del_mstp_instance_sync(p_inst_key, TRUE);
}

int32
tbl_mstp_instance_set_mstp_instance_field(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_field_id_t field_id)
{
    return tbl_mstp_instance_set_mstp_instance_field_sync(p_inst, field_id, TRUE);
}

tbl_mstp_instance_t*
tbl_mstp_instance_get_mstp_instance(tbl_mstp_instance_key_t *p_inst_key)
{
    tbl_mstp_instance_master_t *p_master = _g_p_tbl_mstp_instance_master;
    tbl_mstp_instance_t lkp;

    sal_memcpy(&lkp.key, p_inst_key, sizeof(tbl_mstp_instance_key_t));
    return ctclib_slistdata_lookup(p_master->inst_list, &lkp);
}

char*
tbl_mstp_instance_key_val2str(tbl_mstp_instance_t *p_inst, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_inst->key.instance);
    return str;
}

int32
tbl_mstp_instance_key_str2val(char *str, tbl_mstp_instance_t *p_inst)
{
    int32 ret = 0;

    p_inst->key.instance = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_mstp_instance_key_name_dump(tbl_mstp_instance_t *p_inst, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_INSTANCE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_inst->key.instance);
    return str;
}

char*
tbl_mstp_instance_key_value_dump(tbl_mstp_instance_t *p_inst, char *str)
{
    sal_sprintf(str, "%u", p_inst->key.instance);
    return str;
}

char*
tbl_mstp_instance_field_name_dump(tbl_mstp_instance_t *p_inst, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_INSTANCE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MSTP_INSTANCE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mstp_instance_field_value_dump(tbl_mstp_instance_t *p_inst, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_inst->key.instance);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_INSTANCE_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_inst->instance_oid);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_VLAN, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_inst->vlan, sizeof(p_inst->vlan)/4);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_inst->bridge_priority);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_BRIDGE_ID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_inst->bridge_id);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_PORTID, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_portId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_inst->root_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_ROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_inst->root_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_BRIDGEPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_MESSAGEAGE, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_inst->root_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_PRS_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_inst->prs_state);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_HOPCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_inst->hopCount);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_G8032_RING_ID, field_id))
    {
        sal_sprintf(str, "%u", p_inst->g8032_ring_id);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_PORT_LIST, field_id))
    {
    }
    return str;
}

char**
tbl_mstp_instance_table_dump(tbl_mstp_instance_t *p_inst, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mstp_instance_key_name_dump(p_inst, buf));
    for(i=1; i<TBL_MSTP_INSTANCE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mstp_instance_field_name_dump(p_inst, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mstp_instance_field_value_dump(p_inst, i, buf));
    }
    return str;
}

int32
tbl_mstp_instance_field_value_parser(char *str, int32 field_id, tbl_mstp_instance_t *p_inst)
{
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_KEY, field_id))
    {
        int32 ret;
        p_inst->key.instance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_INSTANCE_OID, field_id))
    {
        int32 ret;
        p_inst->instance_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_VLAN, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_inst->vlan, sizeof(p_inst->vlan)/4, err);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY, field_id))
    {
        int32 ret;
        p_inst->bridge_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_BRIDGE_ID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_inst->bridge_id);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_PORTID, field_id))
    {
        int32 ret;
        p_inst->root_portId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_inst->root_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_ROOTPATHCOST, field_id))
    {
        int32 ret;
        p_inst->root_rootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_inst->root_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDPORTID, field_id))
    {
        int32 ret;
        p_inst->root_designatedPortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_BRIDGEPORTID, field_id))
    {
        int32 ret;
        p_inst->root_bridgePortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_MESSAGEAGE, field_id))
    {
        int32 ret;
        p_inst->root_messageAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_FWDDELAY, field_id))
    {
        int32 ret;
        p_inst->root_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_HELLOTIME, field_id))
    {
        int32 ret;
        p_inst->root_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_MAXAGE, field_id))
    {
        int32 ret;
        p_inst->root_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_PRS_STATE, field_id))
    {
        int32 ret;
        p_inst->prs_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_HOPCOUNT, field_id))
    {
        int32 ret;
        p_inst->hopCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_G8032_RING_ID, field_id))
    {
        int32 ret;
        p_inst->g8032_ring_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_PORT_LIST, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_mstp_instance_table_parser(char** array, char* key_value,tbl_mstp_instance_t *p_inst)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_INSTANCE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mstp_instance_key_str2val(key_value, p_inst));

    for(i=1; i<TBL_MSTP_INSTANCE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mstp_instance_field_value_parser( array[j++], i, p_inst));
    }

    return PM_E_NONE;
}

int32
tbl_mstp_instance_dump_one(tbl_mstp_instance_t *p_inst, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_INSTANCE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_KEY].name,
            p_inst->key.instance);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_INSTANCE_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_INSTANCE_OID].name,
            p_inst->instance_oid);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_VLAN].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_inst->vlan, sizeof(p_inst->vlan)/4));
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY].name,
            p_inst->bridge_priority);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_BRIDGE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_BRIDGE_ID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_inst->bridge_id));
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_PORTID].name,
            p_inst->root_portId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_ROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_ROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_inst->root_rootId));
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_ROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_ROOTPATHCOST].name,
            p_inst->root_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDBRIDGEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDBRIDGEID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_inst->root_designatedBridgeId));
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDPORTID].name,
            p_inst->root_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_BRIDGEPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_BRIDGEPORTID].name,
            p_inst->root_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_MESSAGEAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_MESSAGEAGE].name,
            p_inst->root_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_FWDDELAY].name,
            p_inst->root_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_HELLOTIME].name,
            p_inst->root_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_ROOT_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_ROOT_MAXAGE].name,
            p_inst->root_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_PRS_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_PRS_STATE].name,
            p_inst->prs_state);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_HOPCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_HOPCOUNT].name,
            p_inst->hopCount);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_G8032_RING_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_INSTANCE_FLD_G8032_RING_ID].name,
            p_inst->g8032_ring_id);
    }
    if (FLD_MATCH(TBL_MSTP_INSTANCE_FLD_PORT_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[TBL_MSTP_INSTANCE_FLD_PORT_LIST].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_inst->port_list));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mstp_instance_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mstp_instance_master_t *p_master = _g_p_tbl_mstp_instance_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->inst_list, p_db_inst, listnode, next)
    {
        rc |= fn(p_db_inst, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mstp_instance_master_t*
tbl_mstp_instance_get_master()
{
    return _g_p_tbl_mstp_instance_master;
}

tbl_mstp_instance_master_t*
tbl_mstp_instance_init_mstp_instance()
{
    _g_p_tbl_mstp_instance_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_mstp_instance_master_t));
    _g_p_tbl_mstp_instance_master->inst_list = ctclib_slist_create(tbl_mstp_instance_cmp, NULL);
    return _g_p_tbl_mstp_instance_master;
}

int32
tbl_mstp_instance_set_port_list_sync(tbl_mstp_instance_t *p_inst, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&p_inst->key);
    if (NULL == p_db_inst)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_inst->port_list, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_inst->port_list, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_MSTI_PORT, add, &refer);
        cdb_sync_tbl_set_refer(TBL_MSTP_INSTANCE, TBL_MSTP_INSTANCE_FLD_PORT_LIST, p_inst, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
tbl_mstp_instance_set_port_list(tbl_mstp_instance_t *p_inst, uint32 add, void *p_object)
{
    return tbl_mstp_instance_set_port_list_sync(p_inst, add, p_object, TRUE);
}

