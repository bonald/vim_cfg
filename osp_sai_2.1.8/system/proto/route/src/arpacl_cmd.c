#include "proto.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "route.h"
#include "cdb_const.h"
#include "cfg_cmd.h"
#include "arp_define.h"
#include "glb_l2_define.h"
#include "glb_arpacl_define.h"
#include "arpinsp.h"
#include "arpacl.h"


static int32
_arpacl_config_cmd_add_arpacl(char *arpacl_name, cfg_cmd_para_t *para)
{
    tbl_arpacl_config_t *p_arpacl_config = NULL;
    tbl_arpacl_config_key_t  arpacl_config_key;
    tbl_acl_config_t *p_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    tbl_arpacl_config_master_t* p_arp_master = tbl_arpacl_config_get_master();
    uint32 current_arpacl_count = p_arp_master->arpacl_config_hash->count;
    int32 rc = PM_E_NONE;

    /*1. check the arpacl name and the normal acl(ip acl, mac acl, ipv6 acl)*/
    if (check_name_character_and_len(arpacl_name, ARPACL_NAME_SIZE) != 0)
    {
        CFG_CONFLICT_RET("The initial character of ARP ACL name should be a-z, A-Z, or 0-9. The max ARP ACL name len is %d.", ARPACL_NAME_SIZE);
    }

    /*modified by yejl for bug 49965, 2018-11-13*/
    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
    sal_strcpy(acl_config_key.name, arpacl_name);
    p_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL != p_acl_config)
    {
        CFG_CONFLICT_RET("Can't configure the ARP ACL, it has configured by %s", glb_acl_type_str[p_acl_config->type_identifying]);
    }
    /*end*/
    
    /*2. duplicate check*/
    /*added by yejl to fix bug44162, 2017-05-25*/
    sal_memset(&arpacl_config_key, 0, sizeof(tbl_arpacl_config_key_t));
    sal_strncpy(arpacl_config_key.name, arpacl_name, ARPACL_NAME_SIZE);
    p_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_config_key);
    if (NULL != p_arpacl_config)
    {
        return PM_E_NONE;
    }
    
    /*3. check the arp acl number*/
    if (current_arpacl_count >= GLB_ACC_ARP_LIST_NUM_MAX)
    {
        CFG_CONFLICT_RET("The max ARP ACL number is %d", GLB_ACC_ARP_LIST_NUM_MAX);
    }
    
    /*4. call api*/
    rc = arpacl_config_add_arpacl(arpacl_name);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Add ARP ACL fail");
    }
    return PM_E_NONE;
}

static int32
_arpacl_config_cmd_del_arpacl(char *arpacl_name, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_arpacl_config_t *p_arpacl_config = NULL;
    tbl_arpacl_config_key_t  arpacl_config_key;
    bool is_used = FALSE;

    /*1. check the arpacl name*/
    if (check_name_character_and_len(arpacl_name, ARPACL_NAME_SIZE) != 0)
    {
        CFG_CONFLICT_RET("The initial character of ARP ACL name should be a-z, A-Z, or 0-9. The max ARP ACL name len is %d.", ARPACL_NAME_SIZE);
    }

    /*2. check exist*/
    sal_memset(&arpacl_config_key, 0, sizeof(tbl_arpacl_config_key_t));
    sal_strncpy(arpacl_config_key.name, arpacl_name, ARPACL_NAME_SIZE);
    p_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_config_key);
    if (NULL == p_arpacl_config)
    {
        CFG_CONFLICT_RET("ARP ACL %s does not exist", arpacl_name);
    }

    /*3. check the arp acl is used in the arp inspection*/
    is_used = arpacl_config_is_used(arpacl_name);
    if (is_used)
    {
        CFG_CONFLICT_RET("Can't delete ARP ACL: the list currently in use");
    }
    
    /*4. clear arp ace config*/
    rc = arpacl_config_clear_arpace(arpacl_name);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Delete ARP ACE fail");
    }
    
    /*5. call api*/
    rc = arpacl_config_del_arpacl(arpacl_name);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Delete ARP ACL fail");
    }
    return PM_E_NONE;
}

