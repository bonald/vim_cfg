
#ifndef __IF_AGG_H__
#define __IF_AGG_H__

#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"

typedef enum
{
    IF_AGG_CB_MODULE_FDB = 0,
    IF_AGG_CB_MODULE_L2MCAST,
    IF_AGG_CB_MODULE_SECURITY,
    IF_AGG_CB_MODULE_SMART_LINK,
    IF_AGG_CB_MODULE_MONITOR_LINK,
    IF_AGG_CB_MODULE_INTF_TPID,
    IF_AGG_CB_MODULE_IF_MTU, /* modified by huangxt for bug 19814, 2012-07-26 */
    IF_AGG_CB_MODULE_IF_NDP, /* modified by huangxt for bug 19814, 2012-09-11 */
    IF_AGG_CB_MODULE_IF_URPF, /* modified by huangxt for bug 19814, 2012-09-11 */
    IF_AGG_CB_MODULE_IF_FLOW_CTRL, /* add by chenzh for bug 17370, 2011-07-06*/
    IF_AGG_CB_MODULE_PORT_ISOLATE, /* modified by huangxt for bug 13838, 2011-01-10 */
    IF_AGG_CB_MODULE_VLAN_CLASS,
    IF_AGG_CB_MODULE_VLAN_MAPPING, /* modified by liwh for bug 18252 for vlan mapping performance */
    IF_AGG_CB_MODULE_PVLAN, /* modified by huangxt for bug 26924, 2014-01-20. support pvlan */
    IF_AGG_CB_MODULE_VOICE_VLAN,
    IF_AGG_CB_MODULE_L2PROTO,
    IF_AGG_CB_MODULE_ERPS,
    IF_AGG_CB_MODULE_G8031_G8032, /* add by chenzh for bug 15103, 2011-07-06*/
    IF_AGG_CB_MODULE_CFM, /* modified by liwh for bug 16813 , 2011-11-17 */
    IF_AGG_CB_MODULE_ACL,
    IF_AGG_CB_MODULE_ACL_POLICY,
    IF_AGG_CB_MODULE_QOS,
    IF_AGG_CB_MODULE_MIRROR,
    IF_AGG_CB_MODULE_LBDT,    /* modified by sunh for bug 27882, 2014-03-29 */
    IF_AGG_CB_MODULE_IPV6_STATIC, /* modified by huangxt for bug 19814, 2012-07-21 */ 
    IF_AGG_CB_MODULE_ARP_INSPECTION,
    IF_AGG_CB_MODULE_IP_BINDING,
    IF_AGG_CB_MODULE_DHCP,
    IF_AGG_CB_MODULE_DHCP_CLIENT, /* added by liuyang for bug 17218 2011-12-13*/
    IF_AGG_CB_MODULE_DHCP_SERVER,
    IF_AGG_CB_MODULE_DHCPV6, /* added by liuyang 2012-9-12*/
    IF_AGG_CB_MODULE_PBR, /* add by chenzh for bug 15103, 2011-07-06*/
    IF_AGG_CB_MODULE_SFLOW,
    IF_AGG_CB_MODULE_IVI,
    IF_AGG_CB_MODULE_TAP,
    IF_AGG_CB_MODULE_OPENFLOW,
    IF_AGG_CB_MODULE_MLAG, /* added by kcao 2015-12-09 for MLAG */
    IF_AGG_CB_MODULE_MAX
} if_agg_cb_module_id_t;

typedef enum
{
    IF_AGG_CB_NOTIFY_CREATE_AGGREGATOR,     /* if_agg_create_agg_sync_member_prop_set_cb */
    IF_AGG_CB_NOTIFY_DESTROY_AGGREGATOR,    /* if_agg_destory_agg_sync_member_prop_set_cb */
    IF_AGG_CB_NOTIFY_ADD_MEMBER_SYNC,       /* if_agg_add_member_sync_prop_set_cb */
    IF_AGG_CB_NOTIFY_DEL_MEMBER_SYNC,       /* if_agg_del_member_sync_prop_set_cb */
    IF_AGG_CB_NOTIFY_ATTACH_MEMBER_SYNC,    /* if_agg_attach_member_sync_prop_set_cb */
    IF_AGG_CB_NOTIFY_DETACH_MEMBER_SYNC,    /* if_agg_detach_member_sync_prop_set_cb */
    IF_AGG_CB_NOTIFY_ADD_MEMBER_CHECK,      /* if_agg_add_member_check_set_cb */
    IF_AGG_CB_NOTIFY_OP_MAX
} if_agg_cb_notify_type_t;


typedef enum
{
    IF_AGG_ACTIVE_BACKUP_OP_ADD_MEMBER,
    IF_AGG_ACTIVE_BACKUP_OP_DEL_MEMBER,
    IF_AGG_ACTIVE_BACKUP_OP_CHANGE_MAX_NUMBER,
    IF_AGG_ACTIVE_BACKUP_OP_MEMBER_LINK_DOWN,
    IF_AGG_ACTIVE_BACKUP_OP_CHANGE_MEM_PRIORITY,
    IF_AGG_ACTIVE_BACKUP_OP_MAX
} if_agg_active_backup_oper_t;

typedef int32 (*AGG_CB_NOTIFY_FUNC)(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

typedef struct
{
    uint32          is_static;
    cfg_result_t    *p_rslt;
} if_agg_cb_param_t;

int32
if_agg_add_agg(ds_lag_t *p_lag, cfg_cmd_para_t *para, tbl_interface_t *p_db_if, uint32 agg_member_priority);

int32
if_agg_del_agg(tbl_interface_t *p_db_if);

int32
_if_agg_create_agg_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if);

int32
_if_agg_destory_agg_sync_prop(tbl_interface_t *p_db_agg_if);

int32
_if_agg_add_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if);

int32
_if_agg_del_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if);

int32
_if_agg_attach_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if);

int32
_if_agg_detach_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if);

int32
_if_agg_add_member_check(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, cfg_cmd_para_t *para);

int32
_if_agg_sync_agg_prop(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if);

int32
if_agg_aggregator_bw_update(tbl_interface_t *p_db_if_agg);

int32
if_agg_if_running_update_mismatch(tbl_interface_t *p_db_if_lag);

int32
if_agg_attach_mux_to_aggregator(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem);

int32
if_agg_detach_mux_from_aggregator(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem);

int32
if_agg_static_agg_mux_aggregator_update(tbl_interface_t *p_db_if_mem);

/* set agg callback functions */
int32
if_agg_create_agg_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32
if_agg_destory_agg_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32
if_agg_add_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32
if_agg_del_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32
if_agg_attach_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32
if_agg_detach_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32
if_agg_add_member_check_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func);

int32 _if_active_backup_agg_oper(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem, if_agg_active_backup_oper_t op, uint32 agg_member_priority, char *agg_name);

#endif /* !__IF_AGG_H__ */

