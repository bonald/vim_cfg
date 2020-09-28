#ifndef __HSRV_OPENFLOW_TTP_GROUP_H__
#define __HSRV_OPENFLOW_TTP_GROUP_H__

/* hsrv openflow memory group related macros */
#define HSRV_OPENFLOW_GROUP_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_GROUP, size)
#define HSRV_OPENFLOW_GROUP_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_GROUP, size)
#define HSRV_OPENFLOW_GROUP_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_GROUP, ptr);  \
    ptr = NULL; \
}while(0)


/** L2 Interface Group Bucket */
typedef struct hsrv_openflow_ttp_l2_interface_group_bucket_data_s
{
  /** bucket action set */
  /** controller responsible for assuring data in outputPort
      and the port identifier data encoded in the groupId are equal */

  uint32    output_port;

  /** controller responsible for assuring vlan is identified in groupId */
  /** flag indicating if outer VLAN tag should be stripped
  (0 - do not strip VLAN tag, 1 - strip VLAN tag) */

  uint32    pop_vlan_tag;  /* optional */

  uint32    push_vlan_tag;

  /* for tagged port to set vlan */
  uint16    vlan_id;

}hsrv_openflow_ttp_L2_interface_group_bucket_data_t;

/** L3 Unicast Group Bucket */
typedef struct hsrv_openflow_ttp_l3_unicast_group_bucket_data_s
{
  /** bucket action set */
  uint32    l2_interface_group_id;

  /** data for Set-Field action */
  uint8     src_mac[MAC_ADDR_LEN];

  uint8     dst_mac[MAC_ADDR_LEN];

} hsrv_openflow_ttp_l3_unicast_group_bucket_data_t;


/** Group Bucket Table Entry */
typedef struct hsrv_openflow_ttp_group_bucket_entry_s
{
  /** Group Id */
  uint32    group_id;

  /** Bucket Index */
  uint32    bucket_index;

  /** bucket action set */
  /** References a chained group entry, must be zero for L2 Interface Group entries */

  uint32    reference_group_id;

  union
  {
    /** L2 Interface */
    hsrv_openflow_ttp_L2_interface_group_bucket_data_t  l2_interface;

    /** L3 Unicast */
    hsrv_openflow_ttp_l3_unicast_group_bucket_data_t    l3_unicast;

  } bucket_data;

} hsrv_openflow_ttp_group_bucket_entry_t;


struct hsrv_bucket_index_s
{
    ctclib_list_node_t node;

    uint32 bucket_index;
};
typedef struct hsrv_bucket_index_s hsrv_bucket_index_t;


/**
 * @brief Hsrv layer group info
 */
struct hsrv_openflow_ttp_group_s
{

    uint32      group_id;    /* GLB_OPENFLOW_TTP_GROUP_TYPE */

    /*only use for sdk mcast group id*/
    uint32      mc_group_id;

    /**
     * reference count, unsigned 32-bit integer per to openflow spec,
     * group may be referenced by flow entries and other group buckets
     */
    uint32      ref_count;

    /** bucket count contained in this group */
    uint32      bucket_count;

    uint32      nh_id;

    uint8       stats_supported;

    uint32      stats_id;
    
    ctclib_list_t   bucket_index_list;    /**< hsrv_bucket_index_t */
    

};
typedef struct hsrv_openflow_ttp_group_s hsrv_openflow_ttp_group_t;



hsrv_openflow_ttp_group_t*
hsrv_openflow_group_get_group_info(uint32 group_id);

int32
hsrv_openflow_show_group_brief(FILE *fp);

int32
hsrv_openflow_show_group(FILE *fp, uint32 group_id);

int32
hsrv_openflow_group_start(void);

#endif /* !__HSRV_OPENFLOW_GROUP_H__ */
