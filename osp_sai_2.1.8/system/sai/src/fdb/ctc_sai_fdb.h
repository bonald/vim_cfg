#if !defined (__SAI_FDB_H_)
#define __SAI_FDB_H_

#include <saifdb.h>

typedef struct ctc_sai_fdb_entry_s
{
    sai_fdb_entry_type_t    type;
    sai_fdb_entry_t         fdb_key;
    sai_object_id_t         port;
    sai_packet_action_t     action;
    sai_fdb_l2protocol_flag_t flag;
}ctc_sai_fdb_entry_t;

typedef struct ctc_sai_fdb_info_s
{
    uint32_t            	fdb_max_count;
}ctc_sai_fdb_info_t;

sai_status_t
ctc_fdb_lookup_entry(
	_In_  const sai_fdb_entry_t *,
	_In_  ctc_sai_fdb_entry_t *);

sai_status_t
ctc_sai_fdb_init();

int32
ctc_fdb_get_by_port_and_vlanid(uint32 vlanid, uint32 gport);

#endif

