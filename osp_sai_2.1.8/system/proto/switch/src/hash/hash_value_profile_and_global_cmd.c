#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "glb_hash_field_value_define.h"
#include "ctclib_opb.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_hash_field_profile_define.h" 
#include "gen/tbl_hash_field_profile.h" 
#include "gen/tbl_hash_value_profile_define.h" 
#include "gen/tbl_hash_value_profile.h" 
#include "gen/tbl_hash_value_global_define.h" 
#include "gen/tbl_hash_value_global.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "switch.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"

extern ctclib_opb_t    hash_value_pro_index_opb;

/*modified by wanh for bug 54047, 2019-10-16*/
int32
hash_value_get_profile_id_by_profile_name(char* profile_name)
{
    int32 field_id = 0;
    tbl_hash_field_profile_key_t hash_fld_pro_key;
    tbl_hash_field_profile_t *p_db_hash_fld_pro = NULL;

    sal_memset(&hash_fld_pro_key, 0, sizeof(hash_fld_pro_key));
    sal_strncpy(hash_fld_pro_key.name, profile_name, sizeof(hash_fld_pro_key.name));
    p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&hash_fld_pro_key);
    
    if (NULL == p_db_hash_fld_pro)
    {
        field_id = -1;
    }
    else
    {
        field_id = p_db_hash_fld_pro->index;
    }

    return field_id;   
}
/*wanh end*/

int32
hash_value_field_cmd_set_description(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_hash_value_pro->description, p_db_hash_value_pro->description, DESC_SIZE))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_description(p_hash_value_pro));
    return rc;
}

int32
hash_value_field_cmd_set_port_channel_dynamic(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_pro->bitmap_data == p_db_hash_value_pro->bitmap_data &&
        p_hash_value_pro->agg_dynamic == p_db_hash_value_pro->agg_dynamic)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_port_channel_dynamic(p_hash_value_pro));
    return rc;
}

int32
hash_value_field_cmd_set_port_channel_static(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_pro->bitmap_data == p_db_hash_value_pro->bitmap_data &&
        p_hash_value_pro->agg_static == p_db_hash_value_pro->agg_static)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_port_channel_static(p_hash_value_pro));
    return rc;
}


int32
hash_value_field_cmd_set_port_channel_all(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_pro->bitmap_data == p_db_hash_value_pro->bitmap_data &&
        p_hash_value_pro->agg_input == p_db_hash_value_pro->agg_input &&
        p_hash_value_pro->agg_static == p_db_hash_value_pro->agg_static &&
        p_hash_value_pro->agg_dynamic == p_db_hash_value_pro->agg_dynamic)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_port_channel_all(p_hash_value_pro));
    return rc;
}

int32
hash_value_field_cmd_set_ecmp_dynamic(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_pro->bitmap_data == p_db_hash_value_pro->bitmap_data &&
        p_hash_value_pro->ecmp_dynamic == p_db_hash_value_pro->ecmp_dynamic &&
        p_hash_value_pro->ecmp_dynamic_flow_id == p_db_hash_value_pro->ecmp_dynamic_flow_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_ecmp_dynamic(p_hash_value_pro));
    return rc;
}

int32
hash_value_field_cmd_set_ecmp_static(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_pro->bitmap_data == p_db_hash_value_pro->bitmap_data &&
        p_hash_value_pro->ecmp_static == p_db_hash_value_pro->ecmp_static &&
        p_hash_value_pro->ecmp_static_l3 == p_db_hash_value_pro->ecmp_static_l3 &&
        p_hash_value_pro->ecmp_static_nvgre == p_db_hash_value_pro->ecmp_static_nvgre &&
        p_hash_value_pro->ecmp_static_vxlan == p_db_hash_value_pro->ecmp_static_vxlan)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_ecmp_static(p_hash_value_pro));
    return rc;
}

int32
hash_value_field_cmd_set_ecmp_all(tbl_hash_value_profile_t* p_hash_value_pro, tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_pro->bitmap_data == p_db_hash_value_pro->bitmap_data &&
        p_hash_value_pro->ecmp_all == p_db_hash_value_pro->ecmp_all &&
        p_hash_value_pro->ecmp_static_l3 == p_db_hash_value_pro->ecmp_static_l3 &&
        p_hash_value_pro->ecmp_static_nvgre == p_db_hash_value_pro->ecmp_static_nvgre &&
        p_hash_value_pro->ecmp_static_vxlan == p_db_hash_value_pro->ecmp_static_vxlan &&
        p_hash_value_pro->ecmp_dynamic == p_db_hash_value_pro->ecmp_dynamic &&
        p_hash_value_pro->ecmp_dynamic_flow_id == p_db_hash_value_pro->ecmp_dynamic_flow_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_field_set_ecmp_all(p_hash_value_pro));
    return rc;
}

int32
hash_value_global_cmd_set_port_channel(tbl_hash_value_global_t* p_hash_value_glb, tbl_hash_value_global_t* p_db_hash_value_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_glb->bitmap_data == p_db_hash_value_glb->bitmap_data &&
        p_hash_value_glb->agg_hash == p_db_hash_value_glb->agg_hash)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_global_set_port_channel(p_hash_value_glb));
    return rc;
}

int32
hash_value_global_cmd_set_lbid(tbl_hash_value_global_t* p_hash_value_glb, tbl_hash_value_global_t* p_db_hash_value_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_glb->bitmap_data == p_db_hash_value_glb->bitmap_data &&
        p_hash_value_glb->lbid == p_db_hash_value_glb->lbid)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_global_set_lbid(p_hash_value_glb));
    return rc;
}

int32
hash_value_global_cmd_set_ecmp_dynamic(tbl_hash_value_global_t* p_hash_value_glb, tbl_hash_value_global_t* p_db_hash_value_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_glb->bitmap_data == p_db_hash_value_glb->bitmap_data &&
        p_hash_value_glb->ecmp_dynamic == p_db_hash_value_glb->ecmp_dynamic &&
        p_hash_value_glb->ecmp_dynamic_flow_id == p_db_hash_value_glb->ecmp_dynamic_flow_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_global_set_ecmp_dynamic(p_hash_value_glb));
    return rc;
}

int32
hash_value_global_cmd_set_ecmp_static(tbl_hash_value_global_t* p_hash_value_glb, tbl_hash_value_global_t* p_db_hash_value_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_glb->bitmap_data == p_db_hash_value_glb->bitmap_data &&
        p_hash_value_glb->ecmp_static == p_db_hash_value_glb->ecmp_static &&
        p_hash_value_glb->ecmp_static_l3 == p_db_hash_value_glb->ecmp_static_l3 &&
        p_hash_value_glb->ecmp_static_nvgre == p_db_hash_value_glb->ecmp_static_nvgre &&
        p_hash_value_glb->ecmp_static_vxlan == p_db_hash_value_glb->ecmp_static_vxlan)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_global_set_ecmp_static(p_hash_value_glb));
    return rc;
}

