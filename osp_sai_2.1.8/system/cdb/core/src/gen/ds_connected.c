
#include "proto.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "cdb_data_cmp.h"

int32
ds_connected_add_connected_sync(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_connect = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_connect = ds_connected_get_connected(p_rtif, p_connect);
    if (p_db_connect)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_connect = XCALLOC(MEM_DS_CONNECTED, sizeof(ds_connected_t));
    if (NULL == p_db_connect)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_connect, p_connect, sizeof(ds_connected_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_rtif->ifc_ipv4, p_db_connect));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_ROUTE_IF, DS_CONNECTED, p_db_rtif, p_db_connect);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_ROUTE_IF, DS_CONNECTED, p_db_rtif, p_db_connect);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_connected_del_connected_sync(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_connect = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_connect = ds_connected_get_connected(p_rtif, p_connect);
    if (NULL == p_db_connect)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_ROUTE_IF, DS_CONNECTED, p_db_rtif, p_db_connect);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_ROUTE_IF, DS_CONNECTED, p_db_rtif, p_db_connect);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_rtif->ifc_ipv4, p_db_connect));

    /* 4. free db entry */
    XFREE(MEM_DS_CONNECTED, p_db_connect);

    return PM_E_NONE;
}

int32
ds_connected_set_connected_field_sync(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, ds_connected_field_id_t field_id, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_connect = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_connect = ds_connected_get_connected(p_rtif, p_connect);
    if (NULL == p_db_connect)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_CONNECTED_FLD_DESTINATION:
        sal_memcpy(&p_db_connect->destination, &p_connect->destination, sizeof(p_connect->destination));
        break;

    case DS_CONNECTED_FLD_FLAGS_SECONDARY:
        CDB_FLAG_CHECK_SET(p_db_connect, p_connect, flags, RT_IF_ADDR_SECONDARY)
        break;

    case DS_CONNECTED_FLD_FLAGS_ANYCAST:
        CDB_FLAG_CHECK_SET(p_db_connect, p_connect, flags, RT_IF_ADDR_ANYCAST)
        break;

    case DS_CONNECTED_FLD_FLAGS_VIRTUAL:
        CDB_FLAG_CHECK_SET(p_db_connect, p_connect, flags, RT_IF_ADDR_VIRTUAL)
        break;

    case DS_CONNECTED_FLD_FLAGS_DUPLICATE:
        CDB_FLAG_CHECK_SET(p_db_connect, p_connect, flags, RT_IF_ADDR_DUPLICATE)
        break;

    case DS_CONNECTED_FLD_INSTALLED:
        p_db_connect->installed = p_connect->installed;
        break;

    case DS_CONNECTED_FLD_MAX:
        sal_memcpy(p_db_connect, p_connect, sizeof(ds_connected_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_ROUTE_IF, DS_CONNECTED, field_id, p_db_rtif, p_db_connect);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_ROUTE_IF, DS_CONNECTED, field_id, p_db_rtif, p_db_connect);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_connected_add_connected(tbl_route_if_t *p_rtif, ds_connected_t *p_connect)
{
    return ds_connected_add_connected_sync(p_rtif, p_connect, TRUE);
}

int32
ds_connected_del_connected(tbl_route_if_t *p_rtif, ds_connected_t *p_connect)
{
    return ds_connected_del_connected_sync(p_rtif, p_connect, TRUE);
}

int32
ds_connected_set_connected_field(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, ds_connected_field_id_t field_id)
{
    return ds_connected_set_connected_field_sync(p_rtif, p_connect, field_id, TRUE);
}

ds_connected_t*
ds_connected_get_connected(tbl_route_if_t *p_rtif, ds_connected_t *p_connect)
{
    tbl_route_if_t *p_db_rtif = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_rtif->ifc_ipv4, p_connect);
}

