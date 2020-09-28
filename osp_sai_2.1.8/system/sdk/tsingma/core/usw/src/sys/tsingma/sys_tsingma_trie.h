/****************************************************************************
 * cmodel_nlpm_trie.h:
 *   nlpm trie definition
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : cuixl
 * Date         : 2017-Feb-10 10:44
 * Reason       : First Create.
 ****************************************************************************/
#ifndef __NALPM_TRIE_H__
#define __NALPM_TRIE_H__

#include "sal_types.h"

/*#define NALPM_DEBUG_OUT printf*/ /* */
/*#define NALPM_DEBUG_ERR NALPM_DEBUG_OUT*/ /* */
#define NALPM_E_SUCCESS(rv) ((rv) >= 0)/* */
#define NALPM_E_FAILURE(rv) ((rv) < 0)/* */
/*
x       :   value
0       :   0
1-31    :   1
32      :   2
*/
#if 1
#define NALPM_BITS2WORDS(x)        (((x) + 31) / 32)
#else
#define NALPM_BITS2WORDS(x)        ((x) / 32)

#endif
#define BITS2WORDS(x)        NALPM_BITS2WORDS(x)


typedef struct trie_node_s trie_node_t;

typedef enum _node_type_e {
    INTERNAL,
    PAYLOAD,
    MAX
} node_type_e_t;

typedef struct child_node_s {
    trie_node_t *child_node;
} child_node_t;

struct trie_node_s {
    trie_node_t *trie_node;
#define _MAX_CHILD_     (2)
    child_node_t child[_MAX_CHILD_];
    unsigned int skip_len;/* eg 192.168.0.0/16-->skip_len = 16*/
    unsigned int skip_addr;/* prefix value: eg 192.168.0.0/16->skip_addr = 0.0.192.168*/
    node_type_e_t type;
    unsigned int count; /* number of payload node counts */
    unsigned int bpm; /* best prefix match bit map - 32 bits */
    unsigned int dirty_flag; /* set to 1 when this node is not using */
};

typedef struct trie_s {
    trie_node_t *trie;         /* trie root pointer */
    unsigned int v6_key;       /* support 144 bits key, otherwise expect 48 bits key */
} trie_t;

typedef int (*trie_callback_f)(trie_node_t *trie, void *datum);

typedef struct trie_bpm_cb_info_s {
    unsigned int *pfx; /* prefix buffer pointer from caller space */
    unsigned int  len;  /* prefix length */
    void         *user_data;
} trie_bpm_cb_info_t;

typedef int (*trie_propagate_cb_f)(trie_node_t *trie,
        trie_bpm_cb_info_t *info);

struct sys_nalpm_clone_para_s {
    trie_node_t* father;
    uint8 bit;
};
typedef struct sys_nalpm_clone_para_s sys_nalpm_clone_para_t;

/**
 brief:for dump
*/
struct trie_dump_node_s {
    trie_node_t *root; /* */
    trie_node_t *father; /* */
    uint32 father_lvl; /* */
};
typedef struct trie_dump_node_s trie_dump_node_t;

/*
 * This macro is a tidy way of performing subtraction to move from a
 * pointer within an object to a pointer to the object.
 *
 * Arguments are:
 *    type of object to recover
 *    pointer to object from which to recover element pointer
 *    pointer to an object of type t
 *    name of the trie node field in t through which the object is linked on trie
 * Returns:
 *    a pointer to the object, of type t
 */
#define TRIE_ELEMENT(t, p, ep, f) \
    ((t) (((char *) (p)) - (((char *) &((ep)->f)) - ((char *) (ep)))))

/*
 * TRIE_ELEMENT_GET performs the same function as TRIE_ELEMENT, but does not
 * require a pointer of type (t).  This form is preferred as TRIE_ELEMENT
 * typically generate Coverity errors, and the (ep) argument is unnecessary.
 *
 * Arguments are:
 *    type of object to recover
 *    pointer to object from which to recover element pointer
 *    name of the trie node field in t through which the object is linked on trie
 * Returns:
 *    a pointer to the object, of type t
 */
