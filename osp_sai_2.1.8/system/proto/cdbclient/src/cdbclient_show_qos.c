#include "proto.h"
#include "cdbclient.h"

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
#include "gen/tbl_cpu_traffic_define.h"
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "gen/tbl_aclqos_if_stats_define.h"
#include "gen/tbl_aclqos_if_stats.h"
#include "gen/tbl_cpu_traffic_group_define.h"
#include "gen/tbl_cpu_traffic_group.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "gen/tbl_class_map_action_config_define.h"
#include "gen/tbl_qos_mpls_domain_define.h"
#include "gen/tbl_qos_mpls_domain.h"

typedef enum 
{
    QOS_POLICER = 0,
    QOS_PORT_SHAPE,
    QOS_QUEUE_SHAPE,
    QOS_DROP
}qos_flag_t;

typedef struct
{
    char                 policy_name[128];
    char                 policer[POLICER_NAME_SIZE+1];
}cdbctl_class_map_action_t;

typedef struct
{
    char                 policy_map_name[QOS_NAME_SIZE];
    char                 class_map_name[QOS_NAME_SIZE];
}cdbctl_class_policy_t;

typedef struct
{
    char                 if_name[IFNAME_SIZE];
    char                 input_policer[POLICER_NAME_SIZE+1];
    char                 output_policer[POLICER_NAME_SIZE+1];
    char                 port_shape_profile[QOS_NAME_SIZE+1];
    char                 queue_shape_profile[256];
    char                 queue_drop_profile[256];
} cdbctl_aclqos_attr_entry_t;

typedef struct
{
    char                 if_name[IFNAME_SIZE];
    uint32              dir; //0=input,1=output,2=both
    uint32              queue[GLB_QOS_PORT_QUEUE_NUM];
    uint32              queue_count;
}cdbctl_aclqos_field_entry_t;

int32
cfg_split_token(char *str, const char *delimiter, uint32 max_token, char **token_array, uint32 *token_count);

