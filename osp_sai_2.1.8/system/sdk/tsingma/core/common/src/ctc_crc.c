/****************************************************************************
 *file ctc_crc.c

 *author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 *date 2012-11-26

 *version v2.0

  This file contains  debug header file.
 ****************************************************************************/

/****************************************************************
*
* Header Files
*
***************************************************************/
#include "sal.h"
#include "ctc_crc.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/



/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

/**
 @brief Calculate CRC-8

 @param[in]  data       Pointer to the buffer needed to calculate CRC-8

 @param[in]  len        Length of the buffer

 @param[in]  init_crc   initial CRC-8 value

 @remark None

 @return new CRC-8
*/
uint8
ctc_crc_calculate_crc8(uint8* data, uint32 len, uint32 init_crc)
{
    uint32 crc = 0;
    int32 i = 0;
    uint32 msb_bit = 0;
    uint32 poly_mask = 0;
    uint32 topone=0;
    uint32 poly = 0x107;
    uint32 poly_len = 8;
    uint32 bit_len = len * 8;

    if (NULL == data)
    {
        return 0;
    }

    poly_mask = (1<<poly_len) -1;
    topone = (1 << (poly_len-1));
    crc = init_crc & poly_mask;

    for (i = 0; i < bit_len; i++)
    {
        msb_bit = ((data[i / 8] >> (7 - (i % 8))) & 0x1) << (poly_len - 1);
        crc ^= msb_bit;
        crc = (crc << 1) ^ ((crc & topone) ? poly : 0);
    }

    crc &= poly_mask;
    return crc;
}

/**
 @brief Calculate CRC-4

 @param[in]  data       Pointer to the buffer needed to calculate CRC-4

 @param[in]  len        Length of the buffer

 @param[in]  init_crc   initial CRC-4 value

 @remark None

 @return new CRC-4
*/
uint8
ctc_crc_calculate_crc4(uint8* data, uint32 len, uint32 init_crc)
{
    uint32 crc = 0;
    int32 i = 0;
    uint32 msb_bit = 0;
    uint32 poly_mask = 0;
    uint32 topone=0;
    uint32 poly = 0x13;
    uint32 poly_len = 4;
    uint32 bit_len = len * 8;

    if (NULL == data)
    {
        return 0;
    }

    poly_mask = (1<<poly_len) -1;
    topone = (1 << (poly_len-1));
    crc = init_crc & poly_mask;

    for (i = 0; i < bit_len; i++)
    {
        msb_bit = ((data[i / 8] >> (7 - (i % 8))) & 0x1) << (poly_len - 1);
        crc ^= msb_bit;
        crc = (crc << 1) ^ ((crc & topone) ? poly : 0);
    }

    crc &= poly_mask;
    return crc;
}

