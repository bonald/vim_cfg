#ifndef __MSTP_SM_H__
#define __MSTP_SM_H__

typedef struct
{
    mstp_brg_id_t        rootId;                /* 17.19.21/13.24.12 portPriority rootId */
    uint32               rootPathCost;          /* 17.19.21/13.24.12 portPriority externalRootPathCost */
    mstp_brg_id_t        regionRootId;          /* 17.19.21/13.24.12 portPriority regionRootId */
    uint32               intRootPathCost;       /* 17.19.21/13.24.12 portPriority internalRootPathCost */
    mstp_brg_id_t        designatedBridgeId;    /* 17.19.21/13.24.12 portPriority designatedBridgeId */
    uint16               designatedPortId;      /* 17.19.21/13.24.12 portPriority designatedPortId */
    uint16               bridgePortId;          /* 17.19.21/13.24.12 portPriority bridgePortId */
} mstp_port_priority_vector_t;

int32
updtRolesTree();

int32
mstp_sm_port_start_pti(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_pti(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_prx(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_prx(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_ppm(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_ppm(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_bdm(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_bdm(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_ptx(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_ptx(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_pim(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_pim(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_prs(tbl_mstp_global_t *p_db_glb);
int32
mstp_sm_port_stop_prs(tbl_mstp_global_t *p_db_glb);
int32
mstp_sm_port_start_prt(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_prt(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_pst(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_pst(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_start_tcm(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_port_stop_tcm(tbl_mstp_port_t *p_db_port);
int32
mstp_sm_send_bpdu(tbl_mstp_port_t *p_db_port);

mstp_bpdu_role_t
mstp_pdu_get_bpdu_role(tbl_mstp_port_t *p_db_port);

/* 17.23 */
int32
mstp_sm_prx_process_event(tbl_mstp_port_t *p_db_port, prx_event_t event);
/* 17.24 */
int32
mstp_sm_ppm_process_event(tbl_mstp_port_t *p_db_port, ppm_event_t event);
/* 17.25 */
int32
mstp_sm_bdm_process_event(tbl_mstp_port_t *p_db_port, bdm_event_t event);
/* 17.26 */
int32
mstp_sm_ptx_process_event(tbl_mstp_port_t *p_db_port, ptx_event_t event);
/* 17.27 */
int32
mstp_sm_pim_process_event(tbl_mstp_port_t *p_db_port, pim_event_t event);
/* 17.28 */
int32
mstp_sm_prs_process_event(tbl_mstp_global_t *p_db_glb, prs_event_t event);  /* bridge State Machine */
/* 17.29 */
int32
mstp_sm_prt_process_event(tbl_mstp_port_t *p_db_port, prt_event_t event);
/* 17.30 */
int32
mstp_sm_pst_process_event(tbl_mstp_port_t *p_db_port, pst_event_t event);
/* 17.31 */
int32
mstp_sm_tcm_process_event(tbl_mstp_port_t *p_db_port, tcm_event_t event);

uint32
mstp_sm_has_topo_change();

uint32
mstp_sm_update_topo_change();

int32
mstp_sm_init();

int32
mstp_sm_rx_bpdu(tbl_mstp_port_t *p_db_port);

int32 
getBridgePriority(mstp_port_priority_vector_t *bridge_priority, tbl_mstp_global_t *p_db_glb);
int32 
updateBestPortPriority(mstp_port_priority_vector_t *port_priority, tbl_mstp_port_t *p_db_port);

int32
mstp_sm_port_do_mcheck(tbl_mstp_port_t *p_db_port);

int32 
setTcPropTree(tbl_mstp_port_t *p_db_port);

#endif /* !__MSTP_SM_H__ */