#define TRIE_ELEMENT_GET(t, p, f) \
    ((t) (((char *) (p)) - (((char *) &(((t)(0))->f)))))


#define _CLONE_TRIE_NODE_(dest,src) \
    sal_memcpy((dest),(src),sizeof(trie_node_t))

#define _CLONE_ROUTE_TRIE_PAYLOAD_(dest, src) \
    sal_memcpy((dest),(src),sizeof(sys_nalpm_trie_payload_t))


typedef enum _trie_split2_states_e_s {
    TRIE_SPLIT2_STATE_NONE,
    TRIE_SPLIT2_STATE_PRUNE_NODES,
    TRIE_SPLIT2_STATE_DONE,
    TRIE_SPLIT2_STATE_MAX
} trie_split2_states_e_t;


typedef enum _trie_traverse_states_e_s {
    TRIE_TRAVERSE_STATE_NONE,
    TRIE_TRAVERSE_STATE_DELETED,
    TRIE_TRAVERSE_STATE_DONE,
    TRIE_TRAVERSE_STATE_MAX
} trie_traverse_states_e_t;

typedef int (*trie_callback_ext_f)(trie_node_t *ptrie, trie_node_t *trie,
                                   trie_traverse_states_e_t *state, void *info);


/*
 * Function:
 *     sys_tsingma_trie_init
 * Purpose:
 *     allocates a trie & initializes it
 */
extern int sys_nalpm_trie_init(unsigned int max_key_len, trie_t **ptrie);

/*
 * Function:
 *     sys_nalpm_trie_destroy
 * Purpose:
 *     destroys a trie
 */
extern int sys_nalpm_trie_destroy(trie_t *trie);

/*
 * Function:
 *     sys_nalpm_trie_insert
 * Purpose:
 *     Inserts provided prefix/length in to the trie
 */
extern int sys_nalpm_trie_insert(trie_t *trie,
        unsigned int *key,
        /* bpm bit map if bpm management is required,
           passing null skips bpm management */
        unsigned int *bpm,
        unsigned int length,
        trie_node_t *payload,
        uint8 is_route);

/*
 * Function:
 *     sys_nalpm_trie_delete
 * Purpose:
 *     Deletes provided prefix/length in to the trie
 */
extern int sys_nalpm_trie_delete(trie_t *trie,
        unsigned int *key,
        unsigned int length,
        trie_node_t **payload,
        uint8 is_route);



int trie_merge(trie_t *parent_trie,
               trie_node_t *child_trie,
               unsigned int *child_pivot,
               unsigned int length);


/*
 * Function:
 *     trie_delete
 * Purpose:
 *     Deletes provided prefix/length in to the trie
 */
    int
trie_delete (trie_t * trie,
        unsigned int *key, unsigned int length, trie_node_t ** payload);


    int
trie_destroy (trie_t * trie);


/*
 * Function:
 *     sys_nalpm_trie_dump
 * Purpose:
 *     Dumps the trie pre-order [root|left|child]
 */
extern int sys_nalpm_trie_dump(trie_t *trie, trie_callback_f cb, void *user_data);

/*
 * Function:
 *     sys_nalpm_trie_find_lpm
 * Purpose:
 *     Find the longest prefix matched with iven prefix
 */
extern int sys_nalpm_trie_find_lpm(trie_t *trie,
        unsigned int *key,
        unsigned int length,
        trie_node_t **payload,
        uint8 is_route);

/*
 * Function:
 *     sys_nalpm_trie_split
 * Purpose:
 *     Split the trie into 2 based on optimum pivot
 */
extern int sys_nalpm_trie_split(trie_t *trie,
        const unsigned int max_split_len,
        const int split_to_pair,
        unsigned int *pivot,
        unsigned int *length,
        trie_node_t **split_trie_root,
        unsigned int *bpm,
        /* if set split will strictly split only on payload nodes
         * if not set splits at optimal point on the trie */
        uint8 payload_node_split);