static int32
_cdbclt_show_qos_global_one(char *line)
{
    cfg_parse_get_info_t info;
    int32 value = 0;
    int32 qos_ipg_en = 0;
    
    cfg_get_parse(line, &info, 0);        

    sal_printf( "\nQoS global state:\n");         

    value = sal_atoi(info.value[TBL_QOS_GLOBAL_FLD_QOS_ENABLE]);
    qos_ipg_en = sal_atoi(info.value[TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE]);
    sal_printf( "  QoS enable state             : %s\n", enable_str(value)); 
    sal_printf( "  QoS ipg state                : %s\n", enable_str(qos_ipg_en)); 
#if 0
    value = sal_atoi(info.value[TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE]);
    sal_printf( "  QoS port-policer first state : %s\n", enable_str(value));       
#endif
    return PM_E_NONE;
}
int32
cdbclt_show_qos_global(FILE *fp, cfg_parse_info_t *p_info)
{
    char line[FILE_READ_LINE_LEN];
                             
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_global_one(line);        
    }
    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_domain_one(char *line, char *type_str)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t phb_info;
    cfg_parse_get_info_t tc_info;
    cfg_parse_get_info_t pri_info;
    cfg_parse_get_info_t color_info;
    cfg_parse_get_info_t value_info;
    int32 i = 0;
    int32 j = 0;
    int32 phb_enable = 0;
    int32 cfi_enable = 0;
    char domain_id[10] = { 0 };

    char *
    phb2str[GLB_QOS_PHB_MAX] = 
    {
        "cs1", "df", "af11", "af12", "af13", "af21","af22", "af23", "af31", "af32", "af33", 
        "af41", "af42", "af43", "cs2", "cs3", "cs6", "cs7", "cs4", "cs5", "ef" 
    };    
    char *
    color2str[GLB_QOS_COLOR_MAX] = 
    {
        "none", "red", "yellow", "green"
    };        

    cfg_get_parse(line, &info, 0);        
    
    phb_enable = sal_atoi(info.value[TBL_QOS_DOMAIN_FLD_PHB_ENABLE]);
    cfi_enable = sal_atoi(info.value[TBL_QOS_DOMAIN_FLD_CFI_ENABLE]);
    if (GLB_QOS_DOMAIN_NUM == sal_atoi(info.value[TBL_QOS_DOMAIN_FLD_KEY]))
    {
        sal_strcpy(domain_id, "default");
    }
    else
    {
        sal_strcpy(domain_id, info.value[TBL_QOS_DOMAIN_FLD_KEY]);
    }

    if (phb_enable)
    {
        /* COS ==> PHB */
        if (0 == sal_strcmp(type_str, "cos-phb"))
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_COS2PHB], &phb_info);
            sal_printf("\n");
            sal_printf("QoS DOMAIN %s, CFI %s, COS map to PHB:\n", domain_id, enable_str(cfi_enable));
            sal_printf("--------------------------------------------------------------\n");
            sal_printf("  COS     : 0      1      2      3      4      5      6      7\n");
            sal_printf("  PHB     : ");
            for (i = 0; i < GLB_QOS_COS_NUM; i++)
            {
                sal_printf("%-7s", phb2str[sal_atoi(phb_info.argv[i])]);    
            }
            sal_printf("\n");
        }
        /* DSCP ==> PHB */
        else if (0 == sal_strcmp(type_str, "dscp-phb"))        
        {
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, DSCP map to PHB:\n", domain_id);
        
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_DSCP2PHB], &phb_info);
            for (j = 0; j < 8; j++)
            {
                sal_printf("--------------------------------------------------------------\n");  
                sal_printf("  DSCP    : ");
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7d", i);
                }
                sal_printf("\n");
                sal_printf("  PHB     : "); 
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7s", phb2str[sal_atoi(phb_info.argv[i])]);
                }
                sal_printf("\n");
            }   
            sal_printf("\n");    
        }    
        /* PREC ==> PHB */
        else if (0 == sal_strcmp(type_str, "ip-prec-phb"))        
        {
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, IP PRECEDENCE map to PHB:\n", domain_id);
            sal_printf("--------------------------------------------------------------\n");  
            sal_printf("  IP-prec : 0      1      2      3      4      5      6      7\n");
            sal_printf("  PHB     : ");
        
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PREC2PHB], &phb_info);
            for (i = 0; i < 8; i++)
            {
                sal_printf("%-7s", phb2str[sal_atoi(phb_info.argv[i])]);
            }   
            sal_printf("\n");    
        }  
        /* EXP ==> PHB */
        else if (0 == sal_strcmp(type_str, "exp-phb"))        
        {
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, IP PRECEDENCE map to PHB:\n", domain_id);
            sal_printf("--------------------------------------------------------------\n");  
            sal_printf("  IP-prec : 0      1      2      3      4      5      6      7\n");
            sal_printf("  PHB     : ");
        
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_EXP2PHB], &phb_info);
            for (i = 0; i < 8; i++)
            {
                sal_printf("%-7s", phb2str[sal_atoi(phb_info.argv[i])]);
            }   
            sal_printf("\n");    
        }        
        /* PHB ==> COS */
        else if (0 == sal_strcmp(type_str, "phb-cos"))        
        {
            sal_printf("\n"); 
            sal_printf(" QoS DOMAIN %s, CFI %s, PHB map to CoS:\n", domain_id, enable_str(cfi_enable));
            sal_printf(" %-8s |", "PHB Name");  
            sal_printf(" %-6s", "CoS");
            sal_printf("\n");
            sal_printf("-------------------------------------------\n");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PHB2COS], &value_info);
            for (i = GLB_QOS_PHB_CS1; i < GLB_QOS_PHB_MAX; i++)
            {
                sal_printf(" %-8s |", phb2str[i]);
                sal_printf(" %-6s\n", value_info.argv[i]);
            }   
            sal_printf("\n");    
        }      
        /* PHB ==> DSCP */
        else if (0 == sal_strcmp(type_str, "phb-dscp"))        
        {
            sal_printf("\n"); 
            sal_printf(" QoS DOMAIN %s, PHB map to DSCP:\n", domain_id);
            sal_printf(" %-8s |", "PHB Name");  
            sal_printf(" %-6s", "DSCP");
            sal_printf("\n");
            sal_printf("-------------------------------------------\n");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PHB2DSCP], &value_info);
            for (i = GLB_QOS_PHB_CS1; i < GLB_QOS_PHB_MAX; i++)
            {
                sal_printf(" %-8s |", phb2str[i]);
                sal_printf(" %-6s\n", value_info.argv[i]);
            }   
            sal_printf("\n");    
        }     
        /* PHB ==> EXP */
        if (0 == sal_strcmp(type_str, "phb-exp"))        
        {
            sal_printf("\n"); 
            sal_printf(" QoS DOMAIN %s, PHB map to EXP:\n", domain_id);
            sal_printf(" %-8s |", "PHB Name");  
            sal_printf(" %-6s", "EXP");
            sal_printf("\n");
            sal_printf("-------------------------------------------\n");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PHB2EXP], &value_info);
            for (i = GLB_QOS_PHB_CS1; i < GLB_QOS_PHB_MAX; i++)
            {
                sal_printf(" %-8s |", phb2str[i]);
                sal_printf(" %-6s\n", value_info.argv[i]);
            }   
            sal_printf("\n");    
        }        
    }
    else
    {     
         /* COS ==> PRIORITY & COLOR */
        if (0 == sal_strcmp(type_str, "cos-pri-color"))  
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_COS2PRI], &pri_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_COS2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, CFI %s, COS map to PRIORITY & COLOR:\n", domain_id, enable_str(cfi_enable));
            sal_printf("--------------------------------------------------------------\n");
            sal_printf("  COS     : 0      1      2      3      4      5      6      7\n");
            sal_printf("  priority: ");
            for (i = 0; i < GLB_QOS_COS_NUM; i++)
            {
                sal_printf("%-7s", pri_info.argv[i]);    
            }
            sal_printf("\n");        
            sal_printf("  color   : "); 
            for (i = 0; i < GLB_QOS_COS_NUM; i++)
            {
                sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);    
            }
            sal_printf("\n");
        }             
        else if (0 == sal_strcmp(type_str, "dscp-pri-color"))   /* DSCP ==> PRIORITY & COLOR */
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_DSCP2PRI], &pri_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_DSCP2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, DSCP map to PRIORITY & COLOR:\n", domain_id);
//        sal_printf("  priority: ");
            for (j = 0; j < 8; j++)
            {
                sal_printf("--------------------------------------------------------------\n");  
                sal_printf("  DSCP    : ");
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7d", i);
                }
                sal_printf("\n");
                sal_printf("  priority: "); 
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7s", pri_info.argv[i]);
                }
                sal_printf("\n");
                sal_printf("  color   : ");
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);
                }
                sal_printf("\n");
            }
            sal_printf("\n");
        }
        else if (0 == sal_strcmp(type_str, "ip-prec-pri-color"))  /* PREC ==> PRIORITY & COLOR */
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PREC2PRI], &pri_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PREC2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, IP PRECEDENCE map to PRIORITY & COLOR:\n", domain_id);
            sal_printf("--------------------------------------------------------------\n");
            sal_printf("  IP-prec : 0      1      2      3      4      5      6      7\n");
            sal_printf("  priority: ");
            for (i = 0; i < GLB_QOS_IP_PREC_NUM; i++)
            {
                sal_printf("%-7s", pri_info.argv[i]);    
            }
            sal_printf("\n");        
            sal_printf("  color   : "); 
            for (i = 0; i < GLB_QOS_IP_PREC_NUM; i++)
            {
                sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);    
            }
            sal_printf("\n");
        }
        else  if (0 == sal_strcmp(type_str, "exp-pri-color"))  /* EXP ==> PRIORITY & COLOR */
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_EXP2PRI], &pri_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_EXP2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, EXP map to PRIORITY & COLOR:\n", domain_id);
            sal_printf("--------------------------------------------------------------\n");
            sal_printf("  EXP     : 0      1      2      3      4      5      6      7\n");
            sal_printf("  priority: ");
            for (i = 0; i < GLB_QOS_EXP_NUM; i++)
            {
                sal_printf("%-7s", pri_info.argv[i]);    
            }
            sal_printf("\n");        
            sal_printf("  color   : "); 
            for (i = 0; i < GLB_QOS_EXP_NUM; i++)
            {
                sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);    
            }
            sal_printf("\n");
        }
        else if (0 == sal_strcmp(type_str, "pri-color-cos"))   /* PRIORITY & COLOR ==> COS */
        {
            sal_printf("\n"); 
            sal_printf(" QoS Domain %s, CFI %s, PRIORITY & COLOR map to CoS:\n", info.value[TBL_QOS_DOMAIN_FLD_KEY], enable_str(cfi_enable));
            sal_printf("              | COLOR:\n");  
            sal_printf("              | red     yellow  green  \n");        
            sal_printf("-------------------------------------------\n");
            sal_printf(" PRIORITY: ");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS], &value_info);
            for (j = 0; j < GLB_QOS_PRIORITY_NUM; j++)
            {
                sal_printf("%-2d |", j);
                for (i = 1; i < GLB_QOS_COLOR_NUM; i++)
                {
                    sal_printf(" %-7s", value_info.argv[(j<<2) + i]);
                }
                if (63 == j)
                {
                    sal_printf("    COS value\n");
                }
                else
                {
                    sal_printf("\n");
                    sal_printf("           ");
                }
            }   
            sal_printf("\n");    
        }
        else if (0 == sal_strcmp(type_str, "pri-color-dscp"))   /* PRIORITY & COLOR ==> DSCP */
        {
            sal_printf("\n"); 
            sal_printf(" QoS Domain %s, PRIORITY & COLOR map to DSCP:\n", info.value[TBL_QOS_DOMAIN_FLD_KEY]);
            sal_printf("              | COLOR:\n");  
            sal_printf("              | red     yellow  green  \n");        
            sal_printf("-------------------------------------------\n");
            sal_printf(" PRIORITY: ");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP], &value_info);
            for (j = 0; j < GLB_QOS_PRIORITY_NUM; j++)
            {
                sal_printf("%-2d |", j);
                for (i = 1; i < GLB_QOS_COLOR_NUM; i++)
                {
                    sal_printf(" %-7s", value_info.argv[(j<<2) + i]);
                }
                if (63 == j)
                {
                    sal_printf("    DSCP value\n");
                }
                else
                {
                    sal_printf("\n");
                    sal_printf("           ");
                }
            }   
            sal_printf("\n");    
        }       
        else if (0 == sal_strcmp(type_str, "pri-color-exp"))   /* PRIORITY & COLOR ==> EXP */
        {
            sal_printf("\n"); 
            sal_printf(" QoS Domain %s, PRIORITY & COLOR map to EXP:\n", info.value[TBL_QOS_DOMAIN_FLD_KEY]);
            sal_printf("              | COLOR:\n");  
            sal_printf("              | red     yellow  green  \n");        
            sal_printf("-------------------------------------------\n");
            sal_printf(" PRIORITY: ");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP], &value_info);
            for (j = 0; j < GLB_QOS_PRIORITY_NUM; j++)
            {
                sal_printf("%-2d |", j);
                for (i = 1; i < GLB_QOS_COLOR_NUM; i++)
                {
                    sal_printf(" %-7s", value_info.argv[(j<<2) + i]);
                }
                if (63 == j)
                {
                    sal_printf("    EXP value\n");
                }
                else
                {
                    sal_printf("\n");
                    sal_printf("           ");
                }
            }   
            sal_printf("\n");    
        }  
        else if(0 == sal_strcmp(type_str, "cos-tc-color"))
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_COS2TC], &tc_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_COS2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, CFI %s, COS map to TC & COLOR:\n", domain_id, enable_str(cfi_enable));
            sal_printf("-----------+------+------+------+------+------+------+------+------\n");
            sal_printf("  COS     : ");
            for (i = 0;i < GLB_QOS_COS_NUM; i++)
            {
                sal_printf("%-7d", i);   
            }
            sal_printf("\n");
            
            sal_printf("  TC      : ");
            for (i = 0; i < GLB_QOS_COS_NUM; i++)
            {
                sal_printf("%-7s", tc_info.argv[i]);    
            }
            sal_printf("\n");        
            sal_printf("  color   : "); 
            for (i = 0; i < GLB_QOS_COS_NUM; i++)
            {
                sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);    
            }
            sal_printf("\n");
        }
       else if(0 == sal_strcmp(type_str, "exp-tc-color"))
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_EXP2TC], &tc_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_EXP2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, EXP map to TC & COLOR:\n", domain_id);
            sal_printf("-----------+------+------+------+------+------+------+------+------\n");
            sal_printf("  EXP     : ");
            for (i = 0;i < GLB_QOS_EXP_NUM; i++)
            {
                sal_printf("%-7d", i);   
            }
            sal_printf("\n");
            sal_printf("  TC      : ");
            for (i = 0; i < GLB_QOS_EXP_NUM; i++)
            {
                sal_printf("%-7s", tc_info.argv[i]);    
            }
            sal_printf("\n");        
            sal_printf("  color   : "); 
            for (i = 0; i < GLB_QOS_EXP_NUM; i++)
            {
                sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);    
            }
            sal_printf("\n");
        }
        else if (0 == sal_strcmp(type_str, "dscp-tc-color"))   /* DSCP ==> PRIORITY & COLOR */
        {
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_DSCP2TC], &tc_info);
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_DSCP2COLOR], &color_info);
            sal_printf("\n"); 
            sal_printf("QoS DOMAIN %s, DSCP map to TC & COLOR:\n", domain_id);
