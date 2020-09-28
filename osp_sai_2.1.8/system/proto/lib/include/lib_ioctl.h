
#ifndef __LIB_IOCTL_H__
#define __LIB_IOCTL_H__

int32
lib_ioctl_if_add_if(const char *ifname);

int32
lib_ioctl_if_add_if_loopback(const char *ifname);

int32
lib_ioctl_if_set_mtu(const char *ifname, uint32 mtu);

int32
lib_ioctl_if_set_hw_addr(const char *ifname, uint8 *addr, uint32 addr_len);

int32
lib_ioctl_if_set_flags(const char *ifname, uint32 flags, uint32 is_set);

int32
lib_ioctl_if_set_up(const char *ifname, uint32 is_set);

int32
lib_ioctl_if_set_up_running(const char *ifname, uint32_t is_set, uint32_t *flags);

int32
lib_ioctl_if_set_running(const char *ifname, uint32 is_set);

int32
lib_ioctl_if_set_phy_state(const char *ifname, glb_phy_state_t *state);

int32
lib_ioctl_if_set_phy_type(const char *ifname, uint32 *type);

uint32
lib_ioctl_if_get_flags(const char *ifname);

uint32
lib_ioctl_if_get_flags_and_check(const char *ifname);

int32
lib_ioctl_if_get_phy_type(const char *ifname, uint32 *type);

int32
lib_ioctl_if_get_phy_state(const char *ifname, glb_phy_state_t *state);

int32
lib_ioctl_manage_if_set_addr(int32 fd, const char* name, int32 cmd, addr_ipv4_t addr);

#endif /* !__LIB_IOCTL_H__ */

