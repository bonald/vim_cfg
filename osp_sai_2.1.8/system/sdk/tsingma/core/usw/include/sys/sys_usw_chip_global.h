/**
 @file ctc_chip.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2016-9-9

 @version v2.0

   This file contains all chip related data structure, enum, macro and proto.
*/

#ifndef _CTC_USW_CHIP_GLOBAL_H
#define _CTC_USW_CHIP_GLOBAL_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "sal_types.h"
#include "ctc_mix.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
extern uint8 g_lchip_num;
extern uint8 g_ctcs_api_en;
extern uint32 g_rchain_en[2];


#define SYS_LCHIP_CHECK_ACTIVE(lchip)                       \
    do {                                                    \
        if(0 != sys_usw_chip_check_active(lchip)){              \
            return CTC_E_INVALID_CHIP_ID; }            \
    } while (0)

#define SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip)             \
    do {                                                    \
        int32 (op) = 0;                                     \
        (op)=(sys_usw_chip_check_active(lchip));                \
        if ((op)<0)                                         \
        {                                                   \
            return;                                         \
        }                                                   \
    } while (0)

/*Below Define is for CTC APIs dispatch*/
/*---------------------------------------------------------------*/
#define LCHIP_CHECK(lchip)                                  \
    do {                                                    \
        if ((lchip) >= g_lchip_num){                        \
            return CTC_E_INVALID_CHIP_ID; }            \
    } while (0)

#define SYS_MAP_GPORT_TO_LCHIP(gport, lchip)                                    \
    do{                                                                         \
        if(!g_ctcs_api_en)                                                      \
        {                                                                       \
            SYS_MAP_GCHIP_TO_LCHIP(SYS_MAP_CTC_GPORT_TO_GCHIP(gport), lchip);   \
        }                                                                       \
    }while(0);

#define CTC_SET_API_LCHIP(lchip, api_lchip)                 \
    do{                                                     \
        if(!g_ctcs_api_en) lchip = api_lchip;               \
    }while(0);

#define CTC_FOREACH_LCHIP(lchip_start,lchip_end,all_lchip)  \
      if(g_ctcs_api_en && ((all_lchip) != 0xff))                                     \
      {                                                     \
          lchip_start = lchip;                              \
          lchip_end = lchip + 1;                            \
      }                                                     \
      else                                                  \
      {                                                     \
          lchip_start = all_lchip?0:lchip_start;            \
          lchip_end = all_lchip?g_lchip_num:lchip_end;      \
      }                                                     \
      for(lchip = lchip_start ; lchip < lchip_end; lchip++)

#define CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)     \
      if(g_ctcs_api_en)                                 \
      {                                                 \
          lchip_start = 0;                              \
          lchip_end = 0;                                \
      }                                                 \
      else                                              \
      {                                                 \
         lchip_start = (lchip != lchip_end)? 0:lchip;   \
         lchip_end   = lchip;                           \
      }                                                 \
      for(lchip = lchip_start ; lchip < lchip_end; lchip++)

#define CTC_FOREACH_ERROR_RETURN(exist_ret,ret,op)     \
    {                                                  \
        int32 ret_tmp = CTC_E_NONE;                    \
        ret_tmp = (op);                                \
        if(exist_ret == ret_tmp)                       \
        {                                              \
            if(CTC_E_NONE == ret) {ret = ret_tmp;}     \
            continue;                                  \
        }                                              \
        else if(CTC_E_NONE == ret_tmp)                 \
        {                                              \
            continue;                                  \
        }                                              \
        else                                           \
        {                                              \
            ret = ret_tmp;                             \
            break;                                     \
        }                                              \
    }

#define CTC_FOREACH_ERROR_RETURN2(exist_ret1,exist_ret2,ret,op)     \
    {                                                               \
        int32 ret_tmp = CTC_E_NONE;                                 \
        ret_tmp = (op);                                             \
        if(g_lchip_num == 1 || g_ctcs_api_en)                        \
        {                                                           \
            ret = ret_tmp;                                          \
        }                                                           \
        else                                                        \
        {                                                           \
            if((exist_ret1 == ret_tmp) || (exist_ret2 == ret_tmp))  \
            {                                                       \
                if(++count == g_lchip_num)                          \
                {                                                   \
                    ret = ret_tmp;                                  \
                }                                                   \
                continue;                                           \
            }                                                       \
            else if(CTC_E_NONE == ret_tmp)                          \
            {                                                       \
                continue;                                           \
            }                                                       \
            else                                                    \
            {                                                       \
                ret = ret_tmp;                                      \
                break;                                              \
            }                                                       \
        }                                                           \
    }

#define CTC_ACL_SCL_FOREACH_ERROR_RETURN(exist_ret,ret,op)          \
    {                                                               \
        int32 ret_tmp = CTC_E_NONE;                                 \
        ret_tmp = (op);                                             \
        if(g_lchip_num == 1 || g_ctcs_api_en)                       \
        {                                                           \
            ret = ret_tmp;                                          \
        }                                                           \
        else                                                        \
        {                                                           \
            if(exist_ret == ret_tmp)                                \
            {                                                       \
                if(++count == g_lchip_num)                          \
                {                                                   \
                    ret = exist_ret;                                \
                }                                                   \
                continue;                                           \
            }                                                       \
            else if(CTC_E_NONE == ret_tmp)                          \
            {                                                       \
                continue;                                           \
            }                                                       \
            else                                                    \
            {                                                       \
                ret = ret_tmp;                                      \
                break;                                              \
            }                                                       \
        }                                                           \
    }

/****************************************************************
*
* Data Structures
*
****************************************************************/

extern int32 sys_usw_chip_set_active(uint8 lchip, uint32 value);
extern int32 sys_usw_chip_check_active(uint8 lchip);
/**@} end of @defgroup chip global  */

#ifdef __cplusplus
}
#endif

#endif

