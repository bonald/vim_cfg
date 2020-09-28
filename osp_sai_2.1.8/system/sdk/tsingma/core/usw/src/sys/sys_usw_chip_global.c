/**
 @file sys_chip_global.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2017-03-03

 @version v2.0

 The file contains all chip APIs of ctc layer
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_chip.h"
#include "sys_usw_chip_global.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_CHIP_BITMAP_NUM     32

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
uint32 usw_chip_valid[CTC_MAX_LOCAL_CHIP_NUM/SYS_CHIP_BITMAP_NUM+1] = {0};

int32
sys_usw_chip_set_active(uint8 lchip, uint32 value)
{
    if (lchip >= CTC_MAX_LOCAL_CHIP_NUM)
    {
        return CTC_E_INVALID_CHIP_ID;
    }

    if (value)
    {
        if (CTC_IS_BIT_SET(usw_chip_valid[lchip/SYS_CHIP_BITMAP_NUM], lchip%SYS_CHIP_BITMAP_NUM))
        {
            return CTC_E_EXIST;
        }
        else
        {
            CTC_BIT_SET(usw_chip_valid[lchip/SYS_CHIP_BITMAP_NUM], lchip%SYS_CHIP_BITMAP_NUM);
        }
    }
    else
    {
        if (CTC_IS_BIT_SET(usw_chip_valid[lchip/SYS_CHIP_BITMAP_NUM], lchip%SYS_CHIP_BITMAP_NUM))
        {
            CTC_BIT_UNSET(usw_chip_valid[lchip/SYS_CHIP_BITMAP_NUM], lchip%SYS_CHIP_BITMAP_NUM);
        }
        else
        {
            return CTC_E_NOT_INIT;
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_chip_check_active(uint8 lchip)
{
    if (lchip >= CTC_MAX_LOCAL_CHIP_NUM)
    {
        return CTC_E_INVALID_CHIP_ID;
    }

    if (CTC_IS_BIT_SET(usw_chip_valid[lchip/SYS_CHIP_BITMAP_NUM], lchip%SYS_CHIP_BITMAP_NUM))
    {
        return CTC_E_NONE;
    }
    else
    {
        return CTC_E_NOT_INIT;
    }

    return CTC_E_NONE;
}

