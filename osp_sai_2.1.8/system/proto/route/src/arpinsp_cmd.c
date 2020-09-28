#include "proto.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "route.h"
#include "cdb_const.h"
#include "glb_l2_define.h"
#include "glb_arpacl_define.h"
#include "arp_define.h"
#include "arpinsp_api.h"
#include "arpinsp.h"
#include "arpacl.h"
#include "arpratelimit.h"
#include "arpratelimit_api.h"

/* log buffer queue for arp inspection */
S_ArpInspLogBuf g_arpinsp_logBuffer;

char* 
_arpinsp_get_logfilter_str(uint32 log_filter)
{
    switch(log_filter)
    {
    case GLB_ACL_DHCP_DENY:
        return "deny";
    case GLB_ACL_MATCH_MATCHLOG:
        return "matchlog";
    case GLB_ACL_MATCH_NONE:
        return "none";
    case GLB_DHCP_BINDINGS_ALL:
        return "all";
    case GLB_DHCP_BINDINGS_NONE:
        return "none";
    case GLB_DHCP_BINDINGS_PERMIT:
        return "permit";
    default:
        return "";
    }
}

static int32
_arpinsp_cmd_set_arpinsp_enable_state(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_t *p_db_arpinsp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /*1. p_db_arpinsp check*/
    if (NULL == p_db_arpinsp || NULL == p_arpinsp)
    {
        CFG_FUNC_ERR_RET("The table of arpinsp is NULL!");
    } 
    /*2. duplicate check*/
    if (p_db_arpinsp->enable_state == p_arpinsp->enable_state)
    {
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_arpinsp_enable_state(p_arpinsp);
    return rc;
}

static int32
_arpinsp_cmd_set_arpinsp_filter_state(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_t *p_db_arpinsp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /*1. p_db_arpinsp check*/
    if (NULL == p_db_arpinsp || NULL == p_arpinsp)
    {
        CFG_FUNC_ERR_RET("The table of arpinsp is NULL!");
    } 
    /*2. duplicate check*/
    if ((0 == sal_strcmp(p_db_arpinsp->filter, p_arpinsp->filter)) && (p_db_arpinsp->filter_state == p_arpinsp->filter_state))
    {
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_arpinsp_filter_state(p_arpinsp);
    return rc;
}

static int32
_arpinsp_cmd_set_arpinsp_acl_match(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_t *p_db_arpinsp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /*1. p_db_arpinsp check*/
    if (NULL == p_db_arpinsp || NULL == p_arpinsp)
    {
        CFG_FUNC_ERR_RET("The table of arpinsp is NULL!");
    } 
    /*2. duplicate check*/
    if (p_db_arpinsp->acllog_filter == p_arpinsp->acllog_filter)
    {
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_arpinsp_acl_match(p_arpinsp);
    return rc;
}

static int32
_arpinsp_cmd_set_arpinsp_dhcp_binding(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_t *p_db_arpinsp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /*1. p_db_arpinsp check*/
    if (NULL == p_db_arpinsp || NULL == p_arpinsp)
    {
        CFG_FUNC_ERR_RET("The table of arpinsp is NULL!");
    }  
    /*2. duplicate check*/
    if (p_db_arpinsp->dhcplog_filter == p_arpinsp->dhcplog_filter)
    {
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_arpinsp_dhcp_binding(p_arpinsp);
    return rc;
}

static int32
_arpinsp_cmd_set_logbuf_entrynum(uint32 arpinsp_log_entry_num, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    /*1. p_brg_glb check*/
    if (NULL == p_rt_glb)
    {
        CFG_FUNC_ERR_RET("The table of route_global is NULL!");
    }
    /*2. duplicate check*/
    if (p_rt_glb->arpinsp_logbuf_entrynum == arpinsp_log_entry_num)
    {   
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_logbuf_entrynum(arpinsp_log_entry_num);
    return rc;
}

static int32
 _arpinsp_cmd_set_logbuf_number_and_second(uint32 arpinsp_log_num, uint32 arpinsp_log_second, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    /*1. p_brg_glb check*/
    if (NULL == p_rt_glb)
    {
        CFG_FUNC_ERR_RET("The table of route_global is NULL!");
    }
    /*2. duplicate check*/
    if (p_rt_glb->arpinsp_logbuf_lognum == arpinsp_log_num && 
        p_rt_glb->arpinsp_logbuf_logsec == arpinsp_log_second)
    {   
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_logbuf_number_and_second(arpinsp_log_num, arpinsp_log_second);
    return rc;        
}

static int32
_arpinsp_cmd_set_arpinsp_validate(uint32 arpinsp_validate_flag, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    /*1. p_brg_glb check*/
    if (NULL == p_rt_glb)
    {
        CFG_FUNC_ERR_RET("The table of route_global is NULL!");
    }
    /*2. duplicate check*/
    if (p_rt_glb->arpinsp_validate_flag == arpinsp_validate_flag)
    {
        return PM_E_NONE;
    }
    /*3. call API */
    rc = arpinsp_set_arpinsp_validate(arpinsp_validate_flag);
    return rc;
}

int32
_arpinsp_cmd_read_arpinsp(vid_t vid, cfg_cmd_para_t *para)
{
    tbl_arpinsp_key_t arpinsp_key;
    tbl_arpinsp_t *p_db_arpinsp = NULL;
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    /* 1. check exist */
    arpinsp_key.vlan_id = vid;
    p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
    if (NULL == p_db_arpinsp)
    {
        sal_fclose(fp);
        return PM_E_NONE;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    /* 2. dump arpinsp */
    tbl_arpinsp_dump_one(p_db_arpinsp, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
_arpinsp_cmd_read_arpinsp_all(cfg_cmd_para_t *para)
{
    tbl_arpinsp_key_t arpinsp_key;
    tbl_arpinsp_t *p_db_arpinsp = NULL;
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    vid_t vid = 0;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    for (vid = GLB_VLAN_DEFAULT_VID; vid < GLB_MAX_VLAN_ID; vid++)
    {
        /* 1. check exist */
        arpinsp_key.vlan_id = vid;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }

        sal_memset(&args, 0 ,sizeof(args));
        p_field = &field;
        sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
        args.argv[0] = p_field;
        args.argv[1] = fp;

        /* 2. dump arp inspection */
        tbl_arpinsp_dump_one(p_db_arpinsp, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
arpinsp_cmd_process_arpinsp(char **argv, int32 argc, cfg_cmd_para_t *para)
{    
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_arpinsp_t arpinsp;
    tbl_arpinsp_t *p_db_arpinsp = NULL;
    tbl_arpacl_config_t *p_db_arpacl_config = NULL;
    tbl_arpacl_config_key_t arpacl_config_key;
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    vid_t  vid = 0;     
    int32  i = 0;
    uint32 total_count = 0;
    uint32 no_vlan_num = 0;
    uint32 acl_match_type =0;
    uint32 dhcp_binding_type =0;
    uint32 arpinsp_log_entry_num = 0;
    uint32 arpinsp_log_num = 0;
    uint32 arpinsp_log_second = 0;
    bool   filter_state = 0;
    bool   enable_state = FALSE;
    bool   validate_check = FALSE;
    char   err_desc[256];
    int32  rc = PM_E_NONE;
    vlan_bmp_t bmp;

    sal_memset(bmp, 0, sizeof(bmp));

    CFG_DBG_FUNC();    

    switch (para->oper)    
    {    
    case CDB_OPER_ADD:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        if (argc == 1 || argc == 4)
        {
            rc = cdb_bitmap_str2val(argv[0], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
            if (rc < 0)
            {
                /*modified by yejl to fix bug44057, 2017-05-18*/
                CFG_INVALID_PARAM_RET("%s", err_desc);
            }
            if (argc == 1) /*set arpinsp vlan enable*/
            {
                enable_state = TRUE;
            }
            else          /*set arpinsp vlan logging*/
            {   
                if (0 == sal_strcmp(argv[2], "acl-match"))
                {
                    if (0 == sal_strcmp(argv[3], "matchlog"))
                    {
                        acl_match_type = GLB_ACL_MATCH_MATCHLOG;
                    }
                    else if (0 == sal_strcmp(argv[3], "none"))
                    {
                        acl_match_type = GLB_ACL_MATCH_NONE;    
                    }
                    else if (0 == sal_strcmp(argv[3], "deny"))
                    {
                        acl_match_type = GLB_ACL_DHCP_DENY;    
                    }
                }
                else if (0 == sal_strcmp(argv[2], "dhcp-bindings"))
                {
                    if (0 == sal_strcmp(argv[3], "all"))
                    {
                        dhcp_binding_type = GLB_DHCP_BINDINGS_ALL;
                    }
                    else if (0 == sal_strcmp(argv[3], "none"))
                    {
                        dhcp_binding_type = GLB_DHCP_BINDINGS_NONE;
                    }
                    else if (0 == sal_strcmp(argv[3], "permit"))
                    {
                        dhcp_binding_type = GLB_DHCP_BINDINGS_PERMIT;
                    }
                    else if (0 == sal_strcmp(argv[3], "deny"))
                    {
                        dhcp_binding_type = GLB_ACL_DHCP_DENY;
                    }
                }
            }
        }
        else if (argc == 3)  /*set arpinsp vlan filter*/
        {
            if (check_name_character_and_len(argv[0], ARPINSP_FILTER_MAX_NUM) != 0)
            {
                CFG_INVALID_PARAM_RET("The first character should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is %d.", ARPINSP_FILTER_MAX_NUM);
            }
            
            /*modified by yejl for bug 49965, 2018-11-13*/
            sal_memset(&arpacl_config_key, 0, sizeof(arpacl_config_key));
            sal_strncpy(arpacl_config_key.name, argv[0], ARPACL_NAME_SIZE);
            p_db_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_config_key);
            if (NULL == p_db_arpacl_config)
            {
                CFG_INVALID_PARAM_RET("ARP ACL %s does not exist", argv[0]);
            }
            /*end*/
            
            rc = cdb_bitmap_str2val(argv[1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("%s", err_desc);
            }
            if (0 == sal_strcmp(argv[2], "dynamic"))
            {
                filter_state = GLB_ARPINSP_ACL_DYNAMIC;
            }
            else if (0 == sal_strcmp(argv[2], "static"))
            {
                filter_state = GLB_ARPINSP_ACL_STATIC;
            }
        }
        
        GLB_BMP_ITER_BEGIN(bmp, i)
        {
            total_count++;
            sal_memset(&vlan_key, 0, sizeof(tbl_vlan_key_t));
            vlan_key.vid = i;
            p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (NULL == p_db_vlan)
            {
                no_vlan_num++;
                continue;
            }
            sal_memset(&arpinsp, 0, sizeof(tbl_arpinsp_t));
            arpinsp.key.vlan_id = i; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&(arpinsp.key));
            if (NULL == p_db_arpinsp)
            {
                arpinsp.key.vlan_id = i;
                rc = tbl_arpinsp_add_arpinsp(&arpinsp);
                if (PM_E_NONE != rc)
                {
                    return rc;
                }
                arpinsp.key.vlan_id = i; 
                p_db_arpinsp = tbl_arpinsp_get_arpinsp(&(arpinsp.key));
                if (NULL == p_db_arpinsp)
                {   
                    return PM_E_NOT_EXIST;
                }
            }
            if (argc == 1)      /*set vlan enable*/
            {
                arpinsp.enable_state = enable_state;
                rc = _arpinsp_cmd_set_arpinsp_enable_state(&arpinsp, p_db_arpinsp, para);
            }
            else if (argc == 3)  /*set filter type*/
            {
                arpinsp.filter_state = filter_state;
                sal_strncpy(arpinsp.filter, argv[0], ARPINSP_FILTER_MAX_NUM);
                rc = _arpinsp_cmd_set_arpinsp_filter_state(&arpinsp, p_db_arpinsp, para);
            }
            else if (argc == 4)  /*set logging type*/
            {
                if (0 == sal_strcmp(argv[2], "acl-match"))
                {
                    arpinsp.acllog_filter = acl_match_type;
                    rc = _arpinsp_cmd_set_arpinsp_acl_match(&arpinsp, p_db_arpinsp, para);
                }
                else if (0 == sal_strcmp(argv[2], "dhcp-bindings"))
                {
                    arpinsp.dhcplog_filter = dhcp_binding_type;
                    rc = _arpinsp_cmd_set_arpinsp_dhcp_binding(&arpinsp, p_db_arpinsp, para);
                }
            }
        }
        GLB_BMP_ITER_END(bmp, i);

        /*notice*/
        if (total_count == 1 && no_vlan_num == 1)
        {
            CFG_INVALID_PARAM_RET("The vlan is not existent");
        }
        else if (no_vlan_num == 1)
        {
            CFG_INVALID_PARAM_RET("One of the vlans is not existent");
        }
        else if (no_vlan_num > 1)
        {
            CFG_INVALID_PARAM_RET("Some of the vlans are not existent");
        }
        break;    
        
     case CDB_OPER_DEL:        
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs); 
        if (argc == 1 || argc == 3)
        {
            rc = cdb_bitmap_str2val(argv[0], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("%s", err_desc);
            }
            if (argc == 1)    /*set arpinsp vlan disable*/
            {
                enable_state = FALSE;
            }
            else              /*set arpinsp logging none*/
            {
                if (0 == sal_strcmp(argv[2], "acl-match"))
                {
                    acl_match_type = GLB_ACL_DHCP_DENY;
                }
                else if (0 == sal_strcmp(argv[2], "dhcp-bindings"))
                {
                    dhcp_binding_type = GLB_ACL_DHCP_DENY;
                }
            }
        }
        else if (argc == 2)  /*set arpinsp filter none*/
        {
            if (check_name_character_and_len(argv[0], ARPINSP_FILTER_MAX_NUM) != 0)
            {
                CFG_INVALID_PARAM_RET("The first character should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is %d.", ARPINSP_FILTER_MAX_NUM);
            }
            rc = cdb_bitmap_str2val(argv[1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("%s", err_desc);
            }
            filter_state = GLB_ARPINSP_ACL_CLEAR;
        }
        GLB_BMP_ITER_BEGIN(bmp, i)
        {
            sal_memset(&arpinsp, 0, sizeof(tbl_arpinsp_t));
            arpinsp.key.vlan_id = i; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&(arpinsp.key));
            if (NULL == p_db_arpinsp)
            {
                continue;
            }
            if (argc == 1)      /*set arpinsp vlan disable*/
            {
                arpinsp.enable_state = enable_state;
                rc = _arpinsp_cmd_set_arpinsp_enable_state(&arpinsp, p_db_arpinsp, para);
            }
            else if (argc == 2)  /*set arpinsp filter none*/
            {
                arpinsp.filter_state = filter_state;
                if (!sal_strcmp(argv[0], p_db_arpinsp->filter))
                {
                    sal_memset(arpinsp.filter, 0, ARPINSP_FILTER_MAX_NUM);
                    rc = _arpinsp_cmd_set_arpinsp_filter_state(&arpinsp, p_db_arpinsp, para);
                }
            }
            else if (argc == 3) /*set arpinsp logging none*/
            {
                if (0 == sal_strcmp(argv[2], "acl-match"))
                {
                    arpinsp.acllog_filter = acl_match_type;
                    rc = _arpinsp_cmd_set_arpinsp_acl_match(&arpinsp, p_db_arpinsp, para);
                }
                else if (0 == sal_strcmp(argv[2], "dhcp-bindings"))
                {
                    arpinsp.dhcplog_filter = dhcp_binding_type;
                    rc = _arpinsp_cmd_set_arpinsp_dhcp_binding(&arpinsp, p_db_arpinsp, para);
                }
            }

            /*if all of the field is 0, delete the arpinsp*/
            arpinsp.key.vlan_id = i; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&(arpinsp.key));
            if (p_db_arpinsp && arpinsp_is_NULL(p_db_arpinsp))
            {
                PM_E_RETURN(tbl_arpinsp_del_arpinsp(&(arpinsp.key)));
            }
        }
        GLB_BMP_ITER_END(bmp, i);
        break;  
        
     case CDB_OPER_SET:        
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs); 
        if (argc == 2)              /*set arpinsp log entry number*/
        {
            if (0 == sal_strcmp(argv[0], "entries"))
            {
                PM_CFG_GET_UINT(arpinsp_log_entry_num, argv[1], para->p_rs);
                rc = _arpinsp_cmd_set_logbuf_entrynum(arpinsp_log_entry_num, para);
            }
        }
        else if (argc == 3)         /*set arpinsp log number and seconds*/
        {
            if (0 == sal_strcmp(argv[0], "logs"))
            {
                PM_CFG_GET_UINT(arpinsp_log_num, argv[1], para->p_rs);
                PM_CFG_GET_UINT(arpinsp_log_second, argv[2], para->p_rs);
                rc = _arpinsp_cmd_set_logbuf_number_and_second(arpinsp_log_num, arpinsp_log_second, para);
            }
        }
        if (argc == 4)               /*set arpinsp validate*/
        {
            uint32 arpinsp_validate_flag = 0;
            uint32 num = 0;
            PM_CFG_GET_UINT(validate_check, argv[3], para->p_rs);
            if (validate_check)    /*set the arpinsp validate*/
            {
                for (num = 0; num < argc-1; num++)
                {
                    if (0 == sal_strcmp(argv[num], "dst-mac"))
                    {
                        GLB_SET_FLAG(arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_DESTMAC);
                    }
                    if (0 == sal_strcmp(argv[num], "ip"))
                    {
                        GLB_SET_FLAG(arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_IP);
                    }
                    if (0 == sal_strcmp(argv[num], "src-mac"))
                    {
                        GLB_SET_FLAG(arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_SRCMAC);
                    }
                }
            }
            else                    /*unset the arpinsp validate*/
            {
                arpinsp_validate_flag = p_rt_glb->arpinsp_validate_flag;
                for (num = 0; num < argc-1; num++)
                {
                    if (0 == sal_strcmp(argv[num], "dst-mac"))
                    {
                        if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_DESTMAC))
                        {
                            GLB_UNSET_FLAG(arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_DESTMAC);
                        }
                    }
                    if (0 == sal_strcmp(argv[num], "ip"))
                    {
                        if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_IP))
                        {
                            GLB_UNSET_FLAG(arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_IP);
                        }
                    }
                    if (0 == sal_strcmp(argv[num], "src-mac"))
                    {
                        if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_SRCMAC))
                        {
                            GLB_UNSET_FLAG(arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_SRCMAC);
                        }
                    }
                }
            }
            rc = _arpinsp_cmd_set_arpinsp_validate(arpinsp_validate_flag, para);
        }
        break;
     case CDB_OPER_GET:        
        if (1 == argc)
        {
            PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
            rc = _arpinsp_cmd_read_arpinsp(vid, para); 
        }
        else
        {
            rc = _arpinsp_cmd_read_arpinsp_all(para);
        }        
        break;
    default:
        break;
    }   
    return rc;
}

static int32
_arpinsp_cmd_show_interface_trust_banner(FILE *fp)
{
    sal_fprintf(fp,"Interface      Trust State\n");
    sal_fprintf(fp,"--------------+-------------\n");
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_vlan1_banner(FILE *fp)
{
    sal_fprintf(fp,"Vlan    Configuration      ACL Match     Static ACL\n");
    sal_fprintf(fp,"-------+------------------+-------------+------------\n");
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_vlan2_banner(FILE *fp)
{
    sal_fprintf(fp,"Vlan     ACL Logging     DHCP Logging\n");
    sal_fprintf(fp,"--------+---------------+--------------\n");
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_vlan3_banner(FILE *fp)
{
    sal_fprintf(fp,"Vlan     Forwarded      Dropped     DHCP Drops     ACL Drops\n");
    sal_fprintf(fp,"--------+--------------+-----------+--------------+-----------\n");
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_vlan4_banner(FILE *fp)
{
    sal_fprintf(fp,"Vlan     DHCP Permits     ACL Permits     Source MAC Failures\n");
    sal_fprintf(fp,"--------+----------------+---------------+--------------------\n");
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_vlan5_banner(FILE *fp)
{
    sal_fprintf(fp,"Vlan     Dest MAC Failures  IP Validation Failures  Invalid Protocol Data\n");
    sal_fprintf(fp,"--------+------------------+-----------------------+----------------------\n");
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_interface_numberlimit_all_banner(FILE *fp)
{
    sal_fprintf(fp,"ArpNumLimit Port  MaxLimitNum  CurrentNum  ViolationMode\n");
    sal_fprintf(fp,"-----------------+------------+-----------+--------------\n");
    return PM_E_NONE;
}


int32
_arpinsp_cmd_show_interface_trust_one(tbl_interface_t* p_db_if, FILE *fp)
{
    char ifname_ext[GLB_IFNAME_SZ];
    bool trust_enable = FALSE;
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext); 
    if (p_db_if->arpinsp_trust_enable)
    {
        trust_enable = TRUE;
    }
    else
    {
        trust_enable = FALSE;
    }
    sal_fprintf(fp, "%-14s %-13s\n", ifname_ext, (trust_enable? "trusted":"untrusted"));
    return PM_E_NONE;
}

int32
_arpinsp_cmd_show_interface_trust(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    bool interface_config = FALSE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (p_db_if)
    {
        _arpinsp_cmd_show_interface_trust_banner(fp);
        _arpinsp_cmd_show_interface_trust_one(p_db_if, fp);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF || p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
            {
                if (interface_config == FALSE)
                {
                    _arpinsp_cmd_show_interface_trust_banner(fp);
                    interface_config = TRUE;
                }
                _arpinsp_cmd_show_interface_trust_one(p_db_if, fp);
            }
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
_arpinsp_show_interface_numberlimit_all(tbl_interface_t *p_db_if, FILE* fp)
{          
    tbl_route_if_t * p_db_rt_if = NULL;
    tbl_route_if_key_t rt_if_key;
    char type_str[CMD_BUF_128];
    char ifname_ext[GLB_IFNAME_SZ]; 
    uint32 MaxLimitNum = 0;
    uint32 LimitMode = 0;
    uint32 currLimitNum = 0;
      
    MaxLimitNum = p_db_if->arp_numberlimit_number;
    LimitMode = p_db_if->arp_numberlimit_violate;

    sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memcpy(rt_if_key.name, p_db_if->key.name, sizeof(rt_if_key.name));
    p_db_rt_if = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rt_if)
    {
        sal_fprintf(fp, "Route interface is not exist\n");
        sal_fclose(fp);  
        fp = NULL;
        return PM_E_NONE;
    }

    type_str[0] = CMD_ZERO_CHAR;
    if (ARP_PORT_NUMBERLIMIT_PROTECT == LimitMode)
    {
         sal_strcat(type_str, "protect");
    }
    else if(ARP_PORT_NUMBERLIMIT_RESTRICT == LimitMode)
    {
         sal_strcat(type_str, "restrict");
    }
    else if (ARP_PORT_NUMBERLIMIT_ERRDISABLE == LimitMode)
    {
         sal_strcat(type_str, "errdisable");
    }
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext); 
    currLimitNum = p_db_rt_if->arp_curr_dynamic;
      
    sal_fprintf(fp, "%-17s %-12d %-11d %-14s\n",
    ifname_ext, MaxLimitNum, currLimitNum, type_str);
      
    return PM_E_NONE;
}

int32
_arpinsp_cmd_show_interface_numberlimit_all(cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    tbl_interface_t * p_db_if = NULL;
    bool numberlimit_enable = FALSE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_MODE_L3 != p_db_if->mode)
        {
            continue;
        }
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        if (p_db_if->arp_numberlimit_enable)
        {
            if (numberlimit_enable == FALSE)
            {
                _arpinsp_cmd_show_interface_numberlimit_all_banner(fp);
            }
            _arpinsp_show_interface_numberlimit_all(p_db_if, fp);
            numberlimit_enable = TRUE;
        }
    }
    if (numberlimit_enable == FALSE)
    {
        sal_fprintf(fp, "No arp number limit in system");
        sal_fclose(fp);  
        fp = NULL;
        return PM_E_NONE;
    }
    
    sal_fclose(fp);  
    fp = NULL;
    return PM_E_NONE;
}

int32
_arpinsp_cmd_show_interface_numberlimit_status(tbl_interface_t * p_db_if, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_route_if_t * p_db_rt_if = NULL;
    tbl_route_if_key_t rt_if_key;
    uint32 currlimit_num = 0;

    sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memcpy(rt_if_key.name, p_db_if->key.name, sizeof(rt_if_key.name));
    p_db_rt_if = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rt_if)
    {
        sal_fprintf(fp, "%% Route interface is not exist\n");
        sal_fclose(fp);  
        fp = NULL;
        return PM_E_NONE;
    }
    
    currlimit_num = p_db_rt_if->arp_curr_dynamic;
    sal_fprintf(fp,"Arp number limit                    : %s\n", (p_db_if->arp_numberlimit_enable)? "enabled":"disabled");
    if (p_db_if->arp_numberlimit_violate == ARP_PORT_NUMBERLIMIT_PROTECT)
         sal_fprintf(fp,"Arp number limit violate mode       : %s\n", "discard packet silence");
    else if (p_db_if->arp_numberlimit_violate == ARP_PORT_NUMBERLIMIT_RESTRICT)
         sal_fprintf(fp,"Arp number limit violate mode       : %s\n", "discard packet and log");
    else if (p_db_if->arp_numberlimit_violate == ARP_PORT_NUMBERLIMIT_ERRDISABLE)
         sal_fprintf(fp,"Arp number limit violate mode       : %s\n", "discard packet, log and set error-disabled");
    
    sal_fprintf(fp,"Arp number limit maximum            : %d \n",  p_db_if->arp_numberlimit_number);
    sal_fprintf(fp,"ARP number limit total number       : %d \n",  currlimit_num);

    sal_fclose(fp);  
    fp = NULL;
    
    return PM_E_NONE;
}

void
arpinsp_show_log(FILE *fp, S_ArpInspLogBuf *logList, uint32 log_num)
{
    S_ArpInspLogItem *pItem = NULL;
    int nCnt = 0;

    if (0 == log_num || NULL == fp)
    {
        return;
    }

    if (log_num < 0)
    {
        nCnt = -log_num;
        TAILQ_FOREACH(pItem, logList, entry)
        {
            sal_fprintf(fp, "%s: %s", pItem->time_str, pItem->pszLog);
            nCnt--;
            if (nCnt <= 0)
            {
                break;
            }
        }
    }
    else
    {
        nCnt = log_num;
        TAILQ_FOREACH_REVERSE(pItem, logList, tagLogQueue, entry)
        {
            sal_fprintf(fp, "%s: %s", pItem->time_str, pItem->pszLog);
            nCnt--;
            if (nCnt <= 0)
            {
                break;
            }
        }
    }
    return;
}

int32
arpinsp_cmd_show_log(uint32 log_num, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_route_global_t* p_rt_glb = tbl_route_global_get_route_global();
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "Total Log Buffer Size : %d\n", p_rt_glb->arpinsp_logbuf_entrynum);
    sal_fprintf(fp, "Syslog rate : %d entries per %d seconds.\n", p_rt_glb->arpinsp_logbuf_lognum, p_rt_glb->arpinsp_logbuf_logsec);

    if (0 == p_rt_glb->arpinsp_logbuf_curnum)
    {
        sal_fprintf(fp, "\nNo entries in log buffer\n");
    }
    else 
    {
        sal_fprintf(fp, "\n");
        arpinsp_show_log(fp, &g_arpinsp_logBuffer, log_num);
    }
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_show_validate(FILE *fp)
{
    tbl_route_global_t* p_rt_glb = tbl_route_global_get_route_global();
    if (NULL == p_rt_glb)
    {
        sal_fprintf(fp, "Tbl route global is NULL\n");
        return PM_E_NONE;
    }
    sal_fprintf(fp, "Source Mac Validation      : %s \n", 
        GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_SRCMAC)? "Enabled":"Disabled");
    sal_fprintf(fp, "Destination Mac Validation : %s \n", 
        GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_DESTMAC)? "Enabled":"Disabled");
    sal_fprintf(fp, "IP Address Validation      : %s \n", 
        GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_IP)? "Enabled":"Disabled");
    return PM_E_NONE;
}

static int32 
_arpinsp_cmd_show_vlan1(tbl_arpinsp_t* p_db_arpinsp, FILE *fp)
{
    if (p_db_arpinsp->enable_state)
    {
        if (p_db_arpinsp->filter_state == GLB_ARPINSP_ACL_STATIC)
        {
            sal_fprintf(fp, "%-7d %-18s %-13s %-12s\n", p_db_arpinsp->key.vlan_id, "enabled", p_db_arpinsp->filter, "static");
        }
        else
        {
            sal_fprintf(fp, "%-7d %-18s %-13s %-12s\n", p_db_arpinsp->key.vlan_id, "enabled", p_db_arpinsp->filter, "");
        }
    }
    return PM_E_NONE;
}

static int32 
_arpinsp_cmd_show_vlan2(tbl_arpinsp_t* p_db_arpinsp, FILE *fp)
{
    uint32 acl_log = 0;
    uint32 dhcp_log = 0;
    if (p_db_arpinsp->enable_state)
    {
        acl_log = p_db_arpinsp->acllog_filter;
        dhcp_log = p_db_arpinsp->dhcplog_filter;
        
        sal_fprintf(fp, "%-8d %-15s %-14s\n", 
            p_db_arpinsp->key.vlan_id, _arpinsp_get_logfilter_str(acl_log), _arpinsp_get_logfilter_str(dhcp_log));
    }
    return PM_E_NONE;
}

static int32 
_arpinsp_cmd_show_vlan3(tbl_arpinsp_t* p_db_arpinsp, FILE *fp)
{
    if (p_db_arpinsp->enable_state)
    {   
        sal_fprintf(fp, "%-8d %-14"PRIu64" %-11"PRIu64" %-14"PRIu64" %-11"PRIu64"\n", 
            p_db_arpinsp->key.vlan_id, p_db_arpinsp->forwarded, p_db_arpinsp->dropped, p_db_arpinsp->dhcp_dropped, p_db_arpinsp->acl_dropped);
    }
    return PM_E_NONE;
}

static int32 
_arpinsp_cmd_show_vlan4(tbl_arpinsp_t* p_db_arpinsp, FILE *fp)
{
    if (p_db_arpinsp->enable_state)
    {   
        sal_fprintf(fp, "%-8d %-16"PRIu64" %-15"PRIu64" %-20"PRIu64"\n", 
            p_db_arpinsp->key.vlan_id, p_db_arpinsp->dhcp_permit, p_db_arpinsp->acl_permit, p_db_arpinsp->srcmac_failure);
    }
    return PM_E_NONE;
}

static int32 
_arpinsp_cmd_show_vlan5(tbl_arpinsp_t* p_db_arpinsp, FILE *fp)
{
    if (p_db_arpinsp->enable_state)
    {   
        sal_fprintf(fp, "%-8d %-18"PRIu64" %-23"PRIu64" %-22"PRIu64"\n", 
            p_db_arpinsp->key.vlan_id, p_db_arpinsp->dstmac_failure, p_db_arpinsp->ip_failure, p_db_arpinsp->invalid_protocol);
    }
    return PM_E_NONE;
}

static int32
_arpinsp_cmd_get_vlan_str(char* vlan_str, uint32* bmp)
{
    vid_t vid = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    char vlan_tmp[16] = {'\0'};
    uint32 has_left_vlan = FALSE;
    vlan_bmp_t tmp_bmp;
    sal_memset(tmp_bmp, 0, sizeof(tmp_bmp));
    sal_memcpy(tmp_bmp, bmp, sizeof(vlan_bmp_t));
    
    for (offset = GLB_DEF_VLAN_ID; offset < GLB_BMP_BITS(tmp_bmp); offset++)
    {
        if (GLB_BMP_ISSET(tmp_bmp, offset))
        {
            vid = offset;
            end_vid = vid;
            do 
            {
                end_vid++;
            } while (GLB_BMP_ISSET(tmp_bmp, end_vid));
            end_vid--;
            offset = end_vid;
            if (vid == end_vid)
            {
                sal_sprintf(vlan_tmp, "%u,", vid);
                sal_strncat(vlan_str, vlan_tmp, 5);
                has_left_vlan = TRUE;
            }                
            else
            {          
                sal_sprintf(vlan_tmp, "%u-%u,", vid, end_vid);
                sal_strncat(vlan_str, vlan_tmp, 10);
                has_left_vlan = TRUE;
            }                             
        }
    }

    if (has_left_vlan)
    {
        vlan_str[sal_strlen(vlan_str)-1] = CMD_ZERO_CHAR;
    }
    return PM_E_NONE;
}

int32
_arpinsp_cmd_show_vlan_all(uint32 vlan_enable, uint32 statistics_enable, uint32 vlan_all, uint32* bmp, cfg_cmd_para_t *para)
{
    uint32 vid = 0;
    char   vlan_str[256] = {'\0'};
    tbl_arpinsp_t* p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    bool vlan1_config = FALSE;
    bool vlan2_config = FALSE;
    bool vlan3_config = FALSE;
    bool vlan4_config = FALSE;
    bool vlan5_config = FALSE;
    uint32 arpinsp_count1 = 0;
    uint32 arpinsp_count2 = 0;
    uint32 arpinsp_count3 = 0;
    uint32 arpinsp_count4 = 0;
    vlan_bmp_t tmp_bmp1;
    vlan_bmp_t tmp_bmp2;
    vlan_bmp_t tmp_bmp3;
    vlan_bmp_t tmp_bmp4;
    vlan_bmp_t tmp_bmp5;
    
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(tmp_bmp1, 0, sizeof(tmp_bmp1));
    sal_memset(tmp_bmp2, 0, sizeof(tmp_bmp2));
    sal_memset(tmp_bmp3, 0, sizeof(tmp_bmp3));
    sal_memset(tmp_bmp4, 0, sizeof(tmp_bmp4));
    sal_memset(tmp_bmp5, 0, sizeof(tmp_bmp5));
    sal_memcpy(tmp_bmp1, bmp, sizeof(vlan_bmp_t));
    sal_memcpy(tmp_bmp2, bmp, sizeof(vlan_bmp_t));
    sal_memcpy(tmp_bmp3, bmp, sizeof(vlan_bmp_t));
    sal_memcpy(tmp_bmp4, bmp, sizeof(vlan_bmp_t));
    sal_memcpy(tmp_bmp5, bmp, sizeof(vlan_bmp_t));

    if (vlan_enable || vlan_all)
    {
        _arpinsp_cmd_show_validate(fp);
        sal_fprintf(fp, "\n");

        GLB_BMP_ITER_BEGIN(tmp_bmp1, vid)
        {
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = vid; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                continue;
            }
            if (p_db_arpinsp->enable_state)
            {
                arpinsp_count1++; 
                if (FALSE == vlan1_config)
                {
                    _arpinsp_cmd_show_vlan1_banner(fp);
                    _arpinsp_cmd_show_vlan1(p_db_arpinsp, fp);
                    vlan1_config = TRUE;
                }
                else
                {
                    _arpinsp_cmd_show_vlan1(p_db_arpinsp, fp);
                }
            }
        }
        GLB_BMP_ITER_END(tmp_bmp1, vid);
        if (arpinsp_count1)
        {
            sal_fprintf(fp, "\n");
        }

        vid = 0;
        GLB_BMP_ITER_BEGIN(tmp_bmp2, vid)
        {
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = vid; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                continue;
            }
            if (p_db_arpinsp->enable_state)
            {
                arpinsp_count2++;
                if (FALSE == vlan2_config)
                {
                    _arpinsp_cmd_show_vlan2_banner(fp);
                    _arpinsp_cmd_show_vlan2(p_db_arpinsp, fp);
                    vlan2_config = TRUE;
                }
                else
                {
                    _arpinsp_cmd_show_vlan2(p_db_arpinsp, fp);
                }
            }
        }
        GLB_BMP_ITER_END(tmp_bmp2, vid);
        if (arpinsp_count2)
        {
            sal_fprintf(fp, "\n");
        }
    }

    if (statistics_enable || vlan_all)
    {
        GLB_BMP_ITER_BEGIN(tmp_bmp3, vid)
        {
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = vid; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                continue;
            }
            if (p_db_arpinsp->enable_state)
            {
                arpinsp_count3++;
                if (FALSE == vlan3_config)
                {
                    _arpinsp_cmd_show_vlan3_banner(fp);
                    _arpinsp_cmd_show_vlan3(p_db_arpinsp, fp);
                    vlan3_config = TRUE;
                }
                else
                {
                    _arpinsp_cmd_show_vlan3(p_db_arpinsp, fp);
                }
            }
        }
        GLB_BMP_ITER_END(tmp_bmp3, vid);
        if (arpinsp_count3)
        {   
            sal_fprintf(fp, "\n");
        }

        vid = 0;
        GLB_BMP_ITER_BEGIN(tmp_bmp4, vid)
        {
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = vid; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                continue;
            }
            if (p_db_arpinsp->enable_state)
            {
                arpinsp_count4++;
                if (FALSE == vlan4_config)
                {
                    _arpinsp_cmd_show_vlan4_banner(fp);
                    _arpinsp_cmd_show_vlan4(p_db_arpinsp, fp);
                    vlan4_config = TRUE;
                }
                else
                {
                    _arpinsp_cmd_show_vlan4(p_db_arpinsp, fp);
                }
            }
        }
        GLB_BMP_ITER_END(tmp_bmp4, vid);
        if (arpinsp_count4)
        {
            sal_fprintf(fp, "\n");
        }

        vid = 0;
        GLB_BMP_ITER_BEGIN(tmp_bmp5, vid)
        {
            sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
            arpinsp_key.vlan_id = vid; 
            p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
            if (NULL == p_db_arpinsp)
            {
                continue;
            }
            if (p_db_arpinsp->enable_state)
            {
                if (FALSE == vlan5_config)
                {
                    _arpinsp_cmd_show_vlan5_banner(fp);
                    _arpinsp_cmd_show_vlan5(p_db_arpinsp, fp);
                    vlan5_config = TRUE;
                }
                else
                {
                    _arpinsp_cmd_show_vlan5(p_db_arpinsp, fp);
                }
            }
        }
        GLB_BMP_ITER_END(tmp_bmp5, vid);
    }

    if ((0 == arpinsp_count1) && (0 == arpinsp_count3))
    {
        _arpinsp_cmd_get_vlan_str(vlan_str, bmp);
        if (0 == sal_strcmp(vlan_str, "1-4094"))
        {
            /*modified by yejl to fix bug44123, 2017-05-23*/
            sal_fprintf(fp, "No vlan enabled ARP Inspection on switch\n");
        }
        else
        {
            sal_fprintf(fp, "%% No VLAN enabled ARP inspection in VLAN: %s\n", vlan_str);
        }
        sal_fclose(fp);
        fp = NULL;
        return PM_E_NONE;
    }
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
arpinsp_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    uint32 log_num = 0;
    char err_desc[256];
    int32 rc = PM_E_NONE;
    vlan_bmp_t bmp;
    char ifname[IFNAME_SIZE] = {0};
    tbl_route_if_t *p_db_rtif = NULL;
    
    sal_memset(bmp, 0, sizeof(bmp));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 0)      /*show ip arp inspection*/
        {
            rc = cdb_bitmap_str2val("1-4094", GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("%s", err_desc);
            }
            _arpinsp_cmd_show_vlan_all(FALSE, FALSE, TRUE, bmp, para);    
        }
        else            
        {
            if (0 == sal_strcmp(argv[0], "interface"))      /*show ip arp inspection interface trust*/
            {
                if (argc == 1)
                {
                    _arpinsp_cmd_show_interface_trust(NULL, para);
                }
                else if (argc == 2)
                {
                    sal_memset(&if_key, 0, sizeof(if_key));
                    sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
                    p_db_if = tbl_interface_get_interface(&if_key);
                    if (NULL == p_db_if)
                    {
                        IFNAME_ETH2FULL(argv[1], ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                    } 
                    _arpinsp_cmd_show_interface_trust(p_db_if, para);
                }
            }
            else if (0 == sal_strcmp(argv[0], "number-limit"))      /*show ip arp number limit*/
            {
                if (argc == 1)
                {
                    _arpinsp_cmd_show_interface_numberlimit_all(para);
                }
                else if (argc == 2)
                {
                    sal_memset(&if_key, 0, sizeof(if_key));
                    sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
                    p_db_if = tbl_interface_get_interface(&if_key);
                    if (NULL == p_db_if)
                    {
                        IFNAME_ETH2FULL(argv[1], ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                    } 
                    if (GLB_IF_MODE_L3 != p_db_if->mode)
                    {
                        CFG_INVALID_PARAM_RET("Interface is not L3 port");
                    }
                    _arpinsp_cmd_show_interface_numberlimit_status(p_db_if, para);
                }
            }
            else if (0 == sal_strcmp(argv[0], "log"))       /*show ip arp inspection log*/
            {
                if (argc == 1)
                {
                    log_num = GLB_ARPINSP_LOGBUF_DEFAULT_NUM;
                }
                else if (argc == 2)
                {
                    PM_CFG_GET_UINT(log_num, argv[1], para->p_rs);
                    if (log_num < GLB_ARPINSP_LOGBUF_MIN_NUM || log_num > GLB_ARPINSP_LOGBUF_MAX_NUM)
                    {
                        CFG_INVALID_PARAM_RET("Invalid number");
                    }
                }
                arpinsp_cmd_show_log(log_num, para);
            }
            else if (0 == sal_strcmp(argv[0], "statistics"))    /*show ip arp inspection statistics*/
            {
                if (argc == 1)
                {
                    rc = cdb_bitmap_str2val("1-4094", GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("%s", err_desc);
                    }
                }
                else if (argc == 2)
                {
                    rc = cdb_bitmap_str2val(argv[1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("%s", err_desc);
                    }
                }
                _arpinsp_cmd_show_vlan_all(FALSE, TRUE, FALSE, bmp, para);
            }
            else if (0 == sal_strcmp(argv[0], "vlan"))       /*show ip arp inspection vlan*/
            {
                rc = cdb_bitmap_str2val(argv[1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, bmp, GLB_VLAN_BMP_WORD_MAX, err_desc);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("%s", err_desc);
                }
                _arpinsp_cmd_show_vlan_all(TRUE, FALSE, FALSE, bmp, para);
            }
            else if (0 == sal_strcmp(argv[0], "arpratelimit"))      /*show ip arp rate-limit [interface ]*/
            {
                if (argc == 1)
                {
                    _arpratelimit_cmd_show_interface(NULL, para);
                }
                else if (argc == 2)
                {
                    sal_strncpy(ifname, argv[1], IFNAME_SIZE);
                    p_db_if = tbl_interface_get_interface_by_name(ifname);
                    if (NULL == p_db_if)
                    {
                        IFNAME_ETH2FULL(argv[1], ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                    } 

                    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                    {
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                        CFG_CONFLICT_RET("Interface %s is lag member port ", ifname_ext);
                    }
                    
                    p_db_rtif = tbl_route_if_get_route_if_by_name(p_db_if->key.name);
                    if (NULL == p_db_rtif)
                    {
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                        CFG_CONFLICT_RET("Interface %s is not L3 port ", ifname_ext);
                    }
                    _arpratelimit_cmd_show_interface(p_db_if, para);
                }
            }
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }
    return rc;
}

int32
arpinsp_cmd_clear_logbuffer(cfg_cmd_para_t *para)
{
    tbl_route_global_t* p_rt_glb = tbl_route_global_get_route_global();
    S_ArpInspLogItem *pItem = NULL;
    while (!TAILQ_EMPTY(&g_arpinsp_logBuffer))
    {
        pItem = TAILQ_FIRST(&g_arpinsp_logBuffer);
        TAILQ_REMOVE(&g_arpinsp_logBuffer, pItem, entry);
        XFREE(MEM_ARPINSP_LOGBUF, pItem);
    }
    p_rt_glb->arpinsp_logbuf_curnum = 0;
    return PM_E_NONE;
}

int32
arpinsp_cmd_clear_statistics(cfg_cmd_para_t *para)
{
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 i = 0;

    for (i = GLB_VLAN_DEFAULT_VID; i <= GLB_VLAN_MAX; i++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
        arpinsp_key.vlan_id = i; 
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if (p_db_arpinsp->enable_state)
        {
            p_db_arpinsp->forwarded = 0;
            p_db_arpinsp->dropped = 0;
            p_db_arpinsp->dhcp_dropped = 0;
            p_db_arpinsp->acl_dropped = 0;
            p_db_arpinsp->dhcp_permit = 0;
            p_db_arpinsp->acl_permit = 0;
            p_db_arpinsp->srcmac_failure = 0;
            p_db_arpinsp->dstmac_failure = 0;
            p_db_arpinsp->ip_failure = 0;
            p_db_arpinsp->invalid_protocol = 0;
        }
    }
    return PM_E_NONE;
}

int32
arpinsp_cmd_process_clear(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (1 != argc)
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        if (0 == sal_strcmp(argv[0], "log-buffer")) 
        {
            arpinsp_cmd_clear_logbuffer(para);
        }
        else if (0 == sal_strcmp(argv[0], "statistics"))
        {
            arpinsp_cmd_clear_statistics(para);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return PM_E_NONE;
}

int32 
arpinsp_vlan_del_callback(vid_t vid)
{
    tbl_arpinsp_t* p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    int32 rc = PM_E_NONE;

    sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
    arpinsp_key.vlan_id = vid;
    p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
    if (p_db_arpinsp)
    {
        PM_E_RETURN(tbl_arpinsp_del_arpinsp(&arpinsp_key));
    }
    return rc;
}

int32
arpinsp_cmd_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_key_t  *p_vlan_key = NULL;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        p_vlan_key = (tbl_vlan_key_t*)p_tbl;
        arpinsp_vlan_del_callback(p_vlan_key->vid);
        break;

    case CDB_OPER_SET:        
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

