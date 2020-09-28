/****************************************************************************
 * memmgr.h      memory manager defines and macros.
 *
 * Copyright (C) 2011 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Zhu Zhengdong.
 * Date:         2007-3-22.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"

mem_type_name_t mtype_name[MEM_TYPE_MAX] =
{
    {MEM_SYSTEM_MODULE, "MEM_SYSTEM_MODULE"},

    {MEM_FDB_MODULE, "MEM_FDB_MODULE"},

    {MEM_MPLS_MODULE, "MEM_MPLS_MODULE"},

    {MEM_QUEUE_MODULE, "MEM_QUEUE_MODULE"},

    {MEM_IPUC_MODULE, "MEM_IPUC_MODULE"},

    {MEM_IPMC_MODULE, "MEM_IPMC_MODULE"},

    {MEM_RPF_MODULE, "MEM_RPF_MODULE"},

    {MEM_NEXTHOP_MODULE, "MEM_NEXTHOP_MODULE"},

    {MEM_AVL_MODULE, "MEM_AVL_MODULE"},

    {MEM_STATS_MODULE, "MEM_STATS_MODULE"},

    {MEM_L3IF_MODULE, "MEM_L3IF_MODULE"},

    {MEM_PORT_MODULE, "MEM_PORT_MODULE"},

    {MEM_VLAN_MODULE, "MEM_VLAN_MODULE"},

    {MEM_APS_MODULE, "MEM_APS_MODULE"},

    {MEM_VPN_MODULE, "MEM_VPN_MODULE"},

    {MEM_SORT_KEY_MODULE, "MEM_SORT_KEY_MODULE"},

    {MEM_LINKAGG_MODULE, "MEM_LINKAGG_MODULE"},

    {MEM_USRID_MODULE, "MEM_USRID_MODULE"},

    {MEM_SCL_MODULE, "MEM_SCL_MODULE"},

    {MEM_ACL_MODULE, "MEM_ACL_MODULE"},

    {MEM_LINKLIST_MODULE, "MEM_LINKLIST_MODULE"},

    {MEM_CLI_MODULE, "MEM_CLI_MODULE"},

    {MEM_VECTOR_MODULE, "MEM_VECTOR_MODULE"},

    {MEM_HASH_MODULE, "MEM_HASH_MODULE"},

    {MEM_SPOOL_MODULE, "MEM_SPOOL_MODULE"},

    {MEM_OPF_MODULE, "MEM_OPF_MODULE"},

    {MEM_SAL_MODULE, "MEM_SAL_MODULE"},

    {MEM_OAM_MODULE, "MEM_OAM_MODULE"},

    {MEM_PTP_MODULE, "MEM_PTP_MODULE"},

    {MEM_FTM_MODULE, "MEM_FTM_MODULE"},

    {MEM_STK_MODULE, "MEM_STK_MODULE"},

    {MEM_LIBCTCCLI_MODULE, "MEM_LIBCTCCLI_MODULE"},

    {MEM_DMA_MODULE, "MEM_DMA_MODULE"},

    {MEM_STMCTL_MODULE, "MEM_STMCTL_MODULE"},

    {MEM_L3_HASH_MODULE, "MEM_L3_HASH_MODULE"},

    {MEM_FPA_MODULE, "MEM_FPA_MODULE"},

    {MEM_MIRROR_MODULE, "MEM_MIRROR_MODULE"},

    {MEM_SYNC_ETHER_MODULE, "MEM_SYNC_ETHER_MODULE"},

    {MEM_MONITOR_MODULE, "MEM_MONITOR_MODULE "},

    {MEM_OVERLAY_TUNNEL_MODULE, "MEM_OVERLAY_TUNNEL_MODULE "},

    {MEM_EFD_MODULE, "MEM_EFD_MODULE "},

    {MEM_IPFIX_MODULE, "MEM_IPFIX_MODULE"},

    {MEM_TRILL_MODULE, "MEM_TRILL_MODULE"},

    {MEM_FCOE_MODULE, "MEM_FCOE_MODULE"},

    {MEM_WLAN_MODULE, "MEM_WLAN_MODULE"},

    {MEM_DOT1AE_MODULE, "MEM_DOT1AE_MODULE"},

    {MEM_DIAG_MODULE, "MEM_DIAG_MODULE"}

};

#if defined(SDK_IN_KERNEL) || defined(SDK_IN_USERMODE)
bucket_info_t bucket_info[] =
{
    {.block_size = MEM_BLK_4_SIZE, .block_count = MEM_BLK_4_COUNT, .name = "4" },

    {.block_size = MEM_BLK_8_SIZE, .block_count = MEM_BLK_8_COUNT, .name = "8"},

    {.block_size = MEM_BLK_16_SIZE, .block_count = MEM_BLK_16_COUNT, .name = "16"},

    {.block_size = MEM_BLK_32_SIZE, .block_count = MEM_BLK_32_COUNT, .name = "32"},

    {.block_size = MEM_BLK_64_SIZE, .block_count = MEM_BLK_64_COUNT, .name = "64"},

    {.block_size = MEM_BLK_96_SIZE, .block_count = MEM_BLK_96_COUNT, .name = "96"},

    {.block_size = MEM_BLK_128_SIZE, .block_count = MEM_BLK_128_COUNT, .name = "128"},

    {.block_size = MEM_BLK_192_SIZE, .block_count = MEM_BLK_192_COUNT, .name = "192"},

    {.block_size = MEM_BLK_256_SIZE, .block_count = MEM_BLK_256_COUNT, .name = "256"},

    {.block_size = MEM_BLK_384_SIZE, .block_count = MEM_BLK_384_COUNT, .name = "284"},

    {.block_size = MEM_BLK_512_SIZE, .block_count = MEM_BLK_512_COUNT, .name = "512"},

    {.block_size = MEM_BLK_768_SIZE, .block_count = MEM_BLK_768_COUNT, .name = "768"},

    {.block_size = MEM_BLK_1K_SIZE, .block_count = MEM_BLK_1K_COUNT, .name = "1k"},

    {.block_size = MEM_BLK_1536_SIZE, .block_count = MEM_BLK_1536_COUNT, .name = "1536"},

    {.block_size = MEM_BLK_2K_SIZE, .block_count = MEM_BLK_2K_COUNT, .name = "2k"},

    {.block_size = MEM_BLK_4K_SIZE, .block_count = MEM_BLK_4K_COUNT, .name = "4k"},

    {.block_size = MEM_BLK_8K_SIZE, .block_count = MEM_BLK_8K_COUNT, .name = "8k"},

    {.block_size = MEM_BLK_16K_SIZE, .block_count = MEM_BLK_16K_COUNT, .name = "16k"},

    {.block_size = MEM_BLK_32K_SIZE, .block_count = MEM_BLK_32K_COUNT, .name = "32k"},

    {.block_size = MEM_BLK_64K_SIZE, .block_count = MEM_BLK_64K_COUNT, .name = "64k"},

    {.block_size = MEM_BLK_124K_SIZE, .block_count = MEM_BLK_124K_COUNT, .name = "124k"},

    {.block_size = MEM_BLK_INVALID_SIZE},

};

uintptr bucket_ptr_min = 0;
uintptr bucket_ptr_max = 0;

static struct mem_table  mtype_table[MEM_TYPE_MAX];

/******************************************************************************
 * Name        : mem_mgr_get_block_index(uint32 req_size, uint32 *index)
 * Purpose     : get bucket index
 * Input         : required size
 * Output       : bucket index
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
static INLINE int32
mem_mgr_get_block_index(uint32 req_size, uint32* index)
{
    int i = 0;

    if (__builtin_constant_p(req_size))
    {
#define GET_SIZE(x) \
    if (req_size <= x) \
    { \
        goto _found; \
    } \
    else \
    { \
        i++; \
    }

        GET_SIZE(MEM_BLK_4_SIZE)
        GET_SIZE(MEM_BLK_8_SIZE)
        GET_SIZE(MEM_BLK_16_SIZE)
        GET_SIZE(MEM_BLK_32_SIZE)
        GET_SIZE(MEM_BLK_64_SIZE)
        GET_SIZE(MEM_BLK_96_SIZE)
        GET_SIZE(MEM_BLK_128_SIZE)
        GET_SIZE(MEM_BLK_192_SIZE)
        GET_SIZE(MEM_BLK_256_SIZE)
        GET_SIZE(MEM_BLK_384_SIZE)
        GET_SIZE(MEM_BLK_512_SIZE)
        GET_SIZE(MEM_BLK_768_SIZE)
        GET_SIZE(MEM_BLK_1K_SIZE)
        GET_SIZE(MEM_BLK_1536_SIZE)
        GET_SIZE(MEM_BLK_2K_SIZE)
        GET_SIZE(MEM_BLK_4K_SIZE)
        GET_SIZE(MEM_BLK_8K_SIZE)
        GET_SIZE(MEM_BLK_16K_SIZE)
        GET_SIZE(MEM_BLK_32K_SIZE)
        GET_SIZE(MEM_BLK_64K_SIZE)
        GET_SIZE(MEM_BLK_124K_SIZE)
#undef GET_SIZE
    }
    else
    {
        while (req_size > bucket_info[i].block_size)
        {
            i++;
        }
    }

_found:
    if (bucket_info[i].block_size == MEM_BLK_INVALID_SIZE)
    {
        return -1;
    }

    *index = i;
    return 0;
}

/******************************************************************************
 * Name        : mem_mgr_get_overhead_size ()
 * Purpose     : get overhead size
 * Input         : N/A
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC int32
mem_mgr_get_overhead_size()
{
    uint32 size;

    size = sizeof(struct mem_block_header);

#if MEM_RUNTIME_CHECK
    size += sizeof(struct mem_block_trailer);
#endif

    return size;
}

/******************************************************************************
 * Name        : mem_mgr_add_to_mtype_table (uint32 mtype, struct mem_block_header *nmhdr, uint32 req_size)
 * Purpose     : add allocated memory to mtype table
 * Input         : uint32 mtype, struct mem_block_header *nmhdr, uint32 req_size
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC void
mem_mgr_add_to_mtype_table(uint32 mtype, struct mem_block_header* nmhdr, uint32 req_size)
{
    /* update mtype stats */
    mtype_table[mtype].size += (bucket_info[nmhdr->bukt].block_size + mem_mgr_get_overhead_size());
    mtype_table[mtype].count++;

