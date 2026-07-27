#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t c0 = 0x61707865;
uint32_t c1 = 0x3320646e;
uint32_t c2 = 0x79622d32;
uint32_t c3 = 0x6b206574;
unsigned char key[32] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f
};
unsigned char nonce[8] = {
    0xa0, 0xa1, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7
};
uint64_t counter = 0;
uint32_t key_words[8];
uint32_t nonce_words[2];
uint32_t counter_words[2];
unsigned char keystream[64];
unsigned char plain[64] = {
    0x48, 0x65, 0x6c, 0x6c, 
    0x6f, 0x2c, 0x20, 0x53,
    0x61, 0x6c, 0x73, 0x61,
    0x32, 0x30, 0x21};
unsigned char cipher[64];

void keytoword(const unsigned char key[32], uint32_t key_words[8]){
    for(int i=0 ; i<8 ; i++){
        key_words[i] = 0;
        for(int j=0 ; j<4 ; j++){
            key_words[i] |= ((uint32_t)key[4*i + j] << (8 * j));
        }
    }
}

void noncetoword(const unsigned char nonce[8], uint32_t nonce_words[2]){
    for(int i=0 ; i<2 ; i++){
        nonce_words[i] = 0;
        for(int j=0 ; j<4 ; j++){
            nonce_words[i] |= ((uint32_t)nonce[4*i + j] << (8 * j));
        }
    }
}

void dividecounter(uint64_t counter, uint32_t counter_words[2]){
    counter_words[0] = (uint32_t)counter;
    counter_words[1] = (uint32_t)(counter >> 32);
}

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

int main(){
    keytoword(key, key_words);
    noncetoword(nonce, nonce_words);
    dividecounter(counter, counter_words);
    uint32_t state[16] = {
        c0, key_words[0], key_words[1], key_words[2],
        key_words[3], c1, nonce_words[0], nonce_words[1],
        counter_words[0], counter_words[1], c2, key_words[4],
        key_words[5], key_words[6], key_words[7], c3
    };
    uint32_t originalstate[16];
    for(int i=0 ; i<16 ; i++){
        originalstate[i] = state[i];
    }
    
    for(int round=0 ; round<10 ; round++){
        Column_Round(state);
        Row_Round(state);
    }
    for(int i=0 ; i<16 ; i++){
        state[i] += originalstate[i];
    }
    for(int i=0 ; i<16 ; i++){
        for(int j=0 ; j<4 ; j++){
            keystream[4*i + j] = ((state[i] >> (8 * j)) & 0xFF);
        }
    }
    
    size_t plain_length = strlen((char*)plain);

    for(int i=0 ; i<plain_length ; i++){
        cipher[i] = keystream[i] ^ plain[i];
    }
    for(size_t i = 0; i < plain_length; i++){
        printf("%02x", cipher[i]);
    }
}