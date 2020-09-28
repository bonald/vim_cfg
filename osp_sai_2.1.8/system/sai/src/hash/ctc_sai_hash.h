#if !defined (__SAI_HASH_H_)
#define __SAI_HASH_H_

#include <saihash.h>
#include <ctc_opf.h>

#define     HASH_ID_MAX_NUM   2

typedef struct ctc_sai_hash_entry_s
{
    sai_object_id_t     hash_id;
    uint32              flags;
} ctc_sai_hash_entry_t;

typedef struct ctc_sai_hash_info_s
{
    ctc_sai_hash_entry_t  hash[HASH_ID_MAX_NUM];
}ctc_sai_hash_info_t;

sai_status_t ctc_sai_hash_init();

ctc_sai_hash_entry_t* ctc_sai_hash_entry_get(sai_object_id_t hash_id);

#endif


