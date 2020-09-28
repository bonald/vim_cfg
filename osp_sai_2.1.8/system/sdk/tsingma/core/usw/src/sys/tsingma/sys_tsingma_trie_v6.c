/****************************************************************************
* cmodel_nlpm_trie_v6.c:
*   nlpm ipv6 trie process
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision     : R0.01
* Author       : cuixl
* Date         : 2017-Feb-10 14:16
* Reason       : First Create.
****************************************************************************/
#include "sys_tsingma_trie.h"
#include "sys_tsingma_trie_v6.h"
#include "sys_tsingma_nalpm.h"
#include "sys_usw_ipuc.h"

#define _MAX_KEY_LEN_   (_MAX_KEY_LEN_128_)
#define _MAX_KEY_WORDS_ (NALPM_BITS2WORDS(_MAX_KEY_LEN_))


/* key packing expetations:
* eg., 144 bit key
* - 0x10 / 8 -> key[0] = 0, key[1] = 0, key[2] = 0, key[3] = 0, key[0] = 0x10
* - 0x123456789a / 48 -> key[0] = 0, key[1] = 0, key[2] = 0, key[3] = 0x12 key[4] = 0x3456789a
* length - represents number of valid bits from farther to lower index ie., 1->0
*/
#if 1
#define KEY_BIT2IDX(x) (((NALPM_BITS2WORDS(_MAX_KEY_LEN_)*32) - (x))/32)
#else
#define KEY_BIT2IDX(x) ((x)/32)
#endif

extern sys_nalpm_master_t* g_sys_nalpm_master[];

/********************************************************/
/* Get a chunk of bits from a key (MSB bit - on word0, lsb on word 1)..
*/
STATIC unsigned int
_key_get_bits_2 (unsigned int *key,
               unsigned int pos /* 1based, msb bit position */ ,
               unsigned int len, unsigned int skip_len_check)
{
    if (!key || (pos < len) || (pos > _MAX_KEY_LEN_) ||
        ((skip_len_check == TRUE) && (len > _MAX_SKIP_LEN_)))
    {
        /*assert (0);*/
    }

    /* use Macro, convert to what's required by Macro */
    return _TAPS_GET_KEY_BITS (key, pos - len, len, _MAX_KEY_LEN_);
}
/*
* Assumes the layout for
* 0 - most significant word
* _MAX_KEY_WORDS_ - least significant word
* eg., for key size of 48, word0-[bits 48 - 32] word1 - [bits31 - 0]
*/
STATIC int
_key_append_2 (unsigned int *key,
             unsigned int *length,
             unsigned int skip_addr, unsigned int skip_len)
{
    int rv = CTC_E_NONE;

    if (!key || !length || ((skip_len + *length) > _MAX_KEY_LEN_) ||
        (skip_len > _MAX_SKIP_LEN_))
    {
        return CTC_E_INVALID_PARAM;
    }

    rv = sys_nalpm_taps_key_shift (_MAX_KEY_LEN_, key, *length, 0 - (int) skip_len);
    if (NALPM_E_SUCCESS (rv))
    {
        key[KEY_BIT2IDX (1)] |= skip_addr;
        *length += skip_len;
    }

    return rv;
}

/*
* Function:
*     _lcplen_v6
* Purpose:
*     returns longest common prefix length provided a key & skip address
*/
unsigned int STATIC
_lcplen_v6 (unsigned int *key, unsigned int len1,
        unsigned int skip_addr, unsigned int len2)
{
    unsigned int diff;
    unsigned int lcp = len1 < len2 ? len1 : len2;

    if ((len1 > _MAX_KEY_LEN_) || (len2 > _MAX_KEY_LEN_))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "len1 %d or len2 %d is larger than %d\n", len1, len2,
                      _MAX_KEY_LEN_);
        /*assert (0);*/
    }

    if ((len1 == 0) || (len2 == 0))
    {
        return 0;
    }

    diff = _key_get_bits_2 (key, len1, lcp, TRUE);
    diff ^= (SHR (skip_addr, len2 - lcp, _MAX_SKIP_LEN_) & MASK (lcp));

    while (diff)
    {
        diff >>= 1;
        --lcp;
    }

    return lcp;
}

