
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/ds_aclqos_if_define.h"
#include "gen/ds_aclqos_if.h"
#include "gen/tbl_qos_domain_define.h"
#include "gen/tbl_qos_domain.h"
#include "gen/tbl_qos_drop_profile_define.h"
#include "gen/tbl_qos_drop_profile.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "gen/tbl_qos_policer_action_profile_define.h"
#include "gen/tbl_qos_policer_action_profile.h"
#include "gen/tbl_qos_policer_profile_define.h"
#include "gen/tbl_qos_policer_profile.h"
#include "gen/tbl_qos_queue_shape_profile_define.h"
#include "gen/tbl_qos_queue_shape_profile.h"
#include "gen/tbl_qos_port_shape_profile_define.h"
#include "gen/tbl_qos_port_shape_profile.h"
#include "gen/tbl_qos_mpls_domain_define.h"
#include "gen/tbl_qos_mpls_domain.h"
#include "gen/tbl_qos_policer_res_define.h"
#include "gen/tbl_qos_policer_res.h"
#include "gen/tbl_qos_queue_smart_buffer_define.h"
#include "gen/tbl_qos_queue_smart_buffer.h"
#include "glb_qos_define.h"
#include "switch.h"
#include "qos/qos_mgr.h"
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"

#define QOS_MGR_CMD_CHK_QOS_ENABLE()                    \
{                                                       \
    if (!tbl_qos_global_get_qos_enable())               \
    {                                                   \
        CFG_CONFLICT_RET("QoS state is disabled");  \
    }                                                   \
}

#define POLICER_ACTION_FUEATURE_CHECK(str)   \
{   \
    if (0 == sal_strcmp(str, "drop") || 0 == sal_strcmp(str, "transmit"))  \
    { \
        if (GLB_CHIP_TSINGMA == tbl_chip_get_type()) \
        { \
            CFG_UNSUPPORT_PRODUCT_RET(); \
        } \
    } \
}

static int32
_qos_mgr_cmd_set_color_action(glb_qos_color_action_t *color_action, char **argv, int32 argc)
{
    if (argc < 3)
    {
        return PM_E_NONE;
    }

    if (0 == sal_strcmp(argv[2], ""))
    {
        /* reset color action default action:red color drop, tc invalid */
        if (0 == sal_strcmp(argv[1], "violate_action"))
        {
            color_action->drop_packet = 1;
        }
        else
        {
            color_action->drop_packet = 0;
        }
        color_action->tc_valid = 0;
        color_action->tc = 0;
        return PM_E_NONE;
    }
    
    if (0 == sal_strcmp(argv[2], "transmit"))
    {
        color_action->drop_packet = 0;
    }
    else if (0 == sal_strcmp(argv[2], "drop"))
    {
        color_action->drop_packet = 1;
    }
    else if (0 == sal_strcmp(argv[2], "tc"))
    {
        if ('\0' != argv[3][0])
        {
            color_action->tc_valid = 1;
            color_action->tc = sal_atoi(argv[3]);
        }
    }
    return PM_E_NONE;
}

static glb_qos_color_t
_qos_mgr_cmd_str2color(char* str)
{
    int32 i = 0;
    char *color_str[] = {"none", "red", "yellow", "green"};
    
    for (i = GLB_QOS_COLOR_NONE; i < GLB_QOS_COLOR_MAX; i++)
    {
        if (0 == sal_strcmp(str, color_str[i]))
        {
            return i;
        }
    }
    return GLB_QOS_COLOR_MAX;
}

static int32
_qos_mgr_cmd_add_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 id = 0;
    tbl_qos_domain_t domain;

    /* 1. check parameter */

    /* 2. check exist */
    id = sal_atoi(argv[0]);
    if (tbl_qos_domain_get_qos_domain_by_id(id))
    {
        return PM_E_NONE;
    }

    /* 3. add entry */
    sal_memset(&domain, 0, sizeof(domain));
    domain.key.id = id;
    rc = tbl_qos_domain_add_qos_domain(&domain);

    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add qos domain failed, %s", pm_err_desc(rc));
    }

    /* 4. update handle */
    /* nothing */
    
    return rc;
}

static int32
_qos_mgr_cmd_set_domain_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 id = 0;
    int32 i = 0;
    int32 value = 0;
    int32 field_id = 0;
    int32 flags_field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_domain_t domain;
    tbl_qos_domain_master_t *p_domain_master = NULL;
    tbl_qos_domain_t *p_db_domain = NULL;
    glb_qos_pri_color_map_t map_info;

    /* 1. check parameter: key/field_str/filed_value/.. */
    if (argc < 3)
    {
        CFG_INVALID_PARAM_RET("Qos domain ID is not specified");
    }

    /* 2. check exist */
    id = sal_atoi(argv[0]);
    p_db_domain = tbl_qos_domain_get_qos_domain_by_id(id);
    if (NULL == p_db_domain)
    {
        CFG_INVALID_PARAM_RET("Qos domain not exist");
    }

    sal_memset(&map_info, 0 ,sizeof(map_info));
    /* 3. update handle: unset */
    /* nothing */
    
    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_QOS_DOMAIN);
    i = 1;
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }

    p_domain_master = tbl_qos_domain_get_master();
    sal_memset(&domain, 0, sizeof(domain));
    domain.key.id = id;

    PM_E_RETURN(tbl_qos_domain_get_qos_domain_field(id, field_id, &domain));
    i++;
    uint8_bmp_t  bmp;
    char err_desc[256];
    uint32 offset = 0;
    sal_memset(bmp, 0, sizeof(bmp));
    rc = cdb_bitmap_str2val(argv[i], 0, 64, bmp, GLB_UINT8_BMP_WORD_MAX, err_desc);
    if(rc < 0)
    {
        CFG_INVALID_PARAM_RET("Invalid range value");
    }
    /* 5. set field */
    switch (field_id)
    {        
    case TBL_QOS_DOMAIN_FLD_COS2COLOR:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_COS_NUM) || (offset < 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid COS value");
                }
                if (0 != sal_strcmp(argv[3], ""))
                {
                    domain.cos2color[offset] = _qos_mgr_cmd_str2color(argv[3]);
                }
                else
                {
                    domain.cos2color[offset] = p_domain_master->default_domain->cos2color[offset];
                }
            }
        }
        break;
    case TBL_QOS_DOMAIN_FLD_EXP2COLOR:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_EXP_NUM) || (offset < 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid EXP value");
                }
                if (0 != sal_strcmp(argv[3], ""))
                {
                    domain.exp2color[offset] = _qos_mgr_cmd_str2color(argv[3]);
                }
                else
                {
                    domain.exp2color[offset] = p_domain_master->default_domain->exp2color[offset];
                }
            }
        }
    break;

    case TBL_QOS_DOMAIN_FLD_DSCP2COLOR:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_DSCP_NUM) || (offset < 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid DSCP value");
                }
                if (0 != sal_strcmp(argv[3], ""))
                {
                    domain.dscp2color[offset] = _qos_mgr_cmd_str2color(argv[3]);
                }
                else
                {
                    domain.dscp2color[offset] = p_domain_master->default_domain->dscp2color[offset];
                }
            }
        }
        break;

    case TBL_QOS_DOMAIN_FLD_COS2TC:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_COS_NUM) || (offset < 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid COS value");
                }
                if (0 != sal_strcmp(argv[3], ""))
                {
                    domain.cos2tc[offset] = sal_atoi(argv[3]);
                }
                else
                {
                    domain.cos2tc[offset] = p_domain_master->default_domain->cos2tc[offset];
                }
            }
        }
        break;
    case TBL_QOS_DOMAIN_FLD_EXP2TC:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_EXP_NUM) || (offset < 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid EXP value");
                }
                if (0 != sal_strcmp(argv[3], ""))
                {
                    domain.exp2tc[offset] = sal_atoi(argv[3]);
                }
                else
                {
                    domain.exp2tc[offset] = p_domain_master->default_domain->exp2tc[offset];
                }
            }
        }
        break;


    case TBL_QOS_DOMAIN_FLD_DSCP2TC:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_DSCP_NUM) || (offset < 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid DSCP value");
                }
                if (0 != sal_strcmp(argv[3], ""))
                {
                    domain.dscp2tc[offset] = sal_atoi(argv[3]);
                }
                else
                {
                    domain.dscp2tc[offset] = p_domain_master->default_domain->dscp2tc[offset];
                }
            }
        }
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2COS:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_TC_NUM) || (offset < 0))
                {
                   CFG_INVALID_PARAM_RET("Invalid Tc value");
                }
                value = _qos_mgr_cmd_str2color(argv[3]);
                if (0 != sal_strcmp(argv[4], ""))
                {
                    domain.tc_color2cos[(offset << 2) + value] = sal_atoi(argv[4]);
                }
                else
                {
                    domain.tc_color2cos[(offset << 2) + value] = p_domain_master->default_domain->tc_color2cos[(offset << 2) + value];
                }
            }
        }
    case TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_TC_NUM) || (offset < 0))
                {
                   CFG_INVALID_PARAM_RET("Invalid Tc value");
                }
                value = _qos_mgr_cmd_str2color(argv[3]);
                if (0 != sal_strcmp(argv[4], ""))
                {
                    domain.tc_color2exp[(offset << 2) + value] = sal_atoi(argv[4]);
                }
                else
                {
                    domain.tc_color2exp[(offset << 2) + value] = p_domain_master->default_domain->tc_color2exp[(offset << 2) + value];
                }
            }
        }

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP:
        for ((offset) = 0; (offset) < GLB_BMP_BITS((bmp)); (offset)++)
        {
            if (GLB_BMP_ISSET((bmp), (offset)))
            {
                if ((offset >= GLB_QOS_TC_NUM) || (offset < 0))
                {
                   CFG_INVALID_PARAM_RET("Invalid Tc value");
                }
                value = _qos_mgr_cmd_str2color(argv[3]);
                if (0 != sal_strcmp(argv[4], ""))
                {
                    domain.tc_color2dscp[(offset << 2) + value] = sal_atoi(argv[4]);
                }
                else
                {
                    domain.tc_color2dscp[(offset << 2) + value] = p_domain_master->default_domain->tc_color2dscp[(offset << 2) + value];
                }
            }
        }
        break;

    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }
    
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, field_id));
    if (flags_field_id)
    {
        PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, flags_field_id));
    }

    /* 6. update handle */
    /* nothing */
    
    return rc;
}

