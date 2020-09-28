#include "ctclib_vector.h"
#include "ctclib_memory.h"

#define CTCLIB_MAX_VEC_BLOCK_NUM   256

/* Vector db structure. */
struct ctclib_vector_db_s 
{
  ctclib_vector_t vec; 
  uint32  vec_size[CTCLIB_MAX_VEC_BLOCK_NUM];
  void ***pp_array;
};
typedef struct ctclib_vector_db_s  ctclib_vector_db_t;


/**
 @brief  Add data to the vector according to the specified index 
 */
bool 
ctclib_vector_add(ctclib_vector_t *vec, uint32 index, void *data)
{
    uint8  idx_1d = 0;
    uint32 idx_2d = 0;  
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;

    if (!vec || !p_vector->pp_array)
    {
        return FALSE;
    }
     
    idx_1d  = index / p_vector->vec.block_size;
    idx_2d  = index % p_vector->vec.block_size;
    if ( idx_1d >= p_vector->vec.max_block_num)
    {
        return FALSE;
    }
    
    if (!p_vector->pp_array[idx_1d])
    {
        p_vector->pp_array[idx_1d] = (void **)XCALLOC(MEM_LIB_VECTOR_DATA, p_vector->vec.block_size * sizeof(void*));
    }

    if (!p_vector->pp_array[idx_1d])
    {
        return FALSE;
    }

    if (!p_vector->pp_array[idx_1d][idx_2d] && data)
    {
        p_vector->vec_size[idx_1d]++;
    }

    p_vector->pp_array[idx_1d][idx_2d] = data;

    return TRUE;
}

/**
 @brief  Delete data from the vector according the specified index
 */
void *
ctclib_vector_del(ctclib_vector_t *vec, uint32 index)
{
    uint32 idx_1d = 0;
    uint32 idx_2d = 0;
    void   *p_data = NULL;
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;

    if (!vec || !p_vector->pp_array)
    {
        return NULL;
    }

    idx_1d = index / p_vector->vec.block_size;
    idx_2d = index % p_vector->vec.block_size;

    if (!p_vector->pp_array[idx_1d])
    {
        return NULL; 
    }

    if (idx_1d >= p_vector->vec.max_block_num)
    {
        return NULL;
    }

    if (p_vector->pp_array[idx_1d][idx_2d])
    {
        p_vector->vec_size[idx_1d]--;
    }

    p_data = p_vector->pp_array[idx_1d][idx_2d];
    p_vector->pp_array[idx_1d][idx_2d] = NULL;
    if(p_vector->vec_size[idx_1d] == 0)
    {
        XFREE(MEM_LIB_VECTOR_DATA, p_vector->pp_array[idx_1d]);
        p_vector->pp_array[idx_1d] = NULL;
    }

    return p_data;
}


/**
 @brief  Get data from the vector according to the specified index
 */
void * 
ctclib_vector_get(ctclib_vector_t *vec, uint32 index)
{
    uint32  idx_1d = 0;
    uint32  idx_2d = 0; 
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;
   
    if (!vec || !p_vector->pp_array)
    {
        return NULL;
    }
   
    idx_1d  = index / p_vector->vec.block_size;
    idx_2d  = index % p_vector->vec.block_size;
    if (idx_1d >= p_vector->vec.max_block_num || !p_vector->pp_array[idx_1d])
    {
        return NULL; 
    }
  
    return p_vector->pp_array[idx_1d][idx_2d];
}

/**
 @brief  Traversing a vector
 */
int32 
ctclib_vector_traverse(ctclib_vector_t *vec, ctclib_vector_trav_func fn, void *data)
{
    uint32 idx_1d, idx_2d; 
    int32  ret = 0;
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;
   
    if (!vec || !p_vector->pp_array)
    {
        return 0;
    }

    for(idx_1d = 0; idx_1d < p_vector->vec.max_block_num; idx_1d++)
    {
        if(!p_vector->pp_array[idx_1d])
        {
            continue;
        }
        
        for(idx_2d = 0; idx_2d < p_vector->vec.block_size; idx_2d++)
        {
            if(!p_vector->pp_array[idx_1d][idx_2d])
            {
                continue;
            }
            if ((ret = (*fn)(p_vector->pp_array[idx_1d][idx_2d], data)) < 0)
            {
                return ret;
            }
        }
    }

    return 0;
}

/**
 @brief  Reserve vector space with the specified number of blocks
 */
void 
ctclib_vector_reserve(ctclib_vector_t *vec, uint8 block_num)
{
    uint32  idx_1d = 0;
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;
   
    if (!vec || !p_vector->pp_array)
    {
        return ;
    }
    
    if (block_num > p_vector->vec.max_block_num)
    {
        return ;
    }      

    for (idx_1d = 0; idx_1d < block_num; idx_1d++)
    {
        if (!p_vector->pp_array[idx_1d])
        {
            p_vector->pp_array[idx_1d] = 
                (void *)XCALLOC(MEM_LIB_VECTOR_DATA, p_vector->vec.block_size * sizeof(void *));
        }
    }
}


/**
 @brief  Release vector memory space
 */  
void 
ctclib_vector_release(struct ctclib_vector_s *vec)
{
    uint32  idx_1d = 0;
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;
   
    if (!vec || !p_vector->pp_array)
    {
        return ;
    }
    
    for (idx_1d = 0; idx_1d < p_vector->vec.max_block_num; idx_1d++)
    {
        if (p_vector->pp_array[idx_1d])
        {
            XFREE(MEM_LIB_VECTOR_DATA, p_vector->pp_array[idx_1d]);
            p_vector->pp_array[idx_1d] = NULL;
            p_vector->vec_size[idx_1d] = 0;
        }
    }
  
    XFREE(MEM_LIB_VECTOR_DATA, p_vector->pp_array);
    p_vector->pp_array = NULL;
}


/**
 @brief  Get the number of stored member
 */
uint32 
ctclib_vector_get_size(ctclib_vector_t* vec, uint8 block_index)
{
    ctclib_vector_db_t *p_vector = (ctclib_vector_db_t *)vec;
    
    if (!vec || !p_vector->pp_array)
    {
        return 0;
    }
    
    if (block_index >= p_vector->vec.max_block_num)
    {
        return 0;
    }

    return p_vector->vec_size[block_index];
}


/**
 @brief  Create a Vector structure
 */
ctclib_vector_t* 
ctclib_vector_init(uint8 max_block_num, uint32 block_size)
{
    ctclib_vector_db_t *p_vector = NULL;

    p_vector = (ctclib_vector_db_t *)XCALLOC(MEM_LIB_VECTOR_DB, sizeof(ctclib_vector_db_t));
    if (!p_vector)
    {
       return NULL;
    }

    p_vector->vec.max_block_num = max_block_num;
    p_vector->vec.block_size    = block_size;   
    p_vector->pp_array          = (void ***)XCALLOC(MEM_LIB_VECTOR_DATA, max_block_num * sizeof(void **));

    if (!p_vector->pp_array)
    {
       XFREE(MEM_LIB_VECTOR_DB, p_vector);
       return NULL;
    }

    return (ctclib_vector_t*)&p_vector->vec;
}


