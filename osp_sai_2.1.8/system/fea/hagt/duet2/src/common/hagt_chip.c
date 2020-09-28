
/****************************************************************************
 * $Id$
 *  HAGT Chip
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Kun Cao
 * Date          : 2010-08-09 16:00
 * Reason        : First Create.
 ****************************************************************************/

/****************************************************************************
 *
 * Header Files 
 *
 ****************************************************************************/
#include "hagt_inc.h"
#if 0
#include "sal.h"
#include "glb_macro.h"
#endif
#include "glb_if_define.h"
#include "hagt_chip.h"

/****************************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Global and Declarations
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/

/***************************************************************************************************
 * Name         : hagt_chip_local2glb 
 * Purpose      : convert local chip NO. to global chip NO.
 * Input        : lchip: local chip NO.      
 * Output       : N/A              
 * Return       : int32: global chip NO., return GLB_INVALID_CHIPID if covert fail.
 * Note         : 
***************************************************************************************************/
int32
hagt_chip_local2glb(uint8 lchip)
{
    uint8 gchip = 0;

    ctc_get_gchip_id(0, &gchip);
    return (int32)gchip;
}

/***************************************************************************************************
 * Name         : hagt_chip_glb2local 
 * Purpose      : convert global chip NO. to local chip NO.
 * Input        : lchip: global chip NO.      
 * Output       : N/A              
 * Return       : int32: local chip NO., return GLB_INVALID_CHIPID if covert fail.
 * Note         : 
***************************************************************************************************/
int32
hagt_chip_glb2local(uint8 gchip)
{
    uint8 lchip_num;
    uint8 index; /*for loop*/
    uint8 glb_chip;

    HAGT_IF_ERROR_RETURN(ctc_get_local_chip_num(&lchip_num));
    /*2. check is local*/
    for (index = 0; index < lchip_num; index++)
    {
        HAGT_IF_ERROR_RETURN(ctc_get_gchip_id(index, &glb_chip));
        if (glb_chip == gchip)
        {
            return index;
        }
    }
    return GLB_INVALID_CHIPID;    
}

/***************************************************************************************************
 * Name         : hagt_chip_port_is_local 
 * Purpose      : check whether this port is in local linecard      
 * Input        : gportid: global portid         
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
bool
hagt_chip_port_is_local (uint16 gportid)
{
    uint8 lchip_num;
    uint8 index; /*for loop*/
    uint8 gchipid_port;
    uint8 gchip;

    /*1. get local chip num*/
    HAGT_IF_ERROR_RETURN(ctc_get_local_chip_num(&lchip_num));
    gchipid_port = GLB_GET_GLOBAL_CHIPID(gportid);

    /*2. check is local*/
    for (index = 0; index < lchip_num; index++)
    {
        HAGT_IF_ERROR_RETURN(ctc_get_gchip_id(index, &gchip));
        if (gchipid_port == gchip)
        {
            return TRUE;
        }
    }

    return FALSE;
}