static int32
_qos_mgr_cmd_get_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 id = 0;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_domain_t* p_db_domain;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        id = sal_atoi(argv[0]);
        p_db_domain = tbl_qos_domain_get_qos_domain_by_id(id);
        if (!p_db_domain)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Qos domain %d not exist", id);
        }
        /* 2. update handle: update stats, maybe handel in interface module */
        /* nothing */
        rc = tbl_qos_domain_dump_one(p_db_domain, &dump_args);
    }
    else
    {
        rc = tbl_qos_domain_iterate((TBL_ITER_CB_FUNC)tbl_qos_domain_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 id = 0; 
    CFG_DBG_FUNC();

    /* 1. check confilct: */
    QOS_MGR_CMD_CHK_QOS_ENABLE(); 

    /* 2. check parameter: */
    if (argc)
    {
        /* 8 is default domain */
        PM_CFG_GET_UINT(id ,argv[0], para->p_rs);

        if (id > GLB_QOS_DOMAIN_NUM)
        {
            CFG_INVALID_PARAM_RET("Invalid qos domain ID: %d", id);
        }
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        PM_E_RETURN(_qos_mgr_cmd_add_domain(argv, argc, para));
        break;

    case CDB_OPER_DEL:
        CFG_INVALID_PARAM_RET("Qos domain can not be removed");
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_domain_field(argv, argc, para));
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_domain(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_mgr_cmd_get_mpls_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_mpls_domain_t* p_db_mpls_domain;
    tbl_qos_mpls_domain_key_t key;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        key.mpls_label = sal_atoi(argv[0]);
        p_db_mpls_domain = tbl_qos_mpls_domain_get_qos_mpls_domain(&key);
        if (!p_db_mpls_domain)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Qos mpls label %d domain not exist", key.mpls_label);
        }
        /* 2. update handle: update stats, maybe handel in interface module */
        /* nothing */
        rc = tbl_qos_mpls_domain_dump_one(p_db_mpls_domain, &dump_args);
    }
    else
    {
        rc = tbl_qos_mpls_domain_iterate((TBL_ITER_CB_FUNC)tbl_qos_mpls_domain_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

/*modifed by wanh for bug 52727, 2019-09-16*/
static int32
_qos_mgr_cmd_set_mpls_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 mpls_label = 0;
    uint32 exp_domain = 0;
    tbl_qos_mpls_domain_t* p_db_mpls_domain;
    tbl_qos_mpls_domain_key_t key;
    tbl_qos_mpls_domain_t p_mpls_domain;
    tbl_qos_domain_t *p_db_qos_domain = NULL;
    tbl_qos_mpls_domain_master_t *p_master = tbl_qos_mpls_domain_get_master();
    mpls_label_bmp_t mpls_label_bmp;

    /* check parameter: */
    if (argc < 2)
    {
        CFG_INVALID_PARAM_RET("Need parameter");
    }
    sal_memset(&mpls_label_bmp, 0x0, sizeof(mpls_label_bmp));
    rc = cdb_mpls_label_bitmap_str2val(argv[0], MPLS_LABEL_VALUE_MIN, MPLS_LABEL_VALUE_MAX, mpls_label_bmp, GLB_MPLS_LABEL_BMP_WORD_MAX, para->p_rs->desc);
    if (rc < 0)
    	{
		para->p_rs->rc = rc;
             return rc;
	}

   GLB_BMP_ITER_BEGIN(mpls_label_bmp, mpls_label)
   {	
          sal_memset(&p_mpls_domain, 0x0, sizeof(tbl_qos_mpls_domain_t));
	    exp_domain = sal_atoi(argv[1]);
	    key.mpls_label = mpls_label;
	    p_db_qos_domain = tbl_qos_domain_get_qos_domain_by_id(exp_domain);
	    if (!p_db_qos_domain)
	    {
	        CFG_INVALID_PARAM_RET("Exp domain %d does not exist", exp_domain);
	    }

	    p_db_mpls_domain = tbl_qos_mpls_domain_get_qos_mpls_domain(&key);
	    if (p_db_mpls_domain)
	    {
	        if (0 == exp_domain)
	        {
	            //if exp domain equal default mpls label domain(0), remove mpls label domain
	            PM_E_RETURN(tbl_qos_mpls_domain_del_qos_mpls_domain(&key));
	        }
	        else
	        {
	            p_db_mpls_domain->exp_domain = exp_domain;
	            PM_E_RETURN(tbl_qos_mpls_domain_set_qos_mpls_domain_field(p_db_mpls_domain, TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN));
	        }
	    }
	    else
	    {
	        if (0 != exp_domain)
	        {
	            //check qos mpls label num
	            if (p_master->qos_mpls_domain_hash->count >= GLB_QOS_MPLS_LABEL_NUM)
	            {
	                CFG_FUNC_ERR_RET("Qos mpls label resource full!");
	            }
	            
	            //add mpls label domain
	            p_mpls_domain.key.mpls_label= mpls_label;
	            p_mpls_domain.exp_domain = exp_domain;
	            PM_E_RETURN(tbl_qos_mpls_domain_add_qos_mpls_domain(&p_mpls_domain ));
	        }
	    }
   }
   GLB_BMP_ITER_END(mpls_label_bmp, mpls_label);

    return rc;
}
/*wanh end*/

int32
qos_mgr_cmd_process_mpls_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    CFG_DBG_FUNC();

    /* 1. check confilct: */
    QOS_MGR_CMD_CHK_QOS_ENABLE(); 

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_mpls_domain(argv, argc, para));
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_mpls_domain(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_mgr_cmd_get_policer_res(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    rc = tbl_qos_policer_res_iterate((TBL_ITER_CB_FUNC)tbl_qos_policer_res_dump_one, &dump_args);            

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_policer_res(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    CFG_DBG_FUNC();

    /* 2. check parameter: */
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_policer_res(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_mgr_cmd_add_policer_action_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 id = 0;
    tbl_qos_policer_action_profile_t action_profile;
    tbl_qos_policer_action_profile_t* p_db_action_profile;

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos policer action profile name is not specified");
    }

    sal_memset(&action_profile, 0, sizeof(action_profile));
    /* 2. check exist */
    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(argv[0]);
    if (p_db_action_profile)
    {
        return PM_E_NONE;
    }

    /* 3. add entry */
    if(PM_E_NONE != tbl_qos_policer_action_profile_alloc_profile_id(&id))
    {
        CFG_FUNC_ERR_RET("QoS policer action profile resource full!");
    }
    action_profile.key.id = id;
    sal_strcpy(action_profile.name, argv[0]);
    /* default action, red drop, prio invalid */
    action_profile.violate_action.drop_packet = 1;
    action_profile.violate_action.tc_valid = 0;
    action_profile.violate_action.tc = 0;
    rc = tbl_qos_policer_action_profile_add_qos_policer_action_profile(&action_profile);
    
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add qos policer action profile failed, %s", pm_err_desc(rc));
    }

    return rc;
}

static int32
_qos_mgr_cmd_del_policer_action_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_qos_policer_action_profile_t* p_db_action_profile;

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos action profile name is not specified");
    }

    /* 2. check exist */
    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(argv[0]);
    if (!p_db_action_profile)
    {
        CFG_FUNC_ERR_RET("The policer action profile %s not exist", argv[0]);
    }    
    if(p_db_action_profile->ref_cnt)
    {
        CFG_INVALID_PARAM_RET("This policer action profile has been used!");
    }
    /* delete entry */
    PM_E_RETURN(tbl_qos_policer_action_profile_del_qos_policer_action_profile(&p_db_action_profile->key)); 
    
    return rc;
}