/*
* Internal function for LPM match searching.
* callback on all payload nodes if cb != NULL.
*/
int
sys_trie_v6_find_lpm (trie_node_t *parent,
                   trie_node_t * trie,
                   unsigned int *key,
                   unsigned int length,
                   trie_node_t ** payload,
                   trie_callback_f cb, void *user_data)
{
    unsigned int lcp = 0;
    int bit = 0, rv = CTC_E_NONE;

    if (!trie)
        return CTC_E_INVALID_PARAM;

    lcp = _lcplen_v6 (key, length, trie->skip_addr, trie->skip_len);

    if ((length > trie->skip_len) && (lcp == trie->skip_len))
    {
        if (trie->type == PAYLOAD)
        {
            /* lpm cases */
            if (payload != NULL && 1 != trie->dirty_flag)
            {
                /* update lpm result */
                *payload = trie;
                parent = trie;
            }
            else if(payload != NULL && 1 == trie->dirty_flag)
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
            return sys_trie_v6_find_lpm (parent, trie->child[bit].child_node, key,
                                      length - lcp - 1, payload, cb, user_data);
        }
    }
    else if ((length == trie->skip_len) && (lcp == length))
    {
        if (trie->type == PAYLOAD)
        {
            /* exact match case */
            if (payload != NULL && 1 != trie->dirty_flag)
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

/* trie->bpm format:
* bit 0 is for the pivot itself (longest)
* bit skip_len is for the trie branch leading to the pivot node (shortest)
* bits (0-skip_len) is for the routes in the parent node's bucket
*/
int
sys_trie_v6_find_bpm (trie_node_t * trie,
                   unsigned int *key, unsigned int length, int *bpm_length)
{
    unsigned int lcp = 0, local_bpm_mask = 0;
    int bit = 0, rv = CTC_E_NONE, local_bpm = 0;

    if (!trie || (length > _MAX_KEY_LEN_))
        return CTC_E_INVALID_PARAM;

    /* calculate number of matching msb bits */
    lcp = _lcplen_v6 (key, length, trie->skip_addr, trie->skip_len);

    if (length > trie->skip_len)
    {
        if (lcp == trie->skip_len)
        {
            /* fully matched and more bits to check, go down the trie */
            bit = (key[KEY_BIT2IDX (length - lcp)] &
            SHL (1, (length - lcp - 1) % _NUM_WORD_BITS_,
                 _NUM_WORD_BITS_)) ? 1 : 0;

            if (trie->child[bit].child_node)
            {
                rv =
                sys_trie_v6_find_bpm (trie->child[bit].child_node, key,
                                   length - lcp - 1, bpm_length);
                /* on the way back, start bpm_length accumulation when encounter first non-0 bpm */
                if (*bpm_length >= 0)
                {
                    /* child node has non-zero bpm, just need to accumulate skip_len and branch bit */
                    *bpm_length += (trie->skip_len + 1);
                    return rv;
                }
                else if (trie->bpm & BITMASK (trie->skip_len + 1))
                {
                    /* first non-zero bmp on the way back */
                    BITGETLSBSET (trie->bpm, trie->skip_len, local_bpm);
                    if (local_bpm >= 0)
                    {
                        *bpm_length = trie->skip_len - local_bpm;
                    }
                }
                /* on the way back, and so far all bpm are 0 */
                return rv;
            }
        }
    }

    /* no need to go further, we find whatever bits matched and
    * check that part of the bpm mask
    */
    local_bpm_mask = trie->bpm & (~(BITMASK (trie->skip_len - lcp)));
    if (local_bpm_mask & BITMASK (trie->skip_len + 1))
    {
        /* first non-zero bmp on the way back */
        BITGETLSBSET (local_bpm_mask, trie->skip_len, local_bpm);
        if (local_bpm >= 0)
        {
            *bpm_length = trie->skip_len - local_bpm;
        }
    }

    return rv;
}

/*
* Function:
*   sys_trie_v6_skip_node_alloc
* Purpose:
*   create a chain of trie_node_t that has the payload at the end.
*   each node in the chain can skip upto _MAX_SKIP_LEN number of bits,
 *   while the child pointer in the chain represent 1 bit. So totally
*   each node can absorb (_MAX_SKIP_LEN + 1) bits.
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
int
sys_trie_v6_skip_node_alloc (trie_node_t ** node, unsigned int *key,
                          /* bpm bit map if bpm management is required, passing null skips bpm management */
                          unsigned int *bpm, unsigned int msb, 	/* NOTE: valid msb position 1 based, 0 means skip0/0 node */
                          unsigned int skip_len, trie_node_t * payload, unsigned int count)	/* payload count underneath - mostly 1 except some tricky cases */
{
    int lsb = 0, msbpos = 0, lsbpos = 0, bit = 0, index;
    trie_node_t *child = NULL, *skip_node = NULL;

    /* calculate lsb bit position, also 1 based */
    lsb = ((msb) ? msb + 1 - skip_len : msb);

    /*assert (((int) msb >= 0) && (lsb >= 0));*/

    if (!node || !key || !payload || msb > _MAX_KEY_LEN_ || msb < skip_len)
        return CTC_E_INVALID_PARAM;

    if (msb)
    {
        for (index = BITS2SKIPOFF (lsb), lsbpos = lsb - 1;
            index <= BITS2SKIPOFF (msb);
        index++)
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
            msbpos = index * _MAX_SKIP_LEN_ - 1;
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
                skip_node->skip_addr =
                key[KEY_BIT2IDX (msbpos + 1)] & MASK ((msbpos + 1) %
                _NUM_WORD_BITS_);
                skip_node->skip_addr =
                SHL (skip_node->skip_addr,
                     skip_node->skip_len - ((msbpos + 1) % _NUM_WORD_BITS_),
                     _NUM_WORD_BITS_);
                skip_node->skip_addr |=
                SHR (key[KEY_BIT2IDX (lsbpos + 1)],
                     (lsbpos % _NUM_WORD_BITS_), _NUM_WORD_BITS_);
            }
            else
            {
                skip_node->skip_addr =
                SHR (key[KEY_BIT2IDX (msbpos + 1)],
                     (lsbpos % _NUM_WORD_BITS_), _NUM_WORD_BITS_);
            }

            /* set up the chain of child pointer, first node has no child since "child" was inited to NULL */
            if (child)
            {
                skip_node->child[bit].child_node = child;
            }

            /* calculate child pointer for next loop. NOTE: skip_addr has not been masked
            * so we still have the child bit in the skip_addr here.
            */
            bit =
            (skip_node->skip_addr & SHL (1, skip_node->skip_len - 1,
                                         _MAX_SKIP_LEN_)) ? 1 : 0;

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
                    _key_get_bits_2 (bpm, lsbpos, skip_node->skip_len, FALSE);
                }
                else
                {
                    skip_node->bpm =
                    _key_get_bits_2 (bpm, lsbpos + 1, skip_node->skip_len + 1,
                                   FALSE);
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
            skip_node->bpm = _key_get_bits_2 (bpm, 1, 1, TRUE);
        }
    }

    *node = skip_node;
    return CTC_E_NONE;
}

