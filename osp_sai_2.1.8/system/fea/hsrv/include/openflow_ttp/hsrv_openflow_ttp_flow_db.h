#ifndef __HSRV_OPENFLOW_FLOW_PRIV_H__
#define __HSRV_OPENFLOW_FLOW_PRIV_H__

struct hsrv_openflow_ttp_flow_master_s
{
    ctclib_hash_t* p_flow_hash;         /* hsrv_openflow_ttp_flow_t */
    ctclib_hash_t* p_metadata_hash;         /* metadata_mapping_t */
    ctclib_opf_t* pst_metadata_fid_opf;
    ctclib_opf_t* pst_metadata_vrf_opf;
};
typedef struct hsrv_openflow_ttp_flow_master_s hsrv_openflow_ttp_flow_master_t;

#define HSRV_OPENFLOW_TTP_FLOW_HASH           g_pst_openflow_ttp_flow_master->p_flow_hash
#define P_METADATA_HASH                                   g_pst_openflow_ttp_flow_master->p_metadata_hash

#define HSRV_OPF_FID_MIN 4098
#define HSRV_OPF_FID_MAX_NUM 3000

#define HSRV_OPF_VRF_MIN 1
#define HSRV_OPF_VRF_MAX_NUM 3000

int32
hsrv_openflow_flowdb_add_flow(hsrv_openflow_ttp_flow_t* p_flow);

int32
hsrv_openflow_flowdb_lookup_flow(uint32 flow_id, hsrv_openflow_ttp_flow_t** pp_flow);

int32
hsrv_openflow_flowdb_del_flow(uint32 flow_id);

int32
hsrv_openflow_flowdb_alloc_fid(uint16 *fid);

int32
hsrv_openflow_flowdb_release_fid(uint16 fid);

int32
hsrv_openflow_flowdb_alloc_vrf_id(uint16 *vrf_id);

int32
hsrv_openflow_flowdb_release_vrf_id(uint16 vrf_id);

int32
hsrv_openflow_flowdb_add_metadata_mapping(metadata_mapping_t* p_metadata);

int32
hsrv_openflow_flowdb_lookup_metadata_mapping(uint64 metadata, metadata_mapping_t** pp_metadata);

int32
hsrv_openflow_flowdb_del_metadata_mapping(uint64 metadata);

/*
int32
hsrv_openflow_flowdb_loop_flow(int32 (*fn)());
*/

int32
hsrv_openflow_flowdb_start(void);

#endif /* !__HSRV_OPENFLOW_FLOW_PRIV_H__ */
