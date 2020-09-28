/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctclib_avl_tree.h"
#include "ctclib_memory.h"

/* Get a free AVL tree node. */
static ctclib_avl_node_t*
_ctclib_avl_node_new (ctclib_avl_tree_t* p_avl_tree)
{
    ctclib_avl_node_t* p_node = NULL;
    int32 node_sz;

    p_node = p_avl_tree->free_list;
    if (p_node)
    {
        p_avl_tree->free_list = p_node->right;
        p_avl_tree->count++;
    }
    else if (p_avl_tree->max_nodes == 0)
    {
        node_sz = sizeof (ctclib_avl_node_t);      
        p_node = (ctclib_avl_node_t*) XMALLOC(MEM_LIB_AVL_NODE, node_sz);
        if (!p_node)
        {
            return NULL;
        }
        p_avl_tree->count++;
    }

    return p_node;
}

/* Free a AVL tree node. */
static void
_ctclib_avl_node_free (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node)
{
  p_node->right = p_avl_tree->free_list;
  p_node->left = NULL;
  p_avl_tree->free_list = p_node;
  p_avl_tree->count--;

  return;
}

/* Free AVL free node list. */
static void
_ctclib_avl_freelist_free (ctclib_avl_tree_t* p_avl_tree)
{
    ctclib_avl_node_t* p_node = NULL;
    ctclib_avl_node_t* p_next = NULL;

    for (p_node = p_avl_tree->free_list; p_node; p_node = p_next)
    {
        p_next = p_node->right;
        if (p_node)
        {
            XFREE(MEM_LIB_AVL_NODE, p_node);
        }
        p_avl_tree->count--;
    }
}

/* Create AVL tree. 
   If the max_nodes is 0, no preallocation is done. Every node is allocated
   first from the free_list and then from the system memory. */
int32
ctclib_avl_create (ctclib_avl_tree_t** pp_avl_tree,
        int32 max_nodes,
        int32 (*compare_function) (void *data1, void *data2))
{
    int32 i;
    ctclib_avl_tree_t* p_avl = NULL;
    ctclib_avl_node_t* p_node = NULL;
    int32 node_sz;

    p_avl = (ctclib_avl_tree_t*) XMALLOC(MEM_LIB_AVL_TREE, sizeof (ctclib_avl_tree_t));
    if (!p_avl)
    {
        return -1;
    }

    p_avl->max_nodes = max_nodes;
    p_avl->root = NULL;
    p_avl->compare_function = compare_function;

    /* Initialize free list. */
    p_avl->free_list = NULL;
  
    /* Initialize max count. */
    p_avl->count = max_nodes;

    /* Check if nodes are to be preallocated. */
    if (max_nodes != 0)
    {
        /* Preallocate maximum number of entries and put on free list. */
        node_sz = sizeof (ctclib_avl_node_t);

        p_avl->data = (uint8*)XMALLOC(MEM_LIB_AVL_NODE, (node_sz * max_nodes));
        if (!p_avl->data)
        {
            XFREE(MEM_LIB_AVL_TREE, p_avl);
            return -1;
        }

        /* Add all entries in the free list. */
        for (i = 0; i < max_nodes; i++)
        {
            p_node = (ctclib_avl_node_t*) (p_avl->data + i * node_sz);
            _ctclib_avl_node_free (p_avl, p_node);
        }
    }

    *pp_avl_tree = p_avl;

    return 0;
}

/* Delete AVL tree. */
static int32
ctclib_avl_finish (ctclib_avl_tree_t* p_avl_tree)
{
    /* Check tree nodes allocation type. */
    if(p_avl_tree->max_nodes == 0)
    {
        /* Free free list. */
        _ctclib_avl_freelist_free (p_avl_tree);
    }
    else if(p_avl_tree->data)
    {
        /* Free all preallocated nodes. */
        XFREE(MEM_LIB_AVL_NODE, p_avl_tree->data);
    } 

    /* Free tree. */
    XFREE(MEM_LIB_AVL_TREE, p_avl_tree);

    return 0;
}

