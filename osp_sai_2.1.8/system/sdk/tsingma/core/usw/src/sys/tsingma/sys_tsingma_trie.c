/****************************************************************************
* cmodel_nlpm_trie.c:
*   cmodel_nlpm_trie.c
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision     : R0.01
* Author       : cuixl
* Date         : 2017-Feb-10 11:17
* Reason       : First Create.
****************************************************************************/

/* Implementation notes:
 * Trie is a prefix based data strucutre. It is based on modification to digital search trie.
 * This implementation is not a Path compressed Binary Trie (or) a Patricia Trie.
 * It is a custom version which represents prefix on a digital search trie as following.
 * A given node on the trie could be a Payload node or a Internal node. Each node is represented
 * by <skip address, skip length> pair. Each node represents the given prefix it represents when
 * the prefix is viewed from Right to Left. ie., Most significant bits to Least significant bits.
 * Each node has a Right & Left child which branches based on bit on that position.
 * There can be empty split node i.e, <0,0> just to host two of its children.
 *
 */
#include "sys_tsingma_nalpm.h"
#include "sys_tsingma_trie.h"
#include "sys_tsingma_trie_v6.h"
#include "sys_usw_ipuc.h"
#include "ctc_error.h"
#include "sal.h"


#define _MAX_SKIP_LEN_  (31)
#define _MAX_KEY_LEN_   (_MAX_KEY_LEN_32_)

#define _MAX_KEY_WORDS_ (NALPM_BITS2WORDS(_MAX_KEY_LEN_))

#define SHL(data, shift, max) \
    (((shift)>=(max))?0:((data)<<(shift)))

#define SHR(data, shift, max) \
    (((shift)>=(max))?0:((data)>>(shift)))

#if 1
#define MASK(len) \
    (((len)>=32 || (len)==0)?0xFFFFFFFF:((1<<(len))-1))
#else

#define MASK(len) \
		(((len)>=32 || (len)==0)?0xFFFFFFFF:(((1<<(len))-1) << (32 - len)) )
#endif

#define BITMASK(len) \
    (((len)>=32)?0xFFFFFFFF:((1<<(len))-1))

#define ABS(n) ((((int)(n)) < 0) ? -(n) : (n))

#define _NUM_WORD_BITS_ (32)

/* (x + 31 - 1) / 31 = (x + 30 / 31)
x       :   value
0       :   0
1-31  :   1
*/
#define BITS2SKIPOFF(x) (((x) + _MAX_SKIP_LEN_-1) / _MAX_SKIP_LEN_)

/* key packing expetations:
* eg., 48 bit key
* - 10 / 8 -> key[0] = 0, key[1] = 8
* - 0x123456789a -> key[0] = 0x12 key[1] = 0x3456789a
* length - represents number of valid bits from farther to lower index ie., 1->0
*/
/* (((48+31)/32) * 32 - x) / 32  = (2 * 32 - x) / 32 = (64 - x) /32
x           :           value
0           :           2
1-32      :           1
33-48    :           0
*/

#if 1
#define KEY_BIT2IDX(x) (((NALPM_BITS2WORDS(_MAX_KEY_LEN_)*32) - (x))/32)
#else
#define KEY_BIT2IDX(x) 0
#endif


/* (internal) Generic operation macro on bit array _a, with bit _b */
#define _BITOP(_a, _b, _op) \
((_a) _op (1U << ((_b) % _NUM_WORD_BITS_)))

/* Specific operations */
#define _BITGET(_a, _b) _BITOP(_a, _b, &)
#define _BITSET(_a, _b) _BITOP(_a, _b, |=)
#define _BITCLR(_a, _b) _BITOP(_a, _b, &= ~)

/* get the bit position of the LSB set in bit 0 to bit "msb" of "data"
* (max 32 bits), "lsb" is set to - 1 if no bit is set in "data".
*/
#define BITGETLSBSET(data, msb, lsb)    \
{                                    \
    lsb = 0;                         \
    while ((lsb)<=(msb)) {           \
        if ((data)&(1<<(lsb)))     { \
            break;                   \
        } else { (lsb)++;}           \
    }                                \
    lsb = ((lsb)>(msb))?-1:(lsb);    \
}

extern sys_nalpm_master_t* g_sys_nalpm_master[];

/*
*
* Function:
*     sys_nalpm_taps_key_shift
* Input:
*     max_key_size  -- max number of bits in the key
*                      ipv4 == 48
*                      ipv6 == 144
*     key   -- uint32 array head. Only "length" number of bits
*              is passed in.
*              for ipv4. Key[0].bit15 - 0 is key bits 47 - 32
*                        Key[1] is key bits 31 - 0
*              for ipv6. Key[0].bit15 - 0 is key bits 143 - 128
*                        Key[1 - 4] is key bits 127 - 0
*     length-- number of valid bits in key array. This would be
*              valid MSB bits of the route. For example,
 *              (vrf = 0x1234, ip = 0xf0000000, length = 20) would store
*              as key[0] = 0, key[1] = 0x1234F, length = 20.
*     shift -- positive means right shift, negative means left shift
*              routine will check if the shifted key is out of
*              max_key_size boundary.
*/
int
sys_nalpm_taps_key_shift (uint32 max_key_size, uint32 * key, uint32 length, int32 shift)
{
    int word_idx, lsb;

    if ((key == NULL) ||
        (length > max_key_size) ||
    ((max_key_size != TAPS_IPV4_KEY_SIZE) &&
    (max_key_size != TAPS_IPV6_KEY_SIZE)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (((length - shift) > max_key_size) || ((shift > 0) && (shift > length)))
    {
        /* left shift resulted in key longer than max_key_size or
        * right shift resulted in key shorter than 0
        */
        return CTC_E_INVALID_PARAM;
    }


    if (shift > 0)
    {
        /* right shift */
        for (lsb = shift, word_idx = NALPM_BITS2WORDS (max_key_size) - 1;
            word_idx >= 0;
        lsb += 32, word_idx--)
        {
            if (lsb < length)
            {
                key[word_idx] =
                _TAPS_GET_KEY_BITS (key, lsb,
                                    ((length - lsb) >=
                                    32) ? 32 : (length - lsb), max_key_size);
            }
            else
            {
                key[word_idx] = 0;
            }
        }
    }
    else if (shift < 0)
    {
        /* left shift */
        shift = 0 - shift;

        /* whole words shifting first */
        for (word_idx = 0;
            ((shift / 32) != 0) && (word_idx < NALPM_BITS2WORDS (max_key_size));
        word_idx++)
        {
            if ((word_idx + (shift / 32)) >= NALPM_BITS2WORDS (max_key_size))
            {
                key[word_idx] = 0;
            }
            else
            {
                key[word_idx] = key[word_idx + (shift / 32)];
            }
        }

        /* shifting remaining bits */
        for (word_idx = 0;
            ((shift % 32) != 0) && (word_idx < NALPM_BITS2WORDS (max_key_size));
        word_idx++)
        {
            if (word_idx == TP_BITS2IDX (0, max_key_size))
            {
                /* at bit 0 word, next word doesn't exist */
                key[word_idx] = TP_SHL (key[word_idx], (shift % 32));
            }
            else
            {
                key[word_idx] = TP_SHL (key[word_idx], (shift % 32)) |
                TP_SHR (key[word_idx + 1], 32 - (shift % 32));
            }
        }

        /* mask off bits higher than max_key_size */
         /*key[0] &= TP_MASK (max_key_size % 32)*/
    }

    return CTC_E_NONE;
}


 /*extern cmodel_nlpm_master_t *g_sys_nalpm_master*/
/********************************************************/
/* Get a chunk of bits from a key (MSB bit - on word0, lsb on word 1)..
*/
STATIC unsigned int
_key_get_bits (unsigned int *key, unsigned int pos /* 1based, msb bit position */ ,
        unsigned int len, unsigned int skip_len_check)
{
    unsigned int val = 0, delta = 0, diff, bitpos;

    /*if (!key || (pos < 1) || (pos > _MAX_KEY_LEN_) ||
            ((skip_len_check == 0) && (len > _MAX_SKIP_LEN_)))
        assert (0);*/

    bitpos = (pos - 1) % _NUM_WORD_BITS_;
    bitpos++;   /* 1 based */

    if (bitpos >= len)
    {
        diff = bitpos - len;
        val = SHR (key[KEY_BIT2IDX (pos)], diff, _NUM_WORD_BITS_);
        val &= MASK (len);
        return val;
    }
    else
    {
        diff = len - bitpos;
        /*if (skip_len_check == 0)
            assert (diff <= _MAX_SKIP_LEN_);*/
        val = key[KEY_BIT2IDX (pos)] & MASK (bitpos);
        val = SHL (val, diff, _NUM_WORD_BITS_);
        /* get bits from next word */
        delta = _key_get_bits (key, pos - bitpos, diff, skip_len_check);
        return (delta | val);
    }
}


/*
 * Assumes the layout for
 * 0 - most significant word
 * _MAX_KEY_WORDS_ - least significant word
 * eg., for key size of 48, word0-[bits 48-32] word1-[bits31-0]
 */
STATIC int
_key_shift_left (unsigned int *key, unsigned int shift)
{
    unsigned int index = 0;

    if (!key || shift > _MAX_SKIP_LEN_)
        return CTC_E_INVALID_PARAM;

    for (index = KEY_BIT2IDX (_MAX_KEY_LEN_); index < KEY_BIT2IDX (1); index++)
    {
        key[index] = SHL (key[index], shift, _NUM_WORD_BITS_) |
            SHR (key[index + 1], _NUM_WORD_BITS_ - shift, _NUM_WORD_BITS_);
    }

    key[index] = SHL (key[index], shift, _NUM_WORD_BITS_);

    /* mask off snippets bit on MSW */
    key[0] &= MASK (_MAX_KEY_LEN_ % _NUM_WORD_BITS_);
    return CTC_E_NONE;
}

/*
 * Assumes the layout for
 * 0 - most significant word
 * _MAX_KEY_WORDS_ - least significant word
 * eg., for key size of 48, word0-[bits 48-32] word1-[bits31-0]
 */
STATIC int
_key_shift_right (unsigned int *key, unsigned int shift)
{
    unsigned int index = 0;

    if (!key || shift > _MAX_SKIP_LEN_)
        return CTC_E_INVALID_PARAM;

    for (index = KEY_BIT2IDX (1); index > KEY_BIT2IDX (_MAX_KEY_LEN_); index--)
    {
        key[index] = SHR (key[index], shift, _NUM_WORD_BITS_) |
            SHL (key[index - 1], _NUM_WORD_BITS_ - shift, _NUM_WORD_BITS_);
    }

    key[index] = SHR (key[index], shift, _NUM_WORD_BITS_);

    /* mask off snippets bit on MSW */
    key[0] &= MASK (_MAX_KEY_LEN_ % _NUM_WORD_BITS_);
    return CTC_E_NONE;
}


/*
 * Assumes the layout for
 * 0 - most significant word
 * _MAX_KEY_WORDS_ - least significant word
 * eg., for key size of 48, word0-[bits 48-32] word1-[bits31-0]
 */
STATIC int
_key_append (unsigned int *key,
        unsigned int *length,
        unsigned int skip_addr, unsigned int skip_len)
{
    int rv = CTC_E_NONE;

    if (!key || !length || (skip_len + *length > _MAX_KEY_LEN_)
            || skip_len > _MAX_SKIP_LEN_)
        return CTC_E_INVALID_PARAM;

    rv = _key_shift_left (key, skip_len);
    if ( rv >= 0)
    {
        key[KEY_BIT2IDX (1)] |= skip_addr;
        *length += skip_len;
    }

    return rv;
}

STATIC int
_bpm_append (unsigned int *key,
        unsigned int *length,
        unsigned int skip_addr, unsigned int skip_len)
{
    int rv = CTC_E_NONE;

    if (!key || !length || (skip_len + *length > _MAX_KEY_LEN_)
            || skip_len > (_MAX_SKIP_LEN_ + 1))
        return CTC_E_INVALID_PARAM;

    if (skip_len == 32)
    {
        key[0] = key[1];
        key[1] = skip_addr;
        *length += skip_len;
    }
    else
    {
        rv = _key_shift_left (key, skip_len);
        if (NALPM_E_SUCCESS (rv))
        {
            key[KEY_BIT2IDX (1)] |= skip_addr;
            *length += skip_len;
        }
    }

    return rv;
}

/*
 * Function:
 *     _lcplen
 * Purpose:
 *     returns longest common prefix length provided a key & skip address
 */
STATIC unsigned int _lcplen (unsigned int *key, unsigned int len1, unsigned int skip_addr, unsigned int skip_len)
{
    unsigned int diff;
    unsigned int lcp = len1 < skip_len ? len1 : skip_len;

    if ((len1 > _MAX_KEY_LEN_) || (skip_len > _MAX_KEY_LEN_))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "len1 %d or skip_len %d is larger than %d\n", len1, skip_len,
                _MAX_KEY_LEN_);
        /*assert (0);*/
    }

    if (len1 == 0 || skip_len == 0)
        return 0;

    diff = _key_get_bits (key, len1, lcp, 0);
    diff ^= (SHR (skip_addr, skip_len - lcp, _MAX_SKIP_LEN_) & MASK (lcp));

    while (diff)
    {
        diff >>= 1;
        --lcp;
    }

    return lcp;
}

