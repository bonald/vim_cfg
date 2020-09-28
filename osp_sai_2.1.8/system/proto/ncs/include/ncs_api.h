
#ifndef __NCS_API_H__
#define __NCS_API_H__

#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_controller_attr_define.h"
#include "gen/tbl_controller_attr.h"

#define NCS_TUNNEL_NUM 200
#define FIELD_INITIAL_VALUE 5

typedef struct
{
  char id[64];
  char datapath_id[24];
  uint8 lost_connection_behavior; /* 0:failSecureMode; 1:failStandaloneMode */
}ncs_switch_t;

typedef struct 
{
    char id[64];
    uint8 role; /*0:master; 1:slave; 2:equal*/
    char ip_address[64];
    uint16 l4_port;
    char local_ip_address[64];
    uint16 local_l4_port; /*not support*/
    uint8 protocol; /*0:tcp; 1:ssl*/
    uint8 mng_flag; /*0:data port(inband); 1:manage port(outband)*/
}ncs_controller_t;

typedef struct
{
    ctclib_list_node_t node;
    char resource_id[64];   /* use interface external name */
    char ifname[IFNAME_SIZE];   /* use interface internal name */
    uint8 is_tunnel;    /* for tunnel port */
    uint32 tunnel_index; /* for tunnel port */
    char source_ip[64];
    char remote_ip[64];
    uint8 has_used; /* for physical port */
}ncs_interface_t;

typedef struct
{
    ctclib_list_t   if_list;    /**ncs_interface_t */
    uint8 tunnel_bitmap[NCS_TUNNEL_NUM];
}ncs_switch_master_t;


typedef enum
{
    FIELD_OP_GROUP_ID,
    FIELD_OP_GROUP_MODE,
    FIELD_OP_G8131_MODE,
    FIELD_OP_G8131_TIMER_HOLD_OFF,
    FIELD_OP_G8131_TIMER_WAIT_TO_RESTORE,
    APS_GROUP_FIELD_MAX
    
}ncs_aps_group_field_op_t;

typedef struct
{
    uint8 field_op[APS_GROUP_FIELD_MAX];/*0, 1, 2 ===>  create, modify delete*/
    uint32 group_id;
    uint8 group_mode; /*0:pw_mode; 1:lsp_mode*/
    uint8 g8131_mode;/*0:revertive; 1:non-revertive*/
    uint32 g8131_timer_hold_off;/*0..100, unit is 100ms*/
    uint32 g8131_timer_wait_to_restore;/*0..12, unit is minutes*/
}ncs_aps_group_t;

typedef enum
{
    FIELD_OP_SESSION_ID,
    FIELD_OP_SESSION_DESCRIPTION,
    FIELD_OP_MEGID,
    FIELD_OP_MEPID,
    FIELD_OP_CC_INTERVAL,
    FIELD_OP_RMEPID,
    FIELD_OP_CC_CONFIG,
    FIELD_OP_DM_CONFIG,
    FIELD_OP_DM_INTERVAL,
    FIELD_OP_DM_THRESHOLD,
    FIELD_OP_LM_CONFIG,
    FIELD_OP_LM_LOCAL_THRESHOLD,
    FIELD_OP_LM_REMOTE_THRESHOLD,
    OAM_SESSION_FIELD_MAX
    
}ncs_oam_session_field_op_t;

typedef struct
{
    uint8 field_op[OAM_SESSION_FIELD_MAX];/*0, 1, 2 ===>  create, modify delete*/
    uint32 session_id;
    char session_description[32];
    char megid[32];
    uint32 mepid;
    uint8 cc_interval;/*1:3.3ms; 2:10ms; 3:100ms; 4:1s; 5:10s; 6:1min; 7:10min*/
    uint32 rmepid;
    uint8 cc_config;/*1:enable; 0:disable*/
    uint8 dm_config;/*1:enable; 0:disable*/
    char dm_interval[32];/*1s:1s; 10s:10s*/
    uint32 dm_threshold;
    uint8 lm_config;/*1:enable; 0:disable*/
    uint32 lm_local_threshold;
    uint32 lm_remote_threshold;
}ncs_oam_session_t;


/* SYS */
int32
ncs_api_set_hostname(cdb_cfg_t cfg, char *hostname);

/* OPENFLOW */

int32
ncs_api_openflow_set_datapath_id(cdb_cfg_t cfg, ncs_switch_t *p_switch);

int32
ncs_api_openflow_add_controller(cdb_cfg_t cfg, ncs_controller_t *p_controller);

int32
ncs_api_openflow_delete_controller(cdb_cfg_t cfg, ncs_controller_t *p_controller);

int32
ncs_api_openflow_add_interface(cdb_cfg_t cfg, ncs_interface_t *p_interface);

int32
ncs_api_openflow_delete_interface(cdb_cfg_t cfg, ncs_interface_t *p_interface);

int32
ncs_api_openflow_interface_set_enable(cdb_cfg_t cfg, ncs_interface_t *p_interface);

int32
ncs_api_openflow_interface_set_disable(cdb_cfg_t cfg, ncs_interface_t *p_interface);

int32
ncs_api_oam_add_aps_group(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group);

int32
ncs_api_oam_delete_aps_group(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group);

int32
ncs_api_oam_set_g8131_mode(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group);

int32
ncs_api_set_g8131_timer_hold_off(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group);

int32
ncs_api_set_g8131_timer_wtr(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group);

int32
ncs_api_add_sessions(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_delete_sessions(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_session_description(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_megid(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_mepid(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_rmepid(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_cc_config(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_dm_config(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_dm_threshold(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_lm_config(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_lm_local_threshold(cdb_cfg_t cfg, ncs_oam_session_t* p_session);

int32
ncs_api_set_lm_remote_threshold(cdb_cfg_t cfg, ncs_oam_session_t* p_session);



#endif /* !__NCS_API_H__ */

