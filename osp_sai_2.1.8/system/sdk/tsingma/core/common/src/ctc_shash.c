#include "sal.h"
#include "ctc_shash.h"

STATIC ctc_shash_node_t *ctc_shash_find__(const ctc_shash_t *,
                                       const char *name, uint32 name_len,
                                       uint32 hash);

static INLINE
uint32 ctc_hash_add(uint32 hash, uint32 data)
{
    data *= 0xcc9e2d51;
    data = CTC_SHASH_ROT(data, 15);
    data *= 0x1b873593;
    hash = hash ^ data;
    hash = CTC_SHASH_ROT(hash, 13);
    return hash * 5 + 0xe6546b64;
}

static INLINE
uint32 ctc_hash_finish(uint32 hash, uint32 final)
{

    hash = hash ^ final;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;

}

STATIC uint32
ctc_shash_bytes(const void *p_, uint32 n, uint32 basis)
{
    const uint32 *p = p_;
    uint32 orig_n = n;
    uint32 hash = 0;

    hash = basis;
    while (n >= 4) {
        hash = ctc_hash_add(hash, (uint32)(*p));
        n -= 4;
        p += 1;
    }

    if (n) {
        uint32 tmp = 0;

        sal_memcpy(&tmp, p, n);
        hash = ctc_hash_add(hash, tmp);
    }

    return ctc_hash_finish(hash, orig_n);
}



STATIC uint32
ctc_shash_name(const char *name)
{
    return ctc_shash_bytes(name, sal_strlen(name), 0);
}

void
ctc_shash_init(ctc_shash_t *sh)
{
    ctc_hmap_init(&sh->map);
}

void
ctc_shash_destroy(ctc_shash_t *sh)
{
    if (sh) {
        ctc_shash_clear(sh);
        ctc_hmap_destroy(&sh->map);
    }
}

/* Like shash_destroy(), but also free() each node's 'data'. */
void
ctc_shash_destroy_free_data(ctc_shash_t *sh)
{
    if (sh)
    {
        ctc_shash_clear_free_data(sh);
        ctc_hmap_destroy(&sh->map);
    }
}

void
ctc_shash_swap(ctc_shash_t *a, ctc_shash_t *b)
{
    ctc_hmap_swap(&a->map, &b->map);
}

void
ctc_shash_moved(ctc_shash_t *sh)
{
    ctc_hmap_moved(&sh->map);
}

void
ctc_shash_clear(ctc_shash_t *sh)
{
    ctc_shash_node_t *node = NULL;
    ctc_shash_node_t *next = NULL;

    SHASH_FOR_EACH_SAFE (node, next, sh)
    {
        ctc_hmap_remove(&sh->map, &node->node);
        sal_free(node->name);
        sal_free(node);
    }
}

/* Like shash_clear(), but also free() each node's 'data'. */
void
ctc_shash_clear_free_data(ctc_shash_t *sh)
{
    ctc_shash_node_t *node = NULL;
    ctc_shash_node_t *next = NULL;

    SHASH_FOR_EACH_SAFE (node, next, sh)
    {
        ctc_hmap_remove(&sh->map, &node->node);
        sal_free(node->data);
        sal_free(node->name);
        sal_free(node);
    }
}

bool
ctc_shash_is_empty(const ctc_shash_t *shash)
{
    return ctc_hmap_is_empty(&shash->map);
}

uint32
ctc_shash_count(const ctc_shash_t *shash)
{
    return ctc_hmap_count(&shash->map);
}

STATIC ctc_shash_node_t *
ctc_shash_add_nocopy__(ctc_shash_t *sh, char *name, const void *data, uint32 hash)
{
    ctc_shash_node_t *node = sal_malloc(sizeof *node);
    if (NULL == node)
    {
        return NULL;
    }
    node->name = name;
    node->data = CONST_CAST(void *, data);
    hmap_insert(&sh->map, &node->node, hash);
    return node;
}

/* It is the caller's responsibility to avoid duplicate names, if that is
 * desirable. */
ctc_shash_node_t *
ctc_shash_add_nocopy(ctc_shash_t *sh, char *name, const void *data)
{
    return ctc_shash_add_nocopy__(sh, name, data, ctc_shash_name(name));
}

/* It is the caller's responsibility to avoid duplicate names, if that is
 * desirable. */
ctc_shash_node_t *
ctc_shash_add(ctc_shash_t *sh, const char *name, const void *data)
{
    char* dest = NULL;
    dest = sal_malloc(sal_strlen(name)+1);
    if (NULL == dest)
    {
        return NULL;
    }
    sal_strcpy(dest, name);
    return ctc_shash_add_nocopy(sh, dest, data);
}