#if MEM_RUNTIME_CHECK
    struct mem_block_header* mhdr;

    if (mtype_table[mtype].list == NULL)
    {
        mtype_table[mtype].list = (void*)nmhdr;
        nmhdr->next = NULL;
        nmhdr->prev = NULL;

    }
    else
    {
        mhdr = (struct mem_block_header*)mtype_table[mtype].list;

        /* add to the head of the list */
        nmhdr->prev = mhdr->prev;
        nmhdr->next = mhdr;

        mhdr->prev = nmhdr;
        mtype_table[mtype].list = nmhdr;
    }

    /* update memory header fields */
    nmhdr->req_size = req_size;

    mtype_table[mtype].req_size += req_size;
#endif
}

/******************************************************************************
 * Name        : mem_mgr_remove_from_mtype_table (int mtype, void *ptr)
 * Purpose     : remove from memory type table
 * Input         : int mtype, void *ptr
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC int32
mem_mgr_remove_from_mtype_table(enum mem_mgr_type mtype, void* ptr)
{
    uint32    size;
    struct mem_block_header* mhdr;

    /* get to the beginning of header */
    mhdr = (struct mem_block_header*)
        ((int8*)ptr - sizeof(struct mem_block_header));

    /* decode header size */
    size = bucket_info[mhdr->bukt].block_size;

    /* update mtype stats */
    mtype_table[mtype].size -= size + mem_mgr_get_overhead_size();
    mtype_table[mtype].count--;

