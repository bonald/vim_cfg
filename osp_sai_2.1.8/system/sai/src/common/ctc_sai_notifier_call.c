#include <saitypes.h>
#include <saistatus.h>
#include <sal.h>
#include <ctc_sai_notifier_call.h>

static int32_t ctc_sai_notifier_chain_register(struct ctc_sai_notifier_block **nl,
		struct ctc_sai_notifier_block *n)
{
	while ((*nl) != NULL) {
		if (n->priority > (*nl)->priority)
			break;
		nl = &((*nl)->next);
	}
	n->next = *nl;
    *nl = n;
	return 0;
}

static int32_t ctc_sai_notifier_chain_unregister(struct ctc_sai_notifier_block **nl,
		struct ctc_sai_notifier_block *n)
{
	while ((*nl) != NULL) {
		if ((*nl) == n) {
			*nl = n->next;
			return 0;
		}
		nl = &((*nl)->next);
	}
	return SAI_STATUS_SUCCESS;
}

int32_t ctc_sai_raw_notifier_chain_register(struct ctc_sai_notifier_head *nh,
		struct ctc_sai_notifier_block *n)
{
	return ctc_sai_notifier_chain_register(&nh->head, n);
}

int32_t ctc_sai_raw_notifier_chain_unregister(struct ctc_sai_notifier_head *nh,
		struct ctc_sai_notifier_block *n)
{
	return ctc_sai_notifier_chain_unregister(&nh->head, n);
}

static int32_t ctc_sai_notifier_call_chain(struct ctc_sai_notifier_block **nl,
			       uint32_t val, void *v,
			       int32_t nr_to_call, int32_t *nr_calls)
{
	int32_t ret = SAI_STATUS_SUCCESS;
	struct ctc_sai_notifier_block *nb, *next_nb;

	nb = *nl;

	while (nb && nr_to_call) {
		next_nb = nb->next;

		ret = nb->notifier_call(nb, val, v);

		if (nr_calls)
			(*nr_calls)++;

		if ((ret & CTC_SAI_NOTIFY_STOP_MASK) == CTC_SAI_NOTIFY_STOP_MASK)
			break;
		nb = next_nb;
		nr_to_call--;
	}
	return ret;
}

int32_t __ctc_sai_raw_notifier_call_chain(struct ctc_sai_notifier_head *nh,
			      uint32_t val, void *v,
			      int32_t nr_to_call, int32_t *nr_calls)
{
	return ctc_sai_notifier_call_chain(&nh->head, val, v, nr_to_call, nr_calls);
}

int32_t ctc_sai_raw_notifier_call_chain(struct ctc_sai_notifier_head *nh,
		uint32_t val, void *v)
{
	return __ctc_sai_raw_notifier_call_chain(nh, val, v, -1, NULL);
}


CTC_SAI_RAW_NOTIFIER_HEAD(fdb_notifier_event);

int32_t ctc_sai_fdbevnet_notifier_register(struct ctc_sai_notifier_block *nb)
{
	int32_t err;

	err = ctc_sai_raw_notifier_chain_register(&fdb_notifier_event, nb);
	return err;
}

int32_t ctc_sai_fdbevnet_notifier_unregister(struct ctc_sai_notifier_block *nb)
{
	return ctc_sai_raw_notifier_chain_unregister(&fdb_notifier_event, nb);;
}

int32_t ctc_sai_fdbevnet_notifier_call_notifiers(uint32_t val, void *v)
{
	return ctc_sai_raw_notifier_call_chain(&fdb_notifier_event, val, v);
}

CTC_SAI_RAW_NOTIFIER_HEAD(nexthop_notifier_event);

int32_t ctc_sai_nexthop_evnet_notifier_register(struct ctc_sai_notifier_block *nb)
{
	int32_t err;

	err = ctc_sai_raw_notifier_chain_register(&nexthop_notifier_event, nb);
	return err;
}

int32_t ctc_sai_nexthop_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb)
{
	return ctc_sai_raw_notifier_chain_unregister(&nexthop_notifier_event, nb);;
}

int32_t ctc_sai_nexthop_evnet_notifier_call_notifiers(uint32_t val, void *v)
{
	return ctc_sai_raw_notifier_call_chain(&nexthop_notifier_event, val, v);
}

CTC_SAI_RAW_NOTIFIER_HEAD(lag_notifier_event);

int32_t ctc_sai_lag_evnet_notifier_register(struct ctc_sai_notifier_block *nb)
{
	int32_t err;

	err = ctc_sai_raw_notifier_chain_register(&lag_notifier_event, nb);
	return err;
}

int32_t ctc_sai_lag_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb)
{
	return ctc_sai_raw_notifier_chain_unregister(&lag_notifier_event, nb);;
}

int32_t ctc_sai_lag_evnet_notifier_call_notifiers(uint32_t val, void *v)
{
	return ctc_sai_raw_notifier_call_chain(&lag_notifier_event, val, v);
}

CTC_SAI_RAW_NOTIFIER_HEAD(vlan_notifier_event);

int32_t ctc_sai_vlan_evnet_notifier_register(struct ctc_sai_notifier_block *nb)
{
	int32_t err;

	err = ctc_sai_raw_notifier_chain_register(&vlan_notifier_event, nb);
	return err;
}

int32_t ctc_sai_vlan_evnet_notifier_unregister(struct ctc_sai_notifier_block *nb)
{
	return ctc_sai_raw_notifier_chain_unregister(&vlan_notifier_event, nb);;
}

int32_t ctc_sai_vlan_evnet_notifier_call_notifiers(uint32_t val, void *v)
{
	return ctc_sai_raw_notifier_call_chain(&vlan_notifier_event, val, v);
}