int32
hash_value_global_cmd_set_ecmp_all(tbl_hash_value_global_t* p_hash_value_glb, tbl_hash_value_global_t* p_db_hash_value_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_value_glb->bitmap_data == p_db_hash_value_glb->bitmap_data &&
        p_hash_value_glb->ecmp_all == p_db_hash_value_glb->ecmp_all &&
        p_hash_value_glb->ecmp_static_l3 == p_db_hash_value_glb->ecmp_static_l3 &&
        p_hash_value_glb->ecmp_static_nvgre == p_db_hash_value_glb->ecmp_static_nvgre &&
        p_hash_value_glb->ecmp_static_vxlan == p_db_hash_value_glb->ecmp_static_vxlan &&
        p_hash_value_glb->ecmp_dynamic == p_db_hash_value_glb->ecmp_dynamic &&
        p_hash_value_glb->ecmp_dynamic_flow_id == p_db_hash_value_glb->ecmp_dynamic_flow_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_value_global_set_ecmp_all(p_hash_value_glb));
    return rc;
}

int32
hash_value_profile_cmd_get_hash_value_profile(char *hash_value_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_hash_value_profile_key_t hash_value_pro_key;
    tbl_hash_value_profile_t* p_db_hash_value_pro = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (NULL == hash_value_name)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_hash_value_profile_dump_one, &args);
    }
    else
    {
        sal_memset(&hash_value_pro_key, 0, sizeof(hash_value_pro_key));
        sal_strncpy(hash_value_pro_key.name, hash_value_name, HASH_VALUE_PROFILE_NAME_SIZE);
        p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&hash_value_pro_key);
        if (NULL == p_db_hash_value_pro)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Hash value %s does not exist", hash_value_name);
        }        
        tbl_hash_value_profile_dump_one(p_db_hash_value_pro, &args);
    }
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
hash_value_global_cmd_get_hash_value_global(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_hash_value_global_t* p_hash_value_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_hash_value_glb = tbl_hash_value_global_get_hash_value_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_hash_value_global_dump_one(p_hash_value_glb, &args);
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

static int32
hash_value_cmd_get_select_value(char* profile_name, cfg_cmd_para_t *para)
{
    int32 select_value = 0;
    
    /*added by wanh for bug 54047, 2019-10-16*/
    /*check keyword*/

    #ifdef TAPPRODUCT
    if (sal_strcmp (profile_name, GLB_HASH_FIELD_ECMP_STR) == 0)
    {
        CFG_CONFLICT_RET("Not support to select hash-field %s", profile_name);
    }    
    #endif

    if ((sal_strcmp (profile_name, GLB_HASH_FIELD_PORT_CHANNEL_STR) == 0) || (sal_strcmp (profile_name, GLB_HASH_FIELD_ECMP_STR) == 0))
    {
        CFG_CONFLICT_RET("Can not select system default hash field: %s",profile_name);
    }
    /*wanh end*/
    
    /*get field id*/
    select_value = hash_value_get_profile_id_by_profile_name(profile_name);
    if (select_value < 0)
    {
        CFG_CONFLICT_RET("Invalid hash field name");
    }
    return select_value;
}

static uint32
hash_value_cmd_get_configure_value(char* arithmetic_type, int32 select_value)
{
    uint32 offset_value = 0;
    uint32 configure_value = 0;

    /*get arithmetic*/
    if (sal_strcmp (arithmetic_type, "first") == 0)
    {
        offset_value = HASH_OFFSET_HASH_ARITHMETIC_FIRST;
    }
    else
    {
        offset_value = HASH_OFFSET_HASH_ARITHMETIC_SECOND;
    }
    
    configure_value = select_value * GLB_HASH_OFFSET_VALUE_SIZE + offset_value;
    
    return configure_value;
}

int32
hash_value_profile_cmd_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_hash_value_profile_t hash_value_pro;
    tbl_hash_value_profile_key_t hash_value_pro_key;
    tbl_hash_value_profile_t* p_db_hash_value_pro = NULL;
    tbl_hash_value_profile_master_t* p_hash_value_pro_master = tbl_hash_value_profile_get_master();
    char *hash_value_name = NULL;
    uint32 configure_value = 0;
    uint32 hash_value_pro_index = 0;
    int32 select_value = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_HASH_VALUE_PROFILE);

    sal_memset(&hash_value_pro, 0, sizeof(hash_value_pro));
    sal_memset(&hash_value_pro_key, 0, sizeof(hash_value_pro_key));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        
        /*added by wanh for bug 54047, 2019-10-16*/
        if ((sal_strcmp (argv[0], "interface-applied") == 0) || (sal_strcmp (argv[0], GLB_HASH_FIELD_PORT_CHANNEL_STR) == 0) || (sal_strcmp (argv[0], GLB_HASH_FIELD_ECMP_STR) == 0))
        {
            CFG_CONFLICT_RET("%s is system keyword", argv[0]);
        }
        /*wanh end*/
        
        sal_strncpy(hash_value_pro.key.name, argv[0], sizeof(hash_value_pro.key.name));
        p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&(hash_value_pro.key));
        if (NULL == p_db_hash_value_pro)
        {
            if (check_name_character_and_len(argv[0], HASH_VALUE_PROFILE_NAME_SIZE) != 0)
            {
                CFG_CONFLICT_RET("The initial character should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 64");
            }
            if (p_hash_value_pro_master->hash_value_profile_list->count >= GLB_HASH_VALUE_PROFILE_MAX_NUM)
            {
                CFG_CONFLICT_RET("The maximum of user-defined hash value is %u", GLB_HASH_VALUE_PROFILE_MAX_NUM);
            }
            rc = ctclib_opb_alloc_offset(&hash_value_pro_index_opb, &hash_value_pro_index);
            if (PM_E_NONE != rc)
            {
                return rc;
            }
            hash_value_pro.index = hash_value_pro_index;
            PM_E_RETURN(tbl_hash_value_profile_add_hash_value_profile(&hash_value_pro));
        }
        else
        {
            return PM_E_NONE;
        }
        break;

    case CDB_OPER_DEL:
        sal_strncpy(hash_value_pro_key.name, argv[0], sizeof(hash_value_pro_key.name));
        p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&hash_value_pro_key);
        if (NULL == p_db_hash_value_pro)
        {
            CFG_CONFLICT_RET("Hash value %s does not exist", argv[0]);
        }
        else
        {
            if (p_db_hash_value_pro->ref_cnt > 0)
            {
                CFG_CONFLICT_RET("Hash value %s is in use", argv[0]);
            }
            hash_value_pro_index = p_db_hash_value_pro->index;
            PM_E_RETURN(tbl_hash_value_profile_del_hash_value_profile(&hash_value_pro_key));
            rc = ctclib_opb_free_offset(&hash_value_pro_index_opb, hash_value_pro_index);
            if (rc)
            {
                return rc;
            }
        }
        break;

    case CDB_OPER_SET:
        sal_strncpy(hash_value_pro.key.name, argv[0], sizeof(hash_value_pro.key.name));
        p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&(hash_value_pro.key));
        if (NULL == p_db_hash_value_pro)
        {
            CFG_CONFLICT_RET("Hash value %s does not exist", argv[0]);
        }
        /*hash value description*/
        if (sal_strcmp (argv[1], "description") == 0)
        {
            if (argc > 2)
            {
                if (check_name_character_and_len(argv[2], DESC_SIZE) != 0)
                {
                    CFG_CONFLICT_RET("The initial character of description name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 64");
                }
                sal_strncpy(hash_value_pro.description, argv[2], DESC_SIZE);
            }
            rc = hash_value_field_cmd_set_description(&hash_value_pro, p_db_hash_value_pro, para);
        }
        /*hash value port channel*/
        else if (sal_strcmp (argv[1], "port-channel") == 0)
        {
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 4, 6, para->p_rs);
            sal_memcpy(&hash_value_pro, p_db_hash_value_pro, sizeof(hash_value_pro));
            hash_value_pro.bitmap_data = 0;
            if (sal_strcmp (argv[2], "set") == 0)
            {
                select_value = hash_value_cmd_get_select_value(argv[4], para);
                if (select_value < 0)
                {
                    return -1;
                }
                configure_value = hash_value_cmd_get_configure_value(argv[5], select_value);               
                if (sal_strcmp (argv[3], "dynamic") == 0)
                {
                    rc = hash_value_on_modify_hash_value_profile_egress(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed because this hash value has been used on link aggregation egress\n");
                    }
                    GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC);
                    hash_value_pro.agg_dynamic = configure_value;
                    GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC);
                    rc = hash_value_field_cmd_set_port_channel_dynamic(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "static") == 0)
                {
                    rc = hash_value_on_modify_hash_value_profile_egress(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed because this hash value has been used on link aggregation egress\n");
                    }
                    GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_STATIC);
                    hash_value_pro.agg_static = configure_value;
                    GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_STATIC);
                    rc = hash_value_field_cmd_set_port_channel_static(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "select") == 0)
                {
                    rc = hash_value_on_modify_profile_agg_hash(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed to synchronize hash value link aggregation hash set\n");
                    }
                    hash_value_pro.agg_input = configure_value;
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_STATIC))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_STATIC);
                        hash_value_pro.agg_static = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC);
                        hash_value_pro.agg_dynamic = configure_value;
                    }
                    GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_ALL);
                    rc = hash_value_field_cmd_set_port_channel_all(&hash_value_pro, p_db_hash_value_pro, para);
                }
            }
            else if (sal_strcmp (argv[2], "unset") == 0)
            {
                sal_memcpy(&hash_value_pro, p_db_hash_value_pro, sizeof(hash_value_pro));
                hash_value_pro.bitmap_data = 0;
                if (sal_strcmp (argv[3], "dynamic") == 0)
                {
                    if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_PORT_CHANNEL_SET_ALL)
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC);
                        hash_value_pro.agg_dynamic = p_db_hash_value_pro->agg_input;
                    }
                    else
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC);
                        hash_value_pro.agg_dynamic = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                    }
                    GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC);
                    rc = hash_value_field_cmd_set_port_channel_dynamic(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "static") == 0)
                {
                    if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_PORT_CHANNEL_SET_ALL)
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_STATIC);
                        hash_value_pro.agg_static = p_db_hash_value_pro->agg_input;
                    }
                    else
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_STATIC);
                        hash_value_pro.agg_static = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                    }
                    GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_STATIC);
                    rc = hash_value_field_cmd_set_port_channel_static(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "select") == 0)
                {
                    rc = hash_value_on_modify_hash_value_profile_egress(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed because this hash value has been used on link aggregation egress\n");
                    }
                    if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_PORT_CHANNEL_SET_ALL)
                    {
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_STATIC))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_STATIC);
                            hash_value_pro.agg_static = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC);
                            hash_value_pro.agg_dynamic = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                    }
                    else
                    {
                        return PM_E_NONE;
                    }
                    GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_PORT_CHANNEL_SET_ALL);
                    rc = hash_value_field_cmd_set_port_channel_all(&hash_value_pro, p_db_hash_value_pro, para);
                }
            }
        }
        /*hash value ecmp*/
        else if (sal_strcmp (argv[1], "ecmp") == 0)
        {
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 7, 9, para->p_rs);
            if (sal_strcmp (argv[2], "set") == 0)
            {
                sal_memcpy(&hash_value_pro, p_db_hash_value_pro, sizeof(hash_value_pro));
                hash_value_pro.bitmap_data = 0;
                select_value = hash_value_cmd_get_select_value(argv[5], para);
                if (select_value < 0)
                {
                    return -1;
                }
                configure_value = hash_value_cmd_get_configure_value(argv[6], select_value); 
                if (sal_strcmp (argv[3], "dynamic") == 0)
                {
                    rc = hash_value_on_modify_hash_value_profile_egress(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed because this hash value has been used on link aggregation egress\n");
                    }
                    if (sal_strcmp (argv[4], "flow-id") == 0)
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID);
                        hash_value_pro.ecmp_dynamic_flow_id = configure_value;
                        GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID);
                    }
                    else
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC);
                        hash_value_pro.ecmp_dynamic = configure_value;
                        GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC);
                    }
                    rc = hash_value_field_cmd_set_ecmp_dynamic(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "static") == 0)
                {
                    rc = hash_value_on_modify_hash_value_profile_egress(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed because this hash value has been used on link aggregation egress\n");
                    }
                    if (sal_strcmp (argv[7], "l3") == 0)
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                        hash_value_pro.ecmp_static_l3 = configure_value;
                        GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3);
                    }
                    else if (sal_strcmp (argv[7], "nvgre") == 0)
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                        hash_value_pro.ecmp_static_nvgre = configure_value;
                        GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE);
                    }
                    else if (sal_strcmp (argv[7], "vxlan") == 0)
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                        hash_value_pro.ecmp_static_vxlan = configure_value;
                        GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN);
                    }
                    else
                    {
                        hash_value_pro.ecmp_static = configure_value;
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                            hash_value_pro.ecmp_static_l3 = configure_value;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                            hash_value_pro.ecmp_static_nvgre = configure_value;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                            hash_value_pro.ecmp_static_vxlan = configure_value;
                        }
                        GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL);
                    }
                    rc = hash_value_field_cmd_set_ecmp_static(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "select") == 0)
                {
                    rc = hash_value_on_modify_hash_value_profile_egress(p_db_hash_value_pro->index);
                    if (rc)
                    {
                        CFG_CONFLICT_RET("Failed because this hash value has been used on link aggregation egress\n");
                    }
                    hash_value_pro.ecmp_all = configure_value;
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC);
                        hash_value_pro.ecmp_dynamic = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID);
                        hash_value_pro.ecmp_dynamic_flow_id = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                        hash_value_pro.ecmp_static_l3 = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                        hash_value_pro.ecmp_static_nvgre = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN))
                    {
                        GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                        hash_value_pro.ecmp_static_vxlan = configure_value;
                    }
                    GLB_SET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_ALL);
                    rc = hash_value_field_cmd_set_ecmp_all(&hash_value_pro, p_db_hash_value_pro, para);
                }
            }
            else if (sal_strcmp (argv[2], "unset") == 0)
            {
                sal_memcpy(&hash_value_pro, p_db_hash_value_pro, sizeof(hash_value_pro));
                hash_value_pro.bitmap_data = 0;
                if (sal_strcmp (argv[3], "dynamic") == 0)
                {
                    if (sal_strcmp (argv[4], "flow-id") == 0)
                    {
                        if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID);
                            hash_value_pro.ecmp_dynamic_flow_id = p_db_hash_value_pro->ecmp_all;
                        }
                        else
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID);
                            hash_value_pro.ecmp_dynamic_flow_id = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID);
                    }
                    else
                    {
                        if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC);
                            hash_value_pro.ecmp_dynamic = p_db_hash_value_pro->ecmp_all;
                        }
                        else
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC);
                            hash_value_pro.ecmp_dynamic = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC);
                    }
                    rc = hash_value_field_cmd_set_ecmp_dynamic(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "static") == 0)
                {
                    if (sal_strcmp (argv[5], "l3") == 0)
                    {
                        if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                        {
                            if (GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                                hash_value_pro.ecmp_static_l3 = p_db_hash_value_pro->ecmp_static;
                            }
                            else
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                                hash_value_pro.ecmp_static_l3 = p_db_hash_value_pro->ecmp_all;
                            }
                        }
                        else
                        {
                            if (GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                                hash_value_pro.ecmp_static_l3 = p_db_hash_value_pro->ecmp_static;
                            }
                            else
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                                hash_value_pro.ecmp_static_l3 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                            }
                        }
                        GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3);
                    }
                    else if (sal_strcmp (argv[5], "nvgre") == 0)
                    {
                        if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                        {
                            if (GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                                hash_value_pro.ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static;
                            }
                            else
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                                hash_value_pro.ecmp_static_nvgre = p_db_hash_value_pro->ecmp_all;
                            }
                        }
                        else
                        {
                            if (GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                                hash_value_pro.ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static;
                            }
                            else
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                                hash_value_pro.ecmp_static_nvgre = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                            }
                        }
                        GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE);
                    }
                    else if (sal_strcmp (argv[5], "vxlan") == 0)
                    {
                        if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                        {
                            if (GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                                hash_value_pro.ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static;
                            }
                            else
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                                hash_value_pro.ecmp_static_vxlan = p_db_hash_value_pro->ecmp_all;
                            }
                        }
                        else
                        {
                            if (GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                                hash_value_pro.ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static;
                            }
                            else
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                                hash_value_pro.ecmp_static_vxlan = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                            }
                        }
                        GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN);
                    }
                    else
                    {
                        if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                        {
                            if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                                hash_value_pro.ecmp_static_l3 = p_db_hash_value_pro->ecmp_all;
                            }
                            if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                                hash_value_pro.ecmp_static_nvgre = p_db_hash_value_pro->ecmp_all;
                            }
                            if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                                hash_value_pro.ecmp_static_vxlan = p_db_hash_value_pro->ecmp_all;
                            }
                        }
                        else
                        {
                            if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                                hash_value_pro.ecmp_static_l3 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                            }
                            if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                                hash_value_pro.ecmp_static_nvgre = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                            }
                            if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN))
                            {
                                GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                                hash_value_pro.ecmp_static_vxlan = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                            }
                        }
                        GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL);
                    }
                    rc = hash_value_field_cmd_set_ecmp_static(&hash_value_pro, p_db_hash_value_pro, para);
                }
                else if (sal_strcmp (argv[3], "select") == 0)
                {
                    if ((p_db_hash_value_pro->bitmap_command) & GLB_COMMAND_ECMP_SET_ALL)
                    {
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC);
                            hash_value_pro.ecmp_dynamic = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID);
                            hash_value_pro.ecmp_dynamic_flow_id = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL) &&
                            !GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3);
                            hash_value_pro.ecmp_static_l3 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL) &&
                            !GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE);
                            hash_value_pro.ecmp_static_nvgre = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL) &&
                            !GLB_FLAG_ISSET(p_db_hash_value_pro->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN))
                        {
                            GLB_SET_FLAG(hash_value_pro.bitmap_data, GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN);
                            hash_value_pro.ecmp_static_vxlan = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
                        }
                    }
                    else
                    {
                        return PM_E_NONE;
                    }
                    GLB_UNSET_FLAG(hash_value_pro.bitmap_command, GLB_COMMAND_ECMP_SET_ALL);
                    rc = hash_value_field_cmd_set_ecmp_all(&hash_value_pro, p_db_hash_value_pro, para);
                }
            }
        }
        break;
    case CDB_OPER_GET:
        if (0 == argc)
        {
            hash_value_name = NULL;
        }
        else if (1 == argc)
        {
            hash_value_name = argv[0];
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = hash_value_profile_cmd_get_hash_value_profile(hash_value_name, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hash_value_global_cmd_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_hash_value_global_t hash_value_glb;
    tbl_hash_value_global_t* p_db_hash_value_glb = tbl_hash_value_global_get_hash_value_global();
    uint32 configure_value = 0;
    int32 select_value = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_HASH_VALUE_GLOBAL);

    sal_memset(&hash_value_glb, 0, sizeof(hash_value_glb));

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        break;
    case CDB_OPER_SET:
        if (sal_strcmp (argv[0], "port-channel") == 0)
        {
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 4, para->p_rs);
            sal_memcpy(&hash_value_glb, p_db_hash_value_glb, sizeof(hash_value_glb));
            hash_value_glb.bitmap_data = 0;
            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_AGG);
            if (sal_strcmp (argv[1], "set") == 0)
            {
                select_value = hash_value_cmd_get_select_value(argv[2], para);
                if (select_value < 0)
                {
                    return -1;
                }
                configure_value = hash_value_cmd_get_configure_value(argv[3], select_value); 
                hash_value_glb.agg_hash = configure_value;
            }
            else
            {
                hash_value_glb.agg_hash = GLB_HASH_AGG_HASH_VALUE_DEFAULT_GLOBAL;
            }
            PM_E_RETURN(hash_value_global_cmd_set_port_channel(&hash_value_glb, p_db_hash_value_glb, para));
            rc = hash_value_on_modify_global_agg_hash(0);
            if (rc)
            {
                CFG_CONFLICT_RET("Failed to synchronize hash value global link aggregation hash set\n");
            }
        }
        else if (sal_strcmp (argv[0], "lbid") == 0)
        {
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 4, para->p_rs);
            sal_memcpy(&hash_value_glb, p_db_hash_value_glb, sizeof(hash_value_glb));
            hash_value_glb.bitmap_data = 0;
            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_LBID);
            if (sal_strcmp (argv[1], "set") == 0)
            {
                select_value = hash_value_cmd_get_select_value(argv[2], para);
                if (select_value < 0)
                {
                    return -1;
                }
                configure_value = hash_value_cmd_get_configure_value(argv[3], select_value);
                hash_value_glb.lbid = configure_value;
            }
            else
            {
                hash_value_glb.lbid = GLB_HASH_LBID_HASH_VALUE_DEFAULT_GLOBAL;
            }
            PM_E_RETURN(hash_value_global_cmd_set_lbid(&hash_value_glb, p_db_hash_value_glb, para));        
        }
        else if (sal_strcmp (argv[0], "ecmp") == 0)
        {
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 6, 8, para->p_rs);
            sal_memcpy(&hash_value_glb, p_db_hash_value_glb, sizeof(hash_value_glb));
            hash_value_glb.bitmap_data = 0;
            if (sal_strcmp (argv[1], "set") == 0)
            {
                select_value = hash_value_cmd_get_select_value(argv[4], para);
                if (select_value < 0)
                {
                    return -1;
                }
                configure_value = hash_value_cmd_get_configure_value(argv[5], select_value);
                if (sal_strcmp (argv[2], "dynamic") == 0)
                {
                    if (sal_strcmp (argv[3], "flow-id") == 0)
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID);
                        hash_value_glb.ecmp_dynamic_flow_id = configure_value;
                        GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID_GLOBAL);
                    }
                    else
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC);
                        hash_value_glb.ecmp_dynamic = configure_value;
                        GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC_GLOBAL);
                    }
                    rc = hash_value_global_cmd_set_ecmp_dynamic(&hash_value_glb, p_db_hash_value_glb, para);
                }
                else if (sal_strcmp (argv[2], "static") == 0)
                {
                    if (sal_strcmp (argv[6], "l3") == 0)
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3);
                        hash_value_glb.ecmp_static_l3 = configure_value; 
                        GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL);
                    }
                    else if (sal_strcmp (argv[6], "nvgre") == 0)
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE);
                        hash_value_glb.ecmp_static_nvgre = configure_value; 
                        GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL);
                    }
                    else if (sal_strcmp (argv[6], "vxlan") == 0)
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN);
                        hash_value_glb.ecmp_static_vxlan = configure_value; 
                        GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL);
                    }
                    else
                    {
                        hash_value_glb.ecmp_static = configure_value;
                        if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL))
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3);
                            hash_value_glb.ecmp_static_l3 = configure_value;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL))
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE);
                            hash_value_glb.ecmp_static_nvgre = configure_value;
                        }
                        if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL))
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN);
                            hash_value_glb.ecmp_static_vxlan = configure_value;
                        }
                        GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL);
                    }
                    rc = hash_value_global_cmd_set_ecmp_static(&hash_value_glb, p_db_hash_value_glb, para);
                }
                else if (sal_strcmp (argv[2], "select") == 0)
                {
                    hash_value_glb.ecmp_all = configure_value;
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC);
                        hash_value_glb.ecmp_dynamic = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID);
                        hash_value_glb.ecmp_dynamic_flow_id = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3);
                        hash_value_glb.ecmp_static_l3 = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE);
                        hash_value_glb.ecmp_static_nvgre = configure_value;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN);
                        hash_value_glb.ecmp_static_vxlan = configure_value;
                    }
                    GLB_SET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_ALL_GLOBAL);
                    rc = hash_value_global_cmd_set_ecmp_all(&hash_value_glb, p_db_hash_value_glb, para);
                }
            }
            else
            {
                if (sal_strcmp (argv[2], "dynamic") == 0)
                {
                    if (sal_strcmp (argv[3], "flow-id") == 0)
                    {
                        if (GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID_GLOBAL))
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID);
                            hash_value_glb.ecmp_dynamic_flow_id = p_db_hash_value_glb->ecmp_all;
                            GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID_GLOBAL);
                        }
                        else
                        {
                            return PM_E_NONE;
                        }
                    }
                    else
                    {
                        if (GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC_GLOBAL))
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC);
                            hash_value_glb.ecmp_dynamic = p_db_hash_value_glb->ecmp_all;
                            GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC_GLOBAL);
                        }
                        else
                        {
                            return PM_E_NONE;
                        }
                    }
                    rc = hash_value_global_cmd_set_ecmp_dynamic(&hash_value_glb, p_db_hash_value_glb, para);
                }
                else if (sal_strcmp (argv[2], "static") == 0)
                {
                    if (sal_strcmp (argv[4], "l3") == 0)
                    {
                        if ((p_db_hash_value_glb->bitmap_command) & GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL)
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3);
                            if (GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL))
                            {
                                hash_value_glb.ecmp_static_l3 = p_db_hash_value_glb->ecmp_static;
                            }
                            else
                            {
                                hash_value_glb.ecmp_static_l3 = p_db_hash_value_glb->ecmp_all;
                            }
                            GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL);
                        }
                        else
                        {
                            return PM_E_NONE;
                        }
                    }
                    else if (sal_strcmp (argv[4], "nvgre") == 0)
                    {
                        if ((p_db_hash_value_glb->bitmap_command) & GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL)
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE);
                            if (GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL))
                            {
                                hash_value_glb.ecmp_static_nvgre = p_db_hash_value_glb->ecmp_static;
                            }
                            else
                            {
                                hash_value_glb.ecmp_static_nvgre = p_db_hash_value_glb->ecmp_all;
                            }
                            GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL);
                        }
                        else
                        {
                            return PM_E_NONE;
                        }
                    }
                    else if (sal_strcmp (argv[4], "vxlan") == 0)
                    {
                        if ((p_db_hash_value_glb->bitmap_command) & GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL)
                        {
                            GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN);
                            if (GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL))
                            {
                                hash_value_glb.ecmp_static_vxlan = p_db_hash_value_glb->ecmp_static;
                            }
                            else
                            {
                                hash_value_glb.ecmp_static_vxlan = p_db_hash_value_glb->ecmp_all;
                            }
                            GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL);
                        }
                        else
                        {
                            return PM_E_NONE;
                        }
                    }
                    else
                    {
                        if ((p_db_hash_value_glb->bitmap_command) & GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL)
                        {
                            hash_value_glb.ecmp_static = p_db_hash_value_glb->ecmp_all;
                            if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL))
                            {
                                GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3);
                                hash_value_glb.ecmp_static_l3 = p_db_hash_value_glb->ecmp_all;
                            }
                            if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL))
                            {
                                GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE);
                                hash_value_glb.ecmp_static_nvgre = p_db_hash_value_glb->ecmp_all;
                            }
                            if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL))
                            {
                                GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN);
                                hash_value_glb.ecmp_static_vxlan = p_db_hash_value_glb->ecmp_all;
                            }
                            GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL);
                        }
                        else
                        {
                            return PM_E_NONE;
                        }
                    }
                    rc = hash_value_global_cmd_set_ecmp_static(&hash_value_glb, p_db_hash_value_glb, para);
                }
                else if (sal_strcmp (argv[2], "select") == 0)
                {
                    hash_value_glb.ecmp_all = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC);
                        hash_value_glb.ecmp_dynamic = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID);
                        hash_value_glb.ecmp_dynamic_flow_id = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3);
                        hash_value_glb.ecmp_static_l3 = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE);
                        hash_value_glb.ecmp_static_nvgre = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
                    }
                    if (!GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL) &&
                        !GLB_FLAG_ISSET(p_db_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL))
                    {
                        GLB_SET_FLAG(hash_value_glb.bitmap_data, GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN);
                        hash_value_glb.ecmp_static_vxlan = GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL;
                    }
                    GLB_UNSET_FLAG(hash_value_glb.bitmap_command, GLB_COMMAND_ECMP_SET_ALL_GLOBAL);
                    rc = hash_value_global_cmd_set_ecmp_all(&hash_value_glb, p_db_hash_value_glb, para);
                }
            }
        }
        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = hash_value_global_cmd_get_hash_value_global(para, p_node, &field);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return rc;
}

