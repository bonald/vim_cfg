#ifndef __CTCLIB_OPF_H__
#define __CTCLIB_OPF_H__

#include "sal.h"

/****************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************/

#define CTCLIB_OPF_INVALID_OFFSET  0xFFFFFFFF

struct ctclib_opf_s
{
    uint8  pool_num;
    uint8  reserved[3];

    uint32 *start_offset_a;
    uint32 *max_size_a;
};
typedef struct ctclib_opf_s ctclib_opf_t;

/****************************************************************
 *  
 * Function
 *
 ****************************************************************/
 
extern ctclib_opf_t* 
ctclib_opf_init(uint8 pool_num);

extern bool 
ctclib_opf_init_offset(ctclib_opf_t* p_opf, uint8 pool_index, uint32 start_offset, uint32 max_size);

extern bool 
ctclib_opf_reserve_size_for_reverse_alloc(ctclib_opf_t *p_opf, uint8 pool_index, uint32 block_size);

extern uint32 
ctclib_opf_alloc_offset(ctclib_opf_t *p_opf, uint8 pool_index, uint32 block_size);

extern uint32  
ctclib_opf_reverse_alloc_offset(ctclib_opf_t *p_opf, uint8 pool_index, uint32 block_size);

/*alloc multiple offset*/
extern uint32 
ctclib_opf_alloc_multiple_offset(ctclib_opf_t *p_opf,uint8 pool_index, uint8 multiple, uint32 block_size);

extern uint32 
ctclib_opf_reverse_alloc_multiple_offset(ctclib_opf_t *p_opf, uint8 pool_index, uint8 multiple,uint32 block_size);

extern bool 
ctclib_opf_free_offset(ctclib_opf_t* p_opf, uint8 pool_index, uint32 block_size, uint32 offset);

extern uint32
ctclib_opf_deinit_offset(ctclib_opf_t* p_opf, uint8 pool_index);

extern uint32 
ctclib_opf_destroy(ctclib_opf_t *p_opf);

extern uint32 
ctclib_opf_get_next_entry_info(ctclib_opf_t* p_opf, uint8 pool_index, uint32 *next_offset_index, uint32 *size, uint32 *offset);

#endif /*__CTCLIB_OPF_H__*/

