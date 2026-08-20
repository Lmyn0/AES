#include <stdio.h>
#include <gmp.h>

mpz_t k; mpz_t u_int; mpz_t x2; mpz_t x3; mpz_t dummy; mpz_t mask;
mpz_t x1; mpz_t z2; mpz_t z3; mpz_t A; mpz_t AA; mpz_t B; mpz_t BB;
mpz_t E; mpz_t p; mpz_t C; mpz_t D; mpz_t DA; mpz_t CB; mpz_t temp1;
mpz_t temp2; mpz_t temp3; mpz_t result; mpz_t p_prime;

unsigned char output[32]; unsigned char KA[32]; unsigned char KB[32]; unsigned char basepoint[32];
unsigned char shared_A[32]; unsigned char shared_B[32];
unsigned char a[32] = {
    0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
    0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
    0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
    0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a
}; 
unsigned char b[32] = {
    0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
    0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
    0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
    0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb
};

void decodeScalar25519(unsigned char *scalar, mpz_t k){
    scalar[0] &= 248;
    scalar[31] &= 127;
    scalar[31] |= 64;
    // 결과 저장, 입력 원소 개수, 원소 순서, 
    // 원소 하나의 크기, 내부 바이트 순서, 버릴 비트 수, 실제 입력 배열
    mpz_import(k, 32, -1, 1, 0, 0, scalar);
}
void decodeUCoordinate(unsigned char *u, mpz_t u_int){
    u[31] &= 127;
    mpz_import(u_int, 32, -1, 1, 0, 0, u);
}
void cswap(int swap, mpz_t x2, mpz_t x3, mpz_t dummy){
    mpz_set_si(mask, -swap);
    mpz_xor(dummy, x2, x3);
    mpz_and(dummy, dummy, mask);
    mpz_xor(x2, x2, dummy);
    mpz_xor(x3, x3, dummy);
}
void X25519(mpz_t k, mpz_t u_int, mpz_t result){
    mpz_set(x1, u_int); mpz_set_ui(x2, 1); mpz_set_ui(z2, 0); mpz_set(x3, u_int);
    mpz_set_ui(z3, 1); int swap = 0; int k_t = 0; unsigned long a24 = 121665;
    for(int t=254 ; t>=0 ; t--){
        k_t = mpz_tstbit(k, t);
        swap ^= k_t;

        cswap(swap, x2, x3, dummy);
        cswap(swap, z2, z3, dummy);
        swap = k_t;

        mpz_add(A, x2, z2);
        mpz_mod(A, A, p);
        mpz_mul(AA, A, A);
        mpz_mod(AA, AA, p);
        mpz_sub(B, x2, z2);
        mpz_mod(B, B, p);
        mpz_mul(BB, B, B);
        mpz_mod(BB, BB, p);
        mpz_sub(E, AA, BB);
        mpz_mod(E, E, p);
        mpz_add(C, x3, z3);
        mpz_mod(C, C, p);
        mpz_sub(D, x3, z3);
        mpz_mod(D, D, p);
        mpz_mul(DA, D, A);
        mpz_mod(DA, DA, p);
        mpz_mul(CB, C, B);
        mpz_mod(CB, CB, p);
        mpz_add(temp1 ,DA, CB);
        mpz_mul(x3, temp1, temp1);
        mpz_mod(x3, x3, p);
        mpz_sub(temp2, DA, CB);
        mpz_mul(temp2, temp2, temp2);
        mpz_mul(z3, x1, temp2);
        mpz_mod(z3, z3, p);
        mpz_mul(x2, AA, BB);
        mpz_mod(x2, x2, p);
        mpz_mul_ui(temp3, E, a24);
        mpz_add(temp3, AA, temp3);
        mpz_mul(z2, temp3, E);
        mpz_mod(z2, z2, p);
    }
    cswap(swap, x2, x3, dummy);
    cswap(swap, z2, z3, dummy);
    mpz_sub_ui(p_prime, p, 2);
    mpz_powm(result, z2, p_prime, p);
    mpz_mul(result, result, x2);
    mpz_mod(result, result, p);
}
void encodeUCoordinate(mpz_t result, unsigned char *output){
    for(int i=0 ; i<32 ; i++){
        output[i] = 0x00;
    }
    size_t count = 0;
    mpz_mod(result, result, p);
    mpz_export(output, &count, -1, 1, 0, 0, result);

}
int main(){
    mpz_init(k); mpz_init(u_int); mpz_init(x2); mpz_init(x3); mpz_init(mask);
    mpz_init(dummy); mpz_init(x1); mpz_init(z2); mpz_init(z3); mpz_init(A);
    mpz_init(AA); mpz_init(B); mpz_init(BB); mpz_init(E); mpz_init(p);
    mpz_init(C); mpz_init(D); mpz_init(DA); mpz_init(CB); mpz_init(temp1);
    mpz_init(temp2); mpz_init(temp3); mpz_init(result); mpz_init(p_prime);

    mpz_set_ui(p, 1); 
    mpz_mul_2exp(p, p, 255);
    mpz_sub_ui(p, p, 19);
    basepoint[0] = 0x09;
    // Alice
    printf("Alice's key\n");
    decodeScalar25519(a, k);
    decodeUCoordinate(basepoint, u_int);
    X25519(k, u_int, result);
    encodeUCoordinate(result, KA);
    for(int i=0 ; i<32 ; i++){
        printf("%02x ", KA[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
    // Bob
    printf("Bob's key\n");
    decodeScalar25519(b, k);
    decodeUCoordinate(basepoint, u_int);
    X25519(k, u_int, result);
    encodeUCoordinate(result, KB);
    for(int i=0 ; i<32 ; i++){
        printf("%02x ", KB[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
    // Alice 개인키, Bob 공개키
    printf("Shared_A\n");
    decodeScalar25519(a, k);
    decodeUCoordinate(KB, u_int);
    X25519(k, u_int, result);
    encodeUCoordinate(result, shared_A);
    for(int i=0 ; i<32 ; i++){
        printf("%02x ", shared_A[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
    // Bob 개인키, Alice 공개키
    printf("Shared_B\n");
    decodeScalar25519(b, k);
    decodeUCoordinate(KA, u_int);
    X25519(k, u_int, result);
    encodeUCoordinate(result, shared_B);
    for(int i=0 ; i<32 ; i++){
        printf("%02x ", shared_B[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
}