#if MEM_RUNTIME_CHECK
    if (mtype_table[mtype].list == NULL)
    {
        SAL_LOG_INFO("MEMMGR::Can't delete - mtype list is empty\n");
        return -1;
    }

    /* make sure mtype matches before removing it from this list */
    if (mtype != mhdr->mid)
    {
        SAL_LOG_INFO("MEMMGR::Mtype id mismatch - can't delete from mtype list\n");
        return -1;
    }

    /* in case of list having only one memory block */
    if (mhdr->next == NULL && mhdr->prev == NULL)
    {
        mtype_table[mtype].list = NULL;
    }
    /* memory block happens to be the first element in the list */
    else if (mhdr == (struct mem_block_header*)mtype_table[mtype].list)
    {
        mtype_table[mtype].list = mhdr->next;
        mhdr->next->prev = NULL;
    }
    /* remove block from mtype list and update pointers */
    else
    {
        mhdr->prev->next = mhdr->next;
        if (mhdr->next != NULL)
        {
            mhdr->next->prev = mhdr->prev;
        }
    }

    mtype_table[mtype].req_size -= mhdr->req_size;
#endif

    return 0; /* success */
}

/******************************************************************************
 * Name        : mem_get_mtype_name(enum mem_mgr_type mtype)
 * Purpose     : get mem type name
 * Input         : mtype
 * Output       : bucket name
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC int8*
mem_get_mtype_name(enum mem_mgr_type mtype)
{
    int32 incr;

    for (incr = 0; incr < MEM_TYPE_MAX; incr++)
    {
        if (mtype == mtype_name[incr].mtype)
        {
            return mtype_name[incr].name;
        }
    }

    return NULL;
}

#if MEM_RUNTIME_CHECK
/******************************************************************************
 * Name        : mem_mgr_get_total_mtype_req_size
 * Purpose     : get total mtype require size
 * Input         : NULL
 * Output       : N/A
 * Return       : size
 * Note          : N/A
 *****************************************************************************/
#ifndef MEM_CHECK_MEM_SIZE
STATIC uint32
mem_mgr_get_total_mtype_req_size()
{
    uint32 size;
    int32        i;

    size = 0;

    for (i = 0; i < MEM_TYPE_MAX; i++)
    {
        size += mtype_table[i].req_size;
    }

    return size;
}

#endif
/******************************************************************************
 * Name        : mem_mgr_set_memblock_trailer (struct mem_block_header *hdr, uint32 index, int8 *filename, uint32 line)
 * Purpose     : set memory block trailer
 * Input         : struct mem_block_header *hdr, uint32 index, int8 *filename, uint32 line
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC void
mem_mgr_set_memblock_tail(struct mem_block_header* hdr, uint32 index, char* filename, uint16 line)
{
    struct mem_block_trailer* mtlr;


    mtlr = (struct mem_block_trailer*)
        ((int8*)hdr + sizeof(struct mem_block_header) + bucket_info[index].block_size);

    /* get filename and its length */
    if (filename != NULL)
    {
        uint32 len = sal_strlen((char*)filename);

        /* truncate filename if it exceeds MAX file size */
        if (len >= MAX_MEM_FILE_SZ)
        {
            len = len - MAX_MEM_FILE_SZ + 1;
            filename = &filename[len];
            len = MAX_MEM_FILE_SZ - 1;
        }

        sal_strcpy((char*)mtlr->filename, (char*)filename);
        mtlr->filename[len] = '\0';
    }

    mtlr->line_number = line;

    /* set up pre guard area */
    hdr->guard = MEM_HEADER_MAGIC_NUMBER;

    /* set up post guard area */
    mtlr->guard = MEM_TAIL_MAGIC_NUMBER;

    if (hdr->req_size <  bucket_info[index].block_size && ((bucket_info[index].block_size - hdr->req_size) >= sizeof(uint32)))
    {
        uint32 *p_val = NULL;
        uint32 len = 0;
        len = sizeof(struct mem_block_header) +  hdr->req_size;
        len = (len%4 == 0)?(len / 4):(len / 4 + 1);
        p_val = (uint32 *)hdr;
        p_val[len] = MEM_TAIL_MAGIC_NUMBER;
    }

}