void
hash_value_show_hash_value_profile_line(FILE *fp)
{
    sal_fprintf (fp, "LBT:load balance type         LBM:load balance mode\n");
    sal_fprintf (fp, "PT :packet type               HF :hash field name\n");
    sal_fprintf (fp, "HA :hash arithmetic\n");
}

void
hash_value_get_hash_value_string(uint32 hash_offset, char profile_name[10], char arithmetic[10])
{
    tbl_hash_field_profile_master_t *p_master_fld_pro = tbl_hash_field_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_hash_field_profile_t* p_hash_fld_pro = NULL;
    int32 hash_select = 0;
    
    hash_select = hash_offset / GLB_HASH_OFFSET_VALUE_SIZE;
    CTCLIB_SLIST_LOOP(p_master_fld_pro->hash_field_profile_list, p_hash_fld_pro, listnode)
    {
        if (p_hash_fld_pro && hash_select == p_hash_fld_pro->index)
        {
            sal_strcpy(profile_name, p_hash_fld_pro->key.name);
            break;
        }
    }
    
    if(hash_offset % GLB_HASH_OFFSET_VALUE_SIZE == 0)
    {
        sal_strcpy(arithmetic, "first");
    }
    else if(hash_offset % GLB_HASH_OFFSET_VALUE_SIZE == 16)
    {
        sal_strcpy(arithmetic, "seccond");
    }
}

