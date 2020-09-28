
#include "proto.h"
#include "lib_ioctl.h"
#include "glb_message_define.h"
#include <linux/netdevice.h>
#include <linux/if_tun.h>

static int32
_lib_ioctl_if_ioctl(int32 cmd, caddr_t buf)
{

    int32 sock = -1;
    int32 rc = PM_E_NONE;

    sock = sal_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        return PM_E_FAIL;
    }

    rc = ioctl(sock, cmd, buf);
    sal_close(sock);

    return rc;
}

int32
lib_ioctl_if_add_if(const char *ifname)
{
    struct ifreq ifr;
    int fd, err;

    char *clonedev = "/dev/net/tun";

    /* Arguments taken by the function:
    *
    * char *dev: the name of an interface (or '\0'). MUST have enough
    * space to hold the interface name if '\0' is passed
    * int flags: interface flags (eg, IFF_TUN etc.)
    */
    /* open the clone device */

    if( (fd = sal_open(clonedev, O_RDWR)) < 0 )
    {
        return fd;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    sal_memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP; /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
    if (*ifname)
    {
        /* if a device name was specified, put it in the structure; otherwise,
        * the kernel will try to allocate the "next" device of the
        * specified type */
        sal_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    }

    /* try to create the device */
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 )
    {
        close(fd);
        return err;
    }

    return fd;
}

int32
lib_ioctl_if_add_if_loopback(const char *ifname)
{
    struct ifreq ifr;
    int fd, err;

    char *clonedev = "/dev/net/tun";

    /* Arguments taken by the function:
    *
    * char *dev: the name of an interface (or '\0'). MUST have enough
    * space to hold the interface name if '\0' is passed
    * int flags: interface flags (eg, IFF_TUN etc.)
    */
    /* open the clone device */

    if( (fd = sal_open(clonedev, O_RDWR)) < 0 )
    {
        return fd;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    sal_memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_LOOPBACK | IFF_NOARP;
    if (*ifname)
    {
        /* if a device name was specified, put it in the structure; otherwise,
        * the kernel will try to allocate the "next" device of the
        * specified type */
        sal_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    }

    /* try to create the device */
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 )
    {
        close(fd);
        return err;
    }

    return fd;
}


int32
lib_ioctl_if_set_mtu(const char *ifname, uint32 mtu)
{
    struct ifreq ifreq;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);

    ifreq.ifr_mtu = mtu;

    return _lib_ioctl_if_ioctl(SIOCSIFMTU, (caddr_t)&ifreq);
}

int32
lib_ioctl_if_set_hw_addr(const char *ifname, uint8 *addr, uint32 addr_len)
{
    struct ifreq ifreq;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);

    ifreq.ifr_hwaddr.sa_family = 1  /*ARPHRD_ETHER*/;
    memcpy(ifreq.ifr_hwaddr.sa_data, addr, addr_len);

    return _lib_ioctl_if_ioctl(SIOCSIFHWADDR, (caddr_t)&ifreq);
}

int32
lib_ioctl_if_set_flags(const char *ifname, uint32 flags, uint32 is_set)
{
    struct ifreq ifreq;
    int32 rc = PM_E_NONE;

    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    rc = _lib_ioctl_if_ioctl(SIOCGIFFLAGS, (caddr_t)&ifreq);
    if (rc < 0)
    {
        return rc;
    }

    if (is_set)
    {
        ifreq.ifr_flags |= flags;
    }
    else
    {
        ifreq.ifr_flags &= ~flags;
    }

    return _lib_ioctl_if_ioctl(SIOCSIFFLAGS, (caddr_t)&ifreq);
}

int32
lib_ioctl_if_set_up(const char *ifname, uint32 is_set)
{
    struct ifreq ifreq;
    int32 rc = PM_E_NONE;

    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    rc = _lib_ioctl_if_ioctl(SIOCGIFFLAGS, (caddr_t)&ifreq);
    if (rc < 0)
    {
        return rc;
    }

    if (is_set)
    {
        ifreq.ifr_flags |= IFF_UP;
    }
    else
    {
        ifreq.ifr_flags &= ~IFF_UP;
    }

    return _lib_ioctl_if_ioctl(SIOCSIFFLAGS, (caddr_t)&ifreq);
}