static int32
_qos_mgr_cmd_set_policer_action_profile_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{  
    int32 rc = PM_E_NONE;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_policer_action_profile_t action_profile;
    tbl_qos_policer_action_profile_t* p_db_action_profile; 
        
    /* check exist */
    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(argv[0]); 
    if (!p_db_action_profile)
    {
        CFG_INVALID_PARAM_RET("Qos policer action profile %s not exist", argv[0]);
    }

    /* nothing */

    sal_memset(&action_profile, 0, sizeof(action_profile));
    sal_memcpy(&action_profile.key, &p_db_action_profile->key, sizeof(action_profile.key));

    /* parse field */
    p_node = cdb_get_tbl(TBL_QOS_POLICER_ACTION_PROFILE);
    
    field_id = cdb_get_field_id(argv[1], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    /* set field */
    switch (field_id)
    {
    case TBL_QOS_POLICER_ACTION_PROFILE_FLD_CONFIRM_ACTION:
        /* copy original action */
        sal_memcpy(&action_profile.confirm_action, &p_db_action_profile->confirm_action, sizeof(glb_qos_color_action_t));
        POLICER_ACTION_FUEATURE_CHECK(argv[2]);
        _qos_mgr_cmd_set_color_action(&action_profile.confirm_action, argv, argc);

        /* check duplicate */
        if (0 == sal_memcmp(&action_profile.confirm_action, &p_db_action_profile->confirm_action, sizeof(glb_qos_color_action_t)))
        {
            return PM_E_NONE;
        }
        break;

    case TBL_QOS_POLICER_ACTION_PROFILE_FLD_EXCEED_ACTION:
        /* copy original action */
        sal_memcpy(&action_profile.exceed_action, &p_db_action_profile->exceed_action, sizeof(glb_qos_color_action_t));
        POLICER_ACTION_FUEATURE_CHECK(argv[2]);
        _qos_mgr_cmd_set_color_action(&action_profile.exceed_action, argv, argc);
        
        /* check duplicate */
        if (0 == sal_memcmp(&action_profile.exceed_action, &p_db_action_profile->exceed_action, sizeof(glb_qos_color_action_t)))
        {
            return PM_E_NONE;
        }
        break;  

    case TBL_QOS_POLICER_ACTION_PROFILE_FLD_VIOLATE_ACTION:
        /* copy original action */
        sal_memcpy(&action_profile.violate_action, &p_db_action_profile->violate_action, sizeof(glb_qos_color_action_t));
        POLICER_ACTION_FUEATURE_CHECK(argv[2]);
        _qos_mgr_cmd_set_color_action(&action_profile.violate_action, argv, argc);

        /* check duplicate */
        if (0 == sal_memcmp(&action_profile.violate_action, &p_db_action_profile->violate_action, sizeof(glb_qos_color_action_t)))
        {
            return PM_E_NONE;
        }
        break;
    }
   
    PM_E_RETURN(tbl_qos_policer_action_profile_set_qos_policer_action_profile_field(&action_profile, field_id));

    return rc;
}  

static int32
_qos_mgr_cmd_get_policer_action_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_policer_action_profile_t* p_db_policer_profile;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        p_db_policer_profile = tbl_qos_policer_action_profile_get_profile_by_name(argv[0]);
        if (NULL == p_db_policer_profile)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Qos policer action profile %s does not exist", argv[0]);
        }
        
        rc = tbl_qos_policer_action_profile_dump_one(p_db_policer_profile, &dump_args);
        if (rc < 0)
        {
            sal_fclose(fp);
            return rc;
        }
    }
    else
    {
        field.field_id[0] = TBL_QOS_POLICER_ACTION_PROFILE_FLD_NAME;
        rc = tbl_qos_policer_action_profile_iterate((TBL_ITER_CB_FUNC)tbl_qos_policer_action_profile_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

static int32
_qos_mgr_cmd_add_policer_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 id = 0;
    tbl_qos_policer_profile_t policer_profile;
    tbl_qos_policer_profile_t* p_db_policer_profile;

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos policer name is not specified");
    }

    sal_memset(&policer_profile, 0, sizeof(policer_profile));
    /* 2. check exist */
    p_db_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[0]);
    if (p_db_policer_profile)
    {  
        return PM_E_NONE;
    }


    /* 3. add entry */
    if(PM_E_NONE != tbl_qos_policer_profile_alloc_profile_id(&id))
    {
        CFG_FUNC_ERR_RET("QoS policer profile resource full!");
    }
    policer_profile.key.id = id;
    sal_strcpy(policer_profile.name, argv[0]);
    rc = tbl_qos_policer_profile_add_qos_policer_profile(&policer_profile);

    /*init default policer parameter*/
    policer_profile.cbs = GLB_QOS_POLICER_DEFAULT_BURST;
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_CBS);
    GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_CBS); 
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS);

    policer_profile.ebs_or_pbs = GLB_QOS_POLICER_DEFAULT_BURST;
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS);
    GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS); 
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS);

    policer_profile.cir = GLB_QOS_POLICER_DEFAULT_RATE;
    rc = tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_CIR);
    GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_CIR); 
    rc = tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR);

    policer_profile.algorithm_mode = GLB_QOS_POLICER_MODE_RFC2697;
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE);
    GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE); 
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE);

    policer_profile.color_mode = 0;
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE);
    GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_COLOR_MODE); 
    rc += tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE);
    
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add qos policer failed, %s", pm_err_desc(rc));
    }

    /* 4. update handle */
    /* nothing */
    return rc;
}

static int32
_qos_mgr_cmd_del_policer_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_qos_policer_profile_t* p_db_policer_profile;
    tbl_qos_policer_action_profile_t* p_db_action_profile;

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos policer name is not specified");
    }

    /* 2. check exist */
    p_db_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[0]);
    if (!p_db_policer_profile)
    {
        CFG_FUNC_ERR_RET("The policer profile %s not exist", argv[0]);
    }    
    if(p_db_policer_profile->ref_cnt)
    {
        CFG_INVALID_PARAM_RET("This policer profile has been used!");
    }

    /* 3. del action profile ref_cnt */
    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(p_db_policer_profile->action_profile_name);
    if (p_db_action_profile)
    {
        p_db_action_profile->ref_cnt --;
    }
    
    /* delete entry */
    PM_E_RETURN(tbl_qos_policer_profile_del_qos_policer_profile(&p_db_policer_profile->key)); 
    
    return rc;
}

static glb_qos_policer_color_mode_t
_qos_mgr_map_policer_mode(char* str)
{
    int32 i = 0;
    char *color_mode_str[GLB_QOS_POLICER_COLOR_MAX] =
    {
        "color-blind",
        "color-aware"
    };
    
    
    for (i = GLB_QOS_POLICER_COLOR_BLIND; i < GLB_QOS_POLICER_COLOR_MAX; i++)
    {
        if (0 == sal_strcmp(str, color_mode_str[i]))
        {
            return i;
        }
    }
    return GLB_QOS_POLICER_COLOR_MAX;
}

static glb_qos_policer_mode_t
_qos_mgr_map_policer_algorithm(char* str)
{
    int32 i = 0;
    char *mode_str[GLB_QOS_POLICER_MODE_MAX] =
    {
        "rfc2697",
        "rfc2698",
        "rfc4115",
        "bwp"
    };
    
    
    for (i = GLB_QOS_POLICER_MODE_RFC2697; i < GLB_QOS_POLICER_MODE_MAX; i++)
    {
        if (0 == sal_strcmp(str, mode_str[i]))
        {
            return i;
        }
    }
    return GLB_QOS_POLICER_MODE_MAX;
}

