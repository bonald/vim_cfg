
#include "proto.h"
#include "switch.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"
#include "glb_if_define.h"
#include "glb_phy_define.h"
#include "glb_l2_define.h"

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
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_class_map_action_config_define.h"
#include "gen/tbl_class_map_action_config.h"
#include "glb_qos_define.h"

#include "qos/qos_mgr.h"
#include "qos/qos_mgr_hal.h"
#include "if_agg.h"
#include "ctclib_slist.h"
#include "glb_qos_define.h"

extern int32
class_map_action_update(char *policy_map_name, char *class_map_name, uint32 field_id, char* value, cfg_cmd_para_t *para);

int32
qos_mgr_map_policer(tbl_qos_policer_profile_t* p_db_qos_policer_profile, glb_qos_policer_t* p_policer)
{
   p_policer->is_srtcm = (p_db_qos_policer_profile->algorithm_mode == GLB_QOS_POLICER_MODE_RFC2697);
   p_policer->is_color_blind = (p_db_qos_policer_profile->color_mode == GLB_QOS_POLICER_COLOR_BLIND);
   p_policer->drop_color = p_db_qos_policer_profile->drop_color;
   p_policer->use_l3_length = GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH);
   p_policer->is_stats_en = GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS);
   p_policer->mode = p_db_qos_policer_profile->algorithm_mode;
   p_policer->cir = p_db_qos_policer_profile->cir;
   p_policer->pir = p_db_qos_policer_profile->eir_or_pir;
   p_policer->cbs = GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CBS) ? p_db_qos_policer_profile->cbs : GLB_QOS_POLICER_DEFAULT_BURST ;
   p_policer->pbs = GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS) ? p_db_qos_policer_profile->ebs_or_pbs : GLB_QOS_POLICER_DEFAULT_BURST ;

   return PM_E_NONE;
}

int32
qos_mgr_map_policer_by_name(char *name, glb_qos_policer_t* p_policer)
{
    tbl_qos_policer_profile_t* p_db_qos_policer_profile = NULL;
   
    if (NULL == name)
    {
        return PM_E_NONE;
    }
    p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(name);
    if (NULL == p_db_qos_policer_profile)
    {
        return PM_E_NONE;
    }
    qos_mgr_map_policer(p_db_qos_policer_profile, p_policer);
    return PM_E_NONE;
}


static qos_phb_prop_t 
qos_phb_property[GLB_QOS_PHB_MAX] = 
{
    /*name  priority    color                   qid     class   weight*/
    {"cs1", 0,          GLB_QOS_COLOR_GREEN,    0,      3,      1},
    {"df",  1,          GLB_QOS_COLOR_GREEN,    1,      3,      1},
    {"af11",2,          GLB_QOS_COLOR_GREEN,    2,      4,      4},
    {"af12",2,          GLB_QOS_COLOR_YELLOW,   2,      4,      4},
    {"af13",2,          GLB_QOS_COLOR_RED,      2,      4,      4},
    {"af21",3,          GLB_QOS_COLOR_GREEN,    3,      4,      10},
    {"af22",3,          GLB_QOS_COLOR_YELLOW,   3,      4,      10},
    {"af23",3,          GLB_QOS_COLOR_RED,      3,      4,      10},
    {"af31",4,          GLB_QOS_COLOR_GREEN,    4,      4,      10},
    {"af32",4,          GLB_QOS_COLOR_YELLOW,   4,      4,      10},
    {"af33",4,          GLB_QOS_COLOR_RED,      4,      4,      10},
    {"af41",5,          GLB_QOS_COLOR_GREEN,    5,      4,      10},
    {"af42",5,          GLB_QOS_COLOR_YELLOW,   5,      4,      10},
    {"af43",5,          GLB_QOS_COLOR_RED,      5,      4,      10},
    {"cs2", 6,          GLB_QOS_COLOR_GREEN,    6,      5,      1},
    {"cs3", 7,          GLB_QOS_COLOR_GREEN,    6,      5,      1},
    {"cs6", 8,          GLB_QOS_COLOR_GREEN,    6,      5,      1},
    {"cs7", 9,          GLB_QOS_COLOR_GREEN,    6,      5,      1},
    {"cs4", 10,         GLB_QOS_COLOR_GREEN,    7,      7,      1},
    {"cs5", 11,         GLB_QOS_COLOR_GREEN,    7,      7,      1},
    {"ef",  12,         GLB_QOS_COLOR_GREEN,    7,      7,      1}

};

int32
qos_mgr_map_queue_drop(tbl_qos_drop_profile_t* p_db_qos_drop_profile, glb_qos_queue_drop_t* p_queue_drop)
{
    /* not belong to drop profile */
    //p_queue_drop->mode = p_db_qos_drop_profile->;
    //TODO: add field later
    //p_queue_drop->weight = ;
    // red      ==> drop_prec 1
    // yellow   ==> drop_prec 2
    // green    ==> drop_prec 3
    // critical ==> drop_prec 0
    // pri + color ==> drop_prec
    p_queue_drop->min_th[1] = p_db_qos_drop_profile->red_min_threashold;
    p_queue_drop->min_th[2] = p_db_qos_drop_profile->yellow_min_threashold;
    p_queue_drop->min_th[3] = p_db_qos_drop_profile->green_min_threashold;
    p_queue_drop->min_th[0] = p_db_qos_drop_profile->green_min_threashold;
    
    p_queue_drop->max_th[1] = p_db_qos_drop_profile->red_max_threashold;
    p_queue_drop->max_th[2] = p_db_qos_drop_profile->yellow_max_threashold;
    p_queue_drop->max_th[3] = p_db_qos_drop_profile->green_max_threashold;
    p_queue_drop->max_th[0] = p_db_qos_drop_profile->green_max_threashold;
    
    p_queue_drop->drop_prob[1] = p_db_qos_drop_profile->red_probability;
    p_queue_drop->drop_prob[2] = p_db_qos_drop_profile->yellow_probability;
    p_queue_drop->drop_prob[3] = p_db_qos_drop_profile->green_probability;
    p_queue_drop->drop_prob[0] = p_db_qos_drop_profile->green_probability;

    return PM_E_NONE;
}


int32 
qos_mgr_get_default_queue_class(int32 qid)
{
    int32 i = 0;
    bool phb_enable = tbl_qos_global_get_phb_enable();
    if (phb_enable)
    {
        for (i = 0; i < GLB_QOS_PHB_MAX; i++)
        {
            if (qos_phb_property[i].qid == qid)
            {
                return qos_phb_property[i].class;
            }
        }

    }
    else
    {
        return qid;
    }

    return qid;
}

int32 
qos_mgr_get_default_queue_weight(int32 qid)
{
    int32 i = 0;
    bool phb_enable = tbl_qos_global_get_phb_enable();
    if (phb_enable)
    {
        for (i = 0; i < GLB_QOS_PHB_MAX; i++)
        {
            if (qos_phb_property[i].qid == qid)
            {
                return qos_phb_property[i].weight;
            }
        }

    }
    else
    {
        return 1;
    }

    return 1;
}

glb_qos_color_t
qos_mgr_map_str2color(char* str)
{
    char *color_str[] = 
    {
        "none",
        "red",
        "yellow",
        "green"
    };
    
    int32 i = 0;
    for (i = GLB_QOS_COLOR_NONE; i < GLB_QOS_COLOR_MAX; i++)
    {
        if (0 == sal_strcmp(str, color_str[i]))
        {
            return i;
        }
    }
    return GLB_QOS_COLOR_MAX;
}

glb_qos_phb_type_t
qos_mgr_map_str2phb(char* str)
{
    int32 i = 0;
    for (i = GLB_QOS_PHB_CS1; i < GLB_QOS_PHB_MAX; i++)
    {
        if (0 == sal_strcmp(str, qos_phb_property[i].name))
        {
            return i;
        }
    }
    return GLB_QOS_PHB_MAX;
}

