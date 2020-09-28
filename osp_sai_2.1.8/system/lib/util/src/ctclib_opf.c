#include "ctclib_opf.h"
#include "ctclib_memory.h"

/****************************************************************************
 *  
 * Defines and Macros
 *
 ****************************************************************************/

struct ctclib_opf_entry_s
 {
     struct ctclib_opf_entry_s *prev;
     struct ctclib_opf_entry_s *next;
     uint32 size;
     uint32 offset;
 };
typedef struct ctclib_opf_entry_s ctclib_opf_entry_t;

struct ctclib_opf_master_s
{
    ctclib_opf_t       opf;
    ctclib_opf_entry_t **pp_opf_pre; 
    ctclib_opf_entry_t **pp_opf_rev; 
    
    uint32             *max_offset_for_pre_alloc;
    uint32             *min_offset_for_rev_alloc;
};
typedef struct ctclib_opf_master_s ctclib_opf_master_t;


/****************************************************************************
 *
 * Function prototype
 *
 ****************************************************************************/
static bool 
_ctclib_opf_pre_free_offset(ctclib_opf_master_t *p_opf_master,uint8 pool_index, uint32 block_size, uint32 offset);
static bool 
_ctclib_opf_reverse_free_offset(ctclib_opf_master_t *p_opf_master,uint8 pool_index, uint32 block_size,uint32 offset);


/****************************************************************************
 *  
 * Function
 *
 ****************************************************************************/

ctclib_opf_t* 
ctclib_opf_init(uint8 pool_num)
{
    ctclib_opf_master_t *p_opf_master;

    
    p_opf_master = (ctclib_opf_master_t *)XCALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_master_t));
    if (!p_opf_master)
    {
        return NULL;
    }
    
    p_opf_master->pp_opf_pre = (ctclib_opf_entry_t **)XCALLOC(MEM_LIB_OPF, sizeof(void*));
    if (!p_opf_master->pp_opf_pre)
    {
        XFREE(MEM_LIB_OPF, p_opf_master);
        return NULL;
    }

    p_opf_master->pp_opf_rev = (ctclib_opf_entry_t **)XCALLOC(MEM_LIB_OPF, sizeof(void*));
    if (!p_opf_master->pp_opf_rev)
    {
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre);
        XFREE(MEM_LIB_OPF, p_opf_master);
        return NULL;
    }
    
    p_opf_master->max_offset_for_pre_alloc = (uint32 *)XCALLOC(MEM_LIB_OPF, pool_num * sizeof(uint32));
    if (!p_opf_master->max_offset_for_pre_alloc)
    {
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_rev);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre);
        XFREE(MEM_LIB_OPF, p_opf_master);
        return NULL;
    }
    
    p_opf_master->min_offset_for_rev_alloc = (uint32 *)XCALLOC(MEM_LIB_OPF, pool_num * sizeof(uint32));
    if (!p_opf_master->min_offset_for_rev_alloc)
    {
        XFREE(MEM_LIB_OPF, p_opf_master->max_offset_for_pre_alloc);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_rev);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre);
        XFREE(MEM_LIB_OPF, p_opf_master);
        return NULL;
    }

    p_opf_master->opf.start_offset_a = (uint32 *)XCALLOC(MEM_LIB_OPF, pool_num * sizeof(uint32));
    if (!p_opf_master->opf.start_offset_a)
    {
        XFREE(MEM_LIB_OPF, p_opf_master->min_offset_for_rev_alloc);
        XFREE(MEM_LIB_OPF, p_opf_master->max_offset_for_pre_alloc);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_rev);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre);
        XFREE(MEM_LIB_OPF, p_opf_master);
        return NULL;
    }

    p_opf_master->opf.max_size_a = (uint32 *)XCALLOC(MEM_LIB_OPF, pool_num * sizeof(uint32));
    if (!p_opf_master->opf.max_size_a)
    {
        XFREE(MEM_LIB_OPF, p_opf_master->opf.start_offset_a);
        XFREE(MEM_LIB_OPF, p_opf_master->min_offset_for_rev_alloc);
        XFREE(MEM_LIB_OPF, p_opf_master->max_offset_for_pre_alloc);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_rev);
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre);
        XFREE(MEM_LIB_OPF, p_opf_master);
        return NULL;
    }
  
    p_opf_master->opf.pool_num = pool_num;

    return &p_opf_master->opf;
}