static int32
_qos_mgr_cmd_process_policer_profile_update(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 op = 0;
    bool flow_resource_full = FALSE;
    tbl_qos_policer_profile_t policer_new;
    tbl_qos_policer_profile_t *policer_old = NULL;

    sal_memset(&policer_new, 0x0, sizeof(tbl_qos_policer_profile_t));

    if (argc < 8)
    {
        CFG_INVALID_PARAM_RET("Need parameter");
    }

    policer_old = tbl_qos_policer_profile_get_profile_by_name(argv[0]);
    if (!policer_old)
    {
        CFG_INVALID_PARAM_RET("Qos policer %s not exist", argv[0]);
    }

    if (0 == sal_strcmp(argv[1], "check"))
    {
        op = 1;
    }
    else if (0 == sal_strcmp(argv[1], "update"))
    {
        op = 2;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    policer_new.algorithm_mode = _qos_mgr_map_policer_algorithm(argv[2]);
    policer_new.is_pps = (0 == sal_strcmp(argv[3], "")) ? 0 : 1;
    policer_new.cir = sal_atoi(argv[4]);
    policer_new.cbs = sal_atoi(argv[5]);
    if (0 == sal_strcmp(argv[5], ""))
    {
        if (policer_new.is_pps)
        {
            policer_new.cbs = GLB_QOS_POLICER_DEFAULT_PPS_BURST;
        }
        else
        {
            policer_new.cbs = GLB_QOS_POLICER_DEFAULT_BURST;
        }
    }
    policer_new.eir_or_pir = sal_atoi(argv[6]);
    policer_new.ebs_or_pbs = sal_atoi(argv[7]);
    if (0 == sal_strcmp(argv[7], ""))
    {
        if (policer_new.is_pps)
        {
            policer_new.ebs_or_pbs = GLB_QOS_POLICER_DEFAULT_PPS_BURST;
        }
        else
        {
            policer_new.ebs_or_pbs = GLB_QOS_POLICER_DEFAULT_BURST;
        }
    }

    if (policer_new.cir > GLB_QOS_POLICER_DEFAULT_RATE && !policer_new.is_pps)
    {
        CFG_INVALID_PARAM_RET("The maximum of cir is %d.", GLB_QOS_POLICER_DEFAULT_RATE);
    }
    if (policer_new.cir > GLB_QOS_POLICER_PPS_DEFAULT_RATE && policer_new.is_pps)
    {
        CFG_INVALID_PARAM_RET("The maximum of cir is %d.", GLB_QOS_POLICER_PPS_DEFAULT_RATE);
    }
    
    if (policer_new.eir_or_pir > GLB_QOS_POLICER_DEFAULT_RATE && !policer_new.is_pps)
    {
        CFG_INVALID_PARAM_RET("The maximum of eir is %d.", GLB_QOS_POLICER_DEFAULT_RATE);
    }
    if (policer_new.eir_or_pir > GLB_QOS_POLICER_PPS_DEFAULT_RATE && policer_new.is_pps)
    {
        CFG_INVALID_PARAM_RET("The maximum of eir is %d.", GLB_QOS_POLICER_PPS_DEFAULT_RATE);
    }
    #ifdef GREATBELT
    /* cir + eir must less than 10G */
    if (policer_new.cir + policer_new.eir_or_pir > GLB_QOS_POLICER_DEFAULT_RATE && policer_new.algorithm_mode == GLB_QOS_POLICER_MODE_RFC4115)
    {
        CFG_INVALID_PARAM_RET("Qos policer cir+eir must less than %d", GLB_QOS_POLICER_DEFAULT_RATE);
    }
    #endif
    
    rc = qos_mgr_policer_res_update_op(&policer_new, policer_old, op, &flow_resource_full);
    if (PM_E_CFG_POLICER_NO_RESOURCE == rc)
    {
        if (flow_resource_full)
        {
            CFG_INVALID_PARAM_RET("Flow policer profile resource full");
        }
        else
        {
            CFG_INVALID_PARAM_RET("Port policer profile resource full");
        }
    }

    return rc;
}

static int32
_qos_mgr_cmd_set_policer_profile_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 field_id = 0;
    int32 flags_field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_policer_profile_t policer_profile; 
    tbl_qos_policer_profile_t* p_db_policer_profile; 
    tbl_qos_policer_action_profile_t* p_db_action_profile; 
   
    /* 1. check parameter: key/field_str/filed_value/.. */
    if (argc < 3)
    {
        CFG_INVALID_PARAM_RET("Policer name is not specified");
    }
    /* 2. check exist */
    p_db_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[0]); 
    if (!p_db_policer_profile)
    {
        CFG_INVALID_PARAM_RET("Qos policer %s not exist", argv[0]);
    }

    /* 3. policer res update process */
    if (argc > 3)
    {
        rc = _qos_mgr_cmd_process_policer_profile_update(argv, argc, para);
        return rc;
    }

    sal_memset(&policer_profile, 0, sizeof(policer_profile));
    sal_memcpy(&policer_profile.key, &p_db_policer_profile->key, sizeof(policer_profile.key));

    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_QOS_POLICER_PROFILE);
    i = 1;
    
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    i++;
    /* 5. set field */
    switch (field_id)
    {
    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP:
        if (sal_atoi(argv[i]))
        {
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_AGP);
        }
        else
        {
            GLB_UNSET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_AGP);
        }
        /* no need to do update handle*/
        PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, field_id));
        return rc;
        break;
        
    case TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE:
        policer_profile.algorithm_mode = _qos_mgr_map_policer_algorithm(argv[i]);

        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE;
        GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE);
        break;  

    case TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE:
        policer_profile.color_mode = _qos_mgr_map_policer_mode(argv[i]);  

        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE;
        GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_COLOR_MODE);        
        break;  

    case TBL_QOS_POLICER_PROFILE_FLD_CIR:
        policer_profile.cir = sal_atoi(argv[i]); 
        if (policer_profile.cir > GLB_QOS_POLICER_DEFAULT_RATE && !p_db_policer_profile->is_pps)
        {
            CFG_INVALID_PARAM_RET("The maximum of cir is %d.", GLB_QOS_POLICER_DEFAULT_RATE);
        }
        if (policer_profile.cir > GLB_QOS_POLICER_PPS_DEFAULT_RATE && p_db_policer_profile->is_pps)
        {
            CFG_INVALID_PARAM_RET("The maximum of cir is %d.", GLB_QOS_POLICER_PPS_DEFAULT_RATE);
        }
        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR;
        GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_CIR);         
        break;  

    case TBL_QOS_POLICER_PROFILE_FLD_CBS:
        policer_profile.cbs = sal_atoi(argv[i]);  
        
        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS;
        if (0 == sal_strcmp(argv[i], ""))
        {
            if (p_db_policer_profile->is_pps)
            {
                policer_profile.cbs = GLB_QOS_POLICER_DEFAULT_PPS_BURST;
            }
            else
            {
                policer_profile.cbs = GLB_QOS_POLICER_DEFAULT_BURST;
            }
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_CBS);                 
        }
        else
        {
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_CBS);  
        }
        break;   

    case TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR:
        policer_profile.eir_or_pir = sal_atoi(argv[i]); 
        if (policer_profile.eir_or_pir > GLB_QOS_POLICER_DEFAULT_RATE && !p_db_policer_profile->is_pps)
        {
            CFG_INVALID_PARAM_RET("The maximum of eir is %d.", GLB_QOS_POLICER_DEFAULT_RATE);
        }
        if (policer_profile.eir_or_pir > GLB_QOS_POLICER_PPS_DEFAULT_RATE && p_db_policer_profile->is_pps)
        {
            CFG_INVALID_PARAM_RET("The maximum of eir is %d.", GLB_QOS_POLICER_PPS_DEFAULT_RATE);
        }
        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR;
        GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR);  
        #if 0
        if (0 == sal_strcmp(argv[i], ""))
        {
            policer_profile.eir_or_pir = GLB_QOS_POLICER_DEFAULT_RATE;
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR);                 
        }
        else
        {
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR);  
        }
        #endif
        
        break;    

    case TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS:
        policer_profile.ebs_or_pbs = sal_atoi(argv[i]); 
        
        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS;
        if (0 == sal_strcmp(argv[i], ""))
        {
            if (p_db_policer_profile->is_pps)
            {
                policer_profile.ebs_or_pbs = GLB_QOS_POLICER_DEFAULT_PPS_BURST;
            }
            else
            {
                policer_profile.ebs_or_pbs = GLB_QOS_POLICER_DEFAULT_BURST;
            }
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS);                 
        }
        else
        {
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS);  
        }
        
        break;   

    case TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR:
        policer_profile.drop_color = qos_mgr_map_str2color(argv[i]);

        flags_field_id = TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR;
        if (0 == sal_strcmp(argv[i], ""))
        {
            GLB_UNSET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_DROP_COLOR);                 
        }
        else
        {
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_DROP_COLOR);  
        }
        break; 

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH:
        if (0 == sal_strcmp(argv[i], ""))
        {
            GLB_UNSET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH);                 
        }
        else
        {
            GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH);  
        }
        break;   

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS:
        if (0 == sal_strcmp(argv[i], ""))
        {
            if(GLB_FLAG_ISSET(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                GLB_UNSET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_STATS);   
            }
            else
            {
                return rc;
            }
        }
        else
        {
            if(!GLB_FLAG_ISSET(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                GLB_SET_FLAG(policer_profile.flags, GLB_POLICER_PROFILE_FLAGS_STATS);  
            }
            else
            {
                return rc;
            }
        }
        break;   
    case TBL_QOS_POLICER_PROFILE_FLD_IS_PPS:
        if (0 == sal_strcmp(argv[i], ""))
        {
            policer_profile.is_pps = 0;
        }
        else
        {
            policer_profile.is_pps = 1;
        }
        break;
    case TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME:
        if (0 == sal_strcmp(argv[i], ""))
        {
            if ('\0' == p_db_policer_profile->action_profile_name[0])
            {
                /* policer not apply any action */
                return PM_E_NONE;
            }
            p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(p_db_policer_profile->action_profile_name);
            if (!p_db_action_profile)
            {
                return PM_E_NOT_EXIST;
            }
            p_db_action_profile->ref_cnt--;
            sal_memset(policer_profile.action_profile_name, 0x0, sizeof(policer_profile.action_profile_name));
        }
        else
        {
            if ('\0' != p_db_policer_profile->action_profile_name[0])
            {
                /* policer has been applied an action profile*/
                CFG_INVALID_PARAM_RET("The policer had been applied an action profile! Please delete it first!");
            }
            
            p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(argv[i]);
            if (!p_db_action_profile)
            {
                CFG_INVALID_PARAM_RET("The policer action profile does not exist!");
            }
            p_db_action_profile->ref_cnt++;
            sal_strcpy(policer_profile.action_profile_name, argv[i]);
        }
        break;
       
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }
   
    PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, field_id));
    if (flags_field_id)
    {
        PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(&policer_profile, flags_field_id));
    }
    return rc;
}  

static int32
_qos_mgr_cmd_get_policer_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_policer_profile_t* p_db_policer_profile;
    glb_qos_policer_stats_t policer_stats;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        p_db_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[0]);
        if (NULL == p_db_policer_profile)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Qos policer profile %s does not exist", argv[0]);
        }
        if (2 == argc)
        {
            if (0 == sal_strcmp(argv[1], "agp"))
            {
                if (!GLB_FLAG_ISSET(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_AGP))
                {
                    sal_fclose(fp);
                    CFG_INVALID_PARAM_RET("Qos aggreate-policer %s not exist", argv[0]);
                }
            }
            else if (0 == sal_strcmp(argv[1], "stats"))
            {
                if (tbl_qos_global_get_policer_stats_enable() && 
                            GLB_FLAG_ISSET(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
                {
                    sal_memset(&policer_stats, 0, sizeof(policer_stats));
                    //fei_qos_get_agp_stats(p_db_policer_profile->name, &policer_stats);
                    p_db_policer_profile->confirm_pkt = policer_stats.confirm_stats.packet_count; 
                    p_db_policer_profile->confirm_byte = policer_stats.confirm_stats.byte_count;
                    p_db_policer_profile->exceed_pkt = policer_stats.exceed_stats.packet_count;
                    p_db_policer_profile->exceed_byte = policer_stats.exceed_stats.byte_count;
                    p_db_policer_profile->violate_pkt = policer_stats.violate_stats.packet_count;
                    p_db_policer_profile->violate_byte = policer_stats.violate_stats.byte_count;

                    GLB_SET_FLAG(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DUMP_STATS);
                }
            }
        }
        else
        {
                if (GLB_FLAG_ISSET(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_AGP))
                {
                    sal_fclose(fp);
                    CFG_INVALID_PARAM_RET("Qos policer profile %s does not exist", argv[0]);
                }
        }
        rc = tbl_qos_policer_profile_dump_one(p_db_policer_profile, &dump_args);
        if (rc < 0)
        {
            sal_fclose(fp);
            return rc;
        }
        GLB_UNSET_FLAG(p_db_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DUMP_STATS);
    }
    else
    {
        field.field_id[0] = TBL_QOS_POLICER_PROFILE_FLD_NAME;
        rc = tbl_qos_policer_profile_iterate((TBL_ITER_CB_FUNC)tbl_qos_policer_profile_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_policer_action_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    CFG_DBG_FUNC();

    QOS_MGR_CMD_CHK_QOS_ENABLE(); 
   
    if (argc)
    {
        rc = cdb_check_name(argv[0], POLICER_NAME_SIZE, para->p_rs->desc); 
        if (CDB_NAME_ERR_NONE != rc)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_E_RETURN(_qos_mgr_cmd_add_policer_action_profile(argv, argc, para));
        break;

    case CDB_OPER_DEL:        
        PM_E_RETURN(_qos_mgr_cmd_del_policer_action_profile(argv, argc, para));
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_policer_action_profile_field(argv, argc, para));
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_policer_action_profile(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

int32
qos_mgr_cmd_process_policer_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    QOS_MGR_CMD_CHK_QOS_ENABLE(); 
   
    if (argc)
    {
        rc = cdb_check_name(argv[0], POLICER_NAME_SIZE, para->p_rs->desc); 
        if (CDB_NAME_ERR_NONE != rc)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_E_RETURN(_qos_mgr_cmd_add_policer_profile(argv, argc, para));

        break;

    case CDB_OPER_DEL:        
        PM_E_RETURN(_qos_mgr_cmd_del_policer_profile(argv, argc, para));
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_policer_profile_field(argv, argc, para));
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_policer_profile(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_mgr_cmd_add_queue_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 id = 0;
    tbl_qos_queue_shape_profile_t queue_shape_profile;
    tbl_qos_queue_shape_profile_t* p_db_queue_shape_profile;    

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos shape name is not specified");
    }

    /* 2. check exist */
    p_db_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(argv[0]);
    if (p_db_queue_shape_profile)
    {
        return PM_E_NONE;
    }

    /* 3. add entry */
    if(PM_E_NONE != tbl_qos_queue_shape_profile_alloc_profile_id(&id))
    {
        CFG_FUNC_ERR_RET("QoS scheduler profile resource full!");
    }
    
    sal_memset(&queue_shape_profile, 0, sizeof(queue_shape_profile));
    queue_shape_profile.key.id = id;
    sal_strcpy(queue_shape_profile.name, argv[0]);
    queue_shape_profile.cir = 0;
    queue_shape_profile.pir = GLB_QOS_DEF_SHAPING_PIR;
    queue_shape_profile.key.id = id;
    queue_shape_profile.mode = SCHEDULER_MODE_SP;
    queue_shape_profile.queue_weight = 1;
    GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR);
    GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR);
    GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE);
    GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT);
    rc = tbl_qos_queue_shape_profile_add_qos_queue_shape_profile(&queue_shape_profile);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add qos shape failed, %s", pm_err_desc(rc));
    }
    return rc;
}


