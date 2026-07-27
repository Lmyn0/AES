#include <stdio.h>
#include <stdint.h>
#include <string.h>
// 고정 상수
uint32_t c0 = 0x61707865;
uint32_t c1 = 0x3320646e;
uint32_t c2 = 0x79622d32;
uint32_t c3 = 0x6b206574;
// 입력 받는 값 -> key, nonce, counter
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
unsigned char nonce[12] = {
    0x00, 0x00, 0x00, 0x09,
    0x00, 0x00, 0x00, 0x4a,
    0x00, 0x00, 0x00, 0x00
};
uint32_t counter = 1;
// 바이트로 입력받은 후 워드 단위로 변환한 값을 저장하는 배열
uint32_t key_words[8];
uint32_t nonce_words[3];
// 바이트를 워드로 변환하는 함수
void keytoword(const unsigned char key[32], uint32_t key_words[8]){
    for(int i=0 ; i<8 ; i++){
        key_words[i] = 0;
        for(int j=0 ; j<4 ; j++){
            key_words[i] |= ((uint32_t)key[4*i + j] << (8 * j));
        }
    }
}
void noncetoword(const unsigned char nonce[12], uint32_t nonce_words[3]){
    for(int i=0 ; i<3 ; i++){
        nonce_words[i] = 0;
        for(int j=0 ; j<4 ; j++){
            nonce_words[i] |= ((uint32_t)nonce[4*i + j] << (8 * j));
        }
    }
}
/* 왼쪽 순환 이동 함수
   x를 n만큼 왼쪽으로 이동 or x를 32-n 만큼 오른쪽으로 이동 */
uint32_t leftr(uint32_t x, int n){
    return (x << n) | (x >> (32 - n));
}
// chacha20 알고리즘에서 사용하는 핵심 함수
void Quarter_Round(uint32_t state[16], int a, int b, int c, int d){
    state[a] += state[b];
    state[d] ^= state[a];
    state[d] = leftr(state[d], 16);
    state[c] += state[d];
    state[b] ^= state[c];
    state[b] = leftr(state[b], 12);
    state[a] += state[b];
    state[d] ^= state[a];
    state[d] = leftr(state[d], 8);
    state[c] += state[d];
    state[b] ^= state[c];
    state[b] = leftr(state[b], 7);
}
// 각 세로 열에 QR 을 적용하는 함수
void Column_Round(uint32_t state[16]){
    Quarter_Round(state, 0, 4, 8, 12);
    Quarter_Round(state, 1, 5, 9, 13);
    Quarter_Round(state, 2, 6, 10, 14);
    Quarter_Round(state, 3, 7, 11, 15);
}
// 대각선으로 QR 을 적용하는 함수
void Diagonal_Round(uint32_t state[16]){
    Quarter_Round(state, 0, 5, 10, 15);
    Quarter_Round(state, 1, 6, 11, 12);
    Quarter_Round(state, 2, 7, 8, 13);
    Quarter_Round(state, 3, 4, 9, 14);
}

int main(){
    keytoword(key, key_words);
    noncetoword(nonce, nonce_words);
    uint32_t originalstate[16];
    unsigned char keystream[64];
    uint32_t state[16] = {
        c0, c1, c2, c3,
        key_words[0], key_words[1], key_words[2], key_words[3],
        key_words[4], key_words[5], key_words[6], key_words[7], 
        counter, nonce_words[0], nonce_words[1], nonce_words[2]
    };
    // 초기상태를 작업용 배열에 복사
    for(int i=0 ; i<16 ; i++){
        originalstate[i] = state[i];
    }
    for(int round=0 ; round<10 ; round++){
        Column_Round(state);
        Diagonal_Round(state);
    }
    // 초기 배열 + 작업한 배열 
    for(int i=0 ; i<16 ; i++){
        state[i] += originalstate[i];
    }
    for(int i=0 ; i<16 ; i++){
        for(int j=0 ; j<4 ; j++){
            keystream[4*i + j] = ((state[i] >> (8 * j)) & 0xFF);
        }
    }
    for(int i=0 ; i<4 ; i++){
        for(int j=0 ; j<16 ; j++){
            printf("%02x ", keystream[4*i + j]);
        }
        printf("\n");
    }
}