int32
lib_ioctl_if_set_up_running(const char *ifname, uint32 is_set, uint32_t *flags)
{
    struct ifreq ifreq;
    int32 rc = PM_E_NONE;

    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    rc = _lib_ioctl_if_ioctl(SIOCGIFFLAGS, (caddr_t)&ifreq);
    if (rc < 0)
    {
        return rc;
    }

    if (is_set)
    {
        ifreq.ifr_flags |= IFF_RUNNING;
    }
    else
    {
        ifreq.ifr_flags &= ~IFF_RUNNING;
    }

    *flags = ifreq.ifr_flags;

    return rc;
}

int32
lib_ioctl_if_set_running(const char *ifname, uint32 is_set)
{
    glb_phy_state_t state;

    sal_memset(&state, 0, sizeof(state));
    state.link_up = is_set;
    return lib_ioctl_if_set_phy_state(ifname, &state);;
}

int32
lib_ioctl_if_set_phy_state(const char *ifname, glb_phy_state_t *state)
{
    struct ifreq ifreq;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    ifreq.ifr_data = (void *)state;

    return _lib_ioctl_if_ioctl(SIOC_UPD_PHY_STATE, (caddr_t)&ifreq);
}

int32
lib_ioctl_if_set_phy_type(const char *ifname, uint32 *type)
{
    struct ifreq ifreq;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    ifreq.ifr_data = (void *)type;

    return _lib_ioctl_if_ioctl(SIOC_UPD_PHY_TYPE, (caddr_t)&ifreq);
}

uint32
lib_ioctl_if_get_flags(const char *ifname)
{
    struct ifreq ifreq;
    int32 rc = 0;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);

    rc = _lib_ioctl_if_ioctl(SIOCGIFFLAGS, (caddr_t)&ifreq);
    if (rc < 0)
    {
        return 0;
    }

    return (ifreq.ifr_flags & 0x0000FFFF);
}

uint32
lib_ioctl_if_get_flags_and_check(const char *ifname)
{
    struct ifreq ifreq;
    int32 rc = 0;
    uint32 is_up_and_run = 0;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);

    rc = _lib_ioctl_if_ioctl(SIOCGIFFLAGS, (caddr_t)&ifreq);
    if (rc < 0)
    {
        return 0;
    }

    if (ifreq.ifr_flags & IFF_RUNNING && ifreq.ifr_flags & IFF_UP)
    {
        is_up_and_run = 1;
    }

    return is_up_and_run;
}


int32
lib_ioctl_if_get_phy_type(const char *ifname, uint32 *type)
{
    struct ifreq ifreq;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    ifreq.ifr_data = (void *)type;

    return _lib_ioctl_if_ioctl(SIOC_GET_PHY_TYPE, (caddr_t)&ifreq);
}

int32
lib_ioctl_if_get_phy_state(const char *ifname, glb_phy_state_t *state)
{
    struct ifreq ifreq;

    sal_memset(&ifreq, 0, sizeof(ifreq));
    sal_strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    ifreq.ifr_data = (void *)state;

    return _lib_ioctl_if_ioctl(SIOC_GET_PHY_STATE, (caddr_t)&ifreq);
}

int32
lib_ioctl_manage_if_set_addr(int32 fd, const char* name, int32 cmd, addr_ipv4_t addr)
{
    struct ifreq ifreq;
    struct sockaddr_in sin;

    sal_strncpy(ifreq.ifr_name, name, IFNAMSIZ);
    sal_memset(&sin, 0, sizeof(struct sockaddr));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = addr.s_addr;
    sal_memcpy(&ifreq.ifr_addr, &sin, sizeof(struct sockaddr));

    if (ioctl(fd, cmd, &ifreq) < 0)
    {
        return PM_E_IOCTL_FAIL;
    }

    return PM_E_NONE;
}

