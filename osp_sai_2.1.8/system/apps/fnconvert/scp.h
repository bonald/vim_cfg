#ifndef _OSP_SCP_H_
#define _OSP_SCP_H_

#define BUFF_SIZE_256 256

int32 gen_scp_op_init(network_handle_t * h);
void gen_scp_op_release(network_handle_t * h);
int32 gen_scp_op_parseurl(network_handle_t * h, char *url);
int32 gen_scp_op_append_dir(network_handle_t * h, char *dir);
int32 gen_scp_op_interactive_get_param(network_handle_t * h);
int32 gen_scp_op_sendto(network_handle_t * h, char *src, char *vrf);
int32 gen_scp_op_recvfrom(network_handle_t * h, char *dst, char *vrf);

#endif /* !_OSP_SCP_H_ */
