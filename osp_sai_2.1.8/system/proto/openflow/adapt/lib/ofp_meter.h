#ifndef __OFP_METER_H__
#define __OFP_METER_H__

struct ofp_meter_info_s
{
    uint32_t meter_id;
};
typedef struct ofp_meter_info_s ofp_meter_info_t;

#define OFP_METER_INVALID_METER_ID      0
#define OFP_MAX_BAND_PER_METER          1        /**< max band per meter */

#endif /* !__OFP_METER_H__ */
