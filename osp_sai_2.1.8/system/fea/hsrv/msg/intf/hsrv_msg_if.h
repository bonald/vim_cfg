#ifndef __HSRV_MSG_IF_H__
#define __HSRV_MSG_IF_H__

typedef struct
{
    char        ifname[FEI_NAME_LEN];
    uint32      ifindex;
    uint32      speed;
    uint32      duplex;
    uint32      link_up;
    uint32      flowctrl_send;
    uint32      flowctrl_recv;
    uint32      media;
} fei_if_phy_state_notify_t;

#endif /* !__HSRV_MSG_IF_H__ */