bool
mem_mgr_ptr_is_exsit(void* ptr)
{

    if (bucket_ptr_min <= (uintptr)ptr &&  (uintptr)ptr <= bucket_ptr_max)
    {
        return 1;
    }

    return 0;

}

/******************************************************************************
 * Name        : mem_mgr_ptr_check (void *ptr)
 * Purpose     : check if buffer has error.
 * Input         : void *ptr
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
int32
mem_mgr_ptr_check(void* ptr)
{
    struct mem_block_header* mhdr;
    struct mem_block_trailer* mtlr;


    /* check ptr exsit!! */
    if (!mem_mgr_ptr_is_exsit(ptr))
    {
        return 0;
    }

    mhdr = (struct mem_block_header*)((int8*)ptr - sizeof(struct mem_block_header));

    if (mhdr->guard != MEM_HEADER_MAGIC_NUMBER)
    {
        return 0;
    }

    mtlr = (struct mem_block_trailer*)((int8*)ptr + bucket_info[mhdr->bukt].block_size);

    /* get user buffer size */
    if (mtlr->guard != MEM_TAIL_MAGIC_NUMBER)
    {
       /* sal_printf("MEMMGR::Post guard check failed  - hdr %p - mtype %d - file %s - line %d\n",
                     mhdr, mhdr->mid, mtlr->filename, mtlr->line_number);*/
        return -1;
    }


    if (mhdr->req_size <  bucket_info[mhdr->bukt].block_size && ((bucket_info[mhdr->bukt].block_size - mhdr->req_size) >= sizeof(uint32)))
    {
        uint32 *p_val = NULL;
        uint32 len = 0;
        len = sizeof(struct mem_block_header) +  mhdr->req_size;
        len = (len%4 == 0)?(len / 4):(len / 4 + 1);
        p_val = (uint32 *)mhdr;

        if (p_val[len] != MEM_TAIL_MAGIC_NUMBER)
        {
             /* sal_printf("MEMMGR::Post guard check failed  - hdr %p - mtype %d - file %s - line %d\n",
                         mhdr, mhdr->mid, mtlr->filename, mtlr->line_number);*/
                 return -1;
        }

    }
    return 0;

}


/******************************************************************************
 * Name        : mem_mgr_buffer_check (void *ptr)
 * Purpose     : check if buffer has error.
 * Input         : void *ptr
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC void
mem_mgr_buffer_check(void* ptr)
{
    struct mem_block_header* mhdr;
    struct mem_block_trailer* mtlr;

    mhdr = (struct mem_block_header*)((int8*)ptr - sizeof(struct mem_block_header));
    mtlr = (struct mem_block_trailer*)((int8*)ptr + bucket_info[mhdr->bukt].block_size);

    /* see if any of the pre guard area is corrupted */
    if (mhdr->guard != MEM_HEADER_MAGIC_NUMBER)
    {
        SAL_LOG_INFO("MEMMGR::re guard check failed  - hdr %p - mtype %d - file %s - line %d\n",
                     mhdr, mhdr->mid, mtlr->filename, mtlr->line_number);
    }

    /* get user buffer size */
    if (mtlr->guard != MEM_TAIL_MAGIC_NUMBER)
    {
        SAL_LOG_INFO("MEMMGR::Post guard check failed  - hdr %p - mtype %d - file %s - line %d\n",
                     mhdr, mhdr->mid, mtlr->filename, mtlr->line_number);
    }
}

