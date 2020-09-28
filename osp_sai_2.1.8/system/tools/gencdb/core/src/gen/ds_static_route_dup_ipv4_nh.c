
#include "proto.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "cdb_data_cmp.h"

int32
ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh_sync(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, uint32 sync)
{
    tbl_arp_t *p_db_arp = NULL;
    ds_static_route_dup_ipv4_nh_t *p_db_static_route_dup_ipv4_nh = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_static_route_dup_ipv4_nh = ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(p_arp, p_static_route_dup_ipv4_nh);
    if (p_db_static_route_dup_ipv4_nh)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_static_route_dup_ipv4_nh = XCALLOC(MEM_DS_STATIC_ROUTE_DUP_IPV4_NH, sizeof(ds_static_route_dup_ipv4_nh_t));
    if (NULL == p_db_static_route_dup_ipv4_nh)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_static_route_dup_ipv4_nh, p_static_route_dup_ipv4_nh, sizeof(ds_static_route_dup_ipv4_nh_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_arp->static_route_dup_ipv4_nh, p_db_static_route_dup_ipv4_nh));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_ARP, DS_STATIC_ROUTE_DUP_IPV4_NH, p_db_arp, p_db_static_route_dup_ipv4_nh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_ARP, DS_STATIC_ROUTE_DUP_IPV4_NH, p_db_arp, p_db_static_route_dup_ipv4_nh);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh_sync(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, uint32 sync)
{
    tbl_arp_t *p_db_arp = NULL;
    ds_static_route_dup_ipv4_nh_t *p_db_static_route_dup_ipv4_nh = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_static_route_dup_ipv4_nh = ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(p_arp, p_static_route_dup_ipv4_nh);
    if (NULL == p_db_static_route_dup_ipv4_nh)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_ARP, DS_STATIC_ROUTE_DUP_IPV4_NH, p_db_arp, p_db_static_route_dup_ipv4_nh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_ARP, DS_STATIC_ROUTE_DUP_IPV4_NH, p_db_arp, p_db_static_route_dup_ipv4_nh);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_arp->static_route_dup_ipv4_nh, p_db_static_route_dup_ipv4_nh));

    /* 4. free db entry */
    XFREE(MEM_DS_STATIC_ROUTE_DUP_IPV4_NH, p_db_static_route_dup_ipv4_nh);

    return PM_E_NONE;
}

int32
ds_static_route_dup_ipv4_nh_set_static_route_dup_ipv4_nh_field_sync(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, ds_static_route_dup_ipv4_nh_field_id_t field_id, uint32 sync)
{
    tbl_arp_t *p_db_arp = NULL;
    ds_static_route_dup_ipv4_nh_t *p_db_static_route_dup_ipv4_nh = NULL;

    /* 1. lookup entry exist */
    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_static_route_dup_ipv4_nh = ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(p_arp, p_static_route_dup_ipv4_nh);
    if (NULL == p_db_static_route_dup_ipv4_nh)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_MAX:
        sal_memcpy(p_db_static_route_dup_ipv4_nh, p_static_route_dup_ipv4_nh, sizeof(ds_static_route_dup_ipv4_nh_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_ARP, DS_STATIC_ROUTE_DUP_IPV4_NH, field_id, p_db_arp, p_db_static_route_dup_ipv4_nh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_ARP, DS_STATIC_ROUTE_DUP_IPV4_NH, field_id, p_db_arp, p_db_static_route_dup_ipv4_nh);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh)
{
    return ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh_sync(p_arp, p_static_route_dup_ipv4_nh, TRUE);
}

int32
ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh)
{
    return ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh_sync(p_arp, p_static_route_dup_ipv4_nh, TRUE);
}

int32
ds_static_route_dup_ipv4_nh_set_static_route_dup_ipv4_nh_field(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, ds_static_route_dup_ipv4_nh_field_id_t field_id)
{
    return ds_static_route_dup_ipv4_nh_set_static_route_dup_ipv4_nh_field_sync(p_arp, p_static_route_dup_ipv4_nh, field_id, TRUE);
}

ds_static_route_dup_ipv4_nh_t*
ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh)
{
    tbl_arp_t *p_db_arp = NULL;

    /* 1. lookup entry exist */
    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_arp->static_route_dup_ipv4_nh, p_static_route_dup_ipv4_nh);
}

int32
ds_static_route_dup_ipv4_nh_key_str2val(char* tbl_key_value, char* str, tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh)
{

    int32 ret = 0;

    ret = tbl_arp_key_str2val(tbl_key_value, p_arp);

    ret = cdb_addr_ipv4_str2val(&p_static_route_dup_ipv4_nh->key.address, str, 0);

    return ret;
}

char*
ds_static_route_dup_ipv4_nh_key_value_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, char* str)
{


    cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_static_route_dup_ipv4_nh->key.address);
    return str;
}

char*
ds_static_route_dup_ipv4_nh_key_name_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_ARP);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_STATIC_ROUTE_DUP_IPV4_NH);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_arp_key_value_dump(p_arp, tbl_str), p_node_ds->name, ds_static_route_dup_ipv4_nh_key_value_dump(p_arp, p_static_route_dup_ipv4_nh, ds_str));
    return str;
}

char*
ds_static_route_dup_ipv4_nh_field_name_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_STATIC_ROUTE_DUP_IPV4_NH);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_static_route_dup_ipv4_nh_field_value_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_static_route_dup_ipv4_nh->key.address);
    }
    return str;
}

char**
ds_static_route_dup_ipv4_nh_table_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_static_route_dup_ipv4_nh_key_name_dump(p_arp, p_static_route_dup_ipv4_nh, buf));
    for(i=1; i<DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_static_route_dup_ipv4_nh_field_name_dump(p_arp, p_static_route_dup_ipv4_nh, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_static_route_dup_ipv4_nh_field_value_dump(p_arp, p_static_route_dup_ipv4_nh, i, buf));
    }
    return str;
}

int32
ds_static_route_dup_ipv4_nh_field_value_parser(char *str, int32 field_id, tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh)
{
    int32 ret=0;
    if (FLD_MATCH(DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_str2val(&p_static_route_dup_ipv4_nh->key.address, str, 0);
    }
    return ret;
}

int32
ds_static_route_dup_ipv4_nh_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_STATIC_ROUTE_DUP_IPV4_NH);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_static_route_dup_ipv4_nh_key_str2val(tbl_key_value, ds_key_value, p_arp, p_static_route_dup_ipv4_nh));

    for(i=1; i<DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_static_route_dup_ipv4_nh_field_value_parser( array[j++], i, p_arp, p_static_route_dup_ipv4_nh));
    }

    return PM_E_NONE;
}

int32
ds_static_route_dup_ipv4_nh_dump_one(ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_STATIC_ROUTE_DUP_IPV4_NH);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_KEY].name,
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_static_route_dup_ipv4_nh->key.address));
    }

    return PM_E_NONE;
}
