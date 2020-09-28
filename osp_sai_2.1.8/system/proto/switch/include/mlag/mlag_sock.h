
#ifndef __MLAG_SOCK_H__
#define __MLAG_SOCK_H__

int32
mpn_sock_cb_connect(tbl_mlag_peer_t *peer);

int32
mpn_sock_server_init();

int32
mpn_sock_server_deinit();

int32
mpn_sock_client_init();

int32
mpn_sock_client_deinit();

int32
mpn_sock_client_connect();

int32
mpn_sock_client_disconnect();

mlag_sock_connected_t
mpn_sock_check_connected(tbl_mlag_peer_t *peer);

#endif /* !__MLAG_SOCK_H__ */

