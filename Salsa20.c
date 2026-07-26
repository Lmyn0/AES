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

void mword(const unsigned char key[32], uint32_t key_words[8]){
    for(int i=0 ; i<8 ; i++){
        key_words[i] = 0;
        for(int j=0 ; j<4 ; j++){
            key_words[i] |= ((uint32_t)key[4*i + j] << (8 * j));
        }
    }
}
int main(){
    mword(key, key_words);
    for(int i=0 ; i<8 ; i++){
        printf("%08x\n", key_words[i]);
    }
}