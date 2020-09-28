#include "sal.h"
#include "ctclib_slab.h"
#include "ctclib_list.h"
//#include "ctclib_debug.h"

#define CTCLIB_MMAP_SIZE_ALIGN(_v) (((_v) + CTCLIB_M_PAGE_SIZE - 1) & ~(CTCLIB_M_PAGE_SIZE - 1))
#define CTCLIB_M_ALIGN(n) (((n) + 3) & ~3)
#define CTCLIB_M_ASSERT(expr)

static ctclib_mem_cache_t internal_cache;
static ctclib_mem_cache_t shm_cache;    /* share memory slab cache */
static ctclib_mem_cache_t shm_comm_cache;    /* share memory slab cache */


static inline void *ctclib_shm_vm_alloc(size_t size)
{
    int ctc_shm_fd;

    ctc_shm_fd = open("/dev/ctc_shm", O_RDWR);
    if(ctc_shm_fd < 0)
    {
        return NULL;
    }

    return mmap(NULL, CTCLIB_MMAP_SIZE_ALIGN(size), PROT_READ|PROT_WRITE, MAP_SHARED, ctc_shm_fd, 0);
}

#ifdef HAVE_SHM
static inline void *ctclib_shm_comm_vm_alloc(size_t size)
{
    return ctclib_shm_get_and_attach(GLB_SHM_COMM_FNAME, size);
}
#endif /*HAVE_SHM*/