//        sal_printf("  priority: ");
            for (j = 0; j < 8; j++)
            {
                sal_printf("-----------+------+------+------+------+------+------+------+------\n");
                sal_printf("  DSCP    : ");
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7d", i);
                }
                sal_printf("\n");
                sal_printf("  TC      : ");
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7s", tc_info.argv[i]);
                }
                sal_printf("\n");
                sal_printf("  color   : "); 
                for (i = j*8; i < (j + 1)*8; i ++)
                {
                    sal_printf("%-7s", color2str[sal_atoi(color_info.argv[i])]);
                }
                sal_printf("\n");
            }
            sal_printf("\n");
        }
        else if (0 == sal_strcmp(type_str, "tc-color-cos"))   /* PRIORITY & COLOR ==> COS */
        {
            sal_printf("\n"); 
            sal_printf(" QoS Domain %s, CFI %s, TC & COLOR map to CoS:\n", info.value[TBL_QOS_DOMAIN_FLD_KEY], enable_str(cfi_enable));
            sal_printf("              | COLOR:\n");  
            sal_printf("              | red     yellow  green  \n");        
            sal_printf("-------------------------------------------\n");
            sal_printf(" TC      : ");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_TC_COLOR2COS], &value_info);
            for (j = 0; j < GLB_QOS_TC_NUM; j++)
            {
                sal_printf("%-2d |", j);
                for (i = 1; i < GLB_QOS_COLOR_NUM; i++)
                {
                    sal_printf(" %-7s", value_info.argv[(j<<2) + i]);
                }
                if (63 == j)
                {
                    sal_printf("    COS value\n");
                }
                else
                {
                    sal_printf("\n");
                    sal_printf("           ");
                }
            }   
            sal_printf("\n");    
        }
        else if (0 == sal_strcmp(type_str, "tc-color-exp"))
        {
            sal_printf("\n"); 
            sal_printf(" QoS Domain %s, TC & COLOR map to EXP:\n", info.value[TBL_QOS_DOMAIN_FLD_KEY]);
            sal_printf("              | COLOR:\n");  
            sal_printf("              | red     yellow  green  \n");        
            sal_printf("-------------------------------------------\n");
            sal_printf(" TC      : ");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP], &value_info);
            for (j = 0; j < GLB_QOS_TC_NUM; j++)
            {
                sal_printf("%-2d |", j);
                for (i = 1; i < GLB_QOS_COLOR_NUM; i++)
                {
                    sal_printf(" %-7s", value_info.argv[(j<<2) + i]);
                }
                if (63 == j)
                {
                    sal_printf("    COS value\n");
                }
                else
                {
                    sal_printf("\n");
                    sal_printf("           ");
                }
            }   
            sal_printf("\n");    
        }
        else if (0 == sal_strcmp(type_str, "tc-color-dscp"))   /* PRIORITY & COLOR ==> DSCP */
        {
            sal_printf("\n"); 
            sal_printf(" QoS Domain %s, TC & COLOR map to DSCP:\n", info.value[TBL_QOS_DOMAIN_FLD_KEY]);
            sal_printf("              | COLOR:\n");  
            sal_printf("              | red     yellow  green  \n");        
            sal_printf("-------------------------------------------\n");
            sal_printf(" TC      : ");
            cfg_get_parse_comma(info.value[TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP], &value_info);
            for (j = 0; j < GLB_QOS_TC_NUM; j++)
            {
                sal_printf("%-2d |", j);
                for (i = 1; i < GLB_QOS_COLOR_NUM; i++)
                {
                    sal_printf(" %-7s", value_info.argv[(j<<2) + i]);
                }
                if (63 == j)
                {
                    sal_printf("    DSCP value\n");
                }
                else
                {
                    sal_printf("\n");
                    sal_printf("           ");
                }
            }   
            sal_printf("\n");    
        }       
    }
    
    sal_printf("\n");

    return PM_E_NONE;
}


