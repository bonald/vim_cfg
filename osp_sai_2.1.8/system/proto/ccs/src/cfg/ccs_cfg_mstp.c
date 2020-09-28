
#include "proto.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "mstp_define.h"

int32
ccs_cfg_mstp_build_one_instance(FILE* fp, cfg_cmd_para_t *para, tbl_mstp_instance_t *p_db_inst)
{
    char str[CMD_BUF_256];
    char tmp[CMD_BUF_32];
    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    uint32 used = 0;
    uint32 tmp_len = 0;
    uint32 line_len = 60;
    
    sal_memset(str, 0, sizeof(str));
    str[0] = CMD_ZERO_CHAR;

    for (offset = 0; offset < GLB_MAX_VLAN_ID; offset++)
    {
        if (GLB_BMP_ISSET(p_db_inst->vlan, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(p_db_inst->vlan, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= line_len)
                {
                    if ((used > 0) && (str[used-1] == ','))
                    {
                        str[used-1] = 0;
                    }
                    sal_fprintf(fp, INTEND_1_STR"instance %u vlan %s\n", p_db_inst->key.instance, str);
                    str[0] = CMD_ZERO_CHAR;
                    used = 0;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 5);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= line_len)
                {
                    if ((used > 0) && (str[used-1] == ','))
                    {
                        str[used-1] = 0;
                    }
                    sal_fprintf(fp, INTEND_1_STR"instance %u vlan %s\n", p_db_inst->key.instance, str);
                    str[0] = CMD_ZERO_CHAR;
                    used = 0;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 10);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
        sal_fprintf(fp, INTEND_1_STR"instance %u vlan %s\n", p_db_inst->key.instance, str);
        str[0] = CMD_ZERO_CHAR;
        used = 0;
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_mstp_mst(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_db_global = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_master_t* p_instance_master = tbl_mstp_instance_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    uint32 has_config = FALSE;
    uint32 all_zero = FALSE;

    if (sal_strlen(p_db_global->mstp_region_name))
    {
        has_config = TRUE;
    }
    if (MSTP_REVISION_LEVEL_DEF != p_db_global->mstp_revision_level)
    {
        has_config = TRUE;
    }
    CTCLIB_SLIST_LOOP(p_instance_master->inst_list, p_db_inst, listnode)
    {
        if (MSTP_DEFAULT_INSTANCE == p_db_inst->key.instance)
        {
            continue;
        }
        GLB_BMP_CHECK_ALL_ZERO(p_db_inst->vlan, all_zero);
        if (all_zero)
        {
            continue;
        }
        has_config = TRUE;
    }    

    if (!has_config)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "spanning-tree mst configuration\n");

    if (sal_strlen(p_db_global->mstp_region_name))
    {
        sal_fprintf(fp, INTEND_1_STR"region %s\n", p_db_global->mstp_region_name);
    }
    if (MSTP_REVISION_LEVEL_DEF != p_db_global->mstp_revision_level)
    {
        sal_fprintf(fp, INTEND_1_STR"revision %u\n", p_db_global->mstp_revision_level);
    }

    CTCLIB_SLIST_LOOP(p_instance_master->inst_list, p_db_inst, listnode)
    {
        if (MSTP_DEFAULT_INSTANCE == p_db_inst->key.instance)
        {
            continue;
        }
        GLB_BMP_CHECK_ALL_ZERO(p_db_inst->vlan, all_zero);
        if (all_zero)
        {
            continue;
        }
        ccs_cfg_mstp_build_one_instance(fp, para, p_db_inst);
    }

    sal_fprintf(fp, "!\n");

    has_config = FALSE;
    CTCLIB_SLIST_LOOP(p_instance_master->inst_list, p_db_inst, listnode)
    {
        if (MSTP_DEFAULT_INSTANCE == p_db_inst->key.instance)
        {
            continue;
        }
        
        if (MSTP_DEF_BRIDGE_PRIORITY != p_db_inst->bridge_priority)
        {
            sal_fprintf(fp, "spanning-tree instance %u priority %u\n", p_db_inst->key.instance, p_db_inst->bridge_priority);
            has_config = TRUE;
        }
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
        
    return PM_E_NONE;
}

#define MSTP_ORDER_ERROR             0
#define MSTP_ORDER_HELLO_MAXAGE_FWD  1
#define MSTP_ORDER_HELLO_FWD_MAXAGE  2
#define MSTP_ORDER_FWD_MAXAGE_HELLO  3
#define MSTP_ORDER_FWD_HELLO_MAXAGE  4
#define MSTP_ORDER_MAXAGE_HELLO_FWD  5
#define MSTP_ORDER_MAXAGE_FWD_HELLO  6

static inline int32
mstp_cli_is_fwd_delay_gt_maxage(uint32 forward_delay, uint32 max_age)
{
    if (2 * ( forward_delay - 1) >= max_age)
        return TRUE;
    return FALSE;
}

static inline int32
mstp_cli_is_maxage_gt_hello_time(uint32 max_age, uint32 hello_time)
{
    if (max_age >= 2 * ( hello_time + 1 ))
        return TRUE;
    return FALSE;
}
    
#define MSTP_WRITE_TIMER_HELLO() \
do { \
    if (MSTP_TIMER_DEF_HELLO_TIME != p_glb->bridge_helloTime) \
    { \
        sal_fprintf(fp, "spanning-tree hello-time %u\n", p_glb->bridge_helloTime); \
        *has_config = TRUE; \
    } \
} while(0)

#define MSTP_WRITE_TIMER_MAX_AGE() \
do { \
    if (MSTP_TIMER_DEF_MAX_AGE != p_glb->bridge_maxAge) \
    { \
        sal_fprintf(fp, "spanning-tree max-age %u\n", p_glb->bridge_maxAge); \
        *has_config = TRUE; \
    } \
} while(0)

#define MSTP_WRITE_TIMER_FORWARD_DELAY() \
do { \
    if (MSTP_TIMER_DEF_FWD_DELAY != p_glb->bridge_fwdDelay) \
    { \
        sal_fprintf(fp, "spanning-tree forward-time %u\n", p_glb->bridge_fwdDelay); \
        *has_config = TRUE; \
    } \
} while(0)

int32
ccs_cfg_build_mstp_global_timers(FILE* fp, uint32 *has_config, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    uint32 order = 0;

    if ( mstp_cli_is_maxage_gt_hello_time(MSTP_TIMER_DEF_MAX_AGE, p_glb->bridge_helloTime)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, p_glb->bridge_helloTime)
         && mstp_cli_is_fwd_delay_gt_maxage(MSTP_TIMER_DEF_FWD_DELAY, p_glb->bridge_maxAge)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, p_glb->bridge_maxAge)
         )
    {
        order = MSTP_ORDER_HELLO_MAXAGE_FWD;
    }
    else if (mstp_cli_is_maxage_gt_hello_time(MSTP_TIMER_DEF_MAX_AGE, p_glb->bridge_helloTime)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, MSTP_TIMER_DEF_MAX_AGE)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, p_glb->bridge_helloTime)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, p_glb->bridge_maxAge)
         )
    {
        order = MSTP_ORDER_HELLO_FWD_MAXAGE;
    }
    else if (mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, MSTP_TIMER_DEF_MAX_AGE)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, p_glb->bridge_maxAge)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, MSTP_TIMER_DEF_HELLO_TIME)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, p_glb->bridge_helloTime)
         )
    {
        order = MSTP_ORDER_FWD_MAXAGE_HELLO;
    }
    else if (mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, MSTP_TIMER_DEF_MAX_AGE)
         && mstp_cli_is_maxage_gt_hello_time(MSTP_TIMER_DEF_MAX_AGE, p_glb->bridge_helloTime)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, p_glb->bridge_maxAge)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, p_glb->bridge_helloTime)
         )
    {
        order = MSTP_ORDER_FWD_HELLO_MAXAGE;
    }
    else if (mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, MSTP_TIMER_DEF_HELLO_TIME)
         && mstp_cli_is_fwd_delay_gt_maxage(MSTP_TIMER_DEF_FWD_DELAY, p_glb->bridge_maxAge)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, p_glb->bridge_helloTime)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, p_glb->bridge_maxAge)
         )
    {
        order = MSTP_ORDER_MAXAGE_HELLO_FWD;
    }
    else if (mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, MSTP_TIMER_DEF_HELLO_TIME)
         && mstp_cli_is_fwd_delay_gt_maxage(MSTP_TIMER_DEF_FWD_DELAY, p_glb->bridge_maxAge)
         && mstp_cli_is_fwd_delay_gt_maxage(p_glb->bridge_fwdDelay, p_glb->bridge_maxAge)
         && mstp_cli_is_maxage_gt_hello_time(p_glb->bridge_maxAge, p_glb->bridge_helloTime)
         )
    {
        order = MSTP_ORDER_MAXAGE_FWD_HELLO;
    }
    else
    {
        order = MSTP_ORDER_ERROR;
    }

    switch (order)
    {
    case MSTP_ORDER_HELLO_MAXAGE_FWD:
        MSTP_WRITE_TIMER_HELLO();
        MSTP_WRITE_TIMER_MAX_AGE();
        MSTP_WRITE_TIMER_FORWARD_DELAY();
        break;

    case MSTP_ORDER_HELLO_FWD_MAXAGE:
        MSTP_WRITE_TIMER_HELLO();
        MSTP_WRITE_TIMER_FORWARD_DELAY();
        MSTP_WRITE_TIMER_MAX_AGE();
        break;

    case MSTP_ORDER_MAXAGE_HELLO_FWD:
        MSTP_WRITE_TIMER_MAX_AGE();
        MSTP_WRITE_TIMER_HELLO();
        MSTP_WRITE_TIMER_FORWARD_DELAY();
        break;

    case MSTP_ORDER_MAXAGE_FWD_HELLO:
        MSTP_WRITE_TIMER_MAX_AGE();
        MSTP_WRITE_TIMER_FORWARD_DELAY();
        MSTP_WRITE_TIMER_HELLO();
        break;

    case MSTP_ORDER_FWD_HELLO_MAXAGE:
        MSTP_WRITE_TIMER_FORWARD_DELAY();
        MSTP_WRITE_TIMER_HELLO();
        MSTP_WRITE_TIMER_MAX_AGE();
        break;

    case MSTP_ORDER_FWD_MAXAGE_HELLO:
        MSTP_WRITE_TIMER_FORWARD_DELAY();
        MSTP_WRITE_TIMER_MAX_AGE();
        MSTP_WRITE_TIMER_HELLO();
        break;

    default:
        /* use the default config */
        break;
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_mstp_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    uint32 has_config = FALSE;
    char str[MAX_CMD_STR_LEN];

    if (MSTP_TYPE_RSTP != p_glb->type)
    {
        sal_fprintf(fp, "spanning-tree mode %s\n", 
            cdb_enum_val2str(mstp_type_lower_strs, MSTP_TYPE_MAX, p_glb->type));
        has_config = TRUE;
    }
    
    if (MSTP_DEF_BRIDGE_PRIORITY != p_glb->bridge_priority)
    {
        sal_fprintf(fp, "spanning-tree priority %u\n", p_glb->bridge_priority);
        has_config = TRUE;
    }

    if (MSTP_DEF_PATH_COST != p_glb->pathcost_standard)
    {
        sal_fprintf(fp, "spanning-tree pathcost-standard %s\n", 
            cdb_enum_val2str(mstp_path_cost_strs, MSTP_PATH_COST_MAX, p_glb->pathcost_standard));
        has_config = TRUE;
    }

    ccs_cfg_build_mstp_global_timers(fp, &has_config, para);

    if (MSTP_DEF_MAX_HOPS != p_glb->bridge_maxHops)
    {
        sal_fprintf(fp, "spanning-tree max-hops %u\n", p_glb->bridge_maxHops);
        has_config = TRUE;
    }

    if (MSTP_TX_HOLD_COUNT != p_glb->txHoldCount)
    {
        sal_fprintf(fp, "spanning-tree transmit-holdcount %u\n", p_glb->txHoldCount);
        has_config = TRUE;
    }

    if (p_glb->tc_protection)
    {
        sal_fprintf(fp, "spanning-tree tc-protection\n");
        has_config = TRUE;
    }

    if (p_glb->bpduguard)
    {
        sal_fprintf(fp, "spanning-tree edgeport bpdu-guard\n");
        has_config = TRUE;
    }

    if (p_glb->bpdufilter)
    {
        sal_fprintf(fp, "spanning-tree edgeport bpdu-filter\n");
        has_config = TRUE;
    }

    if (p_glb->enable)
    {
        sal_fprintf(fp, "spanning-tree enable\n");
        has_config = TRUE;
    }

    if (p_glb->of_exclude_all_vlan)
    {
        sal_fprintf(fp, "openflow spanning-tree exclude-all-vlans\n");
        has_config = TRUE;
    }
    sal_memset(str, 0, sizeof(str));
    cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_glb->of_include_vlan_bmp, sizeof(p_glb->of_include_vlan_bmp)/4);
    if (sal_strlen(str))
    {
        sal_fprintf(fp, "openflow spanning-tree include-vlan %s\n", str);
        has_config = TRUE;
    }
    
    ccs_cfg_build_mstp_mst(fp, para);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}