static int32 
_qos_mgr_init_default_domain(tbl_qos_domain_t **pp_domain)
{
    int32 rc = PM_E_NONE;
    int i = 0;
    int j = 0;
    tbl_qos_domain_master_t* p_domain_master = tbl_qos_domain_get_master();
    tbl_qos_domain_t domain;
    bool phb_enable = tbl_qos_global_get_phb_enable();
    
    sal_memset(&domain, 0, sizeof(tbl_qos_domain_t));
    /* 1. init default domain */
    domain.phb_enable = FALSE;
    domain.cfi_enable = FALSE;
    domain.key.id = GLB_QOS_DOMAIN_NUM;
    domain.phb_enable = phb_enable;

    /* ingress */
    for (i = 0; i < GLB_QOS_COS_NUM; i++)
    {
        domain.cos2pri[i] = i << 3;
        domain.cos2color[i] = GLB_QOS_COLOR_GREEN;
        domain.cos2tc[i] = i;
    }
    for (i = 0; i < GLB_QOS_DSCP_NUM; i++)
    {
        domain.dscp2pri[i] = i;
        domain.dscp2color[i] = GLB_QOS_COLOR_GREEN;
        domain.dscp2tc[i] = i/8;
    }
    for (i = 0; i < GLB_QOS_IP_PREC_NUM; i++)
    {
        domain.prec2pri[i] = i << 3;
        domain.prec2color[i] = GLB_QOS_COLOR_GREEN;
    }
    for (i = 0; i < GLB_QOS_EXP_NUM; i++)
    {
        domain.exp2pri[i] = i << 3;
        domain.exp2color[i] = GLB_QOS_COLOR_GREEN;
        domain.exp2tc[i] = i; 
    }
    /* egress */
    for (i = 0; i < GLB_QOS_PRIORITY_NUM; i++)
    {
        for (j = GLB_QOS_COLOR_NONE; j < GLB_QOS_COLOR_MAX; j++)
        {
            domain.pri_color2cos[(i << 2) + j] = i >> 3;
            domain.pri_color2dscp[(i << 2) + j] = i;
            domain.pri_color2exp[(i << 2) + j] = i >> 3;
        }
    }
    for(i = 0; i < GLB_QOS_TC_NUM; i++)
    {
        for(j = GLB_QOS_COLOR_NONE; j < GLB_QOS_COLOR_MAX; j++)
        {
            domain.tc_color2cos[(i << 2) + j] = i;
            domain.tc_color2exp[(i << 2) + j] = i; 
            domain.tc_color2dscp[(i<<2) + j] = i*8;
        }
    }

    if (NULL == p_domain_master->default_domain)
    {
        p_domain_master->default_domain = XCALLOC(MEM_TBL_QOS_DOMAIN, sizeof(tbl_qos_domain_t)); 
    }

    sal_memcpy(p_domain_master->default_domain, &domain, sizeof(domain));
    if (pp_domain)
    {
        *pp_domain = p_domain_master->default_domain;
    }
    
    return rc;
}

static int32 
_qos_mgr_deinit_default_domain(tbl_qos_domain_t **pp_domain)
{
    int32 rc = PM_E_NONE;
    tbl_qos_domain_master_t* p_domain_master = tbl_qos_domain_get_master();

    XFREE(MEM_TBL_QOS_DOMAIN, p_domain_master->default_domain);
    p_domain_master->default_domain = NULL;
    return rc;
}

static int32
_qos_mgr_mpls_domain_del_one(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_iter_args_t *pargs)
{
    int rc = PM_E_NONE;
    PM_E_RETURN(tbl_qos_mpls_domain_del_qos_mpls_domain(&p_qos_mpls_domain->key));
    return PM_E_NONE;
}

static int32
_qos_mgr_del_qos_mpls_domain()
{
    int rc = PM_E_NONE;
    PM_E_RETURN(tbl_qos_mpls_domain_iterate((TBL_ITER_CB_FUNC)_qos_mgr_mpls_domain_del_one, NULL));
    return PM_E_NONE;
}