void
hash_value_show_hash_value_profile_agg_and_ecmp(tbl_hash_value_profile_t* p_db_hash_value_pro, tbl_hash_value_profile_t* hash_value_pro)
{
    if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_ALL)
    {
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_STATIC)
        {
            hash_value_pro->agg_static = p_db_hash_value_pro->agg_static;
        }
        else
        {
            hash_value_pro->agg_static = p_db_hash_value_pro->agg_input;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC)
        {
            hash_value_pro->agg_dynamic = p_db_hash_value_pro->agg_dynamic;
        }
        else
        {
            hash_value_pro->agg_dynamic = p_db_hash_value_pro->agg_input;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_RESILIENT)
        {
            hash_value_pro->agg_resilient = p_db_hash_value_pro->agg_resilient;
        }
        else
        {
            hash_value_pro->agg_resilient = p_db_hash_value_pro->agg_input;
        }
    }
    else
    {
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_STATIC)
        {
            hash_value_pro->agg_static = p_db_hash_value_pro->agg_static;
        }
        else
        {
            hash_value_pro->agg_static = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC)
        {
            hash_value_pro->agg_dynamic = p_db_hash_value_pro->agg_dynamic;
        }
        else
        {
            hash_value_pro->agg_dynamic = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_RESILIENT)
        {
            hash_value_pro->agg_resilient = p_db_hash_value_pro->agg_resilient;
        }
        else
        {
            hash_value_pro->agg_resilient = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
        }
    }

    if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_ALL)
    {
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_DYNAMIC)
        {
            hash_value_pro->ecmp_dynamic = p_db_hash_value_pro->ecmp_dynamic;
        }
        else
        {
            hash_value_pro->ecmp_dynamic = p_db_hash_value_pro->ecmp_all;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_FLOW_ID)
        {
            hash_value_pro->ecmp_dynamic_flow_id = p_db_hash_value_pro->ecmp_dynamic_flow_id;
        }
        else
        {
            hash_value_pro->ecmp_dynamic_flow_id = p_db_hash_value_pro->ecmp_all;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_ALL)
        {
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_L3)
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_static_l3;
            }
            else
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_static;
            }
            
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_NVGRE)
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static_nvgre;
            }
            else
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static;
            }
            
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_VXLAN)
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static_vxlan;
            }
            else
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static;
            }
        }
        else
        {
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_L3)
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_static_l3;
            }
            else
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_all;
            }
            
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_NVGRE)
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static_nvgre;
            }
            else
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_all;
            }
            
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_VXLAN)
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static_vxlan;
            }
            else
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_all;
            }
        }
    }
    else
    {
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_DYNAMIC)
        {
            hash_value_pro->ecmp_dynamic = p_db_hash_value_pro->ecmp_dynamic;
        }
        else
        {
            hash_value_pro->ecmp_dynamic = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_FLOW_ID)
        {
            hash_value_pro->ecmp_dynamic_flow_id = p_db_hash_value_pro->ecmp_dynamic_flow_id;
        }
        else
        {
            hash_value_pro->ecmp_dynamic_flow_id = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
        }
        if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_ALL)
        {
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_L3)
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_static_l3;
            }
            else
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_static;
            }

            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_NVGRE)
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static_nvgre;
            }
            else
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static;
            }

            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_VXLAN)
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static_vxlan;
            }
            else
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static;
            }
        }
        else
        {
            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_L3)
            {
                hash_value_pro->ecmp_static_l3 = p_db_hash_value_pro->ecmp_static_l3;
            }
            else
            {
                hash_value_pro->ecmp_static_l3 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            }

            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_NVGRE)
            {
                hash_value_pro->ecmp_static_nvgre = p_db_hash_value_pro->ecmp_static_nvgre;
            }
            else
            {
                hash_value_pro->ecmp_static_nvgre = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            }

            if (p_db_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_VXLAN)
            {
                hash_value_pro->ecmp_static_vxlan = p_db_hash_value_pro->ecmp_static_vxlan;
            }
            else
            {
                hash_value_pro->ecmp_static_vxlan = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            }
        }
    }
}

