
#include "proto.h"
#include "cdb_sync.h"
#include "cdb_store.h"

#include "rdb_hiredis.h"
#include "gen/tbl_interface.h"

static char* g_p_array[CMD_BUF_512];

int32
cdb_sync_tbl_add(uint32 tbl_id, void *p_db_tbl)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data(CDB_OPER_ADD, CDB_NODE_TBL, tbl_id, 0, p_db_tbl, p_node->tbl_info->data_size);
}

int32
cdb_sync_tbl_del(uint32 tbl_id, void *p_db_tbl)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data(CDB_OPER_DEL, CDB_NODE_TBL, tbl_id, 0, p_db_tbl, p_node->tbl_info->key_size);
}

int32
cdb_sync_tbl_set(uint32 tbl_id, uint32 field_id, void *p_db_tbl)
{
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data(CDB_OPER_SET, CDB_NODE_TBL, tbl_id, field_id, p_db_tbl, p_node->tbl_info->data_size);
}

int32
cdb_sync_ds_add(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void *p_db_ds)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds = cdb_get_ds(ds_id);
    if (NULL == p_node_tbl || NULL == p_node_ds)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data_ds(CDB_OPER_ADD, CDB_NODE_DS, tbl_id, ds_id, 0, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, p_db_ds, p_node_ds->tbl_info->data_size);
}

int32
cdb_sync_ds_del(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void *p_db_ds)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds = cdb_get_ds(ds_id);
    if (NULL == p_node_tbl || NULL == p_node_ds)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data_ds(CDB_OPER_DEL, CDB_NODE_DS, tbl_id, ds_id, 0, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, p_db_ds, p_node_ds->tbl_info->data_size);
}

int32
cdb_sync_ds_set(uint32 tbl_id, uint32 ds_id, uint32 field_id, void *p_db_tbl, void *p_db_ds)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds = cdb_get_ds(ds_id);
    if (NULL == p_node_tbl || NULL == p_node_ds)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data_ds(CDB_OPER_SET, CDB_NODE_DS, tbl_id, ds_id, field_id, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, p_db_ds, p_node_ds->tbl_info->data_size);
    return PM_E_NONE;
}

int32
cdb_sync_ds2_add(uint32 tbl_id, uint32 ds_id1, uint32 ds_id2, void *p_db_tbl, void *p_db_ds1, void *p_db_ds2)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds1 = cdb_get_ds(ds_id1);
    cdb_node_t* p_node_ds2 = cdb_get_ds(ds_id2);
    if (NULL == p_node_tbl || NULL == p_node_ds1 || NULL == p_node_ds2)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data_ds2(CDB_OPER_ADD, CDB_NODE_DS, tbl_id, ds_id1, ds_id2, 0, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, 
        p_db_ds1, p_node_ds1->tbl_info->data_size, 
        p_db_ds2, p_node_ds2->tbl_info->data_size);
}

int32
cdb_sync_ds2_del(uint32 tbl_id, uint32 ds_id1, uint32 ds_id2, void *p_db_tbl, void *p_db_ds1, void *p_db_ds2)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds1 = cdb_get_ds(ds_id1);
    cdb_node_t* p_node_ds2 = cdb_get_ds(ds_id2);
    if (NULL == p_node_tbl || NULL == p_node_ds1 || NULL == p_node_ds2)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data_ds2(CDB_OPER_DEL, CDB_NODE_DS, tbl_id, ds_id1, ds_id2, 0, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, 
        p_db_ds1, p_node_ds1->tbl_info->data_size,
        p_db_ds2, p_node_ds2->tbl_info->data_size);
}

int32
cdb_sync_ds2_set(uint32 tbl_id, uint32 ds_id1, uint32 ds_id2, uint32 field_id, void *p_db_tbl, void *p_db_ds1, void *p_db_ds2)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds1 = cdb_get_ds(ds_id1);
    cdb_node_t* p_node_ds2 = cdb_get_ds(ds_id2);
    if (NULL == p_node_tbl || NULL == p_node_ds1 || NULL == p_node_ds2)
    {
        return PM_E_INVALID_PARAM;
    }

    return ipc_pm_to_cds_data_ds2(CDB_OPER_SET, CDB_NODE_DS, tbl_id, ds_id1, ds_id2, field_id, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, 
        p_db_ds1, p_node_ds1->tbl_info->data_size,
        p_db_ds2, p_node_ds2->tbl_info->data_size);
}

