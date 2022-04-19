#include "btreestore.h"
#include "nmmintrin.h" // for SSE4.2
#include "immintrin.h" // for AVX 


void decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4]) 
{
    uint64_t sum = 0xDDE6E400;
    uint64_t delta = 0x9E3779B9;
    uint32_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;

    for (int i = 0; i < 1024; i++) {
        tmp4 = ((cipher[0] << 4) + key[2]) & RAISE32 - 1;
        tmp5 = (cipher[0] + sum) & RAISE32 - 1;
        tmp6 = ((cipher[0] >> 5) + key[3]) & RAISE32 - 1;
        cipher[1] = (cipher[1] - ((tmp4 ^ tmp5) ^ tmp6)) & RAISE32 - 1;

        tmp1 = ((cipher[1] << 4) + key[0]) & RAISE32 - 1;
        tmp2 = (cipher[1] + sum) & RAISE32 - 1;
        tmp3 = ((cipher[1] >> 5) + key[1]) & RAISE32 - 1;
        cipher[0] = (cipher[0] - ((tmp1 ^ tmp2) ^ tmp3)) & RAISE32 - 1;
        sum = (sum - delta) & RAISE32 - 1;
    }
    plain[0] = cipher[0];
    plain[1] = cipher[1];

    return;
}

void encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4]) 
{
    uint64_t sum = 0;
    uint64_t delta = 0x9E3779B9;
    uint32_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    cipher[0] = plain[0];
    cipher[1] = plain[1];

    for (int i = 0; i < 1024; i++) {
        sum = (sum + delta) & RAISE32 - 1;
        tmp1 = ((cipher[1] << 4) + key[0]) & RAISE32 - 1;
        tmp2 = (cipher[1] + sum) & RAISE32 - 1;
        tmp3 = ((cipher[1] >> 5) + key[1]) & RAISE32 - 1;
        cipher[0] = (cipher[0] + ((tmp1 ^ tmp2) ^ tmp3)) & RAISE32 - 1;
        tmp4 = ((cipher[0] << 4) + key[2]) & RAISE32 - 1;
        tmp5 = (cipher[0] + sum) & RAISE32 - 1;
        tmp6 = ((cipher[0] >> 5) + key[3]) & RAISE32 - 1;
        cipher[1] = (cipher[1] + ((tmp4 ^ tmp5) ^ tmp6)) & RAISE32 - 1;
    }

    return;
}

void encrypt_tea_ctr(uint64_t * plain, //
                     uint32_t key[4], 
                     uint64_t nonce, 
                     uint64_t * cipher, 
                     uint32_t num_blocks) 
{
    uint64_t tmp1, tmp2;
    uint32_t tmp1_arr[2], tmp2_arr[2];

    uint64_t *plaintext = malloc(num_blocks * 10);
    memset((char *)plaintext, 0, num_blocks * 8);
    memcpy((char *)plaintext, (char *)plain, (num_blocks * 8));

    for (uint32_t i = 0; i < (num_blocks); i++) {
        tmp1 = i ^ nonce;//
        memcpy(tmp1_arr, &tmp1, sizeof(uint32_t) * 2);
        encrypt_tea(tmp1_arr, tmp2_arr, key);
        memcpy(&tmp2, tmp2_arr, sizeof(uint32_t) * 2);
        cipher[i] = plaintext[i] ^ tmp2;
    }
    free(plaintext);

    return;
}

void decrypt_tea_ctr(uint64_t * cipher, 
                     uint32_t key[4], 
                     uint64_t nonce, 
                     uint64_t * plain, 
                     uint32_t num_blocks) 
{
    uint64_t tmp1, tmp2;
    uint32_t tmp1_arr[2], tmp2_arr[2];

    uint64_t *ciphertext = malloc(num_blocks * 10);
    memset((char *)ciphertext, 0, num_blocks * 8);
    memcpy((char *)ciphertext, (char *)cipher, (num_blocks * 8));

    for (uint32_t i = 0; i < (num_blocks); i++) {
/*
cipher[i] = plaintext[i] ^ tmp2 == plaintext[i] = cipher[i] ^ tmp2
*/
        tmp1 = i ^ nonce;
        memcpy(tmp1_arr, &tmp1, sizeof(uint32_t) * 2);
        encrypt_tea(tmp1_arr, tmp2_arr, key);
        memcpy(&tmp2, tmp2_arr, sizeof(uint32_t) * 2);
        plain[i] = cipher[i] ^ tmp2;
    }
    free(ciphertext);

    return;
}