static int32
_qos_mgr_cmd_del_queue_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_qos_queue_shape_profile_t* p_db_queue_shape_profile;
    
    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos shape name is not specified");
    }

    /* 2. check exist */
    p_db_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(argv[0]);
    if (!p_db_queue_shape_profile)
    {
        CFG_FUNC_ERR_RET("The scheduler profile %s not exist", argv[0]);
    }    

    if(p_db_queue_shape_profile->ref_cnt)
    {
        CFG_INVALID_PARAM_RET("This scheduler profile has been used!");
    }
    
    /* delete entry */
    PM_E_RETURN(tbl_qos_queue_shape_profile_del_qos_queue_shape_profile(&p_db_queue_shape_profile->key));

    return rc;
}

static int32
_qos_mgr_cmd_set_queue_shape_profile_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 field_id = 0;
    int32 flags_field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_queue_shape_profile_t queue_shape_profile; 
    tbl_qos_queue_shape_profile_t* p_db_queue_shape_profile; 
    
    /* 1. check parameter: key/field_str/filed_value/.. */
    if (argc < 3)
    {
        CFG_INVALID_PARAM_RET("Policer name is not specified");
    }
    /* 2. check exist */
    p_db_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(argv[0]); 
    if (!p_db_queue_shape_profile)
    {
        CFG_INVALID_PARAM_RET("Qos shape %s not exist", argv[0]);
    }

    sal_memset(&queue_shape_profile, 0, sizeof(queue_shape_profile));
    sal_memcpy(&queue_shape_profile.key, &p_db_queue_shape_profile->key, sizeof(queue_shape_profile.key));

    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_QOS_QUEUE_SHAPE_PROFILE);
    i = 1;
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    i++;
    /* 5. set field */
    switch (field_id)
    {
    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR:
        queue_shape_profile.pir = sal_atoi(argv[i]);
        if (queue_shape_profile.pir > GLB_QOS_DEF_SHAPING_PIR)
        {
            CFG_INVALID_PARAM_RET("The maximum of pir is %d.", GLB_QOS_DEF_SHAPING_PIR);
        }
        flags_field_id = TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_PIR;
        if (0 == sal_strcmp(argv[i], ""))
        {
            queue_shape_profile.pir = GLB_QOS_DEF_SHAPING_PIR;
            GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR);
        }
        else
        {
            GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR);
        }
        break;
    
    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_CIR:
        queue_shape_profile.cir = sal_atoi(argv[i]);
        flags_field_id = TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_CIR;
        if (0 == sal_strcmp(argv[i], ""))
        {
            GLB_UNSET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR);
        }
        else
        {
            GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR);
        }
        break;
    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE:
        if(0 == sal_strcmp(argv[i], "sp"))
        {
            queue_shape_profile.mode = SCHEDULER_MODE_SP;
        }
        else
        {
            queue_shape_profile.mode = SCHEDULER_MODE_DWRR;
        }
        flags_field_id = TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_MODE;
        if (0 == sal_strcmp(argv[i], ""))
        {
            GLB_UNSET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE);
        }
        else
        {
            GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE);
        }
        break;
    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT:
        queue_shape_profile.queue_weight= sal_atoi(argv[i]);
        flags_field_id = TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_WEIGHT;
        if (0 == sal_strcmp(argv[i], ""))
        {
            queue_shape_profile.queue_weight = 1;
            GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT);
        }
        else
        {
            GLB_SET_FLAG(queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT);
        }
        break;
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }
    
    /* 3. update handle: unset */
    /* nothing */
    PM_E_RETURN(tbl_qos_queue_shape_profile_set_qos_queue_shape_profile_field(&queue_shape_profile, field_id));
    if (flags_field_id)
    {
        PM_E_RETURN(tbl_qos_queue_shape_profile_set_qos_queue_shape_profile_field(&queue_shape_profile, flags_field_id));
    }
#if 0 
    /* 6. update handle */
    PM_E_RETURN(qos_mgr_shape_profile_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_queue_shape_profile->name));   
#endif    
    return rc;
}

static int32
_qos_mgr_cmd_get_queue_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_queue_shape_profile_t* p_db_queue_shape_profile;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        p_db_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(argv[0]);
        if (!p_db_queue_shape_profile)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Qos scheduler profile %s does not exist", argv[0]);
        }
        rc = tbl_qos_queue_shape_profile_dump_one(p_db_queue_shape_profile, &dump_args);
    }
    else
    {
        field.field_id[0] = TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_NAME;
        rc = tbl_qos_queue_shape_profile_iterate((TBL_ITER_CB_FUNC)tbl_qos_queue_shape_profile_dump_one, &dump_args);      
    }

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_queue_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    
    /* 1. check conflict */
    QOS_MGR_CMD_CHK_QOS_ENABLE(); 

    /* 2. check parameter */
    if (argc)
    {
        rc = cdb_check_name(argv[0], QOS_SHAPE_NAME_SIZE, para->p_rs->desc); 
        if (CDB_NAME_ERR_NONE != rc)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }     
    }

    if (argc && 0 == sal_strcmp(argv[0], GLB_QOS_DEF_SCHEDULER_PROFILE))
    {
        CFG_INVALID_PARAM_RET("This name has been used by system, user can not use or configure it!");
    }
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_E_RETURN(_qos_mgr_cmd_add_queue_shape_profile(argv, argc, para));
        break;

    case CDB_OPER_DEL:        
        PM_E_RETURN(_qos_mgr_cmd_del_queue_shape_profile(argv, argc, para));
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_queue_shape_profile_field(argv, argc, para));        
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_queue_shape_profile(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_mgr_cmd_add_port_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 id = 0;
    tbl_qos_port_shape_profile_t port_shape_profile;
    tbl_qos_port_shape_profile_t* p_db_port_shape_profile;    

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos shape name is not specified");
    }

    /* 2. check exist */
    p_db_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(argv[0]);
    if (p_db_port_shape_profile)
    {
        return PM_E_NONE;
    }

    /* 3. add entry */
    if(PM_E_NONE != tbl_qos_port_shape_profile_alloc_profile_id(&id))
    {
        CFG_FUNC_ERR_RET("QoS port shape profile resource full!");
    }
    
    sal_memset(&port_shape_profile, 0, sizeof(port_shape_profile));
    port_shape_profile.key.id = id;
    sal_strcpy(port_shape_profile.name, argv[0]);
    port_shape_profile.pir = GLB_QOS_DEF_SHAPING_PIR;
    port_shape_profile.key.id = id;
    GLB_SET_FLAG(port_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR);
    rc = tbl_qos_port_shape_profile_add_qos_port_shape_profile(&port_shape_profile);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add qos port shape failed, %s", pm_err_desc(rc));
    }
    return rc;
}

static int32
_qos_mgr_cmd_del_port_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_qos_port_shape_profile_t* p_db_port_shape_profile;
    
    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos shape name is not specified");
    }

    /* 2. check exist */
    p_db_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(argv[0]);
    if (!p_db_port_shape_profile)
    {
        CFG_FUNC_ERR_RET("The port shape profile %s not exist", argv[0]);
    }    

    if(p_db_port_shape_profile->ref_cnt)
    {
        CFG_INVALID_PARAM_RET("This port shape profile has been used!");
    }
    
    /* delete entry */
    PM_E_RETURN(tbl_qos_port_shape_profile_del_qos_port_shape_profile(&p_db_port_shape_profile->key));

    return rc;
}

static int32
_qos_mgr_cmd_set_port_shape_profile_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 field_id = 0;
    int32 flags_field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_port_shape_profile_t port_shape_profile; 
    tbl_qos_port_shape_profile_t* p_db_port_shape_profile; 
    
    /* 1. check parameter: key/field_str/filed_value/.. */
    if (argc < 3)
    {
        CFG_INVALID_PARAM_RET("Policer name is not specified");
    }
    /* 2. check exist */
    p_db_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(argv[0]); 
    if (!p_db_port_shape_profile)
    {
        CFG_INVALID_PARAM_RET("Qos port shape profile %s not exist", argv[0]);
    }

    sal_memset(&port_shape_profile, 0, sizeof(port_shape_profile));
    sal_memcpy(&port_shape_profile.key, &p_db_port_shape_profile->key, sizeof(port_shape_profile.key));

    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_QOS_PORT_SHAPE_PROFILE);
    i = 1;
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    i++;
    /* 5. set field */
    switch (field_id)
    {
    case TBL_QOS_PORT_SHAPE_PROFILE_FLD_PIR:
        port_shape_profile.pir = sal_atoi(argv[i]);
        flags_field_id = TBL_QOS_PORT_SHAPE_PROFILE_FLD_FLAGS_PIR;
        if (port_shape_profile.pir > GLB_QOS_DEF_SHAPING_PIR)
        {
            CFG_INVALID_PARAM_RET("The maximum of pir is %d.", GLB_QOS_DEF_SHAPING_PIR);
        }
        if (0 == sal_strcmp(argv[i], ""))
        {
            port_shape_profile.pir = GLB_QOS_DEF_SHAPING_PIR;
            GLB_SET_FLAG(port_shape_profile.flags, GLB_PORT_SHAPE_PROFILE_FLAGS_PIR);
        }
        else
        {
            GLB_SET_FLAG(port_shape_profile.flags, GLB_PORT_SHAPE_PROFILE_FLAGS_PIR);
        }
        break;
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }
    
    /* 3. update handle: unset */
    /* nothing */
    PM_E_RETURN(tbl_qos_port_shape_profile_set_qos_port_shape_profile_field(&port_shape_profile, field_id));
    if (flags_field_id)
    {
        PM_E_RETURN(tbl_qos_port_shape_profile_set_qos_port_shape_profile_field(&port_shape_profile, flags_field_id));
    }
    return rc;
}