#endif
/******************************************************************************
 * Name        : mem_mgr_check_mtype(enum mem_mgr_type mtype, bool is_show_detail)
 * Purpose     : check allocated block, show information
 * Input         : memory type, is_show_detail
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
void
mem_mgr_check_mtype(enum mem_mgr_type mtype, bool is_show_detail)
{
    int8* m_name;

    if (mtype < 0 || mtype >= MEM_TYPE_MAX)
    {
        SAL_LOG_INFO("\nMEMMGR::Unknown memory type specified (%d)\n", mtype);
        return;
    }

    m_name = mem_get_mtype_name(mtype);
    if (NULL == m_name)
    {
        return;
    }

#if MEM_RUNTIME_CHECK
    struct mem_block_header* hdr;
    struct mem_block_trailer* mtlr;
    uint32 count, block_size;
    int8* ptr;

    ptr = NULL;
    mtlr = NULL;
    block_size = 0;

    SAL_LOG_INFO("MEMMGR:: mtype %s, req_size %d, total size %d (debug used:%d), block_count %d\n",
                 m_name, mtype_table[mtype].req_size, mtype_table[mtype].size,
                 (mtype_table[mtype].count * mem_mgr_get_overhead_size()),
                 mtype_table[mtype].count);

    hdr = (struct mem_block_header*)mtype_table[mtype].list;
    count = mtype_table[mtype].count;

    while (count > 0 && hdr != NULL)
    {
        ptr = (int8*)hdr + sizeof(struct mem_block_header);
        mem_mgr_buffer_check(ptr);
        block_size = bucket_info[hdr->bukt].block_size;

        mtlr = (struct mem_block_trailer*)(ptr + block_size);
        if (is_show_detail)
        {
            SAL_LOG_INFO("MEMMGR::req_size %d - real_size %d - file %s - line %d\n",
                         hdr->req_size, block_size, mtlr->filename, mtlr->line_number);
        }

        hdr = hdr->next;
        count--;
    }

    if (count != 0)
    {
        SAL_LOG_INFO("MEMMGR::Mismatch of count and # of elements in mtype list\n");
    }

#else
    SAL_LOG_INFO("MEMMGR:: mtype %s, total size %d (debug used:%d), block_count %d\n",
                 m_name, mtype_table[mtype].size,
                 (mtype_table[mtype].count * mem_mgr_get_overhead_size()), mtype_table[mtype].count);
#endif
}

int32
mem_mgr_get_mtype_info(enum mem_mgr_type mtype, mem_table_t* p_mtype_table_info, int8** m_name)
{
    *m_name = mem_get_mtype_name(mtype);
    if (NULL == *m_name)
    {
        return -1;
    }

    if (mtype < 0 || mtype >= MEM_TYPE_MAX)
    {
        SAL_LOG_INFO("\nMEMMGR::Unknown memory type specified (%d)\n", mtype);
        return -1;
    }

    p_mtype_table_info->req_size = mtype_table[mtype].req_size;
    p_mtype_table_info->size = mtype_table[mtype].size;
    p_mtype_table_info->count = mtype_table[mtype].count;

    return 0;

}

/******************************************************************************
 * Name        : mem_show_bucket_used_count(uint32 mtype)
 * Purpose     : show bucket used count
 * Input         : N/A
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
void
mem_show_bucket_used_count()
{
    int32 incr = 0;

    SAL_LOG_INFO("MEMMGR::Memory bucket used counter: \n");
    SAL_LOG_INFO("%-24s %-16s\n", "bucket size(bytes)", "used_count(entries)");

    while (bucket_info[incr].block_size != MEM_BLK_INVALID_SIZE)
    {
        SAL_LOG_INFO("%-24s %-16d \n", bucket_info[incr].name, bucket_info[incr].used_count);
        incr++;
    }
}


int32
mem_get_bucket_used_info(bucket_info_t* bucket_used_info, uint8 index)
{

    if (bucket_info[index].block_size != MEM_BLK_INVALID_SIZE)
    {
        sal_memcpy(bucket_used_info->name, bucket_info[index].name, sizeof(int8)*20);
        bucket_used_info->block_count= bucket_info[index].block_count;
        bucket_used_info->used_count = bucket_info[index].used_count;
        bucket_used_info->block_size = bucket_info[index].block_size;
        return 0;
    }
    else
    {
        return -1;
    }
}
#endif

#ifdef SDK_IN_KERNEL
/******************************************************************************
 * Name        : mem_mgr_add_to_free_table (void *ptr)
 * Purpose     : add ptr to free table
 * Input         : *ptr
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC void
mem_mgr_free_to_mem_pool(void* ptr)
{
    struct mem_block_header* hdr;
    uint32 bucket;

    hdr = (struct mem_block_header*)
        ((int8*)ptr - sizeof(struct mem_block_header));
    bucket = hdr->bukt;

    SAL_MEM_POOL_FREE(bucket_info[hdr->bukt].mem_pool, hdr);

    bucket_info[bucket].used_count--;
}

/******************************************************************************
 * Name        : mem_mgr_get_user_membuf (int req_size, int mtype, int8 *filename, int line)
 * Purpose     : Get user memory buffer.
 * Input         : int req_size, int mtype, int8 *filename, int line
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
STATIC int8*
mem_mgr_get_user_membuf(uint32 req_size, uint32 mtype, char* filename, uint16 line)
{
    int8* ptr;
    struct    mem_block_header* mhdr;
    uint32       index;
    uint32 allocated_size;
    int32 ret;

    allocated_size = 0;
    /* get bucket index */
    ret = mem_mgr_get_block_index(req_size, &index);

    if (ret < 0)
    {
        SAL_LOG_INFO("MEMMGR::%d Too large: memory size of more than 124k.(%s, line:%d) \n", req_size, filename, line);
        return NULL;
    }

#if MEM_RUNTIME_CHECK
#ifndef MEM_CHECK_MEM_SIZE
    /* Check for maximum allowed memory only if it is not set to
     * infinite
     */
    allocated_size = mem_mgr_get_total_mtype_req_size() + req_size;
    if (allocated_size > MAX_MEM_SIZE)
    {
        SAL_LOG_INFO("MEMMGR::Maximum memory threshold %u KB reached\n", (MAX_MEM_SIZE / 1000));
        return NULL;
    }

