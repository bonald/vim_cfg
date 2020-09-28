#include "sal.h"
#include "ctclib_hash.h"
#include "ctclib_memory.h"

typedef enum hash_mem_type_s
{
    CTCLIB_HASH_SYS_MEM,    /* get memory from system */
    CTCLIB_HASH_SHARE_MEM,  /* get memory from share memory area with kernel */
    CTCLIB_HASH_MEM_LAST
}hash_mem_type_t;

/* Allocate a new hash.  */
static ctclib_hash_t *
_ctclib_hash_create_size (uint32 size, hash_mem_type_t type,
      uint32 (*hash_key) (), bool (*hash_cmp) ())
{
    ctclib_hash_t *hash;

    hash = XMALLOC(MEM_LIB_HASH, sizeof(ctclib_hash_t));
    if (NULL == hash)
    {
        return NULL;
    }

    hash->index = XCALLOC(MEM_LIB_HASH_BACKET_LIST, sizeof(ctclib_hash_backet_t*) * size);
    if (NULL == hash->index)
    {
        XFREE(MEM_LIB_HASH, hash);
        return NULL;
    }

    hash->size = size;
    hash->hash_key = hash_key;
    hash->hash_cmp = hash_cmp;
    hash->count = 0;

    return hash;
}

ctclib_hash_t *
ctclib_hash_create_size (uint32 size, 
      uint32 (*hash_key) (), bool (*hash_cmp) ())
{
    return _ctclib_hash_create_size(size, CTCLIB_HASH_SYS_MEM, hash_key, hash_cmp);
}

/* Allocate a new hash with default hash size.  */
ctclib_hash_t *
ctclib_hash_create (uint32 (*hash_key) (), bool (*hash_cmp) ())
{
    return ctclib_hash_create_size (CTCLIB_HASHTABSIZE, hash_key, hash_cmp);
}

/* Utility function for hash_get().  When this function is specified
   as alloc_func, return arugment as it is.  This function is used for
   intern already allocated value.  */
void *
ctclib_hash_alloc_intern (void *arg)
{
    return arg;
}

/* Lookup and return hash backet in hash.  If there is no
   corresponding hash backet and alloc_func is specified, create new
   hash backet.  */
static void *
_ctclib_hash_get (hash_mem_type_t type, ctclib_hash_t *hash, void *data, void * (*alloc_func) ())
{
    uint32 key;
    uint32 index;
    void *newdata;
    ctclib_hash_backet_t *backet;

    if (!hash)
        return NULL;

    key = (*hash->hash_key) (data);
    index = key % hash->size;

    for (backet = hash->index[index]; backet != NULL; backet = backet->next) 
    {
        if (backet->key == key && (*hash->hash_cmp) (backet->data, data) == TRUE)
        {
            return backet->data;
        }
    }
    if (alloc_func)
    {
        newdata = (*alloc_func) (data);
        if (newdata == NULL)
            return NULL;

        backet = XMALLOC(MEM_LIB_HASH_BACKET, sizeof(ctclib_hash_backet_t));
        if (!backet)
            return NULL;

        backet->data = newdata;
        backet->key = key;
        backet->next = hash->index[index];
        hash->index[index] = backet;
        hash->count++;
        return backet->data;
    }
    return NULL;
}

void *
ctclib_hash_get (ctclib_hash_t *hash, void *data, void * (*alloc_func) ())
{
    return _ctclib_hash_get(CTCLIB_HASH_SYS_MEM, hash, data, alloc_func);
}

/* Lookup and return hash backet in hash.  If there is no
   corresponding hash backet and alloc_func is specified, create new
   hash backet.  */
void *
ctclib_hash_get2(ctclib_hash_t *hash, void *data,  void *arg, void * (*alloc_func) ())
{
    uint32 key;
    uint32 index;
    void *newdata;
    ctclib_hash_backet_t *backet;

    if (!hash)
        return NULL;

    key = (*hash->hash_key) (data);
    index = key % hash->size;

    for (backet = hash->index[index]; backet != NULL; backet = backet->next) 
    {
        if (backet->key == key && (*hash->hash_cmp) (backet->data, data) == TRUE)
        {
            return backet->data;
        }
    }

    if (alloc_func)
    {
        newdata = (*alloc_func) (data, arg);
        if (newdata == NULL)
            return NULL;

        backet = XMALLOC(MEM_LIB_HASH_BACKET, sizeof(ctclib_hash_backet_t));
        if (!backet)
            return NULL;

        backet->data = newdata;
        backet->key = key;
        backet->next = hash->index[index];
        hash->index[index] = backet;
        hash->count++;
        return backet->data;
    }
    return NULL;
}

