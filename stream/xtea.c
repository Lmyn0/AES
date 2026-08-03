#include <stdio.h>
#include <stdint.h>

unsigned char key[16] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};
unsigned char plain[8] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07
};
uint32_t K[4] = {0x00};
uint32_t P[2] = {0x00};
uint32_t delta = 0x9e3779b9;

uint32_t lshift(uint32_t z){
    return (z << 4);
}
uint32_t rshift(uint32_t z){
    return (z >> 5);
}
uint32_t F(uint32_t z){
    return ((lshift(z) ^ rshift(z))) + z;
}

void keytoword(const unsigned char key[16], uint32_t K[4]){
    for(int i=0 ; i<4 ; i++){
        K[i] = 0x00;
        for(int j=0 ; j<4 ; j++){
            K[i] |= ((uint32_t)key[4*i + j] << (8 * (3 - j)));
        }
    }
}

void plaintoword(const unsigned char plain[8], uint32_t P[2]){
    for(int i=0 ; i<2 ; i++){
        P[i] = 0x00;
        for(int j=0 ; j<4 ; j++){
            P[i] |= ((uint32_t)plain[4*i + j] << (8 * (3 - j)));
        }
    }
}

void xtea(){
    keytoword(key, K);
    plaintoword(plain, P);

    uint32_t y = P[0];
    uint32_t z = P[1];
    uint32_t sum = 0;
    for(int round=0 ; round<32 ; round++){
        uint32_t i = sum & 3;

        uint32_t A = F(z);
        uint32_t B = sum + K[i];
        uint32_t M = A ^ B;

        y += M;

        sum += delta;

        uint32_t j = (sum >> 11) & 3;

        uint32_t C = F(y);
        uint32_t D = sum + K[j];
        uint32_t N = C ^ D;

        z += N;
    }
    P[0] = y;
    P[1] = z;
}

int main(void){
    xtea();

    printf("%08x %08x", P[0], P[1]);

    return 0;
}