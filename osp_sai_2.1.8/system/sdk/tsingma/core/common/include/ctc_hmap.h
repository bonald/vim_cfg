#ifndef _CTC_HMAP_H
#define _CTC_HMAP_H

#ifdef  __cplusplus
extern "C" {
#endif

/* A hash map node, to be embedded inside the data structure being mapped. */
struct ctc_hmap_node_s
{
    struct ctc_hmap_node_s *next;     /* Next in linked list. */
    uint32 hash;                /* Hash value. */

};
typedef struct ctc_hmap_node_s ctc_hmap_node_t;

/* Returns the hash value embedded in 'node'. */
static INLINE
uint32 ctc_hmap_node_hash(const ctc_hmap_node_t *node)
{
    return node->hash;
}

#define HMAP_NODE_NULL ((ctc_hmap_node_t *) 1)
#define HMAP_NODE_NULL_INITIALIZER { 0, HMAP_NODE_NULL }

/* Given a pointer-typed lvalue OBJECT, expands to a pointer type that may be
 * assigned to OBJECT. */
#ifdef __GNUC__
#define TYPEOF(OBJECT) typeof(OBJECT)
#else
#define TYPEOF(OBJECT) void *
#endif
/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#ifndef offsetof
#define offsetof(STRUCTURE,FIELD) ((uintptr)((char*)&((STRUCTURE*)0)->FIELD))
#endif


/* Given OBJECT of type pointer-to-structure, expands to the offset of MEMBER
 * within an instance of the structure.
 *
 * The GCC-specific version avoids the technicality of undefined behavior if
 * OBJECT is null, invalid, or not yet initialized.  This makes some static
 * checkers (like Coverity) happier.  But the non-GCC version does not actually
 * dereference any pointer, so it would be surprising for it to cause any
 * problems in practice.
 */
#ifdef __GNUC__
#define OBJECT_OFFSETOF(OBJECT, MEMBER) offsetof(TYPEOF(*(OBJECT)), MEMBER)
#else
#define OBJECT_OFFSETOF(OBJECT, MEMBER) \
    ((char *) &(OBJECT)->MEMBER - (char *) (OBJECT))
#endif

/* Yields the size of MEMBER within STRUCT. */
#define MEMBER_SIZEOF(STRUCT, MEMBER) (sizeof(((STRUCT *) NULL)->MEMBER))

/* Given POINTER, the address of the given MEMBER in a STRUCT object, returns
   the STRUCT object. */
#define CONTAINER_OF(POINTER, STRUCT, MEMBER)                           \
        ((STRUCT *) (void *) ((char *) (POINTER) - offsetof (STRUCT, MEMBER)))

/* Casts 'pointer' to 'type' and issues a compiler warning if the cast changes
 * anything other than an outermost "const" or "volatile" qualifier.
 *
 * The cast to int is present only to suppress an "expression using sizeof
 * bool" warning from "sparse" (see
 * http://permalink.gmane.org/gmane.comp.parsers.sparse/2967). */
#define CONST_CAST(TYPE, POINTER)                               \
    ((void) sizeof ((int) ((POINTER) == (TYPE) (POINTER))),     \
     (TYPE) (POINTER))

/* As explained in the comment above OBJECT_OFFSETOF(), non-GNUC compilers
 * like MSVC will complain about un-initialized variables if OBJECT
 * hasn't already been initialized. To prevent such warnings, INIT_CONTAINER()
 * can be used as a wrapper around ASSIGN_CONTAINER. */
#define INIT_CONTAINER(OBJECT, POINTER, MEMBER) \
    ((OBJECT) = NULL, ASSIGN_CONTAINER(OBJECT, POINTER, MEMBER))

/* Given POINTER, the address of the given MEMBER within an object of the type
 * that that OBJECT points to, returns OBJECT as an assignment-compatible
 * pointer type (either the correct pointer type or "void *").  OBJECT must be
 * an lvalue.
 *
 * This is the same as CONTAINER_OF except that it infers the structure type
 * from the type of '*OBJECT'. */
#define OBJECT_CONTAINING(POINTER, OBJECT, MEMBER)                      \
    ((TYPEOF(OBJECT)) (void *)                                      \
     ((char *) (POINTER) - OBJECT_OFFSETOF(OBJECT, MEMBER)))

/* Given POINTER, the address of the given MEMBER within an object of the type
 * that that OBJECT points to, assigns the address of the outer object to
 * OBJECT, which must be an lvalue.
 *
 * Evaluates to (void) 0 as the result is not to be used. */
#define ASSIGN_CONTAINER(OBJECT, POINTER, MEMBER) \
    ((OBJECT) = OBJECT_CONTAINING(POINTER, OBJECT, MEMBER), (void) 0)

/* Returns true if 'node' has been set to null by hmap_node_nullify() and has
 * not been un-nullified by being inserted into an hmap. */
static INLINE bool
ctc_hmap_node_is_null(ctc_hmap_node_t *node)
{
    return node->next == HMAP_NODE_NULL;
}

/* Marks 'node' with a distinctive value that can be tested with
 * hmap_node_is_null().  */
static INLINE void
ctc_hmap_node_nullify(ctc_hmap_node_t *node)
{
    node->next = HMAP_NODE_NULL;
}

/* A hash map. */
struct ctc_hmap_s {
    ctc_hmap_node_t **buckets; /* Must point to 'one' iff 'mask' == 0. */
    ctc_hmap_node_t *one;
    uint32 mask;
    uint32 n;
};
typedef struct ctc_hmap_s ctc_hmap_t;

/* Initializer for an empty hash map. */
#define HMAP_INITIALIZER(HMAP) \
    { (ctc_hmap_node_t **const) &(HMAP)->one, NULL, 0, 0 }

/* Initializer for an immutable struct hmap 'HMAP' that contains a single
 * 'NODE'. */
#define HMAP_CONST1(HMAP, NODE) {                                   \
        CONST_CAST(ctc_hmap_node_t **, &(HMAP)->one), NODE, 0, 1 }
