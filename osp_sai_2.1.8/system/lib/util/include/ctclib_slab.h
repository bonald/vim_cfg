#ifndef __SLAB_H__
#define __SLAB_H__
#include "ctclib_list.h"
#include <assert.h>

#define CTCLIB_M_SLAB_SHIFT 16
#define CTCLIB_M_SLAB_SIZE  (1 << CTCLIB_M_SLAB_SHIFT)
#define CTCLIB_M_SLAB_MASK  (~(CTCLIB_M_SLAB_SIZE - 1))
#define CTCLIB_M_PAGE_SIZE  0x1000

typedef struct ctclib_mem_cache_s ctclib_mem_cache_t;

typedef enum
{
    CTC_SLAB_CACHE_INTERNAL,
    CTC_SLAB_CACHE_SHM,
#ifdef HAVE_SHM
    CTC_SLAB_CACHE_SHM_COMM, /*CTC_SLAB_CACHE_SHM is reserved for peth.*/
#endif /*HAVE_SHM*/
    CTC_SLAB_CACHE_MAX,
}slab_cache_type_e;

void ctclib_mem_cache_init();

ctclib_mem_cache_t *ctclib_mem_cache_create(size_t obj_size, slab_cache_type_e type);

void ctclib_mem_cache_destroy(ctclib_mem_cache_t *cache);

void *ctclib_mem_cache_alloc(ctclib_mem_cache_t *cache);

void ctclib_mem_cache_free(/*mem_cache_t *cache, */void *p);
#if 0
size_t ctclib_mem_cache_get_obj_size(mem_cache_t *cache);
size_t ctclib_mem_cache_get_obj_size_by_ptr(void *p);
#endif

struct ctclib_mem_slab_s
{
#if 0
    _list_node_t hash_node;
#endif
    ctclib_list_node_t list_node;
    ctclib_mem_cache_t *cache;
    void *vm;
    void *left;
    size_t used;
    size_t limit;
    void *free_list;
} ;
typedef struct ctclib_mem_slab_s ctclib_mem_slab_t;

struct ctclib_mem_cache_s
{
    ctclib_list_t full_slabs;
    ctclib_list_t free_slabs;
    size_t obj_size;
    size_t slab_size;
    slab_cache_type_e type;
};

static inline ctclib_mem_slab_t *ctclib_vm_to_slab(void *p)
{
    return *(ctclib_mem_slab_t **)((long)p - sizeof(ctclib_mem_slab_t *));
}
static inline size_t ctclib_mem_cache_get_obj_size(ctclib_mem_cache_t *cache)
{
    //M_ASSERT(cache);
    return cache->obj_size;
}

static inline size_t ctclib_mem_cache_get_obj_size_by_ptr(void *p)
{
    ctclib_mem_slab_t *slab;

    slab = ctclib_vm_to_slab(p);
    //M_ASSERT(slab);
    /*modified by weij for bug 18356, 2012-05-15: check duplicate free memory*/
    assert(slab);

    return slab->cache->obj_size;
}


#endif /* !__SLAB_H__ */
