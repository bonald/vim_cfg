
#include "hsrv_inc.h"
#include "lib_ioctl.h"
#include "hsrv_msg_if.h"
#include "hsrv_if.h"
#include "hsrv_hostif.h"

#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"

enum hsrv_if_com_flag_e {
    IF_COM_FLAG_UP = 0x00000001,
    IF_COM_FLAG_RUNNING = 0x00000002,
};
typedef enum hsrv_if_com_flag_e hsrv_if_com_flag_t;


int32
_hsrv_if_mapping_flag(uint32 hal_flag, hsrv_if_flag_t* p_hsrv_flag);
int32
_hsrv_if_set_kernel_phy(char *ifname, uint32 state_change);
int32
_hsrv_if_set_kernel_logic_phy_state(char *ifname, uint32 is_running);
#if 0
int32
_hsrv_if_set_kernel_phy(char *ifname, uint32 state_change)
{
    glb_phy_state_t phy_state;
    //uint32 media_type = 0;
    
    sal_memset(&phy_state, 0, sizeof(phy_state));
    //HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_phy_state_by_name(ifname, &phy_state, &media_type));
    //TODO:
    
    if (state_change)
    {
        /*set speed, duplex and then send to kernel*/
        HSRV_IF_ERROR_RETURN(lib_ioctl_if_set_phy_state(ifname, &phy_state));
    }
    //HSRV_IF_ERROR_RETURN(lib_ioctl_if_set_phy_type(ifname, &media_type));
    
    return HSRV_E_NONE;
}
#endif

int32
_hsrv_if_set_kernel_phy(char *ifname, uint32 is_running)
{
    glb_phy_state_t phy_state;
    
    sal_memset(&phy_state, 0, sizeof(phy_state));
    if (is_running)
    {
        phy_state.link_up = TRUE;
    }
    
    HSRV_IF_ERROR_RETURN(lib_ioctl_if_set_phy_state(ifname, &phy_state));

    
    return HSRV_E_NONE;
}

int32
_hsrv_if_set_kernel_logic_phy_state(char *ifname, uint32 is_running)
{
    glb_phy_state_t phy_state;
    
    sal_memset(&phy_state, 0, sizeof(phy_state));
    if (is_running)
    {
        phy_state.link_up = TRUE;
    }
    
    HSRV_IF_ERROR_RETURN(lib_ioctl_if_set_phy_state(ifname, &phy_state));

    return HSRV_E_NONE;
}

int32
_hsrv_if_mapping_flag(uint32 hal_flag, hsrv_if_flag_t* p_hsrv_flag)
{
    /*do pointer check in high level API*/
    if (GLB_IFF_UP == hal_flag)
    {
        *p_hsrv_flag = IF_COM_FLAG_UP;
    }
    else if (GLB_IFF_RUNNING == hal_flag)
    {
        *p_hsrv_flag = IF_COM_FLAG_RUNNING;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

