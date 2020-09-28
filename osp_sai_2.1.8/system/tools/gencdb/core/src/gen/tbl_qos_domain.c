
#include "proto.h"
#include "gen/tbl_qos_domain_define.h"
#include "gen/tbl_qos_domain.h"
#include "cdb_data_cmp.h"

tbl_qos_domain_master_t *_g_p_tbl_qos_domain_master = NULL;

int32
tbl_qos_domain_add_qos_domain_sync(tbl_qos_domain_t *p_qos_domain, uint32 sync)
{
    tbl_qos_domain_master_t *p_master = _g_p_tbl_qos_domain_master;
    tbl_qos_domain_t *p_db_qos_domain = NULL;

    /* 1. lookup entry exist */
    if (tbl_qos_domain_get_qos_domain(&p_qos_domain->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_domain = XCALLOC(MEM_TBL_QOS_DOMAIN, sizeof(tbl_qos_domain_t));
    if (NULL == p_db_qos_domain)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_domain, p_qos_domain, sizeof(tbl_qos_domain_t));

    /* 4. add to db */
    p_master->domain_array[p_qos_domain->key.id] = p_db_qos_domain;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_QOS_DOMAIN, p_db_qos_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_QOS_DOMAIN, p_db_qos_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_domain_del_qos_domain_sync(tbl_qos_domain_key_t *p_qos_domain_key, uint32 sync)
{
    tbl_qos_domain_master_t *p_master = _g_p_tbl_qos_domain_master;
    tbl_qos_domain_t *p_db_qos_domain = NULL;

    /* 1. lookup entry exist */
    p_db_qos_domain = tbl_qos_domain_get_qos_domain(p_qos_domain_key);
    if (NULL == p_db_qos_domain)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_QOS_DOMAIN, p_db_qos_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_QOS_DOMAIN, p_db_qos_domain);
        #endif 
    }

    /* 3. delete from db */
    p_master->domain_array[p_qos_domain_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_QOS_DOMAIN, p_db_qos_domain);

    return PM_E_NONE;
}