int
sys_nalpm_print_trie_node (trie_node_t * trie, void *datum)
{
    if (trie != NULL)
    {

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
             "trie: %p, type %s, skip_addr 0x%x skip_len %d count:%d bpm:0x%x Child[0]:%p Child[1]:%p\n",
             trie, (trie->type == PAYLOAD) ? "P" : "I", trie->skip_addr,
             trie->skip_len, trie->count, trie->bpm, trie->child[0].child_node,
             trie->child[1].child_node);
    }
    return CTC_E_NONE;
}

    STATIC int
_trie_preorder_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *tmp1, *tmp2;

    if (trie == NULL || !cb)
    {

        return CTC_E_NONE;
    }
    else
    {
        /* make the node delete safe */
        tmp1 = trie->child[0].child_node;
        tmp2 = trie->child[1].child_node;
        rv = cb (trie, user_data);
    }

    if (NALPM_E_SUCCESS (rv))
    {
        rv = _trie_preorder_traverse (tmp1, cb, user_data);
    }
    if (NALPM_E_SUCCESS (rv))
    {
        rv = _trie_preorder_traverse (tmp2, cb, user_data);
    }
    return rv;
}

    STATIC int
_trie_postorder_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;

    if (trie == NULL)
    {
        return CTC_E_NONE;
    }

    if (NALPM_E_SUCCESS (rv))
    {
        rv =
            _trie_postorder_traverse (trie->child[0].child_node, cb, user_data);
    }
    if (NALPM_E_SUCCESS (rv))
    {
        rv =
            _trie_postorder_traverse (trie->child[1].child_node, cb, user_data);
    }
    if (NALPM_E_SUCCESS (rv))
    {
        rv = cb (trie, user_data);
    }
    return rv;
}

    STATIC int
_trie_inorder_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *tmp = NULL;

    if (trie == NULL)
    {
        return CTC_E_NONE;
    }

    if (NALPM_E_SUCCESS (rv))
    {
        rv = _trie_inorder_traverse (trie->child[0].child_node, cb, user_data);
    }

    /* make the trie pointers delete safe */
    tmp = trie->child[1].child_node;

    if (NALPM_E_SUCCESS (rv))
    {
        rv = cb (trie, user_data);
    }

    if (NALPM_E_SUCCESS (rv))
    {
        rv = _trie_inorder_traverse (tmp, cb, user_data);
    }
    return rv;
}
/*********modification begin*********
  * reason: dump traverse
  * mender: cuixl
  *   date: 2017-02-21 10:58
  */
    STATIC int
_trie_dump_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *tmp1, *tmp2;
    trie_dump_node_t *p_dump_node = (trie_dump_node_t *)user_data;
    trie_dump_node_t dump_node;

    if (trie == NULL || !cb || !p_dump_node)
    {
        return CTC_E_NONE;
    }
    else
    {
        /* make the node delete safe */
        tmp1 = trie->child[0].child_node;
        tmp2 = trie->child[1].child_node;
        sal_memcpy(&dump_node, p_dump_node, sizeof(trie_dump_node_t));
        rv = cb (trie, &dump_node);
    }

    if ((NULL != tmp1) && NALPM_E_SUCCESS (rv))
    {
        dump_node.father = trie;
        dump_node.father_lvl = p_dump_node->father_lvl + 1;
        rv = _trie_dump_traverse (tmp1, cb, &dump_node);
    }

    if ((NULL != tmp2) && NALPM_E_SUCCESS (rv))
    {
        dump_node.father = trie;
        dump_node.father_lvl = p_dump_node->father_lvl + 1;
        rv = _trie_dump_traverse (tmp2, cb, &dump_node);
    }
    return rv;
}
/*********modification end*********/

    STATIC int
_trie_clone_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *tmp1, *tmp2;
    sys_nalpm_clone_para_t* p_para = (sys_nalpm_clone_para_t*)user_data;
    sys_nalpm_clone_para_t para;

    if (trie == NULL || !cb)
    {

        return CTC_E_NONE;
    }
    else
    {
        tmp1 = trie->child[0].child_node;
        tmp2 = trie->child[1].child_node;
        sal_memcpy(&para, p_para, sizeof(sys_nalpm_clone_para_t));
        rv = cb (trie, &para);
    }

    if (NALPM_E_SUCCESS (rv))
    {
        para.bit = 0;
        rv = _trie_clone_traverse (tmp1, cb, &para);
    }
    if (NALPM_E_SUCCESS (rv))
    {
        para.bit = 1;
        rv = _trie_clone_traverse (tmp2, cb, &para);
    }
    return rv;
}

    STATIC int
_trie_clear_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *tmp1, *tmp2;

    if (trie == NULL || !cb)
    {
        return CTC_E_NONE;
    }
    else
    {
        /* make the node delete safe */
        tmp1 = trie->child[0].child_node;
        tmp2 = trie->child[1].child_node;
        rv = cb (trie, NULL);
    }

    if (NALPM_E_SUCCESS (rv))
    {
        rv = _trie_clear_traverse (tmp1, cb, NULL);
    }
    if (NALPM_E_SUCCESS (rv))
    {
        rv = _trie_clear_traverse (tmp2, cb, NULL);
    }

    return rv;
}

STATIC int
_trie_arrange_fragment_traverse(trie_node_t * trie, trie_callback_f cb, void *user_data)
{
    trie_node_t *tmp1, *tmp2;


    if (trie == NULL || !cb)
    {

        return CTC_E_NONE;
    }
    else
    {
        /* make the node delete safe */
        tmp1 = trie->child[0].child_node;
        tmp2 = trie->child[1].child_node;
        cb (trie, user_data);
    }

    if (1)
    {
        _trie_arrange_fragment_traverse (tmp1, cb, user_data);
    }
    if (1)
    {
        _trie_arrange_fragment_traverse (tmp2, cb, user_data);
    }
    return CTC_E_NONE;
}


STATIC int
_trie_preorder_ad_route_traverse (trie_node_t * trie,trie_callback_f cb,
        void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *tmp1, *tmp2;

    if (trie == NULL || !cb)
    {

        return CTC_E_NONE;
    }
    else
    {
        /* make the node delete safe */
        tmp1 = trie->child[0].child_node;
        tmp2 = trie->child[1].child_node;
        rv = cb (trie, user_data);
    }

    if (NALPM_E_SUCCESS (rv))
    {
        _trie_preorder_ad_route_traverse (tmp1, cb, user_data);
    }
    if (NALPM_E_SUCCESS (rv))
    {
        _trie_preorder_ad_route_traverse (tmp2, cb, user_data);
    }
    return CTC_E_NONE;
}

    STATIC int
_trie_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data, trie_traverse_order_e_t order)
{
    int rv = CTC_E_NONE;

    switch (order)
    {
        case _TRIE_PREORDER_TRAVERSE:
            rv = _trie_preorder_traverse (trie, cb, user_data);
            break;
        case _TRIE_POSTORDER_TRAVERSE:
            rv = _trie_postorder_traverse (trie, cb, user_data);
            break;
        case _TRIE_INORDER_TRAVERSE:
            rv = _trie_inorder_traverse (trie, cb, user_data);
            break;
/*********modification begin*********
  * reason: dump traverse
  * mender: cuixl
  *   date: 2017-02-21 10:56
  */
        case _TRIE_DUMP_TRAVERSE:
            rv = _trie_dump_traverse (trie, cb, user_data);
            break;
/*********modification end*********/
        case _TRIE_CLONE_TRAVERSE:
            rv = _trie_clone_traverse (trie, cb, user_data);
            break;
        case _TRIE_CLEAR_TRAVERSE:
            rv = _trie_clear_traverse (trie, cb, user_data);
            break;
        case _TRIE_ARRANGE_FRAGMENT_TRAVERSE:
            rv = _trie_arrange_fragment_traverse(trie, cb, user_data);
            break;
        case _TRIE_PREORDER_AD_ROUTE_TRAVERSE:
            rv = _trie_preorder_ad_route_traverse(trie, cb, user_data);
            break;
        default:
            /*assert (0);*/
            break;
    }

    return rv;
}

/*
 * Function:
 *     sys_nalpm_trie_traverse
 * Purpose:
 *     Traverse the trie & call the application callback with user data
 */
    int
sys_nalpm_trie_traverse (trie_node_t * trie, trie_callback_f cb,
        void *user_data, trie_traverse_order_e_t order)
{
    if (order < _TRIE_PREORDER_TRAVERSE || order >= _TRIE_TRAVERSE_MAX || !cb)
        return CTC_E_INVALID_PARAM;

    if (trie == NULL)
    {
        return CTC_E_NONE;
    }
    else
    {
        return _trie_traverse (trie, cb, user_data, order);
    }
}

    STATIC int
_trie_dump (trie_node_t * trie, trie_callback_f cb,
        void *user_data, unsigned int level)
{
    if (trie == NULL)
    {
        if (cb)
        {
            cb (trie, user_data);
        }
        return CTC_E_NONE;
    }
    else
    {
        if (cb)
        {
            cb (trie, user_data);
        }
        else
        {
            unsigned int lvl = level;
            while (lvl)
            {
                if (lvl == 1)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|-");
                }
                else
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "| ");
                }
                lvl--;
            }
            sys_nalpm_print_trie_node (trie, NULL);
        }
    }

    _trie_dump (trie->child[0].child_node, cb, user_data, level + 1);
    _trie_dump (trie->child[1].child_node, cb, user_data, level + 1);
    return CTC_E_NONE;
}

/*
 * Function:
 *     sys_nalpm_trie_dump
 * Purpose:
 *     Dumps the trie pre-order [root|left|child]
 */
    int