typedef enum _trie_traverse_order_e_s {
    _TRIE_PREORDER_TRAVERSE,  /* root, left, right */
    _TRIE_INORDER_TRAVERSE,   /* left, root, right */
    _TRIE_POSTORDER_TRAVERSE, /* left, right, root */
/*********modification begin*********
  * reason: dump traverse
  * mender: cuixl
  *   date: 2017-02-21 10:56
  */
    _TRIE_DUMP_TRAVERSE,  /* root, left, right */
/*********modification end*********/
    _TRIE_CLONE_TRAVERSE,
    _TRIE_CLEAR_TRAVERSE,
    _TRIE_ARRANGE_FRAGMENT_TRAVERSE,
    _TRIE_PREORDER_AD_ROUTE_TRAVERSE,
    _TRIE_TRAVERSE_MAX
} trie_traverse_order_e_t;

/*
 * Function:
 *     sys_tsingma_trie_traverse
 * Purpose:
 *     Traverse the trie & call the application callback with user data
 */
extern int sys_nalpm_trie_traverse(trie_node_t *trie,
        trie_callback_f cb,
        void *user_data,
        trie_traverse_order_e_t order);

extern int sys_nalpm_trie_clone(trie_t* trie, trie_t** clone_trie);

extern int sys_nalpm_trie_clear(trie_node_t* node);

int sys_nalpm_trie_traverse2(trie_t *trie, trie_callback_ext_f cb,
                   void *user_data, trie_traverse_order_e_t order);

/*=================================
 * Used by internal functions only
 *================================*/
#if 0
#define _MAX_KEY_LEN_48_  (48)
#define _MAX_KEY_LEN_144_ (144)
#else
#define _MAX_KEY_LEN_48_  (_MAX_KEY_LEN_32_)
#define _MAX_KEY_LEN_144_ (_MAX_KEY_LEN_128_)
#endif
#define _MAX_KEY_LEN_32_  (32)
#define _MAX_KEY_LEN_128_ (128)

typedef enum _trie_split_states_e_s {
    TRIE_SPLIT_STATE_NONE,
    TRIE_SPLIT_STATE_PAYLOAD_SPLIT,
    TRIE_SPLIT_STATE_PAYLOAD_SPLIT_DONE,
    TRIE_SPLIT_STATE_PRUNE_NODES,
    TRIE_SPLIT_STATE_DONE,
    TRIE_SPLIT_STATE_MAX
} trie_split_states_e_t;

#define _MAX_SKIP_LEN_  (31)

#define SHL(data, shift, max) \
    (((shift)>=(max))?0:((data)<<(shift)))

#define SHR(data, shift, max) \
    (((shift)>=(max))?0:((data)>>(shift)))

#define MASK(len) \
    (((len)>=32 || (len)==0)?0xFFFFFFFF:((1<<(len))-1))

#define BITMASK(len) \
    (((len)>=32)?0xFFFFFFFF:((1<<(len))-1))

#define ABS(n) ((((int)(n)) < 0) ? -(n) : (n))

#define _NUM_WORD_BITS_ (32)

/*
 * bit 0                                  - 0
 * bit [1, _MAX_SKIP_LEN]                 - 1
 * bit [_MAX_SKIP_LEN+1, 2*_MAX_SKIP_LEN] - 2...
 */
#define BITS2SKIPOFF(x) (((x) + _MAX_SKIP_LEN_-1) / _MAX_SKIP_LEN_)

/* (internal) Generic operation macro on bit array _a, with bit _b */
#define	_BITOP(_a, _b, _op)\
    ((_a) _op (1U << ((_b) % _NUM_WORD_BITS_)))

/* Specific operations */
#define _BITGET(_a, _b) _BITOP(_a, _b, &)
#define _BITSET(_a, _b) _BITOP(_a, _b, |=)
#define _BITCLR(_a, _b) _BITOP(_a, _b, &= ~)

/* get the bit position of the LSB set in bit 0 to bit "msb" of "data"
 * (max 32 bits), "lsb" is set to -1 if no bit is set in "data".
 */
