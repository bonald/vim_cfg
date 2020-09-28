
#ifndef __AUTH_MD5_H__
#define __AUTH_MD5_H__

typedef struct
{
    uint32 state[4];
    uint32 count[2];
    uint8 buffer[64];
} AUTH_MD5_CTX;

void auth_md5_init(AUTH_MD5_CTX *);
void auth_md5_update(AUTH_MD5_CTX *, const void *, uint32);
void auth_md5_final(uint8 *, AUTH_MD5_CTX *);

void auth_md5_authenticator(uint8 *);
void auth_hmac_md5(uint8 *, uint32, char *, uint32, uint8 *);

#endif /* __AUTH_MD5_H__ */