/* Internal function for traversal. */
static int32
_ctclib_avl_traverse (ctclib_avl_tree_t* p_avl_tree,
            ctclib_avl_node_t* p_node,
            ctclib_avl_traversal_fn fn,
            void* p_data)
{
    int32  ret;
                                                                               
    if (p_node != NULL) 
    {
        if ((ret = _ctclib_avl_traverse(p_avl_tree, p_node->left, fn, p_data)) < 0) 
            return ret;

        if ((ret = (*fn)(p_node->info, p_data)) < 0) 
            return ret;
                                                                               
        if ((ret = _ctclib_avl_traverse(p_avl_tree, p_node->right, fn, p_data)) < 0) 
            return ret;
    }

    return 0;
}

static int32
_ctclib_avl_traverse2 (ctclib_avl_tree_t* p_avl_tree,
                ctclib_avl_node_t* p_node,
                ctclib_avl_traversal2_fn fn,
                void* p_data1, void* p_data2)
{
    int32  ret;

    if (p_node != NULL)
    {
        if ((ret = _ctclib_avl_traverse2(p_avl_tree, p_node->left, fn, p_data1, p_data2)) < 0)
            return ret;

        if ((ret = (*fn)(p_node->info, p_data1, p_data2)) < 0)
            return ret;

        if ((ret = _ctclib_avl_traverse2(p_avl_tree, p_node->right, fn, p_data1, p_data2)) < 0)
            return ret;
    }

    return 0;
}

static int32
_ctclib_avl_traverse3 (ctclib_avl_tree_t* p_avl_tree,
                ctclib_avl_node_t* p_node,
                ctclib_avl_traversal3_fn fn,
                void* p_data1, void* p_data2, void* p_data3)
{
    int32  ret;

    if (p_node != NULL)
    {
        if ((ret = _ctclib_avl_traverse3(p_avl_tree, p_node->left, fn, p_data1, p_data2, p_data3)) < 0)
            return ret;

        if ((ret = (*fn)(p_node->info, p_data1, p_data2, p_data3)) < 0)
            return ret;

        if ((ret = _ctclib_avl_traverse3(p_avl_tree, p_node->right, fn, p_data1, p_data2, p_data3)) < 0)
            return ret;
    }

    return 0;
}

/* Traverse tree. */
int32
ctclib_avl_traverse (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_traversal_fn fn, void* p_data)
{
  return _ctclib_avl_traverse (p_avl_tree, p_avl_tree->root, fn, p_data);
}

int32
ctclib_avl_traverse2 (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_traversal2_fn fn, void* p_data1, void* p_data2)
{
  return _ctclib_avl_traverse2 (p_avl_tree, p_avl_tree->root, fn, p_data1, p_data2);
}

int32
ctclib_avl_traverse3 (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_traversal3_fn fn, void* p_data1, void* p_data2, void* p_data3)
{
  return _ctclib_avl_traverse3 (p_avl_tree, p_avl_tree->root, fn, p_data1, p_data2, p_data3);
}

/* Tree top. */
ctclib_avl_node_t*
ctclib_avl_top (ctclib_avl_tree_t* p_avl_tree)
{
  return (p_avl_tree != NULL) ? p_avl_tree->root : NULL;
}

/* Internal function to lookup node. */
static ctclib_avl_node_t*
_ctclib_avl_lookup (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node, void* p_data)
{    
    int32 cmp;

    if (p_node == NULL)
    {
        return NULL;
    }

    cmp = (*(p_avl_tree->compare_function)) (p_data, p_node->info);
    if (cmp < 0)
        return _ctclib_avl_lookup (p_avl_tree, p_node->left, p_data);
    if (cmp > 0)
        return _ctclib_avl_lookup (p_avl_tree, p_node->right, p_data);

    return p_node;
}

/* Lookup AVL tree. */
ctclib_avl_node_t*
ctclib_avl_search (ctclib_avl_tree_t* p_avl_tree, void* p_data)
{
    if (p_avl_tree->root == NULL)
    {
        return NULL;
    }

    return _ctclib_avl_lookup (p_avl_tree, p_avl_tree->root, p_data);
}

/* Next node. */
ctclib_avl_node_t* 
ctclib_avl_next (ctclib_avl_node_t* p_node)
{
//    ctclib_avl_node_t* p_start = NULL;
    int32 count = 0;

    if (p_node->left)
    {
        return p_node->left;
    }

    if (p_node->right)
    {
        return p_node->right;
    }

//    p_start = p_node;
    while (p_node->parent)
    {
        if ((p_node->parent->left == p_node) && p_node->parent->right)
        {
            return p_node->parent->right;
        }
        count++;

        p_node = p_node->parent;
    }

    return NULL;
}

