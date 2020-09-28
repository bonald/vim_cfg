/**
 @file glb_message_define.h
 
 @date 2012-03-24
 
 @version v1.0
 
The file define  all CNOS module's  common Macros .

*/

#ifndef __GLB_MESSAGE_DEFINE_H__
#define __GLB_MESSAGE_DEFINE_H__

struct glb_message_nl_memchk_s
{
    unsigned int thresh0;/*free memory low*/
    unsigned int thresh1;/*free memory middle*/
    unsigned int thresh2;/*free memory high*/
    unsigned int status;   
};
typedef struct glb_message_nl_memchk_s glb_message_nl_memchk_t;


enum glb_nl_memchk_tx_e
{
    GLB_NL_MEMCHK_TX_SET_THRESH,
    GLB_NL_MEMCHK_TX_GET_STATUS,
    GLB_NL_MEMCHK_TX_SET_MAX
};
typedef enum glb_nl_memchk_tx_e glb_nl_memchk_tx_t;

enum glb_nl_memchk_rx_e
{
    GLB_NL_MEMCHK_RX_TYPE0 = 11, /* NLMSG_MIN_TYPE + 1 */
    GLB_NL_MEMCHK_RX_NOTIFY_CHANGES,
    
    GLB_NL_MEMCHK_RX_MAX = 32
};
typedef enum glb_nl_memchk_rx_e glb_nl_memchk_rx_t;

enum ctclib_nlmsg_sock_group_e
{
    CTCLIB_NLMSG_GROUP_MEMCHK_STATUS = (1 << 0),
};
typedef enum ctclib_nlmsg_sock_group_e ctclib_nlmsg_sock_group_t;

#define SIOC_GET_PHY_TYPE   (SIOCDEVPRIVATE + 0) /* get phy type */
#define SIOC_GET_PHY_STATE  (SIOCDEVPRIVATE + 1) /* get phy state */
#define SIOC_UPD_PHY_STATE  (SIOCDEVPRIVATE + 2) /* update phy state */
#define SIOC_UPD_PHY_TYPE   (SIOCDEVPRIVATE + 3) /* update phy type */

#define NSM_BINDING_TABLE_CMD_ADD           0 /* add an entry */
#define NSM_BINDING_TABLE_CMD_DEL           1 /* delete an entry */
#define NSM_BINDING_TABLE_CMD_UPDATE        2 /* update time for an entry */
#define NSM_BINDING_TABLE_CMD_FLUSH         3 /* clean all entry */

#endif
