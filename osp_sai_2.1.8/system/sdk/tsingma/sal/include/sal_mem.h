#ifndef __SAL_MEM_H__
#define __SAL_MEM_H__

#ifndef _SAL_VXWORKS
/**
 * @file sal_mem.h
 */

/**
 * @defgroup mem Memory Allocation
 * @{
 */

/**
 * @brief Allocate dynamic memory
 *
 * Wrapper on sal_malloc(). Memory allocation failures will be traced.
 *
 * @param[out] p
 * @param[in]  type
 * @param[in]  size
 *
 * @see sal_malloc
 */
#define SAL_MALLOC(p, type, size)              \
    {                                                    \
        p = sal_malloc(size);                      \
    }

/**
 * @brief Allocate dynamic memory without blocking
 *
 * Wrapper on sal_malloc_atomic(). Memory allocation failures will be traced.
 *
 * @param[out] p
 * @param[in]  type
 * @param[in]  size
 *
 * @see sal_malloc_atomic
 */
#define SAL_MALLOC_ATOMIC(p, type, size)             \
    {                                                    \
        p = (type)sal_malloc(size); \
    }

/**
 * @brief Free dynamic memory
 *
 * Wrapper on sal_free(). After the memory pointed by @e p is freed, @e p is
 * assigned NULL
 *
 * @param[in] p
 *
 * @see sal_free
 */
#define SAL_FREE(p) \
    {                   \
        sal_free(p);    \
        p = NULL;       \
    }

#ifdef __cplusplus
extern "C"
{
#endif



/**
 * @brief Trace memory allocation failure
 *
 * @param[in] file
 * @param[in] line
 * @param[in] size
 */
void ctc_sal_malloc_failed(const char* file, int line, size_t size);

/**
 * Memory pool object
 */
typedef struct sal_mem_pool sal_mem_pool_t;

/**
 * Wrapper on sal_mem_pool_alloc(). Memory allocation failures will be traced.
 *
 * @param p
 * @param type
 * @param mem_pool
 * @param atomic
 */
#ifdef _SAL_DEBUG
#define SAL_MEM_POOL_ALLOC(p, type, mem_pool, atomic)                   \
    {                                                                       \
        p = (type)ctc_sal_mem_pool_alloc(mem_pool, atomic, __FILE__, __LINE__);  \
    }
#else
#define SAL_MEM_POOL_ALLOC(p, type, mem_pool, atomic)                   \
    {                                                                       \
        p = (type)ctc_sal_mem_pool_alloc(mem_pool, atomic);                      \
        if (!p){                                                             \
            ctc_sal_malloc_failed(__FILE__, __LINE__, size); }                    \
    }
#endif

/**
 * Wrapper on sal_mem_pool_free(). After the memory pointed by @e p is freed, @e p is
 * assigned NULL
 *
 * @param mem_pool
 * @param p
 */
#define SAL_MEM_POOL_FREE(mem_pool, p)  \
    {                                       \
        ctc_sal_mem_pool_free(mem_pool, p);     \
        p = NULL;                           \
    }

int ctc_sal_mem_pool_create(sal_mem_pool_t** mem_pool, const char* name,
                        size_t size, size_t align, uint32 min_nr);
void ctc_sal_mem_pool_destroy(sal_mem_pool_t* mem_pool);

#ifdef _SAL_DEBUG
void* ctc_sal_mem_pool_alloc(sal_mem_pool_t* mem_pool, bool atomic, const char* file,
                         int line);
#else
void* ctc_sal_mem_pool_alloc(sal_mem_pool_t* mem_pool, bool atomic);
#endif

void ctc_sal_mem_pool_free(sal_mem_pool_t* mem_pool, void* p);

#ifdef _SAL_LINUX_KM
void *ctc_sal_realloc(void *ptr, size_t size);
#endif

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup mem */
#endif
#endif /* !__SAL_MEM_H__ */

