#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_usw_ip_tunnel.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_USW_IP_TUNNEL_H
#define _SYS_USW_IP_TUNNEL_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "ctc_hash.h"

#include "ctc_ipuc.h"
/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
 enum sys_ip_tunnel_lookup_mode_e
{
    SYS_NAT_HASH_LOOKUP        =0x01,           /* NAT HASH is used to lookup NAT entry */
    SYS_NAT_TCAM_LOOKUP        =0x02           /* NAT HASH is used to lookup NAT entry */
};
typedef enum sys_ip_tunnel_lookup_mode_e sys_ip_tunnel_lookup_mode_t;

 struct sys_ip_tunnel_natsa_info_s
{
    uint16 ad_offset;
    uint16 key_offset;
    uint16 vrf_id;
    uint8  in_tcam;
    uint8  is_tcp_port;

    uint16 l4_src_port;
    uint16 new_l4_src_port;

    ip_addr_t ipv4;
    ip_addr_t new_ipv4;
};
typedef struct sys_ip_tunnel_natsa_info_s sys_ip_tunnel_natsa_info_t;

 struct sys_ip_tunnel_master_s
{
    sal_mutex_t* mutex;
    ctc_hash_t*  nat_hash;
    uint32 max_snat_tcam_num;
    uint32 napt_hash_count; /*napt store in host1 Daport hash*/
    uint32 snat_hash_count; /*snat store in host1 SaPort hash*/
    uint32 snat_tcam_count; /*snat store in tcam*/
    uint8 lookup_mode[MAX_CTC_IP_VER];
    uint8 opf_type_ipv4_nat;
};
typedef struct sys_ip_tunnel_master_s sys_ip_tunnel_master_t;

#define SYS_IPUC_NAT_KEY_MAP(p_ctc_nat, p_sys_nat)        \
    {                                                       \
        (p_sys_nat)->vrf_id = (p_ctc_nat)->vrf_id;        \
        (p_sys_nat)->l4_src_port = (p_ctc_nat)->l4_src_port;      \
        sal_memcpy(&(p_sys_nat->ipv4), &(p_ctc_nat->ipsa.ipv4),    \
                   sizeof(ip_addr_t)); \
        if ((p_sys_nat)->l4_src_port > 0)   \
        {   \
            (p_sys_nat)->is_tcp_port = ((p_ctc_nat)->flag & CTC_IPUC_NAT_FLAG_USE_TCP_PORT) ? 1:0; \
        }   \
    }

#define SYS_IPUC_NAT_DATA_MAP(p_ctc_nat, p_sys_nat)                            \
    {                                                                            \
        (p_sys_nat)->new_l4_src_port = (p_ctc_nat)->new_l4_src_port;      \
        sal_memcpy(&(p_sys_nat->new_ipv4), &(p_ctc_nat->new_ipsa.ipv4),    \
                   sizeof(ip_addr_t)); \
    }

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_ip_tunnel_add_natsa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param);

extern int32
sys_usw_ip_tunnel_remove_natsa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param);

extern int32
sys_usw_ip_tunnel_add_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param);

extern int32
sys_usw_ip_tunnel_remove_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param);

extern int32
sys_usw_ip_tunnel_set_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param, uint8 hit);

extern int32
sys_usw_ip_tunnel_get_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param, uint8* hit);

extern int32
sys_usw_ip_tunnel_init(uint8 lchip);

extern int32
sys_usw_ip_tunnel_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

#endif