/* Get next node. */
ctclib_avl_node_t* 
ctclib_avl_getnext (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node)
{     
    ctclib_avl_node_t* p_nodeA = NULL;
    ctclib_avl_node_t* p_nodeB = NULL;

    if (p_avl_tree->root == NULL) 
    {
        return NULL;
    }
    
    p_nodeA = p_node;
    p_nodeB = (p_nodeA == NULL ? p_avl_tree->root : p_nodeA->right);
    if (p_nodeB == NULL)
    {  
        /* Go up parent from the left subtree */
        for (;;)
        {  
            p_nodeB = p_nodeA->parent;
            if (p_nodeB == NULL)
                break;
            if (p_nodeA->flag == 0)
                break;
            p_nodeA = p_nodeB;
        }
    }
    else
    {  
        /* go downstairs into the right subtree */
        for (;;)
        {  
            p_nodeA = p_nodeB->left;
            if (p_nodeA == NULL)
                break;
            p_nodeB = p_nodeA;
        }
    }
    return p_nodeB;
}

/* Rotate tree for balancing. */
static ctclib_avl_node_t*
_ctclib_avl_rotate (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node)
{
    ctclib_avl_node_t *nodeA, *nodeB, *nodeC, *nodeD, *nodeE, *nodeF;

    nodeB = p_node;

    if (nodeB->balance < 0)
    {  
        /* Negative (left) rotation */
        nodeA = nodeB->parent; 
        nodeC = nodeB->left; 
        nodeD = nodeC->right;
        if (nodeC->balance <= 0)
        {  
            /* Single negative rotation */
            if (nodeA == NULL)
                p_avl_tree->root = nodeC;
            else if (nodeB->flag == 0) 
                nodeA->left = nodeC; 
            else 
                nodeA->right = nodeC;
            
            nodeB->rank -= nodeC->rank;
            nodeC->parent = nodeA; 
            nodeC->flag = nodeB->flag; 
            nodeC->balance++; 
            nodeC->right = nodeB;
            nodeB->parent = nodeC; 
            nodeB->flag = 1;
            nodeB->balance = (-nodeC->balance); 
            nodeB->left = nodeD;
            if (nodeD != NULL) 
            {
                nodeD->parent = nodeB;
                nodeD->flag = 0;
            }
            p_node = nodeC;
        }
        else
        {  
            /* Double negative rotation */
            nodeE = nodeD->left; nodeF = nodeD->right;
            if (nodeA == NULL)
                p_avl_tree->root = nodeD;
            else  if (nodeB->flag == 0) 
                nodeA->left = nodeD; else nodeA->right = nodeD;
            
            nodeB->rank -= (nodeC->rank + nodeD->rank);
            nodeD->rank += nodeC->rank;
            nodeB->balance = (nodeD->balance >= 0 ? 0 : +1);
            nodeC->balance = (nodeD->balance <= 0 ? 0 : -1);
            nodeD->parent = nodeA; 
            nodeD->flag = nodeB->flag; 
            nodeD->balance = 0;
            nodeD->left = nodeC; 
            nodeD->right = nodeB;
            nodeB->parent = nodeD; 
            nodeB->flag = 1; 
            nodeB->left = nodeF;
            nodeC->parent = nodeD; 
            nodeC->flag = 0; 
            nodeC->right = nodeE;
            if (nodeE != NULL) 
            {
                nodeE->parent = nodeC;
                nodeE->flag = 1;
            }
            if (nodeF != NULL)
            {
                nodeF->parent = nodeB;
                nodeF->flag = 0;
            }
            p_node = nodeD;
        }
    }
    else
    {  
        /* Positive (right) rotation */
        nodeA = nodeB->parent; 
        nodeC = nodeB->right; 
        nodeD = nodeC->left;
        if (nodeC->balance >= 0)
        {  
            /* Single positive rotation */
            if (nodeA == NULL)
                p_avl_tree->root = nodeC;
            else if (nodeB->flag == 0) 
                nodeA->left = nodeC; 
            else 
                nodeA->right = nodeC;
            
            nodeC->rank += nodeB->rank;
            nodeC->parent = nodeA; 
            nodeC->flag = nodeB->flag; 
            nodeC->balance--; 
            nodeC->left = nodeB;
            nodeB->parent = nodeC; 
            nodeB->flag = 0;
            nodeB->balance = (-nodeC->balance); 
            nodeB->right = nodeD;
            if (nodeD != NULL)
            {
                nodeD->parent = nodeB;
                nodeD->flag = 1;
            }
            p_node = nodeC;
        }
        else
        {  
            /* Double positive rotation */
            nodeE = nodeD->left; 
            nodeF = nodeD->right;
            if (nodeA == NULL)
                p_avl_tree->root = nodeD;
            else if (nodeB->flag == 0) 
                nodeA->left = nodeD; 
            else 
                nodeA->right = nodeD;
            
            nodeC->rank -= nodeD->rank;
            nodeD->rank += nodeB->rank;
            nodeB->balance = (nodeD->balance <= 0 ? 0 : -1);
            nodeC->balance = (nodeD->balance >= 0 ? 0 : +1);
            nodeD->parent = nodeA; 
            nodeD->flag = nodeB->flag; 
            nodeD->balance = 0;
            nodeD->left = nodeB; 
            nodeD->right = nodeC;
            nodeB->parent = nodeD; 
            nodeB->flag = 0; 
            nodeB->right = nodeE;
            nodeC->parent = nodeD; 
            nodeC->flag = 1; 
            nodeC->left = nodeF;
            if (nodeE != NULL)
            {
                nodeE->parent = nodeB;
                nodeE->flag = 1;
            }
            if (nodeF != NULL)
            {
                nodeF->parent = nodeC;
                nodeF->flag = 0;
            }
            p_node = nodeD;
        }
    }
    return p_node;
}

