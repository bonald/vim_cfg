#include "proto.h"

#include "lib_fs.h"
#include "tpoam_list.h"
#if 0
#include "tpoam_sock.h"

#include "tpoam_pdu.h"
#include "tpoam_lm.h"
#include "tpoam_event.h"
#include "tpoam_dm.h"
#include "tpoam_error.h"
#include "tpoam_api.h"
#endif
#ifndef list_func
/* Free list. */
void list_free(struct list *l)
{
	XFREE(MTYPE_LINK_LIST, l);
}

/* Free listnode. */
static void listnode_free(struct listnode *node)
{
	XFREE(MTYPE_LINK_NODE, node);
}

/* Delete the node from list.  For ospfd and ospf6d. */
void list_delete_node(struct list *list, struct listnode *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else
		list->tail = node->prev;
	list->count--;
	listnode_free(node);
}


/*from cnos frr*/
/* Delete specific date pointer from the list. */
void listnode_delete(struct list *list, void *val)
{
	struct listnode *node;

	assert(list);
	for (node = list->head; node; node = node->next) {
		if (node->data == val) {
			if (node->prev)
				node->prev->next = node->next;
			else
				list->head = node->next;

			if (node->next)
				node->next->prev = node->prev;
			else
				list->tail = node->prev;

			list->count--;
			listnode_free(node);
			return;
		}
	}
}


/* Allocate new listnode.  Internal use only. */
static struct listnode *listnode_new(void)
{
	return XCALLOC(MTYPE_LINK_NODE, sizeof(struct listnode));
}

/* Add new data to the list. */
void listnode_add(struct list *list, void *val)
{
	struct listnode *node;

	assert(val != NULL);

	node = listnode_new();

	node->prev = list->tail;
	node->data = val;

	if (list->head == NULL)
		list->head = node;
	else
		list->tail->next = node;
	list->tail = node;

	list->count++;
}

/* Allocate new list. */
struct list *
list_new ()
{
  struct list *new;

  new = XCALLOC (MTYPE_LINK_LIST, sizeof (struct list));
  return new;
}
#endif