int32
cdbclt_show_qos_domain(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    cfg_parse_info_t parse_info;    
    char  line[FILE_READ_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    rc = cfg_cmd_parse_para(para, &parse_info);
    if (rc < 0)
    {
        return rc;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_domain_one(line, parse_info.argv[2]);
    }

    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_mpls_domain_one(char * line)
{
    cfg_parse_get_info_t info;
    
    cfg_get_parse(line, &info, 0);

    sal_printf("%-15s|         %s\n", info.value[TBL_QOS_MPLS_DOMAIN_FLD_KEY], info.value[TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN]);
    sal_printf("-------------------------------\n");
    return PM_E_NONE;
}

int32
cdbclt_show_qos_mpls_domain(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    cfg_parse_info_t parse_info;    
    char  line[FILE_READ_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    rc = cfg_cmd_parse_para(para, &parse_info);
    if (rc < 0)
    {
        return rc;
    }

    sal_printf("mpls-label     |     exp-domain\n");
    sal_printf("-------------------------------\n");
    sal_printf("default        |         0\n");
    sal_printf("-------------------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_mpls_domain_one(line);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_get_class_map_action(cdbctl_class_map_action_t *array, uint32 *array_size)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    FILE *fp = NULL;
    char path[MAX_CMD_SHOW_FNAME_LEN];
    uint32 i = 0;
    
    cdbclt_get_tbl(TBL_CLASS_MAP_ACTION_CONFIG, NULL, 0, path);

    fp = sal_fopen(path, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        sal_strcpy(array[i].policy_name, info.value[TBL_CLASS_MAP_ACTION_CONFIG_FLD_KEY]);
        sal_strcpy(array[i].policer, info.value[TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER]);
        i++;
    }

    *array_size = i;
    sal_fclose(fp);
    sal_unlink(path);

    return PM_E_NONE;
}

extern char*
cfg_bra_strchr(char *s, char c);

static int32
_cdbctl_get_class_action_name(char *profile_name, cdbctl_class_map_action_t *c_array, uint32 c_array_size, cdbctl_class_policy_t *policy_class, uint32 *p_size)
{
    uint32 i = 0;
    uint32 j = 0;
    char *pos = NULL;
    for (i = 0; i < c_array_size; i++)
    {
        if (!sal_strcmp(c_array[i].policer, profile_name) && c_array[i].policer)
        {
            pos = c_array[i].policy_name;
            pos = cfg_bra_strchr(c_array[i].policy_name, '#');
            *pos = 0;
            sal_strcpy(policy_class[j].policy_map_name, c_array[i].policy_name);
            pos++;
            sal_strcpy(policy_class[j].class_map_name, pos);
            j++;
        }
    }
    *p_size = j;
    
    return PM_E_NONE;
}

static int32
_cdbctl_get_aclqos_if_field(uint32 flag, char *profile_name, cdbctl_aclqos_attr_entry_t *array, uint32 array_size, cdbctl_aclqos_field_entry_t *array2, uint32 *array_size2)
{
    uint32 i = 0;
    uint32 j = 0;
    uint32 k = 0;
    uint32 queue_num = 0;
    char *queue[GLB_QOS_PORT_QUEUE_NUM];
    uint32 count = 0;
    uint32 has_config = 0;
    
    for (i = 0; i < array_size; i++)
    {
        has_config = 0;
        queue_num = 0;
        if (QOS_POLICER == flag)//policer
        {
            if ((!sal_strcmp(array[i].input_policer, profile_name) ) && array[i].input_policer)
            {
                sal_strcpy(array2[j].if_name, array[i].if_name);
                array2[j].dir = 0;
                has_config  = 1;
            }
            if ((!sal_strcmp(array[i].output_policer, profile_name)) && array[i].output_policer)
            {
                sal_strcpy(array2[j].if_name, array[i].if_name);
                array2[j].dir = 1;
                has_config = 1;
            }
            if ((!sal_strcmp(array[i].input_policer, profile_name) ) && (!sal_strcmp(array[i].output_policer, profile_name)) && array[i].port_shape_profile && array[i].output_policer)
            {
                sal_strcpy(array2[j].if_name, array[i].if_name);
                array2[j].dir = 2;
                has_config = 1;
            }
            if (has_config)
            {
                j++;
            }
        }
        else if (QOS_PORT_SHAPE == flag)//port-shape
        {
            if (!sal_strcmp(array[i].port_shape_profile, profile_name) && array[i].port_shape_profile)
            {
                sal_strcpy(array2[j].if_name, array[i].if_name);
                j++;
            }
        }
        else if (QOS_QUEUE_SHAPE == flag)//queue-shape
        {
            cfg_split_token(array[i].queue_shape_profile, ",", GLB_QOS_PORT_QUEUE_NUM, queue, &count);
            for (k = 0; k< count; k++)
            {            
                if (!sal_strcmp(queue[k], profile_name))
                {
                    array2[j].queue[queue_num] = k;
                    queue_num ++;
                }
            }
            array2[j].queue_count = queue_num;
            if (queue_num > 0)
            {
                sal_strcpy(array2[j].if_name, array[i].if_name);
                j++;
            }

        }
        else if (QOS_DROP == flag)//drop
        {
            cfg_split_token(array[i].queue_drop_profile, ",", GLB_QOS_PORT_QUEUE_NUM, queue, &count);
            for (k = 0; k< count; k++)
            {            
                if (!sal_strcmp(queue[k], profile_name))
                {
                    array2[j].queue[queue_num] = k;
                    queue_num ++;
                }
            }
            array2[j].queue_count = queue_num;
            if (queue_num > 0)
            {
                sal_strcpy(array2[j].if_name, array[i].if_name);
                j++;
            }
        }
    }
    *array_size2  = j;

    return PM_E_NONE;
}

int32
cdbctl_show_get_aclqos_attr(cdbctl_aclqos_attr_entry_t *array, uint32 *array_size)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    FILE *fp = NULL;
    char path[MAX_CMD_SHOW_FNAME_LEN];
    uint32 i = 0;
    int32 rc = PM_E_NONE;
    
    rc = cdbclt_get_tbl(TBL_ACLQOS_IF, NULL, 0, path);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
        
    fp = sal_fopen(path, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        sal_strcpy(array[i].if_name, info.value[TBL_ACLQOS_IF_FLD_KEY]);
        sal_strcpy(array[i].input_policer, info.value[TBL_ACLQOS_IF_FLD_INPUT_POLICER]);
        sal_strcpy(array[i].output_policer, info.value[TBL_ACLQOS_IF_FLD_OUTPUT_POLICER]);
        sal_strcpy(array[i].port_shape_profile, info.value[TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE]);
        sal_strcpy(array[i].queue_shape_profile, info.value[TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE]);
        sal_strcpy(array[i].queue_drop_profile, info.value[TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE]);
        i++;
    }

    *array_size = i;
    sal_fclose(fp);
    sal_unlink(path);

    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_policer_action_profile_one(char *line)
{
    cfg_parse_get_info_t info;
    glb_qos_color_action_t confirm_action;
    glb_qos_color_action_t exceed_action;
    glb_qos_color_action_t violate_action;
    char tc_value[4] = {0}; 

    sal_memset(&confirm_action, 0, sizeof(glb_qos_color_action_t));
    sal_memset(&exceed_action, 0, sizeof(glb_qos_color_action_t));
    sal_memset(&violate_action, 0, sizeof(glb_qos_color_action_t));
    
    cfg_get_parse(line, &info, 0);
    if (!info.value[TBL_QOS_POLICER_ACTION_PROFILE_FLD_CONFIRM_ACTION])
    {
        sal_printf("  policer-action-profile-name: %s\n",info.value[TBL_QOS_POLICER_ACTION_PROFILE_FLD_KEY]);
    }
    else
    {
        cdb_qos_color_action_str2val(info.value[TBL_QOS_POLICER_ACTION_PROFILE_FLD_CONFIRM_ACTION], &confirm_action);
        cdb_qos_color_action_str2val(info.value[TBL_QOS_POLICER_ACTION_PROFILE_FLD_EXCEED_ACTION], &exceed_action);
        cdb_qos_color_action_str2val(info.value[TBL_QOS_POLICER_ACTION_PROFILE_FLD_VIOLATE_ACTION], &violate_action);
        sal_printf("  POLICER-ACTION-PROFILE-NAME: %s\n",info.value[TBL_QOS_POLICER_ACTION_PROFILE_FLD_NAME]);
        sal_sprintf(tc_value, "%d", confirm_action.tc);
        sal_printf("   confirm-action %s tc %s\n", confirm_action.drop_packet?"drop":"transmit", 
                                                                confirm_action.tc_valid?tc_value:"invalid");
        sal_sprintf(tc_value, "%d", exceed_action.tc);
        sal_printf("   exceed-action %s tc %s\n", exceed_action.drop_packet?"drop":"transmit", 
                                                                exceed_action.tc_valid?tc_value:"invalid");
        sal_sprintf(tc_value, "%d", violate_action.tc);
        sal_printf("   violate-action %s tc %s\n", violate_action.drop_packet?"drop":"transmit", 
                                                                violate_action.tc_valid?tc_value:"invalid");
    }
    return PM_E_NONE;
}

int32
cdbclt_show_qos_policer_action_profile(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    cfg_parse_info_t parse_info;
    char  line[FILE_READ_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    rc = cfg_cmd_parse_para(para, &parse_info);
    if (rc < 0)
    {
        return rc;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_policer_action_profile_one(line);
    }

    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_policer_profile_one(char *line, bool print_stats, cdbctl_aclqos_attr_entry_t *if_array, uint32 array_size, cdbctl_class_map_action_t *c_array, uint32 c_array_size)
{
    cfg_parse_get_info_t info;
    int32 value = 0;
    char ifname_full[IFNAME_EXT_SIZE];
    bool use_pir_pbs = FALSE;
    bool is_rfc2697 = FALSE;
    char* mode_str[] = 
    {
        "rfc2697",
        "rfc2698",
        "rfc4115",
        "bwp"
    };
    char* type_str[] = 
    {
        "POLICER-PROFILE-NAME",
        "AGGREGATOR-POLICER-NAME"        
    };    
    char* color_mode_str[] = 
    {
        "color blind mode",
        "color aware mode"
    };   
    char* color_str[] = 
    {
        "none",
        "red",
        "yellow"
    };    
    cdbctl_aclqos_field_entry_t if_array2[ GLB_PORT_NUM_PER_CHIP];
    cdbctl_class_policy_t   policy_class_array[128];
    uint32 array2_size = 0;  
    uint32 p_size = 0;
    uint32 i = 0;
    uint32 is_pps = 0;
    
    cfg_get_parse(line, &info, 0);
    if (!info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE])
    {
        sal_printf("  policer-profile-name: %s\n",info.value[TBL_QOS_POLICER_PROFILE_FLD_KEY]);
    }
    else
    {
        _cdbctl_get_aclqos_if_field(QOS_POLICER, info.value[TBL_QOS_POLICER_PROFILE_FLD_NAME], if_array, array_size, if_array2, &array2_size);
        _cdbctl_get_class_action_name(info.value[TBL_QOS_POLICER_PROFILE_FLD_NAME], c_array, c_array_size,  policy_class_array, &p_size);
        
        value = sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP]);
        sal_printf(" %s: %s\n", type_str[value], info.value[TBL_QOS_POLICER_PROFILE_FLD_NAME]); 

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE]))
        {
            value = sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE]);
            sal_printf("  mode %s", mode_str[value]); 
            if (0 == sal_strcmp(mode_str[value], "rfc2698"))
            {
                use_pir_pbs = TRUE;
            }

            is_rfc2697 = ( 0 == value ) ? TRUE : FALSE;
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_IS_PPS]))
        {
            is_pps = 1;
            sal_printf(", pps mode");
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE]))
        {
            value = sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE]);
            sal_printf(", %s", color_mode_str[value]); 
        }
        
        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR]))
        {
            if (is_pps)
            {
                sal_printf(", CIR %s pps", info.value[TBL_QOS_POLICER_PROFILE_FLD_CIR]); 
            }
            else
            {
                sal_printf(", CIR %s Kbps", info.value[TBL_QOS_POLICER_PROFILE_FLD_CIR]); 
            }  
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS]))
        {
            if (is_pps)
            {
                sal_printf(", CBS %s packets", info.value[TBL_QOS_POLICER_PROFILE_FLD_CBS]); 
            }
            else
            {
                sal_printf(", CBS %s Bytes", info.value[TBL_QOS_POLICER_PROFILE_FLD_CBS]); 
            }
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR]))
        {
            /* BEGIN: fix bug 39336 add by wangqj 2016.8.1 */
            if(is_rfc2697 == FALSE)
            {
                if (is_pps)
                {
                    sal_printf(", %s %s pps", use_pir_pbs ? "PIR" : "EIR", info.value[TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR]); 
                }
                else
                {
                    sal_printf(", %s %s Kbps", use_pir_pbs ? "PIR" : "EIR", info.value[TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR]); 
                }
            }
            /* END: fix bug 39336 add by wangqj 2016.8.1 */
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS]))
        {
            if (is_pps)
            {
                sal_printf(", %s %s packets", use_pir_pbs ? "PBS" : "EBS", info.value[TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS]); 
            }
            else
            {
                sal_printf(", %s %s Bytes", use_pir_pbs ? "PBS" : "EBS", info.value[TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS]); 
            }
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR]))
        {
            value = sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR]);
            sal_printf(", drop color is %s", color_str[value]); 
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH]))
        {
            sal_printf(", use-l3-length"); 
        }

        if (sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS]))
        {
            sal_printf(", statistics enable"); 
        }
        sal_printf("\n");    

        if ('\0' != info.value[TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME][0])
        {
            sal_printf(" bind action profile: %s\n", info.value[TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME]); 
        }

        if (print_stats && sal_atoi(info.value[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS]))
        {
            sal_printf("  Confirm packets %s, bytes %s\n", info.value[TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT],
                                                    info.value[TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE]); 
            sal_printf("  Exceed packets %s, bytes %s\n", info.value[TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT],
                                                    info.value[TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE]); 
            sal_printf("  Violate packets %s, bytes %s\n", info.value[TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT],
                                                    info.value[TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE]);                                                 
        }
        sal_printf("  Applied-interfaces:\n");
        sal_printf("             interface                direction\n");
        sal_printf("             ----------------------------------\n");
        for (i = 0; i< array2_size; i++)
        {
            sal_printf("             %-25s",IFNAME_ETH2FULL(if_array2[i].if_name, ifname_full));
            if (0== if_array2[i].dir)
            {
                sal_printf("input\n");
            }
            else if (1== if_array2[i].dir)
            {
                sal_printf("output\n");
            }
            else if (2== if_array2[i].dir)
            {
                sal_printf("input/output\n");
            }
        }
        if (!i || i %3 != 0)
        {
            sal_printf("\n");
        }
        sal_printf("  Applied-policy-map:\n");
        sal_printf("             policy-map               class-map\n");
        sal_printf("             ----------------------------------\n");
        for (i = 0; i< p_size; i++)
        {
            sal_printf("             %-25s", policy_class_array[i].policy_map_name);
            sal_printf("%s", policy_class_array[i].class_map_name);

            sal_printf("\n");
        }
    }
    return PM_E_NONE;
}