#endif
#endif

    SAL_MEM_POOL_ALLOC(mhdr, struct mem_block_header*, bucket_info[index].mem_pool, FALSE);
    if (NULL == mhdr)
    {
        return NULL;
    }

    mhdr->bukt = index;
    mhdr->mid = mtype;
    mhdr->rsv = 0xFFFF;
    ptr = (int8*)mhdr + sizeof(struct mem_block_header);

    /* add this block to mtype table */
    MEMMGR_LOCK(mtype_table[mtype].p_mem_mutex);
    mem_mgr_add_to_mtype_table(mtype, mhdr, req_size);
    MEMMGR_UNLOCK(mtype_table[mtype].p_mem_mutex);

#if MEM_RUNTIME_CHECK
    /* init gaurd area and copy file and line information */
    mem_mgr_set_memblock_tail(mhdr, index, filename, line);
#endif
    bucket_info[index].used_count++;

    return ptr;
}

/******************************************************************************
 * Name        : mem_mgr_init(void)
 * Purpose     : init memory manage
 * Input         : NULL
 * Output       : NULL
 * Return       : TRUE = SUCCESS
 *                    FALSE   = FAIL
 * Note          : N/A
 *****************************************************************************/

int32
mem_mgr_init(void)
{
#ifdef MEMMGR
    int32  index;
    int32  ret;
    uint32 block_size;

    /* init memory table */
    sal_memset(mtype_table,  0,  sizeof(struct mem_table) * MEM_TYPE_MAX);

#if MEM_RUNTIME_CHECK

    for (index = 0; index < MEM_TYPE_MAX; index++)
    {
        mtype_table[index].list  = NULL;

        ret = sal_mutex_create(&(mtype_table[index].p_mem_mutex));

        if ((ret != 0) || (NULL == mtype_table[index].p_mem_mutex))
        {
            return -9982;
        }
    }

#endif

    index = 0;

    while (bucket_info[index].block_size != MEM_BLK_INVALID_SIZE)
    {
        bucket_info[index].mem_pool = NULL;
        block_size = bucket_info[index].block_size + mem_mgr_get_overhead_size();
        ret = ctc_sal_mem_pool_create(&bucket_info[index].mem_pool, bucket_info[index].name, block_size,
                                  4, bucket_info[index].block_count);
        if (ret < 0)
        {
            return ret;
        }

        bucket_info[index].used_count = 0;
        index++;
    }
#endif

    return 0;
}

/******************************************************************************
 * Name        : memmgr_init_memtable(void)
 * Purpose     : init memory manage
 * Input         : NULL
 * Output       : NULL
 * Return       : TRUE = SUCCESS
 *                    FALSE   = FAIL
 * Note          : N/A
 *****************************************************************************/

int32
mem_mgr_deinit(void)
{
    int32 index;

    index = 0;

    while (bucket_info[index].block_size != MEM_BLK_INVALID_SIZE)
    {
        if (bucket_info[index].mem_pool)
        {
            ctc_sal_mem_pool_destroy(bucket_info[index].mem_pool);
        }

        bucket_info[index].mem_pool = NULL;
        index++;
    }

#if MEM_RUNTIME_CHECK

    for (index = 0; index < MEM_TYPE_MAX; index++)
    {
        sal_mutex_destroy(mtype_table[index].p_mem_mutex);
    }

#endif

    SAL_LOG_INFO("Memory release OK\n");

    return 0;
}

/******************************************************************************
 * Name        : mem_mgr_free (int mtype, void *ptr, int8 *file, int line)
 * Purpose     : free memory
 * Input         : int mtype, void *ptr, int8 *file, int line
 * Output       : N/A
 * Return       : N/A
 * Note          : N/A
 *****************************************************************************/
void
mem_mgr_free(void* ptr, char* file, int line)
{
    int32 ret = 0;
    enum mem_mgr_type mtype;
    struct mem_block_header* mhdr;

    mhdr = NULL;
    mtype = MEM_TYPE_MAX;

    if (NULL == ptr)
    {
        return;
    }

#if MEM_RUNTIME_CHECK
    mem_mgr_buffer_check(ptr);
#endif

    mhdr = (struct mem_block_header*)((int8*)ptr - sizeof(struct mem_block_header));
    mtype = mhdr->mid;
    if ((mtype < 0) || (mtype >= MEM_TYPE_MAX))
    {
        SAL_LOG_INFO("MEMMGR::Unknown memory type specified\n");
        return;
    }
    MEMMGR_LOCK(mtype_table[mtype].p_mem_mutex);
    ret = mem_mgr_remove_from_mtype_table(mtype, ptr);
    MEMMGR_UNLOCK(mtype_table[mtype].p_mem_mutex);
    if (ret != 0)
    {
        return;
    }

    mem_mgr_free_to_mem_pool(ptr);
}

/******************************************************************************
 * Name        : mem_mgr_malloc (uint32 size, uint32 mtype, char *filename, uint32 line)
 * Purpose     : malloc memory
 * Input         : uint32 size, uint32 mtype, int8 *filename, uint32 line
 * Output       : N/A
 * Return       : void *ptr
 * Note          : N/A
 *****************************************************************************/
void*
mem_mgr_malloc(uint32 size, enum mem_mgr_type mtype, char* filename, uint16 line)
{
    int8* bufptr;

    /* do parameter check */
    if (size == 0)
    {
        return NULL;
    }

    if ((mtype < 0) || (mtype >= MEM_TYPE_MAX))
    {
        SAL_LOG_INFO("MEMMGR::Unknown memory type specified\n");
        return NULL;
    }

    bufptr = mem_mgr_get_user_membuf(size, mtype, filename, line);

    return (void*)bufptr;
}