#define HMAP_NODE_INIT(HASH) { HASH, NULL }

/* Initialization. */
void ctc_hmap_init(ctc_hmap_t *);
void ctc_hmap_destroy(ctc_hmap_t *);
void ctc_hmap_clear(ctc_hmap_t *);
void ctc_hmap_swap(ctc_hmap_t *a, ctc_hmap_t *b);
void ctc_hmap_moved(ctc_hmap_t *hmap);
static INLINE uint32 ctc_hmap_count(const ctc_hmap_t *);
static INLINE bool ctc_hmap_is_empty(const ctc_hmap_t *);

/* Adjusting capacity. */
void ctc_hmap_expand_at(ctc_hmap_t * );
#define hmap_expand(HMAP) ctc_hmap_expand_at(HMAP)

void ctc_hmap_shrink_at(ctc_hmap_t *);
#define hmap_shrink(HMAP) ctc_hmap_shrink_at(HMAP)

void ctc_hmap_reserve_at(ctc_hmap_t *, uint32 capacity);
#define hmap_reserve(HMAP, CAPACITY) \
    ctc_hmap_reserve_at(HMAP, CAPACITY)

/* Insertion and deletion. */
static INLINE void ctc_hmap_insert_at(ctc_hmap_t *, ctc_hmap_node_t *, uint32 hash);
#define hmap_insert(HMAP, NODE, HASH) \
    ctc_hmap_insert_at(HMAP, NODE, HASH)

static INLINE void ctc_hmap_insert_fast(ctc_hmap_t *,
                                     ctc_hmap_node_t *, uint32 ctc_hash);
static INLINE void ctc_hmap_remove(ctc_hmap_t *, ctc_hmap_node_t *);

