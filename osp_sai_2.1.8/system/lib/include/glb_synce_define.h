/****************************************************************************
 * $Id$
 *  Centec SyncE related global MACRO, ENUM, Date Structure defines file
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Zhen Jiang
 * Date          : 2011-03-05 16:00
 * Reason        : First Create.
 ****************************************************************************/
#ifndef __GLB_SYNCE_DEFINE_H__
#define __GLB_SYNCE_DEFINE_H__

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#define GLB_SYNCE_SERDES_MIN 0x0
#define GLB_SYNCE_SERDES_MAX 0xF

#define GLB_SYNCE_DIVIDER_MIN 0x0
#define GLB_SYNCE_DIVIDER_MAX 0x3F

enum glb_synce_type_e
{
    GLB_SYNCE_PRIMARY,
    GLB_SYNCE_SECONDARY1,
    GLB_SYNCE_SECONDARY2,
    GLB_SYNCE_SECONDARY3,
};
typedef enum glb_synce_type_e glb_synce_type_t;

/* modified by yaom for bug 18501 20110306 */  
enum glb_synce_state_e
{
    GLB_SYNCE_STATE_UNKNOWN1,
    GLB_SYNCE_STATE_FREERUN,
    GLB_SYNCE_STATE_HOLDOVER,
    GLB_SYNCE_STATE_UNKNOWN2,
    GLB_SYNCE_STATE_LOCKED,
    GLB_SYNCE_STATE_PRELOCKED2,
    GLB_SYNCE_STATE_PRELOCKED,
    GLB_SYNCE_STATE_LOSSOFLOCK
};
typedef enum glb_synce_state_e glb_synce_state_t;

/****************************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/

#endif /*!__GLB_SYNCE_DEFINE_H__*/
