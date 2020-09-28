/*
 * Copyright (c) 2003, 2004 X/IO Labs, xiolabs.com.
 * Copyright (c) 2003, 2004, 2005 Lev Walkin <vlm@lionet.info>.
 * 	All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#include <asn_internal.h>

#ifdef _ASN_MEMORY_STATS_
#define ASN1_LOCATION_CNT 1
uint32_t g_asn1_location = 0;
static uint32_t g_asn1_cnt_calloc[ASN1_LOCATION_CNT];
static uint32_t g_asn1_cnt_malloc[ASN1_LOCATION_CNT];
static uint32_t g_asn1_cnt_realloc[ASN1_LOCATION_CNT];
static uint32_t g_asn1_cnt_free[ASN1_LOCATION_CNT];

void*
CALLOC(size_t nmemb, size_t size)
{
    g_asn1_cnt_calloc[g_asn1_location]++;
    return calloc(nmemb, size);
}

void*
MALLOC(size_t size)
{
    g_asn1_cnt_malloc[g_asn1_location]++;
    return malloc(size);
}

void*
REALLOC(void *oldptr, size_t size)
{
    if (NULL == oldptr)
    {
        g_asn1_cnt_calloc[g_asn1_location]++;      
    }
    g_asn1_cnt_realloc[g_asn1_location]++;
    return realloc(oldptr, size);
}

void
FREEMEM(void *ptr)
{
    g_asn1_cnt_free[g_asn1_location]++;
    return free(ptr);
}
#endif /* _ASN_MEMORY_STATS_ */

void
asn_dump_memory(FILE *fp)
{
    fprintf(fp, "\nASN.1 memory statistics\n");
#ifdef _ASN_MEMORY_STATS_
    int i = 0;
    for (i = 0; i < ASN1_LOCATION_CNT; i++)
    {
        fprintf(fp, "####### location %d #######\n", i);
        fprintf(fp, "g_asn1_cnt_calloc:     %d\n", g_asn1_cnt_calloc[i]);
        fprintf(fp, "g_asn1_cnt_malloc:     %d\n", g_asn1_cnt_malloc[i]);
        fprintf(fp, "g_asn1_cnt_realloc:    %d\n", g_asn1_cnt_realloc[i]);
        fprintf(fp, "g_asn1_cnt_free:       %d\n", g_asn1_cnt_free[i]);
    }
#else
    fprintf(fp, "Unsupport\n");
#endif /* _ASN_MEMORY_STATS_ */
}

