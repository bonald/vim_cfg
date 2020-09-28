/****************************************************************************
* $Id$
*  Enterprise oid OEM
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-11-25 08:44
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "oem.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#define M_OEM_INFO_FILE             "/tmp/oem_info"
#define M_OID_FIELD_NAME            "snmp_enterprise_oid"
#define M_DEFAULT_ENTERPRISE_OID    27975

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
static int g_enterprise_id = M_DEFAULT_ENTERPRISE_OID;

/****************************************************************************
 *
* Function
* 
****************************************************************************/

int get_enterprise_id()
{
    return g_enterprise_id;
}

void set_enterprise_id(int eid)
{
    g_enterprise_id = eid;
}

void oem_for_trap(oid *obj_id, size_t objlen, oid *version_id, size_t verlen)
{
    obj_id[objlen - 2] = (oid)get_enterprise_id();
    version_id[verlen - 2] = (oid)get_enterprise_id();
    return;
}

void oem_for_version_sysoid(oid *poid, size_t len)
{
    poid[len - 2] = (oid)get_enterprise_id();
    return;
}

int oem_init()
{
    FILE *fp = NULL;
    int ret = -1;
    char szLine[1024];
    char *p;
    int myoid = -1;

    if ((fp = fopen(M_OEM_INFO_FILE, "r")) == NULL) {
        snmp_log(LOG_ERR, "Open OEM info file failed\n");
        goto err_out;
    }

    while (fgets(szLine, 1024, fp)) {
        p = strstr(szLine, M_OID_FIELD_NAME);
        if (NULL != p) {
            p += strlen(M_OID_FIELD_NAME) + 1;
            myoid = strtol(p, NULL, 10);
            set_enterprise_id((int)myoid);
            ret = 0;
            break;
        }
    }
    
err_out:
    if (NULL != fp) {
        fclose(fp);
    }
    return ret;
}

