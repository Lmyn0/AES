#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "HMAC-KDF.h"

unsigned char IKM[22] = {
    0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b
};
unsigned char salt[13] = {
    0x00, 0x01, 0x02, 0x03, 
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c
};
unsigned char info[10] = {
    0xf0, 0xf1, 0xf2, 0xf3,
    0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9
};
unsigned char OKM[42];
unsigned char PRK[32];
uint32_t L = 42;

void extract(unsigned char *salt, size_t slen, unsigned char *IKM, size_t IKMlen, unsigned char *PRK){
    HMAC(salt, slen, IKM, IKMlen, PRK);
}

void expand(unsigned char *PRK, size_t plen, unsigned char *info, size_t ilen, uint32_t L, unsigned char *OKM){
    uint32_t N = (L + 31) / 32;
    unsigned char message[ilen + 1];
    unsigned char T[32];
    size_t mlen = ilen + 1;
    memcpy(message, info, ilen);
    message[ilen] = 0x01;
    //T1 생성 후 OKM에 복사
    HMAC(PRK, plen, message, mlen, T);
    memcpy(OKM, T, 32);
    //T2 생성
    unsigned char message1[32 + ilen + 1];
    size_t m1len = 32 + ilen + 1;
    memcpy(message1, T, 32);
    memcpy(message1 + 32, info, ilen);
    message1[32 + ilen] = 0x02;
    
    HMAC(PRK, plen, message1, m1len, T);
    memcpy(OKM + 32, T, L - 32);

    for(int i=0 ; i<L ; i++){
        printf("%02x ", OKM[i]);
        if((i + 1) % 16 == 0){
            printf("\n");
        }
    }
}

int main(){
    size_t slen = sizeof(salt);
    size_t IKMlen = sizeof(IKM);
    size_t ilen = sizeof(info);
    size_t plen = sizeof(PRK);

    extract(salt, slen, IKM, IKMlen, PRK);
    expand(PRK, plen, info, ilen, L, OKM);
}