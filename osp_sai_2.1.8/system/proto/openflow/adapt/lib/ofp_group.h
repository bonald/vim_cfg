#ifndef __OFP_GROUP_H__
#define __OFP_GROUP_H__

/* group num */
#define OFP_GROUP_NUM_TYPE_ALL          160
#define OFP_GROUP_NUM_TYPE_SELECT       63
#define OFP_GROUP_NUM_TYPE_INDIRECT     63
#define OFP_GROUP_NUM_TYPE_FF           63
#define OFP_GROUP_NUM_TYPE_LSP_APS           500
#define OFP_GROUP_NUM_TYPE_PW_APS           500

/* group bucket num */
#define OFP_BUCKET_NUM_PER_ALL_GROUP         GLB_BUCKET_NUM_PER_ALL_GROUP
#define OFP_BUCKET_NUM_PER_SELECT_GROUP      GLB_BUCKET_NUM_PER_SELECT_GROUP
#define OFP_BUCKET_NUM_PER_INDIRECT_GROUP    GLB_BUCKET_NUM_PER_INDIRECT_GROUP
#define OFP_BUCKET_NUM_PER_FF_GROUP          OFP_BUCKET_NUM_PER_FF_GROUP

/* group limit */
#define OFP_REF_GROUP_NUM_PER_BUCKET        1
#define OFP_OUTPUT_NUM_PER_SELECT_BUCKET    1
#define OFP_GROUP_INVALID_GROUP_ID          0xffffffff


/* should be same with ofp11_group_type */
enum ofp_group_type_e
{
    OFP_GROUP_TYPE_ALL,      /* All (multicast/broadcast) group. */
    OFP_GROUP_TYPE_SELECT,   /* Select group. */
    OFP_GROUP_TYPE_INDIRECT, /* Indirect group. */
    OFP_GROUP_TYPE_FF,       /* Fast failover group. */
    OFP_GROUP_TYPE_PW_APS=6,
    OFP_GROUP_TYPE_LSP_APS=7,

    OFP_GROUP_TYPE_MAX       /* Fast failover group. */
};
typedef enum ofp_group_type_e ofp_group_type_t;

struct ofp_group_info_s
{
    uint32_t group_id;
};
typedef struct ofp_group_info_s ofp_group_info_t;

#endif /* !__OFP_GROUP_H__ */
