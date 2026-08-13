#include <stdio.h>
#include <stdint.h>
#include "PBKDF2.h"
#include "HMAC-KDF.h"

//============= 입력값 ==============
unsigned char P[] = "";
unsigned char Salt[] = "";
uint64_t N = 16;
uint32_t r = 1;
uint32_t p = 1;
size_t dkLen = 64;
//============= 사이즈 ==============
size_t Plen = sizeof(P) - 1;
size_t Slen = sizeof(Salt) - 1;
//============= Salsa20에서 가져옴 =============
uint32_t leftr(uint32_t x, int n){
    return (x << n) | (x >> (32 - n));
}
void Quarter_Round(uint32_t state[16], int a, int b, int c, int d){
    state[b] ^= leftr(state[a] + state[d], 7);
    state[c] ^= leftr(state[b] + state[a], 9);
    state[d] ^= leftr(state[c] + state[b], 13);
    state[a] ^= leftr(state[d] + state[c], 18);
}
void Column_Round(uint32_t state[16]){
    Quarter_Round(state, 0, 4, 8, 12);
    Quarter_Round(state, 5, 9, 13, 1);
    Quarter_Round(state, 10, 14, 2, 6);
    Quarter_Round(state, 15, 3, 7, 11);
}
void Row_Round(uint32_t state[16]){
    Quarter_Round(state, 0, 1, 2, 3);
    Quarter_Round(state, 5, 6, 7, 4);
    Quarter_Round(state, 10, 11, 8, 9);
    Quarter_Round(state, 15, 12, 13, 14);
}
void Salsa20_8(unsigned char *input, unsigned char *output){
    uint32_t state[16];
    uint32_t original[16];
    for(int i=0 ; i<16 ; i++){
        state[i] = 0x00;
        for(int j=0 ; j<4 ; j++){
            state[i] |= ((uint32_t)input[4*i + j] << (8 * j));
        }
    }
    for(int i=0 ; i<16 ; i++){
        original[i] = state[i];
    }   
    for(int round=0 ; round<4 ; round++){
        Column_Round(state);
        Row_Round(state);
    }
    for(int i=0 ; i<16 ; i++){
        state[i] += original[i];
    }
    for(int i=0 ; i<16 ; i++){
        for(int j=0 ; j<4 ; j++){
            output[4*i + j] = (state[i] >> (8 * j)) & 0xff;
        }
    }
}
//============= BlockMix ===============
void BlockMix(unsigned char *input, unsigned char *output, uint32_t r){
    unsigned char X[64];
    unsigned char T[64];
    unsigned char Y[128 * r];
    for(int i=0 ; i<64 ; i++){
        X[i] = input[64 * (2*r - 1) + i];
    }    
    for(int i=0 ; i<2*r ; i++){
        for(int j=0 ; j<64 ; j++){
            T[j] = X[j] ^ input[64*i + j];
        }
        Salsa20_8(T, X);
        for(int j=0 ; j<64; j++){
            Y[64*i + j] = X[j];
        }
    }
    for(int i=0 ; i<r ; i++){
        for(int j=0 ; j<64 ; j++){
            output[64 * i + j] = Y[64 * (2 * i) + j];
            output[64 * (r+i) + j] = Y[64 * (2 * i + 1) + j];
        }
    }
}
//============= ROMix =============
uint64_t integerify(unsigned char *X, uint32_t r){
    uint32_t offset = 64 * (2*r - 1);
    uint64_t value = 0;
    for(int i=0 ; i<8 ; i++){
        value |= ((uint64_t)X[offset + i] << (8 * i)); 
    }
    return value;
}
void ROMix(unsigned char *input, unsigned char *output){
    unsigned char X[128 * r];
    unsigned char V[N * 128 * r];
    unsigned char temp[128 * r];
    uint64_t j = 0;
    for(int i=0 ; i<128*r ; i++){
        X[i] = input[i];
    }
    for(int i=0 ; i<N ; i++){
        for(int j=0 ; j<128*r ; j++){
            V[i * (128*r) + j] = X[j];
        }
        BlockMix(X, temp, r);
        for(int j=0 ; j<128*r ; j++){
            X[j] = temp[j];
        }
    }
    for(int i=0 ; i<N ; i++){
        j = integerify(X, r) % N;
        for(int k=0 ; k<128*r ; k++){
            X[k] ^= V[j*(128*r) + k];
        }
        BlockMix(X, temp, r);
        for(int l=0 ; l<128*r ; l++){
            X[l] = temp[l];
        }
    }
    for(int i=0 ; i<128*r ; i++){
        output[i] = X[i];
    }
}

int main(){

    unsigned char output[128];
    unsigned char DK[64];
    unsigned char B[128 * r * p];
    size_t Blen = sizeof(B);
    PBKDF2(P, Plen, Salt, Slen, 1, Blen, B);
    ROMix(B, output);
    PBKDF2(P, Plen, output, Blen, 1, dkLen, DK);

    for(int i=0 ; i<64 ; i++){
        printf("%02x ", DK[i]);
        if((i+1) % 16 == 0){
            printf("\n");
        }
    }
    return 0;
}