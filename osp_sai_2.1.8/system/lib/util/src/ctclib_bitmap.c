
/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal.h"
#include "ctclib_bitmap.h"
#include "ctclib_memory.h"
/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
/* +++ for 1st bit position +++ */
/* Note : each item indicates 1st bit's pos in the according half-byte */
const static uint8 ctclib_bit_1st_pos_map_1[16][2] =
{
    /* 0x00 */  /* 0x01 */  /* 0x02 */  /* 0x03 */
    {0, 0xFF},   {4, 0},      {3, 0},      {3, 0},

    /* 0x04 */  /* 0x05 */  /* 0x06 */  /* 0x07 */
    {2, 0},      {2, 0},      {2, 0},      {2, 0},

    /* 0x08 */  /* 0x09 */  /* 0x0A */  /* 0x0B */
    {1, 0},      {1, 0},      {1, 0},      {1, 0},

    /* 0x0C */  /* 0x0D */  /* 0x0E */  /* 0x0F */
    {1, 0},      {1, 0},      {1, 0},      {1, 0},
};

const static uint8 ctclib_bit_1st_pos_map_2[16] =
{
    /* 0x00 */  /* 0x01 */  /* 0x02 */  /* 0x03 */
    0,          8,           7,          7,

    /* 0x04 */  /* 0x05 */  /* 0x06 */  /* 0x07 */
    6,          6,           6,          6,

    /* 0x08 */  /* 0x09 */  /* 0x0A */  /* 0x0B */
    5,          5,           5,          5,

    /* 0x0C */  /* 0x0D */  /* 0x0E */  /* 0x0F */
    5,          5,           5,          5,
};
/* --- for 1st bit position --- */

/****************************************************************************
 *  
* Function
*
*****************************************************************************/
int32
ctclib_bitmap_is_valid(ctclib_bitmap_t* p_bitmap, uint32* p_is_valid)
{
    uint32 is_valid = TRUE;

    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_is_valid)
    {
        return -1;
    }  


    /* 2. do it, deal with the checksum and other valid check, TBD */
    if(0 == p_bitmap->hdr.bit_length)
    {
        is_valid = FALSE;
    };

    
    /* 3. write back, we just set it TRUE temporaly */
    *p_is_valid = is_valid;

    return 0;
};

int32 
ctclib_bitmap_clear(ctclib_bitmap_t * p_bitmap)
{
    uint16 data_byte_cnt;
    uint8 * p_data;


    /* 1. sanity check & init */
    if (NULL == p_bitmap)
    {
        return -1;
    }

    /* 2. do it */
    p_data = (uint8 *)(&(p_bitmap->data[0]));

    data_byte_cnt = (p_bitmap->hdr.bit_length / 8) + ((p_bitmap->hdr.bit_length % 8) ? 1 : 0);   
    sal_memset(p_data, 0, data_byte_cnt);

    return 0;
};


int32 
ctclib_bitmap_set_bit(ctclib_bitmap_t * p_bitmap, uint16 bit_pos)
{
    uint8 * p_data;
    uint32 bytes_cnt, bits_cnt;
    const uint8 mask = 0x80;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || p_bitmap->hdr.bit_length < bit_pos)
    {
        return -1;
    }

    /* 2. do it */
    p_data = &(p_bitmap->data[0]);
    
    bytes_cnt = ((bit_pos - 1) / 8);
    bits_cnt = ((bit_pos - 1) % 8);
    p_data[bytes_cnt] |= (mask >> (bits_cnt));


    return 0;
};

int32 
ctclib_bitmap_clear_bit(ctclib_bitmap_t * p_bitmap, uint16 bit_pos)
{
    uint8 * p_data = NULL;
    uint16 bytes_cnt, bits_cnt;
    const uint8 mask = 0x80;


    /* 1. sanity check & init */
    if (NULL == p_bitmap)
    {
        return -1;
    }

    /* 2. do it */
    p_data = &(p_bitmap->data[0]);
    
    bytes_cnt = ((bit_pos - 1) / 8);
    bits_cnt = ((bit_pos - 1) % 8);
    p_data[bytes_cnt] &= ~(mask >> (bits_cnt));


    return 0;
};