sys_nalpm_trie_dump (trie_t * trie, trie_callback_f cb, void *user_data)
{
    if (trie->trie)
    {
        return _trie_dump (trie->trie, cb, user_data, 0);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
}

/*
 * Internal function for LPM match searching.
 * callback on all payload nodes if cb != NULL.
 */
    STATIC int
_trie_find_lpm (trie_node_t *parent,
        trie_node_t * trie,
        unsigned int *key,
        unsigned int length,
        trie_node_t ** payload, trie_callback_f cb, void *user_data)
{
    unsigned int lcp = 0;
    int bit = 0, rv = CTC_E_NONE;

    if (!trie)
        return CTC_E_INVALID_PARAM;

    lcp = _lcplen (key, length, trie->skip_addr, trie->skip_len);

    if ((length > trie->skip_len) && (lcp == trie->skip_len))
    {
        if (trie->type == PAYLOAD)
        {
            /* lpm cases */
            if (payload != NULL  && 1 != trie->dirty_flag)
            {
                /* update lpm result */
                *payload = trie;
                parent = trie;
            }
            else if(payload != NULL  && 1 == trie->dirty_flag)
            {
                *payload = parent;
            }

            if (cb != NULL)
            {
                /* callback with any nodes which is shorter and matches the prefix */
                rv = cb (trie, user_data);
                if (NALPM_E_FAILURE (rv))
                {
                    /* early bailout if there is error in callback handling */
                    return rv;
                }
            }
        }

        bit = (key[KEY_BIT2IDX (length - lcp)] &
                SHL (1, (length - lcp - 1) % _NUM_WORD_BITS_,
                    _NUM_WORD_BITS_)) ? 1 : 0;

        /* based on next bit branch left or right */
        if (trie->child[bit].child_node)
        {
            return _trie_find_lpm (parent, trie->child[bit].child_node, key,
                    length - lcp - 1, payload, cb, user_data);
        }
    }
    else if ((length == trie->skip_len) && (lcp == length))
    {
        if (trie->type == PAYLOAD)
        {
            /* exact match case */
            if (payload != NULL)
            {
                /* lpm is exact match */
                *payload = (1 == trie->dirty_flag)?parent:trie;
            }

            if (cb != NULL)
            {
                /* callback with the exact match node */
                rv = cb (trie, user_data);
                if (NALPM_E_FAILURE (rv))
                {
                    /* early bailout if there is error in callback handling */
                    return rv;
                }
            }
        }
    }
    return rv;
}

STATIC void _sys_trie_key_convert_to_lsb(uint32* key, uint32 length, uint32 is_v6)
{
    if (is_v6)
    {
        if (length <= 32)
        {
            key[3] = (key[0] >> (32 - length));
            key[0] = 0;
        }
        else if(32 < length && length < 64)
        {
            key[3] = (key[1] >> (64 - length)) + (key[0] << (32 - (64 - length)));
            key[2] = (key[0] >> (64 - length));
            key[0] = 0;
            key[1] = 0;
        }
        else if(length == 64)
        {
            key[3] = key[1];
            key[2] = key[0];
            key[1] = 0;
            key[0] = 0;
        }
        else if(64 < length && length  < 96)
        {
            key[3] = (key[2] >> (96 - length)) + (key[1] << (32 - (96 - length)));
            key[2] = (key[1] >> (96 - length)) + (key[0] << (32 - (96 - length)));
            key[1] = (key[0] >> (96 - length));
            key[0] = 0;
        }
        else if(length == 96)
        {
            key[3] = key[2];
            key[2] = key[1];
            key[1] = key[0];
            key[0] = 0;
        }
        else if(96 < length && length  < 128)
        {
            key[3] = (key[3] >> (128 - length)) + (key[2] << (32 - (128 - length)));
            key[2] = (key[2] >> (128 - length)) + (key[1] << (32 - (128 - length)));
            key[1] = (key[1] >> (128 - length)) + (key[0] << (32 - (128 - length)));
            key[0] = (key[0] >> (128 - length));
        }
    }
    else
    {
        key[0] = key[0] >> (32 - length);
    }

    return;
}
/*
 * Function:
 *     sys_nalpm_trie_find_lpm
 * Purpose:
 *     Find the longest prefix matched with given prefix
 */
    int
sys_nalpm_trie_find_lpm (trie_t * trie,
        unsigned int *key, unsigned int length, trie_node_t ** payload, uint8 is_route)
{
    *payload = NULL;

    if(is_route)
    {
        _sys_trie_key_convert_to_lsb(key, length, trie->v6_key);
    }

    if (trie->trie)
    {
        if (trie->v6_key)
        {
            return sys_trie_v6_find_lpm (trie->trie, trie->trie, key, length, payload, NULL, NULL);
        }
        else
        {
            return _trie_find_lpm (trie->trie, trie->trie, key, length, payload, NULL, NULL);
        }
    }

    return CTC_E_NOT_EXIST;
}

/*
 * Function:
 *   _trie_skip_node_alloc
 * Purpose:
 *   create a chain of trie_node_t that has the payload at the end.
 *   each node in the chain can skip upto _MAX_SKIP_LEN number of bits,
 *   while the child pointer in the chain represent 1 bit. So totally
 *   each node can absorb (_MAX_SKIP_LEN+1) bits.
 * Input:
 *   key      --
 *   bpm      --
 *   msb      --
 *   skip_len --  skip_len of the whole chain
 *   payload  --  payload node we want to insert
 *   count    --  child count
 * Output:
 *   node     -- return pointer of the starting node of the chain.
 */
STATIC int
_trie_skip_node_alloc (trie_node_t ** node, unsigned int *key,
        /* bpm bit map if bpm management is required, passing null skips bpm management */
        unsigned int *bpm, unsigned int msb, /* NOTE: valid msb position 1 based, 0 means skip0/0 node */
        unsigned int skip_len, trie_node_t * payload, unsigned int count) /* payload count underneath - mostly 1 except some tricky cases */
{
    int lsb = 0, msbpos = 0, lsbpos = 0, bit = 0, index;
    trie_node_t *child = NULL, *skip_node = NULL;

    /* calculate lsb bit position, also 1 based */
    lsb = ((msb) ? msb + 1 - skip_len : msb); /* msb >= skip_len >= 0, so lsb >= 0*/

    /*assert (((int) msb >= 0) && (lsb >= 0));*/

    if (!node || !key || !payload || msb > _MAX_KEY_LEN_ || msb < skip_len)
        return CTC_E_INVALID_PARAM;

    if (msb)
    {
        for (index = BITS2SKIPOFF (lsb), lsbpos = lsb - 1; index <= BITS2SKIPOFF (msb); index++)
        {
            /* each loop process _MAX_SKIP_LEN number of bits?? */
            if (lsbpos == lsb - 1)
            {
                /* (lsbpos == lsb-1) is only true for first node (loop) here */
                skip_node = payload;
            }
            else
            {
                /* other nodes need to be created */
                skip_node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
            }

            /* init memory */
            sal_memset (skip_node, 0, sizeof (trie_node_t));

            /* calculate msb bit position of current chunk of bits we are processing */
            msbpos = index * _MAX_SKIP_LEN_ - 1; /*msbpos = 30*/
            if (msbpos > msb - 1)
                msbpos = msb - 1;

            /* calculate the skip_len of the created node */
            if (msbpos - lsbpos < _MAX_SKIP_LEN_)
            {
                skip_node->skip_len = msbpos - lsbpos + 1;
            }
            else
            {
                skip_node->skip_len = _MAX_SKIP_LEN_;
            }

            /* calculate the skip_addr (skip_length number of bits).
             * skip might be skipping bits on 2 different words
             * if msb & lsb spawns 2 word boundary in worst case
             */
            if (NALPM_BITS2WORDS (msbpos + 1) != NALPM_BITS2WORDS (lsbpos + 1))
            {
                /* pull snippets from the different words & fuse */
                skip_node->skip_addr = key[KEY_BIT2IDX (msbpos + 1)] & MASK ((msbpos + 1) % _NUM_WORD_BITS_);
                skip_node->skip_addr = SHL (skip_node->skip_addr, skip_node->skip_len - ((msbpos + 1) % _NUM_WORD_BITS_), _NUM_WORD_BITS_);
                skip_node->skip_addr |= SHR (key[KEY_BIT2IDX (lsbpos + 1)], (lsbpos % _NUM_WORD_BITS_), _NUM_WORD_BITS_);
            }
            else
            {
                skip_node->skip_addr =  SHR (key[KEY_BIT2IDX (msbpos + 1)], (lsbpos % _NUM_WORD_BITS_), _NUM_WORD_BITS_);
            }

            /* set up the chain of child pointer, first node has no child since "child" was inited to NULL */
            if (child)
            {
                skip_node->child[bit].child_node = child;
            }

            /* calculate child pointer for next loop. NOTE: skip_addr has not been masked
             * so we still have the child bit in the skip_addr here.
             */
            bit = (skip_node->skip_addr & SHL (1, skip_node->skip_len - 1, _MAX_SKIP_LEN_)) ? 1 : 0;

            /* calculate node type */
            if (lsbpos == lsb - 1)
            {
                /* first node is payload */
                skip_node->type = PAYLOAD;
            }
            else
            {
                /* other nodes are internal nodes */
                skip_node->type = INTERNAL;
            }

            /* all internal nodes will have the same "count" as the payload node */
            skip_node->count = count;

            /* advance lsb to next word */
            lsbpos += skip_node->skip_len;

            /* calculate bpm of the skip_node */
            if (bpm)
            {
                if (lsbpos == _MAX_KEY_LEN_)
                {
                    /* parent node is 0/0, so there is no branch bit here */
                    skip_node->bpm =
                        _key_get_bits (bpm, lsbpos, skip_node->skip_len, 1);
                }
                else
                {
                    skip_node->bpm =
                        _key_get_bits (bpm, lsbpos + 1, skip_node->skip_len + 1,
                                1);
                }
            }

            /* for all child nodes 0/1 is implicitly obsorbed on parent */
            if (msbpos != msb - 1)
            {
                /* msbpos == (msb-1) is only true for the first node */
                skip_node->skip_len--;
            }


            skip_node->skip_addr &= MASK (skip_node->skip_len);
            child = skip_node;
        }
    }
    else
    {
        /* skip_len == 0 case, create a payload node with skip_len = 0 and bpm should be 1 bits only
         * bit 0 and bit "skip_len" are same bit (bit 0).
         */
        skip_node = payload;
        sal_memset (skip_node, 0, sizeof (trie_node_t));
        skip_node->type = PAYLOAD;
        skip_node->count = count;
        if (bpm)
        {
            skip_node->bpm = _key_get_bits (bpm, 1, 1, 0);
        }
    }

    *node = skip_node;
    return CTC_E_NONE;
}

STATIC int
_trie_insert (trie_node_t * trie, unsigned int *key,
        /* bpm bit map if bpm management is required, passing null skips bpm management */
        unsigned int *bpm, unsigned int length, trie_node_t * payload,/* payload node */
        trie_node_t ** child /* child pointer if the child is modified */ )
{
    unsigned int lcp;
    int rv = CTC_E_NONE, bit = 0;
    trie_node_t *node = NULL;

    if (!trie || !payload || !child || (length > _MAX_KEY_LEN_))
        return CTC_E_INVALID_PARAM;

    *child = NULL;


    lcp = _lcplen (key, length, trie->skip_addr, trie->skip_len);

    /* insert cases:
     * 1 - new key could be the parent of existing node
     * 2 - new node could become the child of a existing node
     * 3 - internal node could be inserted and the key becomes one of child
     * 4 - internal node is converted to a payload node */

    /* if the new key qualifies as new root do the inserts here */
    if (lcp == length)   /* insert shorter prefix*/
    {       /* guaranteed: length < _MAX_SKIP_LEN_ */
        if (trie->skip_len == lcp)
        {
            if (trie->type != INTERNAL)
            {
                /* duplicate */
                return CTC_E_EXIST;
            }
            else
            {
                /* change the internal node to payload node */
                _CLONE_TRIE_NODE_ (payload, trie);
                mem_free (trie);
                payload->type = PAYLOAD;
                payload->count++;
                *child = payload;

                if (bpm)
                {
                    /* bpm at this internal mode must be same as the inserted pivot */
                    payload->bpm |= _key_get_bits (bpm, lcp + 1, lcp + 1, 1);
                    /* implicity preserve the previous bpm & set bit 0 -myself bit */
                }
                return CTC_E_NONE;
            }
        }
        else
        {/* skip length can never be less than lcp implcitly here */
            /* this node is new parent for the old trie node */
            /* lcp is the new skip length */
            _CLONE_TRIE_NODE_ (payload, trie);
            *child = payload;

            bit =
                (trie->skip_addr & SHL (1, trie->skip_len - length - 1,
                                        _MAX_SKIP_LEN_)) ? 1 : 0;
            trie->skip_addr &= MASK (trie->skip_len - length - 1);
            trie->skip_len -= (length + 1);
            /* clone from trie node but dirty_flag should initialize as 0 */
            payload->dirty_flag = 0;

            if (bpm)
            {
                trie->bpm &= MASK (trie->skip_len + 1);
            }

            payload->skip_addr = (length > 0) ? key[KEY_BIT2IDX (length)] : 0;
            payload->skip_addr &= MASK (length);
            payload->skip_len = length;
            payload->child[bit].child_node = trie;
            payload->child[!bit].child_node = NULL;
            payload->type = PAYLOAD;
            payload->count++;

            if (bpm)
            {
                payload->bpm =
                    SHR (payload->bpm, trie->skip_len + 1, _NUM_WORD_BITS_);
                payload->bpm |=
                    _key_get_bits (bpm, payload->skip_len + 1,
                            payload->skip_len + 1, 1);
            }
        }
    }
    else if (lcp == trie->skip_len)  /* insert longer prefix*/
    {
        /* key length is implictly greater than lcp here */
        /* decide based on key's next applicable bit */
        bit = (key[KEY_BIT2IDX (length - lcp)] & SHL (1, (length - lcp - 1) % _NUM_WORD_BITS_, _NUM_WORD_BITS_)) ? 1 : 0;

        if (!trie->child[bit].child_node)
        {
            /* the key is going to be one of the child of existing node */
            /* should be the child */
            rv = _trie_skip_node_alloc (&node, key, bpm, length - lcp - 1,/* 0 based msbit position */
                    length - lcp - 1, payload, 1);
            if (NALPM_E_SUCCESS (rv))
            {
                trie->child[bit].child_node = node;
                trie->count++;
            }
            else
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n Error on trie skip node allocaiton [%d]!!!!\n", rv);
            }
        }
        else
        {
            rv = _trie_insert (trie->child[bit].child_node, key, bpm, length - lcp - 1, payload, child);
            if (NALPM_E_SUCCESS (rv))
            {
                trie->count++;
                if (*child != NULL)
                {/* chande the old child pointer to new child */
                    trie->child[bit].child_node = *child;
                    *child = NULL;
                }
            }
        }
    }
    else
    {
        trie_node_t *newchild = NULL;

        /* need to introduce internal nodes */
        node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
        _CLONE_TRIE_NODE_ (node, trie);
        /* clone from trie node but dirty_flag should initialize as 0 */
        node->dirty_flag = 0;

        rv = _trie_skip_node_alloc (&newchild, key, bpm,
                ((lcp) ? length - lcp - 1 : length - 1),
                length - lcp - 1, payload, 1);
        if (NALPM_E_SUCCESS (rv))
        {
            bit = (key[KEY_BIT2IDX (length - lcp)] &
                    SHL (1, (length - lcp - 1) % _NUM_WORD_BITS_,
                        _NUM_WORD_BITS_)) ? 1 : 0;

            node->child[!bit].child_node = trie;
            node->child[bit].child_node = newchild;
            node->type = INTERNAL;
            node->skip_addr =
                SHR (trie->skip_addr, trie->skip_len - lcp, _MAX_SKIP_LEN_);
            node->skip_len = lcp;
            node->count++;
            if (bpm)
            {
                node->bpm =
                    SHR (node->bpm, trie->skip_len - lcp, _MAX_SKIP_LEN_);
            }
            *child = node;

            trie->skip_addr &= MASK (trie->skip_len - lcp - 1);
            trie->skip_len -= (lcp + 1);
            if (bpm)
            {
                trie->bpm &= MASK (trie->skip_len + 1);
            }
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n Error on trie skip node allocaiton [%d]!!!!\n",
                    rv);
            mem_free (node);
        }
    }

    return rv;
}

