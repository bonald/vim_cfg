#ifndef __FEI_OPENFLOW_BOND_H__
#define __FEI_OPENFLOW_BOND_H__

int32
fei_openflow_bond_create(uint32 ifindex, bool enable);

int32
fei_openflow_bond_update_slave_state(uint32 bond_ifindex, uint32 slave_ifindex, bool enable);

int32
fei_openflow_bond_attach_slave(uint32 bond_ifindex, uint32 slave_ifindex);

int32
fei_openflow_set_bond_mac(uint32 bond_ifindex, uint8* mac);

#endif
