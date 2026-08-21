#include <stdio.h>
#include <gmp.h>
#include "../KDF/sha256-KDF.h"

mpz_t p, a, b, Gx, Gy, n, h;
mpz_t d, e, k, r, s;
unsigned char M[] = "Example of ECDSA with P-256";

typedef struct{
    mpz_t x; mpz_t y; int infinity;
} ECPoint;
void init_domain_parameters(){
    mpz_init(p); mpz_init(a); mpz_init(b); mpz_init(Gx); mpz_init(Gy);
    mpz_init(n); mpz_init(h);
    mpz_set_str(p, "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF", 16);
    mpz_set_str(a, "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC", 16);
    mpz_set_str(b, "5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B", 16);
    mpz_set_str(Gx, "6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296", 16);
    mpz_set_str(Gy, "4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5", 16);
    mpz_set_str(n, "FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551", 16);
    mpz_set_ui(h, 1);
}   
void point_clear(ECPoint *P){
    mpz_clear(P->x);
    mpz_clear(P->y);
}
void point_init(ECPoint *P){
    mpz_init(P->x); mpz_init(P->y); P->infinity = 1; // P를 무한원점으로 설정
}
void point_double(ECPoint *P, ECPoint *R){
    mpz_t lambda, numerator, denominator, inverse;
    mpz_t x3, y3;
    mpz_t temp, temp1;
    if(P->infinity){
        R->infinity = 1;
        return;
    }
    if(mpz_cmp_ui(P->y, 0) == 0){
        R->infinity = 1;
        return;
    }
    mpz_init(lambda); mpz_init(numerator); mpz_init(denominator); mpz_init(inverse);
    mpz_init(x3); mpz_init(y3);
    mpz_init(temp); mpz_init(temp1);
    
    mpz_mul(temp, P->x, P->x);
    mpz_mul_ui(temp, temp, 3);

    mpz_add(numerator, temp, a); mpz_mod(numerator, numerator, p);
    mpz_mul_ui(denominator, P->y, 2); mpz_mod(denominator, denominator, p);
    if(mpz_invert(inverse, denominator, p) == 0){
        R->infinity = 1;
        mpz_clear(lambda); mpz_clear(numerator); mpz_clear(denominator); mpz_clear(inverse);
        mpz_clear(x3); mpz_clear(y3);
        mpz_clear(temp); mpz_clear(temp1);
        return;
    }
    mpz_mul(lambda, numerator, inverse); mpz_mod(lambda, lambda, p);
    
    mpz_mul(temp, lambda, lambda); mpz_mul_ui(temp1, P->x, 2);
    mpz_sub(x3, temp, temp1); mpz_mod(x3, x3, p);

    mpz_sub(temp, P->x, x3); mpz_mul(temp1, lambda, temp);
    mpz_sub(y3, temp1, P->y); mpz_mod(y3, y3, p);

    mpz_set(R->x, x3); mpz_set(R->y, y3); R->infinity = 0;

    mpz_clear(lambda); mpz_clear(numerator); mpz_clear(denominator); mpz_clear(inverse);
    mpz_clear(x3); mpz_clear(y3);
    mpz_clear(temp); mpz_clear(temp1);
}
void point_add(ECPoint *P, ECPoint *Q, ECPoint *R){
    mpz_t lambda, numerator, denominator, inverse;
    mpz_t x3, y3;
    mpz_t temp;
    if(P->infinity == 1){
        mpz_set(R->x, Q->x); mpz_set(R->y, Q->y);
        R->infinity = Q->infinity;
        return;
    }
    else if(Q->infinity == 1){
        mpz_set(R->x, P->x); mpz_set(R->y, P->y);
        R->infinity = P->infinity;
        return;
    }
    if(mpz_cmp(P->x, Q->x) == 0){
        if(mpz_cmp(P->y, Q->y) == 0){
            point_double(P, R);
            return;
        }
        else{
            R->infinity = 1;
            return;
        }
    }
    mpz_init(lambda); mpz_init(numerator); mpz_init(denominator); mpz_init(inverse);
    mpz_init(x3); mpz_init(y3);
    mpz_init(temp);

    mpz_sub(numerator, Q->y, P->y); mpz_mod(numerator, numerator, p);
    mpz_sub(denominator, Q->x, P->x); mpz_mod(denominator, denominator, p);
    if(mpz_invert(inverse, denominator, p) == 0){
        R->infinity = 1;
        mpz_clear(lambda); mpz_clear(numerator); mpz_clear(denominator); mpz_clear(inverse);
        mpz_clear(x3); mpz_clear(y3);
        mpz_clear(temp);
        return;
    }
    mpz_mul(lambda, numerator, inverse); mpz_mod(lambda, lambda, p);
    
    mpz_mul(temp, lambda, lambda);
    mpz_sub(x3, temp, P->x);
    mpz_sub(x3, x3, Q->x); mpz_mod(x3, x3, p);

    mpz_sub(temp, P->x, x3);
    mpz_mul(temp, temp, lambda);
    mpz_sub(y3, temp, P->y); mpz_mod(y3, y3, p);

    mpz_set(R->x, x3); mpz_set(R->y, y3); R->infinity = 0;

    mpz_clear(lambda); mpz_clear(numerator); mpz_clear(denominator); mpz_clear(inverse);
    mpz_clear(x3); mpz_clear(y3);
    mpz_clear(temp);
}
void point_mul(mpz_t k, ECPoint *P, ECPoint *R){
    ECPoint result; ECPoint addend; ECPoint temp;
    point_init(&result); point_init(&addend); point_init(&temp);
    result.infinity = 1;
    mpz_set(addend.x, P->x); mpz_set(addend.y, P->y);
    addend.infinity = P->infinity;
    size_t bits = mpz_sizeinbase(k, 2);
    for(size_t i=0 ; i<bits ; i++){
        if(mpz_tstbit(k, i) == 1){
            point_add(&result, &addend, &temp);
            mpz_set(result.x, temp.x); mpz_set(result.y, temp.y);
            result.infinity = temp.infinity;
        }
        point_double(&addend, &temp);
        mpz_set(addend.x, temp.x); mpz_set(addend.y, temp.y);
        addend.infinity = temp.infinity;
    }
    mpz_set(R->x, result.x); mpz_set(R->y, result.y);
    R->infinity = result.infinity;
    point_clear(&result); point_clear(&addend); point_clear(&temp);
}
void hash_message(const unsigned char *M, size_t len, mpz_t e){
    unsigned char H[32];

    sha256(M, len, H);

    mpz_import(e, 32, 1, 1, 1, 0, H);
}
void g_signature(mpz_t d, mpz_t e, mpz_t k, ECPoint *G, ECPoint *R, mpz_t r, mpz_t s){
    mpz_t kinv, temp1;
    mpz_init(kinv); mpz_init(temp1);
    point_mul(k, G, R);
    mpz_mod(r, R->x, n);
    mpz_invert(kinv, k, n);

    mpz_mul(temp1, r, d);
    mpz_add(temp1, temp1, e); mpz_mod(temp1, temp1, n);

    mpz_mul(s, kinv, temp1); mpz_mod(s, s, n);
    mpz_clear(kinv); mpz_clear(temp1);
}
int v_signature(mpz_t e, mpz_t r, mpz_t s, ECPoint *G, ECPoint *Q){
    ECPoint uG, vQ, R1;
    point_init(&uG); point_init(&vQ); point_init(&R1);
    mpz_t sinv, u, v, r1;
    mpz_init(sinv); mpz_init(u); mpz_init(v); mpz_init(r1);
    if(mpz_cmp_ui(r, 1) < 0 || mpz_cmp(r, n) >= 0){
        point_clear(&uG); point_clear(&vQ); point_clear(&R1);
        mpz_clear(sinv); mpz_clear(u); mpz_clear(v); mpz_clear(r1);
        return 0;
    }
    if(mpz_cmp_ui(s, 1) < 0 || mpz_cmp(s, n) >= 0){
        point_clear(&uG); point_clear(&vQ); point_clear(&R1);
        mpz_clear(sinv); mpz_clear(u); mpz_clear(v); mpz_clear(r1);
        return 0;
    }
    if(mpz_invert(sinv, s, n) == 0){
        point_clear(&uG); point_clear(&vQ); point_clear(&R1);
        mpz_clear(sinv); mpz_clear(u); mpz_clear(v); mpz_clear(r1);
        return 0;
    }
    mpz_mul(u, e, sinv); mpz_mod(u, u, n);
    mpz_mul(v, r, sinv); mpz_mod(v, v, n);
    point_mul(u, G, &uG); point_mul(v, Q, &vQ); point_add(&uG, &vQ, &R1);
    if(R1.infinity){
        point_clear(&uG); point_clear(&vQ); point_clear(&R1);
        mpz_clear(sinv); mpz_clear(u); mpz_clear(v); mpz_clear(r1);
        return 0;
    }
    mpz_mod(r1, R1.x, n);
    if(mpz_cmp(r, r1) == 0){
        point_clear(&uG); point_clear(&vQ); point_clear(&R1);
        mpz_clear(sinv); mpz_clear(u); mpz_clear(v); mpz_clear(r1);
        return 1;
    }
    else{
        point_clear(&uG); point_clear(&vQ); point_clear(&R1);
        mpz_clear(sinv); mpz_clear(u); mpz_clear(v); mpz_clear(r1);
        return 0;
    }
}
int main(){
    ECPoint G, Q, R;
    init_domain_parameters();
    point_init(&G); point_init(&Q); point_init(&R);
    mpz_set(G.x, Gx); mpz_set(G.y, Gy); G.infinity = 0;
    mpz_init(d); mpz_init(e); mpz_init(k); mpz_init(r); mpz_init(s);
    mpz_set_str(d, "C477F9F65C22CCE20657FAA5B2D1D8122336F851A508A1ED04E479C34985BF96", 16);
    mpz_set_str(k, "7A1A7E52797FC8CAAA435D2A4DACE39158504BF204FBE19F14DBB427FAEE50AE", 16);
    point_mul(d, &G, &Q);   // Q = dG

    hash_message(M, sizeof(M) - 1, e);
    g_signature(d, e, k, &G, &R, r, s);

    gmp_printf("d  = %Zx\n", d);

    gmp_printf("Qx = %Zx\n", Q.x);
    gmp_printf("Qy = %Zx\n", Q.y);

    gmp_printf("e  = %Zx\n", e);

    gmp_printf("k  = %Zx\n", k);

    gmp_printf("Rx = %Zx\n", R.x);
    gmp_printf("Ry = %Zx\n", R.y);

    gmp_printf("r  = %Zx\n", r);
    gmp_printf("s  = %Zx\n", s);

    if(v_signature(e, r, s, &G, &Q) == 1){
       printf("valid signature\n");
    }
    else{
        printf("invalid signature\n");
    }
    mpz_add_ui(r, r, 1);
    if(v_signature(e, r, s, &G, &Q)){
        printf("valid signature\n");
    }
    else{
        printf("invalid signature\n");
    }
    return 0;
}