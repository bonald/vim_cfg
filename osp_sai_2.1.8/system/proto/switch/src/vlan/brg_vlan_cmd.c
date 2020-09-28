
#include "proto.h"

#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_arpinsp_define.h"
#include "gen/tbl_arpinsp.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "switch.h"
#include "brg_vlan.h"
#include "if_mgr.h"

static int32
_brg_vlan_cmd_add_vlan(vid_t vid, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = brg_vlan_add_vlan(vid);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add vlan fail, %s", pm_err_desc(rc));
    }
    return rc;
}

static int32
_brg_vlan_cmd_del_vlan(vid_t vid, cfg_cmd_para_t *para)
{
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    int32 rc = PM_E_NONE;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_vlan_key_t key;

    if (GLB_DEF_VLAN_ID == vid)
    {
        CFG_CONFLICT_RET("Cannot remove Default VLAN 1");
    }

    /* vlan related check start */
    key.vid = vid;  
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (!p_db_vlan)
    {
        return PM_E_NONE;
    }

    if (p_db_vlan->erps_domain_id)
    {
        CFG_CONFLICT_RET("Can not delete the control vlan %d of an erps domain", vid);
    }

    /* modified by wangjj for fix erps bug 41216, 2016-12-02 */
    if (p_db_vlan->instance)
    {
        CFG_CONFLICT_RET("Can not delete the vlan because it's already associated with some MSTP instance");
    }
    
    if (p_db_vlan->g8032_ring_id)
    {
        CFG_CONFLICT_RET("Can not delete the vlan because it's already associated with g8032 ring %d", p_db_vlan->g8032_ring_id);
    }
    
    if (p_db_vlan->dot1x_guest_vlan_num)
    {
        CFG_CONFLICT_RET("Cannot delete vlan! The guest vlan is configured on it, please delete the guest vlan first");
    }
    /* vlan related check end */

    /* modified by wangjj for moving the delete vlanif after passing all the check above when delete vlan, 2017-01-06 */
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "vlan%u", vid);

    if (TRUE == p_db_vlan->stats_en)
    {
        p_db_vlan->stats_en = 0;
        brg_vlan_stats_operation(p_db_vlan);
    }
    p_db_if = tbl_interface_get_interface(&if_key);
    if (p_db_if)
    {
        rc = if_mgr_del_vlan_if(p_db_if);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Delete vlan interface fail, %s", pm_err_desc(rc));
        }
    }
    
    rc = brg_vlan_del_vlan(vid);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Delete vlan fail, %s", pm_err_desc(rc));
    }
    return rc;
}

static int32
_brg_vlan_cmd_set_name(tbl_vlan_t *p_vlan, tbl_vlan_t *p_db_vlan, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_DEF_VLAN_ID == p_db_vlan->key.vid)
    {
        CFG_INVALID_PARAM_RET("Cannot set name of Default VLAN 1");
    }

    /* 2. check duplicate config */
    rc = cdb_check_name(p_vlan->name, VLANNAME_SIZE, para->p_rs->desc);
    if (rc < 0)
    {
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_vlan_set_name(p_vlan));
    return rc;
}

static int32
_brg_vlan_cmd_set_enable(tbl_vlan_t *p_vlan, tbl_vlan_t *p_db_vlan, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_DEF_VLAN_ID == p_db_vlan->key.vid)
    {
        CFG_INVALID_PARAM_RET("Cannot set enable of Default VLAN 1");
    }

    /* 2. check duplicate config */
    if (p_db_vlan->enable == p_vlan->enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_vlan_set_enable(p_vlan));
    return rc;
}

static int32
_brg_vlan_cmd_set_mac_learning_enable(tbl_vlan_t *p_vlan, tbl_vlan_t *p_db_vlan, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_vlan->mac_learning_en == p_vlan->mac_learning_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_vlan_set_mac_learning_en(p_vlan));
    return rc;
}

