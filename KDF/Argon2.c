#include <stdio.h>
#include <stdint.h>
#include "..\hash\blake2b.h"
// 기본 입력값
unsigned char Password[32] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};
unsigned char Salt[16] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};
unsigned char K[8] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};
unsigned char X[12] = {
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04
};
uint32_t p = 4;
uint32_t T = 32;
uint32_t m = 32;
uint32_t mprime = 32;
uint32_t t = 3;
uint32_t v = 0x13;
uint32_t y = 2;
uint32_t q = 8; // q = m' / p 
// 사이즈
uint32_t Plen = sizeof(Password);
uint32_t Slen = sizeof(Salt);
uint32_t Klen = sizeof(K);
uint32_t Xlen = sizeof(X);
// 추가
unsigned char H0_input[108];
unsigned char H0[64];
unsigned char B[4][8][1024];
unsigned char C[1024];
unsigned char Tag[32];
uint32_t low32(uint64_t x){
    return (uint32_t)x;
}
uint64_t rotr64(uint64_t x, uint32_t n){
    return (x >> n) | (x << (64 - n));
}
void LE32(uint32_t a, unsigned char *output){
    for(int i=0 ; i<4 ; i++){
        output[i] = (a >> (8*i)) & 0xff;
    }
}
void LE64(uint64_t a, unsigned char *output){
    for(int i=0 ; i<8 ; i++){
        output[i] = (a >> (8*i)) & 0xff;
    }
}
void makeH0_input(){
    LE32(p, H0_input);
    LE32(T, H0_input + 4);
    LE32(m, H0_input + 8);
    LE32(t, H0_input + 12);
    LE32(v, H0_input + 16);
    LE32(y, H0_input + 20);
    LE32(Plen, H0_input + 24);
    for(int i=0 ; i<Plen ; i++){
        H0_input[i + 28] = Password[i];
    }
    LE32(Slen, H0_input + 60);
    for(int i=0 ; i<Slen ; i++){
        H0_input[i + 64] = Salt[i];
    }
    LE32(Klen, H0_input + 80);
    for(int i=0 ; i<Klen ; i++){
        H0_input[i + 84] = K[i];
    }
    LE32(Xlen, H0_input + 92);
    for(int i=0 ; i<Xlen ; i++){
        H0_input[i + 96] = X[i];
    }
}
void make_H0(){
    blake2b(H0_input, 108, 64);
    for(int i=0 ; i<64; i++){
        H0[i] = output[i];
    }
}
void Hprime_1024(unsigned char *A, size_t Alen, unsigned char *out){
    unsigned char temp[76];
    unsigned char V[64];
    LE32(1024, temp);
    for(int i=0 ; i<Alen ; i++){
        temp[i + 4] = A[i];
    }
    blake2b(temp, 76, 64);
    for(int i=0 ; i<64 ; i++){
        V[i] = output[i];
    }
    for(int i=0 ; i<32 ; i++){
        out[i] = V[i];
    }
    for(int j=0 ; j<29 ; j++){
        blake2b(V, 64, 64);
        for(int i=0 ; i<64 ; i++){
            V[i] = output[i];
        }
        for(int i=0 ; i<32 ; i++){
            out[(j+1) * 32 + i] = V[i];
        }
    }
    blake2b(V, 64, 64);
    for(int i=0 ; i<64 ; i++){
        V[i] = output[i];
    }
    for(int i=0 ; i<64 ; i++){
        out[i + 960] = V[i];
    }
}
void Hprime_32(unsigned char *A, size_t Alen, unsigned char *Tag){
    unsigned char tmp[1028];
    LE32(32, tmp);
    for(int i=0 ; i<Alen ; i++){
        tmp[i + 4] = A[i];
    }
    blake2b(tmp, Alen + 4, 32);
    for(int i=0 ; i<32 ; i++){
        Tag[i] = output[i];
    }
}
void make_firstblock(){
    unsigned char block1[72];
    for(int i=0 ; i<4 ; i++){
        for(int j=0 ; j<64 ; j++){
            block1[j] = H0[j];
        }
        LE32(0, block1 + 64);
        LE32(i, block1 + 68);
        Hprime_1024(block1, 72, B[i][0]);
    }
    for(int i=0 ; i<4 ; i++){
        for(int j=0 ; j<64 ; j++){
            block1[j] = H0[j];
        }
        LE32(1, block1 + 64);
        LE32(i, block1 + 68);
        Hprime_1024(block1, 72, B[i][1]);
    }
}
void ArgonGB(uint64_t v[16], int a, int b, int c, int d){
    v[a] += v[b] + (2 * (uint64_t)low32(v[a]) * low32(v[b]));
    v[d] = rotr64((v[d] ^ v[a]), 32);
    v[c] += v[d] + (2 * (uint64_t)low32(v[c]) * low32(v[d]));
    v[b] = rotr64((v[b] ^ v[c]), 24);
    v[a] += v[b] + (2 * (uint64_t)low32(v[a]) * low32(v[b]));
    v[d] = rotr64((v[d] ^ v[a]), 16);
    v[c] += v[d] + (2 * (uint64_t)low32(v[c]) * low32(v[d]));
    v[b] = rotr64((v[b] ^ v[c]), 63);
}
void ArgonP(uint64_t v[16]){
    ArgonGB(v, 0, 4,  8, 12);
    ArgonGB(v, 1, 5,  9, 13);
    ArgonGB(v, 2, 6, 10, 14);
    ArgonGB(v, 3, 7, 11, 15);

    ArgonGB(v, 0, 5, 10, 15);
    ArgonGB(v, 1, 6, 11, 12);
    ArgonGB(v, 2, 7,  8, 13);
    ArgonGB(v, 3, 4,  9, 14);
}
void ArgonG(unsigned char X[1024], unsigned char Y[1024], unsigned char out[1024]){
    unsigned char R[1024];
    unsigned char Z[1024];
    uint64_t R_word[128];
    uint64_t temp[16];
    for(int i=0 ; i<1024 ; i++){
        R[i] = X[i] ^ Y[i];
    }
    for(int i=0 ; i<128 ; i++){
        R_word[i] = 0x00;
        for(int j=0 ; j<8 ; j++){
            R_word[i] |= ((uint64_t)R[8*i + j] << 8 * j); 
        }
    }
    for(int i=0 ; i<8 ; i++){
        for(int j=0 ; j<16 ; j++){
            temp[j] = R_word[16*i + j];
        }
        ArgonP(temp);
        for(int j=0 ; j<16 ; j++){
            R_word[16*i + j] = temp[j];
        }
    }
    for(int i=0 ; i<8 ; i++){
        for(int j=0 ; j<8 ; j++){
            temp[2 * j] = R_word[16*j + 2*i];
            temp[2 * j + 1] = R_word[16*j + 2*i + 1];
        }
        ArgonP(temp);
        for(int j=0 ; j<8 ; j++){
            R_word[16*j + 2*i]     = temp[2*j];
            R_word[16*j + 2*i + 1] = temp[2*j + 1];
        }
    }
    for(int i=0 ; i<128 ; i++){
        for(int j=0 ; j<8 ; j++){
            Z[8*i + j] = (R_word[i] >> (8 * j)) & 0xff;
        }
    }
    for(int i=0 ; i<1024 ; i++){
        out[i] = Z[i] ^ R[i];
    }
}
void J_independent(uint32_t pass, uint32_t lane, uint32_t slice, uint32_t index, uint32_t *J1, uint32_t *J2){
    unsigned char input_block[1024];
    unsigned char address_block[1024];
    unsigned char zero_block[1024];
    unsigned char temp[1024];
    uint64_t J = 0;
    for(int i=0 ; i<1024 ; i++){
        input_block[i] = 0x00;
        zero_block[i] = 0x00;
    }
    LE64(pass, input_block + 0);
    LE64(lane, input_block + 8);
    LE64(slice, input_block + 16);
    LE64(mprime, input_block + 24);
    LE64(t, input_block + 32);
    LE64(y, input_block + 40);
    LE64(1, input_block + 48);
    LE64(0, input_block + 56);

    ArgonG(zero_block, input_block, temp);
    ArgonG(zero_block, temp, address_block);

    for(int i=0 ; i<8 ; i++){
        J |= ((uint64_t)address_block[8*index + i] << (8*i));
    }
    *J1 = (uint32_t)J;
    *J2 = (uint32_t)(J >> 32);
}
void J_dependent(unsigned char *prev, uint32_t *J1, uint32_t *J2){
    uint64_t J = 0;

    for(int i=0 ; i<8 ; i++){
        J |= ((uint64_t)prev[i] << (8*i));
    }

    *J1 = (uint32_t)J;
    *J2 = (uint32_t)(J >> 32);
}
uint32_t get_L(uint32_t J2, uint32_t pass, uint32_t lane, uint32_t slice){
    uint32_t l;
    if(pass == 0 && slice == 0){
        l = lane;
    }
    else{
        l = J2 % p;
    }
    return l;
}
uint32_t W(uint32_t pass, uint32_t lane, uint32_t slice, uint32_t l, uint32_t j){
    uint32_t segmentlen = q / 4;
    uint32_t index = j % segmentlen;
    uint32_t w = 0;
    if(pass == 0){
        if(l == lane){
            w = slice * segmentlen + index - 1;
        }
        else{
            w = slice * segmentlen;
            if(index == 0){
                w = w - 1;
            }
        }
    }
    else{
        if(l == lane){
            w = q - segmentlen + index - 1;
        }
        else{
            w = q - segmentlen;
            if(index == 0){
                w = w - 1;
            }
        }
    }
    return w;
}
uint32_t get_Z(uint32_t J1, uint32_t pass, uint32_t lane, uint32_t slice, uint32_t l, uint32_t j){
    uint32_t w = W(pass, lane, slice, l, j);
    uint64_t x = ((uint64_t)J1 * J1) >> 32;
    uint64_t y = ((uint64_t)w * x) >> 32;
    uint32_t zz = w - 1 - y;
    uint32_t start = 0;
    if(pass == 0){
        start = 0;
    }
    else{
        start = ((slice + 1) * (q / 4)) % q;
    }
    uint32_t z = (start + zz) % q;
    return z;
}
void fill_memory(){
    uint32_t pass;
    uint32_t slice;
    uint32_t lane;
    uint32_t index;
    uint32_t J1;
    uint32_t J2;
    uint32_t prev;
    unsigned char temp[1024];
    for(pass=0 ; pass<t ; pass++){
        for(slice=0 ; slice<4 ; slice++){
            for(lane=0 ; lane<p ; lane++){
                for(index=0 ; index<q/4 ; index++){
                    uint32_t j = slice * (q/4) + index;
                    if(j == 0){
                        prev = q - 1;
                    }
                    else{
                        prev = j - 1;
                    }
                    if(pass == 0 && slice == 0){
                        continue;
                    }
                    if(pass == 0 && (slice == 0 || slice == 1)){
                        J_independent(pass, lane, slice, index, &J1, &J2);
                    }
                    else{
                        J_dependent(B[lane][prev], &J1, &J2);
                    }
                    uint32_t l = get_L(J2, pass, lane, slice);
                    uint32_t z = get_Z(J1, pass, lane, slice, l, j);
                    if(pass == 0){
                        ArgonG(B[lane][prev], B[l][z], B[lane][j]);
                    }
                    else{
                        ArgonG(B[lane][prev], B[l][z], temp);

                        for(int k=0 ; k<1024 ; k++){
                        B[lane][j][k] ^= temp[k];
                        }
                    }
                }
            }
        }
    }
}
void make_C(){
    for(int lane=0 ; lane<p ; lane++){
        for(int i=0 ; i<1024 ; i++){
            C[i] ^= B[lane][q-1][i];
        }
    }
}
int main(){
    makeH0_input();
    make_H0();
    make_firstblock();

    fill_memory();
    make_C();
    Hprime_32(C, 1024, Tag);
    for(int i=0 ; i<32 ; i++){
        printf("%02x ", Tag[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
    return 0;
}
