/* Generic vector interface routine
 * Copyright (C) 1997 Kunihiro Ishiguro
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

#include "zebra.h"
#include "vector.h"

/* Initialize vector : allocate memory and return vector. */
vector
vtysh_vector_init (unsigned int size)
{
  vector v = XCALLOC (MTYPE_VECTOR, sizeof (struct _vector));
  if(!v)
  {
    return NULL;
  }

  /* allocate at least one slot */
  if (size == 0)
    size = 1;

  v->alloced = size;
  v->max = 0;
  v->index = XCALLOC (MTYPE_VECTOR, sizeof (void *) * size);
  if(!v->index)
  {
    XFREE(MTYPE_VECTOR, v);
    return NULL;
  }
  memset(v->index, 0, sizeof (void *) * size);
  v->direction = 0;
  v->is_desc = 0;
  v->is_multiple = 0;

  return v;
}

void
vtysh_vector_only_wrapper_free (vector v)
{
  XFREE (MTYPE_VECTOR, v);
}

void
vector_only_index_free (void *index)
{
  XFREE (MTYPE_VECTOR, index);
}

void
vtysh_vector_free (vector v)
{
  XFREE (MTYPE_VECTOR, v->index);
  XFREE (MTYPE_VECTOR, v);
}

vector
vtysh_vector_copy (vector v)
{
  unsigned int size;
  vector new_v = XCALLOC (MTYPE_VECTOR, sizeof (struct _vector));
  if(!new_v)
  {
    return NULL;
  }
  new_v->max = v->max;
  new_v->alloced = v->alloced;
  new_v->direction = v->direction;
  new_v->is_desc = v->is_desc;
  new_v->is_multiple = v->is_multiple;

  size = sizeof (void *) * (v->alloced);
  new_v->index = (void**)XCALLOC (MTYPE_VECTOR, size);
  if(!new_v->index)
  {
    XFREE(MTYPE_VECTOR, new_v);
    return NULL;
  }
  memcpy (new_v->index, v->index, size);

  return new_v;
}

/* Check assigned index, and if it runs short double index pointer */
void
vtysh_vector_ensure (vector v, unsigned int num)
{
  if (v->alloced > num)
    return;

  v->index = XREALLOC (MTYPE_VECTOR, 
		       v->index, sizeof (void *) * (v->alloced * 2));
  memset (&v->index[v->alloced], 0, sizeof (void *) * v->alloced);
  v->alloced *= 2;
  
  if (v->alloced <= num)
    vtysh_vector_ensure (v, num);
}

/* This function only returns next empty slot index.  It dose not mean
   the slot's index memory is assigned, please call vtysh_vector_ensure()
   after calling this function. */
int
vtysh_vector_empty_slot (vector v)
{
  unsigned int i;

  if (v->max == 0)
    return 0;

  for (i = 0; i < v->max; i++)
    if (v->index[i] == 0)
      return i;

  return i;
}

/* Set value to the smallest empty slot. */
int
vtysh_vector_set (vector v, void *val)
{
  unsigned int i;

  i = vtysh_vector_empty_slot (v);
  vtysh_vector_ensure (v, i);

  v->index[i] = val;

  if (v->max <= i)
    v->max = i + 1;

  return i;
}

/* Set value to specified index slot. */
int
vtysh_vector_set_index (vector v, unsigned int i, void *val)
{
  vtysh_vector_ensure (v, i);

  v->index[i] = val;

  if (v->max <= i)
    v->max = i + 1;

  return i;
}

/* Look up vector.  */
void *
vector_lookup (vector v, unsigned int i)
{
  if (i >= v->max)
    return NULL;
  return v->index[i];
}

/* Lookup vector, ensure it. */
void *
vector_lookup_ensure (vector v, unsigned int i)
{
  vtysh_vector_ensure (v, i);
  return v->index[i];
}

/* Unset value at specified index slot. */
void
vtysh_vector_unset (vector v, unsigned int i)
{
  if (i >= v->alloced)
    return;

  v->index[i] = NULL;

  if (i + 1 == v->max) 
    {
      v->max--;
      while (i && v->index[--i] == NULL && v->max--) 
	;				/* Is this ugly ? */
    }
}

/* Count the number of not emplty slot. */
unsigned int
vtysh_vector_count (vector v)
{
  unsigned int i;
  unsigned count = 0;

  for (i = 0; i < v->max; i++) 
    if (v->index[i] != NULL)
      count++;

  return count;
}


/* Add ctc_vector src items to ctc_vector dest.  */
void
vtysh_vector_add (vector dest, vector src)
{
  int i;
  void *val;

  for (i = 0; i < vector_max (src); i++)
    if ((val = vector_slot (src, i)))
      vtysh_vector_set (dest, val);
}

/* Reset dest before vector add.  */
void
vtysh_vector_dup(vector dest, vector src)
{
  vector_reset(dest);
  vtysh_vector_add(dest, src);
}


