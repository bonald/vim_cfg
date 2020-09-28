#ifndef __TPOAM_LIST_H__
#define __TPOAM_LIST_H__


#ifndef list_for_tpoam

/* listnodes must always contain data to be valid. Adding an empty node
 * to a list is invalid
 */
struct listnode {
	struct listnode *next;
	struct listnode *prev;

	/* private member, use getdata() to retrieve, do not access directly */
	void *data;
};

struct list {
	struct listnode *head;
	struct listnode *tail;

	/* invariant: count is the number of listnodes in the list */
	unsigned int count;

	/*
	 * Returns -1 if val1 < val2, 0 if equal?, 1 if val1 > val2.
	 * Used as definition of sorted for listnode_add_sort
	 */
	int (*cmp)(void *val1, void *val2);

	/* callback to free user-owned data when listnode is deleted. supplying
	 * this callback is very much encouraged!
	 */
	void (*del)(void *val);
};

#define LISTCOUNT(X) (((X) != NULL) ? ((X)->count) : 0)
#define LISTHEAD(X) ((X)->head)
#define LISTTAIL(X) ((X)->tail)

/* List iteration macro. 
 * It allows to delete N and V in the middle of the loop 
 */
#define LIST_LOOP_DEL(L,V,N,NN) \
  if (L) \
    for ((N) = (L)->head, NN = ((N)!=NULL) ? (N)->next : NULL;  \
         (N);                                                   \
         (N) = (NN), NN = ((N)!=NULL) ? (N)->next : NULL)       \
      if (((V) = (N)->data) != NULL)


/* List iteration macro. */
#define LIST_LOOP(L,V,N) \
  if (L) \
    for ((N) = (L)->head; (N); (N) = (N)->next) \
      if (((V) = (N)->data) != NULL)

void list_delete_node(struct list *list, struct listnode *node);
void listnode_delete(struct list *list, void *val);
void listnode_add(struct list *list, void *val);
void list_free(struct list *l);

struct list *
list_new ();
#endif

#endif /* !__TPOAM_LIST_H__ */