int32
qos_mgr_set_qos_enable(bool enable)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    tbl_qos_domain_t domain;
    tbl_qos_domain_t* p_default_domain = NULL;

    tbl_qos_queue_shape_profile_master_t *p_queue_shape_master = NULL;
    tbl_qos_queue_shape_profile_t *p_db_qos_queue_shape_profile = NULL;
    tbl_qos_port_shape_profile_master_t* p_port_shape_master = NULL;
    tbl_qos_port_shape_profile_t *p_db_qos_port_shape_profile = NULL;
    tbl_qos_policer_profile_master_t *p_policer_master = NULL;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    tbl_qos_drop_profile_master_t *p_drop_master = NULL;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;
    tbl_qos_domain_master_t* p_domain_master = tbl_qos_domain_get_master();
    tbl_qos_policer_action_profile_master_t *p_action_master = NULL;
    tbl_qos_policer_action_profile_t *p_db_qos_action_profile = NULL;

    tbl_aclqos_if_master_t *p_master = tbl_aclqos_if_get_master();
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 queue_id = 0;
    tbl_class_map_action_config_master_t* p_class_action_mater = tbl_class_map_action_config_get_master();
    tbl_class_map_action_config_t* p_db_class_action_config = NULL;
    ctclib_slistnode_t *listnode_action = NULL;
    char value; 

    sal_memset(&value, 0, sizeof(char));
    if (enable)
    {
        /* 1. init default domain */
        PM_E_RETURN(_qos_mgr_init_default_domain(&p_default_domain));
    #if 0
        /* 2. create domain 0-7 */
        sal_memcpy(&domain, p_default_domain, sizeof(domain));
        for (i = 0; i < GLB_QOS_DOMAIN_NUM; i++)
        {
            domain.key.id = i;
            tbl_qos_domain_add_qos_domain(&domain);
        }
    #endif

        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_aclqos_if, listnode)
        {
            if (GLB_IF_TYPE_VLAN_IF == p_db_aclqos_if->port_type)
            {
                //vlan if aclqos just care dscp_domain and trust field
                p_db_aclqos_if->dscp_domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DSCP_DOMAIN));

                p_db_aclqos_if->trust = GLB_QOS_TRUST_COS;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_TRUST));
            }
            else
            {
                p_db_aclqos_if->domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DOMAIN));

                p_db_aclqos_if->cos_domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_COS_DOMAIN));
                p_db_aclqos_if->dscp_domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DSCP_DOMAIN));

                p_db_aclqos_if->trust = GLB_QOS_TRUST_COS;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_TRUST));

                p_db_aclqos_if->replace_cos = 1;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_REPLACE_COS));
                GLB_SET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS));
            }
        }
        
    }
    else
    {
        p_default_domain = p_domain_master->default_domain;
        /* 2. create domain 0-7 */
        sal_memcpy(&domain, p_default_domain, sizeof(domain));
        for (i = 0; i < GLB_QOS_DOMAIN_NUM; i++)
        {
            domain.key.id = i;
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_COS2COLOR));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_COS2TC));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_DSCP2COLOR));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_DSCP2TC));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_TC_COLOR2COS));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP));

            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_EXP2COLOR));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_EXP2TC));
            PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP));

        }
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_aclqos_if, listnode)
        {
            if (GLB_IF_TYPE_VLAN_IF == p_db_aclqos_if->port_type)
            {
                //vlan if aclqos just care dscp_domain and trust field
                p_db_aclqos_if->dscp_domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DSCP_DOMAIN));

                p_db_aclqos_if->trust = GLB_QOS_TRUST_PORT;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_TRUST));
            }
            else
            {
                p_db_aclqos_if->domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DOMAIN));
                GLB_UNSET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN));

                p_db_aclqos_if->cos_domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_COS_DOMAIN));
                p_db_aclqos_if->dscp_domain = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DSCP_DOMAIN));

                if (GLB_FLAG_ISSET(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER))
                {
                    PM_E_RETURN(qos_mgr_policer_res_del(p_db_aclqos_if->input_policer, TRUE, TRUE));
                }
                sal_memset(p_db_aclqos_if->input_policer, 0x0, POLICER_NAME_SIZE);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_INPUT_POLICER));
                GLB_UNSET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER));

                if (GLB_FLAG_ISSET(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER))
                {
                    PM_E_RETURN(qos_mgr_policer_res_del(p_db_aclqos_if->output_policer, FALSE, TRUE));
                }
                sal_memset(p_db_aclqos_if->output_policer, 0x0, POLICER_NAME_SIZE);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_OUTPUT_POLICER));
                GLB_UNSET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER));
                
                sal_memcpy(p_db_aclqos_if->port_shape_profile, GLB_QOS_DEF_PORT_SHAPE_PROFILE, QOS_NAME_SIZE);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE));
                GLB_UNSET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE));

                /* BEGIN: clear replace cos and dscp , add by wangqj 2016/07/29 */
                /* clear replace cos */
                p_db_aclqos_if->replace_cos = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_REPLACE_COS));
                GLB_UNSET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS));
                
                /* clear replace dscp */
                p_db_aclqos_if->replace_dscp = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_REPLACE_DSCP));
                GLB_UNSET_FLAG(p_db_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP);
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP));
                /* END: clear replace cos and dscp , add by wangqj 2016/07/29 */

                p_db_aclqos_if->trust = GLB_QOS_TRUST_PORT;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_TRUST));

                p_db_aclqos_if->port_tc = 0;
                PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_PORT_TC));
                
                for(queue_id = 0; queue_id < GLB_QOS_PORT_QUEUE_NUM; queue_id ++)
                {
                    sal_memcpy(&p_db_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE, sizeof(qos_name_t));
                    PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE));
                    p_db_aclqos_if->queue_drop_mode[queue_id] = GLB_QOS_QUEUE_DROP_MODE_WTD;
                    PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE));
                    sal_memcpy(&p_db_aclqos_if->queue_shape_profile[queue_id], GLB_QOS_DEF_SCHEDULER_PROFILE, sizeof(qos_name_t));
                    PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE));
                }
            }
            
        }
        CTCLIB_SLIST_LOOP(p_class_action_mater->class_map_action_config_list, p_db_class_action_config, listnode_action)
        {
            if(0 != sal_strlen(p_db_class_action_config->policer))
            {
                class_map_action_update(p_db_class_action_config->key.policy_name, p_db_class_action_config->key.class_name, TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER,
                                   &value, NULL);
            }
        }

        /* 1. deinit default domain */
        PM_E_RETURN(_qos_mgr_deinit_default_domain(NULL));

        /* del qos mpls domain */
        PM_E_RETURN(_qos_mgr_del_qos_mpls_domain());

        p_queue_shape_master = tbl_qos_queue_shape_profile_get_master();
        for (i = 0; i < GLB_QOS_SHAPE_PROFILE_NUM; i++)
        {
            p_db_qos_queue_shape_profile = p_queue_shape_master->qos_queue_shape_profile_array[i];
            if ((NULL == p_db_qos_queue_shape_profile) || (0 == sal_strcmp(p_db_qos_queue_shape_profile->name, GLB_QOS_DEF_SCHEDULER_PROFILE)))
            {
                continue;
            }

            tbl_qos_queue_shape_profile_del_qos_queue_shape_profile(&p_db_qos_queue_shape_profile->key);
        }

        p_port_shape_master = tbl_qos_port_shape_profile_get_master();
        for (i = 0; i < GLB_QOS_PORT_SHAPE_PROFILE_NUM; i++)
        {
            p_db_qos_port_shape_profile = p_port_shape_master->qos_port_shape_profile_array[i];
            if ((NULL == p_db_qos_port_shape_profile) || (0 == sal_strcmp(p_db_qos_port_shape_profile->name, GLB_QOS_DEF_PORT_SHAPE_PROFILE)))
            {
                continue;
            }

            tbl_qos_port_shape_profile_del_qos_port_shape_profile(&p_db_qos_port_shape_profile->key);
        }

        p_policer_master = tbl_qos_policer_profile_get_master();
        for (i = 0; i < GLB_QOS_POLICER_PROFILE_NUM; i++)
        {
            p_db_qos_policer_profile = p_policer_master->qos_policer_profile_array[i];
            if (NULL == p_db_qos_policer_profile)
            {
                continue;
            }
            tbl_qos_policer_profile_del_qos_policer_profile(&p_db_qos_policer_profile->key);
        }
        
        p_action_master = tbl_qos_policer_action_profile_get_master();
        for (i = 0; i < GLB_QOS_POLICER_ACTION_PROFILE_NUM; i++)
        {
            p_db_qos_action_profile = p_action_master->qos_policer_action_profile_array[i];
            if (NULL == p_db_qos_action_profile)
            {
                continue;
            }
            tbl_qos_policer_action_profile_del_qos_policer_action_profile(&p_db_qos_action_profile->key);
        }
        
        p_drop_master = tbl_qos_drop_profile_get_master();
        for (i = 0; i < GLB_QOS_DROP_PROFILE_NUM; i++)
        {
            p_db_qos_drop_profile = p_drop_master->qos_drop_profile_array[i];
            if ((NULL == p_db_qos_drop_profile) || (0 == sal_strcmp(p_db_qos_drop_profile->name, GLB_QOS_DEF_WTD_PROFILE))
                    || (0 == sal_strcmp(p_db_qos_drop_profile->name, GLB_QOS_DEF_WRED_PROFILE)))
            {
                continue;
            }
            tbl_qos_drop_profile_del_qos_drop_profile(&p_db_qos_drop_profile->key);
        }
    }

    return rc;

}

int32
qos_mgr_map_queue_shape(tbl_qos_queue_shape_profile_t* p_db_qos_queue_shape_profile, glb_qos_queue_shape_t* p_queue_shape)
{
    p_queue_shape->cir = p_db_qos_queue_shape_profile->cir * (1000/8); 
    if (GLB_FLAG_ISSET(p_db_qos_queue_shape_profile->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR))
    {
        p_queue_shape->pir = p_db_qos_queue_shape_profile->pir * (1000/8); 
    }
    else
    {
        p_queue_shape->pir = p_db_qos_queue_shape_profile->cir * (1000/8); 
    }
    // TODO:deal mtu later 
    p_queue_shape->cbs = 9600; 
    p_queue_shape->pbs = 9600; 
    
    return PM_E_NONE;
}