bool 
ctclib_opf_init_offset(ctclib_opf_t* p_opf, uint8 pool_index, uint32 start_offset, uint32 max_size)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;

    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return FALSE;
    }

    if (p_opf_master->pp_opf_pre[pool_index])
    {
        return TRUE;
    }

    p_opf_master->pp_opf_pre[pool_index] = (ctclib_opf_entry_t *)XCALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_entry_t));
    if (!p_opf_master->pp_opf_pre[pool_index])
    {
        return FALSE;
    }    

    p_opf_master->pp_opf_rev[pool_index] = (ctclib_opf_entry_t *)XCALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_entry_t));
    if (!p_opf_master->pp_opf_rev[pool_index])
    {
        XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre[pool_index]);
        return FALSE;
    }

    p_opf->start_offset_a[pool_index] = start_offset;
    p_opf->max_size_a[pool_index]     = max_size;

    p_opf_master->max_offset_for_pre_alloc[pool_index] = start_offset;
    p_opf_master->min_offset_for_rev_alloc[pool_index] = start_offset;
  
    p_opf_master->pp_opf_pre[pool_index]->next   = NULL;
    p_opf_master->pp_opf_pre[pool_index]->prev   = NULL;
    p_opf_master->pp_opf_pre[pool_index]->offset = start_offset;
    p_opf_master->pp_opf_pre[pool_index]->size   = max_size;

    p_opf_master->pp_opf_rev[pool_index]->next   = NULL;
    p_opf_master->pp_opf_rev[pool_index]->prev   = NULL;
    p_opf_master->pp_opf_rev[pool_index]->offset = start_offset;
    p_opf_master->pp_opf_rev[pool_index]->size   = max_size;

    return TRUE;
}

bool 
ctclib_opf_reserve_size_for_reverse_alloc(ctclib_opf_t *p_opf, uint8 pool_index, uint32 block_size)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;
    uint32 start_offset = 0;  
    uint32 max_size = 0;
    
    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return FALSE;
    }

    if (block_size > p_opf->max_size_a[pool_index])
    {
        return FALSE;
    }
    
    if (block_size != 0)
    {
        start_offset = p_opf->start_offset_a[pool_index];
        max_size     = p_opf->max_size_a[pool_index];

        p_opf_master->max_offset_for_pre_alloc[pool_index] = start_offset + max_size - block_size;
        p_opf_master->min_offset_for_rev_alloc[pool_index] = start_offset + max_size - block_size;      
    }
    
    return TRUE;
}

uint32  
ctclib_opf_reverse_alloc_offset(ctclib_opf_t *p_opf, uint8 pool_index, uint32 block_size)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;
    uint32 start_offset = 0;
    uint32 end_offset = 0;
    uint32 reverse_offset = 0;
    uint32 max_size = 0;
    uint32 min_offset = 0;
    ctclib_opf_entry_t *entry, *next;
    
    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    if (block_size > p_opf->max_size_a[pool_index])
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    start_offset = p_opf->start_offset_a[pool_index];
    max_size     = p_opf->max_size_a[pool_index];
    end_offset   = start_offset + max_size;

    entry = p_opf_master->pp_opf_rev[pool_index];

    if (NULL == entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }    

    if (block_size == 0)
    {
        block_size = 1;
    }

    while (entry)
    {
        reverse_offset = (end_offset - entry->offset - block_size) + start_offset;
        if (entry->size >= block_size
           && (reverse_offset >= p_opf_master->max_offset_for_pre_alloc[pool_index]))
        {
            break;
        }
        else
        {
            entry = entry->next;
        }
    }
       
    if (!entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    offset = reverse_offset;

    if (entry->size > block_size)
    {
        entry->size -= block_size;
        entry->offset += block_size;
        min_offset = entry->offset;
    }
    else    /* entry->size == block_size */
    {
        entry->size -= block_size;
        entry->offset += block_size;
        min_offset = entry->offset;
        next = entry->next;

        if (entry->prev && next)
        {
            entry->prev->next = next;
            next->prev = entry->prev;

            XFREE(MEM_LIB_OPF, entry);
        }
        else if (next)
        {
            /* entry->prev == NULL && next != NULL */
            p_opf_master->pp_opf_rev[pool_index] = next;
            next->prev = entry->prev;

            XFREE(MEM_LIB_OPF, entry);
        }
        /* next == NULL in this condition this node cannot be free */
    }

    if (min_offset > p_opf_master->min_offset_for_rev_alloc[pool_index])
    {
        p_opf_master->min_offset_for_rev_alloc[pool_index] = min_offset;
    }

    return offset;
}

