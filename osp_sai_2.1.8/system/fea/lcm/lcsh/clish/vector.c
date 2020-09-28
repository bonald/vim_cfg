
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

#define MTYPE_VECTOR 0
#define MTYPE_VECTOR_INDEX 0

#define XMALLOC(type,size)      malloc(size)
#define XCALLOC(type,size)      calloc(1, size)
#define XFREE(type,ptr)         free(ptr)
#define XREALLOC(type,ptr,size) realloc(ptr, size)
#define XSTRDUP(type,str)       strdup(str)

/* Initialize vector : allocate memory and return vector. */
vector
vector_init (u_int32_t size)
{

  int slot = 0, i = 0, j = 0;
  
  vector v = XMALLOC (MTYPE_VECTOR, sizeof (struct _vector));

  if (!v)
    return v;

  /* allocate at least one slot */
  if (size == 0)
    size = 1;

  memset(v->index, 0, sizeof(void*)*(MAX_VECTOR_SLOT_NUM));
  slot = (size-1)/(MAX_VECTOR_SLOT_INDEX) + 1;
  v->alloced = size;
  v->max = 0;
  v->count = 0;
  for (i = 0; i < slot; i++)
  {
    j = i;
    if ((i+1) < slot || !(size%MAX_VECTOR_SLOT_INDEX))
        v->index[i] = XMALLOC(MTYPE_VECTOR_INDEX, sizeof(void*) * (MAX_VECTOR_SLOT_INDEX));
    else
        v->index[i] = XMALLOC(MTYPE_VECTOR_INDEX, sizeof(void*) * (size%(MAX_VECTOR_SLOT_INDEX)));
    if (!v->index[i])
    {
      for (i = 0; i < j; i++)
        XFREE(MTYPE_VECTOR_INDEX, v->index[i]);
      XFREE(MTYPE_VECTOR, v);
      return NULL;
    }
    if ((i+1) < slot || !(size%MAX_VECTOR_SLOT_INDEX))
        memset (v->index[i], 0, sizeof (void *) * (MAX_VECTOR_SLOT_INDEX));
    else
        memset (v->index[i], 0, sizeof (void *) * (size%(MAX_VECTOR_SLOT_INDEX)));
  }

  
  return v;
}

void
vector_only_wrapper_free (vector v)
{
  XFREE (MTYPE_VECTOR, v);
}

void
vector_free (vector v)
{
  int slot, i;
  
  if (!v)
    return;
  slot = v->alloced/(MAX_VECTOR_SLOT_INDEX) + 1;
  for (i = 0; i < slot; i++)
  {
    if (v->index[i])
        XFREE (MTYPE_VECTOR_INDEX, v->index[i]);
  }

  XFREE (MTYPE_VECTOR, v);
}

vector
vector_copy (vector v)
{
  vector new = XMALLOC (MTYPE_VECTOR, sizeof (struct _vector));
  int slot_num, i, j;
  if (!new)
    return NULL;
  new->max = v->max;
  new->alloced = v->alloced;
  new->count = v->count;
  memset(new->index, 0, sizeof(void*)*MAX_VECTOR_SLOT_NUM);
  slot_num = v->alloced/(MAX_VECTOR_SLOT_INDEX) + 1;
  for (i = 0; i < slot_num; i++)
  {
    j = i;
    if ((i+1) < slot_num || !(v->alloced%(MAX_VECTOR_SLOT_INDEX)))
        new->index[i] = XMALLOC(MTYPE_VECTOR_INDEX, sizeof(void*) * (MAX_VECTOR_SLOT_INDEX));
    else
        new->index[i] = XMALLOC(MTYPE_VECTOR_INDEX, sizeof(void*) * (v->alloced%(MAX_VECTOR_SLOT_INDEX)));
    if (!new->index[i])
    {
      for (i = 0; i < j; i++)
        XFREE(MTYPE_VECTOR_INDEX, new->index[i]);
      XFREE(MTYPE_VECTOR, new);
      return NULL;
    }
    if ((i+1) < slot_num || !(v->alloced%(MAX_VECTOR_SLOT_INDEX)))
        memcpy (new->index[i],  v->index[i], sizeof (void *) * (MAX_VECTOR_SLOT_INDEX));
    else
        memcpy (new->index[i],  v->index[i], sizeof (void *) * (v->alloced%(MAX_VECTOR_SLOT_INDEX)));
  }

  return new;
}