void ctc_hmap_node_moved(ctc_hmap_t *, ctc_hmap_node_t *, ctc_hmap_node_t *);
static INLINE void ctc_hmap_replace(ctc_hmap_t *, const ctc_hmap_node_t *old,
                                ctc_hmap_node_t *new_node);

 /*struct hmap_node *hmap_random_node(const struct hmap *);*/

/* Search.
 *
 * HMAP_FOR_EACH_WITH_HASH iterates NODE over all of the nodes in HMAP that
 * have hash value equal to HASH.  HMAP_FOR_EACH_IN_BUCKET iterates NODE over
 * all of the nodes in HMAP that would fall in the same bucket as HASH.  MEMBER
 * must be the name of the 'struct hmap_node' member within NODE.
 *
 * These macros may be used interchangeably to search for a particular value in
 * an hmap, see, e.g. shash_find() for an example.  Usually, using
 * HMAP_FOR_EACH_WITH_HASH provides an optimization, because comparing a hash
 * value is usually cheaper than comparing an entire hash map key.  But for
 * simple hash map keys, it makes sense to use HMAP_FOR_EACH_IN_BUCKET because
 * it avoids doing two comparisons when a single simple comparison suffices.
 *
 * The loop should not change NODE to point to a different node or insert or
 * delete nodes in HMAP (unless it "break"s out of the loop to terminate
 * iteration).
 *
 * HASH is only evaluated once.
 *
 * When the loop terminates normally, meaning the iteration has completed
 * without using 'break', NODE will be NULL.  This is true for all of the
 * HMAP_FOR_EACH_*() macros.
 */
#define HMAP_FOR_EACH_WITH_HASH(NODE, MEMBER, HASH, HMAP)               \
    for (INIT_CONTAINER(NODE, ctc_hmap_first_with_hash(HMAP, HASH), MEMBER); \
         (NODE != OBJECT_CONTAINING(NULL, NODE, MEMBER)) || (NODE = NULL); \
         ASSIGN_CONTAINER(NODE, ctc_hmap_next_with_hash(&(NODE)->MEMBER),   \
                          MEMBER))
#define HMAP_FOR_EACH_IN_BUCKET(NODE, MEMBER, HASH, HMAP)               \
    for (INIT_CONTAINER(NODE, ctc_hmap_first_in_bucket(HMAP, HASH), MEMBER); \
         (NODE != OBJECT_CONTAINING(NULL, NODE, MEMBER)) || (NODE = NULL); \
         ASSIGN_CONTAINER(NODE, ctc_hmap_next_in_bucket(&(NODE)->MEMBER), MEMBER))

static INLINE ctc_hmap_node_t *ctc_hmap_first_with_hash(const ctc_hmap_t *, uint32 hash);
static INLINE ctc_hmap_node_t *ctc_hmap_next_with_hash(const ctc_hmap_node_t *);
static INLINE ctc_hmap_node_t *ctc_hmap_first_in_bucket(const ctc_hmap_t *, uint32 hash);
static INLINE ctc_hmap_node_t *ctc_hmap_next_in_bucket(const ctc_hmap_node_t *);

bool ctc_hmap_contains(const ctc_hmap_t *,const ctc_hmap_node_t *);

/* Iteration. */

/* Iterates through every node in HMAP. */
#define HMAP_FOR_EACH(NODE, MEMBER, HMAP)                               \
    for (INIT_CONTAINER(NODE, ctc_hmap_first(HMAP), MEMBER);                \
         (NODE != OBJECT_CONTAINING(NULL, NODE, MEMBER)) || (NODE = NULL); \
         ASSIGN_CONTAINER(NODE, ctc_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER))

/* Safe when NODE may be freed (not needed when NODE may be removed from the
 * hash map but its members remain accessible and intact). */
