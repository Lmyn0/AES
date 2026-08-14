#include <stdio.h>
#include <stdint.h>
#include "..\hash\blake2b.h"

// 기본 입력값
unsigned char Password[32] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};
unsigned char Salt[16] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};
unsigned char K[8] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};
unsigned char X[12] = {
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04
};
uint32_t p = 4;
uint32_t T = 32;
uint32_t m = 32;
uint32_t t = 3;
uint32_t v = 0x13;
uint32_t y = 2;
uint32_t q = 8; // q = m' / p 
// 사이즈
uint32_t Plen = sizeof(Password);
uint32_t Slen = sizeof(Salt);
uint32_t Klen = sizeof(K);
uint32_t Xlen = sizeof(X);
// 추가
unsigned char H0_input[108];
unsigned char H0[64];
unsigned char B[4][8][1024];
uint32_t low32(uint64_t x){
    return (uint32_t)x;
}
uint64_t rotr64(uint64_t x, uint32_t n){
    return (x >> n) | (x << (64 - n));
}

void LE32(uint32_t a, unsigned char *output){
    for(int i=0 ; i<4 ; i++){
        output[i] = (a >> (8*i)) & 0xff;
    }
}
void makeH0_input(){
    LE32(p, H0_input);
    LE32(T, H0_input + 4);
    LE32(m, H0_input + 8);
    LE32(t, H0_input + 12);
    LE32(v, H0_input + 16);
    LE32(y, H0_input + 20);
    LE32(Plen, H0_input + 24);
    for(int i=0 ; i<Plen ; i++){
        H0_input[i + 28] = Password[i];
    }
    LE32(Slen, H0_input + 60);
    for(int i=0 ; i<Slen ; i++){
        H0_input[i + 64] = Salt[i];
    }
    LE32(Klen, H0_input + 80);
    for(int i=0 ; i<Klen ; i++){
        H0_input[i + 84] = K[i];
    }
    LE32(Xlen, H0_input + 92);
    for(int i=0 ; i<Xlen ; i++){
        H0_input[i + 96] = X[i];
    }
}
void make_H0(){
    blake2b(H0_input, 108, 64);
    for(int i=0 ; i<64; i++){
        H0[i] = output[i];
        printf("%02x ", H0[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
}
void Hprime_1024(unsigned char *A, size_t Alen, unsigned char *out){
    unsigned char temp[76];
    unsigned char V[64];
    LE32(1024, temp);
    for(int i=0 ; i<Alen ; i++){
        temp[i + 4] = A[i];
    }
    blake2b(temp, 76, 64);
    for(int i=0 ; i<64 ; i++){
        V[i] = output[i];
    }
    for(int i=0 ; i<32 ; i++){
        out[i] = V[i];
    }
    for(int j=0 ; j<29 ; j++){
        blake2b(V, 64, 64);
        for(int i=0 ; i<64 ; i++){
            V[i] = output[i];
        }
        for(int i=0 ; i<32 ; i++){
            out[(j+1) * 32 + i] = V[i];
        }
    }
    blake2b(V, 64, 64);
    for(int i=0 ; i<64 ; i++){
        V[i] = output[i];
    }
    for(int i=0 ; i<64 ; i++){
        out[i + 960] = V[i];
    }
}
void make_firstblock(){
    unsigned char block1[72];
    for(int i=0 ; i<4 ; i++){
        for(int j=0 ; j<64 ; j++){
            block1[j] = H0[j];
        }
        LE32(0, block1 + 64);
        LE32(i, block1 + 68);
        Hprime_1024(block1, 72, B[i][0]);
    }
    for(int i=0 ; i<4 ; i++){
        for(int j=0 ; j<64 ; j++){
            block1[j] = H0[j];
        }
        LE32(1, block1 + 64);
        LE32(i, block1 + 68);
        Hprime_1024(block1, 72, B[i][1]);
    }
}
void ArgonG(unsigned char X[1024], unsigned char Y[1024], unsigned char out[1024]){
    unsigned char R[1024];
    uint64_t R_word[128];
    for(int i=0 ; i<1024 ; i++){
        R[i] = X[i] ^ Y[i];
    }
    for(int i=0 ; i<128 ; i++){
        R_word[i] = 0x00;
        for(int j=0 ; j<8 ; j++){
            R_word[i] |= ((uint64_t)R[8*i + j] << 8 * j); 
        }
    }
    
}
void ArgonGB(uint64_t v[16], int a, int b, int c, int d){
    v[a] += v[b] + (2 * (uint64_t)low32(v[a]) * low32(v[b]));
    v[d] = rotr64((v[d] ^ v[a]), 32);
    v[c] += v[d] + (2 * (uint64_t)low32(v[c]) * low32(v[d]));
    v[b] = rotr64((v[b] ^ v[c]), 24);
    v[a] += v[b] + (2 * (uint64_t)low32(v[a]) * low32(v[b]));
    v[d] = rotr64((v[d] ^ v[a]), 16);
    v[c] += v[d] + (2 * (uint64_t)low32(v[c]) * low32(v[d]));
    v[b] = rotr64((v[b] ^ v[c]), 63);
}
void ArgonP(uint64_t v[16]){
    ArgonGB(v, 0, 4,  8, 12);
    ArgonGB(v, 1, 5,  9, 13);
    ArgonGB(v, 2, 6, 10, 14);
    ArgonGB(v, 3, 7, 11, 15);

    ArgonGB(v, 0, 5, 10, 15);
    ArgonGB(v, 1, 6, 11, 12);
    ArgonGB(v, 2, 7,  8, 13);
    ArgonGB(v, 3, 4,  9, 14);
}

int main(){
    makeH0_input();
    make_H0();
}