static int32
_brg_vlan_cmd_set_security_action(tbl_vlan_t *p_vlan, tbl_vlan_t *p_db_vlan, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (p_db_vlan->security_action == p_vlan->security_action)
    {
        return PM_E_NONE;
    }

    p_vlan->max_mac_count = p_db_vlan->max_mac_count;
    
    /* 2. call API */
    rc = brg_vlan_set_security_action(p_vlan, TRUE);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Can't set mac-limit action");
    }
    
    return rc;
}

static int32
_brg_vlan_cmd_set_max_mac_count(tbl_vlan_t *p_vlan, tbl_vlan_t *p_db_vlan, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (p_db_vlan->max_mac_count == p_vlan->max_mac_count)
    {
        return PM_E_NONE;
    }

    p_vlan->security_action = p_db_vlan->security_action;
    
    /* 2. call API */
    rc = brg_vlan_set_max_mac_count(p_vlan, TRUE);
    if (rc < 0)
    {
        if (GLB_E_RESOURCE_FULL == rc)
        {
            /* get vlan security mac count */
            brg_vlan_get_cur_mac_count(p_db_vlan);
            CFG_CONFLICT_RET("Current mac-addresses is: %d. Can't set maximum", p_db_vlan->cur_mac_count);
        }
        else
        {
            CFG_CONFLICT_RET("Can't set maximum");
        }
    }
    return rc;
}

static int32
_brg_vlan_cmd_stats_operation(tbl_vlan_t *p_vlan, tbl_vlan_t *p_db_vlan, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t *p_sys_global = NULL;
    

    /* 1. config check */
    if (1 == p_vlan->stats_en && 0 == p_db_vlan->stats_en)
    {
        p_sys_global = tbl_sys_global_get_sys_global();
        if (p_sys_global->vlan_stats_cnt >= GLB_MAX_VLAN_STATS_ID)
        {
            CFG_CONFLICT_RET("The maximum number of vlan allowed to do statistics is exceeded 128\n");
        }
    }

    /* 2. check duplicate config */
    if (p_db_vlan->stats_en == p_vlan->stats_en)
    {
        return PM_E_NONE;
    }
    
    if (CDB_CFG_NC_CHECK == para->cfg)
    {
        return PM_E_NONE;
    }
    /* 3. call API */
    PM_E_RETURN(brg_vlan_stats_operation(p_vlan));
    return rc;
    
}
static int32
_brg_vlan_cmd_get_vlan(vid_t vid, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (GLB_ALL_VLAN_ID == vid)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vlan_dump_one, &args);
    }
    else
    {
        key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (!p_db_vlan)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Vlan %u does not exist", vid);
        }        
        tbl_vlan_dump_one(p_db_vlan, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
tbl_vlan_security_dump_one(tbl_vlan_t *p_vlan, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if ((p_vlan->mac_learning_en)
        && (GLB_VLAN_SEC_SPEC_MAX_MAC_COUNT == p_vlan->max_mac_count)
        && (GLB_VLAN_SECURITY_ACTION_FORWARD == p_vlan->security_action))
    {
        return PM_E_NONE;
    }
  
    if (FLD_MATCH(TBL_VLAN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_KEY].name,
            p_vlan->key.vid);
    }
       
    if (FLD_MATCH(TBL_VLAN_FLD_MAC_LEARNING_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_MAC_LEARNING_EN].name,
            p_vlan->mac_learning_en);
    }

    if (FLD_MATCH(TBL_VLAN_FLD_MAX_MAC_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_MAX_MAC_COUNT].name,
                p_vlan->max_mac_count);
    }

    brg_vlan_get_cur_mac_count(p_vlan);
    if (FLD_MATCH(TBL_VLAN_FLD_CUR_MAC_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_CUR_MAC_COUNT].name,
            p_vlan->cur_mac_count);
    }

    if (FLD_MATCH(TBL_VLAN_FLD_SECURITY_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_FLD_SECURITY_ACTION].name, 
            cdb_enum_val2str(glb_vlan_security_action_strs, GLB_VLAN_SECURITY_ACTION_MAX, p_vlan->security_action));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