/*
 * Function:
 *     sys_nalpm_trie_insert
 * Purpose:
 *     Inserts provided prefix/length in to the trie
 */
    int
sys_nalpm_trie_insert (trie_t * trie, unsigned int *key, unsigned int *bpm, unsigned int length, trie_node_t * payload, uint8 is_route)
{
    int rv = CTC_E_NONE;
    trie_node_t *child = NULL;

    if (!trie)
        return CTC_E_INVALID_PARAM;

    if(is_route)
    {
        _sys_trie_key_convert_to_lsb(key, length, trie->v6_key);
    }

    if (trie->trie == NULL)
    {
        if (trie->v6_key)
        {
            rv = sys_trie_v6_skip_node_alloc (&trie->trie, key, bpm, length, length, payload, 1);
        }
        else
        {
            rv = _trie_skip_node_alloc (&trie->trie, key, bpm, length, length, payload, 1);
        }
    }
    else
    {
        if (trie->v6_key)
        {
            rv =
                sys_trie_v6_insert (trie->trie, key, bpm, length, payload, &child);
        }
        else
        {
            rv = _trie_insert (trie->trie, key, bpm, length, payload, &child);
        }
        if (child)
        {           /* chande the old child pointer to new child */
            trie->trie = child;
        }
    }

    return rv;
}

    int
sys_nalpm_trie_fuse_child (trie_node_t * trie, int bit)
{
    trie_node_t *child = NULL;
    int rv = CTC_E_NONE;

    if (trie->child[0].child_node && trie->child[1].child_node)
    {
        return CTC_E_INVALID_PARAM;
    }

    bit = (bit > 0) ? 1 : 0;
    child = trie->child[bit].child_node;

    if (child == NULL)
    {
        return CTC_E_INVALID_PARAM;
    }
    else
    {
        if (trie->skip_len + child->skip_len + 1 <= _MAX_SKIP_LEN_)
        {

            if (trie->skip_len == 0)
                trie->skip_addr = 0;

            if (child->skip_len < _MAX_SKIP_LEN_)
            {
                trie->skip_addr =
                    SHL (trie->skip_addr, child->skip_len + 1, _MAX_SKIP_LEN_);
            }

            trie->skip_addr |= SHL (bit, child->skip_len, _MAX_SKIP_LEN_);
            child->skip_addr |= trie->skip_addr;
            child->bpm |= SHL (trie->bpm, child->skip_len + 1, _MAX_SKIP_LEN_);
            child->skip_len += trie->skip_len + 1;
            /* do not free payload nodes as they are user managed */
            if (trie->type == INTERNAL)
            {
                mem_free (trie);
            }

        }
    }

    return rv;
}

    STATIC int
_trie_delete (trie_node_t * trie,
        unsigned int *key,
        unsigned int length,
        trie_node_t ** payload, trie_node_t ** child)
{
    unsigned int lcp;
    int rv = CTC_E_NONE, bit = 0;
    trie_node_t *node = NULL;

    /* our algorithm should return before the length < 0, so this means
     * something wrong with the trie structure. Internal error?
     */
    if (!trie || !payload || !child || (length > _MAX_KEY_LEN_))
    {
        return CTC_E_INVALID_PARAM;
    }

    *child = NULL;

    /* check a section of key, return the number of matched bits and value of next bit */
    lcp = _lcplen (key, length, trie->skip_addr, trie->skip_len);

    if (length > trie->skip_len)
    {

        if (lcp == trie->skip_len)
        {

            bit = (key[KEY_BIT2IDX (length - lcp)] &
                    SHL (1, (length - lcp - 1) % _NUM_WORD_BITS_,
                        _NUM_WORD_BITS_)) ? 1 : 0;

            /* based on next bit branch left or right */
            if (trie->child[bit].child_node)
            {

                /* has child node, keep searching */
                rv =
                    _trie_delete (trie->child[bit].child_node, key,
                            length - lcp - 1, payload, child);

                if (rv == CTC_E_IN_USE)
                {

                    trie->child[bit].child_node = NULL;	/* mem_free the child */
                    rv = CTC_E_NONE;
                    trie->count--;

                    if (trie->type == INTERNAL)
                    {

                        bit = (bit == 0) ? 1 : 0;

                        if (trie->child[bit].child_node == NULL)
                        {
                            /* parent and child connected, mem_free the middle-node itself */
                            mem_free (trie);
                            rv = CTC_E_IN_USE;
                        }
                        else
                        {
                            /* fuse the parent & child */
                            if (trie->skip_len +
                                    trie->child[bit].child_node->skip_len + 1 <=
                                    _MAX_SKIP_LEN_)
                            {
                                *child = trie->child[bit].child_node;
                                rv = sys_nalpm_trie_fuse_child (trie, bit);
                                if (rv != CTC_E_NONE)
                                {
                                    *child = NULL;
                                }
                            }
                        }
                    }
                }
                else if (NALPM_E_SUCCESS (rv))
                {
                    trie->count--;
                    /* update child pointer if applicable */
                    if (*child != NULL)
                    {
                        trie->child[bit].child_node = *child;
                        *child = NULL;
                    }
                }
            }
            else
            {
                /* no child node case 0: not found */
                rv = CTC_E_NOT_EXIST;
            }

        }
        else
        {
            /* some bits are not matching, case 0: not found */
            rv = CTC_E_NOT_EXIST;
        }
    }
    else if (length == trie->skip_len)
    {
        /* when length equal to skip_len, unless this is a payload node
         * and it's an exact match (lcp == length), we can not found a match
         */
        if (!((lcp == length) && (trie->type == PAYLOAD)))
        {
            rv = CTC_E_NOT_EXIST;
        }
        else
        {
            /* payload node can be deleted */
            /* if this node has 2 children update it to internal node */
            rv = CTC_E_NONE;

            if (trie->child[0].child_node && trie->child[1].child_node)
            {
                /* the node has 2 children, update it to internal node */
                node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
                _CLONE_TRIE_NODE_ (node, trie);
                node->type = INTERNAL;
                node->count--;
                *child = node;
            }
            else if (trie->child[0].child_node || trie->child[1].child_node)
            {
                /* if this node has 1 children fuse the children with this node */
                bit = (trie->child[0].child_node) ? 0 : 1;
                trie->count--;
                if (trie->skip_len + trie->child[bit].child_node->skip_len +
                        1 <= _MAX_SKIP_LEN_)
                {
                    /* able to fuse the node with its child node */
                    *child = trie->child[bit].child_node;
                    rv = sys_nalpm_trie_fuse_child (trie, bit);
                    if (rv != CTC_E_NONE)
                    {
                        *child = NULL;
                    }
                }
                else
                {
                    /* convert it to internal node, we need to alloc new memory for internal nodes
                     * since the old payload node memory will be freed by caller
                     */
                    node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
                    _CLONE_TRIE_NODE_ (node, trie);
                    node->type = INTERNAL;
                    *child = node;
                }
            }
            else
            {
                rv = CTC_E_IN_USE;
            }

            *payload = trie;
        }
    }
    else
    {
        /* key length is shorter, no match if it's internal node,
         * will not exact match even if this is a payload node
         */
        rv = CTC_E_NOT_EXIST;	/* case 0: not found */
    }

    return rv;
}

/*
 * Function:
 *     sys_nalpm_trie_delete
 * Purpose:
 *     Deletes provided prefix/length in to the trie
 */
    int
sys_nalpm_trie_delete (trie_t * trie,
        unsigned int *key, unsigned int length, trie_node_t ** payload, uint8 is_route)
{
    int rv = CTC_E_NONE;
    trie_node_t *child = NULL;
    if(NULL == trie)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(is_route)
    {
        _sys_trie_key_convert_to_lsb(key, length, trie->v6_key);
    }

    if (trie->trie)
    {
        if (trie->v6_key)
        {
            rv = sys_trie_v6_delete (trie->trie, key, length, payload, &child);
        }
        else
        {
            rv = _trie_delete (trie->trie, key, length, payload, &child);
        }
        if (rv == CTC_E_IN_USE)
        {
            /* the head node of trie was deleted, reset trie pointer to null */
            trie->trie = NULL;
            rv = CTC_E_NONE;
        }
        else if (rv == CTC_E_NONE && child != NULL)
        {
            trie->trie = child;
        }
    }
    else
    {
        rv = CTC_E_NOT_EXIST;
    }
    return rv;
}

/*
 * Function:
 *     _trie_split
 * Purpose:
 *     Split the trie into 2 based on optimum pivot
 * NOTE:
 *     max_split_len -- split will make sure the split point
 *                has a length shorter or equal to the max_split_len
 *                unless this will cause a no-split (all prefixs
 *                stays below the split point)
 *     split_to_pair -- used only when the split point will be
 *                used to create a pair of tries later (i.e: dbucket
 *                pair. we assume the split point itself will always be
 *                put into 0* trie if itself is a payload/prefix)
 */
    STATIC int