static inline void *ctclib_vm_alloc(size_t size)
{
    return mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

static inline void ctclib_vm_free(void *p, size_t size)
{
    munmap(p, size);
}

static void ctclib_alloc_slab(ctclib_mem_cache_t *cache)
{
    ctclib_mem_slab_t *slab = NULL;
    void *vm = NULL;
    size_t vm_limit;

#if 0
    if(cache->type != CTC_SLAB_CACHE_SHM)
        vm = ctclib_vm_alloc(cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);
    else
        vm = ctclib_shm_vm_alloc(cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);
#endif

    if(CTC_SLAB_CACHE_INTERNAL == cache->type)
        vm = ctclib_vm_alloc(cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);

#ifdef _USER_KERNEL_SHM_
    else if(CTC_SLAB_CACHE_SHM == cache->type)
        vm = ctclib_shm_vm_alloc(cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);
#endif /*_USER_KERNEL_SHM_*/

#ifdef HAVE_SHM
    else if(CTC_SLAB_CACHE_SHM_COMM == cache->type)
    {
        vm = ctclib_shm_comm_vm_alloc(cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);
    }
#endif /*HAVE_SHM*/

    /*modified by xgu for bug 9329, 2011-03-04, merged from v2.2 branch*/
    if (!vm || vm == MAP_FAILED)
        return;

    if (cache == &internal_cache || cache == &shm_cache || cache == &shm_comm_cache) {
        vm_limit = cache->slab_size/*CTCLIB_M_SLAB_SIZE*/ - CTCLIB_M_ALIGN(sizeof(ctclib_mem_slab_t));
        slab = (ctclib_mem_slab_t *)(vm + vm_limit);
    }
    else {
        vm_limit = cache->slab_size/*CTCLIB_M_SLAB_SIZE*/;
#if 0
        if(cache->type != CTC_SLAB_CACHE_SHM)
            slab = ctclib_mem_cache_alloc(&internal_cache);
        else
            slab = ctclib_mem_cache_alloc(&shm_cache);
#else
        if(cache->type == CTC_SLAB_CACHE_INTERNAL)
            slab = ctclib_mem_cache_alloc(&internal_cache);
        else if(cache->type == CTC_SLAB_CACHE_SHM)
            slab = ctclib_mem_cache_alloc(&shm_cache);
#ifdef HAVE_SHM
        else if(cache->type == CTC_SLAB_CACHE_SHM_COMM)
            slab = ctclib_mem_cache_alloc(&shm_comm_cache);
#endif /*HAVE_SHM*/
#endif
        if (!slab) {
            ctclib_vm_free(vm, cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);
            return;
        }
    }

    slab->cache = cache;
    slab->vm = vm;
    slab->left = vm;
    slab->used = 0;
    slab->limit = vm_limit / CTCLIB_M_ALIGN(cache->obj_size + sizeof(ctclib_mem_slab_t *));
    slab->free_list = NULL;

    ctclib_list_insert_tail(&cache->free_slabs, &slab->list_node);
#if 0
    hash_slab(slab);
#endif
}

static void ctclib_free_slab(ctclib_mem_cache_t *cache, ctclib_mem_slab_t *slab)
{
#if 0
    unhash_slab(slab);
#endif
    ctclib_vm_free(slab->vm, cache->slab_size/*CTCLIB_M_SLAB_SIZE*/);
    if (cache != &internal_cache && cache != &shm_cache && cache != &shm_comm_cache)
        ctclib_mem_cache_free(/*&internal_cache, */slab);
}

void ctclib_mem_cache_init()
{
    union __uu {
        ctclib_mem_cache_t cache;
        ctclib_mem_slab_t slab;
    };

#if 0
    int i;

    for (i = 0; i < M_HASH_SIZE; i++)
        _list_init(&slab_htab[i]);
#endif

    ctclib_list_init(&internal_cache.full_slabs);
    ctclib_list_init(&internal_cache.free_slabs);
    internal_cache.obj_size = sizeof(union __uu);
    internal_cache.slab_size = CTCLIB_M_SLAB_SIZE;

    ctclib_list_init(&shm_cache.full_slabs);
    ctclib_list_init(&shm_cache.free_slabs);
    shm_cache.obj_size = sizeof(union __uu);
    shm_cache.slab_size = CTCLIB_M_SLAB_SIZE;

    ctclib_list_init(&shm_comm_cache.full_slabs);
    ctclib_list_init(&shm_comm_cache.free_slabs);
    shm_comm_cache.obj_size = sizeof(union __uu);
    shm_comm_cache.slab_size = CTCLIB_M_SLAB_SIZE;

}

ctclib_mem_cache_t *ctclib_mem_cache_create(size_t obj_size, slab_cache_type_e type)
{
    ctclib_mem_cache_t *cache;

    if(CTC_SLAB_CACHE_INTERNAL == type)
        cache = ctclib_mem_cache_alloc(&internal_cache);
    else if(CTC_SLAB_CACHE_SHM == type)
        cache = ctclib_mem_cache_alloc(&shm_cache);
#ifdef HAVE_SHM
    else if(CTC_SLAB_CACHE_SHM_COMM == type)
        cache = ctclib_mem_cache_alloc(&shm_comm_cache);
#endif /*HAVE_SHM*/
    else
        return NULL;

    if (cache) {
        ctclib_list_init(&cache->full_slabs);
        ctclib_list_init(&cache->free_slabs);
        cache->obj_size = obj_size;
        cache->type = type;
        if (obj_size <= 0x100)
            cache->slab_size = CTCLIB_M_SLAB_SIZE/8;
        else if (obj_size < (0x1000 - sizeof(ctclib_mem_slab_t *)))
            cache->slab_size = CTCLIB_M_SLAB_SIZE;
        else
            cache->slab_size = obj_size + sizeof(ctclib_mem_slab_t *);
    }

    return cache;
}

void ctclib_mem_cache_destroy(ctclib_mem_cache_t *cache)
{
    CTCLIB_M_ASSERT(ctclib_list_empty(&cache->full_slabs));
    CTCLIB_M_ASSERT(ctclib_list_empty(&cache->free_slabs));
    ctclib_mem_cache_free(/*&internal_cache, */cache);
}

void *ctclib_mem_cache_alloc(ctclib_mem_cache_t *cache)
{
    ctclib_mem_slab_t *slab;
    void *p;

    if (ctclib_list_empty(&cache->free_slabs))
        ctclib_alloc_slab(cache);

    if (ctclib_list_empty(&cache->free_slabs))
        return NULL;

    slab = ctclib_container_of(ctclib_list_head(&cache->free_slabs), ctclib_mem_slab_t, list_node);
    if (slab->free_list) {
        p = slab->free_list;
        slab->free_list = *(void **)p;
        /*modified by weij for bug 18356, 2012-05-15: check duplicate free memory*/
        *(ctclib_mem_slab_t **)((long)p - sizeof(ctclib_mem_slab_t *)) = slab;
    }
    else {
        p = slab->left;
        *(ctclib_mem_slab_t **)p = slab;
        p += sizeof(ctclib_mem_slab_t *);
        slab->left += CTCLIB_M_ALIGN(cache->obj_size + sizeof(ctclib_mem_slab_t *));
    }

    slab->used++;
    if (slab->used == slab->limit) {
        ctclib_list_delete(&cache->free_slabs, &slab->list_node);
        ctclib_list_insert_tail(&cache->full_slabs, &slab->list_node);
    }

    return p;
}

void ctclib_mem_cache_free(/*mem_cache_t *cache, */void *p)
{
    ctclib_mem_slab_t *slab;
    ctclib_mem_cache_t *cache;
#if 0
    _list_node_t *next;
#endif

    slab = ctclib_vm_to_slab(p);
    CTCLIB_M_ASSERT(slab);
    cache = slab->cache;

    *(void **)p = slab->free_list;
    slab->free_list = p;
    slab->used--;
    /*modified by weij for bug 18356, 2012-05-15: check duplicate free memory*/
    *(ctclib_mem_slab_t **)((long)p - sizeof(ctclib_mem_slab_t *)) = NULL;

    if (slab->used == slab->limit - 1) {
        ctclib_list_delete(&cache->full_slabs, &slab->list_node);
        ctclib_list_insert_head(&cache->free_slabs, &slab->list_node);
    }
#ifdef HAVE_SHM
    if ((slab->used == 0) && (cache->type == CTC_SLAB_CACHE_INTERNAL))
#else
    if ((slab->used == 0) && (cache->type != CTC_SLAB_CACHE_SHM))
#endif /*HAVE_SHM*/
    {
        ctclib_list_delete(&cache->free_slabs, &slab->list_node);
        ctclib_free_slab(cache, slab);
    }

#if 0
    else {
       next =  _list_next(&slab->list_node);
       if (next) {
            _list_delete(&cache->free_slabs, &slab->list_node);
           _list_insert_after(&cache->free_slabs, next, &slab->list_node);
       }
    }
#endif
}

#if 0
size_t ctclib_mem_cache_get_obj_size(mem_cache_t *cache)
{
    CTCLIB_M_ASSERT(cache);
    return cache->obj_size;
}

size_t ctclib_mem_cache_get_obj_size_by_ptr(void *p)
{
    mem_slab_t *slab;

    slab = vm_to_slab(p);
    CTCLIB_M_ASSERT(slab);

    return slab->cache->obj_size;
}
#endif

