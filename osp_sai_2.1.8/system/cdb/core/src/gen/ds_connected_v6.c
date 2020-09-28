
#include "proto.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "cdb_data_cmp.h"

int32
ds_connected_v6_add_connected_v6_sync(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_v6_t *p_db_connect_v6 = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_connect_v6 = ds_connected_v6_get_connected_v6(p_rtif, p_connect_v6);
    if (p_db_connect_v6)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_connect_v6 = XCALLOC(MEM_DS_CONNECTED_V6, sizeof(ds_connected_v6_t));
    if (NULL == p_db_connect_v6)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_connect_v6, p_connect_v6, sizeof(ds_connected_v6_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_rtif->ifc_ipv6, p_db_connect_v6));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_ROUTE_IF, DS_CONNECTED_V6, p_db_rtif, p_db_connect_v6);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_ROUTE_IF, DS_CONNECTED_V6, p_db_rtif, p_db_connect_v6);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_connected_v6_del_connected_v6_sync(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_v6_t *p_db_connect_v6 = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_connect_v6 = ds_connected_v6_get_connected_v6(p_rtif, p_connect_v6);
    if (NULL == p_db_connect_v6)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_ROUTE_IF, DS_CONNECTED_V6, p_db_rtif, p_db_connect_v6);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_ROUTE_IF, DS_CONNECTED_V6, p_db_rtif, p_db_connect_v6);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_rtif->ifc_ipv6, p_db_connect_v6));

    /* 4. free db entry */
    XFREE(MEM_DS_CONNECTED_V6, p_db_connect_v6);

    return PM_E_NONE;
}

int32
ds_connected_v6_set_connected_v6_field_sync(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, ds_connected_v6_field_id_t field_id, uint32 sync)
{
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_v6_t *p_db_connect_v6 = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_connect_v6 = ds_connected_v6_get_connected_v6(p_rtif, p_connect_v6);
    if (NULL == p_db_connect_v6)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_CONNECTED_V6_FLD_DESTINATION:
        sal_memcpy(&p_db_connect_v6->destination, &p_connect_v6->destination, sizeof(p_connect_v6->destination));
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_SECONDARY:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDR_SECONDARY)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_ANYCAST:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDR_ANYCAST)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_VIRTUAL:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDR_VIRTUAL)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_DUPLICATE:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDR_DUPLICATE)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_GLOBAL)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_LINKLOCAL)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_AUTO_LINKLOCAL)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL_EUI64:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_GLOBAL_EUI64)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_DAD_IN_PROGRESS)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_NA_IN_PROGRESS:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_NA_IN_PROGRESS)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_CONFIGURED:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_CONFIGURED)
        break;

    case DS_CONNECTED_V6_FLD_FLAGS_V6_REAL:
        CDB_FLAG_CHECK_SET(p_db_connect_v6, p_connect_v6, flags, RT_IF_ADDRV6_REAL)
        break;

    case DS_CONNECTED_V6_FLD_INSTALLED:
        p_db_connect_v6->installed = p_connect_v6->installed;
        break;

    case DS_CONNECTED_V6_FLD_IFNAME:
        sal_memcpy(p_db_connect_v6->ifname, p_connect_v6->ifname, sizeof(p_connect_v6->ifname));
        break;

    case DS_CONNECTED_V6_FLD_IFINDEX:
        p_db_connect_v6->ifindex = p_connect_v6->ifindex;
        break;

    case DS_CONNECTED_V6_FLD_T_TIMER:
        p_db_connect_v6->t_timer = p_connect_v6->t_timer;
        break;

    case DS_CONNECTED_V6_FLD_DAD_NS_SEND:
        p_db_connect_v6->dad_ns_send = p_connect_v6->dad_ns_send;
        break;

    case DS_CONNECTED_V6_FLD_UNSOLICITED_NA:
        p_db_connect_v6->unsolicited_na = p_connect_v6->unsolicited_na;
        break;

    case DS_CONNECTED_V6_FLD_MAX:
        sal_memcpy(p_db_connect_v6, p_connect_v6, sizeof(ds_connected_v6_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_ROUTE_IF, DS_CONNECTED_V6, field_id, p_db_rtif, p_db_connect_v6);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_ROUTE_IF, DS_CONNECTED_V6, field_id, p_db_rtif, p_db_connect_v6);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_connected_v6_add_connected_v6(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6)
{
    return ds_connected_v6_add_connected_v6_sync(p_rtif, p_connect_v6, TRUE);
}

int32
ds_connected_v6_del_connected_v6(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6)
{
    return ds_connected_v6_del_connected_v6_sync(p_rtif, p_connect_v6, TRUE);
}

int32
ds_connected_v6_set_connected_v6_field(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, ds_connected_v6_field_id_t field_id)
{
    return ds_connected_v6_set_connected_v6_field_sync(p_rtif, p_connect_v6, field_id, TRUE);
}

ds_connected_v6_t*
ds_connected_v6_get_connected_v6(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6)
{
    tbl_route_if_t *p_db_rtif = NULL;

    /* 1. lookup entry exist */
    p_db_rtif = tbl_route_if_get_route_if(&p_rtif->key);
    if (NULL == p_db_rtif)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_rtif->ifc_ipv6, p_connect_v6);
}

