
#ifndef __MLAG_ENCODE_H__
#define __MLAG_ENCODE_H__

int32
mlag_send_open(tbl_mlag_peer_t *peer);
int32
mlag_send_keepalive(tbl_mlag_peer_t *peer);
int32
mlag_send_disconnect(tbl_mlag_peer_t *peer);
int32
mlag_send_failover(tbl_mlag_peer_t *peer, uint32 mlagid, uint32 is_recover);
int32
mlag_send_conf(tbl_mlag_peer_t *peer, uint32 mlagid, uint32 is_conf, uint32 is_up);
int32
mlag_send_fdbsync(tbl_mlag_peer_t *peer, glb_mlag_fdb_sync_t *sync);
int32
mlag_send_stp_global_info(tbl_mlag_peer_t *peer, uint8 enable, uint8 type, uint16 prio, uint8 *addr);
int32
mlag_send_stp_port_info(tbl_mlag_peer_t *peer, uint8 enable, uint16 mlagid);
int32
mlag_send_stp_state(tbl_mlag_peer_t *peer, uint16 mlagid, uint16 instance, uint8 state);
int32
mlag_send_stp_bpdu_packet(tbl_mlag_peer_t *peer, uint16 mlagid, l2_socket_addr_t *addr, uint32 length, uint8 *pkt);
int32
mlag_send_stp_instance(tbl_mlag_peer_t *peer, uint16 instance);
int32
mlag_send_syspri(tbl_mlag_peer_t *peer);
int32
mlag_send_peer_fdb(tbl_mlag_peer_t *peer, uint8 op, vid_t *vlan, uint16 vlan_num);

#endif /* !__MLAG_ENCODE_H__ */

