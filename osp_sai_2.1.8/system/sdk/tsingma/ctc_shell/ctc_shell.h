/**
 @file ctc_master_cli.h

 @date 2014-12-22

 @version v2.0

 This file define the types used in APIs

*/

#ifndef _CTC_MASTER_CLI_H
#define _CTC_MASTER_CLI_H
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
 @defgroup ctc_master CTC_MASTER
 @{
*/
/**
 @brief define netlink protocol
*/
#define CTC_SDK_NETLINK 		20

/**
 @brief define netlik msg length
*/
#define CTC_SDK_NETLINK_MSG_LEN 9600

/**
 @brief define tcp port
*/
#define CTC_SDK_TCP_PORT 		8100

/**
 @brief define CMD
*/
#define CTC_SDK_CMD_QUIT        0x0001

struct ctc_msg_hdr {
	uint32_t		msg_len;		/* Length of message including header */
	uint16_t		msg_type;		/* Message content */
	uint16_t		msg_flags;		/* Additional flags */
	uint32_t		msg_turnsize;	/* Ture size */
	uint32_t		msg_pid;		/* Sending process port ID */
};

/**
 @brief packet
*/
typedef struct ctc_sdk_packet_s
{
    struct 	ctc_msg_hdr hdr;
    char 				msg[CTC_SDK_NETLINK_MSG_LEN];
}ctc_sdk_packet_t;

/**@} end of @defgroup ctc_master  */

#ifdef __cplusplus
}
#endif

#endif
