#ifndef _CTC_SHASH_H
#define _CTC_SHASH_H

#ifdef  __cplusplus
extern "C" {
#endif
#include "common/include/ctc_hmap.h"

struct ctc_shash_node_s {
    ctc_hmap_node_t node;
    char *name;
    void *data;
};
typedef struct ctc_shash_node_s ctc_shash_node_t;

struct ctc_shash_s {
    ctc_hmap_t map;
};
typedef struct ctc_shash_s ctc_shash_t;

#define CTC_SHASH_ROT(x, k) (x << k) | (x >> (32 - k))
#define SHASH_INITIALIZER(SHASH) { HMAP_INITIALIZER(&(SHASH)->map) }

#define SHASH_FOR_EACH(SHASH_NODE, SHASH) \
    HMAP_FOR_EACH (SHASH_NODE, node, &(SHASH)->map)

#define SHASH_FOR_EACH_SAFE(SHASH_NODE, NEXT, SHASH) \
    HMAP_FOR_EACH_SAFE (SHASH_NODE, NEXT, node, &(SHASH)->map)

void ctc_shash_init(ctc_shash_t *);
void ctc_shash_destroy(ctc_shash_t *);
void ctc_shash_destroy_free_data(ctc_shash_t *);
void ctc_shash_swap(ctc_shash_t *, ctc_shash_t *);
void ctc_shash_moved(ctc_shash_t *);
void ctc_shash_clear(ctc_shash_t *);
void ctc_shash_clear_free_data(ctc_shash_t *);
extern bool ctc_shash_is_empty(const ctc_shash_t *);
uint32 ctc_shash_count(const ctc_shash_t *);
ctc_shash_node_t *ctc_shash_add(ctc_shash_t *, const char *, const void *);
ctc_shash_node_t *ctc_shash_add_nocopy(ctc_shash_t *, char *, const void *);
extern bool ctc_shash_add_once(ctc_shash_t *, const char *, const void *);
void ctc_shash_add_assert(ctc_shash_t *, const char *, const void *);
void *ctc_shash_replace(ctc_shash_t *, const char *, const void *data);
void ctc_shash_delete(ctc_shash_t *, ctc_shash_node_t *);
char *ctc_shash_steal(ctc_shash_t *, ctc_shash_node_t *);
ctc_shash_node_t *ctc_shash_find(const ctc_shash_t *, const char *);
ctc_shash_node_t *ctc_shash_find_len(const ctc_shash_t *, const char *, uint32);
void *ctc_shash_find_data(const ctc_shash_t *, const char *);
void *ctc_shash_find_and_delete(ctc_shash_t *, const char *);
void *ctc_shash_find_and_delete_assert(ctc_shash_t *, const char *);
ctc_shash_node_t *ctc_shash_first(const ctc_shash_t *);
const ctc_shash_node_t **ctc_shash_sort(const ctc_shash_t *);
extern bool ctc_shash_equal_keys(const ctc_shash_t *, const ctc_shash_t *);


#ifdef  __cplusplus
}
#endif

#endif /* ctc_shash.h */
