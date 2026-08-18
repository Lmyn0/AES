#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

uint64_t IV[8] = {
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
    0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
};
unsigned char Sigma[10][16] ={
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14 ,10 ,4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0}
};
unsigned char output[64];
uint64_t rotr(uint64_t x, uint32_t n){
    return (x >> n) | (x << (64 - n));
}

void blakeG(uint64_t v[16], int a, int b, int c, int d, uint64_t x, uint64_t y){
    v[a] += v[b] + x;
    v[d] = rotr((v[d] ^ v[a]), 32);
    v[c] += v[d];
    v[b] = rotr((v[b] ^ v[c]), 24);
    v[a] += v[b] + y;
    v[d] = rotr((v[d] ^ v[a]), 16);
    v[c] += v[d];
    v[b] = rotr((v[b] ^ v[c]), 63);
}
void bytes_to_words(unsigned char *block, uint64_t m[16]){
    for(int i=0 ; i<16 ; i++){
        m[i] = 0x00;
        for(int j=0 ; j<8 ; j++){
            m[i] |= ((uint64_t)block[8*i + j] << (8 * j)); 
        }
    }
}
void blakeF(uint64_t h[8], uint64_t m[16], uint64_t t_low, uint64_t t_high, bool f){
    uint64_t v[16];
    for(int i=0 ; i<8 ; i++){
        v[i] = h[i];
    }
    for(int i=0 ; i<8 ; i++){
        v[i + 8] = IV[i];
    }
    v[12] ^= t_low;
    v[13] ^= t_high;
    if(f == true){
        v[14] = ~v[14];
    }
    for(int i=0 ; i<12 ; i++){
        uint64_t x = 0;
        uint64_t y = 0;
        // 세로 G
        x = m[Sigma[i%10][0]];
        y = m[Sigma[i%10][1]];
        blakeG(v, 0, 4, 8, 12, x, y);
        x = m[Sigma[i%10][2]];
        y = m[Sigma[i%10][3]];
        blakeG(v, 1, 5, 9, 13, x, y);
        x = m[Sigma[i%10][4]];
        y = m[Sigma[i%10][5]];
        blakeG(v, 2, 6, 10, 14, x, y);
        x = m[Sigma[i%10][6]];
        y = m[Sigma[i%10][7]];
        blakeG(v, 3, 7, 11, 15, x, y);
        // 대각선 G
        x = m[Sigma[i%10][8]];
        y = m[Sigma[i%10][9]];
        blakeG(v, 0, 5, 10, 15, x, y);
        x = m[Sigma[i%10][10]];
        y = m[Sigma[i%10][11]];
        blakeG(v, 1, 6, 11, 12, x, y);
        x = m[Sigma[i%10][12]];
        y = m[Sigma[i%10][13]];
        blakeG(v, 2, 7, 8, 13, x, y);
        x = m[Sigma[i%10][14]];
        y = m[Sigma[i%10][15]];
        blakeG(v, 3, 4, 9, 14, x, y);
    }
    for(int i=0 ; i<8 ; i++){
        h[i] = h[i] ^ v[i] ^ v[i + 8];
    }
}
void blake2b(unsigned char *message, size_t mlen, size_t nn){
    unsigned char temp[128];
    size_t blocks = mlen / 128;
    size_t remain = mlen % 128;
    uint64_t m[16];
    uint64_t h[8];
    uint64_t total = 0;
    for(int k=0 ; k<8 ; k++){
        h[k] = IV[k]; 
    } 
    h[0] ^= 0x01010000 ^ nn;
    for(size_t i=0 ; i<blocks ; i++){
        bool final = false;
        if(remain == 0 && i == blocks-1){
            final = true;
        }
        for(int j=0 ; j<128 ; j++){
            temp[j] = message[128*i + j];
        }
        bytes_to_words(temp, m);
        total += 128;
        blakeF(h, m, total, 0, final);
    }
    if(remain > 0){
        for(int i=0 ; i<128 ; i++){
            temp[i] = 0x00;
        }
        for(size_t j=0 ; j<remain ; j++){
            temp[j] = message[blocks*128 + j];
        }
        bytes_to_words(temp, m);
        total += remain;
        blakeF(h, m, total, 0, true);
    }
    for(int i=0 ; i<8 ; i++){
        for(int j=0 ; j<8 ; j++){
            output[8*i + j] = (h[i] >> (8 * j)) & 0xff;
        }
    }
}
/*
int main(){
    unsigned char block[128];
    unsigned char message[] = "abc";
    uint64_t h[8];
    uint64_t m[16];
    size_t mlen = sizeof(message) - 1;
    size_t nn = 64;

    blake2b(message, mlen, m, h, nn);
}*/