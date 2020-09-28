/****************************************************************************
* $Id: dhcrelay_func.h,v 1.1 2007/01/19 06:10:33 xliu Exp $
*  functions for DHCP relay
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-09 18:41
* Reason        : First Create.
****************************************************************************/

#ifndef _DHCRELAY_FUNC_H_
#define _DHCRELAY_FUNC_H_

#include "bsd_queue.h"
#include "param_check.h"
#include <net/if.h>

#define DHCP_RELAY_MAX_GROUP        16
#define DHCP_RELAY_MAX_SERVER       8

/* service switch */
#define DHCP_SERVICE_FLAG           0x01
#define DHCP_RELAY_SERVICE_FLAG     0x02

/* agent relay policy */
typedef enum {
	forward_and_append,	/* Forward and append our own relay option. */
	forward_and_replace,	/* Forward, but replace theirs with ours. */
	forward_untouched,	/* Forward without changes. */
	discard
} E_RelayPolicy;
#define E_DEFAULT_RELAY_POLICY      forward_untouched

extern void init_fallback_interface(void);
extern struct interface_info *dhcprelay_register_interface(char *name);
extern void dhcprelay_unregister_interface(char *name);
extern int dhcprelay_service_onoff(unsigned char bOn);

int dhcprelay_recv_interface_add(char *pszIntfName, int isLayer3);
int dhcprelay_recv_interface_delete(char *pszIntfName);

extern void dhcprelay_send_bootrequest(struct interface_info *pInfo,
									   struct interface_info *pRawInfo,
									   struct packet *packet,
									   struct dhcp_packet *raw,
									   size_t len, struct in_addr from,
									   int *pnErrCnt, int *pnRelayedCnt);

extern int dhcprelay_trusted_interface(struct interface_info *pInfo);
extern int dhcprelay_update_primary_address(struct interface_info *pInfo, uint32 *addr);
extern void dhcprelay_signal_init();

extern void dhcprelay_dump_packet(struct dhcp_packet *tp, int len);

/* modified by liwh for bug 43886, 2018-11-06 */
#ifdef _CENTEC_
int dhcprelay_delete_primary_address(struct interface_info *pInfo);
#endif
/* liwh end */

int dhcprelay_send_msg_to_snooping(struct dhcp_packet *packet, int length,
								   struct interface_info *ifp);
int dhcrelay_pdu_rx(int sock, int sess_fd, ctc_msg_t *p_msg);

#endif							/* !_DHCRELAY_FUNC_H_ */
