#if !defined (__CTCLIB_OPB_H_)
#define __CTCLIB_OPB_H_

#include <sal.h>

#define CTCLIB_MAX_OPB 32

typedef struct
{
    uint32 *data;           /**< bitmap of allocator */
    uint32 words;           /**< number fo 32 bit words in allocator */
    uint32 start_offset;
    uint32 max_size;
    char   *desc;           /**< descption of allocator */    
} ctclib_opb_t;

typedef struct
{
    ctclib_opb_t  *opb_array[CTCLIB_MAX_OPB];
    uint32         index;
    uint32         inited;
} ctclib_opb_master_t;

int32
ctclib_opb_create(ctclib_opb_t* p_opb, uint32 start_offset, uint32 max_size, char *desc);

uint32
ctclib_opb_offset_is_alloced(ctclib_opb_t* p_opb, uint32 offset);

int32
ctclib_opb_alloc_designated_offset(ctclib_opb_t* p_opb, uint32 offset);

int32
ctclib_opb_alloc_offset(ctclib_opb_t* p_opb, uint32* offset);

int32
ctclib_opb_alloc_offset_position_reverse(ctclib_opb_t* p_opb, uint32 position, uint32* offset);

int32
ctclib_opb_alloc_offset_last_reverse(ctclib_opb_t* p_opb, uint32* offset);

int32
ctclib_opb_free_offset(ctclib_opb_t* p_opb, uint32 offset);

int32
ctclib_opb_dump(FILE *fp);

int32
ctclib_opb_init();

#endif /* !__CTCLIB_OPB_H_ */