static bool 
_ctclib_opf_reverse_free_offset(ctclib_opf_master_t *p_opf_master,uint8 pool_index, uint32 block_size,uint32 offset)
{
    uint32 start_offset = 0;
    uint32 end_offset = 0;
    uint32 reverse_offset = 0;
    uint32 max_size = 0;
    uint32 prev_offset = 0;
    ctclib_opf_entry_t *entry, *next, *prev, *node;
    
    if (!p_opf_master || pool_index >= p_opf_master->opf.pool_num)
    {
        return FALSE;
    }
    
    start_offset = p_opf_master->opf.start_offset_a[pool_index];
    max_size     = p_opf_master->opf.max_size_a[pool_index];
    end_offset   = start_offset + max_size;

    if (block_size > max_size)
    {
        return FALSE;
    }
    
    /*reverse offset for pre-allocation freelist*/
    reverse_offset = end_offset - offset - block_size + start_offset;

    if (reverse_offset < start_offset || reverse_offset > end_offset )
    {
        return FALSE;
    }
    
    entry = p_opf_master->pp_opf_rev[pool_index];
    while (entry != NULL)
    {
        if (reverse_offset + block_size > entry->offset)
        {
            prev_offset = entry->offset + entry->size;    
            entry = entry->next;
        }
        else if (reverse_offset + block_size == entry->offset)
        {
            /*change offset and block size */
            entry->offset = reverse_offset;
            entry->size += block_size;
            break;
        }
        else
        {
            if (reverse_offset < prev_offset)
            {
                /* an error condition */
                entry = NULL;
                break;
            }
            /* insert a new free node */
            node = (ctclib_opf_entry_t*)XCALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_entry_t));
            if (!node)
            {
                return FALSE;
            }

            node->offset = reverse_offset;
            node->size = block_size;
            prev = entry->prev;
            
            if (prev)
            {
                prev->next = node;
            }
            else
            {
                /* if the enry is the first node, we must update global database */
                p_opf_master->pp_opf_rev[pool_index] = node;
            }
            node->next  = entry;
            node->prev  = prev;
            entry->prev = node;
            entry = node;
            break;
        }
    }

    if (entry != NULL)
    {
        prev = entry->prev;
        next = entry->next;
        
        if (prev && (prev_offset == entry->offset))
        {
            prev->next = next;

            if (next)
            {
                next->prev = prev;
            }
            prev->size += entry->size;
            XFREE(MEM_LIB_OPF, entry);

            entry = prev;
        }
        
        if (!entry->next)
        {
            p_opf_master->min_offset_for_rev_alloc[pool_index] = entry->offset;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

uint32 
ctclib_opf_alloc_offset(ctclib_opf_t *p_opf, uint8 pool_index, uint32 block_size)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;
    uint32 start_offset = 0;
    uint32 max_size = 0;
    uint32 end_offset = 0;
    uint32 reverse_offset =0;
    ctclib_opf_entry_t *entry, *next;

    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }

    start_offset = p_opf->start_offset_a[pool_index];
    max_size     = p_opf->max_size_a[pool_index];
    end_offset   = start_offset + max_size;
    

    if (pool_index >= p_opf->pool_num)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }

    if (block_size == 0)
    {
        block_size = 1;
    }
    
    if (block_size > p_opf->max_size_a[pool_index])
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }

    entry = p_opf_master->pp_opf_pre[pool_index];

    if (NULL == entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }

    while (entry)
    {
        reverse_offset = (end_offset - entry->offset - block_size) + start_offset;
        
        if (entry->size >= block_size
           && (reverse_offset >=  p_opf_master->min_offset_for_rev_alloc[pool_index]))
        {
            break;
        }
        else
        {
            entry = entry->next;
        }
    }
    
    if (!entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }

    offset = entry->offset;

    entry->size -= block_size;
    if(entry->size == 0)
    {
        next = entry->next;
        if(entry->prev)
        {
            entry->prev->next = next;
        }
        else
        {
            p_opf_master->pp_opf_pre[pool_index] = next;
        }
        
        if(next)
        {
            next->prev = entry->prev;
        }
        XFREE(MEM_LIB_OPF, entry);
    }
    else
    {
        entry->offset += block_size;
    }

    if ((offset + block_size) > p_opf_master->max_offset_for_pre_alloc[pool_index])
    {
        p_opf_master->max_offset_for_pre_alloc[pool_index] = offset + block_size;
    }

	return offset;
}



