#include <stdio.h>
#include <stdint.h>
#include "../aes.h"
// 입력값
unsigned char K[4][4] = {
    {0x40, 0x44, 0x48, 0x4c},
    {0x41, 0x45, 0x49, 0x4d},
    {0x42, 0x46, 0x4a, 0x4e},
    {0x43, 0x47, 0x4b, 0x4f}
};
unsigned char N[7] = {
    0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16
};
unsigned char P[4] = {
    0x20, 0x21, 0x22, 0x23
};
unsigned char A[8] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07
};
size_t Tlen = 32;
// 사이즈 변환
size_t Klen = sizeof(K) * 8;
size_t Nlen = sizeof(N) * 8;
size_t Alen = sizeof(A) * 8;
size_t Plen = sizeof(P) * 8;
size_t n = sizeof(N);
size_t a = sizeof(A);
size_t p = sizeof(P);

// 배열
unsigned char Q[8];
unsigned char B0[16];
unsigned char B1[16];
unsigned char B2[16];
unsigned char Y0[16];
unsigned char Y1[16];
unsigned char Y2[16];
unsigned char CTR0[16];
unsigned char CTR1[16];
unsigned char CTR2[16];
unsigned char S0[16];
unsigned char S1[16];
size_t qlen = sizeof(Q);
uint32_t Flags = 0;

void makeFlags(size_t t, size_t q, size_t a){
    uint32_t R = 0;
    uint32_t Adata = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    if(a == 0){
        Adata = 0;
    }
    else{
        Adata = 1;
    }
    x = (t-2)/2;
    y = q-1;
    Flags = (R << 7) | (Adata << 6) | (x << 3) | y;
}
void makeQ(size_t p, size_t q){
    for(int i=0 ; i<q ; i++){
        Q[i] = (p >> 8 * (q - 1 - i));
    }
}
void makeB0(){
    B0[0] = Flags;
    for(int i=1 ; i<n+1 ; i++){
        B0[i] = N[i-1];
    }
    for(int i=0 ; i<qlen ; i++){
        B0[i + n + 1] = Q[i];
    }
}
void makeB1(){
    for(int i=0 ; i<2 ; i++){
        B1[i] = (a >> 8 * (1 - i)); 
    }
    for(int i=0 ; i<8 ; i++){
        B1[i + 2] = A[i];
    }
    for(int i=0 ; i<6 ; i++){
        B1[i + 10] = 0x00;
    }
}
void makeB2(){
    for(int i=0 ; i<p ; i++){
        B2[i] = P[i];
    }
    for(int i=0 ; i<16-p ; i++){
        B2[i + p] = 0x00;
    }
}
void CBC_MAC(size_t t, unsigned char K[4][4], unsigned char *T){
    unsigned char temp[16];
    Cipher_AES(B0, K, Y0);
    for(int i=0 ; i<16 ; i++){
        temp[i] = B1[i] ^ Y0[i];
    }
    Cipher_AES(temp, K, Y1);
    for(int i=0 ; i<16 ; i++){
        temp[i] = B2[i] ^ Y1[i];
    }
    Cipher_AES(temp, K, Y2);
    for(int i=0 ; i<t ; i++){
        T[i] = Y2[i];
    }
}
void makeCTR0(size_t q, size_t n){
    CTR0[0] = (q-1);
    for(int i=0 ; i<n ; i++){
        CTR0[i + 1] = N[i];
    }
}
void makeCTR1(size_t q, size_t n){
    CTR1[0] = q-1;
    for(int i=0 ; i<n ; i++){
        CTR1[i + 1] = N[i];
    }
    CTR1[15] = 0x01;
}
void make_S(){
    Cipher_AES(CTR0, K, S0);
    Cipher_AES(CTR1, K, S1);
}
void encrypt(size_t p, size_t t, unsigned char *encrypt_temp1, unsigned char *encrypt_temp2, unsigned char *T, unsigned char *final){
    for(int i=0 ; i<p ; i++){
        encrypt_temp1[i] = P[i] ^ S1[i];
    }
    for(int i=0 ; i<t ; i++){
        encrypt_temp2[i] = T[i] ^ S0[i];
    }
    for(int i=0 ; i<p ; i++){
        final[i] = encrypt_temp1[i];
    }
    for(int i=0 ; i<t ; i++){
        final[i + p] = encrypt_temp2[i];
    }
    for(int i=0 ; i<p+t ; i++){
        printf("%02x ", final[i]);
    }
}
int main(){

    size_t t = Tlen / 8;
    size_t q = 15 - n;
    unsigned char T[t];
    unsigned char encrypt_temp1[p];
    unsigned char encrypt_temp2[t];
    unsigned char final[p + t];
    makeFlags(t, q, a);
    makeQ(p, q);
    makeB0();
    makeB1();
    makeB2();
    CBC_MAC(t, K, T);
    makeCTR0(q, n);
    makeCTR1(q, n);
    make_S();
    encrypt(p, t, encrypt_temp1, encrypt_temp2, T, final);
}