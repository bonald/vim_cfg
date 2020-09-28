#include "sal.h"
struct sal_mem_pool
{
    uint32 num;
    size_t size;
};

void*
ctc_sal_malloc_atomic(size_t size)
{
    return malloc(size);
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
    SAL_MALLOC(*mem_pool, sal_mem_pool_t*, sizeof(sal_mem_pool_t));
    if (*mem_pool == NULL)
    {
        return ENOMEM;
    }

    (*mem_pool)->num = 0;
    (*mem_pool)->size = size;
    return 0;
}

void
ctc_sal_mem_pool_destroy(sal_mem_pool_t* mem_pool)
{
    if (mem_pool->num != 0)
    {
        SAL_LOG_INFO("fail to destroy this memory pool!");
    }
    else
    {
        SAL_FREE(mem_pool);
    }
}

#ifdef _SAL_DEBUG
void*
ctc_sal_mem_pool_alloc(sal_mem_pool_t* mem_pool, bool atomic,
                   const char* file, int line)
{
    void* temp;

    temp = malloc(mem_pool->size);
    if (temp)
    {
        mem_pool->num++;
    }
    else
    {
        ctc_sal_log(SAL_LL_ERROR, file, line, "malloc(%d) failed!", mem_pool->size);
    }

    return temp;
}

#else
void*
ctc_sal_mem_pool_alloc(sal_mem_pool_t* mem_pool, bool atomic)
{
    void* temp;

    temp = malloc(mem_pool->size);
    if (temp)
    {
        mem_pool->num++;
    }

    return temp;
}

#endif

void
ctc_sal_mem_pool_free(sal_mem_pool_t* mem_pool, void* p)
{
    SAL_FREE(p);
    mem_pool->num--;
}