static int32 
_arpacl_cmd_read_arpacl_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_arpacl_config_iterate((TBL_ITER_CB_FUNC)tbl_arpacl_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static bool
_arpace_entry_config_check(tbl_arpace_config_t *pst_arpace_config)
{
    tbl_iter_args_t args;
    tbl_arpace_config_t *pst_same_ace = NULL;

    args.argv[0] = pst_arpace_config;
    args.argv[1] = pst_same_ace;
    tbl_arpace_config_iterate((TBL_ITER_CB_FUNC)arpacl_check_ace_config, &args);
    if (args.argv[1])
    {
        return TRUE;
    }
    return FALSE;
}

static int32
_arpace_get_arpace_seq_ref(char *acl_name, uint32 *seq_no, uint32 *new_seq_no)
{
    tbl_iter_args_t args;
    args.argv[0] = acl_name;
    args.argv[1] = seq_no;
    args.argv[2] = new_seq_no;

    tbl_arpace_config_iterate((TBL_ITER_CB_FUNC)arpace_get_new_seq_ref, &args);

    return PM_E_NONE;
}

static int32
_arpace_config_cmd_add_arpace(char *arpacl_name, uint32 p_seq_no, char **arpace_para, cfg_cmd_para_t *para)
{
    tbl_arpacl_config_t *pst_arpacl_config = NULL;
    tbl_arpacl_config_t arpacl_config;
    tbl_arpace_config_t *pst_arpace_config = NULL;
    tbl_arpace_config_t arpace_config;
    bool deby_enable     = FALSE;
    bool is_log          = FALSE;
    uint32 arp_type      = 0;
    char **key_type      = NULL;
    int rc               = PM_E_NONE;
    
    tbl_arpace_config_master_t* p_arpace_master = tbl_arpace_config_get_master();
    uint32 current_arpace_count = p_arpace_master->arpace_config_hash->count;
  
    sal_memset(&arpacl_config, 0, sizeof(tbl_arpacl_config_t));
    sal_strcpy(arpacl_config.key.name, arpacl_name);

    /*1. check the acp acl is exist*/
    pst_arpacl_config = tbl_arpacl_config_get_arpacl_config(&(arpacl_config.key));
    if (NULL == pst_arpacl_config)
    {
        CFG_CONFLICT_RET("ARP ACL %s does not exist", arpacl_name);
    }
    
    /*2. check arp ace is exist*/
    sal_memset(&arpace_config, 0, sizeof(tbl_arpace_config_t));
    if (0 == p_seq_no)
    {
        if ((pst_arpacl_config->seq_ref + 10) > GLB_ARPACL_MAX_ACE_SEQUENCE_NUMBER)
        {
            CFG_INVALID_PARAM_RET("Exceeded maximum sequence number");
        }
        else
        {
            arpace_config.key.seq_no = pst_arpacl_config->seq_ref + 10;
        }
        arpacl_config.seq_ref = arpace_config.key.seq_no;
    }
    else
    {
        arpace_config.key.seq_no = p_seq_no;
        if (p_seq_no > pst_arpacl_config->seq_ref)
        {
            arpacl_config.seq_ref = p_seq_no;
        }
        else
        {
            arpacl_config.seq_ref = pst_arpacl_config->seq_ref;
        }
    }
    sal_strcpy(arpace_config.key.arpacl_name, arpacl_name);
    pst_arpace_config = tbl_arpace_config_get_arpace_config(&(arpace_config.key));
    if (NULL != pst_arpace_config)
    {
        CFG_CONFLICT_RET("ARP ACE %s is exist", arpacl_name);
    }

    key_type = arpace_para;
    
    /*3. get the filter type: permit or deny*/
    if (0 == sal_strcmp(key_type[0], "deny"))
    {
        deby_enable = TRUE;
    }
    else if (0 == sal_strcmp(key_type[0], "permit"))
    {
        deby_enable = FALSE;
    }
    arpace_config.arp_deny = deby_enable;
    
    /*4. get arp type: request or response or all*/
    if (0 == sal_strcmp(key_type[1], "request"))
    {
        arp_type = GLB_ARP_REQUEST;
    }
    else if (0 == sal_strcmp(key_type[1], "response"))
    {
        arp_type = GLB_ARP_RESPONSE;
    }
    else if (0 == sal_strcmp(key_type[1], "all"))
    {
        arp_type = GLB_ARP_TYPE_ALL;
    }
    arpace_config.arp_type = arp_type;

    /*5. get the send ip type*/
    if (0 == sal_strcmp(key_type[3], "host"))
    {
        CFG_PARSER_IP_ADDR(key_type[4], &arpace_config.src_ip, para->p_rs);
        CFG_PARSER_IP_ADDR("0.0.0.0", &arpace_config.src_ip_mask, para->p_rs); 
    }
    else if (0 == sal_strcmp(key_type[3], "any"))
    {
        CFG_PARSER_IP_ADDR("255.255.255.255", &arpace_config.src_ip_mask, para->p_rs);
    }
    else if (0 == sal_strcmp(key_type[3], "send_ip"))
    {
        CFG_PARSER_IP_ADDR(key_type[4], &arpace_config.src_ip, para->p_rs);
        CFG_PARSER_IP_ADDR(key_type[5], &arpace_config.src_ip_mask, para->p_rs);
    }
    arpace_config.src_ip.s_addr = arpace_config.src_ip.s_addr & ~(arpace_config.src_ip_mask.s_addr);

    /*6. get the send mac type*/
    if (0 == sal_strcmp(key_type[7], "host"))
    {
        CFG_PARSER_MAC(arpace_config.src_mac, key_type[8], para->p_rs);
        CFG_PARSER_MAC(arpace_config.src_mac_mask, "0000.0000.0000", para->p_rs);
    }
    else if (0 == sal_strcmp(key_type[7], "any"))
    {
        CFG_PARSER_MAC(arpace_config.src_mac_mask, "ffff.ffff.ffff", para->p_rs);
    }
    else if (0 == sal_strcmp(key_type[7], "send_mac"))
    {
        CFG_PARSER_MAC(arpace_config.src_mac, key_type[8], para->p_rs);
        CFG_PARSER_MAC(arpace_config.src_mac_mask, key_type[9], para->p_rs);
    }
    arpace_config.src_mac[0] = arpace_config.src_mac[0] & (0xff - arpace_config.src_mac_mask[0]);
    arpace_config.src_mac[1] = arpace_config.src_mac[1] & (0xff - arpace_config.src_mac_mask[1]);
    arpace_config.src_mac[2] = arpace_config.src_mac[2] & (0xff - arpace_config.src_mac_mask[2]);
    arpace_config.src_mac[3] = arpace_config.src_mac[3] & (0xff - arpace_config.src_mac_mask[3]);
    arpace_config.src_mac[4] = arpace_config.src_mac[4] & (0xff - arpace_config.src_mac_mask[4]);
    arpace_config.src_mac[5] = arpace_config.src_mac[5] & (0xff - arpace_config.src_mac_mask[5]);
        
    /*7. log or not*/
    if (0 == sal_strcmp(key_type[10], "log"))
    {
        is_log = TRUE;
    }
    else if (0 == sal_strcmp(key_type[10], "all"))
    {
        is_log = FALSE;
    }
    arpace_config.arp_log = is_log;

    /*8. check the arp ace number*/
    if (current_arpace_count >= GLB_ACC_ARP_ACE_NUM_MAX)
    {
        CFG_CONFLICT_RET("The max ARP ACE number is %d", GLB_ACC_ARP_ACE_NUM_MAX);
    }
    
    /*9. check the same entry */
    if (_arpace_entry_config_check(&arpace_config))
    {
        CFG_INVALID_PARAM_RET("Same entry has already existed");
    }
    
    /*10. call API*/
    rc = arpace_config_add_arpace(&arpace_config);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Add ARP ACE fail");    
    }

    /*11. set the seq_ref to arpacl_config*/
    PM_E_RETURN(tbl_arpacl_config_set_arpacl_config_field(&arpacl_config, TBL_ARPACL_CONFIG_FLD_SEQ_REF));
    return PM_E_NONE; 
}