int32
cdbclt_show_qos_policer_profile(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    cfg_parse_info_t parse_info;    
    char  line[FILE_READ_LINE_LEN];
    int32 rc = PM_E_NONE;
    bool  print_stats = FALSE;
    cdbctl_aclqos_attr_entry_t if_array[ GLB_PORT_NUM_PER_CHIP];
    uint32 array_size = 0;
    cdbctl_class_map_action_t class_array[128];
    uint32 class_array_size = 0;
    
    rc = cfg_cmd_parse_para(para, &parse_info);
    if (rc < 0)
    {
        return rc;
    }
   
    if (parse_info.argc > 1)
    {
        print_stats = TRUE;
    }

    cdbctl_show_get_class_map_action(class_array,&class_array_size);
    cdbctl_show_get_aclqos_attr(if_array, &array_size);
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_policer_profile_one(line, print_stats, if_array, array_size, class_array, class_array_size);        
    }

    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_drop_profile_one(char *line, cdbctl_aclqos_attr_entry_t *if_array, uint32 array_size)
{
    cfg_parse_get_info_t info;
    cdbctl_aclqos_field_entry_t if_array2[GLB_PORT_NUM_PER_CHIP];
    uint32 array2_size = 0;
    char ifname_full[IFNAME_EXT_SIZE];
    uint32 i = 0;
    uint32 j = 0;
    
    cfg_get_parse(line, &info, 0);
    if (!info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD])
    {
        if (sal_strcmp(info.value[TBL_QOS_DROP_PROFILE_FLD_KEY], "TAIL_DROP") && sal_strcmp(info.value[TBL_QOS_DROP_PROFILE_FLD_KEY], "RANDOM_DROP"))
        {
            sal_printf("  drop-profile-name: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_KEY]);
        }
    }
    else
    {
        _cdbctl_get_aclqos_if_field(QOS_DROP, info.value[TBL_QOS_DROP_PROFILE_FLD_NAME], if_array, array_size, if_array2, &array2_size); 
            
        sal_printf(" DROP-PROFILE-NAME: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_NAME]); 
        sal_printf("  green packet threshold:\n");
        
        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD]))
        {
            sal_printf("    maximum buffer cell: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD]);
        }
        else
        {
            sal_printf("    maximum buffer cell: 0\n");
        }

        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD]))
        {
            sal_printf("    minimum buffer cell: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD]);
        }
        else
        {
            sal_printf("    minimum buffer cell: 0\n");
        }

        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY]))
        {
            sal_printf("    drop probability   : %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY]);
        } 
        else
        {
            sal_printf("    drop probability   : 0\n");
        }

        /* yellow */
        sal_printf("  yellow packet threshold:\n");
        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD]))
        {
            sal_printf("    maximum buffer cell: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD]);
        }
        else
        {
            sal_printf("    maximum buffer cell: 0\n");
        }

        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD]))
        {
            sal_printf("    minimum buffer cell: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD]);
        }
        else
        {
            sal_printf("    minimum buffer cell: 0\n");
        }

        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY]))
        {
            sal_printf("    drop probability   : %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY]);
        } 
        else
        {
            sal_printf("    drop probability   : 0\n");
        }
        
        /* red */
        sal_printf("  red packet threshold:\n");
        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD]))
        {
            sal_printf("    maximum buffer cell: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD]);
        }
        else
        {
            sal_printf("    maximum buffer cell: 0\n");
        }

        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD]))
        {
            sal_printf("    minimum buffer cell: %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD]);
        }
        else
        {
            sal_printf("    minimum buffer cell: 0\n");
        }
        if (sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY]))
        {
            sal_printf("    drop probability   : %s\n", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY]);
        } 
        else
        {
            sal_printf("    drop probability   : 0\n");
        }
        sal_printf("  Applied-queue:\n");
        for (i = 0; i< array2_size; i++)
        {
            sal_printf("            %s:queue  ",IFNAME_ETH2FULL(if_array2[i].if_name, ifname_full));
            for(j = 0; j < if_array2[i].queue_count; j++)
            {
                sal_printf("%d",if_array2[i].queue[j]);
                if (j+1 < if_array2[i].queue_count)
                {
                    sal_printf("/");
                }
            }
            sal_printf("\n");
        }
    }

    return PM_E_NONE;
}


