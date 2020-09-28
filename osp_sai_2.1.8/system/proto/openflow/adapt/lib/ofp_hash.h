#ifndef __OFP_HASH_H__
#define __OFP_HASH_H__

static inline void 
ofp_hash_key_init(uint32* p_hash)
{  
    /* Invented by Dr. Daniel J.Bernstein */
    uint32 hash = 5381;  

    *p_hash = hash;
}  

static inline void 
ofp_hash_key_array(uint32* p_hash, uint8* p_array, uint32 len)
{  
    uint32 hash = *p_hash;
    uint32 i;

    for(i = 0; i < len; i++)  
    {
        hash = ((hash << 5) + hash) + p_array[i];  
    }

    *p_hash = hash;
}

static inline void 
ofp_hash_key_uint8(uint32* p_hash, uint8 value)
{
    ofp_hash_key_array(p_hash, &value, sizeof(value));
}  

static inline void 
ofp_hash_key_uint16(uint32* p_hash, uint16 value)
{
    ofp_hash_key_array(p_hash, (uint8*)&value, sizeof(value));
}

static inline void 
ofp_hash_key_uint32(uint32* p_hash, uint32 value)
{
    ofp_hash_key_array(p_hash, (uint8*)&value, sizeof(value));
}

static inline void 
ofp_hash_key_mac(uint32* p_hash, uint8* mac)
{
    ofp_hash_key_array(p_hash, (uint8*)mac, MAC_ADDR_LEN);
}

static inline uint32 
ofp_hash_make(uint8* p_array, uint32 len)  
{
    uint32 hash;  
    ofp_hash_key_init(&hash);
    ofp_hash_key_array(&hash, p_array, len);

    return hash;  
}  

#endif /* !__OFP_HASH_H__ */