_trie_split (trie_node_t * trie,
        unsigned int *pivot,
        unsigned int *length,
        unsigned int *split_count,
        trie_node_t ** split_node,
        trie_node_t ** child,
        const unsigned int max_count,
        const unsigned int max_split_len,
        const int split_to_pair,
        unsigned int *bpm, trie_split_states_e_t * state)
{
    int bit = 0, rv = CTC_E_NONE;

    if (!trie || !pivot || !length || !split_node || max_count == 0 || !state)
        return CTC_E_INVALID_PARAM;

    if (trie->child[0].child_node && trie->child[1].child_node)
    {
        bit = (trie->child[0].child_node->count >
                trie->child[1].child_node->count) ? 0 : 1;
    }
    else
    {
        bit = (trie->child[0].child_node) ? 0 : 1;
    }

    /* start building the pivot */
    rv = _key_append (pivot, length, trie->skip_addr, trie->skip_len);
    if (NALPM_E_FAILURE (rv))
        return rv;

    if (bpm)
    {
        unsigned int scratch = 0;
        rv = _bpm_append (bpm, &scratch, trie->bpm, trie->skip_len + 1);
        if (NALPM_E_FAILURE (rv))
            return rv;
    }

    {
        /*
         * split logic to make sure the split length is shorter than the
         * requested max_split_len, unless we don't actully split the
         * tree if we stop here.
         * if (*length > max_split_len) && (trie->count != max_count) {
         *    need to split at or above this node. might need to split the node in middle
         * } else if ((ABS(child count*2 - max_count) > ABS(count*2 - max_count)) ||
         *            ((*length == max_split_len) && (trie->count != max_count))) {
         *    (the check above imply trie->count != max_count, so also imply *length < max_split_len)
         *    need to split at this node.
         * } else {
         *    keep searching, will be better split at longer pivot.
         * }
         */
        if ((*length > max_split_len) && (trie->count != max_count))
        {
            /* the pivot is getting too long, we better split at this node for
             * better bucket capacity efficiency if we can. We can split if
             * the trie node has a count != max_count, which means the
             * resulted new trie will not have all pivots (FULL)
             */
            if ((TRIE_SPLIT_STATE_PAYLOAD_SPLIT == *state) && (trie->type == INTERNAL))
            {
                *state = TRIE_SPLIT_STATE_PAYLOAD_SPLIT_DONE;
            }
            else
            {
                if (((*length - max_split_len) > trie->skip_len)
                        && (trie->skip_len == 0))
                {
                    /* the length is longer than max_split_len, and the trie->skip_len is 0,
                     * so the best we can do is use the node as the split point
                     */
                    *split_node = trie;
                    *split_count = trie->count;

                    *state = TRIE_SPLIT_STATE_PRUNE_NODES;
                    return rv;
                }

                /* we need to insert a node and use it as split point */
                *split_node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
                sal_memset ((*split_node), 0, sizeof (trie_node_t));
                (*split_node)->type = INTERNAL;
                (*split_node)->count = trie->count;

                if ((*length - max_split_len) > trie->skip_len)
                {
                    /* the length is longer than the max_split_len, and the trie->skip_len is
                     * shorter than the difference (max_split_len pivot is not covered by this
                     * node but covered by its parent, the best we can do is split at the branch
                     * lead to this node. we insert a skip_len=0 node and use it as split point
                     */
                    (*split_node)->skip_len = 0;
                    (*split_node)->skip_addr = 0;
                    (*split_node)->bpm = (trie->bpm >> trie->skip_len);

                    if (_BITGET (trie->skip_addr, (trie->skip_len - 1)))
                    {
                        (*split_node)->child[1].child_node = trie;
                    }
                    else
                    {
                        (*split_node)->child[0].child_node = trie;
                    }

                    /* the split point is with length max_split_len */
                    *length -= trie->skip_len;

                    /* update the current node to reflect the node inserted */
                    trie->skip_len = trie->skip_len - 1;
                }
                else
                {
                    /* the length is longer than the max_split_len, and the trie->skip_len is
                     * longer than the difference (max_split_len pivot is covered by this
                     * node, we insert a node with length = max_split_len and use it as split point
                     */
                    (*split_node)->skip_len =
                        trie->skip_len - (*length - max_split_len);
                    (*split_node)->skip_addr =
                        (trie->skip_addr >> (*length - max_split_len));
                    (*split_node)->bpm = (trie->bpm >> (*length - max_split_len));

                    if (_BITGET (trie->skip_addr, (*length - max_split_len - 1)))
                    {
                        (*split_node)->child[1].child_node = trie;
                    }
                    else
                    {
                        (*split_node)->child[0].child_node = trie;
                    }

                    /* update the current node to reflect the node inserted */
                    trie->skip_len = *length - max_split_len - 1;

                    /* the split point is with length max_split_len */
                    *length = max_split_len;
                }

                trie->skip_addr = trie->skip_addr & BITMASK (trie->skip_len);
                trie->bpm = trie->bpm & BITMASK (trie->skip_len + 1);

                /* there is no need to update the parent node's child_node pointer
                 * to the "trie" node since we will split here and the parent node's
                 * child_node pointer will be set to NULL later
                 */
                *split_count = trie->count;
                rv = _key_shift_right (bpm, trie->skip_len + 1);

                if (NALPM_E_SUCCESS (rv))
                {
                    rv = _key_shift_right (pivot, trie->skip_len + 1);
                }
                *state = TRIE_SPLIT_STATE_PRUNE_NODES;
                return rv;
            }
        }
#if 0
        else if (((*length == max_split_len) && (trie->count != max_count)) ||
                (ABS (trie->child[bit].child_node->count * 2 - max_count) >
                 ABS (trie->count * 2 - max_count)))
#else
 /*1 make sure split@*/
/*
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count * (g_sys_nalpm_master->split_mode  + 1)- max_count * g_sys_nalpm_master->split_mode) >
		 ABS (trie->count * (g_sys_nalpm_master->split_mode + 1) - max_count * g_sys_nalpm_master->split_mode)))
*/
#if 1
 /*1 split 1/2*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
         (ABS (trie->child[bit].child_node->count * 2 - max_count * 1) >
          ABS (trie->count * 2 - max_count * 1)))
#endif
#if 0
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
          ((trie->count == 7 || trie->count == 8)))
#endif
#if 0
 /*1 split @10*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count - 10) >
		 ABS (trie->count - 10)))
#endif
#if 0
 /*1 split 6/7*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count * 7 - max_count * 6) >
		 ABS (trie->count * 7 - max_count * 6)))
#endif
#if 0
 /*1 split 5/6*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count * 6 - max_count * 5) >
		 ABS (trie->count * 6 - max_count * 5)))
 #endif
#if 0
 /*1 split 3/4*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count * 4 - max_count * 3) >
		 ABS (trie->count * 4 - max_count * 3)))
#endif

#if 0
 /*1 split 2/3*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count * 3 - max_count * 2) >
		 ABS (trie->count * 3 - max_count * 2)))
#endif
#if 0
 /*1 split 1/2*/
else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
		(ABS (trie->child[bit].child_node->count * 2 - max_count) >
		 ABS (trie->count * 2 - max_count)))
#endif
#endif
        {
            /*
             * (1) when the node is at the max_split_len and if used as spliting point
             * the resulted trie will not have all pivots (FULL). we should split
             * at this node.
             * (2) when the node is at the max_split_len and if the resulted trie
             * will have all pivots (FULL), we fall through to keep searching
             * (3) when the node is shorter than the max_split_len and the node
             * has a more even pivot distribution compare to it's child, we
             * can split at this node .
             *
             * NOTE 1:
             *  ABS(trie->count * 2 - max_count) actually means
             *  ABS(trie->count - (max_count - trie->count))
             * which means the count's distance to half depth of the bucket
             * NOTE 2:
             *  when trie->count == max_count, the above check will be FALSE
             *  so here it guarrantees *length < max_split_len. We don't
             *  need to further split this node.
             */
            *split_node = trie;
            *split_count = trie->count;

            if ((TRIE_SPLIT_STATE_PAYLOAD_SPLIT == *state) &&
                    (trie->type == INTERNAL))
            {
                *state = TRIE_SPLIT_STATE_PAYLOAD_SPLIT_DONE;
            }
            else
            {
                *state = TRIE_SPLIT_STATE_PRUNE_NODES;
                return rv;
            }
        }
        else
        {
            /* we can not split at this node, keep searching, it's better to
             * split at longer pivot
             */
            rv = _key_append (pivot, length, bit, 1);
            if (NALPM_E_FAILURE (rv))
                return rv;

            rv = _trie_split (trie->child[bit].child_node,
                    pivot, length,
                    split_count, split_node,
                    child, max_count, max_split_len,
                    split_to_pair, bpm, state);
        }
    }

    /* free up internal nodes if applicable */
    switch (*state)
    {
        case TRIE_SPLIT_STATE_PAYLOAD_SPLIT_DONE:
            if (trie->type == PAYLOAD)
            {
                *state = TRIE_SPLIT_STATE_PRUNE_NODES;
                *split_node = trie;
                *split_count = trie->count;
            }
            else
            {
                /* shift the pivot to right to ignore this internal node */
                rv = _key_shift_right (pivot, trie->skip_len + 1);
                /*assert (*length >= trie->skip_len + 1);*/
                *length -= (trie->skip_len + 1);
            }
            break;

        case TRIE_SPLIT_STATE_PRUNE_NODES:
            if (trie->count == *split_count)
            {
                /* if the split point has associate internal nodes they have to
                 * be cleaned up */
                /*assert (trie->type == INTERNAL);
                assert (!(trie->child[0].child_node && trie->child[1].child_node));*/
                mem_free (trie);
            }
            else
            {
                /*assert (*child == NULL);*/
                /* fuse with child if possible */
                trie->child[bit].child_node = NULL;
                bit = (bit == 0) ? 1 : 0;
                trie->count -= *split_count;

                /* optimize more */
                if ((trie->type == INTERNAL) &&
                        (trie->skip_len +
                         trie->child[bit].child_node->skip_len + 1 <= _MAX_SKIP_LEN_))
                {
                    *child = trie->child[bit].child_node;
                    rv = sys_nalpm_trie_fuse_child (trie, bit);
                    if (rv != CTC_E_NONE)
                    {
                        *child = NULL;
                    }
                }
                *state = TRIE_SPLIT_STATE_DONE;
            }
            break;

        case TRIE_SPLIT_STATE_DONE:
            /* adjust parent's count */
            /*assert (*split_count > 0);
            assert (trie->count >= *split_count);*/

            /* update the child pointer if child was pruned */
            if (*child != NULL)
            {
                trie->child[bit].child_node = *child;
                *child = NULL;
            }
            trie->count -= *split_count;
            break;

        default:
            break;
    }

    return rv;
}

/*
 * Function:
 *     sys_nalpm_trie_split
 * Purpose:
 *     Split the trie into 2 based on optimum pivot
 * Note:
 *     we need to make sure the length is shorter than
 *     the max_split_len (for capacity optimization) if
 *     possible. We should ignore the max_split_len
 *     if that will result into trie not spliting
 */
    int
sys_nalpm_trie_split (trie_t * trie,
        const unsigned int max_split_len,
        const int split_to_pair,
        unsigned int *pivot,
        unsigned int *length,
        trie_node_t ** split_trie_root,
        unsigned int *bpm, uint8 payload_node_split)
{
    int rv = CTC_E_NONE;
    unsigned int split_count = 0, max_count = 0;
    trie_node_t *child = NULL, *node = NULL, clone;
    trie_split_states_e_t state = TRIE_SPLIT_STATE_NONE;
	uint32 t_cnt, l_cnt, r_cnt;
	trie_node_t *root;
    if (!trie || !pivot || !length || !split_trie_root)
    {
        return CTC_E_INVALID_PARAM;
    }
	root = trie->trie;
	t_cnt = trie->trie->count;
	l_cnt = (trie->trie->child[0].child_node!= NULL) ? trie->trie->child[0].child_node->count : 0;
	r_cnt = (trie->trie->child[1].child_node!= NULL) ? trie->trie->child[1].child_node->count : 0;

    *length = 0;

    if (trie->trie)
    {

        if (payload_node_split)
            state = TRIE_SPLIT_STATE_PAYLOAD_SPLIT;

        max_count = trie->trie->count;

        if (trie->v6_key)
        {
            sal_memset (pivot, 0, sizeof (unsigned int) * NALPM_BITS2WORDS (_MAX_KEY_LEN_144_));
            if (bpm)
            {
                sal_memset (bpm, 0, sizeof (unsigned int) *  NALPM_BITS2WORDS (_MAX_KEY_LEN_144_));
            }
            rv =sys_trie_v6_split (trie->trie, pivot, length, &split_count, split_trie_root, &child, max_count, max_split_len, split_to_pair, bpm, &state);
        }
        else
        {
            sal_memset (pivot, 0, sizeof (unsigned int) * NALPM_BITS2WORDS (_MAX_KEY_LEN_48_));
            if (bpm)
            {
                sal_memset (bpm, 0, sizeof (unsigned int) * NALPM_BITS2WORDS (_MAX_KEY_LEN_48_));
            }

            rv = _trie_split (trie->trie, pivot, length, &split_count, split_trie_root, &child, max_count, max_split_len, split_to_pair, bpm, &state);
        }

        if (NALPM_E_SUCCESS (rv) && (TRIE_SPLIT_STATE_DONE == state))
        {
            /* adjust parent's count */
            /*assert (split_count > 0);*/
            if (trie->trie == NULL)
            {
                trie_t *c1, *c2;
                sys_nalpm_trie_init (_MAX_KEY_LEN_48_, &c1);
                sys_nalpm_trie_init (_MAX_KEY_LEN_48_, &c2);

                c1->trie = child;
                c2->trie = *split_trie_root;
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "error:dumping the 2 child trees\n");
                sys_nalpm_trie_dump (c1, 0, 0);
                sys_nalpm_trie_dump (c2, 0, 0);
            }
            /*assert (trie->trie->count >= split_count  || (*split_trie_root)->count >= split_count);*/
            /* update the child pointer if child was pruned */
            if (child != NULL)
            {
                trie->trie = child;
            }

            sal_memcpy (&clone, *split_trie_root, sizeof (trie_node_t));
            child = *split_trie_root;

			/*********modification end*********/

            /* take advantage of thie function by passing in internal or payload node whatever
             * is the new root. If internal the function assumed it as payload node & changes type.
             * But this method is efficient to reuse the last internal or payload node possible to
             * implant the new pivot */
            if (trie->v6_key)
            {
                rv = sys_trie_v6_skip_node_alloc (&node, pivot, NULL, *length, *length, child, child->count);
            }
            else
            {
                rv = _trie_skip_node_alloc (&node, pivot, NULL,  *length, *length, child, child->count);
            }

            if (NALPM_E_SUCCESS (rv))
            {
                if (clone.type == INTERNAL)
                {
                    child->type = INTERNAL;	/* since skip alloc would have reset it to payload */
                }
                child->child[0].child_node = clone.child[0].child_node;
                child->child[1].child_node = clone.child[1].child_node;
                *split_trie_root = node;
            }
        }
        else
        {
            if (NALPM_E_SUCCESS (rv) && (TRIE_SPLIT_STATE_PRUNE_NODES == state))
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "retry[%d/%d/%d] fail[%p]\r\n", t_cnt, l_cnt, r_cnt, root);
                rv = CTC_E_NOT_READY;
            }
            else
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "!split[%d/%d/%d] fail[%p]\r\n", t_cnt, l_cnt, r_cnt, root);
            }
        }
    }
    else
    {
        rv = CTC_E_INVALID_PARAM;
    }

    return rv;
}

