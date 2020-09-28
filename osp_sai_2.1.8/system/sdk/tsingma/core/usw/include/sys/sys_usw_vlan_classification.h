/**
 @file sys_usw_vlan_classification.h

 @date 2009-12-15

 @version v2.0

 The file contains all apis of sys Vlan Classification
*/

#ifndef _SYS_USW_VLAN_CLASSIFICATION_H
#define _SYS_USW_VLAN_CLASSIFICATION_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_vlan.h"
#include "ctc_vector.h"
/***********************************************
*
*   Macros and Defines
*
***********************************************/

/***********************************************
*
*   Functions
*
***********************************************/

extern int32
sys_usw_vlan_class_init(uint8 lchip, void* vlan_global_cfg);
extern int32
sys_usw_vlan_class_deinit(uint8 lchip);
extern int32
sys_usw_vlan_add_vlan_class(uint8 lchip, ctc_vlan_class_t* vlan_class_info);
extern int32
sys_usw_vlan_remove_vlan_class(uint8 lchip, ctc_vlan_class_t* vlan_class_info);
extern int32
sys_usw_vlan_remove_all_vlan_class(uint8 lchip, ctc_vlan_class_type_t type);
extern int32
sys_usw_vlan_add_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t type,  ctc_vlan_miss_t* p_action);
extern int32
sys_usw_vlan_remove_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t type);
extern int32
sys_usw_vlan_show_default_vlan_class(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