uint32 
ctclib_opf_alloc_multiple_offset(ctclib_opf_t *p_opf,uint8 pool_index, uint8 multiple, uint32 block_size)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;
    uint32 remainder = 0;
    uint32 skip_value = 0;
    uint32 start_offset = 0;
    uint32 end_offset = 0;
    uint32 reverse_offset =0;  
    uint32 max_size = 0;
    ctclib_opf_entry_t *entry, *next;
    ctclib_opf_entry_t  *node;
      
    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    start_offset = p_opf->start_offset_a[pool_index];
    max_size     = p_opf->max_size_a[pool_index];
    end_offset   = start_offset + max_size;
    
    if (block_size > p_opf->max_size_a[pool_index])
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }

    entry = p_opf_master->pp_opf_pre[pool_index];
    if (NULL == entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }    

    if (block_size == 0)
    {
        block_size = 1;
    }
    
    while (entry)
    {
        reverse_offset = (end_offset - entry->offset - block_size) + start_offset;
        
        if (entry->size >= block_size && 
            (reverse_offset >= p_opf_master->min_offset_for_rev_alloc[pool_index]))
        {
            remainder = entry->offset % multiple;
            
            skip_value = remainder ? (multiple - remainder): remainder;

            if (entry->size >= (skip_value + block_size))
            {
                if (remainder)
                {
                    /*insert a node, allocate offset from first multiple offset*/
                    node = (ctclib_opf_entry_t*) XCALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_entry_t));
                    if (!node)
                    {
                        return CTCLIB_OPF_INVALID_OFFSET;
                    }
                    
                    node->size = entry->size - skip_value;
                    node->offset = entry->offset + skip_value;
                    node->prev = entry;
                    node->next = entry->next;
                    
                    entry->size = skip_value;
                    entry->next = node;
                    entry = node;
                }
                break;
            }       
        }
        entry = entry->next;
    }
    
    if (!entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }


    offset = entry->offset;

    if (entry->size > block_size)
    {
        entry->size -= block_size;
        entry->offset += block_size;
    }
    else    /* entry->size == block_size */
    {
        entry->size -= block_size;
        entry->offset += block_size;

        next = entry->next;

        if (entry->prev && next)
        {
            entry->prev->next = next;
            next->prev = entry->prev;

            XFREE(MEM_LIB_OPF, entry);
        }
        else if (next)
        {
            /* entry->prev == NULL && next != NULL */
            p_opf_master->pp_opf_pre[pool_index] = next;
            next->prev = entry->prev;

            XFREE(MEM_LIB_OPF, entry);
        }
        /* next == NULL in this condition this node cannot be free */
    }
    
    if ((offset + block_size) > p_opf_master->max_offset_for_pre_alloc[pool_index])
    {
        p_opf_master->max_offset_for_pre_alloc[pool_index] = offset + block_size;
    }

    return offset;
}


