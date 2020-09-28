#ifndef _CTCLIB_HASH_H_
#define _CTCLIB_HASH_H_
#include "sal.h"

/* Default hash table size.  */ 
#define CTCLIB_HASHTABSIZE     1024

struct ctclib_hash_backet_s
{
    /* Linked list.  */
    struct ctclib_hash_backet_s *next;

    /* Hash key. */
    uint32 key;

    /* Data.  */
    void *data;
};
typedef struct ctclib_hash_backet_s ctclib_hash_backet_t;

struct ctclib_hash_s
{
  /* Hash backet. */
  ctclib_hash_backet_t **index;

  /* Hash table size. */
  uint32 size;

  /* Key make function. */
  uint32 (*hash_key) ();

  /* Data compare function. */
  bool (*hash_cmp) ();

  /* Backet alloc. */
  uint32 count;
};
typedef struct ctclib_hash_s ctclib_hash_t;

#ifdef _OFP_CENTEC_
typedef uint32 (* hash_key_fn) (void* data);
typedef bool (* hash_cmp_fn) (void* data1, void* data2);
#endif

#ifdef _OFP_CENTEC_
ctclib_hash_t *ctclib_hash_create (hash_key_fn, hash_cmp_fn);
#else
ctclib_hash_t *ctclib_hash_create (uint32 (*) (), bool (*) ());
#endif

ctclib_hash_t *ctclib_hash_create_size (uint32, uint32 (*) (), bool (*) ());

void *ctclib_hash_get (ctclib_hash_t *, void *, void * (*) ());
void *ctclib_hash_get2(ctclib_hash_t *hash, void *data,  void *arg, void * (*alloc_func) ());
void *ctclib_hash_alloc_intern (void *);
void *ctclib_hash_lookup (ctclib_hash_t *, void *);
void *ctclib_hash_release (ctclib_hash_t *, void *);

void ctclib_hash_iterate (ctclib_hash_t *,
                   void (*) (ctclib_hash_backet_t *, void *), void *);
void ctclib_hash_iterate2 (ctclib_hash_t *,
                    int32 (*) (ctclib_hash_backet_t *, void *, void *),
                    void *, void *);
void ctclib_hash_iterate3 (ctclib_hash_t *,
                    void (*) (ctclib_hash_backet_t *, void *, void *, void *),
                    void *, void *, void *);
void
ctclib_hash_iterate4 (ctclib_hash_t *hash,
               int32 (*func) (ctclib_hash_backet_t *, void *, void *, void *, void *),
               void *arg1, void *arg2, void *arg3, void *arg4);

void
ctclib_hash_iterate2_cont (ctclib_hash_t *hash, int cont_index,
               int32 (*func) (ctclib_hash_backet_t *, int, void *, void *),
               void *arg1, void *arg2);

void
ctclib_hash_iterate2_cont_round (ctclib_hash_t *hash, int start_index,
               int32 (*func) (ctclib_hash_backet_t *, int, void *, void *),
               void *arg1, void *arg2);

void ctclib_hash_clean (ctclib_hash_t *, void (*) (void *));
void ctclib_hash_free (ctclib_hash_t *);

#endif /* _CTCLIB_HASH_H_ */
