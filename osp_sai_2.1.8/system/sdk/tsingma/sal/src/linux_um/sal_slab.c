#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>

#include "sal.h"
#include "sal_slab.h"

#define M_SLAB_SHIFT 16
#define M_SLAB_SIZE  (1 << M_SLAB_SHIFT)
#define M_SLAB_MASK  (~(M_SLAB_SIZE - 1))

#define M_ALIGN(n) (((n) + 3) & ~3)

#define M_ASSERT(expr)

static mem_cache_t internal_cache;

STATIC void
sal_slab_list_pointer_delete(sal_slab_list_pointer_t* p_list, sal_slab_list_pointer_node_t* p_node)
{
    if (p_node->p_next != NULL)
    {
        p_node->p_next->p_prev = p_node->p_prev;
    }
    else
    {
        _SAL_SLAB_LTAIL(p_list) = p_node->p_prev;
    }

    p_node->p_prev->p_next = p_node->p_next;

    if (p_list->count != 0)
    {
        p_list->count--;
    }
}

STATIC int
sal_slab_list_pointer_empty(sal_slab_list_pointer_t* p_list)
{
    return _SAL_SLAB_LHEAD(p_list) == NULL;
}

STATIC sal_slab_list_pointer_node_t*
sal_slab_list_pointer_head(sal_slab_list_pointer_t* p_list)
{
    return _SAL_SLAB_LHEAD(p_list);
}

STATIC void
sal_slab_list_pointer_init(sal_slab_list_pointer_t* p_list)
{
    _SAL_SLAB_LHEAD(p_list) = NULL;
    _SAL_SLAB_LTAIL(p_list) = &p_list->head;
    p_list->count = 0;
}

STATIC void
sal_slab_list_pointer_insert_head(sal_slab_list_pointer_t* p_list, sal_slab_list_pointer_node_t* p_node)
{
    if ((p_node->p_next = _SAL_SLAB_LHEAD(p_list)) != NULL)
    {
        p_node->p_next->p_prev = p_node;
    }
    else
    {
        _SAL_SLAB_LTAIL(p_list) = p_node;
    }

    _SAL_SLAB_LHEAD(p_list) = p_node;
    p_node->p_prev = &p_list->head;

    p_list->count++;
}

STATIC void
sal_slab_list_pointer_insert_tail(sal_slab_list_pointer_t* p_list, sal_slab_list_pointer_node_t* p_node)
{
    p_node->p_next = NULL;
    p_node->p_prev = _SAL_SLAB_LTAIL(p_list);
    _SAL_SLAB_LTAIL(p_list)->p_next = p_node;
    _SAL_SLAB_LTAIL(p_list) = p_node;
    p_list->count++;
}

static INLINE void*
vm_alloc(int32 size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static INLINE void
vm_free(void* p, int32 size)
{
    munmap(p, size);
}

STATIC void
alloc_slab(mem_cache_t* cache)
{
    mem_slab_t* slab;
    void* vm;
    int32 vm_limit;

    vm = vm_alloc(cache->slab_size /*M_SLAB_SIZE*/);
    if (!vm || vm == MAP_FAILED)
    {
        return;
    }

    if (cache == &internal_cache)
    {
        vm_limit = cache->slab_size /*M_SLAB_SIZE*/ - M_ALIGN(sizeof(mem_slab_t));
        slab = (mem_slab_t*)((uint8*)vm + vm_limit);
    }
    else
    {
        vm_limit = cache->slab_size /*M_SLAB_SIZE*/;
        slab = mem_cache_alloc(&internal_cache);

        if (!slab)
        {
            vm_free(vm, cache->slab_size /*M_SLAB_SIZE*/);
            return;
        }
    }

    slab->cache = cache;
    slab->vm = vm;
    slab->left = vm;
    slab->used = 0;
    slab->limit = vm_limit / M_ALIGN(cache->obj_size + sizeof(mem_slab_t*));
    slab->free_list = NULL;

    sal_slab_list_pointer_insert_tail(&cache->free_slabs, &slab->list_node);

}

STATIC void
free_slab(mem_cache_t* cache, mem_slab_t* slab)
{
    vm_free(slab->vm, cache->slab_size /*M_SLAB_SIZE*/);
    if (cache != &internal_cache)
    {
        mem_cache_free(/*&internal_cache, */ slab);
    }
}

void
mem_cache_init()
{
    union __uu
    {
        mem_cache_t cache;
        mem_slab_t slab;
    };

    sal_slab_list_pointer_init(&internal_cache.full_slabs);
    sal_slab_list_pointer_init(&internal_cache.free_slabs);
    internal_cache.obj_size = sizeof(union __uu);
    internal_cache.slab_size = M_SLAB_SIZE;
}

mem_cache_t*
mem_cache_create(int32 obj_size)
{
    mem_cache_t* cache;

    cache = mem_cache_alloc(&internal_cache);
    if (cache)
    {
        sal_slab_list_pointer_init(&cache->full_slabs);
        sal_slab_list_pointer_init(&cache->free_slabs);
        cache->obj_size = obj_size;

        if (obj_size < 0x1000)
        {
            cache->slab_size = M_SLAB_SIZE;
        }
        else
        {
            cache->slab_size = obj_size + sizeof(mem_slab_t*);
        }
    }

    return cache;
}

void
mem_cache_destroy(mem_cache_t* cache)
{
    M_ASSERT(sal_slab_list_pointer_empty(&cache->full_slabs));
    M_ASSERT(sal_slab_list_pointer_empty(&cache->free_slabs));
    mem_cache_free(/*&internal_cache, */ cache);
}

void*
mem_cache_alloc(mem_cache_t* cache)
{
    mem_slab_t* slab;
    void* p;

    if (sal_slab_list_pointer_empty(&cache->free_slabs))
    {
        alloc_slab(cache);
    }

    if (sal_slab_list_pointer_empty(&cache->free_slabs))
    {
        return NULL;
    }

    slab = _sal_slab_container_of(sal_slab_list_pointer_head(&cache->free_slabs), mem_slab_t, list_node);
    if (slab->free_list)
    {
        p = slab->free_list;
        slab->free_list = *(void**)p;
    }
    else
    {
        p = slab->left;
        *(mem_slab_t**)p = slab;
        p = (uint8*)p + sizeof(mem_slab_t*);
        slab->left = (uint8*)slab->left + M_ALIGN(cache->obj_size + sizeof(mem_slab_t*));
    }

    slab->used++;
    if (slab->used == slab->limit)
    {
        sal_slab_list_pointer_delete(&cache->free_slabs, &slab->list_node);
        sal_slab_list_pointer_insert_tail(&cache->full_slabs, &slab->list_node);
    }

    return p;
}

void
mem_cache_free(/*mem_cache_t *cache, */ void* p)
{
    mem_slab_t* slab;
    mem_cache_t* cache;

    slab = vm_to_slab(p);
    M_ASSERT(slab);
    cache = slab->cache;

    *(void**)p = slab->free_list;
    slab->free_list = p;
    slab->used--;

    if (slab->used == slab->limit - 1)
    {
        sal_slab_list_pointer_delete(&cache->full_slabs, &slab->list_node);
        sal_slab_list_pointer_insert_head(&cache->free_slabs, &slab->list_node);
    }

    if (slab->used == 0)
    {
        sal_slab_list_pointer_delete(&cache->free_slabs, &slab->list_node);
        free_slab(cache, slab);
    }
}

