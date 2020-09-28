
#ifndef __IPC_H__
#define __IPC_H__
#include "glb_openflow_packet.h"

#define TMP_DIR             "/tmp/"
#define SOCK_ADDR_CCS       "/tmp/sock_ccs_config"
#define SOCK_ADDR_NCS       "/tmp/sock_ncs_config"
#define SOCK_ADDR_CDS_DATA  "/tmp/sock_cds_data"
#define SOCK_ADDR_CCS_SUB_CDB "/tmp/sock_ccs_sub_cdb"
#define SOCK_ADDR_BHM       "/tmp/sock_bhm"
#define SOCK_ADDR_SNMP_TRAP "/var/run/trap-master"

#define SOCK_ADDR_FEI       "/tmp/sock_fei"
#define SOCK_ADDR_FEI_HIGH  "/tmp/sock_fei_high"

#define SOCK_ADDR_PM_PKT    "/tmp/sock_pm_pkt"

#define CFG_SHOWCFG_PATH    "/tmp/running-config"

#define CFG_BUILD_SHOW_PATH(_path_, _pid_, _seq_) \
do { \
    sal_sprintf((_path_), "/tmp/cfgshow%d_%u_%u", (_pid_), (_seq_), para->p_msg->u_hdr.cfg.id); \
} while (0)

#define CFG_DEL_SHOW_PATH(_path_, _fp_) \
do { \
    sal_fclose(_fp_); \
    sal_unlink(_path_); \
} while (0)

#define CTCLIB_PRINT_CON(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}

int32
ipc_pm_cds_filter(uint16 tbl_id);

int32
ipc_register_rx_cb();

int32
ipc_register_hal_rx_fn(uint32 type, SOCK_RX_FN proc_fn);

int32
ipc_register_pkt_rx_fn(uint32 type, SOCK_RX_FN proc_fn);

int32
ipc_ctl_to_ccs_sub_req(uint8 *data, uint32 data_len);

int32
ipc_ctl_to_cds_sub_tbl(uint32 *tbl_id, uint32 tbl_num, uint32 sync, uint32 snmp);

int32
ipc_ctl_to_cds_sub_tbl_resync();

int32
ipc_ctl_to_cds_sub_cdb(cdb_node_t *p_node, cdb_node_t *p_ds_node, field_parse_t *p_field, char *p_data, uint32 data_len);

int32
ipc_ctl_to_ccs_cfg_sdb(cdb_oper_t oper, uint16 node_type, uint16 node_id, void *p_data, uint32 data_len, ctc_msg_t *p_resp_msg);

int32
ipc_ctl_to_ccs_cfg(cdb_oper_t oper, cdb_node_t* p_node, void *p_data, uint32 data_len, ctc_msg_t *p_resp_msg);

int32
ipc_pm_continue_send_to_ccs(int32 sock, int32 pid, cfg_result_t *p_result);

int32
ipc_ccs_to_pm_cfg(int32 sock, ctc_msg_t *p_msg);

int32
ipc_ncs_to_pm_cfg_sync(int32 sock, pm_id_t pm_id, uint16 type, uint16 id, cdb_cfg_t cfg, cdb_oper_t opt, uint8 *data, uint32 data_len, ctc_msg_t *p_resp_msg);

int32
ipc_pm_to_bhm_keepalive();

int32
ipc_pm_to_ccs(int32 sock, ctc_msg_t *p_req_msg, void *p_data, uint32 data_len);

int32
ipc_pm_to_cds_data(uint8 oper, uint8 type, uint16 tbl_id, uint32 field_id, void *p_data, uint32 data_len);

int32
ipc_pm_to_cds_data_ds(uint8 oper, uint8 type, uint16 tbl_id, uint16 ds_id, uint32 field_id, 
                        void *p_data, uint32 data_len, void *p_data1, uint32 data1_len);

int32
ipc_pm_to_cds_data_ds2(uint8 oper, uint8 type, uint16 tbl_id, uint16 ds_id, uint16 ds2_id, uint32 field_id, 
                        void *p_data, uint32 data_len, void *p_data1, uint32 data1_len, void *p_data2, uint32 data2_len);

int32
ipc_pm_to_cds_refer_tbl(uint8 type, uint16 tbl_id, uint32 field_id, 
    void *p_data, uint32 data_len, 
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj);

int32
ipc_pm_to_cds_refer_ds(uint8 type, uint16 tbl_id, uint16 ds_id, uint32 field_id, 
    void *p_data, uint32 data_len, void *p_data1, uint32 data1_len,
    cdb_sync_refer_hdr_t *p_refer_hdr, void *p_refer_obj);

int32
ipc_pm_process_data_sync(int32 sock, ctc_msg_t *p_msg);

int32
ipc_connect_ccs(int32 immediate);

int32
ipc_connect_bhm();

int32
ipc_connect_ccs_cds(int32 immediate);

int32
ipc_connect_ccs_subscribe(int32 immediate);

int32
ipc_connect_snmp_trap();

int32
ipc_sock_init();

int32
ipc_sock_deinit();

int32
ipc_pkt_init(int32 immediate);

int32
ipc_pkt_tx_bpdu(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

int32
ipc_pkt_tx_lacp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

int32
ipc_pkt_tx_arp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid);

int32
ipc_pkt_tx_openflow(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

int32
ipc_pkt_tx_openflow_packet_out(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, glb_openflow_packet_out_param_t* p_tail);

int32
ipc_pkt_tx_sflow(uint8 *p_pkt, uint32 pkt_len, uint32 address, vid_t vid);

int32
ipc_pkt_tx_dhcp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid, int flags);

int32
ipc_pkt_tx_lldp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

int32
ipc_pkt_tx_erps(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

int32
ipc_pkt_tx_igmp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid);

int32
ipc_pkt_tx_igmp_flood(uint8 *p_pkt, uint32 pkt_len, uint32 source_ifindex, vid_t vid);

int32
ipc_pkt_tx_eapol(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

int32
ipc_set_sync_timeout(uint32 sync_time);

int32
ipc_pm_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
ipc_pkt_tx_ptp(uint8 *p_pkt, uint32 pkt_len, uint8 *p_aux, uint32 aux_len, uint32 ifindex, uint32 flags);

int32
ipc_pkt_tx_tpoam(uint8 *p_pkt, uint32 pkt_len, uint8 *p_aux, uint32 aux_len, uint32 ifindex, uint32 flags);

int32
ipc_pkt_tx_vrrp(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid);

int32
ipc_pkt_tx_ssm(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex);

/* support nd modified by liwh for bug 47547, 2018-06-28 */
int32
ipc_pkt_tx_nd(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid);
/* liwh end */

int32
ipc_pkt_tx_raps(uint8 *p_pkt, uint32 pkt_len, uint32 ifindex, vid_t vid);

#endif /* !__IPC_H__ */

