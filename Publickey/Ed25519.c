#include <stdio.h>
#include <gmp.h>
#include "../hash/sha-512.h"

mpz_t p; mpz_t d; mpz_t x; mpz_t inv; mpz_t q; mpz_t temp; mpz_t temp1;
mpz_t g_y; mpz_t g_x; mpz_t yy; mpz_t u; mpz_t v; mpz_t v_inv; mpz_t xx;
mpz_t ex; mpz_t check; mpz_t s; mpz_t r; mpz_t k; mpz_t S; mpz_t S_verify; 
mpz_t k_verify;
unsigned char seed[32] = {
    0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60,
    0xba, 0x84, 0x4a, 0xf4, 0x92, 0xec, 0x2c, 0xc4,
    0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32, 0x69, 0x19,
    0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60
};
unsigned char h[64];
unsigned char pubkey[32];
unsigned char R_enc[32]; unsigned char S_enc[32];
unsigned char signature[64];
typedef struct{
    mpz_t X; mpz_t Y; mpz_t Z; mpz_t T;
} Point;
void modp_inv(mpz_t out, const mpz_t x){
    mpz_t p_prime; mpz_init(p_prime);
    mpz_sub_ui(p_prime, p, 2);
    mpz_powm(out, x, p_prime, p);
    mpz_clear(p_prime);
}
void init_ed25519(){
    // p = 2^255 - 19
    mpz_set_ui(p, 1);
    mpz_mul_2exp(p, p, 255);
    mpz_sub_ui(p, p, 19);
    // q = 2^252 + ...
    mpz_set_ui(q, 1);
    mpz_mul_2exp(q, q, 252);

    mpz_set_str(temp, "27742317777372353535851937790883648493", 10);
    mpz_add(q, q, temp);
    // d = -121665 * 121666^(-1) mod p
    mpz_set_ui(x, 121666);
    modp_inv(inv, x);
    mpz_mul_ui(d, inv, 121665);
    mpz_neg(d, d);
    mpz_mod(d, d, p);
    // g_y = 4 * 5^(-1) mod p
    mpz_set_ui(x, 5);
    modp_inv(temp1, x);
    mpz_mul_ui(g_y, temp1, 4);
    mpz_mod(g_y, g_y, p);
}
int recover_x(mpz_t out, const mpz_t y, int sign){
    if(mpz_cmp_ui(y, 0) < 0){
        return 0;
    }
    else if(mpz_cmp(p, y) <= 0){
        return 0;
    }
    mpz_mul(yy, y, y); mpz_mod(yy, yy, p);
    mpz_sub_ui(u, yy, 1); mpz_mod(u, u, p);
    mpz_mul(v, d, yy); mpz_mod(v, v, p);
    mpz_add_ui(v, v, 1); mpz_mod(v, v, p);
    modp_inv(v_inv, v);
    mpz_mul(xx, u, v_inv); mpz_mod(xx, xx, p);
    mpz_add_ui(ex, p, 3);
    mpz_fdiv_q_ui(ex, ex, 8);
    mpz_powm(x, xx, ex, p);
    mpz_mul(check, x, x); mpz_mod(check, check, p);
    if(mpz_cmp(check, xx) != 0){
        mpz_sub_ui(ex, p, 1);
        mpz_fdiv_q_ui(ex, ex, 4);
        mpz_set_ui(temp1, 2);
        mpz_powm(temp, temp1, ex, p);
        mpz_mul(x, x, temp); mpz_mod(x, x, p);
        mpz_mul(check, x, x); mpz_mod(check, check, p);
        if(mpz_cmp(check, xx) != 0){
            return 0;
        }
    }
    if(mpz_cmp_ui(x, 0) == 0 && sign == 1){
        return 0;
    }
    if(mpz_tstbit(x, 0) != sign){
        mpz_sub(x, p, x);
    }
    mpz_set(out, x);
    return 1;
}
void point_add(Point *out, const Point *P, const Point *Q){
    mpz_t A, B, C, D, E, F, G, H;
    mpz_init(A); mpz_init(B); mpz_init(C); mpz_init(D);
    mpz_init(E); mpz_init(F); mpz_init(G); mpz_init(H);
    mpz_sub(temp, P->Y, P->X);
    mpz_sub(temp1, Q->Y, Q->X);
    mpz_mul(A, temp, temp1); mpz_mod(A, A, p);
    mpz_add(temp, P->Y, P->X);
    mpz_add(temp1, Q->Y, Q->X);
    mpz_mul(B, temp, temp1); mpz_mod(B, B, p);
    mpz_mul(temp, P->T, Q->T);
    mpz_mul(temp, temp, d);
    mpz_mul_ui(C, temp, 2); mpz_mod(C, C, p);
    mpz_mul(temp, P->Z, Q->Z);
    mpz_mul_ui(D, temp, 2); mpz_mod(D, D, p);
    mpz_sub(E, B, A); mpz_mod(E, E, p); 
    mpz_sub(F, D, C); mpz_mod(F, F, p); 
    mpz_add(G, D, C); mpz_mod(G, G, p); 
    mpz_add(H, B, A); mpz_mod(H, H, p); 
    mpz_mul(temp, E, F); mpz_mod(out->X, temp, p);
    mpz_mul(temp, G, H); mpz_mod(out->Y, temp, p);
    mpz_mul(temp, E, H); mpz_mod(out->T, temp, p);
    mpz_mul(temp, G, F); mpz_mod(out->Z, temp, p);
    mpz_clear(A); mpz_clear(B); mpz_clear(C); mpz_clear(D);
    mpz_clear(E); mpz_clear(F); mpz_clear(G); mpz_clear(H);
}
void point_mul(Point *out, const mpz_t s, const Point *P){
    Point Q;
    mpz_init(Q.X); mpz_init(Q.Y); mpz_init(Q.Z); mpz_init(Q.T);
    mpz_set_ui(Q.X, 0); mpz_set_ui(Q.Y, 1); mpz_set_ui(Q.Z, 1); mpz_set_ui(Q.T, 0);
    Point c;
    mpz_init(c.X); mpz_init(c.Y); mpz_init(c.Z); mpz_init(c.T);
    mpz_set(c.X, P->X); mpz_set(c.Y, P->Y); mpz_set(c.Z, P->Z); mpz_set(c.T, P->T);
    mpz_t scalar; mpz_init(scalar);
    mpz_set(scalar, s);
    while(mpz_cmp_ui(scalar, 0) > 0){
        if(mpz_tstbit(scalar, 0) == 1){
            point_add(&Q, &Q, &c);
        }
        point_add(&c, &c, &c);
        mpz_fdiv_q_2exp(scalar, scalar, 1);
    }
    mpz_set(out->X, Q.X); mpz_set(out->Y, Q.Y); mpz_set(out->Z, Q.Z); mpz_set(out->T, Q.T);
    
    mpz_clear(Q.X); mpz_clear(Q.Y); mpz_clear(Q.Z); mpz_clear(Q.T);
    mpz_clear(c.X); mpz_clear(c.Y); mpz_clear(c.Z); mpz_clear(c.T);
    mpz_clear(scalar);
}
void sha_512(mpz_t s, const unsigned char seed[32]){
    sha512(seed, 32, h);
    h[0] &= 248; h[31] &= 63; h[31] |= 64;
    mpz_import(s, 32, -1, 1, 0, 0, h);
}
void point_compress(unsigned char out[32], const Point *P){
    mpz_t z_inv, ax, ay; size_t count;
    mpz_init(z_inv); mpz_init(ax); mpz_init(ay);
    modp_inv(z_inv, P->Z);
    mpz_mul(ax, P->X, z_inv); mpz_mod(ax, ax, p);
    mpz_mul(ay, P->Y, z_inv); mpz_mod(ay, ay, p);
    for(int i=0 ; i<32 ; i++){
        out[i] = 0x00;
    }
    mpz_export(out, &count, -1, 1, 0, 0, ay);
    if(mpz_tstbit(ax, 0) == 1){
        out[31] |= 0x80;
    }
    mpz_clear(z_inv); mpz_clear(ax); mpz_clear(ay);
}
void make_r(mpz_t r, const unsigned char h[64], const unsigned char *message, size_t mlen){
    unsigned char input[32 + mlen]; unsigned char r_hash[64];
    for(int i=0 ; i<32 ; i++){
        input[i] = h[i + 32];
    }
    for(int i=0 ; i<mlen ; i++){
        input[i + 32] = message[i];
    }
    sha512(input, 32+mlen, r_hash);
    mpz_import(r, 64, -1, 1, 0, 0, r_hash); mpz_mod(r, r, q);
}
void make_k(mpz_t k, const unsigned char R_enc[32], const unsigned char pubkey[32], const unsigned char *message, size_t mlen){
    unsigned char t_message[64 + mlen]; unsigned char k_hash[64];
    for(int i=0 ; i<32 ; i++){
        t_message[i] = R_enc[i];
    }
    for(int i=0 ; i<32 ; i++){
        t_message[i + 32] = pubkey[i];
    }
    for(int i=0 ; i<mlen ; i++){
        t_message[64 + i] = message[i];
    }
    sha512(t_message, 64 + mlen, k_hash);
    mpz_import(k, 64, -1, 1, 0, 0, k_hash); mpz_mod(k, k, q);
}
void make_S(mpz_t S, const mpz_t r, const mpz_t k, const mpz_t s){
    size_t count;
    mpz_mul(temp, k, s);
    mpz_add(temp, temp, r);
    mpz_mod(S, temp, q);
    mpz_export(S_enc, &count, -1, 1, 0, 0, S);
}
int point_decompress(Point *P, const unsigned char in[32]){
    unsigned char y_bytes[32];
    int sign; mpz_t x, y;
    mpz_init(x); mpz_init(y);
    for(int i=0 ; i<32 ; i++){
        y_bytes[i] = in[i];
    }
    sign = (y_bytes[31] >> 7) & 1;
    y_bytes[31] &= 0x7f;
    mpz_import(y, 32, -1, 1, 0, 0, y_bytes);
    if(mpz_cmp(y, p) >= 0){
        mpz_clear(y); mpz_clear(x);
        return 0;
    }
    if(recover_x(x, y, sign) == 0){
        mpz_clear(y); mpz_clear(x);
        return 0;
    }
    mpz_set(P->X, x); mpz_set(P->Y, y); mpz_set_ui(P->Z, 1);

    mpz_mul(P->T, x, y); mpz_mod(P->T, P->T, p);
    mpz_clear(x); mpz_clear(y);
    return 1;
}
int point_equal(const Point *P, const Point *Q){
    mpz_t x1z2, x2z1, y1z2, y2z1;
    mpz_init(x1z2); mpz_init(x2z1); mpz_init(y1z2); mpz_init(y2z1);

    mpz_mul(x1z2, P->X, Q->Z); mpz_mod(x1z2, x1z2, p);
    mpz_mul(x2z1, Q->X, P->Z); mpz_mod(x2z1, x2z1, p);
    mpz_mul(y1z2, P->Y, Q->Z); mpz_mod(y1z2, y1z2, p);
    mpz_mul(y2z1, Q->Y, P->Z); mpz_mod(y2z1, y2z1, p);
    int result = (mpz_cmp(x1z2, x2z1) == 0) && (mpz_cmp(y1z2, y2z1) == 0);

    mpz_clear(x1z2); mpz_clear(x2z1); mpz_clear(y1z2); mpz_clear(y2z1);
    return result;
}
int main(){
    unsigned char message[] = ""; size_t mlen = 0;
    mpz_init(p); mpz_init(d); mpz_init(q); mpz_init(g_y); mpz_init(g_x);
    mpz_init(x); mpz_init(inv); mpz_init(temp); mpz_init(temp1); mpz_init(yy);
    mpz_init(u); mpz_init(v); mpz_init(v_inv); mpz_init(xx); mpz_init(ex);
    mpz_init(check); mpz_init(s); mpz_init(r); mpz_init(k); mpz_init(S);
    mpz_init(S_verify); mpz_init(k_verify);
    Point result;
    mpz_init(result.X); mpz_init(result.Y); mpz_init(result.Z); mpz_init(result.T);
    init_ed25519();
    recover_x(g_x, g_y, 0);
    Point G; Point R; Point A; Point R_verify;
    mpz_init(G.X); mpz_init(G.Y); mpz_init(G.Z); mpz_init(G.T);
    mpz_init(R.X); mpz_init(R.Y); mpz_init(R.Z); mpz_init(R.T);
    mpz_init(A.X); mpz_init(A.Y); mpz_init(A.Z); mpz_init(A.T);
    mpz_init(R_verify.X); mpz_init(R_verify.Y); mpz_init(R_verify.Z); mpz_init(R_verify.T);
    mpz_set(G.X, g_x); mpz_set(G.Y, g_y); mpz_set_ui(G.Z, 1);
    mpz_mul(G.T, G.X, G.Y); mpz_mod(G.T, G.T, p); 
    sha_512(s, seed);
    point_mul(&result, s, &G);
    point_compress(pubkey, &result);
    
    make_r(r, h, message, mlen);
    point_mul(&R, r, &G);
    point_compress(R_enc, &R);
    
    make_k(k, R_enc, pubkey, message, mlen);
    make_S(S, r, k, s);
    for(int i=0 ; i<32 ; i++){
        signature[i] = R_enc[i];
    }
    for(int i=0 ; i<32 ; i++){
        signature[i + 32] = S_enc[i];
    }
    if(point_decompress(&A, pubkey) == 0){
        printf("invalid public key\n");
        return 0;
    }
    if(point_decompress(&R_verify, R_enc) == 0){
        printf("invalid R\n");
        return 0;
    }
    mpz_import(S_verify, 32, -1, 1, 0, 0, S_enc);
    if(mpz_cmp(S_verify, q) >= 0){
        printf("invalid signature\n");
        return 0;
    }
    make_k(k_verify, R_enc, pubkey, message, mlen);
    Point Left; Point kA; Point Right;
    mpz_init(Left.X);  mpz_init(Left.Y);  mpz_init(Left.Z);  mpz_init(Left.T);
    mpz_init(kA.X);    mpz_init(kA.Y);    mpz_init(kA.Z);    mpz_init(kA.T);
    mpz_init(Right.X); mpz_init(Right.Y); mpz_init(Right.Z); mpz_init(Right.T);
    point_mul(&Left, S_verify, &G);
    point_mul(&kA, k_verify, &A);
    point_add(&Right, &R_verify, &kA);
    
    printf("Public Key\n");
    for(int i = 0; i < 32; i++){
        printf("%02x ", pubkey[i]);
        if((i + 1) % 8 == 0) printf("\n");
    }

    printf("\nSignature\n");
    for(int i = 0; i < 64; i++){
        printf("%02x ", signature[i]);
        if((i + 1) % 8 == 0) printf("\n");
    }

    printf("\nVerification\n");
    if(point_equal(&Left, &Right)){
        printf("valid signature\n");
    }
    else{
        printf("invalid signature\n");
    }
}