/******************************************************************************
 * Name        : mem_mgr_realloc (void *ptr, uint32 size, uint32 mtype,
                                            char *filename, uint32 line)
 * Purpose     : realloc memory
 * Input         : void *ptr,uint32 size, uint32 mtype, int8 *filename, uint32 line
 * Output       : N/A
 * Return       : void *ptr
 * Note          : N/A
 *****************************************************************************/
void*
mem_mgr_realloc(uint32 mtype, void* ptr, uint32 size, char* filename, uint16 line)
{
    int8* new_ptr = NULL;
    uint32 index = 0;
    int32 ret = 0;
    struct mem_block_header* mhdr = NULL;

    /* do parameter check */
    if (size == 0)
    {
        return NULL;
    }

    if (NULL == ptr)
    {
        return NULL;
    }

    /*get mem block header */
    mhdr = (struct mem_block_header*)((int8*)ptr - sizeof(struct mem_block_header));

    ret = mem_mgr_get_block_index(size, &index);

    if (bucket_info[mhdr->bukt].block_size == bucket_info[index].block_size)
    {
        new_ptr = ptr;
    }
    else if (bucket_info[mhdr->bukt].block_size > bucket_info[index].block_size)
    {
        new_ptr = mem_mgr_malloc(size, mtype, filename, line);
        if (NULL == new_ptr)
        {
            return NULL;
        }
        sal_memmove(new_ptr, ptr, bucket_info[index].block_size);
        mem_mgr_free(ptr, filename, line);
    }
    else
    {
        new_ptr = mem_mgr_malloc(size, mtype, filename, line);
        if (NULL == new_ptr)
        {
            return NULL;
        }
        sal_memmove(new_ptr, ptr, bucket_info[mhdr->bukt].block_size);
        mem_mgr_free(ptr, filename, line);
    }

    return new_ptr;
}

#elif defined(SDK_IN_USERMODE)

int32
mem_mgr_init(void)
{
    int32  index = 0;
    int32  ret;
    uint32 block_size;

    /* init memory table */
    sal_memset(mtype_table,  0,  sizeof(struct mem_table) * MEM_TYPE_MAX);

    for (index = 0; index < MEM_TYPE_MAX; index++)
    {
        mtype_table[index].list  = NULL;

        ret = sal_mutex_create(&(mtype_table[index].p_mem_mutex));

        if ((ret != 0) || (NULL == mtype_table[index].p_mem_mutex))
        {
            return -1;
        }
    }


    mem_cache_init();

    index = 0;

    while (bucket_info[index].block_size != MEM_BLK_INVALID_SIZE)
    {
        block_size = bucket_info[index].block_size + mem_mgr_get_overhead_size();
        bucket_info[index].cache = mem_cache_create(block_size);
        if (!bucket_info[index].cache)
        {
            return -1;
        }

        ret = sal_mutex_create(&(bucket_info[index].cache_mutex));

        if ((ret != 0) || (NULL == bucket_info[index].cache_mutex))
        {
            return -1;
        }

        bucket_info[index].used_count = 0;
        index++;
    }

    return 0;
}

int32
mem_mgr_deinit(void)
{
    int32 index = 0;

    while (bucket_info[index].block_size != MEM_BLK_INVALID_SIZE)
    {
        if (bucket_info[index].cache)
        {
            mem_cache_destroy(bucket_info[index].cache);
            bucket_info[index].cache = NULL;
        }

        if (bucket_info[index].cache_mutex)
        {
            sal_mutex_destroy(bucket_info[index].cache_mutex);
            bucket_info[index].cache_mutex = NULL;
        }

        index++;
    }

#if MEM_RUNTIME_CHECK

    for (index = 0; index < MEM_TYPE_MAX; index++)
    {
        sal_mutex_destroy(mtype_table[index].p_mem_mutex);
    }

#endif

    return 0;
}