static int32 _qos_mgr_cmd_get_port_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_qos_port_shape_profile_t* p_db_port_shape_profile;
    tbl_iter_args_t pargs;
    field_parse_t field; 
    FILE *fp = NULL;


    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    pargs.argv[0] = &field;
    pargs.argv[1] = fp;

    if(argc)
    {
        /* check exist */
        p_db_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(argv[0]);
        if (!p_db_port_shape_profile)
        {
            sal_fclose(fp);
            CFG_FUNC_ERR_RET("Qos port shape profile %s does not exist", argv[0]);
        }    

        /* dump */
        rc = tbl_qos_port_shape_profile_dump_one(p_db_port_shape_profile, &pargs);
    }
    else
    {
        field.field_id[0] = TBL_QOS_PORT_SHAPE_PROFILE_FLD_NAME;
        rc = tbl_qos_port_shape_profile_iterate((TBL_ITER_CB_FUNC)tbl_qos_port_shape_profile_dump_one, &pargs);      
    }
    

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_port_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    
    /* 1. check conflict */
    QOS_MGR_CMD_CHK_QOS_ENABLE(); 

    /* 2. check parameter */
    if (argc)
    {
        rc = cdb_check_name(argv[0], QOS_SHAPE_NAME_SIZE, para->p_rs->desc); 
        if (CDB_NAME_ERR_NONE != rc)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }     
    }

    if (argc && 0 == sal_strcmp(argv[0], GLB_QOS_DEF_PORT_SHAPE_PROFILE))
    {
        CFG_INVALID_PARAM_RET("This name has been used by system, user can not use or configure it!");
    }
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_E_RETURN(_qos_mgr_cmd_add_port_shape_profile(argv, argc, para));
        break;

    case CDB_OPER_DEL:        
        PM_E_RETURN(_qos_mgr_cmd_del_port_shape_profile(argv, argc, para));
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_port_shape_profile_field(argv, argc, para));        
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_port_shape_profile(argv, argc, para));        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_mgr_cmd_add_drop_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 id = 0;
    tbl_qos_drop_profile_t drop_profile;
    
    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos drop profile name is not specified");
    }

    if ((0 == sal_strcmp(argv[0], GLB_QOS_DEF_WTD_PROFILE)) || 
                (0 == sal_strcmp(argv[0], GLB_QOS_DEF_WRED_PROFILE)))
    {
        CFG_CONFLICT_RET("Default drop profile can not be modified");
    }
    /* 2. check exist */
    if (tbl_qos_drop_profile_get_profile_by_name(argv[0]))
    {        
        return PM_E_NONE;
    }

    /* 3. add entry */
    if(PM_E_NONE != tbl_qos_drop_profile_alloc_profile_id(&id))
    {
        CFG_FUNC_ERR_RET("QoS drop profile resource full!");
    }
    
    sal_memset(&drop_profile, 0, sizeof(drop_profile));
    drop_profile.key.id = id;
    sal_strcpy(drop_profile.name, argv[0]);

    /*modified by yejl to fix bug39742 and bug39747*/
    drop_profile.green_max_threashold = GLB_QOS_DROP_THRESHOLD;
    drop_profile.green_min_threashold = drop_profile.green_max_threashold/8;
    drop_profile.green_probability = GLB_QOS_DROP_PROBABILITY;
    drop_profile.yellow_max_threashold = GLB_QOS_DROP_THRESHOLD;
    drop_profile.yellow_min_threashold = drop_profile.yellow_max_threashold/8;
    drop_profile.yellow_probability = GLB_QOS_DROP_PROBABILITY;
    drop_profile.red_max_threashold = GLB_QOS_DROP_THRESHOLD;
    drop_profile.red_min_threashold = drop_profile.red_max_threashold/8;
    drop_profile.red_probability = GLB_QOS_DROP_PROBABILITY;
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB);
    GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_PROB);
    rc = tbl_qos_drop_profile_add_qos_drop_profile(&drop_profile);

    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add qos drop failed, %s", pm_err_desc(rc));
    }

    /* 4. update handle */
    /* nothing */
    return rc;
}

static int32
_qos_mgr_cmd_del_drop_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_qos_drop_profile_t* p_db_drop_profile;

    /* 1. check parameter */
    if (!argc)
    {
        CFG_INVALID_PARAM_RET("Qos drop name is not specified");
    }

    if ((0 == sal_strcmp(argv[0], GLB_QOS_DEF_WTD_PROFILE)) || 
                (0 == sal_strcmp(argv[0], GLB_QOS_DEF_WRED_PROFILE)))
    {
        CFG_CONFLICT_RET("Default drop profile can not be removed");
    }

    /* 2. check exist */
    p_db_drop_profile = tbl_qos_drop_profile_get_profile_by_name(argv[0]);
    if (!p_db_drop_profile)
    {
        CFG_FUNC_ERR_RET("The drop profile %s not exist", argv[0]);
    }    
    if(p_db_drop_profile->ref_cnt)
    {
        CFG_INVALID_PARAM_RET("This drop profile has been used!");
    }
    
    /* 4. delete entry */
    PM_E_RETURN(tbl_qos_drop_profile_del_qos_drop_profile(&p_db_drop_profile->key));    
    return rc;
}

static int32
_qos_mgr_cmd_drop_profile_set_drop_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 field_id = 0;
    int32 flags_field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_drop_profile_t drop_profile; 
    tbl_qos_drop_profile_t* p_db_drop_profile; 
                                        
    /* 1. check parameter: key/field_str/filed_value/.. */
    if (argc < 3)
    {
        CFG_INVALID_PARAM_RET("Drop name is not specified");
    }

    /* 2. check exist */
    p_db_drop_profile = tbl_qos_drop_profile_get_profile_by_name(argv[0]); 
    if (!p_db_drop_profile)
    {
        CFG_INVALID_PARAM_RET("Qos drop %s not exist", argv[0]);
    }

    sal_memset(&drop_profile, 0, sizeof(drop_profile));
    sal_memcpy(&drop_profile.key, &p_db_drop_profile->key, sizeof(drop_profile.key));

    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_QOS_DROP_PROFILE);
    i = 1;

    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    i++;

#if 1 /* SYSTEM MODIFIED: fix qos bug 38760 add by wangqj at 2016/9/18 */
    if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
    {
        switch(field_id)
        {
            case TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY:
            case TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY:
            case TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY:
                if(sal_strlen(argv[i]))
                {
                    CFG_UNSUPPORT_PRODUCT_RET();
                }
                break;
            default: break;
        }
    }
#endif

#if 1 /* SYSTEM MODIFIED: fix qos bug 38692 add by wangqj at 2016/9/20 */
    if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
    {
        switch(field_id)
        {
            case TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD:
            case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD:
            case TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD:
                if(sal_atoi(argv[i]) < 12)
                {
                    CFG_INVALID_PARAM_RET("Drop threshold must greater than 11");
                }
                break;
            default: break;
        }
    }
#endif

    /* 5. set field */
    switch (field_id)
    {
    case TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD:
        drop_profile.green_max_threashold = sal_atoi(argv[i]);
        drop_profile.green_min_threashold = drop_profile.green_max_threashold/8;
        if (drop_profile.green_max_threashold > GLB_QOS_DROP_MAX_THRESHOLD)
        {
            CFG_INVALID_PARAM_RET("The maximun of drop threshold is %d", GLB_QOS_DROP_MAX_THRESHOLD);
        }
        tbl_qos_drop_profile_set_qos_drop_profile_field(&drop_profile, TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD);
        flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD;
        if (sal_strlen(argv[i]))
        {
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD);
        }
        else
        {
            GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD);
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD:
        drop_profile.green_min_threashold = sal_atoi(argv[i]);

        if (drop_profile.green_min_threashold >= p_db_drop_profile->green_max_threashold)
        {
            CFG_INVALID_PARAM_RET("Drop minimun threshold must less than maxinum threshold");
        }

        flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD;
        if (sal_strlen(argv[i]))
        {
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD);
        }
        else
        {
            GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD);
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY:

        if (sal_strlen(argv[i]))
        {
            drop_profile.green_probability = sal_atoi(argv[i]);
            flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY;
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB);
        }
        else
        {
            //GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB);
            return rc;
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD:
        drop_profile.yellow_max_threashold = sal_atoi(argv[i]);
        drop_profile.yellow_min_threashold = drop_profile.yellow_max_threashold/8;
        if (drop_profile.yellow_max_threashold > GLB_QOS_DROP_MAX_THRESHOLD)
        {
            CFG_INVALID_PARAM_RET("The maximun of drop threshold is %d", GLB_QOS_DROP_MAX_THRESHOLD);
        }
        tbl_qos_drop_profile_set_qos_drop_profile_field(&drop_profile, TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD);
        flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD;
        if (sal_strlen(argv[i]))
        {
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD);
        }
        else
        {
            GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD);
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD:
        drop_profile.yellow_min_threashold = sal_atoi(argv[i]);

        if (drop_profile.yellow_min_threashold >= p_db_drop_profile->yellow_max_threashold)
        {
            CFG_INVALID_PARAM_RET("Drop minimun threshold must less than maxinum threshold");
        }

        flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD;
        if (sal_strlen(argv[i]))
        {
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD);
        }
        else
        {
            GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD);
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY:
        if (sal_strlen(argv[i]))
        {
            drop_profile.yellow_probability = sal_atoi(argv[i]);
            flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY;
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB);
        }
        else
        {
            //GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB);
            return rc;
        }
        break;
        
    case TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD:
        drop_profile.red_max_threashold = sal_atoi(argv[i]);
        drop_profile.red_min_threashold = drop_profile.red_max_threashold/8;
        if (drop_profile.red_max_threashold > GLB_QOS_DROP_MAX_THRESHOLD)
        {
            CFG_INVALID_PARAM_RET("The maximun of drop threshold is %d", GLB_QOS_DROP_MAX_THRESHOLD);
        }
        tbl_qos_drop_profile_set_qos_drop_profile_field(&drop_profile, TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD);
        flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD;
        if (sal_strlen(argv[i]))
        {
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD);
        }
        else
        {
            GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD);
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD:
        drop_profile.red_min_threashold = sal_atoi(argv[i]);

        if (drop_profile.red_min_threashold >= p_db_drop_profile->red_max_threashold)
        {
            CFG_INVALID_PARAM_RET("Drop minimun threshold must less than maxinum threshold");
        }

        flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD;
        if (sal_strlen(argv[i]))
        {
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD);
        }
        else
        {
            GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD);
        }
        break;

    case TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY:
        if (sal_strlen(argv[i]))
        {
            drop_profile.red_probability = sal_atoi(argv[i]);
            flags_field_id = TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY;
            GLB_SET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_PROB);
        }
        else
        {
            //GLB_UNSET_FLAG(drop_profile.flags, GLB_DROP_PROFILE_FLAGS_RED_PROB);
            return rc;
        }
        break;
             
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }
    
    PM_E_RETURN(tbl_qos_drop_profile_set_qos_drop_profile_field(&drop_profile, field_id));
    if (flags_field_id)
    {
        PM_E_RETURN(tbl_qos_drop_profile_set_qos_drop_profile_field(&drop_profile, flags_field_id));
    }