/* Check assigned index, and if it runs short double index pointer */
void
vector_ensure (vector v, u_int32_t num)
{
  int slot, added_num = 0;
  if (!v || v->alloced > num)
    return;

  slot = v->alloced/(MAX_VECTOR_SLOT_INDEX);
  if (v->alloced && !(v->alloced% MAX_VECTOR_SLOT_INDEX))
  {
    v->index[slot] = XCALLOC(MTYPE_VECTOR_INDEX, sizeof(void*) * ((MAX_VECTOR_SLOT_INDEX)/4));
    if (!v->index[slot])
      return;
    v->alloced += (MAX_VECTOR_SLOT_INDEX)/4;
    memset (v->index[slot], 0, sizeof (void *) * ((MAX_VECTOR_SLOT_INDEX)/4));
  }
  else
  {
    if (v->alloced * 2 > MAX_VECTOR_SLOT_INDEX)
    {
        added_num = MAX_VECTOR_SLOT_INDEX - v->alloced%(MAX_VECTOR_SLOT_INDEX);
    }
    else
        added_num = v->alloced;
    v->index[slot] = XREALLOC (MTYPE_VECTOR_INDEX, v->index[slot], 
                    sizeof (void*) * (added_num + v->alloced%(MAX_VECTOR_SLOT_INDEX)));
    if (!v->index[slot])
    return;
    
    memset (&v->index[slot][v->alloced%(MAX_VECTOR_SLOT_INDEX)], 0, sizeof (void *) * added_num);
    v->alloced += added_num;
  }

  if (v->alloced <= num)
    vector_ensure (v, num);
}

/* This function only returns next empty slot index.  It dose not mean
   the slot's index memory is assigned, please call vector_ensure()
   after calling this function. */
u_int32_t
vector_empty_slot (vector v)
{
  u_int32_t i, slot, index;

  if(NULL == v)
  {
     return 0;
  }

  if (v->max == 0)
    return 0;

  for (i = 0; i < v->max; i++)
  {
    slot = i/(MAX_VECTOR_SLOT_INDEX);
    index = i%(MAX_VECTOR_SLOT_INDEX);
    if (!v->index[slot])
        return 0;
    if (!v->index[slot][index])
      return i;
  }
  return i;
}

/* Set value to the smallest empty slot. */
u_int32_t
vector_set (vector v, void *val)
{
  u_int32_t i;

  i = vector_empty_slot (v);
  vector_ensure (v, i);

  if (!v->index[i/(MAX_VECTOR_SLOT_INDEX)])
    return 0;
  v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)] = val;

  if (v->max <= i)
    v->max = i + 1;
  v->count++;

  return i;
}

/* Set value to specified index slot. */
u_int32_t
vector_set_index (vector v, u_int32_t i, void *val)
{
  vector_ensure (v, i);

  if (!v->index[i/(MAX_VECTOR_SLOT_INDEX)])
    return 0;
  if (!v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)])
    v->count++;
  v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)] = val;

  if (v->max <= i)
    v->max = i + 1;

  return i;
}

/* Lookup vector, ensure it. */
void *
vector_lookup_index (vector v, u_int32_t i)
{
  vector_ensure (v, i);
  
  if (!v->index[i/(MAX_VECTOR_SLOT_INDEX)])
    return NULL;
  return v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)];
}

/* Unset value at specified index slot. */
void
vector_unset (vector v, u_int32_t i)
{
  if(NULL == v)
  {
     return;
  }

  if (i >= v->alloced || i >= v->max)
    return;
  
  if (!v->index[i/(MAX_VECTOR_SLOT_INDEX)])
    return;
  v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)] = NULL;
  v->count--;
  if ((i+1) == v->max)
  {
    v->max--;
    i--;
    while ((i+1) && v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)] == NULL&& v->max--)
		i--;
    //while (i && v->index[(--i)/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)] == NULL&& v->max--)
    //    ;
  }
}

/* Count the number of not emplty slot. */
u_int32_t
vector_count (vector v)
{
  u_int32_t i;
  unsigned count = 0;

   for (i = 0; i < v->max; i++) 
    {
        if (v->index[i/(MAX_VECTOR_SLOT_INDEX)][i%(MAX_VECTOR_SLOT_INDEX)] != NULL)
            count++;
    }

  return count;
}

/* Add vector src items to vector dest.  */
void
vector_add (vector dest, vector src)
{
  int i;
  void *val;

  for (i = 0; i < vector_max (src); i++)
    if ((val = vector_slot (src, i)))
      vector_set (dest, val);
}

/* Reset dest before vector add.  */
void
vector_dup (vector dest, vector src)
{
  if( !dest || !src)
    return ;
  vector_reset (dest);
  vector_add (dest, src);
}

u_int32_t
vector_traverse(vector v, void (*func)(void *))
{
    u_int32_t i;
    void *val;
  
    for (i = 0; i < v->max; i++) 
    {
        val = vector_slot(v, i);
        if (val)   
            func(val);
    }

    return 0;
}

u_int32_t
vector_traverse1(vector v, void (*func)(void *, void *), void *arg)
{
    u_int32_t i;
    void *val;
  
    for (i = 0; i < v->max; i++) 
    {
        val = vector_slot(v, i);
        if (val)   
            func(val, arg);
    }

    return 0;
}

u_int32_t
vector_traverse1_two(vector v, void (*func)(void *, void *, u_int32_t i), void *arg)
{
    u_int32_t i;
    void *val;
  
    for (i = 0; i < v->max; i++) 
    {
        val = vector_slot(v, i);
        if (val)   
            func(val, arg, i);
    }

    return 0;
}

u_int32_t
vector_traverse2(vector v, void (*func)(void *, void *, void *), void *arg1, void *arg2)
{
    u_int32_t i;
    void *val;
  
    for (i = 0; i < v->max; i++) 
    {
        val = vector_slot(v, i);
        if (val)   
            func(val, arg1, arg2);
    }

    return 0;
}
