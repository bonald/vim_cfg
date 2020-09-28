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

#ifndef _OEM_H_
#define _OEM_H_

int get_enterprise_id();
void set_enterprise_id(int eid);

int oem_init();
void oem_for_version_sysoid(oid *poid, size_t len);
void oem_for_trap(oid *obj_id, size_t objlen, oid *version_id, size_t verlen);

#endif /* !_OEM_H_ */