int32
ds_connected_v6_key_str2val(char* tbl_key_value, char* str, tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6)
{

    int32 ret = 0;

    ret = tbl_route_if_key_str2val(tbl_key_value, p_rtif);

    ret = cdb_prefix_str2val_wo_masklen(str, &p_connect_v6->key.address);

    return ret;
}

char*
ds_connected_v6_key_value_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, char* str)
{


    cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_connect_v6->key.address);
    return str;
}

char*
ds_connected_v6_key_name_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_ROUTE_IF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_CONNECTED_V6);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_route_if_key_value_dump(p_rtif, tbl_str), p_node_ds->name, ds_connected_v6_key_value_dump(p_rtif, p_connect_v6, ds_str));
    return str;
}

char*
ds_connected_v6_field_name_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_CONNECTED_V6);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_CONNECTED_V6_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_connected_v6_field_value_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_CONNECTED_V6_FLD_KEY, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_connect_v6->key.address);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_DESTINATION, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_connect_v6->destination);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_SECONDARY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_SECONDARY));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_ANYCAST, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_ANYCAST));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_VIRTUAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_VIRTUAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_DUPLICATE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_DUPLICATE));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_LINKLOCAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL_EUI64, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL_EUI64));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_NA_IN_PROGRESS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_NA_IN_PROGRESS));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_CONFIGURED, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_CONFIGURED));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_REAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_REAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_INSTALLED, field_id))
    {
        sal_sprintf(str, "%u", p_connect_v6->installed);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_connect_v6->ifname);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_connect_v6->ifindex);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_T_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_connect_v6->t_timer);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_DAD_NS_SEND, field_id))
    {
        sal_sprintf(str, "%u", p_connect_v6->dad_ns_send);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_UNSOLICITED_NA, field_id))
    {
        sal_sprintf(str, "%u", p_connect_v6->unsolicited_na);
    }
    return str;
}

char**
ds_connected_v6_table_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_connected_v6_key_name_dump(p_rtif, p_connect_v6, buf));
    for(i=1; i<DS_CONNECTED_V6_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_connected_v6_field_name_dump(p_rtif, p_connect_v6, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_connected_v6_field_value_dump(p_rtif, p_connect_v6, i, buf));
    }
    return str;
}

int32
ds_connected_v6_field_value_parser(char *str, int32 field_id, tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6)
{
    int32 ret=0;
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_KEY, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_connect_v6->key.address);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_DESTINATION, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_connect_v6->destination);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_SECONDARY, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDR_SECONDARY);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_ANYCAST, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDR_ANYCAST);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_VIRTUAL, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDR_VIRTUAL);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_DUPLICATE, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDR_DUPLICATE);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_LINKLOCAL);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL_EUI64, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL_EUI64);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_NA_IN_PROGRESS, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_CONFIGURED, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_CONFIGURED);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_REAL, field_id))
    {
        GLB_SET_FLAG(p_connect_v6->flags, RT_IF_ADDRV6_REAL);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_INSTALLED, field_id))
    {
        p_connect_v6->installed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_connect_v6->ifname, str);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_IFINDEX, field_id))
    {
        p_connect_v6->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_T_TIMER, field_id))
    {
        ctc_task_str2val(str, p_connect_v6->t_timer);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_DAD_NS_SEND, field_id))
    {
        p_connect_v6->dad_ns_send = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_UNSOLICITED_NA, field_id))
    {
        p_connect_v6->unsolicited_na = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_connected_v6_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_CONNECTED_V6);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_connected_v6_key_str2val(tbl_key_value, ds_key_value, p_rtif, p_connect_v6));

    for(i=1; i<DS_CONNECTED_V6_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_connected_v6_field_value_parser( array[j++], i, p_rtif, p_connect_v6));
    }

    return PM_E_NONE;
}

int32
ds_connected_v6_dump_one(ds_connected_v6_t *p_connect_v6, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_CONNECTED_V6);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_CONNECTED_V6_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_KEY].name,
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_connect_v6->key.address));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_DESTINATION, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_DESTINATION].name,
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_connect_v6->destination));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_SECONDARY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_SECONDARY].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_SECONDARY));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_ANYCAST, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_ANYCAST].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_ANYCAST));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_VIRTUAL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_VIRTUAL].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_VIRTUAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_DUPLICATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_DUPLICATE].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_DUPLICATE));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_LINKLOCAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL_EUI64, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL_EUI64].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL_EUI64));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_NA_IN_PROGRESS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_NA_IN_PROGRESS].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_NA_IN_PROGRESS));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_CONFIGURED, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_CONFIGURED].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_CONFIGURED));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_FLAGS_V6_REAL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_FLAGS_V6_REAL].name,
            GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_REAL));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_INSTALLED, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_INSTALLED].name,
            p_connect_v6->installed);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_IFNAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_IFNAME].name,
            p_connect_v6->ifname);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_IFINDEX, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_IFINDEX].name,
            p_connect_v6->ifindex);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_T_TIMER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_T_TIMER].name,
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_connect_v6->t_timer));
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_DAD_NS_SEND, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_DAD_NS_SEND].name,
            p_connect_v6->dad_ns_send);
    }
    if (FLD_MATCH(DS_CONNECTED_V6_FLD_UNSOLICITED_NA, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CONNECTED_V6_FLD_UNSOLICITED_NA].name,
            p_connect_v6->unsolicited_na);
    }

    return PM_E_NONE;
}
