#include <stdio.h>
#include <gmp.h>
#include "../hash/sha-512.h"

mpz_t p; mpz_t d; mpz_t x; mpz_t inv; mpz_t q; mpz_t temp; mpz_t temp1;
mpz_t g_y; mpz_t g_x; mpz_t yy; mpz_t u; mpz_t v; mpz_t v_inv; mpz_t xx;
mpz_t ex; mpz_t check; 
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
    int result;
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
    if(result == 0){
        printf("recover_x failed\n");
    }

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
int main(){
    mpz_init(p); mpz_init(d); mpz_init(q); mpz_init(g_y); mpz_init(g_x);
    mpz_init(x); mpz_init(inv); mpz_init(temp); mpz_init(temp1); mpz_init(yy);
    mpz_init(u); mpz_init(v); mpz_init(v_inv); mpz_init(xx); mpz_init(ex);
    mpz_init(check);
    Point result;
    mpz_init(result.X); mpz_init(result.Y); mpz_init(result.Z); mpz_init(result.T);
    init_ed25519();
    recover_x(g_x, g_y, 0);
    
}