int32
ds_connected_key_str2val(char* tbl_key_value, char* str, tbl_route_if_t *p_rtif, ds_connected_t *p_connect)
{

    int32 ret = 0;

    ret = tbl_route_if_key_str2val(tbl_key_value, p_rtif);

    ret = cdb_prefix_str2val_wo_masklen(str, &p_connect->key.address);

    return ret;
}

char*
ds_connected_key_value_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, char* str)
{


    cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_connect->key.address);
    return str;
}

char*
ds_connected_key_name_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_ROUTE_IF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_CONNECTED);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_route_if_key_value_dump(p_rtif, tbl_str), p_node_ds->name, ds_connected_key_value_dump(p_rtif, p_connect, ds_str));
    return str;
}

char*
ds_connected_field_name_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_CONNECTED);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_CONNECTED_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_connected_field_value_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_CONNECTED_FLD_KEY, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_connect->key.address);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_DESTINATION, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_connect->destination);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_SECONDARY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_SECONDARY));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_ANYCAST, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_ANYCAST));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_VIRTUAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_VIRTUAL));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_DUPLICATE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_DUPLICATE));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_INSTALLED, field_id))
    {
        sal_sprintf(str, "%u", p_connect->installed);
    }
    return str;
}

char**
ds_connected_table_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_connected_key_name_dump(p_rtif, p_connect, buf));
    for(i=1; i<DS_CONNECTED_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_connected_field_name_dump(p_rtif, p_connect, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_connected_field_value_dump(p_rtif, p_connect, i, buf));
    }
    return str;
}

int32
ds_connected_field_value_parser(char *str, int32 field_id, tbl_route_if_t *p_rtif, ds_connected_t *p_connect)
{
    int32 ret=0;
    if (FLD_MATCH(DS_CONNECTED_FLD_KEY, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_connect->key.address);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_DESTINATION, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_connect->destination);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_SECONDARY, field_id))
    {
        GLB_SET_FLAG(p_connect->flags, RT_IF_ADDR_SECONDARY);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_ANYCAST, field_id))
    {
        GLB_SET_FLAG(p_connect->flags, RT_IF_ADDR_ANYCAST);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_VIRTUAL, field_id))
    {
        GLB_SET_FLAG(p_connect->flags, RT_IF_ADDR_VIRTUAL);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_DUPLICATE, field_id))
    {
        GLB_SET_FLAG(p_connect->flags, RT_IF_ADDR_DUPLICATE);
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_INSTALLED, field_id))
    {
        p_connect->installed = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_connected_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_route_if_t *p_rtif, ds_connected_t *p_connect)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_CONNECTED);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_connected_key_str2val(tbl_key_value, ds_key_value, p_rtif, p_connect));

    for(i=1; i<DS_CONNECTED_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_connected_field_value_parser( array[j++], i, p_rtif, p_connect));
    }

    return PM_E_NONE;
}

int32
ds_connected_dump_one(ds_connected_t *p_connect, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_CONNECTED);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_CONNECTED_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CONNECTED_FLD_KEY].name,
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_connect->key.address));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_DESTINATION, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CONNECTED_FLD_DESTINATION].name,
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_connect->destination));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_SECONDARY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_FLD_FLAGS_SECONDARY].name,
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_SECONDARY));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_ANYCAST, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_FLD_FLAGS_ANYCAST].name,
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_ANYCAST));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_VIRTUAL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_FLD_FLAGS_VIRTUAL].name,
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_VIRTUAL));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_FLAGS_DUPLICATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_FLD_FLAGS_DUPLICATE].name,
            GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_DUPLICATE));
    }
    if (FLD_MATCH(DS_CONNECTED_FLD_INSTALLED, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_FLD_INSTALLED].name,
            p_connect->installed);
    }

    return PM_E_NONE;
}

int32
route_if_addr_exists(addr_t *p_addr)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_conn = NULL;

    if (AF_INET == p_addr->family)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode1)
            {
                if ((p_db_conn->key.address.family == p_addr->family)
                 && (p_db_conn->key.address.u.prefix4.s_addr == p_addr->u.prefix4.s_addr))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