_brg_vlan_cmd_get_vlan_security(vid_t vid, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (GLB_ALL_VLAN_ID == vid)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vlan_security_dump_one, &args);
    }
    else
    {
        key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (!p_db_vlan)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Vlan %u does not exist", vid);
        }        
        tbl_vlan_security_dump_one(p_db_vlan, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_brg_vlan_cmd_get_vlan_brief(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_vlan_master_t *p_master = tbl_vlan_get_master();
    tbl_vlan_t *p_db_vlan = NULL;
    vlan_bmp_t vlan_bmp;
    char buf[MAX_CMD_STR_LEN];
    uint32 vid = 0;

    buf[0] = CMD_ZERO_CHAR;
    sal_memset(vlan_bmp, 0, sizeof(vlan_bmp));
    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        GLB_BMP_SET(vlan_bmp, vid);
    }
    cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, vlan_bmp, GLB_VLAN_BMP_WORD_MAX);

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_fprintf(fp, "%s", buf);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
brg_vlan_cmd_process_show_vlan_one(tbl_vlan_t *p_db_vlan, FILE *fp)
{
    char ifname_ext1[IFNAME_SIZE];
    char ifname_ext2[IFNAME_SIZE];
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    uint32 first_line = TRUE;
    uint32 ifindex = 0;
    uint32 str_len = 0;
    uint32 member_count = 0;
    
    sal_fprintf(fp, "%-8u %-16s %-8s %-8u ",
        p_db_vlan->key.vid, 
        p_db_vlan->name, 
        active_str(p_db_vlan->enable), 
        p_db_vlan->instance);

    ifname_ext1[0] = CMD_ZERO_CHAR;
    ifname_ext2[0] = CMD_ZERO_CHAR;

    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (p_db_if == NULL)
        {
            continue;
        }
		p_db_brgif = p_db_if->brgif;
        if (p_db_brgif == NULL)
        {
            continue;
        }		
        /*the show result is not include the agg member*/
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        
        if ((member_count % 2) == 0)
        {
            IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext1);
            if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type || GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
            {
                sal_strcat(ifname_ext1, "(u)");
        }
        else
        {
        	if (p_db_brgif->pvid == p_db_vlan->key.vid)
        	{
				sal_strcat(ifname_ext1, "(u)");
			}
            else if (GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, p_db_vlan->key.vid))
            {
				sal_strcat(ifname_ext1, "(u)");
            }
			else
			{
				sal_strcat(ifname_ext1, "(t)");
			}
        }
        }
        else
        {
            IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext2);
            if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type || GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
            {
                sal_strcat(ifname_ext2, "(u)");
            }
            else
            {
            	if (p_db_brgif->pvid == p_db_vlan->key.vid)
            	{
					sal_strcat(ifname_ext2, "(u)");
				}
	            else if (GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, p_db_vlan->key.vid))
	            {
					sal_strcat(ifname_ext2, "(u)");
	            }
				else
				{
					sal_strcat(ifname_ext2, "(t)");
				}
            }
        }
        
        member_count++;
        if ((member_count % 2) == 0)
        {
            if (first_line)
            {
                first_line = FALSE;
                sal_fprintf(fp, "%-15s %-15s\n", ifname_ext1, ifname_ext2);
            }
            else
            {
                sal_fprintf(fp, "%-43s %-15s %-15s\n", "", ifname_ext1, ifname_ext2);
            }
            ifname_ext1[0] = CMD_ZERO_CHAR;
            ifname_ext2[0] = CMD_ZERO_CHAR;
        }
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);

    str_len = sal_strlen(ifname_ext1);
    if (str_len)
    {
        if (first_line)
        {
            first_line = FALSE;
            sal_fprintf(fp, "%-15s %-15s\n", ifname_ext1, ifname_ext2);
        }
        else
        {
            sal_fprintf(fp, "%-43s %-15s %-15s\n", "", ifname_ext1, ifname_ext2);
        }
        ifname_ext1[0] = CMD_ZERO_CHAR;
        ifname_ext2[0] = CMD_ZERO_CHAR;
    }
    sal_fprintf(fp, "\n");
        
    return PM_E_NONE;
}

