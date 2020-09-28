/****************************************************************************
* $Id$
*  Message callbacks
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2011-07-25 16:39
* Reason        : First Create.
****************************************************************************/

#ifndef _SCFG_CLIENT_CB_H_
#define _SCFG_CLIENT_CB_H_

typedef int (*scfg_msg_cb_t)(struct message_handler *pms, SCfgMsg_t *);

extern scfg_msg_cb_t scfg_client_cb[op_PR_maxMsg];

#ifndef M_MAX_FILE_SIZE
#define M_MAX_FILE_SIZE     (60 * 1024 * 1024)
#endif
#define M_SCFG_DEPLOY_FILE_MAX_SZ       1048576 /* 1 MBytes */

#define M_SCFG_DEPLOY_INIT_NAME         "init"
#define M_SCFG_DEPLOY_IMAGE_PREFIX      "smartconfig/images"
#define M_SCFG_DEPLOY_CONF_PREFIX       "smartconfig/conf"

#define M_SCFG_DEPLOY_MAC_STR       "mac"
#define M_SCFG_DEPLOY_SN_STR        "sn"
#define M_SCFG_DEPLOY_PRODUCT_STR   "productid"
typedef enum
{
    E_SCFG_DEPLOY_NONE = 0,
    E_SCFG_DEPLOY_MAC,
    E_SCFG_DEPLOY_PRODUCT,
    E_SCFG_DEPLOY_SN
} scfg_deploy_e;

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN 6
#endif
inline unsigned char *scfg_get_sys_mac(void);
inline char *scfg_get_serial_no(void);
inline uint8_t scfg_get_hw_ver(void);
inline uint8_t scfg_get_epld_ver(void);
inline uint8_t scfg_get_scfg_flag(void);

int scfg_init_deploy_start(void);

int scfg_client_tx_initNotify(int sock);

inline void scfg_log(E_SEVERITY eSeverity, const char *fmt, ...);

#endif /* !_SCFG_CLIENT_CB_H_ */