int32
cdb_sync_tbl_set_refer(uint32 tbl_id, uint32 field_id, void *p_db_tbl, 
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);

    return ipc_pm_to_cds_refer_tbl(CDB_NODE_TBL, tbl_id, field_id, 
        p_db_tbl, p_node_tbl->tbl_info->key_size,
        p_refer_hdr, p_refer_obj);

    return PM_E_NONE;
}

int32
cdb_sync_ds_set_refer(uint32 tbl_id, uint32 ds_id, uint32 field_id, void *p_db_tbl, void *p_db_ds, 
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    cdb_node_t* p_node_ds = cdb_get_ds(ds_id);

    return ipc_pm_to_cds_refer_ds(CDB_NODE_DS, tbl_id, ds_id, field_id, 
        p_db_tbl, p_node_tbl->tbl_info->key_size, p_db_ds, p_node_ds->tbl_info->key_size,
        p_refer_hdr, p_refer_obj);

    return PM_E_NONE;
}
#define _CDB_SYNC_RDB_
/*to define the function to dump data to rdb*/

static CDB_SUBSCRIBE_TBL_FUNC g_cdb_sync_tbl_sub_before_fn[TBL_MAX];
static CDB_SUBSCRIBE_TBL_FUNC g_cdb_sync_tbl_sub_after_fn[TBL_MAX];