int32 
ctclib_bitmap_is_bit_set(ctclib_bitmap_t * p_bitmap, uint16 bit_pos, uint32 * is_set)
{
    uint8 * p_data = NULL;
    uint16 bytes_cnt, bits_cnt;
    const uint8 mask = 0x80;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == is_set)
    {
        return -1;
    }

    /* 2. do it */
    p_data = &(p_bitmap->data[0]);
    
    bytes_cnt = ((bit_pos - 1) / 8);
    bits_cnt = ((bit_pos - 1) % 8);
    *is_set = ((p_data[bytes_cnt] & (mask >> (bits_cnt))) ? TRUE : FALSE);


    return 0;
};

int32 
ctclib_bitmap_is_bit_set_range(ctclib_bitmap_t * p_bitmap, uint16 bit_pos_from, 
                                     uint16 bit_pos_to, uint32 * is_set)
{
    uint16 bit_pos = 0;
    uint8 * p_data = NULL;
    uint16 bytes_cnt, bits_cnt;
    const uint8 mask = 0x80;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == is_set)
    {
        return -1;
    }

    if ((p_bitmap->hdr.bit_length < bit_pos_to) || (bit_pos_from >= bit_pos_to))
    {
        return -1;
    }

    /* 2. do it */
    p_data = &(p_bitmap->data[0]);

    for (bit_pos = bit_pos_from; bit_pos <= bit_pos_to; bit_pos++)
    {
        bytes_cnt = ((bit_pos - 1) / 8);
        bits_cnt = ((bit_pos - 1) % 8);
        if (p_data[bytes_cnt] & (mask >> (bits_cnt)))
        {
            *is_set = TRUE;
            return 0;
        }
    }
    *is_set = FALSE;
    return 0;
};

/*********************************************************************
 * Name    : ctclib_bitmap_is_empty
 * Purpose : This function is used to check whethe all the bit is unset
 * Input   : bitmap point address; the number of bit which needs by the bitmap
 * Output  : 
 * Return  : -1:error 1:all the bit is unset 0:one or more bit is set
 * Note    : N/A
*********************************************************************/
int32 
ctclib_bitmap_is_empty(ctclib_bitmap_t * p_bitmap)
{
    uint16 i = 0;
    uint16 bit_cnt = 0;
    uint16 cnt_size = 0;
    uint8 * p_data = NULL;
    
    /* sanity check & init */
    if (NULL == p_bitmap)
    {
        return -1;
    }
  
    bit_cnt = p_bitmap->hdr.bit_length;
    cnt_size = (bit_cnt / 8) + ((bit_cnt % 8) ? 1 : 0);
    p_data = &(p_bitmap->data[0]);

    for(i = 0; i < cnt_size; i++)
    {
        if(p_data[i])
        {
            return 0;           
        };
    };
    return 1;
};

int32 
ctclib_bitmap_get_bit_cnt(ctclib_bitmap_t * p_bitmap, uint16 * p_bit_cnt)
{
    uint16 bit_pos;
    uint16 bit_cnt;
    uint32 is_bit_set;
    uint16 count = 0;
 


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_bit_cnt)
    {
        return -1;
    }
  
    bit_cnt = p_bitmap->hdr.bit_length;

    /* 2. call the trav_func on each valid bit */
    for(bit_pos = 1; bit_pos <= bit_cnt; bit_pos ++)
    {
        ctclib_bitmap_is_bit_set(p_bitmap, bit_pos, &is_bit_set);
        if(is_bit_set)
        {
            count++;            
        };
    };

    *p_bit_cnt = count;
    
    return 0;
};

int32 
ctclib_bitmap_traverse(ctclib_bitmap_t * p_bitmap,
                             uint16 bit_cnt,
                             p_bitmap_trav_t p_trav_func,
                             void * p_data_io)
{
    uint16 bit_pos;
    uint16 actual_bit_cnt;
    uint32 is_bit_set;
    int32 ret = 0;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_trav_func)
    {
        return -1;
    }

    /*use 0 as all bit, add by weij*/
    if (bit_cnt)
    {
        actual_bit_cnt = (bit_cnt <= p_bitmap->hdr.bit_length) ? bit_cnt : p_bitmap->hdr.bit_length;
    }
    else
    {
        actual_bit_cnt = p_bitmap->hdr.bit_length;
    }

    /* 2. call the trav_func on each valid bit */
    for(bit_pos = 1; bit_pos <= actual_bit_cnt; bit_pos ++)
    {
        ctclib_bitmap_is_bit_set(p_bitmap, bit_pos, &is_bit_set);
        if(is_bit_set)
        {
            ret = ((*p_trav_func)(bit_pos, p_data_io));
        };
    };

    return ret;
};

