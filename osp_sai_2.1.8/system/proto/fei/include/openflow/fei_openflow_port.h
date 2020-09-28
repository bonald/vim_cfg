#ifndef __FEI_OPENFLOW_PORT_H__
#define __FEI_OPENFLOW_PORT_H__

int32
fei_openflow_port_notify_link(uint32 ifindex, bool enable);

int32
fei_openflow_port_set_config(uint32 ifindex, uint32 port_config);

int32
fei_openflow_add_port_to_bridge(uint32 ifindex, uint32 flag, bool enable);

#endif /* !__FEI_OPENFLOW_PORT_H__ */