int
_sys_nalpm_clone_node(trie_node_t* node, void* data)
{
    sys_nalpm_clone_para_t* clone_para = (sys_nalpm_clone_para_t*)data;
    trie_node_t* new_node = NULL;

    if(node->type == INTERNAL)
    {
        new_node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
        sal_memcpy(new_node, node, sizeof(trie_node_t));
    }
    else
    {
        new_node = mem_malloc(MEM_IPUC_MODULE, sizeof (sys_nalpm_trie_payload_t));
        sal_memcpy(new_node, node, sizeof(sys_nalpm_trie_payload_t));
    }
    new_node->child[0].child_node = NULL;
    new_node->child[1].child_node = NULL;
    clone_para->father->child[clone_para->bit].child_node= new_node;
    clone_para->father = new_node;

    return CTC_E_NONE;
}

int
sys_nalpm_trie_clone(trie_t* trie, trie_t** clone_trie)
{
    sys_nalpm_clone_para_t para;
    trie_node_t* root_node = NULL;
    sal_memset (&para, 0, sizeof (sys_nalpm_clone_para_t));

    *clone_trie = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_t));
    sal_memcpy (*clone_trie, trie, sizeof (trie_t));
    if(trie->trie->type == INTERNAL)
    {
        root_node = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_node_t));
        sal_memcpy(root_node, trie->trie, sizeof(trie_node_t));
    }
    else
    {
        root_node = mem_malloc(MEM_IPUC_MODULE, sizeof (sys_nalpm_trie_payload_t));
        sal_memcpy(root_node, trie->trie, sizeof(sys_nalpm_trie_payload_t));
    }
    (*clone_trie)->trie = root_node;

    para.father = root_node;
    para.bit = 0;
    sys_nalpm_trie_traverse (trie->trie->child[0].child_node, _sys_nalpm_clone_node,(void *)&para, _TRIE_CLONE_TRAVERSE);
    para.father = root_node;
    para.bit = 1;
    sys_nalpm_trie_traverse (trie->trie->child[1].child_node, _sys_nalpm_clone_node,(void *)&para, _TRIE_CLONE_TRAVERSE);

    return CTC_E_NONE;
}

int
_sys_nalpm_clear_node(trie_node_t* node, void* data)
{
    if(node != NULL)
    {
        mem_free(node);
        node = NULL;
    }
    return CTC_E_NONE;
}

int
sys_nalpm_trie_clear(trie_node_t* node)
{
    sys_nalpm_trie_traverse (node, _sys_nalpm_clear_node, NULL, _TRIE_CLEAR_TRAVERSE);

    return CTC_E_NONE;
}

/*
 * Function:
 *     sys_nalpm_trie_init
 * Purpose:
 *     allocates a trie & initializes it
 */
    int
sys_nalpm_trie_init (unsigned int max_key_len, trie_t ** ptrie)
{
    trie_t *trie = mem_malloc(MEM_IPUC_MODULE, sizeof (trie_t));
    sal_memset (trie, 0, sizeof (trie_t));

    if (max_key_len == _MAX_KEY_LEN_48_)
    {
        trie->v6_key = FALSE;
    }
    else if (max_key_len == _MAX_KEY_LEN_144_)
    {
        trie->v6_key = TRUE;
    }
    else
    {
        mem_free (trie);
        return CTC_E_INVALID_PARAM;
    }

    trie->trie = NULL;/* means nothing is on teie */
    *ptrie = trie;
    return CTC_E_NONE;
}

/*
 * Function:
 *     sys_nalpm_trie_destroy
 * Purpose:
 *     destroys a trie
 */
    int
sys_nalpm_trie_destroy (trie_t * trie)
{

    mem_free (trie);
    trie = NULL;
    return CTC_E_NONE;
}





/*
 * Function:
 *   _trie_skip_node_free
 * Purpose:
 *   Destroy a chain of trie_node_t that has the target node at the end.
 *   The target node is not necessarily PAYLOAD type, but all nodes
 *   on the chain except for the end must have only one branch.
 * Input:
 *   key      --  target key
 *   length   --  target key length
 *   free_end --  free
 */
STATIC int _trie_skip_node_free(trie_node_t *trie,
                                unsigned int *key,
                                unsigned int length)
{
    unsigned int lcp=0;
    int bit=0, rv=CTC_E_NONE;

    if (!trie || (length && trie->skip_len && !key)) return CTC_E_INVALID_PARAM;

    lcp = _lcplen(key, length, trie->skip_addr, trie->skip_len);


    if (length > trie->skip_len) {

        if (lcp == trie->skip_len) {
            bit = (key[KEY_BIT2IDX(length - lcp)] & \
                    (1 << ((length - lcp - 1) % _NUM_WORD_BITS_))) ? 1:0;

            /* There should be only one branch on the chain until the end node */
            if (!trie->child[0].child_node == !trie->child[1].child_node) {
                return CTC_E_INVALID_PARAM;
            }

            /* based on next bit branch left or right */
            if (trie->child[bit].child_node) {
                rv = _trie_skip_node_free(trie->child[bit].child_node, key,
                        length - lcp - 1);
                if (NALPM_E_SUCCESS(rv)) {
    /*                assert(trie->type == INTERNAL); */
                    mem_free(trie);
                }
                return rv;
            } else {
                return CTC_E_NOT_EXIST; /* not found */
            }
        } else {
            return CTC_E_NOT_EXIST; /* not found */
        }
    } else if (length == trie->skip_len) {
        if (lcp == length) {
            /* the end node is not necessarily type payload. */
            /* Do not free the end */

            return CTC_E_NONE;
        }
        else return CTC_E_NOT_EXIST;
    } else {
        return CTC_E_NOT_EXIST; /* not found */
    }
}

STATIC  int
_print_trie_node (trie_node_t * trie, void *datum)
{
    if (trie != NULL)
    {

        sal_printf("trie: %p, type %s, skip_addr 0x%x skip_len %d count:%d bpm:0x%x Child[0]:%p Child[1]:%p\n",
             trie, (trie->type == PAYLOAD) ? "P" : "I", trie->skip_addr,
             trie->skip_len, trie->count, trie->bpm, trie->child[0].child_node,
             trie->child[1].child_node);
    }
    return CTC_E_NONE;
}

STATIC int _trie_search(trie_node_t *trie,
            unsigned int *key,
            unsigned int length,
            trie_node_t **payload,
            unsigned int *result_key,
            unsigned int *result_len,
            unsigned int dump,
            unsigned int find_pivot)
{
    unsigned int lcp=0;
    int bit=0, rv=CTC_E_NONE;

    if (!trie || (length && trie->skip_len && !key)) return CTC_E_INVALID_PARAM;
    if ((result_key && !result_len) || (!result_key && result_len)) return CTC_E_INVALID_PARAM;

    lcp = _lcplen(key, length, trie->skip_addr, trie->skip_len);

    if (dump) {
        _print_trie_node(trie, (unsigned int *)1);
    }

    if (length > trie->skip_len) {
        if (lcp == trie->skip_len) {
            bit = (key[KEY_BIT2IDX(length - lcp)] & \
                   (1 << ((length - lcp - 1) % _NUM_WORD_BITS_))) ? 1:0;
            if (dump) {
                sal_printf(" Length: %d Next-Bit[%d] \n", length, bit);
            }

            if (result_key) {
                rv = _key_append(result_key, result_len, trie->skip_addr, trie->skip_len);
                if (NALPM_E_FAILURE(rv)) return rv;
            }

            /* based on next bit branch left or right */
            if (trie->child[bit].child_node) {

                if (result_key) {
                    rv = _key_append(result_key, result_len, bit, 1);
                    if (NALPM_E_FAILURE(rv)) return rv;
                }

                return _trie_search(trie->child[bit].child_node, key,
                                    length - lcp - 1, payload,
                                    result_key, result_len, dump, find_pivot);
            } else {
                return CTC_E_NOT_EXIST; /* not found */
            }
        } else {
            return CTC_E_NOT_EXIST; /* not found */
        }
    } else if (length == trie->skip_len) {
        if (lcp == length) {
            if (dump) {
                sal_printf(": MATCH \n");
            }
            *payload = trie;
            if (trie->type != PAYLOAD && !find_pivot)
            {
                /* no assert here, possible during dbucket search
                * due to 1* and 0* bucket search
                */
                return CTC_E_NOT_EXIST;
            }
            if (result_key) {
                rv = _key_append(result_key, result_len, trie->skip_addr, trie->skip_len);
                if (NALPM_E_FAILURE(rv)) return rv;
            }
            return CTC_E_NONE;
        }
        else return CTC_E_NOT_EXIST;
    } else {
        if (lcp == length && find_pivot) {
            *payload = trie;
            if (result_key) {
                rv = _key_append(result_key, result_len, trie->skip_addr, trie->skip_len);
                if (NALPM_E_FAILURE(rv)) return rv;
            }
            return CTC_E_NONE;
        }
        return CTC_E_NOT_EXIST; /* not found */
    }
}


