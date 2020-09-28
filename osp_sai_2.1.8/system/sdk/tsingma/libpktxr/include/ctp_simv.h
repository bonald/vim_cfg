/****************************************************************************
* $Id$
*  CTP struct and declares for Simv
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2013-04-16 15:17
* Reason        : First Create.
****************************************************************************/

#ifndef _CTP_SIMV_H_
#define _CTP_SIMV_H_

typedef enum tag_payload_format {
    E_PKT_PLD_FMT_ZERO = 0, /* fill with 0x00 */
    E_PKT_PLD_FMT_FULL = 1, /* fill with 0xFF */
    E_PKT_PLD_FMT_INC_BYTE = 2,  /* fill with 0x00, 0x01, 0x02, ... */
    E_PKT_PLD_FMT_INC_WORD = 3,  /* fill with 0x0000, 0x0001, 0x0002, ... */
    E_PKT_PLD_FMT_FIXED = 4,  /* fixed string ... */
    E_PKT_PLD_FMT_REPEAT = 5,  /* repeated fixed string ... */
    E_PKT_PLD_FMT_RANDOM = 6,  /* random values ... */
} payload_format_e;

typedef struct tag_pkt_mod_param {
    /* BE CAREFUL: < 0 for not care field */
    int32_t nChipId; /* chip identifier,  */
    int32_t nPktId; /* packet id */
    int32_t nSrcChannal; /* source channal */
    int32_t nDstChannal; /* destination channal */

     /* frame size:
      *   0 for keep frame size of the template
      *
      *   if frame size less than encoded head size, ignore this setting
      */
    int32_t nFrameSize;
    payload_format_e ePayloadType; /* payload type */

    int32_t nCrc; /* replace crc field if have, 0 for ignore this */

    char *pszBlockId; /* ipe/fwd/epe/chip */
    char *pszPayload; /* payload template hex string, eg: F01122 */

    char *pszTemplate; /* bridge/route/mpls... */
} pkt_mod_param_t;

typedef enum {
    E_MOD_FLD_CRC = 0,
    E_MOD_FLD_CHIP_ID = 1,
    E_MOD_FLD_PKT_ID = 2,
    E_MOD_FLD_SRC_CHANNAL = 3,
    E_MOD_FLD_DST_CHANNAL = 4,
    E_MOD_MAX
} pkt_mod_field_e;

#define M_FLD_CHIP_ID       "chip_id"
#define M_FLD_PKT_ID        "pkt_id"
#define M_FLD_SRC_CHANNAL   "src_ch"
#define M_FLD_DST_CHANNAL   "dst_ch"
#define M_FLD_CRC           "pkt_crc"

/* save & restore data for generate_pkt */
typedef struct {
    /* save modifier pointer */
    void *ppModArr[E_MOD_MAX];
    void *pszPayloadPattern;  /* payload string */
    payload_format_e ePayloadType;
} pkt_mod_saved_data_t;


#endif /* !_CTP_SIMV_H_ */
