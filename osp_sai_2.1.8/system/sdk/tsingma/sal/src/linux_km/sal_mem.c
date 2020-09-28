#include "sal.h"
#include "linux/slab.h"
#include "linux/mempool.h"
#include "linux/types.h"
#include "linux/gfp.h"

struct sal_mem_pool
{
    mempool_t* mempool;
    struct kmem_cache* cache;
};

void *ctc_sal_realloc(void *ptr, size_t size)
{
    void *new_ptr = NULL;
    if (ptr)
    {
        if (size != 0)
        {
            if (!(new_ptr = kmalloc(size, GFP_KERNEL)))
            {
                return NULL;
            }
            memmove(new_ptr, ptr, size);
        }

        kfree(ptr);
    }
    else
    {
        if (size != 0)
        {
            if (!(new_ptr = kmalloc(size, GFP_KERNEL)))
            {
                return NULL;
            }
        }
    }

    return new_ptr;
}

void *ctc_sal_malloc(size_t size)
{
    return kmalloc(size, GFP_KERNEL);
}

void *ctc_sal_calloc(size_t size)
{
    void *ptr = kmalloc(size, GFP_KERNEL);
    if (ptr)
    {
        memset(ptr, 0, sizeof(size));
    }
    return ptr;
}

void*
ctc_sal_malloc_atomic(size_t size)
{
    return kmalloc(size, GFP_ATOMIC);
}

void ctc_sal_free(void *p)
{
    kfree(p);
}

void
ctc_sal_malloc_failed(const char* file, int line, size_t size)
{
    ctc_sal_log(SAL_LL_ERROR, file, line, "malloc(%d) failed!", size);
}

int
ctc_sal_mem_pool_create(sal_mem_pool_t** mem_pool, const char* name,
                    size_t size, size_t align, uint32 min_nr)
{
    mempool_t* pool;

    SAL_MALLOC(*mem_pool, sal_mem_pool_t*, sizeof(sal_mem_pool_t));
    if (*mem_pool == NULL)
    {
        return ENOMEM;
    }

    (*mem_pool)->cache = kmem_cache_create(name, size, align, 0, NULL);
    if (!((*mem_pool)->cache))
    {
        SAL_FREE(*mem_pool);
        return ENOMEM;
    }

    pool = mempool_create(min_nr, mempool_alloc_slab, mempool_free_slab,
                          (*mem_pool)->cache);
    if (!pool)
    {
        kmem_cache_destroy((*mem_pool)->cache);
        SAL_FREE(*mem_pool);
        return ENOMEM;
    }

    (*mem_pool)->mempool = pool;

    return 0;
}

void
ctc_sal_mem_pool_destroy(sal_mem_pool_t* mem_pool)
{
    mempool_destroy(mem_pool->mempool);
    kmem_cache_destroy(mem_pool->cache);
    SAL_FREE(mem_pool);
}

#ifdef _SAL_DEBUG
void*
ctc_sal_mem_pool_alloc(sal_mem_pool_t* mem_pool, bool atomic,
                   const char* file, int line)
{
    gfp_t gfp_mask;
    void* ret;

    if (atomic)
    {
        gfp_mask = GFP_ATOMIC;
    }
    else
    {
        gfp_mask = GFP_KERNEL;
    }

    ret = mempool_alloc(mem_pool->mempool, gfp_mask);
    if (!ret)
    {
        ctc_sal_log(SAL_LL_ERROR, file, line, "malloc failed!");
    }

    return(ret);
}

#else
void*
ctc_sal_mem_pool_alloc(sal_mem_pool_t* mem_pool, bool atomic)
{
    gfp_t gfp_mask;

    if (atomic)
    {
        gfp_mask = GFP_ATOMIC;
    }
    else
    {
        gfp_mask = GFP_KERNEL;
    }

    return(mempool_alloc(mem_pool->mempool, gfp_mask));
}

#endif

void
ctc_sal_mem_pool_free(sal_mem_pool_t* mem_pool, void* p)
{
    mempool_free(p, mem_pool->mempool);
}

EXPORT_SYMBOL(sal_malloc);
EXPORT_SYMBOL(sal_free);
EXPORT_SYMBOL(ctc_sal_malloc_failed);