int32
cdb_sync_tbl_filter(uint16 tbl_id)
{
    /*these table is created by thread*/
    if (TBL_FEA_FDB == tbl_id || TBL_FIBER == tbl_id || TBL_PORT == tbl_id || TBL_FEA_PTP_TS == tbl_id)
    {
        return PM_E_FAIL;
    }

    /*TBL_RSA field length is about 10K bytes*/
    if (TBL_RSA == tbl_id )
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32
cdb_sync_tbl_field_filter(uint16 tbl_id, uint32 field_id)
{
    /*these table is created by thread*/
    if (TBL_INTERFACE == tbl_id && 
        (TBL_INTERFACE_FLD_PHY_TYPE == field_id ||
        TBL_INTERFACE_FLD_OPER_DUPLEX == field_id ||
        TBL_INTERFACE_FLD_OPER_SPEED == field_id ||
        TBL_INTERFACE_FLD_PHY_STATUS == field_id))
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32 
cdb_sync_parser_channel_string(char* channel, cdb_node_t** p_node_tbl, cdb_node_t** p_node_ds)
{
    char *tbl_name;
    char *ds_name;
    char tmp[MAX_CMD_STR_LEN];

    if(NULL == channel || NULL == p_node_tbl)
    {
        return PM_E_INVALID_PARAM;
    }

    /*get tbl_id/ds_id/ds2_id from channel*/
    sal_strcpy(tmp, channel);
    tbl_name = sal_strstr(tmp, CHANNEL_STR);
    if(NULL == tbl_name)
    {
        return PM_E_INVALID_PARAM;
    }
    tbl_name = tbl_name + sal_strlen(CHANNEL_STR);
    ds_name = sal_strstr(tbl_name, KEY_CONNECT_STR);
    if(NULL == ds_name)
    {
        /*get p_node for table*/
        *p_node_tbl = cdb_get_tbl_by_name(tbl_name);
        if(NULL == *p_node_tbl)
        {
            return PM_E_NOT_EXIST;
        }
    }
    else
    {
        *ds_name = '\0'; /*for tbl_name*/
        *p_node_tbl = cdb_get_tbl_by_name(tbl_name);
        if(NULL == *p_node_tbl)
        {
            return PM_E_NOT_EXIST;
        }
        
        ds_name = ds_name + sal_strlen(KEY_CONNECT_STR);
        *p_node_ds = cdb_get_ds_by_name(ds_name);
        if(NULL == *p_node_ds)
        {
            return PM_E_NOT_EXIST;
        }

    }
    
    return PM_E_NONE;
}


int32 
cdb_sync_parser_publish_string(char* publish, uint32* oper, char* key, char* field)
{
    char *oper_str = NULL;
    char *key_str = NULL;
    char *field_name = NULL;
    char *field_value = NULL;
    char tmp[MAX_CMD_STR_LEN];


    if(NULL == publish || NULL == oper || NULL == field || NULL == key)
    {
        return PM_E_INVALID_PARAM;
    }

    sal_strcpy(tmp, publish);
    oper_str = tmp;
    key_str = sal_strstr(oper_str, RDB_PARTITION_CHAR);
    if(NULL == key_str)
    {
        return PM_E_CDB_SYNC_INVALID_CHANNEL;
    }
    *key_str = '\0';
    key_str = key_str + sal_strlen(RDB_PARTITION_CHAR);
    if(!sal_strcmp(oper_str, REDIS_PUBLISH_ACTION_ADD))
    {
        *oper = CDB_OPER_ADD;
    }
    else if(!sal_strcmp(oper_str, REDIS_PUBLISH_ACTION_SET))
    {
        *oper = CDB_OPER_SET;
    }
    else if(!sal_strcmp(oper_str, REDIS_PUBLISH_ACTION_GET))
    {
        *oper = CDB_OPER_GET;
    }
    else if(!sal_strcmp(oper_str, REDIS_PUBLISH_ACTION_DEL))
    {
        *oper = CDB_OPER_DEL;
    }    
    else 
    {
        return PM_E_CDB_SYNC_INVALID_OPER;
    }


    if (*oper == CDB_OPER_SET)
    {
        field_name = sal_strstr(key_str, RDB_PARTITION_CHAR);
        if(field_name == NULL)
        {
            return PM_E_CDB_SYNC_FIELD_NAME_NOT_EXIST;
        }
        *field_name = '\0';
        field_name = field_name + sal_strlen(RDB_PARTITION_CHAR);

        field_value = sal_strstr(field_name, RDB_PARTITION_CHAR);
        if(field_value) /*we publish field_value but not used in cdb*/
        {
            *field_value = '\0';
        }
        sal_strcpy(field, field_name);
        
    }
    
    sal_strcpy(key, key_str);

    return PM_E_NONE;
}

int32 
cdb_sync_parser_field_string(cdb_node_t* p_node_tbl, cdb_node_t* p_node_ds, char* field_name, uint32* field_id)
{
    cdb_field_t * p_field = NULL;  
    cdb_node_t * p_node;
    int32 field_ret;

    if(p_node_ds)
    {
        p_node = p_node_ds;
    }
    else
    {
        p_node = p_node_tbl;
    }

    field_ret = cdb_get_field_id(field_name, p_node, &p_field);
    if(field_ret < 0)
    {
        return PM_E_CDB_SYNC_INVALID_FIELD;
    }
    
    field_id[0] = field_ret;

    return PM_E_NONE;
}


/*
Note, there are 3 type key string
acl:acl1:mac_ace:1
interface:eth-0-1:bridge:eth-0-1
table_key:acl:acl1
ds_key:mac_ace:1
*/
int32 
cdb_sync_get_table_key(cdb_node_t* p_node_tbl, cdb_node_t* p_node_ds, char* key, 
         char* table_key, char* tbl_key_name, char* tbl_key_value, char* ds_key_name, char* ds_key_value)
{
    char tmp[MAX_CMD_STR_LEN];
    char *tbl_key_value_str = NULL;
    char *ds_key_name_str = NULL;
    char *ds_key_value_str = NULL;

    if(NULL == key || NULL == table_key )
    {
        return PM_E_INVALID_PARAM;
    }
    
    sal_strcpy(tmp, key);
    
    tbl_key_value_str = sal_strstr(tmp, KEY_CONNECT_STR);
    if(NULL == tbl_key_value_str)
    {
        return PM_E_CDB_SYNC_INVALID_KEY;
    }

    tbl_key_value_str = tbl_key_value_str + 1;
    ds_key_name_str = sal_strstr(tbl_key_value_str, KEY_CONNECT_STR);
    if(NULL == ds_key_name_str)
    {
        /*only tbl table, got table_key*/
        sal_strcpy(table_key, tmp);      
    }
    else
    {
        *ds_key_name_str = '\0';
        /*got table_key*/
        sal_strcpy(table_key, tmp);
    }

    *(tbl_key_value_str - 1)  = '\0';
    /*got tbl key name*/
    sal_strcpy(tbl_key_name, tmp);
    
    /*got tbl value name*/
    if(ds_key_name_str)
    {
        *ds_key_name_str = '\0';
        sal_strcpy(tbl_key_value, tbl_key_value_str); 
        ds_key_name_str = ds_key_name_str + 1;
        ds_key_value_str = sal_strstr(ds_key_name_str, KEY_CONNECT_STR);
        if(ds_key_value_str)
        {
           *ds_key_value_str = '\0'; 
           sal_strcpy(ds_key_name, ds_key_name_str); 
           ds_key_value_str = ds_key_value_str + 1;
           sal_strcpy(ds_key_value, ds_key_value_str); 
        }
        sal_strcpy(ds_key_name, ds_key_name_str); 
    }
    else
    {
        sal_strcpy(tbl_key_value, tbl_key_value_str);         
    }
   
    return PM_E_NONE;
}


int32 
cdb_sync_get_table_from_rdb(cdb_node_t* p_node_tbl, cdb_node_t* p_node_ds, char* key, 
               void* p_tbl, void* p_ds)
{
    int32 rc;
    char table_key[MAX_CMD_STR_LEN];
    char tbl_key_name[MAX_CMD_STR_LEN];
    char tbl_key_value[MAX_CMD_STR_LEN];
    char ds_key_name[MAX_CMD_STR_LEN];
    char ds_key_value[MAX_CMD_STR_LEN];

    if(NULL == p_node_tbl)
    {
        return PM_E_INVALID_PARAM;
    }

    /*1, get the table or ds from key, key is the whole key currently*/
    PM_E_RETURN(rdb_sync_get_table(0, key, g_p_array));

    PM_E_RETURN(cdb_sync_get_table_key(p_node_tbl, p_node_ds, key, table_key, 
                     tbl_key_name, tbl_key_value, ds_key_name, ds_key_value));
    /*2, get p_ds*/
    if(p_node_ds)
    {
        if(p_node_ds->tbl_info->tbl_op_db.parser_table)
        {
           p_node_ds->tbl_info->ds_op_db.parser_table(g_p_array, ds_key_value, p_tbl, p_ds); 
        }   
    }
    else /*get p_tbl*/
    {
        p_node_tbl->tbl_info->tbl_op_db.parser_table(g_p_array, tbl_key_value, p_tbl); 
    }
    

    return PM_E_NONE;
}

int 
cdb_sync_subscribe_callback(char* channel, char* pub_str, char** array)
{
    int32 rc;
    uint32 oper;
    void *p_tbl = NULL;
    void* p_ds = NULL;
    int32 data_size = 0;
    int32 key_size = 0;
    uint32 fields[MAX_FLD_DWORD_SZ]; /*just to adapter original cdb defination*/
    char key[MAX_CMD_STR_LEN];
    char field_name[MAX_CMD_STR_LEN];
    cdb_node_t *p_tbl_node = NULL;
    cdb_node_t *p_ds_node = NULL;

    if(NULL == channel || NULL == pub_str)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Subscribe fail, channel is NULL\n");
        return PM_E_NOT_INITED;
    }

    log_sys(M_MOD_LIB, E_DEBUG, "Subscribe channel %s\n %s\n", channel, pub_str);

    /*1, get tbl/ds/ds2 p_node from channel str*/
    PM_E_RETURN(cdb_sync_parser_channel_string(channel, &p_tbl_node, &p_ds_node));

    /*2, get oper/key/field from publish string*/
    PM_E_RETURN(cdb_sync_parser_publish_string(pub_str,&oper, key, field_name));
    if(oper == CDB_OPER_SET)
    {
        PM_E_RETURN(cdb_sync_parser_field_string(p_tbl_node, p_ds_node, field_name, fields));
    }

    /*3, get whole db from rdb by get */
    if(p_ds_node)
    {
        data_size = p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->data_size;
        p_tbl = sal_calloc(data_size);
        p_ds = p_tbl + p_tbl_node->tbl_info->key_size;
    }
    else
    {   data_size = p_tbl_node->tbl_info->data_size;
        p_tbl = sal_calloc(data_size);
        key_size = p_tbl_node->tbl_info->key_size;
    }
    PM_E_RETURN(cdb_sync_get_table_from_rdb(p_tbl_node, p_ds_node, key, p_tbl, p_ds));
    
    /*4, process data before store data to local*/
    if(g_cdb_sync_tbl_sub_before_fn[p_tbl_node->id])
    {
        g_cdb_sync_tbl_sub_before_fn[p_tbl_node->id](p_tbl_node, p_ds_node, oper, fields[0], p_tbl, p_ds);
    }
    /*5, store data to local cdb*/
    if(!p_ds_node)
    {
        if(oper == CDB_OPER_DEL)
        {
            cdb_store_tbl(oper, fields, p_tbl_node, p_tbl, key_size);
        }
        else
        {
            cdb_store_tbl(oper, fields, p_tbl_node, p_tbl, data_size);
        }
    }
    else
    {
        PM_E_RETURN(cdb_store_ds(oper, fields, p_tbl_node, p_ds_node, NULL, p_tbl, data_size));
    }

    /*6, process data after store data to local*/
    if(g_cdb_sync_tbl_sub_after_fn[p_tbl_node->id])
    {
        g_cdb_sync_tbl_sub_after_fn[p_tbl_node->id](p_tbl_node, p_ds_node, oper, fields[0], p_tbl, p_ds);
    }

    sal_free(p_tbl);
    
    return PM_E_NONE;
}


char*
cdb_sync_tbl_channel_name_dump(cdb_tbl_t tbl_id, char* str)
{
    cdb_node_t *p_node = cdb_get_tbl(tbl_id);
    if(!p_node)
    {
        return NULL;
    }

    sal_sprintf(str, CHANNEL_STR"%s", p_node->name);
    return str;
}

char*
cdb_sync_tbl_table_name_dump(cdb_tbl_t tbl_id, char* str)
{
    cdb_node_t *p_node = cdb_get_tbl(tbl_id);

    sal_sprintf(str, TABLE_STR"%s", p_node->name);
    return str;
}

char*
cdb_sync_ds_channel_name_dump(cdb_tbl_t tbl_id, cdb_ds_t ds_id, char* str)
{
    cdb_node_t *p_node = cdb_get_tbl(tbl_id);
    cdb_node_t *p_node_ds = cdb_get_ds(ds_id);
    if(!p_node || !p_node_ds)
    {
        return NULL;
    }

    sal_sprintf(str, CHANNEL_STR"%s"KEY_CONNECT_STR"%s", p_node->name, p_node_ds->name);
    return str;
}

char*
cdb_sync_ds_table_name_dump(cdb_tbl_t tbl_id, cdb_ds_t ds_id, char* str)
{
    cdb_node_t *p_node = cdb_get_tbl(tbl_id);
    cdb_node_t *p_node_ds = cdb_get_ds(ds_id);
    if(!p_node || !p_node_ds)
    {
        return NULL;
    }

    sal_sprintf(str, TABLE_STR"%s"KEY_CONNECT_STR"%s", p_node->name, p_node_ds->name);
    return str;
}

int32
cdb_sync_subscribe_channel(cdb_tbl_t tbl_id, CDB_SUBSCRIBE_TBL_FUNC before_fn, CDB_SUBSCRIBE_TBL_FUNC after_fn)
{
    char str_channel_name[MAX_CMD_STR_LEN];

/*
    if(NULL == cdb_sync_tbl_func[tbl_id])
    {
        return PM_E_NOT_INITED;
    }
*/
    /*subscribe channel, register the process func to callback*/
    rdb_subscribe_channel(cdb_sync_tbl_channel_name_dump(tbl_id, str_channel_name), (void*)cdb_sync_subscribe_callback);


    /*register the callback for module*/
    g_cdb_sync_tbl_sub_before_fn[tbl_id] = before_fn;
    g_cdb_sync_tbl_sub_after_fn[tbl_id] = after_fn;

    return PM_E_NONE;
}

int32
cdb_sync_publish_tbl_add(uint32 tbl_id, void *p_db_tbl)
{
    int32 ret;
    char str_channel_name[MAX_CMD_STR_LEN];
    char str_table_name[MAX_CMD_STR_LEN];
    char str_key_name[MAX_CMD_STR_LEN];
    char str_key_value[MAX_CMD_STR_LEN];
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if(NULL == p_tbl_info)
    {
        return PM_E_NOT_INITED;
    }

    if(cdb_sync_tbl_filter(tbl_id))
    {
        return PM_E_NOT_PERMIT;
    }

    /*prepare the argc and argv for dbapi */
    /*array[0] is used to set DB action*/
    /*array[1]-array[n] is used to set table data*/
    p_tbl_info->tbl_op_db.dump_table(p_db_tbl, &g_p_array[1]);

    ret = rdb_publish_add_table(cdb_sync_tbl_channel_name_dump(tbl_id, str_channel_name),
            cdb_sync_tbl_table_name_dump(tbl_id, str_table_name),
            p_tbl_info->tbl_op_db.dump_key_name(p_db_tbl, str_key_name),
            p_tbl_info->tbl_op_db.dump_key_value(p_db_tbl, str_key_value),
            p_tbl_info->field_num*2, /*(table->field_num-1)*2 + 1(table_key) + 1(db_action)*/
            g_p_array);
    
    return ret;
}

int32
cdb_sync_publish_ds_add(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void* p_db_ds)
{
    int32 ret;
    char str_channel_name[MAX_CMD_STR_LEN];
    char str_table_name[MAX_CMD_STR_LEN];
    char str_key_name[MAX_CMD_STR_LEN];
    char str_key_value[MAX_CMD_STR_LEN];
    cdb_node_t* p_node = cdb_get_ds(ds_id);
    cdb_tbl_info_t *p_ds_info = p_node->tbl_info;
    
    if(NULL == p_ds_info)
    {
        return PM_E_NOT_INITED;
    }

    if(cdb_sync_tbl_filter(tbl_id))
    {
        return PM_E_NOT_PERMIT;
    }

    /*prepare the argc and argv for dbapi */
    /*array[0] is used to set DB action*/
    /*array[1]-array[n] is used to set table data*/
    p_ds_info->ds_op_db.dump_table(p_db_tbl, p_db_ds, &g_p_array[1]);

    ret = rdb_publish_add_table(cdb_sync_ds_channel_name_dump(tbl_id, ds_id, str_channel_name),
            cdb_sync_ds_table_name_dump(tbl_id, ds_id, str_table_name),
            p_ds_info->ds_op_db.dump_key_name(p_db_tbl, p_db_ds, str_key_name),
            p_ds_info->ds_op_db.dump_key_value(p_db_tbl, p_db_ds, str_key_value),
            p_ds_info->field_num*2, /*(table->field_num-1)*2 + 1(table_key) + 1(db_action)*/
            g_p_array);
    
    return ret;
}


int32
cdb_sync_publish_tbl_set(uint32 tbl_id, uint32 field_id, void *p_db_tbl)
{
    char str_channel_name[MAX_CMD_STR_LEN];
    char str_table_name[MAX_CMD_STR_LEN];
    char str_key_name[MAX_CMD_STR_LEN];
    char str_key_value[MAX_CMD_STR_LEN];
    char str_field_name[MAX_CMD_STR_LEN];
    char str_field_value[MAX_CMD_STR_LEN];
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    int ret;

    if(NULL == p_tbl_info)
    {
        return PM_E_NOT_INITED;
    }

    if(cdb_sync_tbl_filter(tbl_id))
    {
        return PM_E_NOT_PERMIT;
    }

    if(cdb_sync_tbl_field_filter(tbl_id, field_id))
    {
        return PM_E_NOT_PERMIT;
    }

    if(NULL == p_tbl_info->tbl_op_db.dump_field_name(p_db_tbl, field_id, str_field_name))
    {
        return PM_E_NOT_SUPPORT;
    }
    
    /*prepare the argc and argv for dbapi */
    ret = rdb_publish_set_table(cdb_sync_tbl_channel_name_dump(tbl_id, str_channel_name),
            cdb_sync_tbl_table_name_dump(tbl_id, str_table_name),
            p_tbl_info->tbl_op_db.dump_key_name(p_db_tbl, str_key_name),
            p_tbl_info->tbl_op_db.dump_key_value(p_db_tbl, str_key_value),
            p_tbl_info->tbl_op_db.dump_field_name(p_db_tbl, field_id, str_field_name),
            p_tbl_info->tbl_op_db.dump_field_value(p_db_tbl, field_id, str_field_value));

    return ret;
}

int32
cdb_sync_publish_ds_set(uint32 tbl_id, uint32 ds_id, uint32 field_id, void *p_db_tbl, void *p_db_ds)
{
    int32 ret;
    char str_channel_name[MAX_CMD_STR_LEN];
    char str_table_name[MAX_CMD_STR_LEN];
    char str_key_name[MAX_CMD_STR_LEN];
    char str_key_value[MAX_CMD_STR_LEN];
    char str_field_name[MAX_CMD_STR_LEN];
    char str_field_value[MAX_CMD_STR_LEN];
    cdb_node_t* p_node = cdb_get_ds(ds_id);
    cdb_tbl_info_t *p_ds_info = p_node->tbl_info;
    
    if(NULL == p_ds_info)
    {
        return PM_E_NOT_INITED;
    }

    if(cdb_sync_tbl_field_filter(ds_id, field_id))
    {
        return PM_E_NOT_PERMIT;
    }

    if(cdb_sync_tbl_filter(tbl_id))
    {
        return PM_E_NOT_PERMIT;
    }
    
    if(NULL == p_ds_info->ds_op_db.dump_field_name(p_db_tbl, p_db_ds, field_id, str_field_name))
    {
        return PM_E_NOT_SUPPORT;
    }
    
    /*prepare the argc and argv for dbapi */
    ret = rdb_publish_set_table(cdb_sync_ds_channel_name_dump(tbl_id, ds_id, str_channel_name),
            cdb_sync_ds_table_name_dump(tbl_id, ds_id, str_table_name),
            p_ds_info->ds_op_db.dump_key_name(p_db_tbl, p_db_ds, str_key_name),
            p_ds_info->ds_op_db.dump_key_value(p_db_tbl, p_db_ds, str_key_value),
            p_ds_info->ds_op_db.dump_field_name(p_db_tbl, p_db_ds, field_id, str_field_name),
            p_ds_info->ds_op_db.dump_field_value(p_db_tbl, p_db_ds, field_id, str_field_value));

    return ret;
}

int32
cdb_sync_publish_tbl_del(uint32 tbl_id, void *p_db_tbl)
{
    int32 ret;
    char str_channel_name[MAX_CMD_STR_LEN];
    char str_table_name[MAX_CMD_STR_LEN];
    char str_key_name[MAX_CMD_STR_LEN];
    char str_key_value[MAX_CMD_STR_LEN];
    cdb_node_t* p_node = cdb_get_tbl(tbl_id);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if(NULL == p_tbl_info)
    {
        return PM_E_NOT_INITED;
    }

    if(cdb_sync_tbl_filter(tbl_id))
    {
        return PM_E_NOT_PERMIT;
    }

    /*prepare the argc and argv for dbapi */
    ret = rdb_publish_del_table(cdb_sync_tbl_channel_name_dump(tbl_id, str_channel_name),
            cdb_sync_tbl_table_name_dump(tbl_id, str_table_name),
            p_tbl_info->tbl_op_db.dump_key_name(p_db_tbl, str_key_name),
            p_tbl_info->tbl_op_db.dump_key_value(p_db_tbl, str_key_value));

    return ret;
}

int32
cdb_sync_publish_ds_del(uint32 tbl_id, uint32 ds_id, void *p_db_tbl, void* p_db_ds)
{
    int32 ret;
    char str_channel_name[MAX_CMD_STR_LEN];
    char str_table_name[MAX_CMD_STR_LEN];
    char str_key_name[MAX_CMD_STR_LEN];
    char str_key_value[MAX_CMD_STR_LEN];
    cdb_node_t* p_node = cdb_get_ds(ds_id);
    cdb_tbl_info_t *p_ds_info = p_node->tbl_info;
    
    if(NULL == p_ds_info)
    {
        return PM_E_NOT_INITED;
    }

    if(cdb_sync_tbl_filter(tbl_id))
    {
        return PM_E_NOT_PERMIT;
    }

    /*prepare the argc and argv for dbapi */
    ret = rdb_publish_del_table(cdb_sync_ds_channel_name_dump(tbl_id, ds_id, str_channel_name),
            cdb_sync_ds_table_name_dump(tbl_id, ds_id, str_table_name),
            p_ds_info->ds_op_db.dump_key_name(p_db_tbl, p_db_ds, str_key_name),
            p_ds_info->ds_op_db.dump_key_value(p_db_tbl, p_db_ds, str_key_value));

    return ret;
}

int32
cdb_sync_publish_init()
{
    int32 i;

    for(i =0; i<CMD_BUF_512; i++)
    {
        g_p_array[i] = sal_calloc(CMD_BUF_256);
    }
    

    return PM_E_NONE;
}

