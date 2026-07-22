#include <stdio.h>
#include <string.h>
#include <stdint.h>

const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
uint32_t CH(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) | ((~x) & z);
}
uint32_t maj(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) | (x & z) | (y & z);
}
uint32_t rightr(uint32_t x, uint32_t s){
    return (x >> s) | (x << (32-s));
}
uint32_t shr(uint32_t x, uint32_t s){
    return (x >> s);
}
uint32_t s_sigma0(uint32_t x){
    return rightr(x, 7) ^ rightr(x, 18) ^ shr(x, 3);
}
uint32_t s_sigma1(uint32_t x){
    return rightr(x, 17) ^ rightr(x, 19) ^ shr(x, 10);
}
uint32_t b_sigma0(uint32_t x){
    return rightr(x, 2) ^ rightr(x, 13) ^ rightr(x, 22);
}
uint32_t b_sigma1(uint32_t x){
    return rightr(x, 6) ^ rightr(x, 11) ^ rightr(x, 25);
}
void sha256(const unsigned char *message, size_t length, unsigned char digest[32]){ 
    unsigned char padded[128] = {0x00};
    uint32_t word[64];
    uint32_t temp1;
    uint32_t temp2;
    uint32_t h0 = 0x6a09e667;       
    uint32_t h1 = 0xbb67ae85;
    uint32_t h2 = 0x3c6ef372;
    uint32_t h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f; 
    uint32_t h5 = 0x9b05688c; 
    uint32_t h6 = 0x1f83d9ab; 
    uint32_t h7 = 0x5be0cd19;
    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;
    uint32_t f = h5;
    uint32_t g = h6;
    uint32_t h = h7;

    size_t padded_length = length + 1 + 8;
    
    while (padded_length % 64 != 0) {
        padded_length++;
    }

    size_t length_position = padded_length - 8;

    for(size_t i=0 ; i<length ; i++){ 
        padded[i] = message[i]; 
    } 
    padded[length] = 0x80; 

    uint64_t bit_length = (uint64_t)length * 8;
    
    for(int j=0 ; j<8 ; j++){
        padded[length_position + j] = (bit_length >> (8 * (7-j))) & 0xFF;
    }

    for(size_t block = 0 ; block < padded_length ; block += 64){
        for(int i=0 ; i<16 ; i++){
            word[i] = 0;
            for(int j=0 ; j<4 ; j++){
                word[i] |= ((uint32_t)padded[block + 4*i + j]) << (8 * (3-j));
            }
        }
        for(int i=16 ; i<64 ; i++){
            word[i] = s_sigma1(word[i-2]) + word[i-7] + s_sigma0(word[i-15]) + word[i-16];
        }
        a = h0;
        b = h1;
        c = h2;
        d = h3;
        e = h4;
        f = h5;
        g = h6;
        h = h7;
        for(int x=0 ; x<64 ; x++){
            temp1 = h + b_sigma1(e) + CH(e, f, g) + K[x] + word[x];
            temp2 = b_sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }
    uint32_t hash[8] = {
        h0, h1, h2, h3, h4, h5, h6, h7
    };

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            digest[i * 4 + j] = (hash[i] >> (8 * (3 - j))) & 0xFF;
            printf("%02x", digest[i*4 + j]);
        }
    }
}   

int main(){ 
    unsigned char message[1024]; 
    unsigned char digest[32]; 

    fgets((char *)message, sizeof(message), stdin); 
    
    for(int i=0 ; i<1024 ; i++){ 
        if(message[i] == '\n'){ 
            message[i] = '\0'; 
            break; } 
        if(message[i] == '\0'){ 
            break; } 
        } 
    size_t length = strlen((char *)message); 
        
    sha256(message, length, digest);
}