int32
tbl_qos_domain_set_qos_domain_field_sync(tbl_qos_domain_t *p_qos_domain, tbl_qos_domain_field_id_t field_id, uint32 sync)
{
    tbl_qos_domain_t *p_db_qos_domain = NULL;

    /* 1. lookup entry exist */
    p_db_qos_domain = tbl_qos_domain_get_qos_domain(&p_qos_domain->key);
    if (NULL == p_db_qos_domain)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_DOMAIN_FLD_PHB_ENABLE:
        p_db_qos_domain->phb_enable = p_qos_domain->phb_enable;
        break;

    case TBL_QOS_DOMAIN_FLD_CFI_ENABLE:
        p_db_qos_domain->cfi_enable = p_qos_domain->cfi_enable;
        break;

    case TBL_QOS_DOMAIN_FLD_COS2PRI:
        sal_memcpy(p_db_qos_domain->cos2pri, p_qos_domain->cos2pri, sizeof(p_qos_domain->cos2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2COLOR:
        sal_memcpy(p_db_qos_domain->cos2color, p_qos_domain->cos2color, sizeof(p_qos_domain->cos2color));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2PHB:
        sal_memcpy(p_db_qos_domain->cos2phb, p_qos_domain->cos2phb, sizeof(p_qos_domain->cos2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2PRI:
        sal_memcpy(p_db_qos_domain->dscp2pri, p_qos_domain->dscp2pri, sizeof(p_qos_domain->dscp2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2COLOR:
        sal_memcpy(p_db_qos_domain->dscp2color, p_qos_domain->dscp2color, sizeof(p_qos_domain->dscp2color));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2PHB:
        sal_memcpy(p_db_qos_domain->dscp2phb, p_qos_domain->dscp2phb, sizeof(p_qos_domain->dscp2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2PRI:
        sal_memcpy(p_db_qos_domain->prec2pri, p_qos_domain->prec2pri, sizeof(p_qos_domain->prec2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2COLOR:
        sal_memcpy(p_db_qos_domain->prec2color, p_qos_domain->prec2color, sizeof(p_qos_domain->prec2color));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2PHB:
        sal_memcpy(p_db_qos_domain->prec2phb, p_qos_domain->prec2phb, sizeof(p_qos_domain->prec2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2PRI:
        sal_memcpy(p_db_qos_domain->exp2pri, p_qos_domain->exp2pri, sizeof(p_qos_domain->exp2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2COLOR:
        sal_memcpy(p_db_qos_domain->exp2color, p_qos_domain->exp2color, sizeof(p_qos_domain->exp2color));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2PHB:
        sal_memcpy(p_db_qos_domain->exp2phb, p_qos_domain->exp2phb, sizeof(p_qos_domain->exp2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS:
        sal_memcpy(p_db_qos_domain->pri_color2cos, p_qos_domain->pri_color2cos, sizeof(p_qos_domain->pri_color2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP:
        sal_memcpy(p_db_qos_domain->pri_color2dscp, p_qos_domain->pri_color2dscp, sizeof(p_qos_domain->pri_color2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP:
        sal_memcpy(p_db_qos_domain->pri_color2exp, p_qos_domain->pri_color2exp, sizeof(p_qos_domain->pri_color2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2COS:
        sal_memcpy(p_db_qos_domain->phb2cos, p_qos_domain->phb2cos, sizeof(p_qos_domain->phb2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2DSCP:
        sal_memcpy(p_db_qos_domain->phb2dscp, p_qos_domain->phb2dscp, sizeof(p_qos_domain->phb2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2EXP:
        sal_memcpy(p_db_qos_domain->phb2exp, p_qos_domain->phb2exp, sizeof(p_qos_domain->phb2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2TC:
        sal_memcpy(p_db_qos_domain->cos2tc, p_qos_domain->cos2tc, sizeof(p_qos_domain->cos2tc));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2TC:
        sal_memcpy(p_db_qos_domain->dscp2tc, p_qos_domain->dscp2tc, sizeof(p_qos_domain->dscp2tc));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2TC:
        sal_memcpy(p_db_qos_domain->exp2tc, p_qos_domain->exp2tc, sizeof(p_qos_domain->exp2tc));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2COS:
        sal_memcpy(p_db_qos_domain->tc_color2cos, p_qos_domain->tc_color2cos, sizeof(p_qos_domain->tc_color2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP:
        sal_memcpy(p_db_qos_domain->tc_color2dscp, p_qos_domain->tc_color2dscp, sizeof(p_qos_domain->tc_color2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP:
        sal_memcpy(p_db_qos_domain->tc_color2exp, p_qos_domain->tc_color2exp, sizeof(p_qos_domain->tc_color2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_MAX:
        sal_memcpy(p_db_qos_domain, p_qos_domain, sizeof(tbl_qos_domain_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_DOMAIN, field_id, p_db_qos_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_DOMAIN, field_id, p_db_qos_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_domain_add_qos_domain(tbl_qos_domain_t *p_qos_domain)
{
    return tbl_qos_domain_add_qos_domain_sync(p_qos_domain, TRUE);
}

int32
tbl_qos_domain_del_qos_domain(tbl_qos_domain_key_t *p_qos_domain_key)
{
    return tbl_qos_domain_del_qos_domain_sync(p_qos_domain_key, TRUE);
}

int32
tbl_qos_domain_set_qos_domain_field(tbl_qos_domain_t *p_qos_domain, tbl_qos_domain_field_id_t field_id)
{
    return tbl_qos_domain_set_qos_domain_field_sync(p_qos_domain, field_id, TRUE);
}

tbl_qos_domain_t*
tbl_qos_domain_get_qos_domain(tbl_qos_domain_key_t *p_qos_domain_key)
{
    tbl_qos_domain_master_t *p_master = _g_p_tbl_qos_domain_master;

    if (GLB_QOS_DOMAIN_NUM <= p_qos_domain_key->id)
    {
        return NULL;
    }
    return (p_master->domain_array[p_qos_domain_key->id]);
}

char*
tbl_qos_domain_key_val2str(tbl_qos_domain_t *p_qos_domain, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_domain->key.id);
    return str;
}

int32
tbl_qos_domain_key_str2val(char *str, tbl_qos_domain_t *p_qos_domain)
{
    int32 ret = 0;

    p_qos_domain->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_qos_domain_key_name_dump(tbl_qos_domain_t *p_qos_domain, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DOMAIN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_qos_domain->key.id);
    return str;
}

char*
tbl_qos_domain_key_value_dump(tbl_qos_domain_t *p_qos_domain, char *str)
{
    sal_sprintf(str, "%u", p_qos_domain->key.id);
    return str;
}

char*
tbl_qos_domain_field_name_dump(tbl_qos_domain_t *p_qos_domain, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_DOMAIN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_domain_field_value_dump(tbl_qos_domain_t *p_qos_domain, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_domain->key.id);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_domain->phb_enable);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_CFI_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_domain->cfi_enable);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2PRI, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->cos2pri, sizeof(p_qos_domain->cos2pri)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2COLOR, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->cos2color, sizeof(p_qos_domain->cos2color)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2PHB, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->cos2phb, sizeof(p_qos_domain->cos2phb)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2PRI, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->dscp2pri, sizeof(p_qos_domain->dscp2pri)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2COLOR, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->dscp2color, sizeof(p_qos_domain->dscp2color)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2PHB, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->dscp2phb, sizeof(p_qos_domain->dscp2phb)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2PRI, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->prec2pri, sizeof(p_qos_domain->prec2pri)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2COLOR, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->prec2color, sizeof(p_qos_domain->prec2color)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2PHB, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->prec2phb, sizeof(p_qos_domain->prec2phb)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2PRI, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->exp2pri, sizeof(p_qos_domain->exp2pri)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2COLOR, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->exp2color, sizeof(p_qos_domain->exp2color)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2PHB, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->exp2phb, sizeof(p_qos_domain->exp2phb)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->pri_color2cos, sizeof(p_qos_domain->pri_color2cos)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->pri_color2dscp, sizeof(p_qos_domain->pri_color2dscp)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->pri_color2exp, sizeof(p_qos_domain->pri_color2exp)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2COS, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->phb2cos, sizeof(p_qos_domain->phb2cos)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2DSCP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->phb2dscp, sizeof(p_qos_domain->phb2dscp)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2EXP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->phb2exp, sizeof(p_qos_domain->phb2exp)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2TC, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->cos2tc, sizeof(p_qos_domain->cos2tc)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2TC, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->dscp2tc, sizeof(p_qos_domain->dscp2tc)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2TC, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->exp2tc, sizeof(p_qos_domain->exp2tc)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2COS, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->tc_color2cos, sizeof(p_qos_domain->tc_color2cos)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->tc_color2dscp, sizeof(p_qos_domain->tc_color2dscp)/4);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_domain->tc_color2exp, sizeof(p_qos_domain->tc_color2exp)/4);
    }
    return str;
}

char**
tbl_qos_domain_table_dump(tbl_qos_domain_t *p_qos_domain, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_domain_key_name_dump(p_qos_domain, buf));
    for(i=1; i<TBL_QOS_DOMAIN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_domain_field_name_dump(p_qos_domain, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_domain_field_value_dump(p_qos_domain, i, buf));
    }
    return str;
}

int32
tbl_qos_domain_field_value_parser(char *str, int32 field_id, tbl_qos_domain_t *p_qos_domain)
{
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_KEY, field_id))
    {
        int32 ret;
        p_qos_domain->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB_ENABLE, field_id))
    {
        int32 ret;
        p_qos_domain->phb_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_CFI_ENABLE, field_id))
    {
        int32 ret;
        p_qos_domain->cfi_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2PRI, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->cos2pri);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2COLOR, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->cos2color);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2PHB, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->cos2phb);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2PRI, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->dscp2pri);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2COLOR, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->dscp2color);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2PHB, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->dscp2phb);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2PRI, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->prec2pri);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2COLOR, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->prec2color);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2PHB, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->prec2phb);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2PRI, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->exp2pri);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2COLOR, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->exp2color);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2PHB, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->exp2phb);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->pri_color2cos);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->pri_color2dscp);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->pri_color2exp);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2COS, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->phb2cos);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2DSCP, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->phb2dscp);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2EXP, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->phb2exp);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2TC, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->cos2tc);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2TC, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->dscp2tc);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2TC, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->exp2tc);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2COS, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->tc_color2cos);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->tc_color2dscp);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_domain->tc_color2exp);
    }
    return PM_E_NONE;
}

int32
tbl_qos_domain_table_parser(char** array, char* key_value,tbl_qos_domain_t *p_qos_domain)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_domain_key_str2val(key_value, p_qos_domain));

    for(i=1; i<TBL_QOS_DOMAIN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_domain_field_value_parser( array[j++], i, p_qos_domain));
    }

    return PM_E_NONE;
}

