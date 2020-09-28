/****************************************************************************
 * ctp_util.h  provides all CTP common Utility and Macros define.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Anchor Xie.
 * Date:         2007-4-13.
 * Reason:       First Create.
*****************************************************************************/

#ifndef _CTP_UTIL_H_
#define _CTP_UTIL_H_

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/

#include "sal_types.h"

/****************************************************************************
 *
 * Data Structure and Defines
 *
****************************************************************************/
/*
 * This enumeration used for List all control word
 */
enum
{
    MTU,
    EN_8022,
    PKT_LEN,
    PKT_TYPE,
    REPEAT,
    NEXT_TYPE,
    FLEN_FID, /* field length field ID */
    FIEL_LEN, /* field length */
    PLD_OFFSET,
    PLD_LEN,
    PKT_CRC, /* calc the whole packet's crc */
    CONTROL_WORD_NUM,
};

/*
 * This enumeration used for list all CTP Error Code
 */
enum ctp_error
{
    CTP_E_NONE,
    CTP_E_MIS_FIELD,
    CTP_E_LESS_LEN,
    CTP_E_MEMORY_ERROR,
    CTP_E_EMPTY,
    CTP_E_EXISTS,
    CTP_E_MODIFY_FAILED,
    CTP_E_NUM,
};
typedef enum ctp_error ctp_error_e;
#define CTP_OUT          if(debug)printf

// this macro execute a function, and determine the return value, if the return value
// is not correct, printf the err string and key
#define CTP_ERR_RETURN( f, k) ret=f;if(CTP_E_NONE!=ret) {\
CTP_OUT("\nERR_CODE = %s, KEY = %s\n",ctc_pktxr_get_err_code_string(ret),k);\
return ret;}

/****************************************************************************
 *
 * Global and Declarations
 *
 ****************************************************************************/
 extern char debug ;

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/

/*******************************************************************************
 * Name   : ctc_pktxr_convert_to_upper
 * Purpose: conver to the src string to upper and store it to dest string.
 * Input  : dest - the destination string pointer.
 *        : src  - the source string pointer.
 * Output : None.
 * Return : N/A.
 * Note   : N/A
*******************************************************************************/
void
ctc_pktxr_convert_to_upper(char *dest, const char *src);

/*******************************************************************************
 * Name   : ctc_pktxr_get_err_code_string
 * Purpose: Get the pointer of stored error string.
 * Input  : error code.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
char *
ctc_pktxr_get_err_code_string(uint16 error);

/*******************************************************************************
 * Name   : ctc_pktxr_length_lookup
 * Purpose: look up the field length.
 * Input  : .
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
uint8
ctc_pktxr_length_lookup(uint16 *cw, uint16 f, uint16 *l);

/****************************************************************************
 * Name:       fill_field_bit
 * Purpose:    copy src to dest with a bits range.
 * Parameters:
 * Input:      src  -- source buffer.
 *             start bit -- start bit number.
 *             end bit -- end bit number.
 * Output:     dest -- dest buffer.
 * Return:     N/A.
 * Note:       because we only use it for little endian,
 *             no need care big endian.
 ****************************************************************************/
void
ctc_pktxr_set_field_bit(uint8 *dest, uint32 start_bit, uint32 end_bit, uint8 *src);

/****************************************************************************
 * Name:       ctc_pktxr_string2char
 * Purpose:    conver a char buffer to string.
 * Parameters:
 * Input:      str   -- pointer to string.
 * Output:     buff  -- pointer to char buffer
 * Note:       none.
 *****************************************************************************/
void
ctc_pktxr_string2char(char *str, uint8 *buff);

/****************************************************************************
 * Name:       ctc_pktxr_get_field_bit
 * Purpose:    copy src to dest with a bits range.
 * Parameters:
 * Input:      src  -- source buffer.
 *             start bit -- start bit number.
 *             end bit -- end bit number.
 * Output:     dest -- dest buffer.
 * Return:     DRV_E_NONE = success.
 *           Other = Error, please reference DRV_E_XXX.
 * Note:       because we only use it for little endian,
 *             no need care big endian.
 ****************************************************************************/
void
ctc_pktxr_get_field_bit(uint8 *dest, uint32 start_bit, uint32 end_bit, uint8 *src);

/*****************************************************************************
 * Name         : ctc_pktxr_ctp_crc32
 * Purpose      : calculate crc32
 * Input        : seed: initial crc seed
 *                data: data buffer to be calculated
 *                data_len: data length
 * Output       : crc: calculated crc
 * Return       :
 * Note         :
 *****************************************************************************/
uint32
ctc_pktxr_ctp_crc32(uint32 seed, unsigned char *data, uint32 data_len);


/****************************************************************************
 * Name:       ctc_pktxr_calculate_chksum
 * Purpose:    calculate uint16 checksum of a memory block.
 * Parameters:
 * Input:       p : base pointer of the memory block
 *             length : length of the memory block
 *             init is the initial seed value for calculation
 * Output:     none.
 * Return:     0 = success. Otherwise failure.
 * Note:       none.
 ****************************************************************************/
uint16
ctc_pktxr_calculate_chksum (uint16 *start, uint16 length, uint32 init);


/******************************************************************************
 * Name   : ctc_pktxr_memcpy_chksum
 * Purpose: copy data to memory location according to edian
 * Input  : p_pkt: address of memory pointer
 *          data:  points to data to be copied
 *          size:  data size
 * Output : p_pkt: memory pointer gets updated based on size
 * Return : SUCCESS
 *          Other = ErrCode
 * Note   : N/A
 *****************************************************************************/
int32
ctc_pktxr_memcpy_chksum(uint8 **p_pkt, void *data, int32 size);

void *ctc_pktxr_ctckal_calloc(size_t nelem, size_t elsize);
char *ctc_pktxr_ctckal_strdup(char *);
void *ctc_pktxr_ctckal_malloc(uint32 size);
void ctc_pktxr_ctckal_free(void *addr);
void ctc_pktxr_uint2str(char *str, uint8 *buff, int len);
uint8 ctc_pktxr_calculate_crc4(uint8 *data, int32 len, uint8 init_crc);
char * mem_strdup (const char *, int, char *, int);
void * mem_calloc (int, int, char *, int);




#endif