STATIC int _trie_insert2(trie_node_t *trie,
unsigned int *key,
/* bpm bit map if bpm management is required, passing null skips bpm management */
unsigned int *bpm,
unsigned int length,
trie_node_t *payload, /* payload node */
            trie_node_t **child, /* child pointer if the child is modified */
            int child_count)
{
    unsigned int lcp;
    int rv=CTC_E_NONE, bit=0;
    trie_node_t *node = NULL;

    if (!trie || (length && trie->skip_len && !key) ||
        !payload || !child || (length > _MAX_KEY_LEN_))
        return CTC_E_INVALID_PARAM;

    *child = NULL;

    lcp = _lcplen(key, length, trie->skip_addr, trie->skip_len);

    /* insert cases:
     * 1 - new key could be the parent of existing node
     * 2 - new key could become the child of a existing node
     * 3 - internal node could be inserted and the key becomes one of child
     * 4 - internal node is converted to a payload node */

    /* if the new key qualifies as new root do the inserts here */
    if (lcp == length) { /* guaranteed: length < _MAX_SKIP_LEN_ */
        if (trie->skip_len == lcp) {
            if (trie->type != INTERNAL) {
                /* duplicate */
                return CTC_E_EXIST;
            } else {
                /* change the internal node to payload node */
                _CLONE_TRIE_NODE_(payload,trie);
                mem_free(trie);
                payload->type = PAYLOAD;
                payload->count += child_count;
                *child = payload;

                if (bpm) {
                    /* bpm at this internal mode must be same as the inserted pivot */
                    payload->bpm |= _key_get_bits(bpm, lcp+1, lcp+1, 1);
                    /* implicity preserve the previous bpm & set bit 0 -myself bit */
                }
                return CTC_E_NONE;
            }
        } else { /* skip length can never be less than lcp implcitly here */
            /* this node is new parent for the old trie node */
            /* lcp is the new skip length */
            _CLONE_TRIE_NODE_(payload,trie);
            *child = payload;

            bit = (trie->skip_addr & SHL(1,trie->skip_len - length - 1,_MAX_SKIP_LEN_)) ? 1 : 0;
            trie->skip_addr &= MASK(trie->skip_len - length - 1);
            trie->skip_len  -= (length + 1);

            if (bpm) {
                trie->bpm &= MASK(trie->skip_len+1);
            }

            payload->skip_addr = (length > 0) ? key[KEY_BIT2IDX(length)] : 0;
            payload->skip_addr &= MASK(length);
            payload->skip_len  = length;
            payload->child[bit].child_node = trie;
            payload->child[!bit].child_node = NULL;
            payload->type = PAYLOAD;
            payload->count += child_count;

            if (bpm) {
                payload->bpm = SHR(payload->bpm, trie->skip_len + 1,_NUM_WORD_BITS_);
                payload->bpm |= _key_get_bits(bpm, payload->skip_len+1, payload->skip_len+1, 1);
            }
        }
    } else if (lcp == trie->skip_len) {
        /* key length is implictly greater than lcp here */
        /* decide based on key's next applicable bit */
        bit = (key[KEY_BIT2IDX(length-lcp)] &
               (1 << ((length - lcp - 1) % _NUM_WORD_BITS_))) ? 1:0;

        if (!trie->child[bit].child_node) {
            /* the key is going to be one of the child of existing node */
            /* should be the child */
            rv = _trie_skip_node_alloc(&node, key, bpm,
            length - lcp - 1, /* 0 based msbit position */
length - lcp - 1,
                       payload, child_count);
            if (NALPM_E_SUCCESS(rv)) {
                trie->child[bit].child_node = node;
                trie->count += child_count;
            } else {
                sal_printf("\n Error on trie skip node allocaiton [%d]!!!!\n", rv);
            }
        } else {
            rv = _trie_insert2(trie->child[bit].child_node,
                              key, bpm, length - lcp - 1,
                              payload, child, child_count);
            if (NALPM_E_SUCCESS(rv)) {
                trie->count += child_count;
                if (*child != NULL) { /* chande the old child pointer to new child */
                    trie->child[bit].child_node = *child;
                    *child = NULL;
                }
            }
        }
    } else {
        trie_node_t *newchild = NULL;

        /* need to introduce internal nodes */
        node = mem_malloc(MEM_IPUC_MODULE, sizeof(trie_node_t));
        _CLONE_TRIE_NODE_(node, trie);

        rv = _trie_skip_node_alloc(&newchild, key, bpm,
                                   ((lcp)?length - lcp - 1:length - 1),
                                   length - lcp - 1,
                                   payload, child_count);
        if (NALPM_E_SUCCESS(rv)) {
            bit = (key[KEY_BIT2IDX(length-lcp)] &
                   (1 << ((length - lcp - 1) % _NUM_WORD_BITS_))) ? 1: 0;

            node->child[!bit].child_node = trie;
            node->child[bit].child_node = newchild;
            node->type = INTERNAL;
            node->skip_addr = SHR(trie->skip_addr,trie->skip_len - lcp,_MAX_SKIP_LEN_);
            node->skip_len = lcp;
            node->count += child_count;
            if (bpm) {
                node->bpm = SHR(node->bpm, trie->skip_len - lcp, _MAX_SKIP_LEN_);
            }
            *child = node;

            trie->skip_addr &= MASK(trie->skip_len - lcp - 1);
            trie->skip_len  -= (lcp + 1);
            if (bpm) {
                trie->bpm &= MASK(trie->skip_len+1);
            }
        }
        else
        {
            sal_printf("\n Error on trie skip node allocaiton [%d]!!!!\n", rv);
            mem_free(node);
        }
    }

    return rv;
}

/*
 * Function:
 *     _trie_merge
 * Purpose:
 *     merge or fuse the child trie with parent trie
 */
STATIC int
_trie_merge(trie_node_t *parent_trie,
            trie_node_t *child_trie,
            unsigned int *pivot,
            unsigned int length,
            trie_node_t **new_parent)
{
    int rv, child_count;
    trie_node_t *child = NULL, clone;
    unsigned int bpm[TAPS_MAX_KEY_SIZE_WORDS] = {0};
    unsigned int child_pivot[BITS2WORDS(_MAX_KEY_LEN_)] = {0};
    unsigned int child_length = 0;
//    sys_nalpm_trie_payload_t* clone = NULL;

    if (!parent_trie || length == 0 || !pivot || !new_parent || (length > _MAX_KEY_LEN_))
        return CTC_E_INVALID_PARAM;

    /*
     * to do merge, there is one and only one condition:
     * parent must cover the child
     */

    /*
     * child pivot could be an internal node, i.e., NOT_FOUND on search
     * so check the out child instead of rv.
     */
    _trie_search(child_trie, pivot, length, &child, child_pivot, &child_length, 0, 1);

    /* The head of a bucket usually is the pivot of the bucket,
     * but for some cases, where the pivot is an INTERNAL node,
     * and it is fused with its child, then the pivot can no longer
     * be found, but we can still search a head. The head can be
     * payload (if this is the only payload head), or internal (if
     * two payload head coexist).
     */
    if (child == NULL) {
        return CTC_E_INVALID_PARAM;
    }
//    clone = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_trie_payload_t));
    _CLONE_TRIE_NODE_(&clone, child);

//    _CLONE_ROUTE_TRIE_PAYLOAD_(clone, (sys_nalpm_trie_payload_t *)child);

    if (child->type == PAYLOAD && child->bpm) {
        _TAPS_SET_KEY_BIT(bpm, 0, TAPS_IPV4_KEY_SIZE);
    }

    if (child != child_trie) {
        rv = _trie_skip_node_free(child_trie, child_pivot, child_length);
        if (rv < 0) {
            return CTC_E_INVALID_PARAM;
        }
    }

    /* Record the child count before being cleared */
    child_count = child->count;

    /* Clear the info before insert, mainly it is to prevent previous non-zero
     * count being erroneously included to calculation.
     */
    sal_memset(child, 0, sizeof(*child));

    /* merge happens on bucket trie, which usually does not need bpm */
    rv = _trie_insert2(parent_trie, child_pivot, bpm, child_length, child,
                      new_parent, child_count);
    if (rv < 0) {
        return CTC_E_INVALID_PARAM;
    }

    /*
     * child node, the inserted node, will be modified during insert,
     * and it must be a leaf node of the parent trie without any child.
     * The child node could be either payload or internal.
     */
    if (child->child[0].child_node || child->child[1].child_node) {
        return CTC_E_INVALID_PARAM;
    }
    if (clone.type == INTERNAL) {
        child->type = INTERNAL;
    }
    child->child[0].child_node = clone.child[0].child_node;
    child->child[1].child_node = clone.child[1].child_node;

    return CTC_E_NONE;
}


/*
 * Function:
 *     trie_merge
 * Purpose:
 *     merge or fuse the child trie with parent trie.
 */
int trie_merge(trie_t *parent_trie,
               trie_node_t *child_trie,
               unsigned int *child_pivot,
               unsigned int length)
{
    int rv=CTC_E_NONE;
    trie_node_t *child=NULL;

    if (!parent_trie) {
        return CTC_E_INVALID_PARAM;
    }

    if (!child_trie) {
        return CTC_E_NONE;
    }

    if (parent_trie->trie == NULL) {
        parent_trie->trie = child_trie;
    } else {
        if (parent_trie->v6_key) {
            rv = _trie_v6_merge(parent_trie->trie, child_trie, child_pivot, length, &child);
        } else {
            rv = _trie_merge(parent_trie->trie, child_trie, child_pivot, length, &child);
        }
        if (child) {
            /* The parent head can be changed if the new payload generates a
             * new internal node, which then becomes the new head.
             */
             parent_trie->trie = child;
        }
    }

    return rv;
}

STATIC int _trie_fuse_child(trie_node_t *trie, int bit)
{
    trie_node_t *child = NULL;
    int rv = CTC_E_NONE;

    if (trie->child[0].child_node && trie->child[1].child_node) {
        return CTC_E_INVALID_PARAM;
    }

    bit = (bit > 0)?1:0;
    child = trie->child[bit].child_node;

    if (child == NULL) {
        return CTC_E_INVALID_PARAM;
    } else {
        if (trie->skip_len + child->skip_len + 1 <= _MAX_SKIP_LEN_) {

            if (trie->skip_len == 0) trie->skip_addr = 0;

            if (child->skip_len < _MAX_SKIP_LEN_) {
                trie->skip_addr = SHL(trie->skip_addr,child->skip_len + 1,_MAX_SKIP_LEN_);
            }

            trie->skip_addr  |= SHL(bit,child->skip_len,_MAX_SKIP_LEN_);
            child->skip_addr |= trie->skip_addr;
            child->bpm       |= SHL(trie->bpm,child->skip_len+1,_MAX_SKIP_LEN_);
            child->skip_len  += trie->skip_len + 1;

            /* do not free payload nodes as they are user managed */
            if (trie->type == INTERNAL) {
                mem_free(trie);
            }
        }
    }

    return rv;
}


/*
 * Function:
 *     trie_split
 * Purpose:
 *     Split the trie into 2 such that the new sub trie covers given prefix/length.
 * NOTE:
 *     key, key_len    -- The given prefix/length
 *     max_split_count -- The sub trie's max allowed count.
 */
int
_trie_split2(trie_node_t *trie,
             unsigned int *key,
             unsigned int key_len,
             unsigned int *pivot,
             unsigned int *pivot_len,
             unsigned int *split_count,
             trie_node_t **split_node,
             trie_node_t **child,
             trie_split2_states_e_t *state,
             const int max_split_count,
             const int exact_same)
{
    unsigned int lcp=0;
    int bit=0, rv=CTC_E_NONE;

    if (!trie || !pivot || !pivot_len || !split_node || !state || max_split_count == 0) return CTC_E_INVALID_PARAM;
    /* start building the pivot */
    rv = _key_append(pivot, pivot_len, trie->skip_addr, trie->skip_len);
    if (NALPM_E_FAILURE(rv)) return rv;


    lcp = _lcplen(key, key_len, trie->skip_addr, trie->skip_len);

    if (lcp == trie->skip_len) {
        if (trie->count <= max_split_count &&
            (!exact_same || (key_len - lcp) == 0)) {
            *split_node = trie;
            *split_count = trie->count;
            if (trie->count < max_split_count) {
                *state = TRIE_SPLIT2_STATE_PRUNE_NODES;
            }
            return CTC_E_NONE;
        }
        if (key_len > lcp) {
            bit = (key[KEY_BIT2IDX(key_len - lcp)] & \
                    (1 << ((key_len - lcp - 1) % _NUM_WORD_BITS_))) ? 1:0;

            /* based on next bit branch left or right */
            if (trie->child[bit].child_node) {
                /* we can not split at this node, keep searching, it's better to
                 * split at longer pivot
                 */
                rv = _key_append(pivot, pivot_len, bit, 1);
                if (NALPM_E_FAILURE(rv)) return rv;

                rv = _trie_split2(trie->child[bit].child_node,
                                  key, key_len - lcp - 1,
                                  pivot, pivot_len, split_count,
                                  split_node, child, state,
                                  max_split_count, exact_same);
                if (NALPM_E_FAILURE(rv)) return rv;
            }
        }
    }

    /* free up internal nodes if applicable */
    switch(*state) {
        case TRIE_SPLIT2_STATE_NONE: /* fail to split */
            break;

        case TRIE_SPLIT2_STATE_PRUNE_NODES:
            if (trie->count == *split_count) {
                /* if the split point has associate internal nodes they have to
                 * be cleaned up */
          /*      assert(trie->type == INTERNAL); */
                /* at most one child */
          /*      assert(!(trie->child[0].child_node && trie->child[1].child_node)); */
                /* at least one child */
          /*      assert(trie->child[0].child_node || trie->child[1].child_node); */
                mem_free(trie);
            } else {
            /*    assert(*child == NULL); */
                /* fuse with child if possible */
                trie->child[bit].child_node = NULL;
                bit = (bit==0)?1:0;
                trie->count -= *split_count;

                /* optimize more */
                if ((trie->type == INTERNAL) &&
                        (trie->skip_len +
                         trie->child[bit].child_node->skip_len + 1 <= _MAX_SKIP_LEN_)) {
                    *child = trie->child[bit].child_node;
                    rv = _trie_fuse_child(trie, bit);
                    if (rv != CTC_E_NONE) {
                        *child = NULL;
                    }
                }
                *state = TRIE_SPLIT2_STATE_DONE;
            }
            break;

        case TRIE_SPLIT2_STATE_DONE:
            /* adjust parent's count */
        /*    assert(*split_count > 0);
            assert(trie->count >= *split_count); */

            /* update the child pointer if child was pruned */
            if (*child != NULL) {
                trie->child[bit].child_node = *child;
                *child = NULL;
            }
            trie->count -= *split_count;
            break;

        default:
            break;
    }

    return rv;
}



