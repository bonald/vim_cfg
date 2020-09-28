/****************************************************************************
 * $Id$
 *  Centec Storm Control related global MACRO, ENUM, Date Structure defines file
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Zhen Jiang
 * Date          : 2010-09-20 16:00
 * Reason        : First Create.

 * Revision      : R0.02
 * Author        : liul
 * Date          : 2012-12-20 16:00
 * Reason        : Add ratelimit op enum struct.
 ****************************************************************************/
#ifndef __GLB_RATELIMIT_DEFINE_H__
#define __GLB_RATELIMIT_DEFINE_H__

/* Modified by kcao for bug 7448 2008-12-12
 * change ratio from 0.9 to 1.0, otherwise 1500 Bytes packets will be discard when config level 100.0
 * error = (valid bandwidth ratio - RATELIMIT_THRESHOLD_RATIO)/RATELIMIT_THRESHOLD_RATIO
 *          64 Bytes        1500 Bytes
 * 0.9 :    -15.6%          8.9%
 * 1.0 :    -24%            -1.3%
 */
#define GLB_RATELIMIT_THRESHOLD_RATIO    (1.0)
/*Modified by xgu for bug 26618, granularity changed from 1000 to 10000, 2014-1-2*/
#define GLB_RATELIMIT_CLEAR_FREQUENCY    (10000)
#define GLB_RATELIMIT_THRESHOLD_ZERO     (0)

/* refer to SYS_CAP_STMCTL_MAX_PPS for usw sdk */
#define GLB_SECURITY_STMCTL_MAX_PPS      (148809523)

enum glb_ratelimit_type_e
{
    GLB_RATELIMIT_UNICAST = 0,
    GLB_RATELIMIT_BROADCAST = 1,
    GLB_RATELIMIT_MULTICAST = 2,
    GLB_RATELIMIT_MAX_TYPE
};
typedef enum glb_ratelimit_type_e glb_ratelimit_type_t;

enum glb_ratelimit_mode_e
{
    GLB_RATELIMIT_MODE_NONE = 0,
    GLB_RATELIMIT_MODE_PPS = 1,
    GLB_RATELIMIT_MODE_LEVEL = 2,
    GLB_RATELIMIT_MODE_BPS = 3,
    GLB_RATELIMIT_MAX_MODE
};
typedef enum glb_ratelimit_mode_e glb_ratelimit_mode_t;

enum glb_ratelimit_unit_e
{
    GLB_RATELIMIT_UNIT_BYTE,
    GLB_RATELIMIT_UNIT_PACKET,
    GLB_RATELIMIT_MAX_UNIT,
};
typedef enum glb_ratelimit_unit_e glb_ratelimit_unit_t;

/*modified by liul for storm control, 2012-12-18*/
enum glb_ratelimit_op_e
{
    GLB_SECURITY_STORM_CTL_OP_PORT = 0, /* port based strorm control */
    GLB_SECURITY_STORM_CTL_OP_VLAN,     /* vlan based strorm control */
    GLB_SECURITY_STORM_CTL_OP_MAX
};
typedef enum glb_ratelimit_op_e glb_ratelimit_op_t;

#endif /*!__GLB_RATELIMIT_DEFINE_H__*/

