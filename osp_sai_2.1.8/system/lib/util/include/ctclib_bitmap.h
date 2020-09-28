#ifndef _CTCLIB_BITMAP_H
#define _CTCLIB_BITMAP_H

/**************************************************************************
 *  
 * Data Type
 *
 **************************************************************************/
struct ctclib_bitmap_hdr_s 
{
    uint16 bit_length;
    uint16 size;
    uint16 type;
    uint16 reserved;
};
typedef struct ctclib_bitmap_hdr_s ctclib_bitmap_hdr_t;

struct ctclib_bitmap_s
{
    ctclib_bitmap_hdr_t hdr;
    uint8 data[1];
};
typedef struct ctclib_bitmap_s ctclib_bitmap_t;

/**************************************************************************
 *  
 * Function Type
 *
 **************************************************************************/
/* Note : the bit_pos starts from 1, NOT 0 ! */
typedef int32 (*p_bitmap_trav_t)(uint32 cur_bit_pos, void * p_data_io);
typedef int32 (*p_bitmap_trav_2args_t)(uint32 cur_bit_pos, void * p_arg1, void * p_arg2);
typedef int32 (*p_bitmap_trav_3args_t)(uint32 cur_bit_pos, void * p_arg1, void * p_arg2, void * p_arg3);

extern int32 ctclib_bitmap_is_valid(ctclib_bitmap_t* p_bitmap, uint32* p_is_valid);

extern int32 ctclib_bitmap_clear(ctclib_bitmap_t * p_bitmap);

extern int32 ctclib_bitmap_set_bit(ctclib_bitmap_t * p_bitmap, uint16 bit_pos);

extern int32 ctclib_bitmap_clear_bit(ctclib_bitmap_t * p_bitmap, uint16 bit_pos);

extern int32 ctclib_bitmap_is_bit_set(ctclib_bitmap_t * p_bitmap, uint16 bit_pos, uint32 * is_set);
extern int32  ctclib_bitmap_is_bit_set_range(ctclib_bitmap_t * p_bitmap, uint16 bit_pos_from, 
                                            uint16 bit_pos_to, uint32 * is_set);
extern int32 ctclib_bitmap_is_empty(ctclib_bitmap_t * p_bitmap);

extern int32 ctclib_bitmap_get_bit_cnt(ctclib_bitmap_t * p_bitmap, uint16 * p_bit_cnt);

extern int32 
ctclib_bitmap_traverse(ctclib_bitmap_t * p_bitmap, uint16 bit_cnt, p_bitmap_trav_t p_trav_func, void * p_data_io);
extern int32 
ctclib_bitmap_traverse_2args(ctclib_bitmap_t * p_bitmap, uint16 bit_cnt,
                             p_bitmap_trav_2args_t p_trav_func, void * p_arg1, void * p_arg2);
extern int32 
ctclib_bitmap_traverse_3args(ctclib_bitmap_t * p_bitmap, uint16 bit_cnt,
                             p_bitmap_trav_3args_t p_trav_func, void * p_arg1, void * p_arg2, void * p_arg3);

extern int32 ctclib_bitmap_create_bmp(ctclib_bitmap_t **pp_bitmap, uint32 bit_cnt);

extern int32 ctclib_bitmap_destroy_bmp(ctclib_bitmap_t *p_bitmap);

extern int32 ctclib_bitmap_data_in_uint8_cnt(ctclib_bitmap_t * p_bitmap, uint16 * p_uint8_cnt);

extern int32 ctclib_bitmap_1st_bit_pos(ctclib_bitmap_t * p_bitmap, uint16 * p_1st_bit_pos);
#endif /*_CTCLIB_BITMAP_H*/

