#ifndef __HSRV_OPENFLOW_TTP_METER_DB_H__
#define __HSRV_OPENFLOW_TTP_METER_DB_H__

struct hsrv_openflow_ttp_meter_master_s
{
    ctclib_hash_t* p_meter_hash;         /**< hsrv_openflow_ttp_meter_info_t */
};
typedef struct hsrv_openflow_ttp_meter_master_s hsrv_openflow_ttp_meter_master_t;

extern hsrv_openflow_ttp_meter_master_t *g_pst_openflow_ttp_meter_master;

#define HSRV_OPENFLOW_TTP_METER_HASH           g_pst_openflow_ttp_meter_master->p_meter_hash


int32
hsrv_openflow_meterdb_add_meter(hsrv_openflow_ttp_meter_info_t* p_meter);

int32
hsrv_openflow_meterdb_del_meter(uint32 meter_id);

int32
hsrv_openflow_meterdb_lookup_meter(uint32 meter_id, hsrv_openflow_ttp_meter_info_t** pp_meter);

int32
hsrv_openflow_meterdb_start(void);

#endif