int32 
ctclib_bitmap_traverse_2args(ctclib_bitmap_t * p_bitmap,
                             uint16 bit_cnt,
                             p_bitmap_trav_2args_t p_trav_func,
                             void * p_arg1, void * p_arg2)
{
    uint16 bit_pos;
    uint16 actual_bit_cnt;
    uint32 is_bit_set;
    int32 ret = 0;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_trav_func)
    {
        return -1;
    }

    /*use 0 as all bit, add by weij*/
    if (bit_cnt)
    {
        actual_bit_cnt = (bit_cnt <= p_bitmap->hdr.bit_length) ? bit_cnt : p_bitmap->hdr.bit_length;
    }
    else
    {
        actual_bit_cnt = p_bitmap->hdr.bit_length;
    }

    /* 2. call the trav_func on each valid bit */
    for(bit_pos = 1; bit_pos <= actual_bit_cnt; bit_pos ++)
    {
        ctclib_bitmap_is_bit_set(p_bitmap, bit_pos, &is_bit_set);
        if(is_bit_set)
        {
            ret = ((*p_trav_func)(bit_pos, p_arg1, p_arg2));
        };
    };

    return ret;
};

int32 
ctclib_bitmap_traverse_3args(ctclib_bitmap_t * p_bitmap,
                             uint16 bit_cnt,
                             p_bitmap_trav_3args_t p_trav_func,
                             void * p_arg1, void * p_arg2, void * p_arg3)
{
    uint16 bit_pos;
    uint16 actual_bit_cnt;
    uint32 is_bit_set;
    int32 ret = 0;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_trav_func)
    {
        return -1;
    }

    /*use 0 as all bit, add by weij*/
    if (bit_cnt)
    {
        actual_bit_cnt = (bit_cnt <= p_bitmap->hdr.bit_length) ? bit_cnt : p_bitmap->hdr.bit_length;
    }
    else
    {
        actual_bit_cnt = p_bitmap->hdr.bit_length;
    }

    /* 2. call the trav_func on each valid bit */
    for(bit_pos = 1; bit_pos <= actual_bit_cnt; bit_pos ++)
    {
        ctclib_bitmap_is_bit_set(p_bitmap, bit_pos, &is_bit_set);
        if(is_bit_set)
        {
            ret = ((*p_trav_func)(bit_pos, p_arg1, p_arg2, p_arg3));
        };
    };

    return ret;
};

/*********************************************************************
 * Name    : ctclib_bitmap_create_bmp
 * Purpose : This function is used to create the bit map
 * Input   : bitmap point address; the number of bit which needs by the bitmap
 * Output  : 
 * Return  : 
 * Note    : N/A
*********************************************************************/
int32
ctclib_bitmap_create_bmp(ctclib_bitmap_t **pp_bitmap, uint32 bit_cnt)
{
    uint32 malloc_size = 0;
    ctclib_bitmap_t *p_bitmap = NULL;

    if (NULL == pp_bitmap)
    {
        return -1;
    }
    malloc_size = (bit_cnt / 8) + ((bit_cnt % 8) ? 1 : 0) + sizeof(ctclib_bitmap_hdr_t);
    p_bitmap = (ctclib_bitmap_t*)XMALLOC(MEM_LIB_BITMAP, malloc_size);
    if (!p_bitmap)
    {
        return -1;
    }
    sal_memset(p_bitmap, 0, malloc_size);
    p_bitmap->hdr.bit_length = bit_cnt;
    p_bitmap->hdr.size = malloc_size;
    
    *pp_bitmap = p_bitmap;
    return 0;
}

int32
ctclib_bitmap_destroy_bmp(ctclib_bitmap_t *p_bitmap)
{
    if (NULL == p_bitmap)
    {
        return -1;
    }
    
    XFREE(MEM_LIB_BITMAP, p_bitmap);

    return 0;
}

