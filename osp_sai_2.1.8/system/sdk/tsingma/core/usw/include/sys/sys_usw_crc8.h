/**
 @file sys_usw_crc8.h

 @date 2010-03-23

 @version v2.0
*/

#ifndef _SYS_USW_CRC8_H_
#define _SYS_USW_CRC8_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

extern uint8
sys_usw_calculate_crc8(uint8 lchip, uint8* data, int32 len, uint8 init_crc);

#ifdef __cplusplus
}
#endif

#endif

