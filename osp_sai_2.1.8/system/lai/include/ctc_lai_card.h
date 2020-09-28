/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-011
* Reason        : First Create.
****************************************************************************/
#if !defined (__CTCLAICARD_H_)
#define __CTCLAICARD_H_
#include "laiinc.h"

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_tempfile_define.h"
#include "lcapi.h"
#include "lcm_specific.h"
#include "lcm_mgt.h"
#include "lcm_client.h"
#include "lcm_error.h"
#include "lcm_card.h"
#include "lcm_port.h"
#include "lcm_dpll.h"
#include "lcm_debug.h"
#include "lcm_srv.h"
#include "lcm_lai.h"

#define LAI_OEM_DEFAULT_NAME "default"
#define LAI_OEM_CENTEC_YEAR            "2015" 

extern lai_card_notification_t* p_lai_card_noti;

lai_status_t
ctc_lai_card_init();

lai_status_t
ctc_lai_get_card_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_card_info_t* fan);

lai_status_t
ctc_lai_set_oem_info(
    _In_ lai_object_id_t oid,
    _In_ lai_oem_info_t* oem_info);

#endif // __CTCLAIFAN_H_