#if 0    
    /* 6. update handle */
    PM_E_RETURN(qos_mgr_drop_profile_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_drop_profile->name));
#endif    
    return rc;
}

static int32
_qos_mgr_cmd_get_drop_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_drop_profile_t* p_db_drop_profile;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        p_db_drop_profile = tbl_qos_drop_profile_get_profile_by_name(argv[0]);
        if (!p_db_drop_profile)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Qos drop profile %s does not exist", argv[0]);
        }
        rc = tbl_qos_drop_profile_dump_one(p_db_drop_profile, &dump_args);
    }
    else
    {
        field.field_id[0] = TBL_QOS_DROP_PROFILE_FLD_NAME;
        rc = tbl_qos_drop_profile_iterate((TBL_ITER_CB_FUNC)tbl_qos_drop_profile_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_get_port_queue_drop(char *ifname, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_drop_profile_t* p_db_drop_profile;
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(ifname, ifname_ext);
    
    p_db_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(ifname);
    if (NULL == p_db_aclqos_if)
    {
        CFG_INVALID_PARAM_RET("Qos interface %s not exist", ifname_ext);
    }

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_db_drop_profile = tbl_qos_drop_profile_get_profile_by_name(p_db_aclqos_if->queue_drop_profile[i]);
        if (NULL == p_db_drop_profile)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Queue %d drop profile is not specified", i);
        }
        if (0 != p_db_aclqos_if->queue_drop_mode[i])
        {
            GLB_SET_FLAG(p_db_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT);
        }
        else
        {
            GLB_UNSET_FLAG(p_db_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT);
        }
        rc = tbl_qos_drop_profile_dump_one(p_db_drop_profile, &dump_args);
    }
    sal_fclose(fp);

    return rc;
}

int32
qos_mgr_cmd_get_port_queue_shape(char *ifname, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_queue_shape_profile_t* p_db_queue_shape_profile;
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(ifname, ifname_ext);
    
    p_db_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(ifname);
    if ((NULL == p_db_aclqos_if))
    {
        CFG_INVALID_PARAM_RET("Qos interface %s not exist", ifname_ext);
    }

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_db_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(p_db_aclqos_if->queue_shape_profile[i]);
        if (NULL == p_db_queue_shape_profile)
        {
            /* null line indicates queue shape is not configured */
            sal_fprintf(fp, "\n");
            continue;
        }
        rc = tbl_qos_queue_shape_profile_dump_one(p_db_queue_shape_profile, &dump_args);
    }
    sal_fclose(fp);

    return rc;
}

int32
qos_mgr_cmd_get_port_shape(char *ifname, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_port_shape_profile_t* p_db_port_shape_profile = NULL;
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(ifname, ifname_ext);
    p_db_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(ifname);
    if ((NULL == p_db_aclqos_if))
    {
        CFG_INVALID_PARAM_RET("Qos interface %s not exist", ifname_ext);
    }
    p_db_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(p_db_aclqos_if->port_shape_profile);
    if (NULL == p_db_port_shape_profile || !sal_strcmp(p_db_port_shape_profile->name, GLB_QOS_DEF_PORT_SHAPE_PROFILE))
    {
        /* null line indicates port shape is not configured */
        CFG_INVALID_PARAM_RET("None qos port-shape applied on this interface");
    }

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    rc = tbl_qos_port_shape_profile_dump_one(p_db_port_shape_profile, &dump_args);
    sal_fclose(fp);

    return rc;
}


int32
qos_mgr_cmd_process_drop_profile(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();

    /* 1. check conflict */
    QOS_MGR_CMD_CHK_QOS_ENABLE(); 
    
    /* 2. check parameter */
    if (argc)
    {
        rc = cdb_check_name(argv[0], QOS_DROP_NAME_SIZE, para->p_rs->desc); 
        if (CDB_NAME_ERR_NONE != rc)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:        
        PM_E_RETURN(_qos_mgr_cmd_add_drop_profile(argv, argc, para));
        break;

    case CDB_OPER_DEL:        
        PM_E_RETURN(_qos_mgr_cmd_del_drop_profile(argv, argc, para));
        break;

    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_drop_profile_set_drop_profile(argv, argc, para));
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_drop_profile(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

static int32
_qos_cmd_set_glb_buffer_event_enable(tbl_qos_global_t  *p_glb,  tbl_qos_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 chip_type = 0;
    
    /* 1. config check */
    chip_type = tbl_chip_get_type();
    if (GLB_CHIP_GOLDENGATE != chip_type)
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    
    /* 2. check duplicate config */
    if (p_glb->buffer_event_enable== p_db_glb->buffer_event_enable)    
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(qos_mgr_set_glb_buffer_event_enable(p_glb, p_glb->buffer_event_enable));
    return rc;  
}

static int32
_qos_cmd_set_glb_buffer_event_threshold(tbl_qos_global_t  *p_glb,  tbl_qos_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 chip_type = 0;
    
    /* 1. config check */
    chip_type = tbl_chip_get_type();
    if (GLB_CHIP_GOLDENGATE != chip_type)
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    
    if (p_glb->buffer_event_low >= p_glb->buffer_event_high)
    {
        CFG_INVALID_PARAM_RET("The high threshold must be greater than low threshold");
    }

    if ((p_glb->buffer_event_low % GLB_BUFFER_EVENT_LOW_THRESHOLD_DEFAULT) != 0) 
    {
        CFG_INVALID_PARAM_RET("The threshold must be multiple of 16");
    }

    if ((p_glb->buffer_event_high % GLB_BUFFER_EVENT_LOW_THRESHOLD_DEFAULT) != 0)
    {   
        CFG_INVALID_PARAM_RET("The threshold must be multiple of 16");
    }

    /* 2. check duplicate config */
    if ((p_glb->buffer_event_low == p_db_glb->buffer_event_low)
        &&(p_glb->buffer_event_high == p_db_glb->buffer_event_high))
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(qos_mgr_set_glb_buffer_event_threshold(p_glb, p_glb->buffer_event_low, p_glb->buffer_event_high));
    return PM_E_NONE;
}

static int32
_qos_cmd_set_glb_monitor_latency_range(tbl_qos_global_t  *p_glb,  tbl_qos_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    uint32 chip_type = 0;
    
    /* 1. config check */
    chip_type = tbl_chip_get_type();
    if (GLB_CHIP_GOLDENGATE != chip_type)
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    
    for (i=1; i < GLB_MONITOR_LATENCY_RANGE_ID_MAX; i++)
    {
        if (p_glb->latency_range[i] < p_glb->latency_range[i-1])
        {
            CFG_INVALID_PARAM_RET("Range %d must be greater than range %d", i+1, i);
        }
    }

    /* 2. check duplicate config */
    if ((p_glb->latency_range[0] == p_db_glb->latency_range[0])
        &&(p_glb->latency_range[1] == p_db_glb->latency_range[1])
        &&(p_glb->latency_range[2] == p_db_glb->latency_range[2])
        &&(p_glb->latency_range[3] == p_db_glb->latency_range[3])
        &&(p_glb->latency_range[4] == p_db_glb->latency_range[4])
        &&(p_glb->latency_range[5] == p_db_glb->latency_range[5])
        &&(p_glb->latency_range[6] == p_db_glb->latency_range[6])
        &&(p_glb->latency_range[7] == p_db_glb->latency_range[7]))
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(qos_mgr_set_glb_monitor_latency_range(p_glb, p_glb->latency_range));
    return PM_E_NONE;
}

static int32
_qos_cmd_set_qos_ipg(tbl_qos_global_t  *p_glb,  tbl_qos_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (p_glb->qos_ipg_enable == p_db_glb->qos_ipg_enable)
    {
        return PM_E_NONE;
    }

    p_db_glb->qos_ipg_enable = p_glb->qos_ipg_enable;

    /* 2. call API */
    PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_db_glb, TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE));
    return PM_E_NONE;
}

/*added by yejl for gg1.0 queue stats, bug 54212, 2019-10-31*/  
static int32
_qos_cmd_set_qos_queue_shape_enable(tbl_qos_global_t  *p_glb, tbl_qos_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (p_glb->queue_shape_enable == p_db_glb->queue_shape_enable)
    {
        return PM_E_NONE;
    }

    p_db_glb->queue_shape_enable = p_glb->queue_shape_enable;

    /* 2. call API */
    PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_db_glb, TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE));
    return PM_E_NONE;
}
/*ended by yejl*/

static int32
_qos_mgr_cmd_set_qos_global_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_qos_global_t glb;
    tbl_qos_global_t *p_db_glb = tbl_qos_global_get_qos_global();

    /* 1. check parameter: field_str/filed_value/.. */
    if (argc < 2)
    {
        CFG_INVALID_PARAM_RET("Parmameter is not specified");
    }
    /* 2. check exist */
    /* nothing */

    /* 3. update handle: unset */
    /* nothing */
                                    
    sal_memset(&glb, 0, sizeof(glb));

    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_QOS_GLOBAL);
    i = 0;
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    i++;
    /* 5. set field */
    switch (field_id)
    {
    case TBL_QOS_GLOBAL_FLD_QOS_ENABLE:
        glb.qos_enable = sal_atoi(argv[i]);
        if (tbl_qos_global_get_qos_enable() == glb.qos_enable)
        {
            return PM_E_NONE;
        }
        PM_E_RETURN(tbl_qos_global_set_qos_global_field(&glb, field_id));
        qos_mgr_set_qos_enable(glb.qos_enable);
        break;      
    case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT(glb.buffer_event_enable, argv[1], para->p_rs);
        rc = _qos_cmd_set_glb_buffer_event_enable(&glb, p_db_glb, para);
        break;
    case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(glb.buffer_event_low, argv[1], para->p_rs);
        PM_CFG_GET_UINT(glb.buffer_event_high, argv[3], para->p_rs);
        rc = _qos_cmd_set_glb_buffer_event_threshold(&glb, p_db_glb, para);
        break;
    case TBL_QOS_GLOBAL_FLD_LATENCY_RANGE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 9, para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[0], argv[1], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[1], argv[2], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[2], argv[3], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[3], argv[4], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[4], argv[5], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[5], argv[6], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[6], argv[7], para->p_rs);
        PM_CFG_GET_UINT(glb.latency_range[7], argv[8], para->p_rs);
        rc =_qos_cmd_set_glb_monitor_latency_range(&glb, p_db_glb, para);
        break;
    case TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE:
        if(!p_db_glb->qos_enable)
        {
            CFG_INVALID_PARAM_RET("QoS state is disabled");
        }
        break;
    case TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        glb.qos_ipg_enable = (0 == sal_strcmp(argv[1], "enable")) ? 1 : 0; 
        rc =_qos_cmd_set_qos_ipg(&glb, p_db_glb, para);
        break;
    /*added by yejl for gg1.0 queue stats, bug 54212, 2019-10-31*/  
    case TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        glb.queue_shape_enable = (0 == sal_strcmp(argv[1], "enable")) ? 1 : 0; 
        rc =_qos_cmd_set_qos_queue_shape_enable(&glb, p_db_glb, para);
        break;
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }
    
    

   return rc;
}