static int32
_arpace_config_cmd_del_arpace(char *arpacl_name, uint32 p_seq_no, cfg_cmd_para_t *para)
{
    tbl_arpacl_config_t *pst_arpacl_config = NULL;
    tbl_arpacl_config_t arpacl_config;
    tbl_arpace_config_t *pst_arpace_config = NULL;
    tbl_arpace_config_key_t arpace_config_key;
    uint32  new_seq_no   = 0;
    int     rc           = PM_E_NONE;

    /*1. check the acp acl is exist*/
    sal_memset(&arpacl_config, 0, sizeof(tbl_arpacl_config_t));
    sal_strcpy(arpacl_config.key.name, arpacl_name);
    pst_arpacl_config = tbl_arpacl_config_get_arpacl_config(&(arpacl_config.key));
    if (NULL == pst_arpacl_config)
    {
        CFG_CONFLICT_RET("ARP ACL %s does not exist", arpacl_name);
    }

    /*2. check arp ace is exist*/
    sal_memset(&arpace_config_key, 0, sizeof(tbl_arpace_config_key_t));
    sal_strcpy(arpace_config_key.arpacl_name, arpacl_name);
    arpace_config_key.seq_no = p_seq_no;
    pst_arpace_config = tbl_arpace_config_get_arpace_config(&arpace_config_key);
    if (NULL == pst_arpace_config)
    {
        CFG_CONFLICT_RET("ARP ACE %d does not exist", p_seq_no);
    }

    /*3. get the new arpacl_config.seq_ref*/
    if (p_seq_no == pst_arpacl_config->seq_ref)
    {
        _arpace_get_arpace_seq_ref(arpace_config_key.arpacl_name, &arpace_config_key.seq_no, &new_seq_no);
        arpacl_config.seq_ref = new_seq_no;
    }
    else
    {
        arpacl_config.seq_ref = pst_arpacl_config->seq_ref;
    }

    /*4. call API*/
    rc = arpace_config_del_arpace(arpacl_name, p_seq_no);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Del ARP ACE %d fail", p_seq_no);    
    }

    /*5. set the seq_ref to arpacl_config*/
    tbl_arpacl_config_set_arpacl_config_field(&arpacl_config, TBL_ARPACL_CONFIG_FLD_SEQ_REF);
    return PM_E_NONE;
}

