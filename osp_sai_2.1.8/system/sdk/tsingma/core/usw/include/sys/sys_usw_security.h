/**
 @file sys_usw_security.h

 @date 2010-2-26

 @version v2.0

---file comments----
*/

#ifndef SYS_USW_SECURITY_H_
#define SYS_USW_SECURITY_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_security.h"
#include "ctc_l2.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

#define SYS_STMCTL_INVALID_OFFSET               CTC_MAX_UINT16_VALUE



#define SYS_SECURITY_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(security, security, SECURITY_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);

#define SYS_SECURITY_DBG_FUNC()                          \
    { \
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);\
    }

#define SYS_SECURITY_DBG_DUMP(FMT, ...)               \
    {                                                 \
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__);\
    }

#define SYS_SECURITY_DBG_INFO(FMT, ...)               \
    {                                                 \
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__);\
    }

#define SYS_SECURITY_DBG_ERROR(FMT, ...) \
    { \
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, FMT, ##__VA_ARGS__);\
    }

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_security_init(uint8 lchip);

/**
 @brief De-Initialize security module
*/
extern int32
sys_usw_security_deinit(uint8 lchip);

/*port security*/
extern int32
sys_usw_mac_security_set_port_security(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_mac_security_get_port_security(uint8 lchip, uint32 gport, bool* p_enable);

/*mac limit*/
extern int32
sys_usw_mac_security_set_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t action);
extern int32
sys_usw_mac_security_get_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t* action);
extern int32
sys_usw_mac_security_set_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t action);
extern int32
sys_usw_mac_security_get_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t* action);
extern int32
sys_usw_mac_security_set_system_mac_limit(uint8 lchip, ctc_maclimit_action_t action);
extern int32
sys_usw_mac_security_get_system_mac_limit(uint8 lchip, ctc_maclimit_action_t* action);

/*learning limit*/

extern int32
sys_usw_mac_security_set_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learn_limit);

extern int32
sys_usw_mac_security_get_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit, uint32* p_running_num);

/*ip source guard*/
extern int32
sys_usw_ip_source_guard_add_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem);

extern int32
sys_usw_ip_source_guard_remove_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem);

/*storm control cfg*/
extern int32
sys_usw_storm_ctl_set_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* stmctl_cfg);

extern int32
sys_usw_storm_ctl_get_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* stmctl_cfg);

extern int32
sys_usw_storm_ctl_set_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg);

extern int32
sys_usw_storm_ctl_get_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg);

/*port isolation*/

extern int32
sys_usw_port_isolation_set_route_obey_isolated_en(uint8 lchip, bool enable);

extern int32
sys_usw_port_isolation_get_route_obey_isolated_en(uint8 lchip, bool* p_enable);

#ifdef __cplusplus
}
#endif

#endif

