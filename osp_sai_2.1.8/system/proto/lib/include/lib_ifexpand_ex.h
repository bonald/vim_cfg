/****************************************************************************
* $Id$
*  Expand interface for CLI
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2008-08-14 10:48
* Reason        : First Create.
****************************************************************************/

#ifndef _INTF_EXPAND_EX_H_
#define _INTF_EXPAND_EX_H_

/* Interface Name format
 *
 *   fastethernet  module-number / port-number
 *   gigabitethernet  module-number / port-number
 *   tengigabitethernet  module-number / port-number
 *   agg <1-255>
 *   loopback <1-255>
 *   vlan <1-4094>
 *   tunnel <0-2147483647>
 *   null <0-0>
 */
typedef enum _tagIntfType {
    E_INTF_NONE     = 0,
    E_INTF_PHYSICAL = 0x1, /* for fe/ge/tenge interface */
    E_INTF_AGG      = 0x2, /* for agg interface */
    E_INTF_LOOPBACK = 0x4, /* for loopback interface */
    E_INTF_VLAN     = 0x8, /* for vlan interface */
    E_INTF_L2GRE    = 0x10, /* for L2GRE interface */
    E_INTF_L3GRE    = 0x20, /* for L3GRE interface */
    E_INTF_NVGRE    = 0x40, /* for NVGRE interface */
    E_INTF_VXLAN    = 0x80, /* for VXLAN interface */
#if 0 /* Not support currently in OSP. */
    E_INTF_TUNNEL   = 0x16, /* for tunnel interface */
    E_INTF_NULL     = 0x32, /* for NULL interface */
    E_INTF_COMMON   = 0x64, /* for combination interfaces */
#endif
    /* modified by liwh for bug 47983, 2019-03-06 
         delete unsupported l3gre */
    //E_INTF_ALL      = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x40 | 0x80, /* for all interfaces, bitmap combination of all supported interfaces */
    E_INTF_ALL      = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x40 | 0x80, /* for all interfaces, bitmap combination of all supported interfaces */
    /* liwh end */
    E_INTF_MAX
} IntfType_e;

typedef enum tagIntfMatch {
    E_INTF_MATCH_NONE = 0,
    E_INTF_MATCH_NAME_INCOMPLETE,
    E_INTF_MATCH_NAME_FULL,
    //rm by weij, in new cli '-' is put to the prefix of interface
    //E_INTF_MATCH_SLOT_CON_FULL,
    E_INTF_MATCH_SLOT_INCOMPLETE,
    E_INTF_MATCH_SLOT_FULL,
    E_INTF_MATCH_PORT_CON_FULL,
    E_INTF_MATCH_PORT_INCOMPLETE,
    E_INTF_MATCH_PORT_FULL,
    E_INTF_MATCH_SUB_PORT_CON_FULL,
    E_INTF_MATCH_SUB_PORT_FULL,
    E_INTF_MATCH_MAX,
} IntfMatch_e;

typedef int IFEXPAND_HELP_OUT_FUNC(void *pArg, const char *pszMatch, const char *pszHelp);

void intf_expand_init(char *ifname);
void intf_expand_finish();
int intf_expand_generate_help(IFEXPAND_HELP_OUT_FUNC pCB, void *pArg, IntfType_e intfType);
/* for keyword match */
char * intf_expand_match(char *cp, IntfType_e intfType, IntfMatch_e *pmatch);
/* for command line complete */
int intf_expand_get_complete_count(IntfType_e intfType);
void intf_expand_get_complete_string(char *oldtext, IntfType_e intfType,
        char **ptr, int *nCntRet);

#endif /* !_INTF_EXPAND_EX_H_ */
