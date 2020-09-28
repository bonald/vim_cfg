#ifndef __HSRV_IF_H__
#define __HSRV_IF_H__

typedef uint32  hsrv_if_flag_t; 

enum hsrv_port_of_flag_e 
{
    IF_PORT_OPENFLOW_FLAG_ENABLE               =     (1 << 0), 
    IF_PORT_OPENFLOW_FLAG_ADD_TO_BR0           =     (1 << 1), 
    IF_PORT_OPENFLOW_FLAG_SLAVE                =     (1 << 2), 
    IF_PORT_OPENFLOW_FLAG_MAX
};
typedef enum hsrv_port_of_flag_e hsrv_port_of_flag_t;

enum hsrv_ifpm_id_e
{
    HSRV_PMID_OPENFLOW,
    HSRV_PMID_MAX
};
typedef enum hsrv_ifpm_id_e hsrv_ifpm_id_t;

int32
_hsrv_if_set_kernel_logic_phy_state(char *ifname, uint32 is_running);

int32
_hsrv_if_set_kernel_phy(char *ifname, uint32 is_running);

#endif /* !__HSRV_IF_H__ */

