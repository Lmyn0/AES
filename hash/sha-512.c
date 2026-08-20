#include <stdint.h>
#include <stdlib.h>

const uint64_t K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};
uint64_t CH(uint64_t x, uint64_t y, uint64_t z){
    return (x & y) | ((~x) & z);
}
uint64_t maj(uint64_t x, uint64_t y, uint64_t z){
    return (x & y) | (x & z) | (y & z);
}
uint64_t rightr(uint64_t x, uint64_t s){
    return (x >> s) | (x << (64-s));
}
uint64_t shr(uint64_t x, uint64_t s){
    return (x >> s);
}
uint64_t s_sigma0(uint64_t x){
    return rightr(x, 1) ^ rightr(x, 8) ^ shr(x, 7);
}
uint64_t s_sigma1(uint64_t x){
    return rightr(x, 19) ^ rightr(x, 61) ^ shr(x, 6);
}
uint64_t b_sigma0(uint64_t x){
    return rightr(x, 28) ^ rightr(x, 34) ^ rightr(x, 39);
}
uint64_t b_sigma1(uint64_t x){
    return rightr(x, 14) ^ rightr(x, 18) ^ rightr(x, 41);
}
void sha512(const unsigned char *message, size_t length, unsigned char digest[64]){ 
    uint64_t word[80];
    uint64_t temp1;
    uint64_t temp2;  
    uint64_t h0 = 0x6a09e667f3bcc908ULL;
    uint64_t h1 = 0xbb67ae8584caa73bULL;
    uint64_t h2 = 0x3c6ef372fe94f82bULL;
    uint64_t h3 = 0xa54ff53a5f1d36f1ULL;
    uint64_t h4 = 0x510e527fade682d1ULL;
    uint64_t h5 = 0x9b05688c2b3e6c1fULL;
    uint64_t h6 = 0x1f83d9abfb41bd6bULL;
    uint64_t h7 = 0x5be0cd19137e2179ULL;
    uint64_t a = h0;
    uint64_t b = h1;
    uint64_t c = h2;
    uint64_t d = h3;
    uint64_t e = h4;
    uint64_t f = h5;
    uint64_t g = h6;
    uint64_t h = h7;

    size_t padded_length = length + 1 + 16;
    while (padded_length % 128 != 0) {
        padded_length++;
    }
    unsigned char *padded = calloc(padded_length, 1);
    if (padded == NULL) {
        return;
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

    for(size_t block = 0 ; block < padded_length ; block += 128){
        for(int i=0 ; i<16 ; i++){
            word[i] = 0;
            for(int j=0 ; j<8 ; j++){
                word[i] |= ((uint64_t)padded[block + 8*i + j]) << (8 * (7-j));
            }
        }
        for(int i=16 ; i<80 ; i++){
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
        for(int x=0 ; x<80 ; x++){
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
    uint64_t hash[8] = {
        h0, h1, h2, h3, h4, h5, h6, h7
    };
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            digest[i * 8 + j] = (hash[i] >> (8 * (7 - j))) & 0xFF;
        }
    }
    free(padded);
}   