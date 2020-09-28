#ifndef __OFP_QUEUE_H__
#define __OFP_QUEUE_H__

struct ofp_queue_info_s
{
    uint32_t queue_id;
};
typedef struct ofp_queue_info_s ofp_queue_info_t;

#define OFP_QUEUE_MAX_VALUE             8
#define OFP_QUEUE_INVALID_QUEUE_ID      OFP_QUEUE_MAX_VALUE


#endif /* !__OFP_QUEUE_H__ */