static int32
_qos_mgr_cmd_get_qos_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_global_t* p_db_global; 

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    /* nothing */
    p_db_global = tbl_qos_global_get_qos_global();
    rc = tbl_qos_global_dump_one(p_db_global, &dump_args);
    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    
    /* 1. check conflict */
    /* 2. check parameter */
    /* nothing */

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_qos_global_field(argv, argc, para));   
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_qos_global(argv, argc, para));   
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}


/*added by yejl for bug 54031, 2019-11-14*/
int32
_qos_mgr_cmd_set_queue_smart_buffer(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 queue_id = 0;
    uint32 smart_buffer_en = 0;
    tbl_qos_queue_smart_buffer_t* p_db_queue_smart_buffer = NULL;
    tbl_qos_queue_smart_buffer_t queue_smart_buffer;
    tbl_aclqos_if_master_t *p_master = tbl_aclqos_if_get_master();
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_qos_drop_profile_t *  p_qos_drop_profile = NULL;
    uint32_t queue_bmp[GLB_QOS_QUEUE_SMART_BUFFER_NUM];
    
    sal_memset(&queue_bmp, 0x0, sizeof(queue_bmp));
    rc = cdb_mpls_label_bitmap_str2val(argv[0], 0, 6, queue_bmp, GLB_QOS_QUEUE_SMART_BUFFER_NUM, para->p_rs->desc);
    if (rc < 0)
    {
	    para->p_rs->rc = rc;
        return rc;
    }

    GLB_BMP_ITER_BEGIN(queue_bmp, queue_id)
    {	
        sal_memset(&queue_smart_buffer, 0x0, sizeof(queue_smart_buffer));
	    smart_buffer_en = sal_atoi(argv[1]);
        queue_smart_buffer.key.queue_id = queue_id;
        queue_smart_buffer.smart_buffer_enable = smart_buffer_en;
        p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&(queue_smart_buffer.key));
        if (NULL == p_db_queue_smart_buffer)
        {
            continue;
        }
        if (smart_buffer_en == p_db_queue_smart_buffer->smart_buffer_enable)
        {
            continue;
        }
        if (smart_buffer_en)
        {
            CTCLIB_SLIST_LOOP(p_master->if_list, p_db_aclqos_if, listnode)
            {
                if (GLB_IF_TYPE_PORT_IF == p_db_aclqos_if->port_type)
                {
                    if (sal_strcmp(p_db_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE))
                    {
                        p_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(p_db_aclqos_if->queue_drop_profile[queue_id]);
                        if (NULL == p_qos_drop_profile)
                        {
                            continue;
                        }
                        p_qos_drop_profile->ref_cnt --;
                        PM_E_RETURN(tbl_qos_drop_profile_set_qos_drop_profile_field(p_qos_drop_profile, TBL_QOS_DROP_PROFILE_FLD_REF_CNT));
                        sal_strcpy(p_db_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE);
                        PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE));
                    }
                    if (p_db_aclqos_if->queue_drop_mode[queue_id] != GLB_QOS_QUEUE_DROP_MODE_WTD)
                    {
                        p_db_aclqos_if->queue_drop_mode[queue_id] = GLB_QOS_QUEUE_DROP_MODE_WTD;
                        PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE));
                    }
                }
            }
        }
        PM_E_RETURN(tbl_qos_queue_smart_buffer_set_qos_queue_smart_buffer_field(&queue_smart_buffer, TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE));
    }
    GLB_BMP_ITER_END(queue_bmp, queue_id);

    return rc;
}

int32
_qos_mgr_cmd_get_queue_smart_buffer(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_qos_queue_smart_buffer_t* p_db_queue_smart_buffer = NULL;
    tbl_qos_queue_smart_buffer_key_t queue_smart_buffer_key;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        queue_smart_buffer_key.queue_id = sal_atoi(argv[0]);
        p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
        if (!p_db_queue_smart_buffer)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Queue %d not exist", queue_smart_buffer_key.queue_id);
        }
        rc = tbl_qos_queue_smart_buffer_dump_one(p_db_queue_smart_buffer, &dump_args);
    }
    else
    {
        rc = tbl_qos_queue_smart_buffer_iterate((TBL_ITER_CB_FUNC)tbl_qos_queue_smart_buffer_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

int32
qos_mgr_cmd_process_queue_smart_buffer(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    
    /* 1. check confilct: */
    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    { 
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    QOS_MGR_CMD_CHK_QOS_ENABLE(); 

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_E_RETURN(_qos_mgr_cmd_set_queue_smart_buffer(argv, argc, para));
        break;

    case CDB_OPER_GET:
        PM_E_RETURN(_qos_mgr_cmd_get_queue_smart_buffer(argv, argc, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }    
    return rc;
}

int32
qos_mgr_cmd_show_queue_drop(tbl_aclqos_if_t* p_aclqos_if, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

#ifdef GREATBELT
        int32 buffer = 256;
        char *cells = "one";
#else
        int32 buffer = 288;
        char *cells = "two";
#endif
    
    tbl_qos_queue_smart_buffer_t* p_db_queue_smart_buffer = NULL;
    tbl_qos_queue_smart_buffer_key_t queue_smart_buffer_key;
    tbl_qos_drop_profile_t* p_qos_drop_profile = NULL;
    uint32 green_max = 0;
    uint32 green_min = 0;
    uint32 green_pro = 0;
    uint32 red_max = 0;
    uint32 red_min = 0;
    uint32 red_pro = 0;
    uint32 yellow_max = 0;
    uint32 yellow_min = 0;
    uint32 yellow_pro = 0;

    uint32 queue_id = 0;

    sal_fprintf(fp,"QUEUE DROP:\n");
    sal_fprintf(fp,"(1)Threshold unit is cell, 1 cell = %d bytes\n", buffer);
    sal_fprintf(fp,"(2)A packet occupies at least %s buffers cell in queue drop threshold\n", cells);
    sal_fprintf(fp,"   and a cell could only be occupied by a packet\n");
    sal_fprintf(fp,"Queue Drop-mode      Color  Max_Treshold  Min_Treshold  Probability\n");
    sal_fprintf(fp,"-----+--------------+------+-------------+-------------+---------\n");
    
    for (queue_id = 0; queue_id < GLB_QOS_PORT_QUEUE_NUM; queue_id ++)
    {
        sal_memset(&queue_smart_buffer_key, 0, sizeof(queue_smart_buffer_key));
        queue_smart_buffer_key.queue_id = queue_id;
        p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
        if (p_db_queue_smart_buffer && p_db_queue_smart_buffer->smart_buffer_enable)
        {
            sal_fprintf(fp,"%-5d %-14s %-6s %-13s %-13s %-9s\n", queue_id, "dynamic", "-", "-", "-", "-");
        }
        else
        {
            p_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(p_aclqos_if->queue_drop_profile[queue_id]);
            if (p_qos_drop_profile)
            {
                green_max = p_qos_drop_profile->green_max_threashold;
                green_min = p_qos_drop_profile->green_min_threashold;
                green_pro = p_qos_drop_profile->green_probability;
                yellow_max = p_qos_drop_profile->yellow_max_threashold;
                yellow_min = p_qos_drop_profile->yellow_min_threashold;
                yellow_pro = p_qos_drop_profile->yellow_probability;
                red_max = p_qos_drop_profile->red_max_threashold;
                red_min = p_qos_drop_profile->red_min_threashold;
                red_pro = p_qos_drop_profile->red_probability;
                
                if (p_aclqos_if->queue_drop_mode[queue_id] == GLB_QOS_QUEUE_DROP_MODE_WTD)
                {
                    sal_fprintf(fp,"%-5d %-14s %-6s %-13d %-13s %-9s\n", queue_id, "tail-drop", "green", green_max, "-", "-");
                    sal_fprintf(fp,"%-20s %-6s %-13d %-13s %-9s\n", "", "yellow", yellow_max, "-", "-");
                    sal_fprintf(fp,"%-20s %-6s %-13d %-13s %-9s\n", "", "red", red_max, "-", "-");
                }
                else
                {
                    sal_fprintf(fp,"%-5d %-14s %-6s %-13d %-13d %-9d\n", queue_id, "random-detect", "green", green_max, green_min, green_pro);
                    sal_fprintf(fp,"%-20s %-6s %-13d %-13d %-9d\n", "", "yellow", yellow_max, yellow_min, yellow_pro);
                    sal_fprintf(fp,"%-20s %-6s %-13d %-13d %-9d\n", "", "red", red_max, red_min, red_pro);
                }
            }
        }
    }
    sal_fprintf(fp,"\n");
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
qos_mgr_cmd_process_show_queue_drop(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(argv[0]);
        if (NULL == p_aclqos_if)
        {
            CFG_CONFLICT_RET("Interface %s does not existed", IFNAME_ETH2FULL(argv[0], ifname_ext));
        }
        
        rc = qos_mgr_cmd_show_queue_drop(p_aclqos_if, para);
        break;
    default:
        break;
    }
    return rc ; 
}
/*ended by yejl*/

