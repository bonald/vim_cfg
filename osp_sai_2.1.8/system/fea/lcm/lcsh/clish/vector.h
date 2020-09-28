
#ifndef _LCSH_VECTOR_H
#define _LCSH_VECTOR_H

/* We can support up to 64K index. There are 16 vector array
     and in each array, there are 4K index. added by zhangwf, 2010-05-05 */
#define MAX_VECTOR_SLOT_INDEX 4096
#define MAX_VECTOR_SLOT_NUM   16

/* Vector structure. */
struct _vector 
{
  /* Max number of used index.  */
  u_int32_t max;

  /* Number of allocated index.  */
  u_int32_t alloced;

  /* active index count */
  u_int32_t count;
  
  /* Index to data.  */
  void **index[MAX_VECTOR_SLOT_NUM];
};

/* Typedef it.  */
typedef struct _vector *vector;

#define VECTOR_MIN_SIZE 8

/* Macros.  */
#define vector_slot(V,I)         ((V)->index[(I)/(MAX_VECTOR_SLOT_INDEX)][(I)%(MAX_VECTOR_SLOT_INDEX)])
/*#define vector_count(V)        ((V)->count)*/
#define vector_max(V)            ((V)->max)
#define vector_size(V)           ((V)->alloced)
#define vector_reset(V)          ((V)->max = 0)
#define vector_swap(A,B) \
  do {                   \
    vector _v;           \
    _v = A;              \
    A = B;               \
    B = _v;              \
  } while (0)

#define VECTOR_LOOP(V, D, I)                                          \
  for ((I) = 0; (I) < vector_max (V); (I)++)                          \
    if (((D) = vector_slot ((V), (I))))

/* Prototypes. */
vector vector_init (u_int32_t size);
void vector_ensure (vector v, u_int32_t num);
u_int32_t vector_empty_slot (vector v);
u_int32_t vector_set (vector v, void *val);
u_int32_t vector_set_index (vector v, u_int32_t i, void *val);
void *vector_lookup_index (vector v, u_int32_t i);
void vector_unset (vector v, u_int32_t i);
u_int32_t vector_count (vector v);
void vector_only_wrapper_free (vector v);
void vector_free (vector v);
vector vector_copy (vector v);
void vector_add (vector dest, vector src);
void vector_dup (vector dest, vector src);
u_int32_t vector_traverse(vector v, void (*func)(void *));
u_int32_t vector_traverse1(vector v, void (*func)(void *, void *), void *arg);
u_int32_t vector_traverse1_two(vector v, void (*func)(void *, void *, u_int32_t i), void *arg);
u_int32_t vector_traverse2(vector v, void (*func)(void *, void *, void *), void *arg1, void *arg2);
#endif /* _LCSH_VECTOR_H */