/* Insert a node in the AVL tree. */
int32
ctclib_avl_insert (ctclib_avl_tree_t* p_avl_tree, void* p_data)
{
    ctclib_avl_node_t *nodeA, *nodeB, *nodeC;
    int32 flag = 0;

    /* Create a new node. */
    nodeC = _ctclib_avl_node_new (p_avl_tree);
    if (nodeC == NULL)
    {
        return -1;
    }

    /* Find location for insertion. */
    nodeA = NULL;
    nodeB = p_avl_tree->root;

    while (nodeB != NULL)
    {
        nodeA = nodeB;
        if (p_avl_tree->compare_function (p_data, nodeA->info) <= 0)
        {
            flag = 0;
            nodeB = nodeA->left;
            nodeA->rank++;
        }
        else
        {
            flag = 1;
            nodeB = nodeA->right;
        }
    }

    nodeC->info = p_data;
    nodeC->rank = 1;
    nodeC->parent = nodeA;
    nodeC->flag = (nodeA == NULL ? 0 : flag);
    nodeC->balance = 0;
    nodeC->left = nodeC->right = NULL;
    
    if (nodeA == NULL)
        p_avl_tree->root = nodeC;
    else if (flag == 0)
        nodeA->left = nodeC;
    else
        nodeA->right = nodeC;

    /* Go parent root and correct all subtrees affected by insertion. */
    while (nodeA != NULL)
    {
        if (flag == 0)
        {
            /* Height of p's left subtree increased. */
            if (nodeA->balance > 0)
            {
                nodeA->balance = 0;
                break;
            }
            if (nodeA->balance < 0)
            {
                _ctclib_avl_rotate (p_avl_tree, nodeA);
                break;
            }
            nodeA->balance = -1;
            flag = nodeA->flag;
            nodeA = nodeA->parent;
        }
        else
        {
            /* Height of p's right subtree increased. */
            if (nodeA->balance < 0)
            {
                nodeA->balance = 0;
                break;
            }
            if (nodeA->balance > 0)
            {
                _ctclib_avl_rotate (p_avl_tree, nodeA);
                break;
            }
            nodeA->balance = +1;
            flag = nodeA->flag;
            nodeA = nodeA->parent;
        }
    }
    return 0;
}