/*
    op_type:    glb_qos_update_handle_op_type_t
    id:         domain id
    ifindex:    interface index
    dir :       glb_direction_t    
    map_type:   glb_qos_pri_color_map_type_t
*/
int32
qos_mgr_port_queue_update_handle(int32 op_type, int32 ifindex, int32 qid, int32 dir, int32 type)
{
    int32 rc = PM_E_NONE;
    char *name = NULL;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    tbl_qos_queue_shape_profile_t *p_db_qos_queue_shape_profile = NULL;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;
    tbl_interface_t *p_db_if = tbl_interface_get_interface_by_ifindex(ifindex); 
    glb_qos_queue_drop_t queue_drop;
    glb_qos_queue_shape_t queue_shape;
    glb_qos_policer_t policer;
//    glb_qos_port_shape_t port_shape;
    switch (op_type)
    {
        case GLB_QOS_UPDATE_OP_ATTACH:
            break;

        case GLB_QOS_UPDATE_OP_DEATTACH:
            break;

        case GLB_QOS_UPDATE_OP_SET:
            switch (type)
            {
                case GLB_QOS_PORT_QUEUE_UPDATE_POLICER:
                    name = (dir == GLB_INGRESS) ? p_db_if->aclqos_if->input_policer :
                        p_db_if->aclqos_if->output_policer;
                    if (name[0] == '\0')
                    {
                        /* unset port policer*/
                    }
                    else
                    {
                        p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(name); 
                        qos_mgr_map_policer(p_db_qos_policer_profile, &policer);
                    }
                    break;
                
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_SHAPE:
                    name = p_db_if->aclqos_if->queue_shape_profile[qid];
                    if (name[0] == '\0')
                    {
                        /* unset queue shape*/
                    }
                    else
                    {
                        p_db_qos_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(name); 
                        qos_mgr_map_queue_shape(p_db_qos_queue_shape_profile, &queue_shape);
                    }
                    break;
                
                case GLB_QOS_PORT_QUEUE_UPDATE_DEFCOS:
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_DOMAIN:
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_TRUST:
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_REPLACE:
                    switch (p_db_if->aclqos_if->replace)
                    {
                        case GLB_QOS_REPLACE_NONE:
                            break;
                        case GLB_QOS_REPLACE_COS:
                        case GLB_QOS_REPLACE_STAG_COS:
                            break;
                        case GLB_QOS_REPLACE_DSCP_EXP:
                            break;
                        case GLB_QOS_REPLACE_CTAG_COS:
                            break;
                        default:
                            break;
                    }
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_SHAPE:
                    if (p_db_if->aclqos_if->shape_pir == GLB_QOS_SHAPE_DEFAULT_RATE)
                    {
                    }
                    else
                    {
//                        port_shape.pir = p_db_if->aclqos_if->shape_pir * (1000 / 8);
//                        port_shape.pbs = 9600;
                    }
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_ATTACH_PMAP:
                    /* if aclqos_if not created, return */
                    if (!p_db_if->aclqos_if)
                    {
                        return rc;
                    }
                    name = (GLB_INGRESS == dir) ? 
                        p_db_if->aclqos_if->input_policy_map :
                        p_db_if->aclqos_if->output_policy_map;
                    /* policy-map name is not specified ,return */ 
                    if ('\0' == name[0])
                    {
                        return rc;
                    }

                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_DEATTACH_PMAP:
                    /* if aclqos_if not created, return */
                    if (!p_db_if->aclqos_if)
                    {
                        return rc;
                    }
                    name = (GLB_INGRESS == dir) ? 
                        p_db_if->aclqos_if->input_policy_map :
                        p_db_if->aclqos_if->output_policy_map;
                    /* policy-map name is not specified ,return */ 
                    if ('\0' == name[0])
                    {
                        return rc;
                    }
                    /* policy-map deattached to last interface */
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_STATS_EN:
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_DROP:
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_DROP_MODE:
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_ECN:
                    name = p_db_if->aclqos_if->queue_drop_profile[qid];
                    if (name[0] == '\0')
                    {
                        /* unset drop shape: using default profile */
                        p_db_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(GLB_QOS_DEF_WTD_PROFILE);

                    }
                    else
                    {
                        p_db_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(name);
                    }
                    sal_memset(&queue_drop, 0, sizeof(queue_drop));
                    qos_mgr_map_queue_drop(p_db_qos_drop_profile, &queue_drop);
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_CLASS:
                    break;
                case GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_WEIGHT:
                    break;
                default:
                    break;
            }
            break;

        case GLB_QOS_UPDATE_OP_UNSET:
            break;

        default:
                break;
    }
    return rc; 
}


int32
qos_mgr_process_get_qos_queuemap(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;
    int32 i = 0;
    char *color_str[] = 
    {
        "none",
        "red",
        "yellow",
        "green"
    };
    

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");        
        if (NULL == fp)
        {
            CFG_FUNC_ERR_RET("Open file %s failed", para->p_show->path);
        }
        if (0 == sal_strcmp(argv[0], "pri-queue"))
        {
            if (!tbl_qos_global_get_phb_enable())
            {
                /* %-5s %-s */
                sal_fprintf(fp, "%-8s %-s\n", "Priority", "QueueId");
                sal_fprintf(fp, "--------+---\n");
                sal_fprintf(fp, "%-8s %-s\n", "0-7", "0");
                sal_fprintf(fp, "%-8s %-s\n", "8-15", "1");
                sal_fprintf(fp, "%-8s %-s\n", "16-23", "2");
                sal_fprintf(fp, "%-8s %-s\n", "24-31", "3");
                sal_fprintf(fp, "%-8s %-s\n", "32-39", "4");
                sal_fprintf(fp, "%-8s %-s\n", "40-47", "5");
                sal_fprintf(fp, "%-8s %-s\n", "48-55", "6");
                sal_fprintf(fp, "%-8s %-s\n", "56-63", "7");
            }
            sal_fclose(fp);
        }
        else if (0 == sal_strcmp(argv[0], "phb-queue-color"))
        {
            if (tbl_qos_global_get_phb_enable())
            {
                /* %-5s %-4s %-s*/
                sal_fprintf(fp, "%-8s %-7s %-s\n", "PHB-Name", "QueueId", "Color");
                sal_fprintf(fp, "--------+-------+-----\n");
                for (i = 0; i < GLB_QOS_PHB_MAX; i++)
                {
                    sal_fprintf(fp, "%-8s %-7d %-s\n", qos_phb_property[i].name, qos_phb_property[i].qid, color_str[qos_phb_property[i].color]);
                }
            }
            sal_fclose(fp);
        }
        else
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Invalid parameter: %s", argv[0]);
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}
    int32
qos_mgr_stop(switch_master_t *master)
{
    tbl_qos_global_t* p_db_qos_global = NULL;
    
    cdb_register_cfg_tbl_cb(TBL_QOS_DOMAIN, NULL);
    cdb_register_cfg_tbl_cb(TBL_QOS_POLICER_PROFILE, NULL);
    cdb_register_cfg_tbl_cb(TBL_QOS_DROP_PROFILE, NULL);
    cdb_register_cfg_tbl_cb(TBL_QOS_QUEUE_SHAPE_PROFILE, NULL);
    cdb_register_cfg_tbl_cb(TBL_QOS_GLOBAL, NULL);
    
    cdb_register_cfg_act_cb(ACT_GET_QOS_QUEUEMAP, NULL);

    sal_memset(p_db_qos_global, 0, sizeof(tbl_qos_global_t));
    return PM_E_NONE;
}

