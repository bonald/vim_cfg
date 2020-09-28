
#include "proto.h"
#include "gen/tbl_fea_pcap_define.h"
#include "gen/tbl_fea_pcap.h"
#include "cdb_data_cmp.h"

tbl_fea_pcap_t *_g_p_tbl_fea_pcap = NULL;

int32
tbl_fea_pcap_set_fea_pcap_field_sync(tbl_fea_pcap_t *p_pcap, tbl_fea_pcap_field_id_t field_id, uint32 sync)
{
    tbl_fea_pcap_t *p_db_pcap = _g_p_tbl_fea_pcap;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_PCAP_FLD_TX_EN:
        p_db_pcap->tx_en = p_pcap->tx_en;
        break;

    case TBL_FEA_PCAP_FLD_RX_EN:
        p_db_pcap->rx_en = p_pcap->rx_en;
        break;

    case TBL_FEA_PCAP_FLD_IFINDEX:
        p_db_pcap->ifindex = p_pcap->ifindex;
        break;

    case TBL_FEA_PCAP_FLD_MAX:
        sal_memcpy(p_db_pcap, p_pcap, sizeof(tbl_fea_pcap_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_PCAP, field_id, p_db_pcap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_PCAP, field_id, p_db_pcap);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_pcap_set_fea_pcap_field(tbl_fea_pcap_t *p_pcap, tbl_fea_pcap_field_id_t field_id)
{
    return tbl_fea_pcap_set_fea_pcap_field_sync(p_pcap, field_id, TRUE);
}

tbl_fea_pcap_t*
tbl_fea_pcap_get_fea_pcap()
{
    return _g_p_tbl_fea_pcap;
}

int32
tbl_fea_pcap_key_str2val(char *str, tbl_fea_pcap_t *p_pcap)
{
    return PM_E_NONE;
}

char*
tbl_fea_pcap_key_name_dump(tbl_fea_pcap_t *p_pcap, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PCAP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_fea_pcap_key_value_dump(tbl_fea_pcap_t *p_pcap, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_fea_pcap_field_name_dump(tbl_fea_pcap_t *p_pcap, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PCAP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_PCAP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_pcap_field_value_dump(tbl_fea_pcap_t *p_pcap, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_TX_EN, field_id))
    {
        sal_sprintf(str, "%u", p_pcap->tx_en);
    }
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_RX_EN, field_id))
    {
        sal_sprintf(str, "%u", p_pcap->rx_en);
    }
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_pcap->ifindex);
    }
    return str;
}

char**
tbl_fea_pcap_table_dump(tbl_fea_pcap_t *p_pcap, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_pcap_key_name_dump(p_pcap, buf));
    for(i=1; i<TBL_FEA_PCAP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_pcap_field_name_dump(p_pcap, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_pcap_field_value_dump(p_pcap, i, buf));
    }
    return str;
}

int32
tbl_fea_pcap_field_value_parser(char *str, int32 field_id, tbl_fea_pcap_t *p_pcap)
{
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_TX_EN, field_id))
    {
        int32 ret;
        p_pcap->tx_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_RX_EN, field_id))
    {
        int32 ret;
        p_pcap->rx_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_pcap->ifindex = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_pcap_table_parser(char** array, char* key_value,tbl_fea_pcap_t *p_pcap)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PCAP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_pcap_key_str2val(key_value, p_pcap));

    for(i=1; i<TBL_FEA_PCAP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_pcap_field_value_parser( array[j++], i, p_pcap));
    }

    return PM_E_NONE;
}

int32
tbl_fea_pcap_dump_one(tbl_fea_pcap_t *p_pcap, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PCAP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_PCAP_FLD_TX_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PCAP_FLD_TX_EN].name,
            p_pcap->tx_en);
    }
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_RX_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PCAP_FLD_RX_EN].name,
            p_pcap->rx_en);
    }
    if (FLD_MATCH(TBL_FEA_PCAP_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PCAP_FLD_IFINDEX].name,
            p_pcap->ifindex);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_pcap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_pcap_t *p_db_pcap = _g_p_tbl_fea_pcap;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_pcap)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_pcap, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_pcap_t*
tbl_fea_pcap_init_fea_pcap()
{
    _g_p_tbl_fea_pcap = XCALLOC(MEM_TBL_FEA_PCAP, sizeof(tbl_fea_pcap_t));
    return _g_p_tbl_fea_pcap;
}

