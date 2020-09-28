#ifndef __HSRV_OPENFLOW_FLOW_PRIV_H__
#define __HSRV_OPENFLOW_FLOW_PRIV_H__

struct hsrv_openflow_flow_master_s
{
    ctclib_hash_t* p_flow_hash;         /* hsrv_openflow_flow_t */
    ctclib_hash_t* p_actlist_hash;      /* hsrv_actlist_info_t */

    uint8 drop_pkt_to_ingress_port;     /* Added by weizj for tunnel bug 33724 */
    uint8 udf_enable;                   /* Added by weizj for udf */
    uint32 global_acl_label;
};
typedef struct hsrv_openflow_flow_master_s hsrv_openflow_flow_master_t;

#define HSRV_OPENFLOW_FLOW_HASH           g_pst_openflow_flow_master->p_flow_hash
#define HSRV_OPENFLOW_ACTLIST_HASH        g_pst_openflow_flow_master->p_actlist_hash
/* Added by weizj for tunnel bug 33724 */
#define HSRV_OPENFLOW_FLOW_DROP_INGRESS   g_pst_openflow_flow_master->drop_pkt_to_ingress_port
/* Added by weizj for udf */
#define HSRV_OPENFLOW_FLOW_UDF_ENABLE     g_pst_openflow_flow_master->udf_enable

int32
hsrv_openflow_flowdb_lookup_flow_actlist(hsrv_openflow_flow_t *p_flow, hsrv_actlist_info_t** pp_actions_info);

int32
hsrv_openflow_flowdb_add_flow_actlist(hsrv_actlist_info_t* p_actions_info);

int32
hsrv_openflow_flowdb_del_flow_actlist(hsrv_actlist_info_t* p_actions_info);

int32
hsrv_openflow_flowdb_add_flow(hsrv_openflow_flow_t* p_flow);

int32
hsrv_openflow_flowdb_lookup_flow(uint32 flow_id, hsrv_openflow_flow_t** pp_flow);

int32
hsrv_openflow_flowdb_del_flow(hsrv_openflow_flow_t* p_flow);

int32
hsrv_openflow_flowdb_loop_flow(int32 (*fn)());

/* Added by weizj for tunnel bug 33724 */
int32
hsrv_openflow_flowdb_set_drop_ingress(uint8 is_drop);
/* End by weizj for tunnel */

/* Added by weizj for udf */
int32
hsrv_openflow_flowdb_udf_enable(uint8 enable);

int32
hsrv_openflow_flowdb_start(void);

#endif /* !__HSRV_OPENFLOW_FLOW_PRIV_H__ */