/* Remove a node entry. */
static int32
_ctclib_avl_delete_node (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node)
{
    ctclib_avl_node_t *nodeA, *nodeB, *nodeC, *nodeD, *nodeE, *nodeF, *nodeG;
    int32 flag;

    if (p_node == NULL)
    {
        return -1;
    }
    nodeB = p_node;

    /* If both subtrees of the specified node are non-empty, the node
     should be interchanged with the next one, at least one subtree
     of which is always empty. */
    if (nodeB->left == NULL || nodeB->right == NULL) 
    {
        goto SKIP;
    }

    nodeA = nodeB->parent; 
    nodeC = nodeB->left;
  
    nodeD = ctclib_avl_getnext (p_avl_tree, nodeB); 
    nodeE = nodeD->right;
    if (nodeB->right == nodeD)
    {  
        if (nodeA == NULL)
            p_avl_tree->root = nodeD;
        else if (nodeB->flag == 0) 
            nodeA->left = nodeD; 
        else 
            nodeA->right = nodeD;
        
        nodeD->rank = nodeB->rank; 
        nodeD->parent = nodeA;
        nodeD->flag = nodeB->flag; 
        nodeD->balance = nodeB->balance;
        nodeD->left = nodeC; 
        nodeD->right = nodeB;
        nodeC->parent = nodeD;
        nodeB->rank = 1; 
        nodeB->parent = nodeD; 
        nodeB->flag = 1;
        nodeB->balance = (nodeE == NULL ? 0 : +1);
        nodeB->left = NULL; 
        nodeB->right = nodeE;
    
        if (nodeE != NULL) 
            nodeE->parent = nodeB;
    }
    else
    {  
        nodeF = nodeB->right; 
        nodeG = nodeD->parent;

        if (nodeA == NULL)
            p_avl_tree->root = nodeD;
        else if (nodeB->flag == 0) 
            nodeA->left = nodeD; 
        else 
            nodeA->right = nodeD;

        nodeD->rank = nodeB->rank; 
        nodeD->parent = nodeA;
        nodeD->flag = nodeB->flag; 
        nodeD->balance = nodeB->balance;
        nodeD->left = nodeC; 
        nodeD->right = nodeF;
        nodeC->parent = nodeD; 
        nodeF->parent = nodeD; 
        nodeG->left = nodeB;
        nodeB->rank = 1; 
        nodeB->parent = nodeG; 
        nodeB->flag = 0;
        nodeB->balance = (nodeE == NULL ? 0 : +1);
        nodeB->left = NULL; 
        nodeB->right = nodeE;
      
        if (nodeE != NULL) 
            nodeE->parent = nodeB;
    }

 SKIP:
    /* Now the specified node [p] has at least one empty subtree;
     go up to the root and adjust the rank field of all nodes
     affected by deletion */
    nodeC = nodeB; 
    nodeA = nodeC->parent;
    while (nodeA != NULL)
    {  
        if (nodeC->flag == 0) 
            nodeA->rank--;
        
        nodeC = nodeA; 
        nodeA = nodeC->parent;
    }

    /* delete the specified node from the tree */
    nodeA = nodeB->parent; 
    flag = nodeB->flag;
    nodeC = nodeB->left != NULL ? nodeB->left : nodeB->right;
  
    if (nodeA == NULL)
        p_avl_tree->root = nodeC;
    else if (flag == 0) 
        nodeA->left = nodeC; 
    else 
        nodeA->right = nodeC;
    
    if (nodeC != NULL) 
    {
        nodeC->parent = nodeA;
        nodeC->flag = flag;
    }
  
    /* Go up to the root and correct all subtrees affected by
     deletion */
    while (nodeA != NULL)
    {  
        if (flag == 0)
        {  
            /* the height of the left subtree of [f] is decreased */
            if (nodeA->balance == 0)
            {  
                nodeA->balance = +1;
                break;
            }
            if (nodeA->balance < 0)
                nodeA->balance = 0;
            else
            {  
                nodeA = _ctclib_avl_rotate (p_avl_tree, nodeA);
                if (nodeA->balance < 0) 
                    break;
            }
            flag = nodeA->flag; 
            nodeA = nodeA->parent;
        }
        else
        {  
            /* the height of the right subtree of [f] is decreased */
            if (nodeA->balance == 0)
            {  
                nodeA->balance = -1;
                break;
            }
            if (nodeA->balance > 0)
                nodeA->balance = 0;
            else
            {  
                nodeA = _ctclib_avl_rotate (p_avl_tree, nodeA);
                if (nodeA->balance > 0)
                    break;
            }
            flag = nodeA->flag; 
            nodeA = nodeA->parent;
        }
    }
    
    /* Free node. */
    _ctclib_avl_node_free (p_avl_tree, nodeB);
    return 0;
}

