#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint32_t CH(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) | ((~x) & z);
}
uint32_t parity(uint32_t x, uint32_t y, uint32_t z){
    return x ^ y ^ z;
}
uint32_t maj(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) | (x & z) | (y & z);
}
uint32_t leftr(uint32_t x, uint32_t s){
    return (x << s) | (x >> (32-s));
}

void sha1(const unsigned char *message, size_t length, unsigned char digest[20]){ 
    unsigned char padded[128] = {0x00};
    uint32_t word[80];
    uint32_t temp;
    uint32_t K;
    uint32_t h0 = 0x67452301;       
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;
    
    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;
    uint32_t f;
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
    a = h0;
    b = h1;
    c = h2;
    d = h3;
    e = h4;
        for(int i=16 ; i<80 ; i++){
            word[i] = leftr(word[i-3] ^ word[i-8] ^ word[i-14] ^ word[i-16], 1);
        }
    
        for(int x=0 ; x<80 ; x++){
            if(x<20){
                K = 0x5A827999;
                f = CH(b, c, d);
            }
            else if(x<40){
                K = 0x6ed9eba1;
                f = parity(b, c, d);
            }
            else if(x<60){
                K = 0x8f1bbcdc;
                f = maj(b, c, d);
            }
            else{
                K = 0xca62c1d6;
                f = parity(b, c, d);
            }
            temp = leftr(a, 5) + f + e + K + word[x];
            e = d;
            d = c;
            c = leftr(b, 30);
            b = a;
            a = temp;
        }
    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
    }
    for(int i=0 ; i<4 ; i++){
        digest[i] = (h0 >> (8 * (3-i))) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[4+i] = (h1 >> (8 * (3-i))) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[8+i] = (h2 >> (8 * (3-i))) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[12+i] = (h3 >> (8 * (3-i))) & 0xFF; 
    }
    for(int i=0 ; i<4 ; i++){
        digest[16+i] = (h4 >> (8 * (3-i))) & 0xFF; 
    }
    for(int i=0 ; i<20 ; i++){
        printf("%02x", digest[i]);
    }
}   

int main(){ 
    unsigned char message[1024]; 
    unsigned char digest[20]; 

    fgets((char *)message, sizeof(message), stdin); 
    
    for(int i=0 ; i<1024 ; i++){ 
        if(message[i] == '\n'){ 
            message[i] = '\0'; 
            break; } 
        if(message[i] == '\0'){ 
            break; } 
        } 
    size_t length = strlen((char *)message); 
        
    sha1(message, length, digest);
}