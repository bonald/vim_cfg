#ifndef __HAGT_OPENFLOW_STATS_DB_H__
#define __HAGT_OPENFLOW_STATS_DB_H__

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "hagt_openflow.h"

/****************************************************************************************
 *
 * Globals
 *
 ****************************************************************************************/

int32
hagt_openflow_statsdb_add_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id, hagt_openflow_stats_info_t *stats_info);

int32
hagt_openflow_statsdb_del_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id);

void*
hagt_openflow_statsdb_get_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id);

int32
hagt_openflow_statsdb_init(void);
#endif /*__HAGT_OPENFLOW_STATS_DB_H__*/