int32
hash_value_show_hash_value_global(cfg_cmd_para_t *para)
{
    char agg_out_p[10] = {'\0'};
    char agg_out_a[10]= {'\0'};
    char ecmp_sl3_p[10]= {'\0'};
    char ecmp_sl3_a[10]= {'\0'};
    char ecmp_snvgre_p[10] = {'\0'};
    char ecmp_snvgre_a[10] = {'\0'};
    char ecmp_svxlan_p[10] = {'\0'};
    char ecmp_svxlan_a[10] = {'\0'};
    char ecmp_d_p[10]= {'\0'};
    char ecmp_d_a[10]= {'\0'};
    char ecmp_flow_p[10]= {'\0'};
    char ecmp_flow_a[10]= {'\0'};
    char lbid_p[10]= {'\0'};
    char lbid_a[10]= {'\0'};

    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_hash_value_global_t * p_hash_value_glb = NULL;
    p_hash_value_glb = tbl_hash_value_global_get_hash_value_global();
    if (NULL == p_hash_value_glb)
    {
        return PM_E_NONE;
    }
    
    hash_value_get_hash_value_string(p_hash_value_glb->ecmp_static_l3, ecmp_sl3_p, ecmp_sl3_a);
    hash_value_get_hash_value_string(p_hash_value_glb->ecmp_static_nvgre, ecmp_snvgre_p, ecmp_snvgre_a);
    hash_value_get_hash_value_string(p_hash_value_glb->ecmp_static_vxlan, ecmp_svxlan_p, ecmp_svxlan_a);
    hash_value_get_hash_value_string(p_hash_value_glb->ecmp_dynamic, ecmp_d_p, ecmp_d_a);
    hash_value_get_hash_value_string(p_hash_value_glb->ecmp_dynamic_flow_id, ecmp_flow_p, ecmp_flow_a);
    hash_value_get_hash_value_string(p_hash_value_glb->agg_hash, agg_out_p, agg_out_a);
    hash_value_get_hash_value_string(p_hash_value_glb->lbid, lbid_p, lbid_a);

    sal_fprintf (fp, "LBT:load balance type         LBM :load balance mode\n");
    sal_fprintf (fp, "PT :packet type               HF  :hash field name\n");
    sal_fprintf (fp, "HA :hash arithmetic\n");
    sal_fprintf (fp, "hash-value global\n");
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "LBT", "LBM", "PT", "HF", "HA");
    sal_fprintf (fp, "----------------------------------------------------------------------\n");
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "port-channel", "-", "all", agg_out_p, agg_out_a);
#ifndef TAPPRODUCT
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "lbid", "-", "all", lbid_p, lbid_a);
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "static", "l3", ecmp_sl3_p, ecmp_sl3_a);
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "static", "nvgre", ecmp_snvgre_p, ecmp_snvgre_a);
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "static", "vxlan", ecmp_svxlan_p, ecmp_svxlan_a);
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "dynamic", "all", ecmp_d_p, ecmp_d_a);
    sal_fprintf (fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "flow id", "all", ecmp_flow_p, ecmp_flow_a);