/*******************************************************************************************
*
********************************************************************************************/
int32
ctclib_bitmap_data_in_uint8_cnt(ctclib_bitmap_t * p_bitmap, uint16 * p_uint8_cnt)
{
    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_uint8_cnt)
    {
        return -1;
    }

    /* 2. do it */
    *p_uint8_cnt = (p_bitmap->hdr.bit_length / 8) + ((p_bitmap->hdr.bit_length % 8) ? 1 : 0);

    return 0;
};

int32
ctclib_bitmap_1st_bit_pos_in_uint8(uint8 data_byte, uint8 * p_1st_bit_pos)
{
    /* 1. sanity check & init */
    if (NULL == p_1st_bit_pos)
    {
        return -1;
    }

    /* 2. do it */
    *p_1st_bit_pos = ctclib_bit_1st_pos_map_1[(data_byte >> 4)][0]
                    | (ctclib_bit_1st_pos_map_1[(data_byte >> 4)][1] & ctclib_bit_1st_pos_map_2[(data_byte & 0x0F)]);


    return 0;
};

int32
ctclib_bitmap_1st_bit_pos_in_uint8_clear(uint8 * p_data_byte, uint8 * p_1st_bit_pos)
{
    uint8 data_hi, data_lo, bit_pos_1st;


    /* 1. sanity check & init */
    if (NULL == p_1st_bit_pos || NULL == p_data_byte)
    {
        return -1;
    }
    
    data_hi = (*p_data_byte >> 4);
    data_lo = (*p_data_byte & 0x0F);


    /* 2. do it */
    bit_pos_1st = ctclib_bit_1st_pos_map_1[data_hi][0]
                | (ctclib_bit_1st_pos_map_1[data_hi][1] & ctclib_bit_1st_pos_map_2[data_lo]);

    *p_data_byte &= ~(0x80 >> (bit_pos_1st - 1));


    /* 3. write back */
    *p_1st_bit_pos = bit_pos_1st;


    return 0;
};


int32
ctclib_bitmap_1st_bit_pos(ctclib_bitmap_t * p_bitmap, uint16 * p_1st_bit_pos)
{
    uint8 cur_1st_bit_pos;
    uint16 data_byte_cnt;
    uint16 i;
    uint8 * p_data_byte;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_1st_bit_pos)
    {
        return -1;
    }
 
    ctclib_bitmap_data_in_uint8_cnt(p_bitmap, &data_byte_cnt);
    p_data_byte = (uint8 *)(&(p_bitmap->data[0]));
    cur_1st_bit_pos = 0;


    /* 2. do it */
    for(i = 0; i < data_byte_cnt; i ++)
    {
        ctclib_bitmap_1st_bit_pos_in_uint8(p_data_byte[i], &cur_1st_bit_pos);
        if(0 != cur_1st_bit_pos)
        {
            break;
        };
    };

    /* 3. write back */
    *p_1st_bit_pos = cur_1st_bit_pos + (i * 8);

    return 0;
};

int32
ctclib_bitmap_1st_bit_pos_clear(ctclib_bitmap_t * p_bitmap, uint16 * p_1st_bit_pos)
{
    uint8 cur_1st_bit_pos;
    uint16 data_byte_cnt;
    uint16 i;
    uint8 * p_data_byte;


    /* 1. sanity check & init */
    if (NULL == p_bitmap || NULL == p_1st_bit_pos)
    {
        return -1;
    }
   
    ctclib_bitmap_data_in_uint8_cnt(p_bitmap, &data_byte_cnt);
    p_data_byte = (uint8 *)(&(p_bitmap->data[0]));
    cur_1st_bit_pos = 0;

    /* 2. do it */
    for(i = 0; i < data_byte_cnt; i ++)
    {
        ctclib_bitmap_1st_bit_pos_in_uint8_clear(&(p_data_byte[i]), &cur_1st_bit_pos);
        if(0 != cur_1st_bit_pos)
        {
            break;
        };
    };


    /* 3. write back */
    *p_1st_bit_pos = cur_1st_bit_pos + (i * 8);


    return 0;
};

