/****************************************************************************
* $Id$
*  netconf for app
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2018-04-24 17:10
* Reason        : First Create.
****************************************************************************/

#ifdef HAVE_NETCONF

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
    
#include "proto.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "ncutil.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#ifdef _GLB_UML_SYSTEM_ 
#define M_NETCONF_AGENT_PROG_NAME   "/centc_switch/sbin/netconf_agent"
#define M_NETCONF_SS_PROG_NAME      "/centc_switch/sbin/ncss_agent"
#else
#define M_NETCONF_AGENT_PROG_NAME   "/usr/bin/netconf_agent"
#define M_NETCONF_SS_PROG_NAME      "/usr/bin/ncss_agent"
#endif

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/****************************************************************************
 *
* Function
* 
****************************************************************************/

int32
app_netconf_generate_cfg(FILE *fp, tbl_ssh_cfg_t *p_db_cfg)
{
    if (p_db_cfg->netconf_service) {
        fprintf(fp, "Port %d\r\n", p_db_cfg->netconf_port);
        fprintf(fp, "Subsystem  netconf    %s\r\n", M_NETCONF_SS_PROG_NAME);
    }
    
    return 0;
}

int32
app_netconf_service_set(int32 enable, uint16 netconf_port)
{
    char szCmd[PATH_MAX];
    int is_uml = 0;
    
    if (enable) {
#ifdef _GLB_UML_SYSTEM_        
        is_uml = 1;
#endif
        snprintf(szCmd, PATH_MAX, 
            "%s start %s %d  >/dev/null 2>&1",
            M_NETCONF_AGENT_PROG_NAME,
             is_uml ? "uml" : "hw", netconf_port);
        if (system(szCmd) != 0) {
            return 0;
        }

        snprintf(szCmd, PATH_MAX, 
            "cdbctl create/cdb/sys/ns_port_forwarding/0#0#%u#0.0.0.0/ip/0.0.0.0",
            netconf_port);
        if (system(szCmd) != 0) {
            return 0;
        }        
        return 0;
    }
   
    snprintf(szCmd, PATH_MAX, 
        "%s stop %s %d >/dev/null 2>&1",
        M_NETCONF_AGENT_PROG_NAME,
        is_uml ? "uml" : "hw", netconf_port);
    if (system(szCmd) != 0) {
        return -1;
    }

    snprintf(szCmd, PATH_MAX, 
        "cdbctl delete/cdb/sys/ns_port_forwarding/0#0#%u#0.0.0.0/ip/0.0.0.0",
        netconf_port);
    if (system(szCmd) != 0) {
        return 0;
    }    
    return 0;
}

#endif /* !HAVE_NETCONF */