/* Delete node containing data. */
int32
ctclib_avl_remove (ctclib_avl_tree_t* p_avl_tree, void* p_data)
{
    ctclib_avl_node_t* p_node = NULL;
    int32 ret = 0;

    p_node =  ctclib_avl_search (p_avl_tree, p_data);
    if (p_node)
    {
        /* Delete node. */
        ret = _ctclib_avl_delete_node (p_avl_tree, p_node);      
    }
    else
    {
        ret = -1;
    }
  
    return ret;
}

/* Delete node from AVL tree. */
int32 
ctclib_avl_delete_node (ctclib_avl_tree_t* p_avl_tree, ctclib_avl_node_t* p_node)
{
  return _ctclib_avl_delete_node (p_avl_tree, p_node);
}

/* Get  AVL tree node count. */
int32 
ctclib_avl_get_tree_size(ctclib_avl_tree_t* p_avl_tree)
{
  return p_avl_tree->count;
}

/*  tree free function.*/
int32
ctclib_avl_tree_free (ctclib_avl_tree_t** pp_avl_tree, void (*avl_data_free)(void *ptr))
{
    ctclib_avl_node_t* p_tmp_node = NULL;
    ctclib_avl_node_t* p_node = NULL;

    if (*pp_avl_tree == NULL)
    {
        return 0;
    }

    p_node = ctclib_avl_top (*pp_avl_tree);
    while (p_node)
    {
        if (p_node->left)
        {
            p_node = p_node->left;
            continue;
        }

        if (p_node->right)
        {
            p_node = p_node->right;
            continue;
        }

        p_tmp_node = p_node;
        p_node = p_node->parent;

        if (p_node != NULL)
        {
            if (p_node->left == p_tmp_node)
                p_node->left = NULL;
            else
                p_node->right = NULL;

            /* Free node data */
            if(avl_data_free)
            {
                (*avl_data_free)(p_tmp_node->info);
            }

            /* Move node to a free list.*/ 
            _ctclib_avl_node_free (*pp_avl_tree, p_tmp_node);
        }
        else
        {
            /* Free node data */
            if(avl_data_free)
            {
                (*avl_data_free)(p_tmp_node->info);
            }

            /* Move node to a free list.*/ 
            _ctclib_avl_node_free (*pp_avl_tree, p_tmp_node);
            break;
        }
    }

    /* Delete tree. */
    ctclib_avl_finish (*pp_avl_tree);
    *pp_avl_tree = NULL;
    return 0;
}

/*Tree cleanup. Remove all nodes from tree but do not free the tree.*/
int32
ctclib_avl_tree_cleanup (ctclib_avl_tree_t* p_avl_tree,void (*avl_data_free)(void *ptr))
{
    ctclib_avl_node_t* p_tmp_node = NULL;
    ctclib_avl_node_t* p_node = NULL;

    if (p_avl_tree == NULL)
    {
        return -1;
    }

    p_node = ctclib_avl_top (p_avl_tree);
    while (p_node)
    {
        if (p_node->left)
        {
            p_node = p_node->left;
            continue;
        }

        if (p_node->right)
        {
            p_node = p_node->right;
            continue;
        }

        p_tmp_node = p_node;
        p_node = p_node->parent;

        if (p_node != NULL)
        {
            if (p_node->left == p_tmp_node)
                p_node->left = NULL;
            else
                p_node->right = NULL;

            /* Free node data */
            if(avl_data_free)
            {
                (*avl_data_free)(p_tmp_node->info);
            }

            /* Move node to a free list.*/ 
            _ctclib_avl_node_free (p_avl_tree, p_tmp_node);
        }
        else
        {
            /* Free node data */
            if(avl_data_free)
            {
                (*avl_data_free)(p_tmp_node->info);
            }

            /* Move node to a free list.*/ 
            _ctclib_avl_node_free (p_avl_tree, p_tmp_node);
            break;
        }
    }

    p_avl_tree->root = NULL;
    return 0;
}


