
#include "hsrv_inc.h"
#include "hsrv_msg_if.h"
#include "glb_message_define.h"

int32
hsrv_msg_notify_link_status(void* arg)
{  
    fei_if_phy_state_notify_t notify;
    sal_memcpy(&notify, arg, sizeof(notify));

    HSRV_IF_DEBUG("HSRV hsrv_msg_notify_link_status,interface %s, ifindex %d, updown %d", 
        notify.ifname, notify.ifindex, notify.link_up);

    
    /* send to switch */
    return(HSRV2SWITCH_MSG_SEND_NOTIFY(SWITCH_FEI_PHY_NOTIFY, &notify, sizeof(fei_if_phy_state_notify_t)));
    //return 0;
}