#define HMAP_FOR_EACH_SAFE(NODE, NEXT, MEMBER, HMAP)                    \
    for (INIT_CONTAINER(NODE, ctc_hmap_first(HMAP), MEMBER);                \
         ((NODE != OBJECT_CONTAINING(NULL, NODE, MEMBER)) || (NODE = NULL) \
          ? INIT_CONTAINER(NEXT, ctc_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER), 1 \
          : 0);                                                         \
         (NODE) = (NEXT))

/* Continues an iteration from just after NODE. */
#define HMAP_FOR_EACH_CONTINUE(NODE, MEMBER, HMAP)                      \
    for (ASSIGN_CONTAINER(NODE, ctc_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER); \
         (NODE != OBJECT_CONTAINING(NULL, NODE, MEMBER)) || (NODE = NULL); \
         ASSIGN_CONTAINER(NODE, ctc_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER))

static INLINE ctc_hmap_node_t *ctc_hmap_first(const ctc_hmap_t *);
static INLINE ctc_hmap_node_t *ctc_hmap_next(const ctc_hmap_t *,const ctc_hmap_node_t *);

 /*struct hmap_node *hmap_at_position(const struct hmap *, uint32 *bucket, uint32 *offset);*/

/* Returns the number of nodes currently in 'hmap'. */
static INLINE uint32
ctc_hmap_count(const ctc_hmap_t *hmap)
{
    return hmap->n;
}

/* Returns the maximum number of nodes that 'hmap' may hold before it should be
 * rehashed. */
static INLINE uint32
ctc_hmap_capacity(const ctc_hmap_t *hmap)
{
    return hmap->mask * 2 + 1;
}

/* Returns true if 'hmap' currently contains no nodes,
 * false otherwise.
 * Note: While hmap in general is not thread-safe without additional locking,
 * hmap_is_empty() is. */
static INLINE bool
ctc_hmap_is_empty(const ctc_hmap_t *hmap)
{
    return hmap->n == 0;
}

/* Inserts 'node', with the given 'hash', into 'hmap'.  'hmap' is never
 * expanded automatically. */
static INLINE void
ctc_hmap_insert_fast(ctc_hmap_t *hmap, ctc_hmap_node_t *node, uint32 hash)
{
    ctc_hmap_node_t **bucket = &hmap->buckets[hash & hmap->mask];
    node->hash = hash;
    node->next = *bucket;
    *bucket = node;
    hmap->n++;
}

/* Inserts 'node', with the given 'hash', into 'hmap', and expands 'hmap' if
 * necessary to optimize search performance.
 *
 * ('where' is used in debug logging.  Commonly one would use hmap_insert() to
 * automatically provide the caller's source file and line number for
 * 'where'.) */
static INLINE void
ctc_hmap_insert_at(ctc_hmap_t *hmap, ctc_hmap_node_t *node, uint32 hash)
{
    ctc_hmap_insert_fast(hmap, node, hash);
    if (hmap->n / 2 > hmap->mask)
    {
        ctc_hmap_expand_at(hmap);
    }
}

/* Removes 'node' from 'hmap'.  Does not shrink the hash table; call
 * hmap_shrink() directly if desired. */
static INLINE void
ctc_hmap_remove(ctc_hmap_t *hmap, ctc_hmap_node_t *node)
{
    ctc_hmap_node_t **bucket = &hmap->buckets[node->hash & hmap->mask];
    while (*bucket != node)
    {
        bucket = &(*bucket)->next;
    }
    *bucket = node->next;
    hmap->n--;
}

/* Puts 'new_node' in the position in 'hmap' currently occupied by 'old_node'.
 * The 'new_node' must hash to the same value as 'old_node'.  The client is
 * responsible for ensuring that the replacement does not violate any
 * client-imposed invariants (e.g. uniqueness of keys within a map).
 *
 * Afterward, 'old_node' is not part of 'hmap', and the client is responsible
 * for freeing it (if this is desirable). */
