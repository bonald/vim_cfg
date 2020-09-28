#ifndef __CTCLIB_AVL_TREE_H__
#define __CTCLIB_AVL_TREE_H__

#include "sal.h"

/**********************************************************************************************
*Structures
*
**********************************************************************************************/
struct ctclib_avl_node_s
{
  struct ctclib_avl_node_s* left;       /* Left node. */
  struct ctclib_avl_node_s* right;      /* Right node. */
  struct ctclib_avl_node_s* parent;     /* Parent node. */
  int32 balance;                        /* Balance factor. */
  int32 rank;     /* Relative position of node in own subtree i.e. number of nodes in left subtree + 1. */
  int32 flag;     /* 0 -> left child of its parent or is root of the tree. 1 -> right child of its parent. */
  void* info;     /* Data. */
};
typedef struct ctclib_avl_node_s ctclib_avl_node_t;

struct ctclib_avl_tree_s
{
  int32 max_nodes ;             /* Maximum number of bytes for data. */
  int32 (*compare_function) (void* data1, void* data2); /* Compare function. */
  uint8* data;                 /* Array for preallocated entries. */
  ctclib_avl_node_t* root;      /* Root. */
  ctclib_avl_node_t* free_list; /* Free list. */
  int32 count;                  /* Number of entries in tree. */
};
typedef struct ctclib_avl_tree_s ctclib_avl_tree_t;

typedef int32 (*ctclib_avl_traversal_fn)(void* data, void* user_data); 
typedef int32 (*ctclib_avl_traversal2_fn)(void* data, void* user_data1,
                                 void* user_data2); 
typedef int32 (*ctclib_avl_traversal3_fn)(void* data, void* user_data1,
                                 void* user_data2, void* user_data3); 
/**********************************************************************************************
*macros
*
**********************************************************************************************/
#define CTCLIB_AVL_NODE_INFO(n)       (n)->info
#define CTCLIB_AVL_COUNT(t)           (t)->count
#define CTCLIB_AVL_NODE_LEFT(n)       (n)->left
#define CTCLIB_AVL_NODE_RIGHT(n)      (n)->right

#define CTCLIB_AVL_TREE_LOOP(T,V,N)\
  if (T)\
    for ((N) = ctclib_avl_top ((T)); (N); (N) = ctclib_avl_next ((N)))\
      if (((V) = (N)->info) != NULL)


#define CTCLIB_AVL_LOOP_DEL(T,V,N)\
  if (T) \
    for ((N) = ctclib_avl_top (T); N;(N) = ctclib_avl_top (T))\
      if (((V) = (N)->info) != NULL)


/**********************************************************************************************
*functions
*
**********************************************************************************************/
/* Delete node from AVL tree. */
extern int32 
ctclib_avl_delete_node (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node);

/* Create AVL tree. 
   If the max_nodes is 0, no preallocation is done. Every node is allocated
   first from the free_list and then from the system memory. */
extern int32 
ctclib_avl_create (ctclib_avl_tree_t** pp_avl_tree,
                  int32 max_nodes,
                  int32 (*compare_function) (void* data1, void* data2));

/* Traverse tree. */
extern int32 
ctclib_avl_traverse (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_traversal_fn fn, void* p_data);

extern int32 
ctclib_avl_traverse2 (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_traversal2_fn fn, 
                               void* p_data1, void* p_data2);

extern int32 
ctclib_avl_traverse3 (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_traversal3_fn fn,
                               void* p_data1, void* p_data2, void* p_data3);

/* Tree top. */
extern ctclib_avl_node_t* 
ctclib_avl_top (ctclib_avl_tree_t* p_avl_tree);

/* Get next entry in AVL tree. */
extern ctclib_avl_node_t* 
ctclib_avl_getnext (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node);

/* Delete AVL node containing the data. */
extern int32 
ctclib_avl_remove (ctclib_avl_tree_t* p_avl_tree, void* p_data);

/* Insert a node in AVL tree. */
extern int32 
ctclib_avl_insert (ctclib_avl_tree_t* p_avl_tree, void* p_data);

/* Lookup AVL tree. */
extern ctclib_avl_node_t* 
ctclib_avl_search (ctclib_avl_tree_t* p_avl_tree, void* p_data);

/* Get next node. */
extern ctclib_avl_node_t* 
ctclib_avl_next (ctclib_avl_node_t* p_node);

/* Get  AVL tree node count. */
extern int32 
ctclib_avl_get_tree_size(ctclib_avl_tree_t* p_avl_tree);

/*Tree cleanup. Remove all nodes from tree but do not free the tree.*/
extern int32 
ctclib_avl_tree_cleanup (ctclib_avl_tree_t* p_avl_tree, void (*avl_data_free)(void *ptr));

/* Free AVL tree . */
extern int32 
ctclib_avl_tree_free (ctclib_avl_tree_t** pp_avl_tree,void (*avl_data_free)(void *ptr));

#endif /*__CTCLIB_AVL_TREE_H__*/