#define BITGETLSBSET(data, msb, lsb)    \
{                                    \
    lsb = 0;                         \
    while ((lsb)<=(msb)) {		 \
        if ((data)&(1<<(lsb)))     { \
            break;                   \
        } else { (lsb)++;}           \
    }                                \
    lsb = ((lsb)>(msb))?-1:(lsb);    \
}

extern int sys_nalpm_trie_fuse_child(trie_node_t *trie, int bit);

extern int sys_nalpm_print_trie_node(trie_node_t *trie, void *datum);

/* loop up key size */
#define TAPS_IPV4_MAX_VRF_SIZE (16)
#define TAPS_IPV4_PFX_SIZE (32)
#define TAPS_IPV4_KEY_SIZE (TAPS_IPV4_MAX_VRF_SIZE + TAPS_IPV4_PFX_SIZE)
#define TAPS_IPV4_KEY_SIZE_WORDS (((TAPS_IPV4_KEY_SIZE)+31)/32)

#define TAPS_IPV6_MAX_VRF_SIZE (16)
#define TAPS_IPV6_PFX_SIZE (128)
 /*#define TAPS_IPV6_KEY_SIZE (TAPS_IPV6_MAX_VRF_SIZE + TAPS_IPV6_PFX_SIZE)*/
#define TAPS_IPV6_KEY_SIZE (128)

#define TAPS_IPV6_KEY_SIZE_WORDS (((TAPS_IPV6_KEY_SIZE)+31)/32)
#define TAPS_MAX_KEY_SIZE       (TAPS_IPV6_KEY_SIZE)
#define TAPS_MAX_KEY_SIZE_WORDS (((TAPS_MAX_KEY_SIZE)+31)/32)




#define TP_MASK(len) \
  (((len)>=32)?0xffffFFFF:((1U<<(len))-1))

#define TP_SHL(data, shift) \
  (((shift)>=32)?0:((data)<<(shift)))

#define TP_SHR(data, shift) \
  (((shift)>=32)?0:((data)>>(shift)))

/* get bit at bit_position from uint32 key array of maximum length of max_len
 * assuming the big-endian word order. bit_pos is 0 based. for example, assuming
 * the max_len is 48 bits, then key[0] has bits 47-32, and key[1] has bits 31-0.
 * use _TAPS_GET_KEY_BIT(key, 0, 48) to get bit 0.
 */
#define TP_BITS2IDX(bit_pos, max_len) \
  ((NALPM_BITS2WORDS(max_len) - 1) - (bit_pos)/32)

#define _TAPS_GET_KEY_BIT(key, bit_pos, max_len) \
  (((key)[TP_BITS2IDX(bit_pos, max_len)] & (1<<((bit_pos)%32))) >> ((bit_pos)%32))

#define _TAPS_SET_KEY_BIT(key, bit_pos, max_len) \
  ((key)[TP_BITS2IDX(bit_pos, max_len)] |= (1<<((bit_pos)%32)))

/* Get "len" number of bits start with lsb bit postion from an unsigned int array
 * the array is assumed to be with format described above in _TAPS_GET_KEY_BIT
 * NOTE: len must be <= 32 bits
 */
#define _TAPS_GET_KEY_BITS(key, lsb, len, max_len)  \
  ((TP_SHR((key)[TP_BITS2IDX(lsb, max_len)], ((lsb)%32)) |	\
    ((TP_BITS2IDX(lsb, max_len)<1)?0:(TP_SHL((key)[TP_BITS2IDX(lsb, max_len)-1], 32-((lsb)%32))))) & \
   (TP_MASK((len))))

typedef struct taps_ipv4_prefix_s {
    uint32 length:8;
    uint32 vrf:16;
    uint32 key;
} taps_ipv4_prefix_t;

typedef struct taps_ipv6_prefix_s {
    uint32 length:8;
    uint32 vrf:16;
    uint32 key[4];
} taps_ipv6_prefix_t;

extern int sys_nalpm_taps_key_shift(uint32 max_key_size, uint32 *key, uint32 length, int32 shift);


#endif /* __NALPM_TRIE_H__ */

