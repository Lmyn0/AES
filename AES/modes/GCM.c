#include <stdio.h>
#include <stdint.h>
#include "../aes.h"

unsigned char R[16] = {
    0xe1, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
unsigned char Z[16];
unsigned char X[16] = {
    0x03, 0x88, 0xda, 0xce,
    0x60, 0xb6, 0xa3, 0x92,
    0xf3, 0x28, 0xc2, 0xb9,
    0x71, 0xb2, 0xfe, 0x78
};
unsigned char Y[16] = {
    0x66, 0xe9, 0x4b, 0xd4,
    0xef, 0x8a, 0x2c, 0x3b,
    0x88, 0x4c, 0xfa, 0x59,
    0xca, 0x34, 0x2b, 0x2e
};
unsigned char tempX[16];
unsigned char H[16];
unsigned char empty[16];
unsigned char J[16];
unsigned char tempJ[16];
unsigned char S[16];
unsigned char key[4][4] = {
    {0xfe, 0x86, 0x6d, 0x67},
    {0xff, 0x65, 0x6a, 0x30},
    {0xe9, 0x73, 0x8f, 0x83},
    {0x92, 0x1c, 0x94, 0x08}
};
unsigned char IV[12] = {
    0xca, 0xfe, 0xba, 0xbe,
    0xfa, 0xce, 0xdb, 0xad,
    0xde, 0xca, 0xf8, 0x88
};
unsigned char P[60] = {
    0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
    0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
    0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
    0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
    0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
    0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
    0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
    0xba, 0x63, 0x7b, 0x39
};
unsigned char A[20] = {
    0xfe, 0xed, 0xfa, 0xce,
    0xde, 0xad, 0xbe, 0xef,
    0xfe, 0xed, 0xfa, 0xce,
    0xde, 0xad, 0xbe, 0xef,
    0xab, 0xad, 0xda, 0xd2
};
unsigned char C[60];
unsigned char T[16];

size_t block_len(size_t X_len){
    if(X_len >= 16){
        return 16;
    }
    else{
        return X_len;
    }
}
void inc32(unsigned char *X){
    for(int i=15 ; i>=12 ; i--){
        X[i] += 0x01;
        if(X[i] != 0x00){
            break;
        }
    }
}
void GF(unsigned char *X, unsigned char *Y, unsigned char *result){
    unsigned char V[16];
    int x_i = 0;
    int v_i = 0;
    int temp = 0;
    for(int i=0 ; i<16 ; i++){
        Z[i] = 0x00;
        V[i] = Y[i];
    }
    for(int i=0 ; i<128 ; i++){
        int carry = 0;
        x_i = (X[i / 8] >> (7 - (i % 8))) & 0x01;
        if(x_i == 1){
            for(int j=0 ; j<16 ; j++){
                Z[j] ^= V[j];
            }
        }
        v_i = V[15] & 0x01;
        for(int k=0 ; k<16 ; k++){
            temp = V[k] & 0x01;
            V[k] = V[k] >> 1 ;
            if(carry == 1){
                V[k] = V[k] | (carry << 7);
            }
            carry = temp;
        }
        if(v_i == 1)
            {for(int l=0 ; l<16 ; l++){
                V[l] ^= R[l];
            }
        }
    }
    for(int m=0 ; m<16 ; m++){
        result[m] = Z[m];
    }
}
void GHASH(unsigned char *X, size_t X_len, unsigned char *H, unsigned char *result){
    size_t m = X_len / 16;
    for(int i=0 ; i<16 ; i++){
        Y[i] = 0x00;
    }
    for(int i=0 ; i<m ; i++){
        for(int j=0 ; j<16; j++){
            tempX[j] = Y[j] ^ X[i*16 + j];
        }
        GF(tempX, H, Y);
    }
    for(int i=0 ; i<16 ; i++){
        result[i] = Y[i];
    }
}
void GCTR(unsigned char ICB[16], unsigned char *X, size_t X_len, unsigned char key[4][4], unsigned char *Y){
    unsigned char CB[16];
    unsigned char AES_result[16];
    if(X_len == 0){
        return;
    }
    size_t n = (X_len + 15) / 16;
    for(int i=0 ; i<16 ; i++){
        CB[i] = ICB[i];
    }
    for(int i=0 ; i<n ; i++){
        Cipher_AES(CB, key, AES_result);
        size_t len = block_len(X_len - i*16);
        for(int j=0 ; j<len ; j++){
            Y[i*16 + j] = X[i*16 + j] ^ AES_result[j];
        }
        inc32(CB);
    }
}
void GCM_AE(unsigned char *IV, size_t IV_len, unsigned char *P, size_t P_len, unsigned char *A, size_t A_len, unsigned char key[4][4], unsigned char *C, unsigned char *T){
    Cipher_AES(empty, key, H);
    size_t s = 16 * ((IV_len + 15) / 16) - IV_len;
    size_t total_len = IV_len + s + 16;
    uint64_t IV_bitlen = (uint64_t)IV_len * 8;
    unsigned char GHASH_input[total_len];
    
    if(IV_len == 12){
        for(int i=0 ; i<12 ; i++){
            J[i] = IV[i];
        }
        for(int i=12 ; i<15 ; i++){
            J[i] = 0x00;
        }
        J[15] = 0x01;
    }
    else{
        for(int i=0 ; i<total_len ; i++){
            GHASH_input[i] = 0x00;
        }
        for(int i=0 ; i<IV_len ; i++){
            GHASH_input[i] = IV[i];
        }
        for(int i=0 ; i<8 ; i++){
            GHASH_input[total_len - 8 + i] = (IV_bitlen >> (56 - (8 * i))) & 0xFF;
        }
        GHASH(GHASH_input, total_len, H, J);
    }
    for(int i=0 ; i<16 ; i++){
        tempJ[i] = J[i];
    }
    inc32(tempJ);
    GCTR(tempJ, P, P_len, key, C);

    size_t C_len = P_len;

    size_t u = 16 * ((C_len +  15)/ 16) - C_len;
    size_t v = 16 * ((A_len +  15)/ 16) - A_len;
    size_t Glen = A_len + C_len + u + v + 16;
    size_t A_bitlen = A_len * 8;
    size_t C_bitlen = C_len * 8;
    unsigned char GHASH_input2[Glen];
    for(int i=0 ; i<Glen ; i++){
        GHASH_input2[i] = 0x00;
    }
    for(int i=0 ; i<A_len ; i++){
        GHASH_input2[i] = A[i];
    }
    for(int i=0 ; i<C_len ; i++){
        GHASH_input2[A_len + v + i] = C[i];
    }
    for(int i=0 ; i<8 ; i++){
        GHASH_input2[A_len + v + C_len + u +  i] = (A_bitlen >> (56 - (8 * i))) & 0xFF;
    }
    for(int i=0 ; i<8 ; i++){
        GHASH_input2[A_len + v + C_len + u + 8 + i] = (C_bitlen >> (56 - (8 * i))) & 0xFF;
    }
    GHASH(GHASH_input2, Glen, H, S);
    GCTR(J, S, 16, key, T);
}

int main(){
    unsigned char result[16];

    size_t X_len = sizeof(X);
    size_t P_len = sizeof(P);
    size_t A_len = sizeof(A);
    size_t C_len = P_len;
    size_t IV_len = sizeof(IV);

    GCM_AE(IV, IV_len, P, P_len, A, A_len, key, C, T);
            
    printf("======= C[i] =======");
    printf("\n");
    for(int i=0 ; i<C_len ; i++){
        printf("%02x ", C[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
    printf("\n");
    printf("======= T[i] =======");
    printf("\n");
    for(int i=0 ; i<16 ; i++){
        printf("%02x ", T[i]);
    }
}