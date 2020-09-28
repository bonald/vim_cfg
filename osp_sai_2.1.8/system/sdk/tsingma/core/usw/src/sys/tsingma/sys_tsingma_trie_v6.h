/****************************************************************************
* cmodel_nlpm_trie_v6.h:
*   nlpm ipv6 trie definitions
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision     : R0.01
* Author       : cuixl
* Date         : 2017-Feb-10 14:40
* Reason       : First Create.
****************************************************************************/

#ifndef __CMODEL_NLPM_TRIE_V6_H__
#define __CMODEL_NLPM_TRIE_V6_H__


extern int sys_trie_v6_find_lpm(trie_node_t *parent,
                             trie_node_t *trie,
                             unsigned int *key,
                             unsigned int length,
                             trie_node_t **payload,
                             trie_callback_f cb,
                             void *user_data);

extern int sys_trie_v6_find_bpm(trie_node_t *trie,
                             unsigned int *key,
                             unsigned int length,
                             int *bpm_length);

extern int sys_trie_v6_skip_node_alloc(trie_node_t **node,
                                    unsigned int *key,
                                    /* bpm bit map if bpm management is required, passing null skips bpm management */
                                    unsigned int *bpm,
                                    unsigned int msb, /* NOTE: valid msb position 1 based, 0 means skip0/0 node */
                                    unsigned int skip_len,
                                    trie_node_t *payload,
                                    unsigned int count);

extern int sys_trie_v6_insert(trie_node_t *trie,
                           unsigned int *key,
                           /* bpm bit map if bpm management is required, passing null skips bpm management */
                           unsigned int *bpm,
                           unsigned int length,
                           trie_node_t *payload, /* payload node */
                           trie_node_t **child /* child pointer if the child is modified */);

extern int sys_trie_v6_delete(trie_node_t *trie,
                           unsigned int *key,
                           unsigned int length,
                           trie_node_t **payload,
                           trie_node_t **child);

extern int sys_trie_v6_split(trie_node_t  *trie,
                          unsigned int *pivot,
                          unsigned int *length,
                          unsigned int *split_count,
                          trie_node_t **split_node,
                          trie_node_t **child,
                          const unsigned int max_count,
                          const unsigned int max_split_len,
                          const int split_to_pair,
                          unsigned int *bpm,
                          trie_split_states_e_t *state);


int
_trie_v6_merge(trie_node_t *parent_trie,
               trie_node_t *child_trie,
               unsigned int *pivot,
               unsigned int length,
               trie_node_t **new_parent);


int _trie_v6_insert(trie_node_t *trie,
                unsigned int *key,
                /* bpm bit map if bpm management is required, passing null skips bpm management */
                unsigned int *bpm,
                unsigned int length,
                trie_node_t *payload, /* payload node */
                trie_node_t **child, /* child pointer if the child is modified */
                int child_count);


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
                const int exact_same);



int _trie_v6_skip_node_alloc(trie_node_t **node,
                unsigned int *key,
                unsigned int *bpm,
                unsigned int msb,
                unsigned int skip_len,
                trie_node_t *payload,
                unsigned int count) ;


#endif /* __CMODEL_NLPM_TRIE_V6_H__ */

