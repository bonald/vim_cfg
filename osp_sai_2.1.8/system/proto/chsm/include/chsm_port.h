#ifndef __CHSM_PORT_H__
#define __CHSM_PORT_H__

#define CHSM_MANG_VRF 64

struct manage_rt_info_s
{
    struct sal_in4_addr net;
    struct sal_in4_addr net_mask;
    struct sal_in4_addr gw;
};
typedef struct manage_rt_info_s manage_rt_info_t;

#ifdef HAVE_IPV6
struct manage_rt_ipv6_info_s
{
    struct sal_in6_addr net6;
    struct sal_in6_addr net_mask_6;
    struct sal_in6_addr gw6;
};
typedef struct manage_rt_ipv6_info_s manage_rt_ipv6_info_t;

struct chsm_nlsock
{
  int sock;
  int seq;
  struct sockaddr_nl snl;
  char *name;
  struct thread *t_read;
};


#endif /*HAVE_IPV6*/

int32 
chsm_port_create(tbl_port_t* p_port);
int32 
chsm_port_dump_transceiver(char* ifname, uint16 slot, uint16 port, uint16 sub_port, uint8 detail, cfg_cmd_para_t *para, uint8 raw);
int32
chsm_port_dump_transceiver_json(char* ifname, uint16 slot, uint16 port, uint16 sub_port, uint8 detail, cfg_cmd_para_t *para, uint8 raw);
#endif

