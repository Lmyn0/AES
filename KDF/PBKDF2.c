#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "HMAC-PBKDF2.h"

unsigned char password[] = "password";
unsigned char salt[] = "salt";
unsigned char U[32];
unsigned char next_U[32];
unsigned char output[32];

uint32_t c = 4096;

void F(unsigned char *password, size_t plen, unsigned char *salt, size_t slen, uint32_t c, uint32_t bindex, unsigned char *output){
    unsigned char index_bytes[4];
    unsigned char message[slen + 4];
    //bindex를 4바이트 빅엔디언으로 변환
    for(int i=0 ; i<4 ; i++){
        index_bytes[i] = (bindex >> (24 - (8 * i))) & 0xFF;
    }
    // salt || bindex 값 계산
    for(int i=0 ; i<slen ; i++){
        message[i] = salt[i];
    }
    for(int i=slen ; i<slen + 4 ; i++){
        message[i] = index_bytes[i - slen];
    }
    // U1 생성 및 보관
    HMAC(password, plen, message, slen + 4, U);
    for(int i=0 ; i<32 ; i++){
        output[i] = U[i];
    }
    // U2 부터는 직전에 계산한 U를 다음 HMAC의 메시지로 사용
    for(int i=1 ; i<c ; i++){
        HMAC(password, plen, U, 32, next_U);
        for(int j=0 ; j<32 ; j++){
            output[j] ^= next_U[j];
            U[j] = next_U[j];
        }
    }
}

void PBKDF2(unsigned char *password, size_t plen, unsigned char *salt, size_t slen, uint32_t c, uint32_t dklen, unsigned char *DK){
    unsigned char temp[32];
    uint32_t block_count;
    block_count = (dklen + 31) / 32;

    for(int i=1 ; i<=block_count ; i++){
        F(password, plen, salt, slen, c, i, temp);
        // 마지막 블록에서는 남은 바이트만큼만 복사
        // tlen : 임시 길이값
        uint32_t x = 32 * (i - 1);
        uint32_t y = dklen - x;
        uint32_t tlen;
        if(y >= 32){
            tlen = 32;
        }
        else{
            tlen = y;
        }
        for(int j=0 ; j<tlen ; j++){
            DK[32*(i - 1) + j] = temp[j];  
        }
    }
    for(int i=0 ; i<dklen ; i++){
        printf("%02x ", DK[i]);
    }
}

int main(void){
    size_t plen = strlen((char *)password);
    size_t slen = strlen((char *) salt);

    size_t dklen = 32;

    unsigned char DK[dklen];

    PBKDF2(password, plen, salt, slen, c, dklen, DK);
    return 0;
}