#if !defined (__SAI_L2MC_H_)
#define __SAI_L2MC_H_

#include <sail2mc.h>

sai_status_t
ctc_sai_l2mc_init();

sai_status_t
ctc_sai_l2mc_alloc_offset(uint32_t *popf_index);

sai_status_t
ctc_sai_l2mc_free_offset(uint32_t opf_index);

#endif /* !__SAI_L2MC_H_ */