int32
qos_mgr_init(void)
{
    /* 3. alloc default queue drop profile */
    tbl_qos_drop_profile_t drop_profile;
    tbl_qos_global_t qos_glb;
    int32 rc = PM_E_NONE;
    uint32 id = 0;

    sal_memset(&drop_profile, 0, sizeof(drop_profile));
    drop_profile.key.id = 0;
    sal_strcpy(drop_profile.name, GLB_QOS_DEF_WTD_PROFILE);
    
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
    PM_E_RETURN(tbl_qos_drop_profile_add_qos_drop_profile(&drop_profile));

    /*wred default profile*/
    sal_memset(&drop_profile, 0, sizeof(drop_profile));
    drop_profile.key.id = 1;
    sal_strcpy(drop_profile.name, GLB_QOS_DEF_WRED_PROFILE);
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
    PM_E_RETURN(tbl_qos_drop_profile_add_qos_drop_profile(&drop_profile));


    tbl_qos_queue_shape_profile_t qos_queue_shape_profile;
    sal_memset(&qos_queue_shape_profile, 0x0, sizeof(tbl_qos_queue_shape_profile_t));
    /*disable shape*/
    sal_strcpy(qos_queue_shape_profile.name, GLB_QOS_DEF_SCHEDULER_PROFILE);
    tbl_qos_queue_shape_profile_alloc_profile_id(&id);
    qos_queue_shape_profile.cir = 0;
    qos_queue_shape_profile.pir = GLB_QOS_DEF_SHAPING_PIR;
    qos_queue_shape_profile.key.id = id;
    qos_queue_shape_profile.mode = SCHEDULER_MODE_SP;
    qos_queue_shape_profile.queue_weight = 1;
    GLB_SET_FLAG(qos_queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR);
    GLB_SET_FLAG(qos_queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR);
    GLB_SET_FLAG(qos_queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE);
    GLB_SET_FLAG(qos_queue_shape_profile.flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT);
    PM_E_RETURN(tbl_qos_queue_shape_profile_add_qos_queue_shape_profile(&qos_queue_shape_profile));


    tbl_qos_port_shape_profile_t qos_port_shape_profile;
    sal_memset(&qos_port_shape_profile, 0x0, sizeof(tbl_qos_port_shape_profile_t));
    /*disable shape*/
    sal_strcpy(qos_port_shape_profile.name, GLB_QOS_DEF_PORT_SHAPE_PROFILE);
    tbl_qos_port_shape_profile_alloc_profile_id(&id);
    qos_port_shape_profile.pir = GLB_QOS_DEF_SHAPING_PIR;
    GLB_SET_FLAG(qos_port_shape_profile.flags, GLB_PORT_SHAPE_PROFILE_FLAGS_PIR);
    PM_E_RETURN(tbl_qos_port_shape_profile_add_qos_port_shape_profile(&qos_port_shape_profile));

    tbl_qos_domain_t domain;
    tbl_qos_domain_t *p_default_domain = NULL;
    int i = 0;
    sal_memset(&domain, 0x0, sizeof(tbl_qos_domain_t));

    PM_E_RETURN(_qos_mgr_init_default_domain(&p_default_domain));
    /* create domain 0-7 */
    sal_memcpy(&domain, p_default_domain, sizeof(domain));

    for (i = 0; i < GLB_QOS_DOMAIN_NUM; i++)
    {
        domain.key.id = i;
        tbl_qos_domain_add_qos_domain(&domain);
    }

#if 0
    domain.key.id = 0;
    PM_E_RETURN(tbl_qos_domain_add_qos_domain(&domain));
    int i = 0;
    int j = 0;
    /* ingress */
    for (i = 0; i < GLB_QOS_COS_NUM; i++)
    {
        domain.cos2pri[i] = i << 3;
        domain.cos2color[i] = GLB_QOS_COLOR_GREEN;
        domain.cos2tc[i] = i;
    }
    for (i = 0; i < GLB_QOS_DSCP_NUM; i++)
    {
        domain.dscp2pri[i] = i;
        domain.dscp2color[i] = GLB_QOS_COLOR_GREEN;
        domain.dscp2tc[i] = i/8;
    }
    for (i = 0; i < GLB_QOS_EXP_NUM; i++)
    {
        domain.exp2pri[i] = i << 3;
        domain.exp2color[i] = GLB_QOS_COLOR_GREEN;
        domain.exp2tc[i] = i;
    }
    /* egress */
    for (i = 0; i < GLB_QOS_PRIORITY_NUM; i++)
    {
        for (j = GLB_QOS_COLOR_NONE; j < GLB_QOS_COLOR_MAX; j++)
        {
            domain.pri_color2cos[(i << 2) + j] = i >> 3;
            domain.pri_color2dscp[(i << 2) + j] = i;
            domain.pri_color2exp[(i << 2) + j] = i >> 3;
        }
    }
    for(i = 0; i < GLB_QOS_TC_NUM; i++)
    {
        for(j = GLB_QOS_COLOR_NONE; j < GLB_QOS_COLOR_MAX; j++)
        {
            domain.tc_color2cos[(i << 2) + j] = i;
            domain.tc_color2exp[(i << 2) + j] = i;  
            domain.tc_color2dscp[(i<<2) + j] = i*8;
        }
    }
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_COS2COLOR));
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_COS2TC));
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_DSCP2COLOR));
    
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_DSCP2TC));
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_TC_COLOR2COS));
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP));
    
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_EXP2COLOR));
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_EXP2TC));
    PM_E_RETURN(tbl_qos_domain_set_qos_domain_field(&domain, TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP));
#endif

    /* by defalut, qos enable */
    //qos_glb.qos_enable = FALSE;
    qos_glb.qos_enable = TRUE;
    qos_glb.port_policer_first_enable = FALSE;
    qos_glb.qos_ipg_enable = TRUE;
    tbl_qos_global_set_qos_global_field(&qos_glb, TBL_QOS_GLOBAL_FLD_QOS_ENABLE);
    tbl_qos_global_set_qos_global_field(&qos_glb, TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE);
    tbl_qos_global_set_qos_global_field(&qos_glb, TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE);

    
    tbl_aclqos_if_master_t *p_master = tbl_aclqos_if_get_master();
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    uint32 queue_id = 0;
    ctclib_slistnode_t *listnode = NULL;
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_aclqos_if, listnode)
    {
        if (GLB_IF_TYPE_VLAN_IF == p_db_aclqos_if->port_type)
        {
            p_db_aclqos_if->dscp_domain = 0;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DSCP_DOMAIN));

            /* by defalut, qos enable */
            //p_db_aclqos_if->trust = GLB_QOS_TRUST_PORT;
            p_db_aclqos_if->trust = GLB_QOS_TRUST_COS;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_TRUST));
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_TRUST));
        }
        else
        {
            for(queue_id = 0; queue_id < GLB_QOS_PORT_QUEUE_NUM; queue_id ++)
            {
                 sal_strcpy(p_db_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE);
                 PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE));
                 
                 p_db_aclqos_if->queue_drop_mode[queue_id] = GLB_QOS_QUEUE_DROP_MODE_WTD;
                 PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE));

                 sal_strcpy(p_db_aclqos_if->queue_shape_profile[queue_id], GLB_QOS_DEF_SCHEDULER_PROFILE);
                 PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE));
            }
            sal_strcpy(p_db_aclqos_if->port_shape_profile, GLB_QOS_DEF_PORT_SHAPE_PROFILE);
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE));
            
            p_db_aclqos_if->domain = 0;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DOMAIN));
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN));

            p_db_aclqos_if->cos_domain = 0;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_COS_DOMAIN));
            p_db_aclqos_if->dscp_domain = 0;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_DSCP_DOMAIN));

            /* by defalut, qos enable */
            //p_db_aclqos_if->trust = GLB_QOS_TRUST_PORT;
            p_db_aclqos_if->trust = GLB_QOS_TRUST_COS;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_TRUST));
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_FLAGS_TRUST));

            /* by defalut, qos enable */
            //p_db_aclqos_if->replace_cos = 0;
            p_db_aclqos_if->replace_cos = 1;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_REPLACE_COS));

            p_db_aclqos_if->replace_dscp = 0;
            PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_db_aclqos_if, TBL_ACLQOS_IF_FLD_REPLACE_DSCP));
        }

    }

#ifndef GREATBELT
    i = 0;
    tbl_qos_queue_smart_buffer_t queue_smart_buffer;
    for (i = 0; i < GLB_QOS_QUEUE_SMART_BUFFER_NUM; i++)
    {
        sal_memset(&queue_smart_buffer, 0x0, sizeof(queue_smart_buffer));
        queue_smart_buffer.key.queue_id = i;
        queue_smart_buffer.smart_buffer_enable = TRUE;
        PM_E_RETURN(tbl_qos_queue_smart_buffer_add_qos_queue_smart_buffer(&queue_smart_buffer));
    }
#endif

    return rc;
}


