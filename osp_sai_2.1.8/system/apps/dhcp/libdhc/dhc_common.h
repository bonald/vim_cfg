/****************************************************************************
* $Id$
*  Common process for dhclient
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-05 15:19
* Reason        : First Create.
****************************************************************************/

#ifndef _DHC_COMMON_H_
#define _DHC_COMMON_H_

#include "lib.h"

/* be careful when change this value */
#define DHC_DISP_BUF_SIZE                   1024
#define DHC_ERR_MSG_BUF_LEN                 256
#define DHC_MAX_ROUTERS_RECORDS             8
#define DHC_PREFIX_BUF_LEN                  64
#define DHC_MAX_TFTPSRV_BUF_LEN             128
#define DHC_MAX_TFTPSRV_RECORDS             8

#define DHC_MAX_OPTION_3_BYTES         32 /* support 8 records */
#define DHC_MAX_OPTION_33_BYTES        64 /* support 8 records */
#define DHC_MAX_OPTION_121_BYTES       72 /* support 8 records */
#define DHC_MAX_OPTION_150_BYTES       32 /* support 8 records */
#define DHC_ROUTERS_MAX_LEN            256 /* routers buffer length */

#define DHC_RELEASE_WAIT_INTERVAL       1500 /* msec */

#define DHC_MAX_DISTANCE                255

/* hold routers parse result */
typedef struct dhc_comm_routers_rec
{
    int mask_width;
    prefix_ipv4_t dest;
    prefix_ipv4_t router;
} dhc_comm_routers_rec_t;

typedef struct dhc_comm_ip_cb_arg
{
    char *errmsg;
    void *data; /* argument for callback functions */
} dhc_comm_ip_cb_arg_t;

typedef u_int32_t  cindex_t;

typedef struct nsm_msg_dhc_bound
{
    cindex_t cindex;
#define NSM_DHC_BOUND_CTYPE_IFNAME         0
#define NSM_DHC_BOUND_CTYPE_VRFID          1
#define NSM_DHC_BOUND_CTYPE_IPADDR         2
#define NSM_DHC_BOUND_CTYPE_TFTPSRV_IPADDR 3
#define NSM_DHC_BOUND_CTYPE_ROUTERS        4 /* Routers 3 */
#define NSM_DHC_BOUND_CTYPE_CFS_ROUTERS    5 /* classful static routers 33 */
#define NSM_DHC_BOUND_CTYPE_CLS_ROUTERS    6 /* classless static routers 121/249 */

    int ifname_len;
    char *ifname;

    int32_t vrfid;
    u_int32_t protocol_id;

    u_int8_t distance; /* default-router distance */

    char *ip; /* client ip, sockunion */
    char *mask; /* client subnet mask, sockunion */
    int tftpsrv_ip_len; /* tftp server ip addresses len */
    char *tftpsrv_ip; /* Format 1.1.1.1,2.2.2.2,3.3.3.3 */

    int routers_len;
    char *routers; /* option 3 */

    int classful_routers_len;
    char *classful_routers; /* option 33 */

    int classless_routers_len;
    char *classless_routers; /* option 121 or 249 */
} nsm_msg_dhc_bound_t;

typedef struct dhc_comm_rt_cb_arg
{
    char *errmsg;
    void *data; /* argument for callback functions */
    nsm_msg_dhc_bound_t *pbound;
    
    /* callback functions */
    int (*cb_install_static_router)(struct dhc_comm_rt_cb_arg *args, 
            dhc_comm_routers_rec_t *prec, unsigned char distance);
} dhc_comm_rt_cb_arg_t;

#if 0
int
dhc_comm_encode_bound_msg(nsm_msg_dhc_bound_t *pmsg, u_int8_t **buf, u_int16_t *size,
            int (*pencode_func)(u_int8_t **, u_int16_t *, nsm_msg_dhc_bound_t *));

int
dhc_comm_install_ip_address(nsm_msg_dhc_bound_t *pbound, 
        dhc_comm_ip_cb_arg_t *proc);

int
dhc_comm_install_classless_routers(
        nsm_msg_dhc_bound_t *pbound,
        dhc_comm_rt_cb_arg_t *proc);

int
dhc_comm_install_classful_routers(
        nsm_msg_dhc_bound_t *pbound,
        dhc_comm_rt_cb_arg_t *proc);

int
dhc_comm_install_static_routers(
        nsm_msg_dhc_bound_t *pbound,
        dhc_comm_rt_cb_arg_t *proc);
int
dhc_comm_install_ip_routers(nsm_msg_dhc_bound_t *pbound,
        dhc_comm_rt_cb_arg_t *proc);

int
dhc_comm_send_notify(void *nch, int what, int result, 
        char *psz_errmsg, size_t errmsg_len,
        char *ifname, s_int32_t vrfid);

int
dhc_comm_send_cmd(void *hdl, int what, char *psz_reason, 
        size_t reason_len, char *ifname, s_int32_t vrfid);
#endif
#endif /* !_DHC_COMMON_H_ */
