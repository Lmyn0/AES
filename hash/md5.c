#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint32_t F(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) | ((~x) & z);
}
uint32_t G(uint32_t x, uint32_t y, uint32_t z){
    return (x & z) | (y & (~z));
}
uint32_t H(uint32_t x, uint32_t y, uint32_t z){
    return x ^ y ^ z;
}
uint32_t I(uint32_t x, uint32_t y, uint32_t z){
    return y ^ (x | (~z));
}
const uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
const uint32_t r1[16] = {
    7, 12, 17, 22, 
    7, 12, 17, 22, 
    7, 12, 17, 22, 
    7, 12, 17, 22
};
const uint32_t r2[16] = {
    5, 9, 14, 20, 
    5, 9, 14, 20, 
    5, 9, 14, 20, 
    5, 9, 14, 20
};
const uint32_t r3[16] = {
    4, 11, 16, 23, 
    4, 11, 16, 23, 
    4, 11, 16, 23, 
    4, 11, 16, 23
};
const uint32_t r4[16] = {
    6, 10, 15, 21, 
    6, 10, 15, 21, 
    6, 10, 15, 21, 
    6, 10, 15, 21
};

uint32_t leftr(uint32_t x, uint32_t s){
    return (x << s) | (x >> (32-s));
}

void md5(const unsigned char *message, size_t length, unsigned char digest[16]){ 
    unsigned char padded[128] = {0x00}; 
    uint32_t word[16];
    uint32_t temp;
    uint32_t A = 0x67452301;
    uint32_t B = 0xefcdab89;
    uint32_t C = 0x98badcfe;
    uint32_t D = 0x10325476;

    uint32_t a = A;
    uint32_t b = B;
    uint32_t c = C;
    uint32_t d = D;
    uint32_t f;
    uint32_t s;
    uint32_t new_b;
    uint32_t old_b;
    int g;
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
        padded[length_position + j] = (bit_length >> (8 * j)) & 0xFF;
    }
    
    for(size_t block = 0 ; block < padded_length ; block += 64){
        for(int i=0 ; i<16 ; i++){
            word[i] = 0;
            for(int j=0 ; j<4 ; j++){
                word[i] |= ((uint32_t)padded[block + 4*i + j]) << (8 * j);
            }
        }
    a = A;
    b = B;
    c = C;
    d = D;
    for(int x=0 ; x<64 ; x++){
        if(x<16){
            f = F(b, c, d);
            g = x;
            s = r1[x];
        }   
        else if(x<32){
            int j = x - 16;

            f = G(b, c, d);
            g = (1 + 5 * j) % 16;
            s = r2[j];
        }
        else if(x<48){
            int j = x - 32;

            f = H(b, c, d);
            g = (5 + 3* j) % 16;
            s = r3[j];
        }
        else{
            int j = x - 48;

            f = I(b, c, d);
            g = (7 * j) % 16;
            s = r4[j];
        }
        temp = a + f + word[g] + K[x];
        new_b = b + leftr(temp, s);
        old_b = b;
        a = d;
        d = c;
        c = old_b;
        b = new_b;
        }
    A += a;
    B += b;
    C += c;
    D += d;
    }
    for(int i=0 ; i<4 ; i++){
        digest[i] = (A >> (8 * i)) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[4+i] = (B >> (8 * i)) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[8+i] = (C >> (8 * i)) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[12+i] = (D >> (8 * i)) & 0xFF; 
    }
    for(int i=0 ; i<16 ; i++){
        printf("%02x", digest[i]);
    }
}   

int main(){ 
    unsigned char message[1024]; 
    unsigned char digest[16]; 

    fgets((char *)message, sizeof(message), stdin); 
    
    for(int i=0 ; i<1024 ; i++){ 
        if(message[i] == '\n'){ 
            message[i] = '\0'; 
            break; } 
        if(message[i] == '\0'){ 
            break; } 
        } 
    size_t length = strlen((char *)message); 
        
    md5(message, length, digest);
}