/* Hash lookup.  */
void *
ctclib_hash_lookup (ctclib_hash_t *hash, void *data)
{
    return ctclib_hash_get (hash, data, NULL);
}

/* This function release registered value from specified hash.  When
   release is successfully finished, return the data pointer in the
   hash backet.  */
void *
ctclib_hash_release (ctclib_hash_t *hash, void *data)
{
    void *ret;
    uint32 key;
    uint32 index;
    ctclib_hash_backet_t *backet;
    ctclib_hash_backet_t *pp;

    key = (*hash->hash_key) (data);
    index = key % hash->size;

    for (backet = pp = hash->index[index]; backet; backet = backet->next)
    {
        if (backet->key == key && (*hash->hash_cmp) (backet->data, data) == TRUE)
        {
            if (backet == pp) 
            {
                hash->index[index] = backet->next;
            }
            else
            {
                pp->next = backet->next;
            }
            
            ret = backet->data;
            XFREE(MEM_LIB_HASH_BACKET, backet);
            hash->count--;
            return ret;
        }
        pp = backet;
    }
    return NULL;
}

/* Iterator function for hash.  */
void
ctclib_hash_iterate (ctclib_hash_t *hash,
              void (*func) (ctclib_hash_backet_t *, void *), void *arg)
{
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next;
    int i;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            (*func) (hb, arg);
        }
    }
    return;
}

/* Iterator function for hash with 2 args  */
void
ctclib_hash_iterate2 (ctclib_hash_t *hash,
               int32 (*func) (ctclib_hash_backet_t *, void *, void *),
               void *arg1, void *arg2)
{
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    int i;
    int rc = 0;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            rc = (*func) (hb, arg1, arg2);
            if (rc < 0)
            {
                /* break iterate */
                return;
            }
        }
    }
    return;
}

/* Iterator function for hash with 2 args  */
void
ctclib_hash_iterate2_cont (ctclib_hash_t *hash, int start_index,
               int32 (*func) (ctclib_hash_backet_t *, int, void *, void *),
               void *arg1, void *arg2)
{
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    int i;
    int rc = 0;

    for (i = start_index; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            rc = (*func) (hb, i, arg1, arg2);
            if (rc < 0)
            {
                /* break iterate */
                return;
            }
        }
    }
    return;
}

/* Iterator function for hash with 2 args  */
void
ctclib_hash_iterate2_cont_round (ctclib_hash_t *hash, int start_index,
               int32 (*func) (ctclib_hash_backet_t *, int, void *, void *),
               void *arg1, void *arg2)
{
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    int i;
    int rc = 0;

    for (i = start_index;;)
    {
        if (!hash->index[i])
        {
            /* Notify func to let it count empty bucket as one node have been scanned. */
            rc = (*func) (NULL, i, arg1, arg2);
            if (rc < 0)
            {
                /* break iterate */
                return;
            }
        }
        else
        {
            for (hb = hash->index[i]; hb; hb = next)
            {
                next = hb->next;
                rc = (*func) (hb, i, arg1, arg2);
                if (rc < 0)
                {
                    /* break iterate */
                    return;
                }
            }
        }

        /* Do the round up. */
        i++; 
        i = i % hash->size;
    }
    
    return;
}

/* Iterator function for hash with 3 args  */
void
ctclib_hash_iterate3 (ctclib_hash_t *hash,
               void (*func) (ctclib_hash_backet_t *, void *, void *, void *),
               void *arg1, void *arg2, void *arg3)
{
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    int i;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            (*func) (hb, arg1, arg2, arg3);
        }
    }
    return;
}

/* Iterator function for hash with 4 args  */
void
ctclib_hash_iterate4 (ctclib_hash_t *hash,
               int32 (*func) (ctclib_hash_backet_t *, void *, void *, void *, void *),
               void *arg1, void *arg2, void *arg3, void *arg4)
{
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    int i;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            (*func) (hb, arg1, arg2, arg3, arg4);
        }
    }

    return;        
}


/* Clean up hash.  */
void
ctclib_hash_clean (ctclib_hash_t *hash, void (*free_func) (void *))
{
    int i;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;

            if (free_func)
                (*free_func) (hb->data);

            XFREE(MEM_LIB_HASH_BACKET, hb);
            hash->count--;
        }
        hash->index[i] = NULL;
    }

    return;
}

/* Free hash memory.  You may call hash_clean before call this
   function.  */
void
ctclib_hash_free (ctclib_hash_t *hash)
{
    XFREE(MEM_LIB_HASH_BACKET_LIST, hash->index);
    XFREE(MEM_LIB_HASH, hash);

    return;
}
