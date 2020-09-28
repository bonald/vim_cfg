#ifndef __GLB_TAP_DEFINE_H__
#define __GLB_TAP_DEFINE_H__

/****************************************************************************
 * glb_tap_define.h :  Macro, data structure for global used layer struct, macro, flags, etc;
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   xuwj
 * Date     :   2017-5-22
 ****************************************************************************/
#include "glb_const.h"

enum glb_tap_flags_e
{
    GLB_TAP_FLAGS_ASSIGN_ID = 0,
    GLB_TAP_FLAGS_MAX,
};
typedef enum glb_tap_flags_e glb_tap_flags_t;

enum glb_tap_group_type_e
{
    GLB_TAP_GROUP_TYPE_NORMAL = 0,
    GLB_TAP_GROUP_TYPE_ASSIGN_ID,
    GLB_TAP_GROUP_TYPE_ALL,
    GLB_TAP_GROUP_TYPE_MAX,
};
typedef enum glb_tap_group_type_e glb_tap_group_type_t;

#define TAP_ACL_CONFIG_PREFIX       "#FLOW#"
#define TAP_ACL_CONFIG_PREFIX_LEN   6

#endif /*__GLB_TAP_DEFINE_H__*/