void*
mem_mgr_malloc(uint32 size, enum mem_mgr_type mtype, char* filename, uint16 line)
{
    int8* bufptr;
    int32 ret;
    uint32 index;
    struct    mem_block_header* mhdr;

    /* do parameter check */
    if (size == 0)
    {
        return NULL;
    }

    if ((mtype < 0) && (mtype >= MEM_TYPE_MAX))
    {
        SAL_LOG_INFO("MEMMGR::Unknown memory type specified\n");
        return NULL;
    }

    ret = mem_mgr_get_block_index(size, &index);
    if (ret < 0)
    {
        /*SAL_LOG_INFO("MEMMGR::%d Too large: memory size of more than 124k.(%s, line:%d) \n",size, filename, line);*/
        /*For memory size of more than 124k, use sal_malloc*/
        mhdr = (struct mem_block_header*)sal_malloc(size + sizeof(struct mem_block_header));
        if (!mhdr)
        {
            return NULL;
        }

        mhdr->bukt = 0xFF;
        mhdr->mid = mtype;
        mhdr->rsv = 0xFFFF;
        bufptr = (int8*)mhdr + sizeof(struct mem_block_header);
        return bufptr;
    }

    MEMMGR_LOCK(bucket_info[index].cache_mutex);
    mhdr = (struct mem_block_header*)mem_cache_alloc(bucket_info[index].cache);
    if (!mhdr)
    {
        MEMMGR_UNLOCK(bucket_info[index].cache_mutex);
        return NULL;
    }

    bucket_info[index].used_count++;
    MEMMGR_UNLOCK(bucket_info[index].cache_mutex);

    mhdr->bukt = index;
    mhdr->mid = mtype;
    mhdr->rsv = 0xFFFF;
    bufptr = (int8*)mhdr + sizeof(struct mem_block_header);

    /* add this block to mtype table */
    MEMMGR_LOCK(mtype_table[mtype].p_mem_mutex);
    mem_mgr_add_to_mtype_table(mtype, mhdr, size);
    MEMMGR_UNLOCK(mtype_table[mtype].p_mem_mutex);

#if MEM_RUNTIME_CHECK
    /* init gaurd area and copy file and line information */
    mem_mgr_set_memblock_tail(mhdr, index, filename, line);

    if (bucket_ptr_min == 0)
    {
        bucket_ptr_min = (uintptr)mhdr;
        bucket_ptr_max = (uintptr)mhdr;
    }
    else
    {
        if ((uintptr)mhdr < bucket_ptr_min)
        {
            bucket_ptr_min = (uintptr)mhdr;
        }

        if ((uintptr)mhdr > bucket_ptr_max)
        {
            bucket_ptr_max = (uintptr)mhdr;
        }
    }

#endif


    return (void*)bufptr;
}

void
mem_mgr_free(void* ptr, char* file, int line)
{
    uint32 index;
    int32 ret;
    enum mem_mgr_type mtype;
    struct mem_block_header* mhdr;

    if (NULL == ptr)
    {
        return;
    }

    mhdr = (struct mem_block_header*)((int8*)ptr - sizeof(struct mem_block_header));
    index = mhdr->bukt;
    if (index == 0xFF)
    {
        /*For memory size of more than 124k, use sal_free*/
        sal_free(mhdr);
        return;
    }

#if MEM_RUNTIME_CHECK
    mem_mgr_buffer_check(ptr);
#endif

    mtype = mhdr->mid;
    if ((mtype < 0) || (mtype >= MEM_TYPE_MAX))
    {
        SAL_LOG_INFO("MEMMGR::Unknown memory type specified\n");
        return;
    }
    MEMMGR_LOCK(mtype_table[mtype].p_mem_mutex);
    ret = mem_mgr_remove_from_mtype_table(mtype, ptr);
    MEMMGR_UNLOCK(mtype_table[mtype].p_mem_mutex);
    if (ret != 0)
    {
        return;
    }

    MEMMGR_LOCK(bucket_info[index].cache_mutex);
    mem_cache_free(mhdr);
    bucket_info[index].used_count--;
    MEMMGR_UNLOCK(bucket_info[index].cache_mutex);

}

void*
mem_mgr_realloc(uint32 mtype, void* ptr, uint32 size, char* filename, uint16 line)
{
    int8* new_ptr = NULL;
    uint32 index = 0;
    int32 ret = 0;
    struct mem_block_header* mhdr = NULL;

    /* do parameter check */
    if (size == 0)
    {
        return NULL;
    }

    if (NULL == ptr)
    {
        return NULL;
    }

    /*get mem block header */
    mhdr = (struct mem_block_header*)((int8*)ptr - sizeof(struct mem_block_header));

    ret = mem_mgr_get_block_index(size, &index);
    if (ret < 0)
    {
        return sal_realloc(ptr, size);
    }
    else
    {
        if (bucket_info[mhdr->bukt].block_size == bucket_info[index].block_size)
        {
            new_ptr = ptr;
        }
        else if (bucket_info[mhdr->bukt].block_size > bucket_info[index].block_size)
        {
            new_ptr = mem_mgr_malloc(size, mtype, filename, line);
            if (NULL == new_ptr)
            {
                return NULL;
            }
            sal_memmove(new_ptr, ptr, bucket_info[index].block_size);
            mem_mgr_free(ptr, filename, line);
        }
        else
        {
            new_ptr = mem_mgr_malloc(size, mtype, filename, line);
            if (NULL == new_ptr)
            {
                return NULL;
            }
            sal_memmove(new_ptr, ptr, bucket_info[mhdr->bukt].block_size);
            mem_mgr_free(ptr, filename, line);
        }
    }

    return new_ptr;
}

#elif defined(SDK_IN_VXWORKS)
int32
mem_mgr_init(void)
{
    return 0;
}

int32
mem_mgr_deinit(void)
{
    return 0;
}

void*
mem_mgr_malloc(uint32 size, enum mem_mgr_type mtype, char* filename, uint16 line)
{
    /* do parameter check */
    if (size == 0)
    {
        return NULL;
    }
    return malloc(size);
}

void*
mem_mgr_realloc(uint32 mtype, void* ptr, uint32 size, char* filename, uint16 line)
{
    return realloc(ptr, size);
}

void
mem_mgr_free(void* ptr, char* file, int line)
{
    free(ptr);
}

void
mem_mgr_check_mtype(enum mem_mgr_type mtype, bool is_show_detail)
{
    return;
}

void
mem_show_bucket_used_count()
{
    return;
}

#endif

