#ifndef __HSRV_OPENFLOW_STATS_H__
#define __HSRV_OPENFLOW_STATS_H__

struct hsrv_openflow_stats_master_s
{
    ctclib_hash_t* openflow_stats_hash;      /**< hsrv_of_stats_t */
    ctclib_opf_t* pst_openflow_stats_id_opf;
};
typedef struct hsrv_openflow_stats_master_s hsrv_openflow_stats_master_t;

#define HSRV_OF_STATS_HASH        g_pst_openflow_stats_master->openflow_stats_hash

enum hsrv_openflow_stats_resource_type_e
{
    HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW = 0,
    HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER,
    HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP,

    HSRV_OPENFLOW_STATS_RESOURCE_TYPE_MAX
};
typedef enum hsrv_openflow_stats_resource_type_e hsrv_openflow_stats_resource_type_t;

struct hsrv_openflow_stats_resource_alloc_type_s
{
    union
    {
        glb_openflow_group_alloc_type_t group_alloc_type;
    } sub_type;
};
typedef struct hsrv_openflow_stats_resource_alloc_type_s hsrv_openflow_stats_resource_alloc_type_t;

struct hsrv_openflow_stats_debug_data_s
{
    FILE* fp;
    int32 i;
};
typedef struct hsrv_openflow_stats_debug_data_s hsrv_openflow_stats_debug_data_t;

struct hsrv_of_stats_s
{
    uint32 stats_id;
    hsrv_openflow_stats_resource_type_t resource_type;
    hsrv_openflow_stats_resource_alloc_type_t resource_alloc_type;
    glb_openflow_stats_t stats;
    uint8 is_mpls_key;
    uint8 is_mpls_vc;
};
typedef struct hsrv_of_stats_s hsrv_of_stats_t;

#define HSRV_OF_STATS_TYPE_STR(type)                      \
    (HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW == (type)   ? "flow" : \
     HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER == (type)  ? "meter" : \
     HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP == (type)  ? "group" : \
     "unknown")

/* hsrv openflow memory stats related macros */
#define HSRV_OPENFLOW_STATS_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_STATS, size)
#define HSRV_OPENFLOW_STATS_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_STATS, size)
#define HSRV_OPENFLOW_STATS_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_STATS, ptr);  \
    ptr = NULL; \
}while(0)

int32
hsrv_openflow_stats_add_stats(uint32 stats_id, hsrv_of_stats_t *p_stats_info);

int32
hsrv_openflow_stats_del_stats(uint32 stats_id);

void*
hsrv_openflow_stats_get_stats(uint32 stats_id);

int32
hsrv_openflow_msg_create_stats(hsrv_of_stats_t* p_stats);

int32
hsrv_openflow_msg_del_stats(hsrv_of_stats_t* p_stats);

int32
hsrv_openflow_msg_clear_stats(hsrv_of_stats_t* p_stats);

int32
hsrv_openflow_show_stats(FILE *fp);

int32
hsrv_openflow_stats_start(void);

int32
hsrv_openflow_stats_init(hsrv_system_spec_info_t* p_system_info);

#endif /* !__HSRV_OPENFLOW_STATS_H__ */
