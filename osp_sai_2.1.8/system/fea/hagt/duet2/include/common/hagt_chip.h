/****************************************************************************
 * hagt_chip.h      Hal Agnt Chip file.
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Kun Cao 
 * Date          :2010-08-09.
 * Reason        :First Create.
 ****************************************************************************/
#ifndef __HAGT_CHIP_H__
#define __HAGT_CHIP_H__

/****************************************************************************
 *
 * Header Files 
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
int32
hagt_chip_local2glb(uint8 lchip);
int32
hagt_chip_glb2local(uint8 gchip);
/***************************************************************************************************
 * Name         : hagt_chip_port_is_local 
 * Purpose      : check whether this port is in local linecard      
 * Input        : gportid: global portid         
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
extern bool
hagt_chip_port_is_local (uint16 gportid);

#endif /* __HAGT_CHIP_H__*/