#endif
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}


int32
hash_value_show_hash_value_profile_one(FILE *fp, tbl_hash_value_profile_t* p_db_hash_value_pro)
{
    char agg_s_p[10] = {'\0'};
    char agg_s_a[10] = {'\0'};
    char agg_d_p[10] = {'\0'};
    char agg_d_a[10] = {'\0'};
//    char agg_r_p[10] = {'\0'};
 //   char agg_r_a[10] = {'\0'};
    char ecmp_sl3_p[10] = {'\0'};
    char ecmp_sl3_a[10] = {'\0'};
    char ecmp_snvgre_p[10] = {'\0'};
    char ecmp_snvgre_a[10] = {'\0'};
    char ecmp_svxlan_p[10] = {'\0'};
    char ecmp_svxlan_a[10] = {'\0'};
    char ecmp_d_p[10] = {'\0'};
    char ecmp_d_a[10] = {'\0'};
    char ecmp_flow_p[10] = {'\0'};
    char ecmp_flow_a[10] = {'\0'};

    tbl_hash_value_profile_t hash_value_pro;
    sal_memset(&hash_value_pro, 0, sizeof(hash_value_pro));
    hash_value_show_hash_value_profile_agg_and_ecmp(p_db_hash_value_pro,&hash_value_pro);
    
    /*hash value agg static*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.agg_static)
    {
        sal_strcpy(agg_s_p, "NOCFG");
        sal_strcpy(agg_s_a, "NOCFG");
    }
    else
    {
        hash_value_get_hash_value_string(hash_value_pro.agg_static, agg_s_p, agg_s_a);
    }

    /*hash value agg dynamic*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.agg_dynamic)
    {
        sal_strcpy(agg_d_p, "NOCFG");
        sal_strcpy(agg_d_a, "NOCFG");
    }
    else
    {            
        hash_value_get_hash_value_string(hash_value_pro.agg_dynamic, agg_d_p, agg_d_a);
    }

    /*hash value ecmp static l3*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.ecmp_static_l3)
    {
        sal_strcpy(ecmp_sl3_p, "NOCFG");
        sal_strcpy(ecmp_sl3_a, "NOCFG");
    }
    else
    {            
        hash_value_get_hash_value_string(hash_value_pro.ecmp_static_l3, ecmp_sl3_p, ecmp_sl3_a);
    }

    /*hash value ecmp static nvgre*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.ecmp_static_nvgre)
    {
        sal_strcpy(ecmp_snvgre_p, "NOCFG");
        sal_strcpy(ecmp_snvgre_a, "NOCFG");
    }
    else
    {            
        hash_value_get_hash_value_string(hash_value_pro.ecmp_static_nvgre, ecmp_snvgre_p, ecmp_snvgre_a);
    }

    /*hash value ecmp static vxlan*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.ecmp_static_vxlan)
    {
        sal_strcpy(ecmp_svxlan_p, "NOCFG");
        sal_strcpy(ecmp_svxlan_a, "NOCFG");
    }
    else
    {            
        hash_value_get_hash_value_string(hash_value_pro.ecmp_static_vxlan, ecmp_svxlan_p, ecmp_svxlan_a);
    }

    /*hash value ecmp dynamic*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.ecmp_dynamic)
    {
        sal_strcpy(ecmp_d_p, "NOCFG");
        sal_strcpy(ecmp_d_a, "NOCFG");
    }
    else
    {            
        hash_value_get_hash_value_string(hash_value_pro.ecmp_dynamic, ecmp_d_p, ecmp_d_a);
    }

    /*hash value ecmp dynamic flow id*/
    if (GLB_HASH_VALUE_PROFILE_INVALID_OFFSET == hash_value_pro.ecmp_dynamic_flow_id)
    {
        sal_strcpy(ecmp_flow_p, "NOCFG");
        sal_strcpy(ecmp_flow_a, "NOCFG");
    }
    else
    {            
        hash_value_get_hash_value_string(hash_value_pro.ecmp_dynamic_flow_id, ecmp_flow_p, ecmp_flow_a);
    }
    
    sal_fprintf(fp, "hash-value name: %s\n", p_db_hash_value_pro->key.name);
    if (sal_strlen(p_db_hash_value_pro->description))
    {
        sal_fprintf(fp, "description: %s\n", p_db_hash_value_pro->description);
    }
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "LBT", "LBM", "PT", "HF", "HA");
    sal_fprintf(fp, "----------------------------------------------------------------------\n");
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "port-channel", "static", "all", agg_s_p, agg_s_a);
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "port-channel", "dynamic", "all", agg_d_p, agg_d_a);
#ifndef TAPPRODUCT
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "static", "l3", ecmp_sl3_p, ecmp_sl3_a);
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "static", "nvgre", ecmp_snvgre_p, ecmp_snvgre_a);
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "static", "vxlan", ecmp_svxlan_p, ecmp_svxlan_a);
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "dynamic", "all", ecmp_d_p, ecmp_d_a);
    sal_fprintf(fp, " %-14s %-14s %-14s %-14s %s\n", "ecmp", "flow id", "all", ecmp_flow_p, ecmp_flow_a);
    sal_fprintf(fp, "\n");