int32
qos_mgr_start(switch_master_t *master)
{
    
    /* 1. register tbl callback */
    cdb_register_cfg_tbl_cb(TBL_QOS_DOMAIN, qos_mgr_cmd_process_domain);
    cdb_register_cfg_tbl_cb(TBL_QOS_MPLS_DOMAIN, qos_mgr_cmd_process_mpls_domain);
    cdb_register_cfg_tbl_cb(TBL_QOS_POLICER_RES, qos_mgr_cmd_process_policer_res);
    cdb_register_cfg_tbl_cb(TBL_QOS_POLICER_ACTION_PROFILE, qos_mgr_cmd_process_policer_action_profile);
    cdb_register_cfg_tbl_cb(TBL_QOS_POLICER_PROFILE, qos_mgr_cmd_process_policer_profile);
    cdb_register_cfg_tbl_cb(TBL_QOS_DROP_PROFILE, qos_mgr_cmd_process_drop_profile);
    cdb_register_cfg_tbl_cb(TBL_QOS_QUEUE_SHAPE_PROFILE, qos_mgr_cmd_process_queue_shape_profile);
    cdb_register_cfg_tbl_cb(TBL_QOS_PORT_SHAPE_PROFILE, qos_mgr_cmd_process_port_shape_profile);
    cdb_register_cfg_tbl_cb(TBL_QOS_QUEUE_SMART_BUFFER, qos_mgr_cmd_process_queue_smart_buffer);
    cdb_register_cfg_tbl_cb(TBL_QOS_GLOBAL, qos_mgr_cmd_process_global);
    cdb_register_cfg_act_cb(ACT_SHOW_QUEUE_DROP, qos_mgr_cmd_process_show_queue_drop);

    cdb_register_cfg_act_cb(ACT_GET_QOS_QUEUEMAP, qos_mgr_process_get_qos_queuemap);

    return PM_E_NONE;
}

int32
qos_mgr_set_glb_buffer_event_enable(tbl_qos_global_t  *p_glb, uint32 enable)
{
    int32 rc = PM_E_NONE;
    /* 1. feature process */   

    /* 2. update local CDB */ 
    p_glb->buffer_event_enable = enable;
    PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_glb , TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE));
    return rc;
}

int32
qos_mgr_set_glb_buffer_event_threshold(tbl_qos_global_t *p_glb, uint32 low_thrd , uint32 high_thrd)
{
    int32 rc = PM_E_NONE;
    /* 1. feature process */   

    /* 2. update local CDB */ 
    p_glb->buffer_event_low = low_thrd;
    PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_glb, TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW));

    p_glb->buffer_event_high= high_thrd;
    PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_glb, TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH));

    return rc;
}

int32
qos_mgr_set_glb_monitor_latency_range(tbl_qos_global_t *p_glb,  uint32 latency_range[GLB_MONITOR_LATENCY_RANGE_ID_MAX])
{
    int32 rc = PM_E_NONE;
    /* 1. feature process */  

    /* 2. update local CDB */ 
    p_glb->latency_range[0] = latency_range[0];
    p_glb->latency_range[1] = latency_range[1];
    p_glb->latency_range[2] = latency_range[2];
    p_glb->latency_range[3] = latency_range[3];
    p_glb->latency_range[4] = latency_range[4];
    p_glb->latency_range[5] = latency_range[5];
    p_glb->latency_range[6] = latency_range[6];
    p_glb->latency_range[7] = latency_range[7];

    PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_glb, TBL_QOS_GLOBAL_FLD_LATENCY_RANGE));

    return rc;
}

int32
qos_monitor_init(void)
{
    tbl_qos_global_t p_glb;
    int32 rc = PM_E_NONE;

    sal_memset(&p_glb, 0, sizeof(p_glb));

    p_glb.buffer_event_low = GLB_BUFFER_EVENT_LOW_THRESHOLD_DEFAULT;
    p_glb.buffer_event_high = GLB_BUFFER_EVENT_HIGH_THRESHOLD_DEFAULT;
    tbl_qos_global_set_qos_global_field(&p_glb, TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW);
    tbl_qos_global_set_qos_global_field(&p_glb, TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH);

    p_glb.latency_range[0] = GLB_MONITOR_LATENCY_RANGE_0_DEF_VALUE;
    p_glb.latency_range[1] = GLB_MONITOR_LATENCY_RANGE_1_DEF_VALUE;
    p_glb.latency_range[2] = GLB_MONITOR_LATENCY_RANGE_2_DEF_VALUE;
    p_glb.latency_range[3] = GLB_MONITOR_LATENCY_RANGE_3_DEF_VALUE;
    p_glb.latency_range[4] = GLB_MONITOR_LATENCY_RANGE_4_DEF_VALUE;
    p_glb.latency_range[5] = GLB_MONITOR_LATENCY_RANGE_5_DEF_VALUE;
    p_glb.latency_range[6] = GLB_MONITOR_LATENCY_RANGE_6_DEF_VALUE;
    p_glb.latency_range[7] = GLB_MONITOR_LATENCY_RANGE_7_DEF_VALUE;

#ifdef TSINGMA
    int index = 1;
    for (; index < 7; ++index)
    {
        p_glb.latency_range[index] -= 300;
    }
#endif
    
    tbl_qos_global_set_qos_global_field(&p_glb, TBL_QOS_GLOBAL_FLD_LATENCY_RANGE);

    return rc;

}