bool
ctc_shash_add_once(ctc_shash_t *sh, const char *name, const void *data)
{
    if (!ctc_shash_find(sh, name))
    {
        ctc_shash_add(sh, name, data);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/*
void
shash_add_assert(struct shash *sh, const char *name, const void *data)
{
    bool added OVS_UNUSED = shash_add_once(sh, name, data);
    assert(added);
}
*/
/* Searches for 'name' in 'sh'.  If it does not already exist, adds it along
 * with 'data' and returns NULL.  If it does already exist, replaces its data
 * by 'data' and returns the data that it formerly contained. */
void *
ctc_shash_replace(ctc_shash_t *sh, const char *name, const void *data)
{
    uint32 hash = ctc_shash_name(name);
    ctc_shash_node_t *node = NULL;
    char* dest = NULL;
    dest = sal_malloc(sal_strlen(name)+1);
    if (NULL == dest)
    {
        return NULL;
    }
    sal_strcpy(dest, name);
    node = ctc_shash_find__(sh, name, sal_strlen(name), hash);

    if (!node)
    {
        ctc_shash_add_nocopy__(sh, dest, data, hash);
        return NULL;
    }
    else
    {
        void *old_data = node->data;
        node->data = CONST_CAST(void *, data);
        sal_free(dest);
        return old_data;
    }
}

/* Deletes 'node' from 'sh' and frees the node's name.  The caller is still
 * responsible for freeing the node's data, if necessary. */
void
ctc_shash_delete(ctc_shash_t *sh, ctc_shash_node_t *node)
{
    sal_free(ctc_shash_steal(sh, node));
}

/* Deletes 'node' from 'sh'.  Neither the node's name nor its data is freed;
 * instead, ownership is transferred to the caller.  Returns the node's
 * name. */
char *
ctc_shash_steal(ctc_shash_t *sh, ctc_shash_node_t *node)
{
    char *name = node->name;

    ctc_hmap_remove(&sh->map, &node->node);
    sal_free(node);
    return name;
}

STATIC ctc_shash_node_t *
ctc_shash_find__(const ctc_shash_t *sh, const char *name, uint32 name_len, uint32 hash)
{
    ctc_shash_node_t *node = NULL;

    HMAP_FOR_EACH_WITH_HASH (node, node, hash, &sh->map)
    {
        if (!sal_strncmp(node->name, name, name_len) && !node->name[name_len])
        {
            return node;
        }
    }
    return NULL;
}

/* If there are duplicates, returns a random element. */
ctc_shash_node_t *
ctc_shash_find(const ctc_shash_t *sh, const char *name)
{
    return ctc_shash_find__(sh, name, sal_strlen(name), ctc_shash_name(name));
}

/* Finds and returns a shash_node within 'sh' that has the given 'name' that is
 * exactly 'len' bytes long.  Returns NULL if no node in 'sh' has that name. */
ctc_shash_node_t *
ctc_shash_find_len(const ctc_shash_t *sh, const char *name, uint32 len)
{
    return ctc_shash_find__(sh, name, len, ctc_shash_bytes(name, len, 0));
}

void *
ctc_shash_find_data(const ctc_shash_t *sh, const char *name)
{
    ctc_shash_node_t *node = ctc_shash_find(sh, name);
    return node ? node->data : NULL;
}

void *
ctc_shash_find_and_delete(ctc_shash_t *sh, const char *name)
{
    ctc_shash_node_t *node = ctc_shash_find(sh, name);
    if (node)
    {
        void *data = node->data;
        ctc_shash_delete(sh, node);
        return data;
    }
    else
    {
        return NULL;
    }
}
/*
void *
shash_find_and_delete_assert(struct shash *sh, const char *name)
{
    void *data = shash_find_and_delete(sh, name);
    assert(data != NULL);
    return data;
}
*/
ctc_shash_node_t *
ctc_shash_first(const ctc_shash_t *shash)
{
    ctc_hmap_node_t *node = ctc_hmap_first(&shash->map);
    return node ? CONTAINER_OF(node, ctc_shash_node_t, node) : NULL;
}

STATIC int
ctc_compare_nodes_by_name(const void *a_, const void *b_)
{
    const ctc_shash_node_t *const *a = a_;
    const ctc_shash_node_t *const *b = b_;
    return sal_strcmp((*a)->name, (*b)->name);
}

const ctc_shash_node_t **
ctc_shash_sort(const ctc_shash_t *sh)
{
    if (ctc_shash_is_empty(sh))
    {
        return NULL;
    }
    else
    {
        const ctc_shash_node_t **nodes;
        ctc_shash_node_t *node = NULL;
        uint32 i, n;

        n = ctc_shash_count(sh);
        nodes = sal_malloc(n * sizeof *nodes);
        if(NULL == nodes)
        {
            return NULL;
        }
        i = 0;
        SHASH_FOR_EACH (node, sh)
        {
            nodes[i++] = node;
        }

        sal_qsort(nodes, n, sizeof *nodes, ctc_compare_nodes_by_name);

        return nodes;
    }
}

/* Returns true if 'a' and 'b' contain the same keys (regardless of their
 * values), false otherwise. */
bool
ctc_shash_equal_keys(const ctc_shash_t *a, const ctc_shash_t *b)
{
    ctc_shash_node_t *node = NULL;

    if (ctc_hmap_count(&a->map) != ctc_hmap_count(&b->map))
    {
        return FALSE;
    }

    SHASH_FOR_EACH (node, a)
    {
        if (!ctc_shash_find(b, node->name))
        {
            return FALSE;
        }
    }
    return TRUE;
}


/* Chooses and returns a randomly selected node from 'sh', which must not be
 * empty.
 *
 * I wouldn't depend on this algorithm to be fair, since I haven't analyzed it.
 * But it does at least ensure that any node in 'sh' can be chosen. */
 /*
struct shash_node *
shash_random_node(struct shash *sh)
{
    return CONTAINER_OF(hmap_random_node(&sh->map), struct shash_node, node);
}*/
