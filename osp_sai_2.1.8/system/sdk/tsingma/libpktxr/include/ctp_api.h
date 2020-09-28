/****************************************************************************
 * ctp_common.h  provides all CTP common APIs define.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Anchor Xie.
 * Date:         2007-4-21.
 * Reason:       First Create.
*****************************************************************************/

#ifndef _CTP_API_H_
#define _CTP_API_H_

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "ctp_util.h"
//#include "ctp_header_format.h"

/****************************************************************************
 *
 * Data Structure and Defines
 *
****************************************************************************/

#if 0
/*******************************************************************************
 * Name   : ctc_pktxr_ctp_encode_command_set
 * Purpose: handle encorder command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set
 * Output : None.
 * Return : .
 * Note   : user need release its own memory
*******************************************************************************/
ctp_error_e
ctc_pktxr_ctp_encode_command_set(const char *keyw, const char *value);

/*******************************************************************************
 * Name   : packet_encoder
 * Purpose: get the result packet.
 * Input  : None.
 * Output : outpacket pointer.
 * Return : .
 * Note   : None.
*******************************************************************************/

ctp_error_e
packet_encoder(int bCompose);
#endif

/*******************************************************************************
 * Name   : ctc_pktxr_ctp_encode_get_outpkt
 * Purpose: get the result packet.
 * Input  : None.
 * Output : outpacket pointer.
 * Return : .
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_ctp_encode_get_outpkt(uint8 **packet, uint16 *pktlen);

/*******************************************************************************
 * Name   : filter_command_set
 * Purpose: handle filter command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set data & mask
 * Output : None.
 * Return : .
 * Note   : user need release its own memory
*******************************************************************************/
ctp_error_e
ctc_pktxr_filter_command_set(const char *keyw, const char *value);

/*******************************************************************************
 * Name   : packet_filter_value
 * Purpose: decode the packet and save the result to dec_pkt.
 * Input  : packet - input packet ponter.
 * Output : None.
 * Return : .
 * Note   : user need release its own memory.
*******************************************************************************/

int32
ctc_pktxr_packet_filter_value(uint8 *packet, uint16 *match, uint8 **got_value);


/******************************************************************************
 * Name   : ctp_tool_set
 * Purpose: determine the tools user used(uml or ixia)
 * Input  : str
 * Output :
 * Return : ret
 * Note   : N/A
 *****************************************************************************/
ctp_error_e
ctc_pktxr_ctp_encode_command_set(const char *keyw, const char *value, void *);

int ctc_pktxr_initialize_ctp_encoder();

#endif