int
sys_trie_v6_insert (trie_node_t * trie, unsigned int *key,
                 /* bpm bit map if bpm management is required, passing null skips bpm management */
                 unsigned int *bpm, unsigned int length, trie_node_t * payload, 	/* payload node */
                 trie_node_t **
                 child /* child pointer if the child is modified */ )
{
    unsigned int lcp;
    int rv = CTC_E_NONE, bit = 0;
    trie_node_t *node = NULL;

    if (!trie || !payload || !child || (length > _MAX_KEY_LEN_))
        return CTC_E_INVALID_PARAM;

    *child = NULL;


    lcp = _lcplen_v6 (key, length, trie->skip_addr, trie->skip_len);

    /* insert cases:
    * 1 - new key could be the parent of existing node
    * 2 - new node could become the child of a existing node
    * 3 - internal node could be inserted and the key becomes one of child
    * 4 - internal node is converted to a payload node */

    /* if the new key qualifies as new root do the inserts here */
    if (lcp == length)
    {
        /* guaranteed: length < _MAX_SKIP_LEN_ */
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
                    payload->bpm |=
                    _key_get_bits_2 (bpm, lcp + 1, lcp + 1, FALSE);
                    /* implicity preserve the previous bpm & set bit 0 -myself bit */
                }
                return CTC_E_NONE;
            }
        }
        else
        {
            /* skip length can never be less than lcp implcitly here */
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
                _key_get_bits_2 (bpm, payload->skip_len + 1,
                               payload->skip_len + 1, FALSE);
            }
        }
    }
    else if (lcp == trie->skip_len)
    {
        /* key length is implictly greater than lcp here */
        /* decide based on key's next applicable bit */
        bit = (key[KEY_BIT2IDX (length - lcp)] &
        SHL (1, (length - lcp - 1) % _NUM_WORD_BITS_,
             _NUM_WORD_BITS_)) ? 1 : 0;

        if (!trie->child[bit].child_node)
        {
            /* the key is going to be one of the child of existing node */
            /* should be the child */
            rv = sys_trie_v6_skip_node_alloc (&node, key, bpm, length - lcp - 1, 	/* 0 based msbit position */
                                           length - lcp - 1, payload, 1);
            if (NALPM_E_SUCCESS (rv))
            {
                trie->child[bit].child_node = node;
                trie->count++;
            }
            else
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                "\n Error on trie skip node allocaiton [%d]!!!!\n", rv);
            }
        }
        else
        {
            rv = sys_trie_v6_insert (trie->child[bit].child_node,
                                  key, bpm, length - lcp - 1, payload, child);
            if (NALPM_E_SUCCESS (rv))
            {
                trie->count++;
                if (*child != NULL)
                {
                    /* chande the old child pointer to new child */
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

        rv = sys_trie_v6_skip_node_alloc (&newchild, key, bpm,
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

int
sys_trie_v6_delete (trie_node_t * trie,
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
    lcp = _lcplen_v6 (key, length, trie->skip_addr, trie->skip_len);

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
                sys_trie_v6_delete (trie->child[bit].child_node, key,
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
*     trie_v6_split
* Purpose:
*     Split the trie into 2 based on optimum pivot
* NOTE:
*     max_split_len -- split will make sure the split point
*                has a length shorter or equal to the max_split_len
*                unless this will cause a no - split (all prefixs
*                stays below the split point)
*     split_to_pair -- used only when the split point will be
*                used to create a pair of tries later (i.e: dbucket
*                pair. we assume the split point itself will always be
*                put into 0* trie if itself is a payload / prefix)
*/
int
sys_trie_v6_split (trie_node_t * trie,
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
    rv = _key_append_2 (pivot, length, trie->skip_addr, trie->skip_len);
    if (NALPM_E_FAILURE (rv))
        return rv;

    {
        /*
        * split logic to make sure the split length is shorter than the
        * requested max_split_len, unless we don't actully split the
        * tree if we stop here.
        * if (*length > max_split_len) && (trie->count != max_count)
        {
            *    need to split at or above this node. might need to split the node in middle
            *
        }
        else if ((ABS(child count*2 - max_count) > ABS(count*2 - max_count)) ||
            *            ((*length == max_split_len) && (trie->count != max_count)))
        {
            *    (the check above imply trie->count != max_count, so also imply *length < max_split_len)
            *    need to split at this node.
            *
        }
        else
        {
            *    keep searching, will be better split at longer pivot.
            *
        }
        */
        if ((*length > max_split_len) && (trie->count != max_count))
        {
            /* the pivot is getting too long, we better split at this node for
            * better bucket capacity efficiency if we can. We can split if
            * the trie node has a count != max_count, which means the
            * resulted new trie will not have all pivots (FULL)
            */
            if ((TRIE_SPLIT_STATE_PAYLOAD_SPLIT == *state) &&
                (trie->type == INTERNAL))
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
                    * lead to this node. we insert a skip_len = 0 node and use it as split point
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
                rv =
                sys_nalpm_taps_key_shift (_MAX_KEY_LEN_, bpm, _MAX_KEY_LEN_,
                                trie->skip_len + 1);

                if (NALPM_E_SUCCESS (rv))
                {
                    rv =
                    sys_nalpm_taps_key_shift (_MAX_KEY_LEN_, pivot, *length,
                                    trie->skip_len + 1);
                }
                *state = TRIE_SPLIT_STATE_PRUNE_NODES;
                return rv;
            }
        }
        else if (((*length == max_split_len) && (trie->count != max_count)) || (!trie->child[bit].child_node) ||
            (ABS (trie->child[bit].child_node->count * 2 - max_count * 1) >
        ABS (trie->count * 2 - max_count * 1)))
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
            rv = _key_append_2 (pivot, length, bit, 1);
            if (NALPM_E_FAILURE (rv))
                return rv;

            rv = sys_trie_v6_split (trie->child[bit].child_node,
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
                rv =
                sys_nalpm_taps_key_shift (_MAX_KEY_LEN_, pivot, *length,
                                trie->skip_len + 1);
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
                mem_free(trie);
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
 *   _trie_v6_skip_node_free
 * Purpose:
 *   Destroy a chain of trie_node_t that has the target node at the end.
 *   The target node is not necessarily PAYLOAD type, but all nodes
 *   on the chain except for the end must have only one branch.
 * Input:
 *   key      --  target key
 *   length   --  target key length
 *   free_end --  free
 */
int _trie_v6_skip_node_free(trie_node_t *trie,
                            unsigned int *key,
                            unsigned int length)
{
    unsigned int lcp = 0;
    int bit = 0, rv = CTC_E_NONE;

    if (!trie || (length && trie->skip_len && !key)) return CTC_E_INVALID_PARAM;

    lcp = _lcplen_v6(key, length, trie->skip_addr, trie->skip_len);


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
                rv = _trie_v6_skip_node_free(trie->child[bit].child_node, key,
                        length - lcp - 1);
                if (NALPM_E_SUCCESS(rv)) {
                    /*assert(trie->type == INTERNAL);*/
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

            return CTC_E_NONE;
        }
        else return CTC_E_NOT_EXIST;
    } else {
        return CTC_E_NOT_EXIST; /* not found */
    }
}

STATIC  int
_print_v6_trie_node (trie_node_t * trie, void *datum)
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


int _trie_v6_search(trie_node_t *trie,
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

    lcp = _lcplen_v6(key, length, trie->skip_addr, trie->skip_len);

    if (dump) {
        _print_v6_trie_node(trie, (unsigned int *)1);
    }

    if (length > trie->skip_len) {
        if (lcp == trie->skip_len) {
            bit = (key[KEY_BIT2IDX(length - lcp)] & \
                   (1 << ((length - lcp -1) % _NUM_WORD_BITS_))) ? 1:0;
            if (dump) {
                sal_printf(" Length: %d Next-Bit[%d] \n", length, bit);
            }

            if (result_key) {
                rv = _key_append_2(result_key, result_len, trie->skip_addr, trie->skip_len);
                if (NALPM_E_FAILURE(rv)) return rv;
            }

            /* based on next bit branch left or right */
            if (trie->child[bit].child_node) {
                if (result_key) {
                    rv = _key_append_2(result_key, result_len, bit, 1);
                    if (NALPM_E_FAILURE(rv)) return rv;
                }

                return _trie_v6_search(trie->child[bit].child_node, key,
                                    length - lcp - 1, payload,
                    result_key, result_len, dump, find_pivot);
                   }
                   else
                   {
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
                rv = _key_append_2(result_key, result_len, trie->skip_addr, trie->skip_len);
                if (NALPM_E_FAILURE(rv)) return rv;
            }
            return CTC_E_NONE;
        }
        else return CTC_E_NOT_EXIST;
    } else {
        if (lcp == length && find_pivot) {
            *payload = trie;
            if (result_key) {
                rv = _key_append_2(result_key, result_len, trie->skip_addr, trie->skip_len);
                if (NALPM_E_FAILURE(rv)) return rv;
            }
        }
        return CTC_E_NOT_EXIST; /* not found */
    }
}


int _trie_v6_insert(trie_node_t *trie,
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
        !payload || !child || (length > _MAX_KEY_LEN_)) return CTC_E_INVALID_PARAM;

    *child = NULL;

    lcp = _lcplen_v6(key, length, trie->skip_addr, trie->skip_len);

    /* insert cases:
     * 1 - new key could be the parent of existing node
     * 2 - new node could become the child of a existing node
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
                    payload->bpm |= _key_get_bits_2(bpm, lcp+1, lcp+1, FALSE);
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
                payload->bpm |= _key_get_bits_2(bpm, payload->skip_len+1, payload->skip_len+1, FALSE);
            }
        }
    } else if (lcp == trie->skip_len) {
        /* key length is implictly greater than lcp here */
        /* decide based on key's next applicable bit */
        bit = (key[KEY_BIT2IDX(length-lcp)] &
               (1 << ((length - lcp -1) % _NUM_WORD_BITS_))) ? 1:0;

        if (!trie->child[bit].child_node) {
            /* the key is going to be one of the child of existing node */
            /* should be the child */
            rv = sys_trie_v6_skip_node_alloc(&node, key, bpm,
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
            rv = _trie_v6_insert(trie->child[bit].child_node,
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

        rv = sys_trie_v6_skip_node_alloc(&newchild, key, bpm,
                                         ((lcp)?length - lcp - 1:length - 1),
                                         length - lcp - 1,
                                         payload, child_count);
        if (NALPM_E_SUCCESS(rv)) {
            bit = (key[KEY_BIT2IDX(length-lcp)] &
                   (1 << ((length - lcp -1) % _NUM_WORD_BITS_))) ? 1: 0;

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
 *     _trie_v6_merge
 * Purpose:
 *     merge or fuse the child trie with parent trie
 */
int
_trie_v6_merge(trie_node_t *parent_trie,
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
    _trie_v6_search(child_trie, pivot, length, &child, child_pivot, &child_length, 0, 1);
    if (child == NULL) {
        return CTC_E_INVALID_PARAM;
    }

    _CLONE_TRIE_NODE_(&clone, child);

    if (child->type == PAYLOAD && child->bpm) {
        _TAPS_SET_KEY_BIT(bpm, 0, TAPS_IPV6_KEY_SIZE);
    }

    if (child != child_trie) {
        rv = _trie_v6_skip_node_free(child_trie, child_pivot, child_length);
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
    rv = _trie_v6_insert(parent_trie, child_pivot, bpm, child_length, child,
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

STATIC int _trie_v6_fuse_child(trie_node_t *trie, int bit)
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
_trie_v6_split2(trie_node_t *trie,
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
    rv = _key_append_2(pivot, pivot_len, trie->skip_addr, trie->skip_len);
    if (NALPM_E_FAILURE(rv)) return rv;


    lcp = _lcplen_v6(key, key_len, trie->skip_addr, trie->skip_len);

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
                rv = _key_append_2(pivot, pivot_len, bit, 1);
                if (NALPM_E_FAILURE(rv)) return rv;

                rv = _trie_v6_split2(trie->child[bit].child_node,
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
         /*       assert(trie->type == INTERNAL); */
                /* at most one child */
        /*        assert(!(trie->child[0].child_node && trie->child[1].child_node)); */
                /* at least one child */
        /*        assert(trie->child[0].child_node || trie->child[1].child_node); */
                mem_free(trie);
            } else {
           /*     assert(*child == NULL); */
                /* fuse with child if possible */
                trie->child[bit].child_node = NULL;
                bit = (bit==0)?1:0;
                trie->count -= *split_count;

                /* optimize more */
                if ((trie->type == INTERNAL) &&
                        (trie->skip_len +
                         trie->child[bit].child_node->skip_len + 1 <= _MAX_SKIP_LEN_)) {
                    *child = trie->child[bit].child_node;
                    rv = _trie_v6_fuse_child(trie, bit);
                    if (rv != CTC_E_NONE) {
                        *child = NULL;
                    }
                }
                *state = TRIE_SPLIT2_STATE_DONE;
            }
            break;

        case TRIE_SPLIT2_STATE_DONE:
            /* adjust parent's count */
      /*      assert(*split_count > 0);
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


int _trie_v6_skip_node_alloc(trie_node_t **node,
                unsigned int *key,
                /* bpm bit map if bpm management is required, passing null skips bpm management */
                unsigned int *bpm,
                unsigned int msb, /* NOTE: valid msb position 1 based, 0 means skip0/0 node */
                unsigned int skip_len,
                trie_node_t *payload,
                unsigned int count) /* payload count underneath - mostly 1 except some tricky cases */
{
    int lsb=0, msbpos=0, lsbpos=0, bit=0, index;
    trie_node_t *child = NULL, *skip_node = NULL;

    /* calculate lsb bit position, also 1 based */
    lsb = ((msb)? msb + 1 - skip_len : msb);

  /*  assert(((int)msb >= 0) && (lsb >= 0));  */

    if (!node || !key || !payload || msb > _MAX_KEY_LEN_ || msb < skip_len) return CTC_E_INVALID_PARAM;

    if (msb) {
        for (index = BITS2SKIPOFF(lsb), lsbpos = lsb - 1; index <= BITS2SKIPOFF(msb); index++) {
        /* each loop process _MAX_SKIP_LEN number of bits?? */
            if (lsbpos == lsb-1) {
        /* (lsbpos == lsb-1) is only true for first node (loop) here */
                skip_node = payload;
            } else {
        /* other nodes need to be created */
                skip_node = mem_malloc(MEM_IPUC_MODULE, sizeof(trie_node_t));
            }

        /* init memory */
            sal_memset(skip_node, 0, sizeof(trie_node_t));

        /* calculate msb bit position of current chunk of bits we are processing */
            msbpos = index * _MAX_SKIP_LEN_ - 1;
            if (msbpos > msb-1) msbpos = msb-1;

        /* calculate the skip_len of the created node */
            if (msbpos - lsbpos < _MAX_SKIP_LEN_) {
                skip_node->skip_len = msbpos - lsbpos + 1;
            } else {
                skip_node->skip_len = _MAX_SKIP_LEN_;
            }

            /* calculate the skip_addr (skip_length number of bits).
         * skip might be skipping bits on 2 different words
             * if msb & lsb spawns 2 word boundary in worst case
         */
            if (BITS2WORDS(msbpos+1) != BITS2WORDS(lsbpos+1)) {
                /* pull snippets from the different words & fuse */
                skip_node->skip_addr = key[KEY_BIT2IDX(msbpos+1)] & MASK((msbpos+1) % _NUM_WORD_BITS_);
                skip_node->skip_addr = SHL(skip_node->skip_addr,
                                           skip_node->skip_len - ((msbpos+1) % _NUM_WORD_BITS_),
                                           _NUM_WORD_BITS_);
                skip_node->skip_addr |= SHR(key[KEY_BIT2IDX(lsbpos+1)],(lsbpos % _NUM_WORD_BITS_),_NUM_WORD_BITS_);
            } else {
                skip_node->skip_addr = SHR(key[KEY_BIT2IDX(msbpos+1)], (lsbpos % _NUM_WORD_BITS_),_NUM_WORD_BITS_);
            }

        /* set up the chain of child pointer, first node has no child since "child" was inited to NULL */
            if (child) {
                skip_node->child[bit].child_node = child;
            }

        /* calculate child pointer for next loop. NOTE: skip_addr has not been masked
         * so we still have the child bit in the skip_addr here.
         */
            bit = (skip_node->skip_addr & SHL(1, skip_node->skip_len - 1,_MAX_SKIP_LEN_)) ? 1:0;

        /* calculate node type */
            if (lsbpos == lsb-1) {
        /* first node is payload */
                skip_node->type = PAYLOAD;
            } else {
        /* other nodes are internal nodes */
                skip_node->type = INTERNAL;
            }

        /* all internal nodes will have the same "count" as the payload node */
            skip_node->count = count;

            /* advance lsb to next word */
            lsbpos += skip_node->skip_len;

        /* calculate bpm of the skip_node */
            if (bpm) {
        if (lsbpos == _MAX_KEY_LEN_) {
            /* parent node is 0/0, so there is no branch bit here */
            skip_node->bpm = _key_get_bits_2(bpm, lsbpos, skip_node->skip_len, FALSE);
        } else {
            skip_node->bpm = _key_get_bits_2(bpm, lsbpos+1, skip_node->skip_len+1, FALSE);
        }
            }

            /* for all child nodes 0/1 is implicitly obsorbed on parent */
            if (msbpos != msb-1) {
        /* msbpos == (msb-1) is only true for the first node */
        skip_node->skip_len--;
        }
            skip_node->bpm &= MASK(skip_node->skip_len + 1);
            skip_node->skip_addr &= MASK(skip_node->skip_len);
            child = skip_node;
        }
    } else {
        /* skip_len == 0 case, create a payload node with skip_len = 0 and bpm should be 1 bits only
        * bit 0 and bit "skip_len" are same bit (bit 0).
        */
        skip_node = payload;
        sal_memset(skip_node, 0, sizeof(trie_node_t));
        skip_node->type = PAYLOAD;
        skip_node->count = count;
        if (bpm) {
            skip_node->bpm =  _key_get_bits_2(bpm,1,1,TRUE);
        }
        }

    *node = skip_node;
    return CTC_E_NONE;
}