uint32 
ctclib_opf_reverse_alloc_multiple_offset(ctclib_opf_t *p_opf, uint8 pool_index, uint8 multiple,uint32 block_size)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;
    uint32 start_offset = 0;   
    uint32 end_offset = 0;  
    uint32 reverse_offset = 0;
    uint32 max_size = 0; 
    uint32 remainder = 0;
    uint32 skip_value = 0;
    uint32 min_offset = 0;
    ctclib_opf_entry_t *entry, *next;
    ctclib_opf_entry_t *node;
    
    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    if (block_size > p_opf->max_size_a[pool_index])
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    start_offset = p_opf->start_offset_a[pool_index];
    max_size = p_opf->max_size_a[pool_index];
    end_offset = start_offset + max_size;

    entry = p_opf_master->pp_opf_rev[pool_index];

    if (NULL == entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;        
    }    

    if (block_size == 0)
    {
        block_size = 1;
    }

    while (entry)
    {
        reverse_offset = (end_offset - entry->offset - block_size) + start_offset;
        if (entry->size >= block_size && 
            (reverse_offset >= p_opf_master->max_offset_for_pre_alloc[pool_index]))
        {
            remainder = reverse_offset % multiple;
            skip_value = multiple - remainder;

            if (entry->size >= (remainder ? (skip_value + block_size) : block_size))
            {
                if (remainder)
                {
                    /*insert a node, allocate offset from first multiple offset*/
                    node = (ctclib_opf_entry_t*) XCALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_entry_t));
                    if (!node)
                    {
                        return CTCLIB_OPF_INVALID_OFFSET;
                    }
                    
                    node->size = entry->size - skip_value;
                    node->offset = entry->offset + skip_value;
                    node->prev = entry;
                    node->next = entry->next;
                    
                    entry->size = skip_value;
                    entry->next = node;
                    entry = node;
                }
                break;
            }          
        }
            entry = entry->next;
    }

   
    if (!entry)
    {
        return CTCLIB_OPF_INVALID_OFFSET;
    }
    
    reverse_offset = (end_offset - entry->offset - block_size) + start_offset;
     
    if (entry->size > block_size)
    {
        entry->size -= block_size;
        entry->offset += block_size;
    }
    else    /* entry->size == block_size */
    {
        entry->size -= block_size;
        entry->offset += block_size;
        min_offset = entry->offset;
        next = entry->next;

        if (entry->prev && next)
        {
            entry->prev->next = next;
            next->prev = entry->prev;

            XFREE(MEM_LIB_OPF, entry);
        }
        else if (next)
        {
            /* entry->prev == NULL && next != NULL */
            p_opf_master->pp_opf_rev[pool_index] = next;
            next->prev = entry->prev;

            XFREE(MEM_LIB_OPF, entry);
        }
        /* next == NULL in this condition this node cannot be free */
    }

    offset = reverse_offset;
    
    if (min_offset > p_opf_master->min_offset_for_rev_alloc[pool_index])
    {
        p_opf_master->min_offset_for_rev_alloc[pool_index] = min_offset;
    }
    
    return offset;
}

static bool 
_ctclib_opf_pre_free_offset(ctclib_opf_master_t *p_opf_master,uint8 pool_index, uint32 block_size, uint32 offset)
{
    uint32 start_offset;
    uint32 max_size = 0;
    ctclib_opf_entry_t *entry, *prev = NULL, *next = NULL, *node;
    uint8  is_merged = 0;
   
    start_offset = p_opf_master->opf.start_offset_a[pool_index];
    max_size     = p_opf_master->opf.max_size_a[pool_index];

    if (block_size == 0)
    {
        block_size = 1;
    }

    if (block_size > max_size)
    {
        return FALSE;
    }

    if (offset < start_offset || (offset + block_size) > (start_offset + max_size))
    {
        return FALSE;
    }
    
    entry = p_opf_master->pp_opf_pre[pool_index];
    
    while (entry != NULL)
    {
        /* |              ..********      | */ 
        if (offset + block_size == entry->offset)
        {
            /*change offset and block size */
            entry->offset = offset;
            entry->size += block_size;
            is_merged = 1;
            break;
        }        
        /* |              ********..      | */
        if (entry->offset + entry->size == offset)
        {
            /*change block size */
            entry->size += block_size;
            is_merged = 1;
            if(entry->next && entry->next->offset == entry->offset + entry->size)
            {
                next = entry->next;
                entry->size += next->size;
                entry->next = next->next;
                if(entry->next)
                {
                    entry->next->prev = entry;
                }
                XFREE(MEM_LIB_OPF, next);
            }
            break;
        }
        /* |            ..  ********      | */
        if(offset + block_size < entry->offset)
        {
            break;
        }
        /* |            ********  ..      | */
        if(offset > entry->offset + entry->size)
        {
            prev = entry;
            entry = entry->next;
            continue;
        }
        /* |            ********          | */
        /* |              ..              | */

        /* Invalid offset or block size */
        return FALSE;
    }

    if(!is_merged)
    {
        node = (ctclib_opf_entry_t*)XMALLOC(MEM_LIB_OPF, sizeof(ctclib_opf_entry_t));
        if (!node)
        {
            return FALSE;
        }

        node->offset = offset;
        node->size = block_size;
        node->prev = prev;
        node->next = entry;

        if (prev)
        {
            prev->next = node;
        }
        else
        {
            p_opf_master->pp_opf_pre[pool_index] = node;
        }

        if (entry)
        {
            entry->prev = node;
        }
    }
    else
    {
        if (p_opf_master->max_offset_for_pre_alloc[pool_index] <= entry->offset + entry->size)
        {
            p_opf_master->max_offset_for_pre_alloc[pool_index] = entry->offset;
        }
    }

    return TRUE;
}


