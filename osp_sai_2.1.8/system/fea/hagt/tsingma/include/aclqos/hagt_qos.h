/****************************************************************************
 * hagt_qos.h :  Macro, data structure for hal agent qos module
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   rongl
 * Date     :   2010-09-15
 ****************************************************************************/

#ifndef __HAGT_QOS_H__
#define __HAGT_QOS_H__

#define HAGT_QACL_TCAM_PORT_BLOCK    0
#define HAGT_QACL_TCAM_VLAN_BLOCK    1

extern int32
hagt_qos_init (hagt_master_t* p_hagt_master);

#endif /* __HAGT_QOS_H__ */

