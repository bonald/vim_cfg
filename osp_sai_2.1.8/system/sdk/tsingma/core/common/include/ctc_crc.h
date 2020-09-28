/****************************************************************************
 *file ctc_crc.h

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2012-11-26

 *version v2.0

  This file contains  debug header file.
 ****************************************************************************/
#ifndef _CTC_CRC_H
#define _CTC_CRC_H

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Header Files
*
***************************************************************/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @addtogroup crc CRC
 @{
*/

extern uint8
ctc_crc_calculate_crc8(uint8* data, uint32 len, uint32 init_crc);

extern uint8
ctc_crc_calculate_crc4(uint8* data, uint32 len, uint32 init_crc);

/**@} end of @addtogroup crc CRC */

#ifdef __cplusplus
}
#endif

#endif /* _CTC_DEBUG_H_ */

