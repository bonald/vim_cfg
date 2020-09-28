
#ifndef __CTC_TTP_DRV_MIRROR_H__
#define __CTC_TTP_DRV_MIRROR_H__

#define CTC_START_MIRROR_ID  0
#define CTC_MAX_MIRROR_ID 4


int32
ctc_ttp_drv_mirror_id_alloc(uint32 *mirror_id);

int32
ctc_ttp_drv_mirror_id_free(uint32 mirror_id);

int32
ctc_ttp_drv_mirror_create(uint32 mirror_id, ctc_direction_t dir, uint32 gportid);

int32
ctc_ttp_drv_mirror_remove(uint32 mirror_id);

int32
ctc_ttp_drv_mirror_init();

#endif