#endif
    
    return PM_E_NONE;
}

int32
hash_value_show_hash_value_profile(tbl_hash_value_profile_t* p_db_hash_value_pro, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_hash_value_profile_master_t *p_master_value_pro = tbl_hash_value_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_line_cfg = FALSE;

    if (p_db_hash_value_pro)
    {
        hash_value_show_hash_value_profile_line(fp);/*added by yejl for bug 54002, 2019-10-16*/
        hash_value_show_hash_value_profile_one(fp, p_db_hash_value_pro);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master_value_pro->hash_value_profile_list, p_db_hash_value_pro, listnode)
        {
            if (has_line_cfg == FALSE)
            {
                hash_value_show_hash_value_profile_line(fp);
            }
            hash_value_show_hash_value_profile_one(fp, p_db_hash_value_pro);
            has_line_cfg = TRUE;
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

char*
hash_value_get_hash_value_profile_name_by_profile_id(uint32 profile_id)
{
    tbl_hash_value_profile_master_t *p_master_value_pro = tbl_hash_value_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_hash_value_profile_t* p_hash_value_pro = NULL;

    CTCLIB_SLIST_LOOP(p_master_value_pro->hash_value_profile_list, p_hash_value_pro, listnode)
    {
        if (p_hash_value_pro && p_hash_value_pro->index == profile_id)
        {
            return p_hash_value_pro->key.name;
        }
    }
    return NULL;
}

int32
hash_value_show_hash_value_profile_applied(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    char *hash_value_name_port = NULL;
    char *hash_value_name_agg = NULL;
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF && p_db_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
        {
            continue;
        }

        if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            if (p_db_if->hash_value_profile_id)
            {
                sal_fprintf(fp, "%s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                hash_value_name_port = hash_value_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id);
                if (hash_value_name_port)
                {
                    sal_fprintf(fp, " hash-value %s input\n", hash_value_name_port);    
                }
            }
        }
        if (p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
        {
            if (p_db_if->hash_value_profile_id && p_db_if->hash_value_profile_id_agg)
            {
                sal_fprintf(fp, "%s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                hash_value_name_port = hash_value_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id);
                if (hash_value_name_port)
                {
                    sal_fprintf(fp, " hash-value %s input\n", hash_value_name_port);
                }
                hash_value_name_agg = hash_value_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id_agg);
                if (hash_value_name_agg)
                {
                    sal_fprintf(fp, " hash-value %s output\n", hash_value_name_agg);
                }
            }
            else if (p_db_if->hash_value_profile_id && !p_db_if->hash_value_profile_id_agg)
            {
                sal_fprintf(fp, "%s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                hash_value_name_port = hash_value_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id);
                if (hash_value_name_port)
                {
                    sal_fprintf(fp, " hash-value %s intput\n", hash_value_name_port);
                }
            }
            else if (!p_db_if->hash_value_profile_id && p_db_if->hash_value_profile_id_agg)
            {
                sal_fprintf(fp, "%s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                hash_value_name_agg = hash_value_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id_agg);
                if (hash_value_name_agg)
                {
                    sal_fprintf(fp, " hash-value %s output\n", hash_value_name_agg);
                }
            }
        }
    }

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;

}

int32
hash_value_cmd_show_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_hash_value_profile_key_t hash_value_pro_key;
    tbl_hash_value_profile_t* p_db_hash_value_pro = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        if (0 == sal_strcmp(argv[0], "global"))
        {
            rc = hash_value_show_hash_value_global(para);
        }
        else if (0 == sal_strcmp(argv[0], "profile"))
        {
            if (argc == 1)
            {
                rc = hash_value_show_hash_value_profile(NULL, para);
            }
            else if (argc == 2)
            {
                sal_memset(&hash_value_pro_key, 0, sizeof(hash_value_pro_key));
                sal_strncpy(hash_value_pro_key.name, argv[1], HASH_VALUE_PROFILE_NAME_SIZE);/*modified by yejl for bug 53921, 2019-10-11*/
                p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&hash_value_pro_key);
                if (NULL == p_db_hash_value_pro)
                {
                    CFG_INVALID_PARAM_RET("Hash value %s does not exist", argv[1]);
                }
                rc = hash_value_show_hash_value_profile(p_db_hash_value_pro, para);
            }
            else if (argc == 3)
            {
                rc = hash_value_show_hash_value_profile_applied(para);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        break;
    default:
        break;
    }
    return rc ; 

}