int32
_arpace_cmd_read_arpace_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_arpace_config_iterate((TBL_ITER_CB_FUNC)tbl_arpace_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
arpacl_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *arpacl_name    = NULL;
    int rc               = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ARPACL_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    } 
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        arpacl_name = argv[0];
        rc = _arpacl_config_cmd_add_arpacl(arpacl_name, para);
        break;
    case CDB_OPER_SET:
        break;
        
    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        arpacl_name = argv[0];
        rc = _arpacl_config_cmd_del_arpacl(arpacl_name, para);
        break;
        
    case CDB_OPER_GET:
        _arpacl_cmd_read_arpacl_all(para);
        break;
        
     default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;      
    }
    return rc;
}


int32
arpace_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *arpacl_name    = NULL;
    uint32 seq_no        = 0;
    int rc               = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ARPACE_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    } 
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 13, para->p_rs);
        arpacl_name = argv[0];
        PM_CFG_GET_UINT(seq_no, argv[1], para->p_rs);
        rc = _arpace_config_cmd_add_arpace(arpacl_name, seq_no, argv+2, para);
        break;
    case CDB_OPER_SET:
        break;
        
    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        arpacl_name = argv[0];
        PM_CFG_GET_UINT(seq_no, argv[1], para->p_rs);
        rc = _arpace_config_cmd_del_arpace(arpacl_name, seq_no, para);
        break;
        
    case CDB_OPER_GET:
        rc = _arpace_cmd_read_arpace_all(para);
        break;
        
     default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;      
    }
    return rc;
}

