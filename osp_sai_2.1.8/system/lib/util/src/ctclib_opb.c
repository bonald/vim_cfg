
#include "sal.h"
#include "glb_macro.h"
#include "ctclib_opb.h"
#include "ctclib_memory.h"

static ctclib_opb_master_t g_ctclib_opb_master;

int32
ctclib_opb_create(ctclib_opb_t* p_opb, uint32 start_offset, uint32 max_size, char *desc)
{
    if (g_ctclib_opb_master.index >= CTCLIB_MAX_OPB)
    {
        return -1;
    }
    
    g_ctclib_opb_master.opb_array[g_ctclib_opb_master.index] = p_opb;
    g_ctclib_opb_master.index++;
    
    p_opb->start_offset = start_offset;
    p_opb->max_size     = max_size;
    p_opb->desc         = strdup(desc);
    p_opb->words = (start_offset + max_size + GLB_UINT32_BITS) / GLB_UINT32_BITS;
    p_opb->data = XCALLOC(MEM_LIB_OPB, sizeof(uint32) * p_opb->words);

    return 0;
}

uint32
ctclib_opb_offset_is_alloced(ctclib_opb_t* p_opb, uint32 offset)
{
    if (offset >= p_opb->start_offset + p_opb->max_size)
    {
        return FALSE;
    }
    
    if (GLB_BMP_ISSET(p_opb->data, offset))
    {
        return TRUE;
    }

    return FALSE;
}

int32
ctclib_opb_alloc_designated_offset(ctclib_opb_t* p_opb, uint32 offset)
{
    if (offset >= p_opb->start_offset + p_opb->max_size)
    {
        return -1;
    }
    
    if (GLB_BMP_ISSET(p_opb->data, offset))
    {
        return -1;
    }

    GLB_BMP_SET(p_opb->data, offset);

    return 0;
}

int32
ctclib_opb_alloc_offset(ctclib_opb_t* p_opb, uint32* offset)
{
    int32 i = 0;
    int32 pos = 0;    
    
    for (i = 0, pos = p_opb->start_offset; i < p_opb->max_size; i++, pos++)
    {
        if (GLB_BMP_ISSET(p_opb->data, pos))
        {
            continue;
        }

        GLB_BMP_SET(p_opb->data, pos);
        *offset = pos;
        return 0;
    }
    
    return -1;
}

int32
ctclib_opb_alloc_offset_position_reverse(ctclib_opb_t* p_opb, uint32 position, uint32* offset)
{
    int32 pos = 0;    

    for (pos = position; pos >= p_opb->start_offset; pos--)
    {
        if (GLB_BMP_ISSET(p_opb->data, pos))
        {
            continue;
        }

        GLB_BMP_SET(p_opb->data, pos);
        *offset = pos;
        return 0;
    }
    
    return -1;
}

int32
ctclib_opb_alloc_offset_last_reverse(ctclib_opb_t* p_opb, uint32* offset)
{
    int32 i = 0;
    int32 pos = 0;    
    
    for (i = 0, pos = p_opb->start_offset+p_opb->max_size-1; i < p_opb->max_size; i++, pos--)
    {
        if (GLB_BMP_ISSET(p_opb->data, pos))
        {
            continue;
        }

        GLB_BMP_SET(p_opb->data, pos);
        *offset = pos;
        return 0;
    }
    
    return -1;
}

int32
ctclib_opb_free_offset(ctclib_opb_t* p_opb, uint32 offset)
{
    if (offset >= p_opb->start_offset + p_opb->max_size)
    {
        return -1;
    }
    
    if (!GLB_BMP_ISSET(p_opb->data, offset))
    {
        return -1;
    }

    GLB_BMP_UNSET(p_opb->data, offset);
    
    return 0;
}

int32
ctclib_bitmap_val2str(FILE *fp, const uint32 *bmp, const uint32 bmp_len)
{
#define CTCLIB_LINE_WIDTH  80

    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    char str[CTCLIB_LINE_WIDTH];
    char tmp[32];
    uint32 used = 0;
    uint32 tmp_len = 0;

    str[0] = 0;
    for (offset = 0; offset < GLB_UINT32_BITS * bmp_len; offset++)
    {
        if (GLB_BMP_ISSET(bmp, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(bmp, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= CTCLIB_LINE_WIDTH)
                {
                    sal_fprintf(fp, "%s\n", str);
                    sal_memset(str, 0, sizeof(str));
                    continue;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 5);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= CTCLIB_LINE_WIDTH)
                {
                    sal_fprintf(fp, "%s\n", str);
                    sal_memset(str, 0, sizeof(str));
                    continue;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 10);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
    }

    if (sal_strlen(str))
    {
        sal_fprintf(fp, "%s\n", str);
        sal_memset(str, 0, sizeof(str));
    }
    
    return 0;
}

int32
ctclib_opb_dump_one(FILE *fp, uint32 i, ctclib_opb_t *opb)
{
    sal_fprintf(fp, "### Index %u, desc %s, range [%u, %u], number %u, words %u ###\n", 
        i, opb->desc, opb->start_offset, (opb->start_offset+opb->max_size-1), opb->max_size, opb->words);
    sal_fprintf(fp, "alloced:\n");
    ctclib_bitmap_val2str(fp, opb->data, opb->words);
    sal_fprintf(fp, "\n");
    
    return 0;    
}

int32
ctclib_opb_dump(FILE *fp)
{
    uint32 i = 0;
    
    for (i = 0; i < g_ctclib_opb_master.index; i++)
    {
        ctclib_opb_dump_one(fp, i, g_ctclib_opb_master.opb_array[i]);
    }
    
    return 0;    
}

int32
ctclib_opb_init()
{
    sal_memset(&g_ctclib_opb_master, 0, sizeof(g_ctclib_opb_master));
    g_ctclib_opb_master.inited = TRUE;

    return 0;
}