int32
brg_vlan_cmd_process_show_vlan(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_vlan_master_t *p_vlan_master = tbl_vlan_get_master();
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    FILE *fp = NULL;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (1 <= argc)
        {
            PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
            key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&key);
            if (NULL == p_db_vlan)
            {
                CFG_CONFLICT_RET("Vlan %u does not exist", key.vid);
            }
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");


    sal_fprintf(fp, "(u)-Untagged, (t)-Tagged\n");
    sal_fprintf(fp, "%-8s %-16s %-8s %-8s %-36s\n", "VLAN ID", "Name", "State", "Instance", "Member ports");
    sal_fprintf(fp, "--------+----------------+--------+--------+----------------------------------\n");

    if (NULL == p_db_vlan)
    {
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_db_vlan = p_vlan_master->vlan_array[vid];
            if (NULL == p_db_vlan)
            {
                continue;
            }
            brg_vlan_cmd_process_show_vlan_one(p_db_vlan, fp);
        }
    }
    else
    {
        brg_vlan_cmd_process_show_vlan_one(p_db_vlan, fp);
    }    
    
    sal_fclose(fp);
    fp = NULL;

    return rc;
}

int32
brg_vlan_cmd_process_vlan(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_vlan_t vlan;
    tbl_vlan_t *p_db_vlan = NULL;
    vid_t vid = 0;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 value = 0;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_VLAN);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
        rc = _brg_vlan_cmd_add_vlan(vid, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
        rc = _brg_vlan_cmd_del_vlan(vid, para);
        break;

    case CDB_OPER_SET:
        PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        sal_memset(&vlan, 0, sizeof(vlan));
        vlan.key.vid = vid;

        /* check exist */
        p_db_vlan = tbl_vlan_get_vlan(&vlan.key);
        if (!p_db_vlan)
        {
            CFG_CONFLICT_RET("Vlan %u not found", vlan.key.vid);
        }

        switch (field_id)
        {
        case TBL_VLAN_FLD_NAME:
            if (argc > i+1)
            {
                /* check the string length */
                if (sal_strlen(argv[i+1]) > VLANNAME_SIZE)
                {
                    CFG_CONFLICT_RET("VLAN name length should not exceed %u", VLANNAME_SIZE);
                }
                sal_strncpy(vlan.name, argv[i+1], VLANNAME_SIZE);
            }
            else
            {
                sal_snprintf(vlan.name, VLANNAME_SIZE, "VLAN%04d", vid);
            }
            rc = _brg_vlan_cmd_set_name(&vlan, p_db_vlan, para);
            break;

        case TBL_VLAN_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            vlan.enable = value ? TRUE : FALSE;
            rc = _brg_vlan_cmd_set_enable(&vlan, p_db_vlan, para);
            break;

        case TBL_VLAN_FLD_MAC_LEARNING_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            vlan.mac_learning_en = value ? TRUE : FALSE;
            rc = _brg_vlan_cmd_set_mac_learning_enable(&vlan, p_db_vlan, para);
            break;

        case TBL_VLAN_FLD_SECURITY_ACTION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            vlan.security_action = value;
            rc = _brg_vlan_cmd_set_security_action(&vlan, p_db_vlan, para);
            break;

        case TBL_VLAN_FLD_MAX_MAC_COUNT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            vlan.max_mac_count = value;
            rc = _brg_vlan_cmd_set_max_mac_count(&vlan, p_db_vlan, para);
            break;
            
        case TBL_VLAN_FLD_STATS_EN:
#if defined(GREATBELT)
            CFG_UNSUPPORT_PRODUCT_RET();
#endif
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            vlan.stats_en = value;
            rc = _brg_vlan_cmd_stats_operation(&vlan, p_db_vlan, para);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            vid = GLB_ALL_VLAN_ID;
        }
        else if ((1 <= argc) && !sal_strcmp(argv[0], "security"))
        {
            if (1 == argc)
            {
                vid = GLB_ALL_VLAN_ID;
            }
            else if (2 == argc)
            {
                PM_CFG_GET_UINT(vid, argv[1], para->p_rs);
            }

            rc = cfg_cmd_parse_field(argv, 1, p_node, &field, TRUE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        
            rc = _brg_vlan_cmd_get_vlan_security(vid, para, p_node, &field);
            break;
        }
        else if ((1 <= argc) && !sal_strcmp(argv[0], "brief"))
        {
            rc = _brg_vlan_cmd_get_vlan_brief(para, p_node, &field);
            break;
        }
        else if (1 == argc)
        {
            PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _brg_vlan_cmd_get_vlan(vid, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_brg_pvlan_cmd_get_pvlan(vid_t vid, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_pvlan_key_t key;
    tbl_pvlan_t *p_db_pvlan = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (GLB_ALL_VLAN_ID == vid)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_pvlan_dump_one, &args);
    }
    else
    {
        key.primary_vid = vid;
        p_db_pvlan = tbl_pvlan_get_pvlan(&key);
        if (!p_db_pvlan)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Vlan %u does not exist", vid);
        }        
        tbl_pvlan_dump_one(p_db_pvlan, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
brg_pvlan_cmd_process_pvlan(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    vid_t vid = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PVLAN);
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            vid = GLB_ALL_VLAN_ID;
        }
        else if (1 == argc)
        {
            PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _brg_pvlan_cmd_get_pvlan(vid, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
brg_vlan_cmd_arpinsp_enable_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_arpinsp_t  *p_arpinsp = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t* p_db_vlan = NULL;

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET: 
        switch (field_id)
        {
        case TBL_ARPINSP_FLD_ENABLE_STATE:
            p_arpinsp = (tbl_arpinsp_t*)p_tbl;
            if (p_arpinsp)
            {
                sal_memset(&vlan_key, 0, sizeof(vlan_key));
                vlan_key.vid = p_arpinsp->key.vlan_id;
                p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                if (NULL == p_db_vlan)
                {
                    return PM_E_NOT_EXIST;
                }
                brg_vlan_set_arpinsp_enable_callback(p_db_vlan, p_arpinsp->enable_state);
            }
            break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}


int32
brg_vlan_cmd_process_show_dot1q_ethertype(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    FILE *fp  = NULL;
    char  ifname_ext[IFNAME_EXT_SIZE];
    int32 rc  = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "%-15s %-15s\n", "Interface", "Ethertype");
    sal_fprintf(fp, "---------------+---------------\n");

    if (argc == 0)
    {
        CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
        {
            if (((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
                || (INTERFACE_IS_LAG_MEMBER(p_db_if)))
            {
                continue;
            }
            else
            {
                sal_fprintf(fp, "%-15s 0x%x\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->svlan_tpid);
            }
        }
    }
    else
    {
        if (sal_strlen(argv[0]))
        {
            p_db_if = tbl_interface_get_interface_by_name(argv[0]);
            if (NULL == p_db_if)
            {
                CFG_CONFLICT_RET("Interface %s not exist", IFNAME_ETH2FULL(argv[0], ifname_ext));
            }

            if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
            {
                CFG_CONFLICT_RET("Interface %s is not port if or linkagg if", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            }

            sal_fprintf(fp, "%-15s 0x%x\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->svlan_tpid);
        }
        else
        {
            CFG_CONFLICT_RET("Invalid ifname");
        }
    }    
    
    sal_fclose(fp);
    fp = NULL;

    return rc;
}


