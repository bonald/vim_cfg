#ifndef __FEI_OPENFLOW_BOND_H__
#define __FEI_OPENFLOW_BOND_H__

int32
fei_openflow_stub_create(uint32 ifindex);

int32
fei_openflow_stub_destroy(uint32 ifindex);

int32
fei_openflow_stub_enable(uint32 ifindex, bool enable);

#endif