int32
qos_mgr_policer_res_check(char *policer_name, bool direct_in, bool port_policer, uint32 *count)
{
    tbl_qos_policer_profile_t *p_db_policer = NULL;
    tbl_qos_policer_res_t *p_db_policer_res = NULL;
    tbl_qos_policer_res_key_t key;
    tbl_qos_global_t *p_qos_global = tbl_qos_global_get_qos_global();
        
    sal_memset(&key, 0x0, sizeof(tbl_qos_policer_res_key_t));

    if (!count)
    {
        return PM_E_INVALID_PARAM;
    }

    p_db_policer = tbl_qos_policer_profile_get_profile_by_name(policer_name);
    if (!p_db_policer)
    {
        return PM_E_NOT_EXIST;
    }

    key.cir = p_db_policer->cir;
    key.cbs = p_db_policer->cbs;
    key.eir = p_db_policer->eir_or_pir;
    key.ebs = p_db_policer->ebs_or_pbs;
    key.mode = p_db_policer->algorithm_mode;
    key.pps_en = p_db_policer->is_pps;
    key.direct = direct_in? 0 : 1 ;
    key.type = port_policer? 0 : 1;

    p_db_policer_res = tbl_qos_policer_res_get_qos_policer_res(&key);
    if (p_db_policer_res)
    {
        return PM_E_NONE;
    }

    if (direct_in)
    {
        if (port_policer)
        {
            if (p_qos_global->in_port_policer_res_cnt + *count + 1 > GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
        else
        {
            if (p_qos_global->in_flow_policer_res_cnt + *count + 1 > GLB_QOS_INGERSS_FLOW_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
    }
    else
    {
        if (port_policer)
        {
            if (p_qos_global->eg_port_policer_res_cnt + *count + 1 > GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
        else
        {
            if (p_qos_global->eg_flow_policer_res_cnt + *count + 1 > GLB_QOS_EGERSS_FLOW_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
    }

    (*count)++;

    return PM_E_NONE;
}

int32
qos_mgr_policer_res_update_check(tbl_qos_policer_profile_t *policer_new, tbl_qos_policer_profile_t *policer_old, bool direct_in, bool port_policer)
{
    tbl_qos_policer_res_t *p_db_policer_res_old = NULL;
    tbl_qos_policer_res_t *p_db_policer_res_new = NULL;
    tbl_qos_policer_res_key_t old_key;
    tbl_qos_policer_res_key_t new_key;
    tbl_qos_global_t *p_qos_global = tbl_qos_global_get_qos_global();
    
    sal_memset(&old_key, 0x0, sizeof(tbl_qos_policer_res_key_t));
    sal_memset(&new_key, 0x0, sizeof(tbl_qos_policer_res_key_t));

    if (!policer_new || !policer_old)
    {
        return PM_E_NONE;
    }

    old_key.cir = policer_old->cir;
    old_key.cbs = policer_old->cbs;
    old_key.eir = policer_old->eir_or_pir;
    old_key.ebs = policer_old->ebs_or_pbs;
    old_key.mode = policer_old->algorithm_mode;
    old_key.pps_en = policer_old->is_pps;
    old_key.direct = direct_in? 0 : 1 ;
    old_key.type = port_policer? 0 : 1;

    new_key.cir = policer_new->cir;
    new_key.cbs = policer_new->cbs;
    new_key.eir = policer_new->eir_or_pir;
    new_key.ebs = policer_new->ebs_or_pbs;
    new_key.mode = policer_new->algorithm_mode;
    new_key.pps_en = policer_new->is_pps;
    new_key.direct = direct_in? 0 : 1 ;
    new_key.type = port_policer? 0 : 1;

    if (0 == sal_memcmp(&old_key, &new_key, sizeof(tbl_qos_policer_res_key_t)))
    {
        return PM_E_NONE;
    }
    
    p_db_policer_res_old = tbl_qos_policer_res_get_qos_policer_res(&old_key);
    p_db_policer_res_new = tbl_qos_policer_res_get_qos_policer_res(&new_key);
    if (p_db_policer_res_old && (1==p_db_policer_res_old->ref_cnt) && (NULL==p_db_policer_res_new))
    {
        //delete old and add new
        return PM_E_NONE;
    }
    else if (p_db_policer_res_old && (1==p_db_policer_res_old->ref_cnt) && (NULL!=p_db_policer_res_new))
    {
        //delete old and add new ref_cnt
        return PM_E_NONE;
    }
    else if (p_db_policer_res_old && (1<p_db_policer_res_old->ref_cnt) && (NULL==p_db_policer_res_new))
    {
        //delete old ref_cnt and add new
    }
    else if (p_db_policer_res_old && (1<p_db_policer_res_old->ref_cnt) && (NULL!=p_db_policer_res_new))
    {
        //delete old ref_cnt and add new ref_cnt
        return PM_E_NONE;
    }
    else
    {
        if (NULL != p_db_policer_res_new)
        {
            return PM_E_NONE;
        }
        else
        {
            
        }
    }


    if (direct_in)
    {
        if (port_policer)
        {
            if (p_qos_global->in_port_policer_res_cnt + 1 > GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
        else
        {
            if (p_qos_global->in_flow_policer_res_cnt + 1 > GLB_QOS_INGERSS_FLOW_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
    }
    else
    {
        if (port_policer)
        {
            if (p_qos_global->eg_port_policer_res_cnt + 1 > GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
        else
        {
            if (p_qos_global->eg_flow_policer_res_cnt  + 1 > GLB_QOS_EGERSS_FLOW_POLICER_PROFILE_NUM)
            {
                return PM_E_CFG_POLICER_NO_RESOURCE;
            }
        }
    }

    return PM_E_NONE;
}


int32
qos_mgr_policer_res_update(tbl_qos_policer_profile_t *policer_new, tbl_qos_policer_profile_t *policer_old, bool direct_in, bool port_policer)
{
    tbl_qos_policer_res_t *p_db_policer_res_old = NULL;
    tbl_qos_policer_res_t *p_db_policer_res_new = NULL;
    tbl_qos_policer_res_t policer_res;
    tbl_qos_policer_res_key_t old_key;
    tbl_qos_policer_res_key_t new_key;
    tbl_qos_global_t *p_qos_global = tbl_qos_global_get_qos_global();
    int32 count = 0;
    int32 rc = PM_E_NONE;
    
    sal_memset(&old_key, 0x0, sizeof(tbl_qos_policer_res_key_t));
    sal_memset(&new_key, 0x0, sizeof(tbl_qos_policer_res_key_t));
    sal_memset(&policer_res, 0x0, sizeof(tbl_qos_policer_res_t));

    if (!policer_new || !policer_old)
    {
        return PM_E_NONE;
    }

    old_key.cir = policer_old->cir;
    old_key.cbs = policer_old->cbs;
    old_key.eir = policer_old->eir_or_pir;
    old_key.ebs = policer_old->ebs_or_pbs;
    old_key.mode = policer_old->algorithm_mode;
    old_key.pps_en = policer_old->is_pps;
    old_key.direct = direct_in? 0 : 1 ;
    old_key.type = port_policer? 0 : 1;

    new_key.cir = policer_new->cir;
    new_key.cbs = policer_new->cbs;
    new_key.eir = policer_new->eir_or_pir;
    new_key.ebs = policer_new->ebs_or_pbs;
    new_key.mode = policer_new->algorithm_mode;
    new_key.pps_en = policer_new->is_pps;
    new_key.direct = direct_in? 0 : 1 ;
    new_key.type = port_policer? 0 : 1;

    if (0 == sal_memcmp(&old_key, &new_key, sizeof(tbl_qos_policer_res_key_t)))
    {
        return PM_E_NONE;
    }
    
    p_db_policer_res_old = tbl_qos_policer_res_get_qos_policer_res(&old_key);
    p_db_policer_res_new = tbl_qos_policer_res_get_qos_policer_res(&new_key);
    if (p_db_policer_res_old && (1==p_db_policer_res_old->ref_cnt) && (NULL==p_db_policer_res_new))
    {
        //delete old and add new
        PM_E_RETURN(tbl_qos_policer_res_del_qos_policer_res(&old_key));
        sal_memcpy(&policer_res.key, &new_key, sizeof(tbl_qos_policer_res_key_t));
        policer_res.ref_cnt ++;
        PM_E_RETURN(tbl_qos_policer_res_add_qos_policer_res(&policer_res));
    }
    else if (p_db_policer_res_old && (1==p_db_policer_res_old->ref_cnt) && (NULL!=p_db_policer_res_new))
    {
        //delete old and add new ref_cnt
        PM_E_RETURN(tbl_qos_policer_res_del_qos_policer_res(&old_key));
        p_db_policer_res_new->ref_cnt ++;
        PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res_new, TBL_QOS_POLICER_RES_FLD_REF_CNT));
        count --;
    }
    else if (p_db_policer_res_old && (1<p_db_policer_res_old->ref_cnt) && (NULL==p_db_policer_res_new))
    {
        //delete old ref_cnt and add new
        p_db_policer_res_old->ref_cnt --;
        PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res_old, TBL_QOS_POLICER_RES_FLD_REF_CNT));
        sal_memcpy(&policer_res.key, &new_key, sizeof(tbl_qos_policer_res_key_t));
        policer_res.ref_cnt ++;
        PM_E_RETURN(tbl_qos_policer_res_add_qos_policer_res(&policer_res));
        count ++;
    }
    else if (p_db_policer_res_old && (1<p_db_policer_res_old->ref_cnt) && (NULL!=p_db_policer_res_new))
    {
        //delete old ref_cnt and add new ref_cnt
        p_db_policer_res_old->ref_cnt --;
        PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res_old, TBL_QOS_POLICER_RES_FLD_REF_CNT));
        p_db_policer_res_new->ref_cnt ++;
        PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res_new, TBL_QOS_POLICER_RES_FLD_REF_CNT));
    }
    else
    {
        if (NULL!=p_db_policer_res_new)
        {
            //add new ref_cnt
            p_db_policer_res_new->ref_cnt ++;
            PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res_new, TBL_QOS_POLICER_RES_FLD_REF_CNT));
        }
        else
        {
            //add new
            sal_memcpy(&policer_res.key, &new_key, sizeof(tbl_qos_policer_res_key_t));
            policer_res.ref_cnt ++;
            PM_E_RETURN(tbl_qos_policer_res_add_qos_policer_res(&policer_res));
            count ++;
        }
    }


    if (direct_in)
    {
        if (port_policer)
        {
            p_qos_global->in_port_policer_res_cnt += count;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT));
        }
        else
        {
            p_qos_global->in_flow_policer_res_cnt += count;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT));
        }
    }
    else
    {
        if (port_policer)
        {
            p_qos_global->eg_port_policer_res_cnt += count;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT));
        }
        else
        {
            p_qos_global->eg_flow_policer_res_cnt += count;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT));
        }
    }

    return rc;
}


