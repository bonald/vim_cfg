
#ifndef __HAL_API_RATELIMIT_H__
#define __HAL_API_RATELIMIT_H__

int32
fei_ratelimit_set_enable(fei_ratelimit_set_en_req_t* pst_req);

int32
fei_ratelimit_set_thresh(fei_ratelimit_set_thresh_req_t* pst_req);

int32
fei_ratelimit_set_ipg_enable(fei_ratelimit_set_ipg_enable_req_t* pst_req);

#endif /* !__HAL_API_RATELIMIT_H__ */

