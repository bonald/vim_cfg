/****************************************************************************
 * ctc_vti_vec.h :         header
 *
 * Copyright (C) 2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         V1.0
 * Date           :         2010-7-28
 * Reason         :         First Create
 ****************************************************************************/

#ifndef _CTC_VTI_VEC_H
#define _CTC_VTI_VEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"

/* struct for vector */
struct _vector
{
    uint32 max;                 /* max number of used slot */
    uint32 alloced;             /* number of allocated slot */
    void** index;               /* index to data */
    unsigned char direction;    /* 0 for transverse, 1for vertical*/
    unsigned char is_desc;      /* 0 for vector pointer, 1 for desc pointer */
    unsigned char is_multiple;  /* 0 for single selection (), 1 for multiple selection {} */
    unsigned char is_option;     /* 0 for cannot skip, 1 for can skip */
};
typedef struct _vector* vector;

#define VECTOR_MIN_SIZE 1

/* (Sometimes) usefull macros.  This macro convert index expression to
 array expression. */
#define vector_slot(V, I)  ((V)->index[(I)])
#define vector_max(V)     ((V)->max)
#define vector_reset(V)   ((V)->max = 0)

/* Prototypes. */
vector ctc_vti_vec_init(uint32 size);
void ctc_vti_vec_ensure(vector v, uint32 num);
int ctc_vti_vec_empty_slot(vector v);
int ctc_vti_vec_set(vector v, void* val);
int ctc_vti_vec_set_index(vector v, uint32 i, void* val);
void ctc_vti_vec_unset(vector v, uint32 i);
int ctc_vti_vec_unset_val(vector v, void* val);
uint32 ctc_vti_vec_count(vector v);
void ctc_vti_vec_only_wrapper_free(vector v);
void ctc_vti_vec_only_index_free(void* index);
void ctc_vti_vec_free(vector v);
vector ctc_vti_vec_copy(vector v);

void* ctc_vti_vec_lookup(vector, uint32);
void* ctc_vti_vec_lookup_ensure(vector, uint32);
void ctc_vti_vec_add(vector dest, vector src);
void ctc_vti_vec_dup(vector dest, vector src);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_VTI_VEC_H */

