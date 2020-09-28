
#ifndef __NCS_H__
#define __NCS_H__

int32
ncs_sock_cfg_sync(cdb_cfg_t cfg, cdb_oper_t opt, uint16 type, uint16 id, char *cmd, uint32 cmd_len);

int32
ncs_nc_init();

int32
ncs_init();

#endif /* !__NCS_H__ */

