
#ifndef __NAT_H__
#define __NAT_H__


#define NAT_LOG_EMERG(fmt, args...)    log_sys(M_MOD_NAT, E_EMERGENCY, fmt, ##args)
#define NAT_LOG_ALERT(fmt, args...)    log_sys(M_MOD_NAT, E_ALERT, fmt, ##args)
#define NAT_LOG_CRIT(fmt, args...)     log_sys(M_MOD_NAT, E_CRITICAL, fmt, ##args)
#define NAT_LOG_ERR(fmt, args...)      log_sys(M_MOD_NAT, E_ERROR, fmt, ##args)
#define NAT_LOG_WARN(fmt, args...)     log_sys(M_MOD_NAT, E_WARNING, fmt, ##args)
#define NAT_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_NAT, E_NOTICE, fmt, ##args)
#define NAT_LOG_INFO(fmt, args...)     log_sys(M_MOD_NAT, E_INFORMATIONAL, fmt, ##args)

/*Add by zhush for bug 52916, 2019-07-26*/
#define NAT_SHOW_TRANS_OPER_DETAIL      1
#define NAT_SHOW_TRANS_OPER_SUMMARY     2
/*end zhush*/

bool nat_api_debug_is_on(uint32 flag);

#define NAT_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (nat_api_debug_is_on(flag)) \
        log_sys(M_MOD_ND, E_DEBUG, fmt, ##args); \
} while (0)

