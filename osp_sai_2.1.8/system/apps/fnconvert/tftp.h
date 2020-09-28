#ifndef __CDB_CLIENT_TFTP_H__
#define __CDB_CLIENT_TFTP_H__

int32 gen_tftp_op_init(network_handle_t * h);

void gen_tftp_op_release(network_handle_t * h);

int32 gen_tftp_op_parseurl(network_handle_t * h, char *url);

int32 gen_tftp_op_append_dir(network_handle_t * h, char *dir);

int32 gen_tftp_op_interactive_get_param(network_handle_t * h);

int32 gen_tftp_op_sendto(network_handle_t * h, char *src, char *vrf);

int32 gen_tftp_op_recvfrom(network_handle_t * h, char *dst, char *vrf);

#endif							/* !__CDB_CLIENT_TFTP_H__ */
