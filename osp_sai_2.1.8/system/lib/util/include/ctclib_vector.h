#ifndef __CTCLIB_VECTOR_H__
#define __CTCLIB_VECTOR_H__

#include  "sal.h"

/**
 @brief Vector structure
*/
struct ctclib_vector_s 
{
  uint32  block_size;
  uint16  reserved1;
  uint8   max_block_num; 
  uint8   reserved2;
};
typedef struct ctclib_vector_s ctclib_vector_t;

typedef int32 (*ctclib_vector_trav_func)(void *array_data, void *user_data); 

/**
 @brief  Create a Vector structure
 */
extern ctclib_vector_t * 
ctclib_vector_init(uint8 max_block_num, uint32 block_size);


/**
 @brief  Release vector memory space
 */
extern void 
ctclib_vector_release(ctclib_vector_t* vec);


/**
 @brief  Traversing a vector
 */
extern int32
ctclib_vector_traverse(ctclib_vector_t *vec, ctclib_vector_trav_func fn, void *data);


/**
 @brief  Reserve vector space with the specified number of blocks
 */
extern void 
ctclib_vector_reserve(ctclib_vector_t* vec, uint8 block_num );


/**
 @brief  Get data from the vector according to the specified index
 */
extern void * 
ctclib_vector_get(ctclib_vector_t* vec, uint32 index );


/**
 @brief  Delete data from the vector according the specified index
 */
extern void *
ctclib_vector_del(ctclib_vector_t* vec, uint32 index);


/**
 @brief  Add data to the vector according to the specified index 
 */
extern  bool 
ctclib_vector_add(ctclib_vector_t* vec, uint32 index, void* data);


/**
 @brief  Get the number of stored member
 */
extern uint32 
ctclib_vector_get_size(ctclib_vector_t* vec, uint8 block_index );

#endif /* __CTCLIB_VECTOR_H__ */

