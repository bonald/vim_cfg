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
#if !defined (__CTCLAISYNCE_H_)
#define __CTCLAISYNCE_H_

#include "laitype.h"

struct ctc_lai_synce_select_port_s
{    
    bool status; /*speed, duplex, media type or mac enable*/
    uint8 slot_idx; /*slot id in prefix-X-X, for bug 14686.*/
    uint8 port_idx;
    uint8 subport_idx;
    uint8 gport; /*from 1 to max_port_num, eth-X-X*/
    uint8 is_10g_mode;
    uint8 bits_enable;
    uint32 speed;
};
typedef struct ctc_lai_synce_select_port_s ctc_lai_synce_select_port_t;


lai_status_t
ctc_lai_synce_set_attribute(    
    _In_ lai_synce_info_t* synce_info);

lai_status_t
ctc_lai_synce_init();

#endif