int32
tbl_qos_domain_dump_one(tbl_qos_domain_t *p_qos_domain, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DOMAIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_KEY].name,
            p_qos_domain->key.id);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PHB_ENABLE].name,
            p_qos_domain->phb_enable);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_CFI_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_CFI_ENABLE].name,
            p_qos_domain->cfi_enable);
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2PRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_COS2PRI].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->cos2pri, sizeof(p_qos_domain->cos2pri)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_COS2COLOR].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->cos2color, sizeof(p_qos_domain->cos2color)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2PHB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_COS2PHB].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->cos2phb, sizeof(p_qos_domain->cos2phb)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2PRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_DSCP2PRI].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->dscp2pri, sizeof(p_qos_domain->dscp2pri)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_DSCP2COLOR].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->dscp2color, sizeof(p_qos_domain->dscp2color)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2PHB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_DSCP2PHB].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->dscp2phb, sizeof(p_qos_domain->dscp2phb)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2PRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PREC2PRI].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->prec2pri, sizeof(p_qos_domain->prec2pri)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PREC2COLOR].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->prec2color, sizeof(p_qos_domain->prec2color)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PREC2PHB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PREC2PHB].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->prec2phb, sizeof(p_qos_domain->prec2phb)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2PRI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_EXP2PRI].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->exp2pri, sizeof(p_qos_domain->exp2pri)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_EXP2COLOR].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->exp2color, sizeof(p_qos_domain->exp2color)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2PHB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_EXP2PHB].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->exp2phb, sizeof(p_qos_domain->exp2phb)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->pri_color2cos, sizeof(p_qos_domain->pri_color2cos)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->pri_color2dscp, sizeof(p_qos_domain->pri_color2dscp)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->pri_color2exp, sizeof(p_qos_domain->pri_color2exp)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PHB2COS].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->phb2cos, sizeof(p_qos_domain->phb2cos)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PHB2DSCP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->phb2dscp, sizeof(p_qos_domain->phb2dscp)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_PHB2EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_PHB2EXP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->phb2exp, sizeof(p_qos_domain->phb2exp)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_COS2TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_COS2TC].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->cos2tc, sizeof(p_qos_domain->cos2tc)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_DSCP2TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_DSCP2TC].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->dscp2tc, sizeof(p_qos_domain->dscp2tc)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_EXP2TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_EXP2TC].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->exp2tc, sizeof(p_qos_domain->exp2tc)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_TC_COLOR2COS].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->tc_color2cos, sizeof(p_qos_domain->tc_color2cos)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->tc_color2dscp, sizeof(p_qos_domain->tc_color2dscp)/4));
    }
    if (FLD_MATCH(TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_domain->tc_color2exp, sizeof(p_qos_domain->tc_color2exp)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_domain_master_t *p_master = _g_p_tbl_qos_domain_master;
    tbl_qos_domain_t *p_db_qos_domain = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_QOS_DOMAIN_NUM; id++)
    {
        p_db_qos_domain = p_master->domain_array[id];
        if (NULL == p_db_qos_domain)
        {
            continue;
        }
        rc |= fn(p_db_qos_domain, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_qos_domain_master_t*
tbl_qos_domain_get_master()
{
    return _g_p_tbl_qos_domain_master;
}

tbl_qos_domain_master_t*
tbl_qos_domain_init_qos_domain()
{
    _g_p_tbl_qos_domain_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_qos_domain_master_t));
    return _g_p_tbl_qos_domain_master;
}

int32
tbl_qos_domain_get_qos_domain_field(uint8 domain_id, tbl_qos_domain_field_id_t field_id, tbl_qos_domain_t* p_domain)
{
    tbl_qos_domain_key_t key;
    tbl_qos_domain_t *p_db_qos_domain = NULL;

    key.id = domain_id;

    p_db_qos_domain = tbl_qos_domain_get_qos_domain(&key);

    /* 1. lookup entry exist */
    if (!p_db_qos_domain)
    {
        return PM_E_NOT_EXIST;
    }

    switch (field_id)
    {
    case TBL_QOS_DOMAIN_FLD_CFI_ENABLE:
        p_domain->cfi_enable = p_db_qos_domain->cfi_enable;
        break;

    case TBL_QOS_DOMAIN_FLD_COS2PRI:
        sal_memcpy(p_domain->cos2pri, p_db_qos_domain->cos2pri, sizeof(p_db_qos_domain->cos2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2COLOR:
        sal_memcpy(p_domain->cos2color, p_db_qos_domain->cos2color, sizeof(p_db_qos_domain->cos2color));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2PHB:
        sal_memcpy(p_domain->cos2phb, p_db_qos_domain->cos2phb, sizeof(p_db_qos_domain->cos2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2PRI:
        sal_memcpy(p_domain->dscp2pri, p_db_qos_domain->dscp2pri, sizeof(p_db_qos_domain->dscp2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2COLOR:
        sal_memcpy(p_domain->dscp2color, p_db_qos_domain->dscp2color, sizeof(p_db_qos_domain->dscp2color));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2PHB:
        sal_memcpy(p_domain->dscp2phb, p_db_qos_domain->dscp2phb, sizeof(p_db_qos_domain->dscp2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2PRI:
        sal_memcpy(p_domain->prec2pri, p_db_qos_domain->prec2pri, sizeof(p_db_qos_domain->prec2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2COLOR:
        sal_memcpy(p_domain->prec2color, p_db_qos_domain->prec2color, sizeof(p_db_qos_domain->prec2color));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2PHB:
        sal_memcpy(p_domain->prec2phb, p_db_qos_domain->prec2phb, sizeof(p_db_qos_domain->prec2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2PRI:
        sal_memcpy(p_domain->exp2pri, p_db_qos_domain->exp2pri, sizeof(p_db_qos_domain->exp2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2COLOR:
        sal_memcpy(p_domain->exp2color, p_db_qos_domain->exp2color, sizeof(p_db_qos_domain->exp2color));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2PHB:
        sal_memcpy(p_domain->exp2phb, p_db_qos_domain->exp2phb, sizeof(p_db_qos_domain->exp2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS:
        sal_memcpy(p_domain->pri_color2cos, p_db_qos_domain->pri_color2cos, sizeof(p_db_qos_domain->pri_color2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP:
        sal_memcpy(p_domain->pri_color2dscp, p_db_qos_domain->pri_color2dscp, sizeof(p_db_qos_domain->pri_color2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP:
        sal_memcpy(p_domain->pri_color2exp, p_db_qos_domain->pri_color2exp, sizeof(p_db_qos_domain->pri_color2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2COS:
        sal_memcpy(p_domain->phb2cos, p_db_qos_domain->phb2cos, sizeof(p_db_qos_domain->phb2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2DSCP:
        sal_memcpy(p_domain->phb2dscp, p_db_qos_domain->phb2dscp, sizeof(p_db_qos_domain->phb2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2EXP:
        sal_memcpy(p_domain->phb2exp, p_db_qos_domain->phb2exp, sizeof(p_db_qos_domain->phb2exp));
        break;
 
    case TBL_QOS_DOMAIN_FLD_COS2TC:
        sal_memcpy(p_domain->cos2tc, p_db_qos_domain->cos2tc, sizeof(p_db_qos_domain->cos2tc));
        break;
        
    case TBL_QOS_DOMAIN_FLD_DSCP2TC:
        sal_memcpy(p_domain->dscp2tc, p_db_qos_domain->dscp2tc, sizeof(p_db_qos_domain->dscp2tc));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2COS:
        sal_memcpy(p_domain->tc_color2cos, p_db_qos_domain->tc_color2cos, sizeof(p_db_qos_domain->tc_color2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP:
        sal_memcpy(p_domain->tc_color2dscp, p_db_qos_domain->tc_color2dscp, sizeof(p_db_qos_domain->tc_color2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2TC:
        sal_memcpy(p_domain->exp2tc, p_db_qos_domain->exp2tc, sizeof(p_db_qos_domain->exp2tc));
        break;
        
    case TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP:
        sal_memcpy(p_domain->tc_color2exp, p_db_qos_domain->tc_color2exp, sizeof(p_db_qos_domain->tc_color2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_MAX:
        sal_memcpy(p_domain, p_db_qos_domain, sizeof(tbl_qos_domain_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}

tbl_qos_domain_t*
tbl_qos_domain_get_qos_domain_by_id(uint8 domain_id)
{
    tbl_qos_domain_key_t key;
    key.id = domain_id;

    if (domain_id == GLB_QOS_DOMAIN_NUM)
    {
        return _g_p_tbl_qos_domain_master->default_domain;
    }
    else
    {
        return tbl_qos_domain_get_qos_domain(&key);
    }
}

