
#ifndef __LIB_CMD_H__
#define __LIB_CMD_H__

typedef enum
{
    LIB_SERVICE_PORT_TELNET = 0,
    LIB_SERVICE_PORT_SSH,
    LIB_SERVICE_PORT_NETCONF,
    LIB_SERVICE_PORT_HTTP,
    LIB_SERVICE_PORT_HTTPS,
    LIB_SERVICE_PORT_RPCAPI,
    LIB_SERVICE_PORT_RPCAPI_SSL
} lib_service_port_t;

int32
lib_cmd_check_port(uint32 port, uint32 type, cfg_cmd_para_t *para);

int32
lib_check_ipv6_addr(addr_t *addr);

int32
lib_check_ipv6_prefix(prefix_t *p);

int32
lib_cmd_init(pm_id_t pm_id);

int32 
lib_cmd_process_cli_return(cfg_cmd_para_t *para, int32 rc);

#endif /* !__LIB_CMD_H__ */
