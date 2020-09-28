#include "sal.h"
#include "ctc_hmap.h"

/* Initializes 'hmap' as an empty hash table. */
void
ctc_hmap_init(ctc_hmap_t *hmap)
{
    hmap->buckets = &hmap->one;
    hmap->one = NULL;
    hmap->mask = 0;
    hmap->n = 0;
}

/* Frees memory reserved by 'hmap'.  It is the client's responsibility to free
 * the nodes themselves, if necessary. */
void
ctc_hmap_destroy(ctc_hmap_t *hmap)
{
    if (hmap && hmap->buckets != &hmap->one)
    {
        sal_free(hmap->buckets);
    }
}

/* Removes all node from 'hmap', leaving it ready to accept more nodes.  Does
 * not free memory allocated for 'hmap'.
 *
 * This function is appropriate when 'hmap' will soon have about as many
 * elements as it did before.  If 'hmap' will likely have fewer elements than
 * before, use hmap_destroy() followed by hmap_init() to save memory and
 * iteration time. */
void
ctc_hmap_clear(ctc_hmap_t *hmap)
{
    if (hmap->n > 0)
    {
        hmap->n = 0;
        sal_memset(hmap->buckets, 0, (hmap->mask + 1) * sizeof *hmap->buckets);
    }
}

/* Exchanges hash maps 'a' and 'b'. */
void
ctc_hmap_swap(ctc_hmap_t *a, ctc_hmap_t *b)
{
    ctc_hmap_t tmp = *a;
    *a = *b;
    *b = tmp;
    ctc_hmap_moved(a);
    ctc_hmap_moved(b);
}

/* Adjusts 'hmap' to compensate for having moved position in memory (e.g. due
 * to realloc()). */
void
ctc_hmap_moved(ctc_hmap_t *hmap)
{
    if (!hmap->mask)
    {
        hmap->buckets = &hmap->one;
    }
}

STATIC void
ctc_resize(ctc_hmap_t *hmap, uint32 new_mask)
{
    ctc_hmap_t tmp;
    uint32 i;

    ctc_hmap_init(&tmp);
    if (new_mask)
    {
        tmp.buckets = sal_malloc(sizeof *tmp.buckets * (new_mask + 1));
        if(NULL == tmp.buckets)
        {
            return ;
        }
        tmp.mask = new_mask;
        for (i = 0; i <= tmp.mask; i++)
        {
            tmp.buckets[i] = NULL;
        }
    }
    for (i = 0; i <= hmap->mask; i++)
    {
        ctc_hmap_node_t *node, *next;
        int count = 0;
        for (node = hmap->buckets[i]; node; node = next)
        {
            next = node->next;
            ctc_hmap_insert_fast(&tmp, node, node->hash);
            count++;
        }
    }
    ctc_hmap_swap(hmap, &tmp);
    ctc_hmap_destroy(&tmp);
}

STATIC uint32
ctc_calc_mask(uint32 capacity)
{
    uint32 mask = capacity / 2;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;
/*
#if SIZE_MAX > UINT32_MAX
    mask |= mask >> 32;
#endif
*/
    /* If we need to dynamically allocate buckets we might as well allocate at
     * least 4 of them. */
    mask |= (mask & 1) << 1;

    return mask;
}

/*
* Expands 'hmap', if necessary, to optimize the performance of searches.
*/
void
ctc_hmap_expand_at(ctc_hmap_t *hmap)
{
    uint32 new_mask = ctc_calc_mask(hmap->n);
    if (new_mask > hmap->mask)
    {
        ctc_resize(hmap, new_mask);
    }
}

/* Shrinks 'hmap', if necessary, to optimize the performance of iteration.
 *
 */
void
ctc_hmap_shrink_at(ctc_hmap_t *hmap)
{
    uint32 new_mask = ctc_calc_mask(hmap->n);
    if (new_mask < hmap->mask)
    {
        ctc_resize(hmap, new_mask);
    }
}

/* Expands 'hmap', if necessary, to optimize the performance of searches when
 * it has up to 'n' elements.  (But iteration will be slow in a hash map whose
 * allocated capacity is much higher than its current number of nodes.)
*/
void
ctc_hmap_reserve_at(ctc_hmap_t *hmap, uint32 n)
{
    uint32 new_mask = ctc_calc_mask(n);
    if (new_mask > hmap->mask)
    {
        ctc_resize(hmap, new_mask);
    }
}

/* Adjusts 'hmap' to compensate for 'old_node' having moved position in memory
 * to 'node' (e.g. due to realloc()). */
void
ctc_hmap_node_moved(ctc_hmap_t *hmap, ctc_hmap_node_t *old_node, ctc_hmap_node_t *node)
{
    ctc_hmap_node_t **bucket = &hmap->buckets[node->hash & hmap->mask];
    while (*bucket != old_node)
    {
        bucket = &(*bucket)->next;
    }
    *bucket = node;
}

/* Returns the next node in 'hmap' in hash order, or NULL if no nodes remain in
 * 'hmap'.  Uses '*bucketp' and '*offsetp' to determine where to begin
 * iteration, and stores new values to pass on the next iteration into them
 * before returning.
 *
 * It's better to use plain HMAP_FOR_EACH and related functions, since they are
 * faster and better at dealing with hmaps that change during iteration.
 *
 * Before beginning iteration, store 0 into '*bucketp' and '*offsetp'.
 */
ctc_hmap_node_t *
ctc_hmap_at_position(ctc_hmap_t *hmap, uint32 *bucketp, uint32 *offsetp)
{
    uint32 offset;
    uint32 b_idx;

    offset = *offsetp;
    for (b_idx = *bucketp; b_idx <= hmap->mask; b_idx++)
    {
        ctc_hmap_node_t *node;
        uint32 n_idx;

        for (n_idx = 0, node = hmap->buckets[b_idx]; node != NULL;
             n_idx++, node = node->next)
        {
            if (n_idx == offset)
            {
                if (node->next)
                {
                    *bucketp = node->hash & hmap->mask;
                    *offsetp = offset + 1;
                }
                else
                {
                    *bucketp = (node->hash & hmap->mask) + 1;
                    *offsetp = 0;
                }
                return node;
            }
        }
        offset = 0;
    }

    *bucketp = 0;
    *offsetp = 0;
    return NULL;
}

/* Returns true if 'node' is in 'hmap', false otherwise. */
bool
ctc_hmap_contains(const ctc_hmap_t *hmap, const ctc_hmap_node_t *node)
{
    ctc_hmap_node_t *p;

    for (p = ctc_hmap_first_in_bucket(hmap, node->hash); p; p = p->next)
    {
        if (p == node)
        {
            return TRUE;
        }
    }

    return FALSE;
}
