#ifndef __RT_IF_H__
#define __RT_IF_H__

#define RT_IF_CMD_CHECK_RT_IF(_ifname_, _para_, _pp_if_, _pp_rt_if_) \
do { \
    rc = rt_if_cmd_check_l3_cmd(_ifname_, _para_, _pp_if_, _pp_rt_if_); \
    if (rc < 0) \
    { \
        return rc; \
    } \
} while (0)

int32
rt_if_set_mode(char *name, uint32 mode);

int32
rt_if_add_if(char *name);

int32
rt_if_del_if(char *name);

int32
rt_if_del_if_addr(char *ifname);

int32
rt_if_set_mtu(char *ifname, uint32 mtu);

int32 
rt_if_mtu_update_agg_member(tbl_interface_t *pst_interface, uint32 mtu);

int32
rt_if_set_arp_retry_interval(char *ifname, uint32 retry_interval);

int32
rt_if_set_arp_timeout(char *ifname, uint32 timeout);

int32
rt_if_after_add_agg_member (tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

int32
rt_if_check_before_add_agg_member(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

int32
rt_if_before_destroy_agg(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

#endif /* !__RT_IF_H__ */

