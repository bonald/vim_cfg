
#ifndef __OPENFLOW_MGR_H__
#define __OPENFLOW_MGR_H__

#include "switch.h"
#include "proto.h"

int32
openflow_mgr_start(switch_master_t *master);

int32
openflow_mgr_stop(switch_master_t *master);

#endif /* !__OPENFLOW_MGR_H__ */
