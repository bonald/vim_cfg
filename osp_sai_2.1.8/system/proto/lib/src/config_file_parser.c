
#include "proto.h"
#include "glb_tempfile_define.h"
#include "config_file_parser.h"

typedef struct _config_key2value_map
{
    int32   id;
    union {
        int32 i32_data;
    }u;
}config_key2value_map;

typedef struct _config_id2str_map
{
    int32   id;
    char    *keyStr;
}config_id2str_map;

static config_id2str_map keymap[]={
    {CONFIG_FEATURE_TAP_TIMESTAMP,"config_tap_timestamp"},
    {CONFIG_FEATURE_TAP_TRUNCTION,"config_tap_trunction"},
    {CONFIG_FEATURE_TAP_TRUNCTION_LENGTH_MAX,"config_tap_trunction_length_max"},
    {CONFIG_FEATURE_TAP_UNTAG,"config_tap_untag"},
    {CONFIG_FEATURE_HYBRID_FORCE_BACK,"config_hybrid_force_back"},
    {CONFIG_FEATURE_HYBRID_MPLS,"config_hybrid_mpls"},
    {CONFIG_FEATURE_HYBRID_TUNNEL,"config_hybrid_tunnel"},
    {CONFIG_FEATURE_HYBRID_ARP_EDIT,"config_hybrid_arp_edit"},
    {CONFIG_FEATURE_HYBRID_SET_QINQ,"config_hybrid_set_qinq"},
    {CONFIG_FEATURE_HYBRID_NEED_MPLS_PARSER_CLI, "config_hybrid_need_mpls_parser_cli"},
    {CONFIG_FEATURE_HYBRID_SOME_FIELD,"config_hybrid_some_field"},
    {CONFIG_FEATURE_ACL_VXLAN,"config_acl_vxlan"},
    {CONFIG_FEATURE_ACL_EDIT_SRC_IP,"config_acl_edit_src_ip"},
    {CONFIG_FEATURE_ACL_STRIP_HEADER,"config_acl_strip_header"},
    {CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH,"config_parser_linkagg_inner_hash"},
    {CONFIG_FEATURE_SELECT_HASH_KEY_NOT_ONLY_IP,"config_select_hash_key_not_only_ip"},
    {CONFIG_FEATURE_ACL_GRE,"config_acl_gre"},
    {CONFIG_FEATURE_ACL_NVGRE,"config_acl_nvgre"},
    {CONFIG_FEATURE_ACL_FLOW_MATCH,"config_acl_flow_match"},
    {CONFIG_FEATURE_ACL_UDF,"config_acl_udf"},
    {CONFIG_FEATURE_ACL_TCAM_RES_SHARE,"config_tap_acl_tcam_res_share"},
    {CONFIG_FEATURE_HYBRID_MPLS_TP_OAM, "config_hybrid_mpls_tp_oam"},
    {CONFIG_FEATURE_HYBRID_VLAN_EDIT_BEFORE_GROUP, "config_hybrid_vlan_edit_before_group"},
    {CONFIG_FEATURE_HYBRID_V4_V6_EXCHANGE, "config_hybrid_v4_v6_exchange"},
    {CONFIG_FEATURE_HYBRID_IPV6_EDIT, "config_hybrid_ipv6_edit"},
    {CONFIG_FEATURE_HYBRID_POP_ALL_MPLS, "config_hybrid_pop_all_mpls"},
    {-1,NULL}
};

config_key2value_map *feature_map = NULL;

void
__config_file_parser_line(int argc, char *argv[])
{
    int id_idx = 0;
    
    for(id_idx = 0; id_idx < sizeof(keymap)/sizeof(keymap[0]); id_idx++)
    {
        if(0 == strcmp(keymap[id_idx].keyStr,argv[0])){
            feature_map[keymap[id_idx].id].id = keymap[id_idx].id;
            feature_map[keymap[id_idx].id].u.i32_data = atoi(argv[1]);
            
            break;
        }
    }
}


void
__config_file_parser(sal_file_t fp)
{
    char    line[256];
    int     argc      = 0;
    char    *argv[12] = {};
    
    if(NULL == feature_map)
    {
        feature_map = malloc(sizeof(config_key2value_map) * CONFIG_FEATURE_MAX);
        if(NULL == feature_map)
        {
            return ;
        }

        memset(feature_map,0, sizeof(config_key2value_map) * CONFIG_FEATURE_MAX);
    }
    memset(line,0, sizeof(line));
    while (NULL != sal_fgets(line, 256, fp))
    {
        argc = 0;
        memset(argv, 0, sizeof(argv));
        argv[argc] = line;
        
        while(argv[argc] = sal_strtok(argv[argc],"=, \r\n"), argv[argc] != NULL) argc++;
        
        __config_file_parser_line(argc,argv);
        
        memset(line,0, sizeof(line));
    }
}

void 
config_file_parser()
{
    sal_file_t fp;

    fp = sal_fopen(GLB_CNOS_CONFIG_FILE, "r");
    if (fp == NULL)
    {
        return ;
    }
    
    __config_file_parser(fp);

    sal_fclose(fp);
}

uint32
check_feature_support(uint32 feature_id) 
{
    if(NULL == feature_map)
    {
        config_file_parser();
    }
    
    if(feature_map){
		if(CONFIG_FEATURE_NULL != feature_map[feature_id].id)
		{
        	return feature_map[feature_id].u.i32_data;
		}
    }
    
    return 0;
}

int32	
config_get_int32(uint32 feature_id, int32 num)
{
	if(NULL == feature_map)
    {
        config_file_parser();
    }
    
    if(feature_map){
		if(CONFIG_FEATURE_NULL != feature_map[feature_id].id)
		{
        	return feature_map[feature_id].u.i32_data;
		}
    }

	return num;
}
