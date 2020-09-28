#ifndef _CTC_SAI_COMMON_NOTIFIER_CALL_H_
#define _CTC_SAI_COMMON_NOTIFIER_CALL_H_

#ifdef __cplusplus
extern C {
#endif

#include <saitypes.h>
#include <saistatus.h>

#define CTC_SAI_NOTIFY_DONE		0x0000			/* Don't care */
#define CTC_SAI_NOTIFY_OK		0x0001			/* Suits me */
#define CTC_SAI_NOTIFY_STOP_MASK	0x8000		/* Don't call further */
#define CTC_SAI_NOTIFY_BAD		(CTC_SAI_NOTIFY_STOP_MASK|0x0002)

struct ctc_sai_notifier_block;

typedef	int32_t (*ctc_sai_notifier_fn_t)(struct ctc_sai_notifier_block *nb,
			uint32_t action, void *data);

struct ctc_sai_notifier_block {
	ctc_sai_notifier_fn_t notifier_call;
	struct ctc_sai_notifier_block *next;
	int priority;
};

struct ctc_sai_notifier_head {
	struct ctc_sai_notifier_block *head;
};

#define CTC_SAI_RAW_NOTIFIER_INIT(name)	{				\
		.head = NULL }

#define CTC_SAI_RAW_NOTIFIER_HEAD(name)					\
	struct ctc_sai_notifier_head name =				\
		CTC_SAI_RAW_NOTIFIER_INIT(name)

int32_t ctc_sai_raw_notifier_chain_register(struct ctc_sai_notifier_head *nh,
		struct ctc_sai_notifier_block *n);

int32_t ctc_sai_raw_notifier_chain_unregister(struct ctc_sai_notifier_head *nh,
		struct ctc_sai_notifier_block *n);

int32_t ctc_sai_raw_notifier_call_chain(struct ctc_sai_notifier_head *nh,
		uint32_t val, void *v);

int32_t ctc_sai_fdbevnet_notifier_register(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_fdbevnet_notifier_unregister(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_fdbevnet_notifier_call_notifiers(uint32_t val, void *v);

int32_t ctc_sai_neighbor_evnet_notifier_register(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_neighbor_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_neighbor_evnet_notifier_call_notifiers(uint32_t val, void *v);

int32_t ctc_sai_nexthop_evnet_notifier_register(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_nexthop_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_nexthop_evnet_notifier_call_notifiers(uint32_t val, void *v);

int32_t ctc_sai_lag_evnet_notifier_register(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_lag_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb);

int32_t ctc_sai_lag_evnet_notifier_call_notifiers(uint32_t val, void *v);

int32_t ctc_sai_vlan_evnet_notifier_register(struct ctc_sai_notifier_block *nb);
int32_t ctc_sai_vlan_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb);
int32_t ctc_sai_vlan_evnet_notifier_call_notifiers(uint32_t val, void *v);

#endif