#define NAT_EVENT_DEBUG(fmt, args...) \
do { \
    NAT_LOG_DEBUG(RTDBG_FLAG_NAT_EVENT, fmt, ##args); \
} while(0)

#define NAT_TIMER_DEBUG(fmt, args...) \
do { \
    NAT_LOG_DEBUG(RTDBG_FLAG_NAT_TIMER, fmt, ##args); \
} while(0)

#define NAT_NFT_CHAIN        "/tmp/nat_nft_chain"


// TODO: tmp use, should to be changed to system define
#ifndef _IS_BIT_SET
#define _IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#endif
    
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif


#ifndef CTC_IPV4_ADDR_LEN_IN_BIT
#define CTC_IPV4_ADDR_LEN_IN_BIT        32          /**< IPv4 address length in bit */
#endif

#ifndef IPV4_LEN_TO_MASK
#define IPV4_LEN_TO_MASK(mask, len)  \
    {                           \
        (mask) = (len) ? ~((1 << (CTC_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0; \
    }
#endif

#ifndef IPV4_MASK_TO_LEN
#define IPV4_MASK_TO_LEN(mask, len) \
    { \
        int32 i; \
        len = 0; \
        for (i = 31; i >= 0; i--) \
        { \
            if (_IS_BIT_SET((mask), (uint32)i)) \
            {(len)++; } \
            else \
            {; } \
        } \
    }
#endif

typedef struct {

  uint8 family;
  uint8 bDnat:1,
        bSnat:1,
        bSPat:1,
        bDpat:1,
        bResv:4;
  uint16 ip_proto;/* IPPROTO_TCP or IPPROTO_UDP...*/
  uint32 timeout;
  uint32 mark;

  char helper[32];
  struct {
      addr_t sip;
      addr_t dip;
      uint16 sport;
      uint16 dport;
  }orig;

  struct {
      addr_t sip;
      addr_t dip;
      uint16 sport;
      uint16 dport;
  }replace;

  struct {
      uint32 bytes;
      uint32 packets;
      uint32 s;
  }counts;

  struct {
    uint8 state;
    char resv[3];
  }tcp;

  struct {
      uint8 type;
      uint8 code;
      uint16 ID;
  }icmp;
}nat_nfct_s;

typedef struct {
    uint8  ip_proto;/* IPPROTO_TCP or IPPROTO_UDP...*/
    uint8  resv;
    uint16 local_port;
    uint16 global_port;

    addr_t local_ip;
    addr_t global_ip;

    char   oifname[IFNAME_SIZE];
    char   iifname[IFNAME_SIZE];
}nat_translations_show_filter_s;

typedef struct {
    uint32 argc;
    void* argv[4];
}nat_nfct_hook_s;

#if _DESC("nat session functions")
extern void nat_nfct_reg();
extern int32 nat_nfct_hook_flush_by_new_rule(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp);
extern int32 nat_nfct_hook_dump_alg(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp);
extern int32 nat_nfct_hook_clear_session(enum nf_conntrack_msg_type nf_type, struct nf_conntrack *ct, void *tmp);
int32 nat_nfct_scan(int32 (*fn_hook)(enum nf_conntrack_msg_type , struct nf_conntrack *, void *), nat_nfct_hook_s *p_data);
extern void nat_session_global_ttl_init();
extern uint32 nat_session_ttl_get(uint8 ip_proto);
extern int32 nat_session_aging_hook(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);
extern void nat_session_struct_init(tbl_nat_session_t *p_session);
extern int32 nat_session_add(tbl_nat_session_t *p_session);
extern int32 nat_session_del(tbl_nat_session_t *p_session, bool if_need_notify_kernel);
extern void nat_session_spec_init();
extern void nat_session_clear_by_oif(tbl_route_if_t *p_db_rt_if);
extern void nat_session_clear_by_iif(tbl_route_if_t        *p_db_rt_if);
extern void nat_session_clear_all();
extern int32 nat_session_tbl_process_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);

#endif

#if _DESC("nat rule functions")
extern void nat_nft_init();
extern int32 nat_nft_connlimit_set(tbl_nat_session_limit_t *p_session_limit_node, uint32 limit);
extern int32 nat_nft_connlimit_unset(tbl_nat_session_limit_t *p_session_limit);
extern int32 nat_nft_connlimit_set_global(uint32 limit);
extern void nat_rule_struct_init(tbl_nat_rule_t *p_rule);
extern int32 nat_rule_add(tbl_nat_rule_t *p_rule);
extern int32 nat_rule_del(tbl_nat_rule_t *p_db_rule);
extern bool nat_rule_if_local_addr_port_used(tbl_nat_rule_t *p_new_rule);
extern bool nat_rule_if_global_addr_port_used(tbl_nat_rule_t *p_new_rule);
extern bool nat_rule_if_outside_rtif_port_used(tbl_route_if_t *p_rt_if, uint16 port_lo, uint16 port_hi);
extern bool nat_pool_if_addr_used(addr_ipv4_t *p_addr, char *pool_name);
extern void nat_pool_struct_init(tbl_nat_pool_t *p_pool);
extern int32 nat_pool_add(tbl_nat_pool_t *p_pool);
extern int32 nat_pool_del(tbl_nat_pool_t *p_db_pool);
extern int32 nat_if_set(tbl_route_if_t         *p_db_rt_if, uint8 enable_old, uint8 enable_new);
extern int32 nat_pool_proc_rtif_addr_add(tbl_route_if_t      *p_rt_if, prefix_t *p_prefix);
extern int32 nat_pool_proc_rtif_addr_del(tbl_route_if_t      *p_rt_if, prefix_t *p_prefix);
extern int32 nat_rule_proc_rtif_addr_add(tbl_route_if_t *p_rt_if, prefix_t *p_prefix);
extern int32 nat_rule_proc_rtif_addr_del(tbl_route_if_t *p_rt_if, prefix_t *p_prefix);
extern int32 nat_pool_get_outside_rtif(tbl_nat_pool_t *p_pool, addr_ipv4_t *p_except_addr, char *if_name);
extern bool nat_rule_if_nfct_related(nat_nfct_s *p_nfct, tbl_nat_rule_t *p_rule);
#endif

#if _DESC("nat cmd functions")
extern bool nat_get_rtif_by_ip(addr_ipv4_t *addr_ip, addr_ipv4_t *except_addr_ip, tbl_route_if_t **pp_rt_if, uint8 *p_prefixlen);
extern bool nat_get_rtif_by_iproute(addr_ipv4_t *addr_ip, tbl_route_if_t **pp_rt_if, uint8 *p_prefixlen);
extern ds_connected_t *nat_get_rtif_primary_connected_by_ifname(char* if_name);
extern char *nat_l3protocol_name(char *str, uint32 len, uint16 l3proto);
extern void nat_cmd_init();
extern bool nat_cmd_translations_filter_session(tbl_nat_session_t *p_session, nat_translations_show_filter_s *p_filter);
#endif

#if _DESC(" external functions ")
extern int sal_prefix_same(const prefix_t *p1, const prefix_t *p2);
extern int ctc_nfct_catch(struct nfct_handle *h);
#endif

#endif /* !__NAT_H__ */