int32
qos_mgr_policer_res_add(char *policer_name, bool direct_in, bool port_policer)
{
    tbl_qos_policer_profile_t *p_db_policer = NULL;
    tbl_qos_policer_res_t *p_db_policer_res = NULL;
    tbl_qos_policer_res_t policer_res;
    tbl_qos_policer_res_key_t key;
    tbl_qos_global_t *p_qos_global = tbl_qos_global_get_qos_global();
    int32 rc = PM_E_NONE;
    
    sal_memset(&key, 0x0, sizeof(tbl_qos_policer_res_key_t));
    sal_memset(&policer_res, 0x0, sizeof(tbl_qos_policer_res_t));

    p_db_policer = tbl_qos_policer_profile_get_profile_by_name(policer_name);
    if (!p_db_policer)
    {
        return PM_E_NOT_EXIST;
    }

    key.cir = p_db_policer->cir;
    key.cbs = p_db_policer->cbs;
    key.eir = p_db_policer->eir_or_pir;
    key.ebs = p_db_policer->ebs_or_pbs;
    key.mode = p_db_policer->algorithm_mode;
    key.pps_en = p_db_policer->is_pps;
    key.direct = direct_in? 0 : 1 ;
    key.type = port_policer? 0 : 1;

    p_db_policer_res = tbl_qos_policer_res_get_qos_policer_res(&key);
    if (p_db_policer_res)
    {
        p_db_policer_res->ref_cnt ++;
        PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res, TBL_QOS_POLICER_RES_FLD_REF_CNT));
        return PM_E_NONE;
    }
    else
    {
        sal_memcpy(&policer_res.key, &key, sizeof(tbl_qos_policer_res_key_t));
        policer_res.ref_cnt ++;
        PM_E_RETURN(tbl_qos_policer_res_add_qos_policer_res(&policer_res));
    }

    if (direct_in)
    {
        if (port_policer)
        {
            p_qos_global->in_port_policer_res_cnt ++;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT));
        }
        else
        {
            p_qos_global->in_flow_policer_res_cnt ++;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT));
        }
    }
    else
    {
        if (port_policer)
        {
            p_qos_global->eg_port_policer_res_cnt ++;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT));
        }
        else
        {
            p_qos_global->eg_flow_policer_res_cnt ++;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT));
        }
    }

    return rc;
}

int32
qos_mgr_policer_res_del(char *policer_name, bool direct_in, bool port_policer)
{
    tbl_qos_policer_profile_t *p_db_policer = NULL;
    tbl_qos_policer_res_t *p_db_policer_res = NULL;
    tbl_qos_policer_res_key_t key;
    tbl_qos_global_t *p_qos_global = tbl_qos_global_get_qos_global();
    int32 rc = PM_E_NONE;
        
    sal_memset(&key, 0x0, sizeof(tbl_qos_policer_res_key_t));

    p_db_policer = tbl_qos_policer_profile_get_profile_by_name(policer_name);
    if (!p_db_policer)
    {
        return PM_E_NOT_EXIST;
    }

    key.cir = p_db_policer->cir;
    key.cbs = p_db_policer->cbs;
    key.eir = p_db_policer->eir_or_pir;
    key.ebs = p_db_policer->ebs_or_pbs;
    key.mode = p_db_policer->algorithm_mode;
    key.pps_en = p_db_policer->is_pps;
    key.direct = direct_in? 0 : 1 ;
    key.type = port_policer? 0 : 1;

    p_db_policer_res = tbl_qos_policer_res_get_qos_policer_res(&key);
    if (!p_db_policer_res)
    {
        return PM_E_NONE;
    }
    else
    {
        if (0 == p_db_policer_res->ref_cnt - 1)
        {
            PM_E_RETURN(tbl_qos_policer_res_del_qos_policer_res(&key));
        }
        else
        {
            p_db_policer_res->ref_cnt --;
            PM_E_RETURN(tbl_qos_policer_res_set_qos_policer_res_field(p_db_policer_res, TBL_QOS_POLICER_RES_FLD_REF_CNT));
            return PM_E_NONE;
        }
     }

    if (direct_in)
    {
        if (port_policer)
        {
            p_qos_global->in_port_policer_res_cnt --;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT));
        }
        else
        {
            p_qos_global->in_flow_policer_res_cnt --;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT));
        }
    }
    else
    {
        if (port_policer)
        {
            p_qos_global->eg_port_policer_res_cnt --;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT));
        }
        else
        {
            p_qos_global->eg_flow_policer_res_cnt --;
            PM_E_RETURN(tbl_qos_global_set_qos_global_field(p_qos_global, TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT));
        }
    }

    return rc;
}

/* op=1:update check; op=2:update */
int32
qos_mgr_policer_res_update_op(tbl_qos_policer_profile_t *policer_new, tbl_qos_policer_profile_t *policer_old, uint32 op, bool *flow_resource_full)
{
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    bool direct_in = TRUE;
    tbl_class_map_action_config_master_t *p_action_config_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *action_config_listnode = NULL;
    ctclib_slistnode_t *action_config_next = NULL;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_aclqos_if_master_t *p_aclqos_master = tbl_aclqos_if_get_master();
    int32 rc = PM_E_NONE;
    
    /* flow policer resource */
    CTCLIB_SLIST_LOOP_DEL(p_action_config_master->class_map_action_config_list, p_db_class_map_action_config, action_config_listnode, action_config_next)
    {
        if(0 == sal_strcmp(p_db_class_map_action_config->policer, policer_old->name))
        {
            CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
            {
                if (0 != sal_strlen(p_db_if->igs_policy_map) && 0 == sal_strcmp(p_db_if->igs_policy_map, p_db_class_map_action_config->key.policy_name))
                {
                    direct_in = TRUE;
                    switch(op)
                    {
                    case 1:
                        rc = qos_mgr_policer_res_update_check(policer_new, policer_old, direct_in, FALSE);
                        if (rc)
                        {
                            *flow_resource_full = TRUE;
                            return rc;
                        }
                        break;
                    case 2:
                        PM_E_RETURN(qos_mgr_policer_res_update(policer_new, policer_old, direct_in, FALSE));
                        break;
                    default:
                        break;
                    }
                }

                if (0 != sal_strlen(p_db_if->egs_policy_map) && 0 == sal_strcmp(p_db_if->egs_policy_map, p_db_class_map_action_config->key.policy_name))
                {
                    direct_in = FALSE;
                    switch(op)
                    {
                    case 1:
                        rc = qos_mgr_policer_res_update_check(policer_new, policer_old, direct_in, FALSE);
                        if (rc)
                        {
                            *flow_resource_full = TRUE;
                            return rc;
                        }
                        break;
                    case 2:
                        PM_E_RETURN(qos_mgr_policer_res_update(policer_new, policer_old, direct_in, FALSE));
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    /* port policer resource */
    CTCLIB_SLIST_LOOP(p_aclqos_master->if_list, p_aclqos_if, listnode)
    {
        if (0 == sal_strcmp(p_aclqos_if->input_policer, policer_old->name)) 
        {
            direct_in = TRUE;
            switch(op)
            {
            case 1:
                rc = qos_mgr_policer_res_update_check(policer_new, policer_old, direct_in, TRUE);
                if (rc)
                {
                    *flow_resource_full = FALSE;
                    return rc;
                }
                break;
            case 2:
                PM_E_RETURN(qos_mgr_policer_res_update(policer_new, policer_old, direct_in, TRUE));
                break;
            default:
                break;
            }
        }

        if (0 == sal_strcmp(p_aclqos_if->output_policer, policer_old->name))
        {
            direct_in = FALSE;
            switch(op)
            {
            case 1:
                rc = qos_mgr_policer_res_update_check(policer_new, policer_old, direct_in, TRUE);
                if (rc)
                {
                    *flow_resource_full = FALSE;
                    return rc;
                }
                break;
            case 2:
                PM_E_RETURN(qos_mgr_policer_res_update(policer_new, policer_old, direct_in, TRUE));
                break;
            default:
                break;
            }
        }
    }
    
    return PM_E_NONE;
}