int32
cdbclt_show_qos_drop_profile(FILE *fp, cfg_parse_info_t *p_info)
{
    char line[FILE_READ_LINE_LEN];
    cdbctl_aclqos_attr_entry_t if_array[GLB_PORT_NUM_PER_CHIP];
    uint32 array_size = 0;
    
    cdbctl_show_get_aclqos_attr(if_array, &array_size);
                             
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_drop_profile_one(line, if_array, array_size);        
    }
    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_queue_shape_profile_one(char *line, cdbctl_aclqos_attr_entry_t *if_array, uint32 array_size)
{
    cfg_parse_get_info_t info;
    uint8 has_config = FALSE;
    cdbctl_aclqos_field_entry_t if_array2[GLB_PORT_NUM_PER_CHIP];
    uint32 array2_size = 0;
    char ifname_full[IFNAME_EXT_SIZE];
    uint32 i =0;
    uint32 j =0;
    
    cfg_get_parse(line, &info, 0);
    if(!info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE])
    {
        if (sal_strcmp(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_KEY], "SCHEDULER_PROFILE"))
        {
            sal_printf("  scheduler-profile-name: %s\n", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_KEY]);
        }
    }
    else
    {
        _cdbctl_get_aclqos_if_field(QOS_QUEUE_SHAPE, info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_NAME], if_array, array_size, if_array2, &array2_size);
        sal_printf(" SCHEDULER-PROFILE-NAME: %s\n", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_NAME]); 
        
        if (sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_CIR]))
        {
            sal_printf("  CIR is %s kbps", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_CIR]);  
            has_config = TRUE;
        }

        if (sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_PIR]))
        {
            sal_printf(", PIR is %s kbps", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR]);  
            has_config = TRUE;
        }
        if(has_config)
        {
            sal_printf("\n");
            has_config = FALSE;
        }
        if(sal_strcmp(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE], "dwrr"))
        {
            sal_printf("  scheduler mode is sp\n");
        }
        else
        {
            sal_printf("  scheduler mode is dwrr\n");
        }
        if(sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT]))
        {
            sal_printf("  scheduler dwrr weight is %s\n", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT]);
        }
        sal_printf("  Applied-queue:\n");
        for (i = 0; i< array2_size; i++)
        {
            sal_printf(  "           %s:queue  ",IFNAME_ETH2FULL(if_array2[i].if_name, ifname_full));
            for(j = 0; j < if_array2[i].queue_count; j++)
            {
                sal_printf("%d",if_array2[i].queue[j]);
                if (j+1 < if_array2[i].queue_count)
                {
                    sal_printf("/");
                }
            }
            sal_printf("\n");
        }
    }
    
    return PM_E_NONE;
}

static int32
_cdbclt_show_qos_port_shape_profile_one(char *line, cdbctl_aclqos_attr_entry_t *if_array, uint32 array_size)
{
    cfg_parse_get_info_t info;
    uint8 has_config = FALSE;
    cdbctl_aclqos_field_entry_t if_array2[GLB_PORT_NUM_PER_CHIP];
    uint32 array2_size = 0;
    char ifname_full[IFNAME_EXT_SIZE];
    uint32 i = 0;
    
    cfg_get_parse(line, &info, 0);        
    if (!info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_PIR])
    {
        if (sal_strcmp(info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_KEY], "PORT_SHAPE_PROFILE"))
        {
            sal_printf("  port-shape-profile-name: %s\n", info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_KEY]);
        }
    }
    else
    {
        _cdbctl_get_aclqos_if_field(QOS_PORT_SHAPE, info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_NAME], if_array, array_size, if_array2, &array2_size);
        sal_printf(" PORT-SHAPE-PROFILE-NAME: %s\n", info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_NAME]); 
        
        if (sal_atoi(info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_FLAGS_PIR]))
        {
            sal_printf("  PIR is %s kbps", info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_PIR]);  
            has_config = TRUE;
        }
        sal_printf("\n");
        sal_printf("  Applied-interfaces: ");
        for (i = 0; i < array2_size; i++)
        {
            sal_printf("%-15s  ",IFNAME_ETH2FULL(if_array2[i].if_name, ifname_full));
            if ((i+1)%3 == 0)
            {
                sal_printf("\n");
                sal_printf("%-22c",' ');
            }
        }
        if(has_config)
        {
            sal_printf("\n");
            has_config = FALSE;
        } 
    }
    
    return PM_E_NONE;
}

int32
cdbclt_show_qos_queue_shape_profile(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cdbctl_aclqos_attr_entry_t if_array[GLB_PORT_NUM_PER_CHIP];
    uint32 array_size = 0;
    
    cdbctl_show_get_aclqos_attr(if_array, &array_size);
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_queue_shape_profile_one(line, if_array, array_size);
    }

    return PM_E_NONE;
}

int32
cdbclt_show_qos_port_shape_profile(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cdbctl_aclqos_attr_entry_t if_array[GLB_PORT_NUM_PER_CHIP];
    uint32 array_size = 0;
    
    cdbctl_show_get_aclqos_attr(if_array, &array_size);
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_qos_port_shape_profile_one(line, if_array, array_size);
    }

    return PM_E_NONE;
}

