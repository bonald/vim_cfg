/*
 * Generic vector interface header.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _ZEBRA_VECTOR_H
#define _ZEBRA_VECTOR_H

/* struct for vector */
struct _vector 
{
    unsigned int max;		/* max number of used slot */
    unsigned int alloced;		/* number of allocated slot */
    void **index;			/* index to data */
    unsigned char direction;     /* 0 for transverse, 1for vertical*/
    unsigned char is_desc;       /* 0 for vector pointer, 1 for desc pointer */
    unsigned char is_multiple; /* 0 for single selection (), 1 for multiple selection {} */
};
typedef struct _vector *vector;

#define VECTOR_MIN_SIZE 1

/* (Sometimes) usefull macros.  This macro convert index expression to
 array expression. */
#define vector_slot(V,I)  ((V)->index[(I)])
#define vector_max(V)     ((V)->max)
#define vector_reset(V)   ((V)->max = 0)

/* Prototypes. */
vector vtysh_vector_init (unsigned int size);
void vtysh_vector_ensure (vector v, unsigned int num);
int vtysh_vector_empty_slot (vector v);
int vtysh_vector_set (vector v, void *val);
int vtysh_vector_set_index (vector v, unsigned int i, void *val);
void vtysh_vector_unset (vector v, unsigned int i);
unsigned int vtysh_vector_count (vector v);
void vtysh_vector_only_wrapper_free (vector v);
void vector_only_index_free (void *index);
void vtysh_vector_free (vector v);
vector vtysh_vector_copy (vector v);

void *vector_lookup (vector, unsigned int);
void *vector_lookup_ensure (vector, unsigned int);
void vtysh_vector_add (vector dest, vector src);
void vtysh_vector_dup(vector dest, vector src);

#endif /* _ZEBRA_VECTOR_H */