static INLINE void
ctc_hmap_replace(ctc_hmap_t *hmap, const ctc_hmap_node_t *old_node, ctc_hmap_node_t *new_node)
{
    ctc_hmap_node_t **bucket = &hmap->buckets[old_node->hash & hmap->mask];
    while (*bucket != old_node)
    {
        bucket = &(*bucket)->next;
    }
    *bucket = new_node;
    new_node->hash = old_node->hash;
    new_node->next = old_node->next;
}

static INLINE ctc_hmap_node_t *
ctc_hmap_next_with_hash__(const ctc_hmap_node_t *node, uint32 hash)
{
    while (node != NULL && node->hash != hash)
    {
        node = node->next;
    }
    return CONST_CAST(ctc_hmap_node_t *, node);
}

/* Returns the first node in 'hmap' with the given 'hash', or a null pointer if
 * no nodes have that hash value. */
static INLINE ctc_hmap_node_t *
ctc_hmap_first_with_hash(const ctc_hmap_t *hmap, uint32 hash)
{
    return ctc_hmap_next_with_hash__(hmap->buckets[hash & hmap->mask], hash);
}

/* Returns the first node in 'hmap' in the bucket in which the given 'hash'
 * would land, or a null pointer if that bucket is empty. */
static INLINE ctc_hmap_node_t *
ctc_hmap_first_in_bucket(const ctc_hmap_t *hmap, uint32 hash)
{
    return hmap->buckets[hash & hmap->mask];
}

/* Returns the next node in the same bucket as 'node', or a null pointer if
 * there are no more nodes in that bucket.
 *
 * If the hash map has been reallocated since 'node' was visited, some nodes
 * may be skipped; if new nodes with the same hash value have been added, they
 * will be skipped.  (Removing 'node' from the hash map does not prevent
 * calling this function, since node->next is preserved, although freeing
 * 'node' of course does.) */
static INLINE ctc_hmap_node_t *
ctc_hmap_next_in_bucket(const ctc_hmap_node_t *node)
{
    return node->next;
}

/* Returns the next node in the same hash map as 'node' with the same hash
 * value, or a null pointer if no more nodes have that hash value.
 *
 * If the hash map has been reallocated since 'node' was visited, some nodes
 * may be skipped; if new nodes with the same hash value have been added, they
 * will be skipped.  (Removing 'node' from the hash map does not prevent
 * calling this function, since node->next is preserved, although freeing
 * 'node' of course does.) */
static INLINE ctc_hmap_node_t *
ctc_hmap_next_with_hash(const ctc_hmap_node_t *node)
{
    return ctc_hmap_next_with_hash__(node->next, node->hash);
}

static INLINE ctc_hmap_node_t *
ctc_hmap_next__(const ctc_hmap_t *hmap, uint32 start)
{
    uint32 i;
    for (i = start; i <= hmap->mask; i++)
    {
        ctc_hmap_node_t *node = hmap->buckets[i];
        if (node) {
            return node;
        }
    }
    return NULL;
}

/* Returns the first node in 'hmap', in arbitrary order, or a null pointer if
 * 'hmap' is empty. */
static INLINE ctc_hmap_node_t *
ctc_hmap_first(const ctc_hmap_t *hmap)
{
    return ctc_hmap_next__(hmap, 0);
}

/* Returns the next node in 'hmap' following 'node', in arbitrary order, or a
 * null pointer if 'node' is the last node in 'hmap'.
 *
 * If the hash map has been reallocated since 'node' was visited, some nodes
 * may be skipped or visited twice.  (Removing 'node' from the hash map does
 * not prevent calling this function, since node->next is preserved, although
 * freeing 'node' of course does.) */
static INLINE ctc_hmap_node_t *
ctc_hmap_next(const ctc_hmap_t *hmap, const ctc_hmap_node_t *node)
{
    return (node->next
            ? node->next
            : ctc_hmap_next__(hmap, (node->hash & hmap->mask) + 1));
}

#ifdef  __cplusplus
}
#endif

#endif /* ctc_hmap.h */