int32
cdbclt_show_copp_cfg(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    sal_printf("Copp config\n");
    sal_printf("---------------------------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        sal_printf("policy-map NAME : %s\n",info.value[TBL_COPP_CFG_FLD_COPP_PNAME]);
        sal_printf("total rate      : %s\n",info.value[TBL_COPP_CFG_FLD_COPP_TOTAL_RATE]);
    }
    return PM_E_NONE;
}

int32
cdbclt_show_cpu_traffic_group(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;
    char* group[6] = {"class 0", "class 1", "class 2", "class 3", "class 4","class 5"};
    /*          %-26s %-12s %-s*/
    sal_printf("%-26s %-12s\n", "control-plane class", "rate (pps)");
    sal_printf("--------------------------+------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        sal_printf("%-26s %-12s\n", group[sal_atoi(info.value[TBL_CPU_TRAFFIC_GROUP_FLD_KEY])], info.value[TBL_CPU_TRAFFIC_GROUP_FLD_RATE]);
    }
    PM_E_RETURN(cdbclt_get_tbl_line(TBL_COPP_CFG, NULL, 0, line, FILE_READ_LINE_LEN));
    sal_memset(&info, 0, sizeof(info));
    cfg_get_parse(line, &info, 0);
    sal_printf("--------------------------+------------\n");
    sal_printf("%-26s %-12s\n", "total rate", info.value[TBL_COPP_CFG_FLD_COPP_TOTAL_RATE]);

   /*modified by yejl for bug 49548, 2018-12-12*/
   /*modified by yejl for bug 50657, 2019-01-14*/
   /*modified by yejl for bug 50660, 2019-01-14*/
    sal_printf("\n"); 
    sal_printf("control-plane class information:\n"); 
    sal_printf("    class 5 is used for all managing packet and ssh telnet packet!\n");
    sal_printf("    class 4 is used for follow protocol packet!\n"); 
    sal_printf("       LACP, ESMC, STP, ERPS, OSPF, VRRP, MLAG, L2PROTOCOL!\n"); 
    sal_printf("    class 3 is used for follow protocol packet!\n"); 
    sal_printf("       LLDP, EAPOL, DHCP, IGMP, PTP!\n"); 
    sal_printf("    class 2 is used for packet-in!\n");
    sal_printf("    class 1 is used for ARP and ICMPv6 (include ND) packet!\n");
    sal_printf("    class 0 is used for other packet!\n");

    return PM_E_NONE;
}

static int32
_cdbclt_show_aclqos_if_stats_one(char *line, cfg_parse_info_t* p_parse_info)
{
    cfg_parse_get_info_t info;
    cfg_get_parse(line, &info, 0); 
    uint32 queue_index = 0;
    uint32 queue_index_max = 0;
    cfg_parse_get_info_t queue_transimt_pkt_info;
    cfg_parse_get_info_t queue_transimt_byte_info;
    cfg_parse_get_info_t queue_drop_pkt_info;
    cfg_parse_get_info_t queue_drop_byte_info;

    /* BEGIN: queue type */
    int32 rc = PM_E_NONE;

    char chip_line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t chip_info;

    PM_E_RETURN(cdbclt_get_tbl_line(TBL_CHIP, NULL, 0, chip_line, FILE_READ_LINE_LEN));
    cfg_get_parse(chip_line, &chip_info, 0);

    if(2 == p_parse_info->argc)
    {
        if(!sal_strcmp(p_parse_info->argv[0], "flags_port_get_input_policer_stats"))
        {
            sal_printf("  Confirm packets %s, bytes %s\n", info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET],
                                                    info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE]); 
            sal_printf("  Exceed packets %s, bytes %s\n", info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET],
                                                    info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE]); 
            sal_printf("  Violate packets %s, bytes %s\n", info.value[TBL_ACLQOS_IF_STATS_FLD_RED_PACKET],
                                                    info.value[TBL_ACLQOS_IF_STATS_FLD_RED_BYTE]);        
            sal_printf("  Total packets %"PRIu64", bytes %"PRIu64"\n",  sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_RED_PACKET], NULL, 10),
                                                              sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_RED_BYTE], NULL, 10));
        }
        else if(!sal_strcmp(p_parse_info->argv[0], "flags_port_get_output_policer_stats"))
        {
            sal_printf("  Confirm packets %s, bytes %s\n", info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT],
                                                    info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT]); 
            sal_printf("  Exceed packets %s, bytes %s\n", info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT],
                                                    info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT]); 
            sal_printf("  Violate packets %s, bytes %s\n", info.value[TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT],
                                                   info.value[TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT]);        
            sal_printf("  Total packets %"PRIu64", bytes %"PRIu64"\n",  sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT], NULL, 10),
                                                              sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT], NULL, 10)
                                                            + sal_strtou64(info.value[TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT], NULL, 10));
        }
        else if(!sal_strcmp(p_parse_info->argv[0], "flags_get_queue_stats"))
        {
            sal_memset(&queue_transimt_pkt_info, 0x0, sizeof(cfg_parse_get_info_t));
            sal_memset(&queue_transimt_byte_info, 0x0, sizeof(cfg_parse_get_info_t));
            sal_memset(&queue_drop_pkt_info, 0x0, sizeof(cfg_parse_get_info_t));
            sal_memset(&queue_drop_byte_info, 0x0, sizeof(cfg_parse_get_info_t));
            cfg_get_parse_comma(info.value[TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT], &queue_transimt_pkt_info);
            cfg_get_parse_comma(info.value[TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE], &queue_transimt_byte_info);
            cfg_get_parse_comma(info.value[TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT], &queue_drop_pkt_info);
            cfg_get_parse_comma(info.value[TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE], &queue_drop_byte_info);
            sal_printf("QUEUE STATS:\n");
            sal_printf("Queue Transmit-Packets     Transmit-Bytes       Drop-Packets    Drop-Bytes\n");
            sal_printf("-----+--------------------+--------------------+---------------+---------------\n");

            /* BEGIN: fix qos bug 38803 add by wangqj 2016.7.26 */
            queue_index_max = GLB_QOS_USED_PORT_QUEUE_NUM;
            /* END: fix qos bug 38803 add by wangqj 2016.7.26 */
            
            for (queue_index = 0; queue_index < queue_index_max; queue_index++)
            {
                sal_printf("%-5d %-20s %-20s", queue_index, queue_transimt_pkt_info.argv[queue_index], queue_transimt_byte_info.argv[queue_index]);
                sal_printf(" %-15s %-s\n", queue_drop_pkt_info.argv[queue_index], queue_drop_byte_info.argv[queue_index]);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_aclqos_if_queue_drop(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    int32 i = 0;
    char  line[FILE_READ_LINE_32K_LEN];
    cfg_parse_get_info_t info;
    bool random_detect = FALSE;
    #ifdef GREATBELT
        int32 buffer = 256;
        char *cells = "one";
    #else
        int32 buffer = 288;
        char *cells = "two";
    #endif

    /*         %-6s   %-15d          %-7s   %-10s     %-10s     %-s           */
    sal_printf("QUEUE DROP:\n");
    sal_printf("(1)Threshold unit is cell, 1 cell = %d bytes\n", buffer);
    sal_printf("(2)A packet occupies at least %s buffers cell in queue drop threshold\n", cells);
    sal_printf("   and a cell could only be occupied by a packet\n");
    sal_printf("Queue Drop-mode      Color  Max_Treshold Min_Treshold Probability\n");
    sal_printf("-----+--------------+------+-------------+-------------+---------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);        
        sal_printf("%-6d", i++);
        if (0 != sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT]))
        {
            random_detect = TRUE;
            sal_printf("%-15s", "random-detect");
        }
        else
        {
            random_detect = FALSE;
            sal_printf("%-15s", "tail-drop");
        }
        sal_printf("%-7s", "green");
        sal_printf("%-14s", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD]);
        if (random_detect)
        {
            sal_printf("%-14u", sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD]));
            sal_printf("%-s", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY]);
        }
        else
        {
            sal_printf("%-14s", "-");
            sal_printf("%-s", "-");
        }
        sal_printf("\n");
        
        sal_printf("%21s", " ");
        sal_printf("%-7s", "yellow");
        sal_printf("%-14s", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD]);
        if (random_detect)
        {
            sal_printf("%-14u", sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD]));
            sal_printf("%-s", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY]);
        }
        else
        {
            sal_printf("%-14s", "-");
            sal_printf("%-s", "-");
        }
        sal_printf("\n");

        sal_printf("%21s", " ");
        sal_printf("%-7s", "red");
        sal_printf("%-14s", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD]);
        if (random_detect)
        {
            sal_printf("%-14u", sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD]));
            sal_printf("%-s", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY]);
        }
        else
        {
            sal_printf("%-14s", "-");
            sal_printf("%-s", "-");
        }
        sal_printf("\n");

    }
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_aclqos_if_queue_shape(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    int32 i = 0;
    char  line[FILE_READ_LINE_32K_LEN];
    cfg_parse_get_info_t info;
    int32 value = 0;
    uint32 flag = FALSE;
    uint32 class = 0;

    /*         %-6s   %-15d     %-d  */
    sal_printf("QUEUE SCHEDULER:\n");
    sal_printf("Queue CIR(Kbps)     PIR(Kbps)     class      weight      mode\n");
    sal_printf("-----+--------------+-------------+----------+----------+---------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        sal_printf("%-6d", i);
        if (('\0' == line[0]) || ('\n' == line[0]))
        {
            sal_printf("\n");
            continue;
        }
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        value = sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_CIR]);
        sal_printf("%-15s", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_CIR]);
        
        value = sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_PIR]);
        sal_printf("%-14s", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR]);
        value = sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT]);
        if(!sal_strcmp(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE], "dwrr"))
        {
            if(!flag)
            {
                class = i;
                flag = TRUE;
            }
            sal_printf("%-11d", class);
        }
        else
        {
            sal_printf("%-11d", i);
        }
        i++;
        if(0 != value)
        {
            if(sal_strcmp(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE], "dwrr"))
            {
                sal_printf("%-9s", "-");
            }
            else
            {
                sal_printf("%-9s", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT]);
            }
        }
        else
        {
            sal_printf("%-9s", "0");
        }
        if(sal_strcmp(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE], "dwrr"))
        {
            sal_printf("%-10s", "  sp");
        }
        else
        {
            sal_printf("%-10s", "  dwrr");
        }
        sal_printf("\n");
    }
    sal_printf("\n");
    return PM_E_NONE;
}