/*
 * Function:
 *     trie_split2
 * Purpose:
 *     Split the trie such that the new sub trie covers given prefix/length.
 *     Basically this is a reverse of trie_merge.
 */

int trie_split2(trie_t *trie,
                unsigned int *key,
                unsigned int key_len,
                unsigned int *pivot,
                unsigned int *pivot_len,
                trie_node_t **split_trie_root,
                const int max_split_count,
                const int exact_same)
{
    int rv = CTC_E_NONE;
    int msc = max_split_count;
    unsigned int split_count=0;
    trie_node_t *child = NULL, *node=NULL, clone;
    trie_split2_states_e_t state = TRIE_SPLIT2_STATE_NONE;

    if (!trie || (key_len && !key) || !pivot || !pivot_len ||
        !split_trie_root || max_split_count == 0) {
        return CTC_E_INVALID_PARAM;
    }

    *split_trie_root = NULL;
    *pivot_len = 0;

    if (trie->trie) {
        if (max_split_count == 0xfffffff) {
            trie_node_t *child2 = NULL;
            trie_node_t *payload;
            payload = mem_malloc(MEM_IPUC_MODULE, sizeof(trie_node_t));
            if (payload == NULL) {
                return CTC_E_NO_MEMORY;
            }

            if (trie->v6_key) {
                rv = _trie_v6_insert(trie->trie, key, NULL, key_len, payload, &child2, 0);
            } else {
                rv = _trie_insert2(trie->trie, key, NULL, key_len, payload, &child2, 0);
            }
            if (child2) { /* change the old child pointer to new child */
                trie->trie = child2;
            }

            if (NALPM_E_SUCCESS(rv)) {
                payload->type = INTERNAL;
            } else {
                mem_free(payload);
                if (rv != CTC_E_EXIST) {
                    return rv;
                }
            }

            msc = trie->trie->count;
        }
        if (trie->v6_key) {
            sal_memset(pivot, 0, sizeof(unsigned int) * BITS2WORDS(_MAX_KEY_LEN_144_));
            rv = _trie_v6_split2(trie->trie, key, key_len, pivot, pivot_len,
                    &split_count, split_trie_root, &child, &state,
                    msc, exact_same);
        } else {
            sal_memset(pivot, 0, sizeof(unsigned int) * BITS2WORDS(_MAX_KEY_LEN_48_));
            rv = _trie_split2(trie->trie, key, key_len, pivot, pivot_len,
                    &split_count, split_trie_root, &child, &state,
                    msc, exact_same);
        }

        if (NALPM_E_SUCCESS(rv) && (TRIE_SPLIT2_STATE_DONE == state)) {
      /*      assert(split_count > 0);
            assert(*split_trie_root); */
            if (max_split_count == 0xfffffff) {
          /*      assert(*pivot_len == key_len); */
            } else {
            /*    assert(*pivot_len < key_len); */
            }

            /* update the child pointer if child was pruned */
            if (child != NULL) {
                trie->trie = child;
            }

            sal_memcpy(&clone, *split_trie_root, sizeof(trie_node_t));
            child = *split_trie_root;

            /* take advantage of thie function by passing in internal or payload node whatever
             * is the new root. If internal the function assumed it as payload node & changes type.
             * But this method is efficient to reuse the last internal or payload node possible to
             * implant the new pivot */
            if (trie->v6_key) {
                rv = _trie_v6_skip_node_alloc(&node, pivot, NULL,
                                              *pivot_len, *pivot_len,
                                              child, child->count);
            } else {
                rv = _trie_skip_node_alloc(&node, pivot, NULL,
                                           *pivot_len, *pivot_len,
                                           child, child->count);
            }

            if (NALPM_E_SUCCESS(rv)) {
                if (clone.type == INTERNAL) {
                    child->type = INTERNAL; /* since skip alloc would have reset it to payload */
                }
                child->child[0].child_node = clone.child[0].child_node;
                child->child[1].child_node = clone.child[1].child_node;
                *split_trie_root = node;
            }
        } else if (NALPM_E_SUCCESS(rv) && (max_split_count == 0xfffffff) &&
                   (split_count == trie->trie->count)) {
            /* take all */
            *split_trie_root = trie->trie;
            trie->trie = NULL;
        } else { /* split2 is not like split which can always succeed */
            sal_printf("Failed to split the trie error:%d state: %d split_trie_root: %p !!!\n", rv, state, *split_trie_root);
            rv = CTC_E_NOT_EXIST;
        }
    } else {
        rv = CTC_E_INVALID_PARAM;
    }

    return rv;
}


#define TRIE_TRAVERSE_STOP(state, rv) \
    {if (state == TRIE_TRAVERSE_STATE_DONE || rv < 0) {return rv;} }

STATIC int _trie_preorder_traverse2(trie_node_t *ptrie,
                                    trie_node_t *trie,
                                    trie_traverse_states_e_t *state,
                                    trie_callback_ext_f cb,
                                    void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *lc, *rc;

    if (trie == NULL || !cb) {
        return CTC_E_NONE;
    } else {
    /*    assert(!ptrie || ptrie->type == PAYLOAD); */

        /* make the trie delete safe */
        lc = trie->child[0].child_node;
        rc = trie->child[1].child_node;
        if (trie->type == PAYLOAD) { /* no need to callback on internal nodes */
            rv = cb(ptrie, trie, state, user_data);
            TRIE_TRAVERSE_STOP(*state, rv);

            /* Change the ptrie as trie if applicable */
            /* make the ptrie delete safe */
            if (*state != TRIE_TRAVERSE_STATE_DELETED) {
                ptrie = trie;
            }
        }
    }

    if (NALPM_E_SUCCESS(rv)) {
        rv = _trie_preorder_traverse2(ptrie, lc, state, cb, user_data);
        TRIE_TRAVERSE_STOP(*state, rv);
    }
    if (NALPM_E_SUCCESS(rv)) {
        rv = _trie_preorder_traverse2(ptrie, rc, state, cb, user_data);
    }
    return rv;
}

STATIC int _trie_postorder_traverse2(trie_node_t *ptrie,
                                    trie_node_t *trie,
                                    trie_traverse_states_e_t *state,
                                    trie_callback_ext_f cb,
                                    void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *ori_ptrie = ptrie;
    trie_node_t *lc, *rc;
    node_type_e_t trie_type;
    if (trie == NULL) {
        return CTC_E_NONE;
    }

 /*   assert(!ptrie || ptrie->type == PAYLOAD); */

    /* Change the ptrie as trie if applicable */
    if (trie->type == PAYLOAD) {
        ptrie = trie;
    }

    /* During the callback, a trie node can be deleted or inserted.
     * For a deleted node, its internal parent could also be deleted, thus to
     * make it safe we should record rc.
     */
    trie_type = trie->type;
    lc = trie->child[0].child_node;
    rc = trie->child[1].child_node;

    if (NALPM_E_SUCCESS(rv)) {
        rv = _trie_postorder_traverse2(ptrie, lc, state, cb, user_data);
        TRIE_TRAVERSE_STOP(*state, rv);
    }
    if (NALPM_E_SUCCESS(rv)) {
        rv = _trie_postorder_traverse2(ptrie, rc, state, cb, user_data);
        TRIE_TRAVERSE_STOP(*state, rv);
    }
    if (NALPM_E_SUCCESS(rv)) {
        if (trie_type == PAYLOAD) {
            rv = cb(ori_ptrie, trie, state, user_data);
        }
    }
    return rv;
}

STATIC int _trie_inorder_traverse2(trie_node_t *ptrie,
                                   trie_node_t *trie,
                                   trie_traverse_states_e_t *state,
                                   trie_callback_ext_f cb,
                                   void *user_data)
{
    int rv = CTC_E_NONE;
    trie_node_t *rc = NULL;
    trie_node_t *ori_ptrie = ptrie;

    if (trie == NULL) {
        return CTC_E_NONE;
    }

 /*   assert(!ptrie || ptrie->type == PAYLOAD); */

    /* Change the ptrie as trie if applicable */
    if (trie->type == PAYLOAD) {
        ptrie = trie;
    }

    rv = _trie_inorder_traverse2(ptrie, trie->child[0].child_node, state, cb, user_data);
    TRIE_TRAVERSE_STOP(*state, rv);

    /* make the trie delete safe */
    rc = trie->child[1].child_node;

    if (NALPM_E_SUCCESS(rv)) {
        if (trie->type == PAYLOAD) {
            rv = cb(ptrie, trie, state, user_data);
            TRIE_TRAVERSE_STOP(*state, rv);
            /* make the ptrie delete safe */
            if (*state == TRIE_TRAVERSE_STATE_DELETED) {
                ptrie = ori_ptrie;
            }
        }
    }

    if (NALPM_E_SUCCESS(rv)) {
        rv = _trie_inorder_traverse2(ptrie, rc, state, cb, user_data);
    }
    return rv;
}



STATIC int _trie_traverse2(trie_node_t *trie, trie_callback_ext_f cb,
                           void *user_data,  trie_traverse_order_e_t order,
                           trie_traverse_states_e_t *state)
{
    int rv = CTC_E_NONE;

    switch(order) {
        case _TRIE_PREORDER_TRAVERSE:
            rv = _trie_preorder_traverse2(NULL, trie, state, cb, user_data);
            break;
        case _TRIE_POSTORDER_TRAVERSE:
            rv = _trie_postorder_traverse2(NULL, trie, state, cb, user_data);
            break;
        case _TRIE_INORDER_TRAVERSE:
            rv = _trie_inorder_traverse2(NULL, trie, state, cb, user_data);
            break;
        default:
      /*      assert(0); */
            break;
    }

    return rv;
}



/*
 * Function:
 *     sys_nalpm_trie_traverse2
 * Purpose:
 *     Traverse the trie (PAYLOAD) & call the extended application callback
 *     which has current node's PAYLOAD parent node with user data.
 *
 */
int sys_nalpm_trie_traverse2(trie_t *trie, trie_callback_ext_f cb,
                   void *user_data, trie_traverse_order_e_t order)
{
    trie_traverse_states_e_t state = TRIE_TRAVERSE_STATE_NONE;

    if (order < _TRIE_PREORDER_TRAVERSE ||
        order >= _TRIE_TRAVERSE_MAX || !cb) return CTC_E_INVALID_PARAM;

    if (trie == NULL) {
        return CTC_E_NONE;
    } else {
        return _trie_traverse2(trie->trie, cb, user_data, order, &state);
    }
}


/*
 * Function:
 *     trie_delete
 * Purpose:
 *     Deletes provided prefix/length in to the trie
 */
    int
trie_delete (trie_t * trie,
        unsigned int *key, unsigned int length, trie_node_t ** payload)
{
    int rv = CTC_E_NONE;
    trie_node_t *child = NULL;

    if (trie->trie)
    {
        if (trie->v6_key)
        {
            rv = sys_trie_v6_delete (trie->trie, key, length, payload, &child);
        }
        else
        {
            rv = _trie_delete (trie->trie, key, length, payload, &child);
        }
        if (rv == CTC_E_IN_USE)
        {
            /* the head node of trie was deleted, reset trie pointer to null */
            trie->trie = NULL;
            rv = CTC_E_NONE;
        }
        else if (rv == CTC_E_NONE && child != NULL)
        {
            trie->trie = child;
        }
    }
    else
    {
        rv = CTC_E_ENTRY_NOT_EXIST;
    }
    return rv;
}


/*
 * Function:
 *     trie_destroy
 * Purpose:
 *     destroys a trie
 */
    int
trie_destroy (trie_t * trie)
{

    mem_free (trie);
    return CTC_E_NONE;
}