int32
_arpace_show_arpace_one(tbl_arpace_config_t * p_db_arpace_config, FILE *fp)
{
    char buf1[256];
    char buf2[256];
    sal_memset(buf1, 0, sizeof(buf1));
    sal_memset(buf2, 0, sizeof(buf2));
    
    /*1.sequence number*/
    sal_fprintf(fp, " %d", p_db_arpace_config->key.seq_no);

    /*2.deny or permit*/
    sal_fprintf(fp, " %s", (p_db_arpace_config->arp_deny? "deny" : "permit"));

    /*3. request or response or all*/
    if (p_db_arpace_config->arp_type == GLB_ARP_REQUEST)
    {
        sal_fprintf(fp, " %s", "request");
    }
    else if (p_db_arpace_config->arp_type == GLB_ARP_RESPONSE)
    {
        sal_fprintf(fp, " %s", "response");
    }

    /*4. ip*/
    sal_fprintf(fp, " %s", "ip");
    cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, &p_db_arpace_config->src_ip);
    cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_db_arpace_config->src_ip_mask);
    if (0xffffffff == p_db_arpace_config->src_ip_mask.s_addr)
    {
        sal_fprintf (fp, " any");
    }
    else if (0 == p_db_arpace_config->src_ip_mask.s_addr)
    {
        sal_fprintf (fp, " host %s", buf1);
    }
    else
    {
        sal_fprintf (fp, " %s %s", buf1, buf2);
    }

    /*5. mac*/
    sal_fprintf(fp, " %s", "mac");
    cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_db_arpace_config->src_mac);
    cdb_mac_addr_val2str(buf2, CMD_BUF_256, p_db_arpace_config->src_mac_mask);
    if (0 == sal_strcmp("ffff.ffff.ffff", buf2))
    {
        sal_fprintf (fp, " any");
    }
    else if (0 == sal_strcmp("0000.0000.0000", buf2))
    {
        sal_fprintf (fp, " host %s", buf1);
    }
    else
    {
        sal_fprintf (fp, " %s %s", buf1, buf2);
    }

    /*6. log*/
    if (p_db_arpace_config->arp_log)
    {
        sal_fprintf (fp, " %s", "log");
    }
    return PM_E_NONE;
     
}

int32
_arpacl_show_arpacl_one(tbl_arpacl_config_t* p_db_arpacl_config, FILE *fp)
{
    tbl_arpace_config_master_t* p_master_arpace = tbl_arpace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_arpace_config_t * p_db_arpace_config = NULL;

    sal_fprintf (fp, "arp access-list %s\n", p_db_arpacl_config->key.name);

    CTCLIB_SLIST_LOOP(p_master_arpace->arpace_config_list, p_db_arpace_config, listnode)
    {
        if ((NULL != p_db_arpace_config) && (!sal_strcmp(p_db_arpacl_config->key.name, p_db_arpace_config->key.arpacl_name)))
        {
            _arpace_show_arpace_one(p_db_arpace_config, fp);
            sal_fprintf (fp, "\n");
        }
    }
    sal_fprintf (fp, "\n");    
    return PM_E_NONE;
}

int32
_arpacl_cmd_show_arpacl(tbl_arpacl_config_t* p_db_arpacl_config,  cfg_cmd_para_t *para)
{
    tbl_arpacl_config_master_t* p_master_arpacl = tbl_arpacl_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_db_arpacl_config)
    {
        _arpacl_show_arpacl_one(p_db_arpacl_config, fp);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master_arpacl->arpacl_config_list, p_db_arpacl_config, listnode)
        {
             _arpacl_show_arpacl_one(p_db_arpacl_config, fp);
        }
    }  
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
arpacl_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_arpacl_config_t *p_db_arpacl_config = NULL;
    tbl_arpacl_config_key_t arpacl_config_key;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 0)
        {
            _arpacl_cmd_show_arpacl(NULL, para);
        }
        else if (argc == 1)
        {
            sal_memset(&arpacl_config_key, 0, sizeof(tbl_arpacl_config_key_t));
            sal_strncpy(arpacl_config_key.name, argv[0], ARPACL_NAME_SIZE);
            p_db_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_config_key);
            if (NULL == p_db_arpacl_config)
            {
                CFG_INVALID_PARAM_RET("ARP ACL %s does not exist", argv[0]);
            }
            _arpacl_cmd_show_arpacl(p_db_arpacl_config, para);
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }
    return rc;
}