static int32
_cdbctl_show_aclqos_if_port_shape(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
    cfg_parse_get_info_t info;

    if (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        if (('\0' == line[0]) || ('\n' == line[0]))
        {
            sal_printf("\n");
            return PM_E_NONE;
        }
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        sal_printf("    port-shape-profile name:%s\n", info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_NAME]);
        sal_printf("    PIR :%d(Kbps)\n", sal_atoi(info.value[TBL_QOS_PORT_SHAPE_PROFILE_FLD_PIR]));
    }
    return PM_E_NONE;
}

static int32
_cdbctl_show_aclqos_if_classification_remark(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
    char temp[CMD_BUF_16];
    char ifname_full[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        if (('\0' == line[0]) || ('\n' == line[0]))
        {
            sal_printf("\n");
            return PM_E_NONE;
        }
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
    #ifdef USW
        sal_printf("Interface   Cos-domain   Dscp-domain  Trust        Replace-cos   Replace-dscp\n");
        sal_printf("-----------+------------+------------+------------+-------------+-------------\n");
        sal_printf("%-11s ", IFNAME_ETH2FULL(info.value[TBL_ACLQOS_IF_FLD_KEY], ifname_full));
        sal_printf("%-12s ", info.value[TBL_ACLQOS_IF_FLD_COS_DOMAIN]);
        sal_printf("%-12s ", info.value[TBL_ACLQOS_IF_FLD_DSCP_DOMAIN]);
        if (0 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_TRUST]))
        {
            sal_sprintf(temp, "port(cos=%s)", info.value[TBL_ACLQOS_IF_FLD_PORT_TC]);
            sal_printf("%-12s ", temp);
        }
        else if(2 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_TRUST]))
        {
            sal_printf("%-12s ", "cos");
        }
        else if(4 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_TRUST]))
        {
            sal_printf("%-12s ", "dscp");
        }

        if (1 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_REPLACE_COS]))
        {
            sal_printf("%-13s ", "enable");
        }
        else
        {
            sal_printf("%-13s ", "disable");
        }
        if (1 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_REPLACE_DSCP]))
        {
            sal_printf("%-13s ", "enable");
        }
        else
        {
            sal_printf("%-13s ", "disable");
        }
    #else
        sal_printf("Interface   Domain     Trust        Replace-cos   Replace-dscp\n");
        sal_printf("-----------+----------+------------+-------------+-------------\n");
        sal_printf("%-11s ", IFNAME_ETH2FULL(info.value[TBL_ACLQOS_IF_FLD_KEY], ifname_full));
        sal_printf("%-10s ", info.value[TBL_ACLQOS_IF_FLD_DOMAIN]);
        if (0 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_TRUST]))
        {
            sal_sprintf(temp, "port(cos=%s)", info.value[TBL_ACLQOS_IF_FLD_PORT_TC]);
            sal_printf("%-12s ", temp);
        }
        else if(2 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_TRUST]))
        {
            sal_printf("%-12s ", "cos");
        }
        else if(4 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_TRUST]))
        {
            sal_printf("%-12s ", "dscp-exp");
        }

        if (1 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_REPLACE_COS]))
        {
            sal_printf("%-13s ", "enable");
        }
        else
        {
            sal_printf("%-13s ", "disable");
        }
        if (1 == sal_atoi(info.value[TBL_ACLQOS_IF_FLD_REPLACE_DSCP]))
        {
            sal_printf("%-13s ", "enable");
        }
        else
        {
            sal_printf("%-13s ", "disable");
        }
   #endif
        sal_printf("\n");
    }
    return PM_E_NONE;
}

int32
cdbctl_show_aclqos_if_stats(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    cfg_parse_info_t parse_info;    
    char  line[FILE_READ_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    rc = cfg_cmd_parse_para(para, &parse_info);
    if (rc < 0)
    {
        return rc;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbclt_show_aclqos_if_stats_one(line, &parse_info);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_aclqos_if(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    cfg_parse_info_t parse_info;    
    int32 rc = PM_E_NONE;

    rc = cfg_cmd_parse_para(para, &parse_info);
    if (rc < 0)
    {
        return rc;
    }
    if (data_len)
    {
        if (!sal_strcmp(para, "queue_drop_profile"))
        {
            _cdbctl_show_aclqos_if_queue_drop(fp, para, &parse_info);
        }
        else if(!sal_strcmp(para, "queue_shape_profile"))
        {
            _cdbctl_show_aclqos_if_queue_shape(fp, para, &parse_info);
        }
        else if(!sal_strcmp(para, "port_shape_profile"))
        {
            _cdbctl_show_aclqos_if_port_shape(fp, para, &parse_info);
        }
        else if(!sal_strcmp(para, "classification_remark"))
        {
            _cdbctl_show_aclqos_if_classification_remark(fp, para, &parse_info);
        }
    }
    return PM_E_NONE;
}
    