bool 
ctclib_opf_free_offset(ctclib_opf_t* p_opf, uint8 pool_index, uint32 block_size, uint32 offset)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *) p_opf;    
    
    if (!p_opf || pool_index >= p_opf->pool_num)
    {
        return FALSE;
    }

    if (offset >= p_opf_master->max_offset_for_pre_alloc[pool_index])
    {
        return  _ctclib_opf_reverse_free_offset(p_opf_master, pool_index, block_size, offset);
    }
    else
    {
        return  _ctclib_opf_pre_free_offset(p_opf_master, pool_index, block_size, offset);
    }
    
}


uint32  
ctclib_opf_deinit_offset(ctclib_opf_t* p_opf, uint8 pool_index)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;

    ctclib_opf_entry_t *entry, *next;

    entry = p_opf_master->pp_opf_pre[pool_index];

    while(entry)
    {
        next = entry->next;
        XFREE(MEM_LIB_OPF, entry);
        entry = next;
    }

    entry = p_opf_master->pp_opf_rev[pool_index];

    while(entry)
    {
        next = entry->next;
        XFREE(MEM_LIB_OPF, entry);
        entry = next;
    }

    return 0;
}

uint32 
ctclib_opf_destroy(ctclib_opf_t *p_opf)
{
    ctclib_opf_master_t *p_opf_master = (ctclib_opf_master_t *)p_opf;

    XFREE(MEM_LIB_OPF, p_opf_master->opf.max_size_a);
    XFREE(MEM_LIB_OPF, p_opf_master->opf.start_offset_a);
    XFREE(MEM_LIB_OPF, p_opf_master->min_offset_for_rev_alloc);
    XFREE(MEM_LIB_OPF, p_opf_master->max_offset_for_pre_alloc);
    XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_rev);
    XFREE(MEM_LIB_OPF, p_opf_master->pp_opf_pre);
    XFREE(MEM_LIB_OPF, p_opf_master);

    return 0;
}

uint32 
ctclib_opf_get_next_entry_info(ctclib_opf_t* p_opf, uint8 pool_index, uint32 *next_offset_index, uint32 *size, uint32 *offset)
{
    ctclib_opf_entry_t *entry = NULL;
    ctclib_opf_master_t *p_opf_master = NULL;
    uint32 tmp_offset_index = 0;

    p_opf_master = (ctclib_opf_master_t *)p_opf;
    entry = p_opf_master->pp_opf_pre[pool_index]; 

    /* walk through the entry list */
    while (entry)
    {   
        tmp_offset_index ++;
        if (tmp_offset_index <= (*next_offset_index))
        {
            entry = entry->next;
            continue;
        }
        
        *size = entry->size;
        *offset = entry->offset;
        *next_offset_index = tmp_offset_index;
        return TRUE;
    }

    return FALSE;
}

