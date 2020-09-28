#ifndef __CHSM_MGT_H__
#define __CHSM_MGT_H__


#include "fiber_api.h"

#define CHSM_POE_SYS_MIN_BUDGET 15400
#define CHSM_POE_SYS_MAX_BUDGET 739200
#define CHSM_POE_SYS_BUDGET_RESERVE  20
#define CHSM_POE_SYS_BUDGET_THRESHOLD 90
#define CHSM_POE_VMAIN_LOW_THRESHOLD  44
#define CHSM_POE_VMAIN_NORMAL         50
#define CHSM_POE_VMAIN_HIGH_THRESHOLD 57
#define CHSM_POE_MAX_TIME_SEC         4



typedef enum chsm_poe_admin_e
{
    CHSM_POE_ENABLE = 0,
    CHSM_POE_DISABLE,
    CHSM_POE_FORCE,
    CHSM_POE_ADMIN_NOT_DEFINED
} chsm_poe_admin_t;

typedef enum chsm_poe_oper_e
{
    CHSM_POE_PD_OFF = 0, /* POE off as admin off */
    CHSM_POE_PD_ON,
    CHSM_POE_PD_DETECTION,
    CHSM_POE_PD_STARTUP_ERROR,
    CHSM_POE_PD_POWERUP_ERROR,
    CHSM_POE_PD_FORCE_READY,
    CHSM_POE_PD_FORCE_ERROR,
    CHSM_POE_PD_OVL_OFF,       /* POE off as overload */
    CHSM_POE_PD_OVL,           /* POE chip detect OVL, but still power on */
    CHSM_POE_PD_PRIORITY_OFF,  /* POE off as software priority calc */
    CHSM_POE_PD_ERR_DISABLE_OFF, /* POE off as PD status changed frequently 8 times in 3s */
    CHSM_POE_PD_UNKNOWN
} chsm_poe_oper_t;

typedef enum chsm_poe_port_priority_e
{
    CHSM_POE_PORT_PRI_LOW = 1,
    CHSM_POE_PORT_PRI_HIGH,
    CHSM_POE_PORT_PRI_CRITICAL,
    CHSM_POE_PORT_PRI_MAX
} chsm_poe_port_priority_t;

typedef enum chsm_poe_pm_e
{
    CHSM_POE_PM_STATIC = 0,
    CHSM_POE_PM_DYNAMIC,
    CHSM_POE_PM_NOT_DEFINED
} chsm_poe_pm_t;

typedef enum chsm_poe_daughter_card_work_mode_e
{
    CHSM_POE_DAUGHTER_CARD_USER_MODE = 0,  /* used to normal user mode */
    CHSM_POE_DAUGHTER_CARD_SIFOS_AT_MODE,  /* used to sifos AT test*/
    CHSM_POE_DAUGHTER_CARD_SIFOS_AF_MODE   /* used to sifos AF test */
} chsm_poe_daughter_card_work_mode_t;

/* IEEE standard class */
typedef enum chsm_poe_class_e
{
    CHSM_POE_CLASS0 = 0,
    CHSM_POE_CLASS1,
    CHSM_POE_CLASS2,
    CHSM_POE_CLASS3,
    CHSM_POE_CLASS4,
    CHSM_POE_CLASS_NOT_DEFINED
} chsm_poe_class_t;

typedef enum chsm_poe_legacy_cap_s
{
    CHSM_POE_DISABLE_LEGACY_CAP = 0,
    CHSM_POE_ENABLE_LEGACY_CAP,
    CHSM_POE_LEGACY_CAP_NOT_DEFINED
} chsm_poe_legacy_cap_t;

struct chsm_sys_mac_s
{
    uint8 sys_mac[6];       /*the mac address allocated for system on the board*/
    uint8 port_mac[6];      /*the begining mac address allocated for interface on the board*/
    uint8 mgt_if_name[8];   /*name for management interface, such as eth1, eth2*/
    uint8 port_mac_max;     /*the max num of interface mac address*/
};
typedef struct chsm_sys_mac_s chsm_sys_mac_t;


#ifdef HAVE_IPV6
struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __u32       ifr6_prefixlen;
    int     ifr6_ifindex; 
};
#endif

int32 chsm_reset_system(void);
int32 chsm_mgt_get_stm_mode();
int32 chsm_mgt_set_stm_mode(uint8 stm_mode);
int32 chsm_mgt_show_sdk_alloc_info();
void chsm_mgt_set_oem_info();
char * _chsm_verdor_centec_product_name_str(glb_board_type_t *board_type);
char * _chsm_verdor_centec_chassis_str(glb_board_type_t *board_type);
char * _chsm_verdor_centec_board_type_str(glb_board_type_t *board_type);
//int32 chsm_mgt_set_real_oem_info(oem_info_t* oem_info);
int32 chsm_mgt_parser_create_oem_info_file(uint8* buffer);
char* chsm_mgt_get_sensor_position_str(uint8 pos);
int32
chsm_manage_sensor_tempr(tbl_sensor_t* sensor);
int32
chsm_manage_fan_status(tbl_fan_t* fan);
int32
chsm_manage_psu_run_status(tbl_psu_t* psu);
int32
chsm_manage_psu_alert_status(tbl_psu_t* psu);

void 
chsm_manage_fiber_status(tbl_port_t *p_db_port, tbl_fiber_t *p_fiber, tbl_fiber_t *p_db_fiber);
int32 
chsm_reload_lc(int slot);
int32 
chsm_mgt_set_port_info(port_info_t* port_info);


int32
chsm_card_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_fiber_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_fiber_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_fan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_psu_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_sensor_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_sensor_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_vcm_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_poe_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
chsm_chassis_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
#endif
