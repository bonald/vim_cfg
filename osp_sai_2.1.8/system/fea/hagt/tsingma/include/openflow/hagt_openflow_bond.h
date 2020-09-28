#ifndef __HAGT_OPENFLOW_BOND_H__
#define __HAGT_OPENFLOW_BOND_H__

#define HAGT_OPENFLOW_BOND_MIN_PORT_NUM 1
#define HAGT_OPENFLOW_BOND_MAX_PORT_NUM 16

#define HAGT_OPENFLOW_IS_VALID_BOND_ID(TID)   \
    (TID >= HAGT_OPENFLOW_BOND_MIN_PORT_NUM && TID <= HAGT_OPENFLOW_BOND_MAX_PORT_NUM)

int32
hagt_openflow_bond_register_callback(void);
int32
hagt_openflow_bond_init(void);

#endif /* !__HAGT_OPENFLOW_BOPND_H__ */

