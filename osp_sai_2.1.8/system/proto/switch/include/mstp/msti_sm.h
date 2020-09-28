#ifndef __MSTI_SM_H__
#define __MSTI_SM_H__

#define MSTP_MSTI_PORT_STR(_STR_, _P_MSTI_PORT_) \
do { \
    char _tmp_[CMD_BUF_16] = {0,}; \
    IFNAME_ETH2FULL((_P_MSTI_PORT_)->key.name, (_STR_)); \
    sal_snprintf(_tmp_, CMD_BUF_16, ":%u", (_P_MSTI_PORT_)->key.instance); \
    sal_strcat((_STR_), _tmp_); \
} while (0)

typedef struct
{
    mstp_brg_id_t        regionRootId;          /* 17.19.21/13.24.12 portPriority regionRootId */
    uint32               intRootPathCost;       /* 17.19.21/13.24.12 portPriority internalRootPathCost */
    mstp_brg_id_t        designatedBridgeId;    /* 17.19.21/13.24.12 portPriority designatedBridgeId */
    uint16               designatedPortId;      /* 17.19.21/13.24.12 portPriority designatedPortId */
    uint16               bridgePortId;          /* 17.19.21/13.24.12 portPriority bridgePortId */
} msti_port_priority_vector_t;

int32 
msti_updtRolesTree(tbl_mstp_instance_t *p_db_inst);

int32
msti_newTcWhile(tbl_msti_port_t *p_db_port);

/* 17.27 */
int32
msti_sm_pim_process_event(tbl_msti_port_t *p_db_port, pim_event_t event);
/* 17.28 */
int32
msti_sm_prs_process_event(tbl_mstp_instance_t *p_db_inst, prs_event_t event);  /* bridge State Machine */
/* 17.29 */

int32
msti_sm_prt_process_event(tbl_msti_port_t *p_db_port, prt_event_t event);
/* 17.30 */
int32
msti_sm_pst_process_event(tbl_msti_port_t *p_db_port, pst_event_t event);
/* 17.31 */
int32
msti_sm_tcm_process_event(tbl_msti_port_t *p_db_port, tcm_event_t event);

int32
msti_sm_init();

int32
msti_sm_port_start_prs(tbl_mstp_instance_t *p_db_inst);
int32
msti_sm_port_stop_prs(tbl_mstp_instance_t *p_db_inst);
int32
msti_sm_port_start_pst(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_stop_pst(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_start_pim(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_stop_pim(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_start_prt(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_stop_prt(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_start_tcm(tbl_msti_port_t *p_db_port);
int32
msti_sm_port_stop_tcm(tbl_msti_port_t *p_db_port);

int32
msti_sm_do_rootguard_process(tbl_mstp_port_t *p_db_port);

int32
msti_sm_PRT_DESIGNATED_DISCARD(tbl_msti_port_t *p_db_port, char *msti_port_name);

#endif /* !__MSTI_SM_H__ */

