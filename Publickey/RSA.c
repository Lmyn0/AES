#include <stdio.h>
#include <gmp.h>
#include "..\KDF\sha256-KDF.h"

mpz_t p; mpz_t q; mpz_t n; mpz_t e; mpz_t d; mpz_t x; mpz_t lambda;
mpz_t temp; mpz_t temp1; mpz_t temp2; mpz_t temp3;  mpz_t temp4; mpz_t limit;
mpz_t m; mpz_t c; mpz_t counter_mpz; mpz_t s;
unsigned char seed[32] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f
    };
unsigned char salt[32] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f
};
void RSA_key(){
    mpz_mul(n, p, q);
    mpz_sub_ui(temp1, p, 1);
    mpz_sub_ui(temp2, q, 1);
    mpz_mul(temp3, temp1, temp2);
    mpz_gcd(temp4, temp1, temp2);
    mpz_divexact(lambda, temp3, temp4);
    mpz_invert(d, e, lambda);
}
void OS2IP(unsigned char *X, size_t xlen, mpz_t x){
    mpz_set_ui(x, 0);
    for(size_t i=0 ; i<xlen ; i++){
        mpz_mul_ui(x, x, 256);
        mpz_add_ui(x, x, X[i]);
    }
}
void I2OSP(unsigned char *X, size_t xlen, mpz_t x){
    mpz_ui_pow_ui(limit, 256, xlen);
    if(mpz_cmp(x, limit) >= 0){
        printf("integer too large");
        return;
    }
    mpz_set(temp, x);
    for(size_t i=0 ; i<xlen ; i++){
        unsigned long byte = mpz_fdiv_q_ui(temp, temp, 256);
        X[xlen-1-i] = (unsigned char)byte;
    }
}
void RSAEP(mpz_t c, mpz_t m, mpz_t e, mpz_t n){
    if(mpz_sgn(m) < 0){
        printf("message representative out of range");
        return;
    }
    else if(mpz_cmp(n, m) <= 0){
        printf("message representative out of range");
        return;
    }
    mpz_powm(c, m, e, n);
}
void RSADP(mpz_t m, mpz_t c, mpz_t d, mpz_t n){
    if(mpz_sgn(c) < 0){
        printf("ciphertext representative out of range");
        return;
    }
    else if(mpz_cmp(c, n) >= 0){
        printf("ciphertext representative out of range");
        return;
    }
    mpz_powm(m, c, d, n);
}
void MGF1(unsigned char *mgfseed, size_t seedlen, unsigned char *mask, size_t masklen){
    unsigned char C[4]; unsigned char input[seedlen + 4]; unsigned char digest[32];
    size_t hlen = 32;
    size_t count = (masklen + hlen - 1) / hlen;
    mpz_ui_pow_ui(temp, 2, 32);
    mpz_mul_ui(temp, temp, hlen);
    if(mpz_cmp_ui(temp, masklen) < 0){
        printf("mask too long");
        return;
    }
    for(size_t counter=0 ; counter<count ; counter++){
        mpz_set_ui(counter_mpz, counter);
        I2OSP(C, 4, counter_mpz);
        for(size_t i=0 ; i<seedlen ; i++){
            input[i] = mgfseed[i];
        }
        for(size_t i=0 ; i<4 ; i++){
            input[i + seedlen] = C[i]; 
        }
        sha256(input, seedlen + 4, digest);
        size_t offset = counter * hlen;
        size_t remain = masklen - offset;
        size_t copylen = 0;
        if(remain >= hlen){
            copylen = hlen;
        }
        else{
            copylen = remain;
        }
        for(size_t i=0 ; i<copylen; i++){
            mask[offset + i] = digest[i];
        }
    }
}
void OAEP_Encode(unsigned char *L, size_t Llen, unsigned char *lHash, unsigned char *M, size_t k, size_t hlen, size_t mlen,unsigned char *EM){
    if(mlen > k-2*hlen-2 ){
        printf("message too long");
        return;
    }
    sha256(L, Llen, lHash);
    size_t pslen = k - mlen - 2*hlen - 2;
    unsigned char PS[pslen]; 
    for(size_t i=0 ; i<pslen ; i++){
        PS[i] = 0x00;
    }
    unsigned char DB[k - hlen - 1];
    for(size_t i=0 ; i<hlen ; i++){
        DB[i] = lHash[i];
    }
    for(size_t i=0 ; i<pslen ; i++){
        DB[hlen + i] = PS[i];
    }
    DB[hlen + pslen] = 0x01;
    for(size_t i=0 ; i<mlen ; i++){
        DB[hlen + pslen + 1 + i] = M[i];
    }
    size_t dbmasklen = k - hlen - 1;
    unsigned char dbmask[dbmasklen]; unsigned char maskedDB[dbmasklen];
    MGF1(seed, hlen, dbmask, dbmasklen);
    for(size_t i=0 ; i<dbmasklen ; i++){
        maskedDB[i] = DB[i] ^ dbmask[i];
    }
    unsigned char seedMask[hlen]; unsigned char maskedSeed[hlen];
    MGF1(maskedDB, dbmasklen, seedMask, hlen);
    for(size_t i=0 ; i<hlen ; i++){
        maskedSeed[i] = seed[i] ^ seedMask[i];
    }
    EM[0] = 0x00;
    for(size_t i=0 ; i<hlen ; i++){
        EM[1 + i] = maskedSeed[i];
    }
    for(size_t i=0 ; i<dbmasklen ; i++){
        EM[1 + hlen + i] = maskedDB[i];
    }
}
void EMSA_PSS(unsigned char *M, size_t mlen, unsigned char *EM, size_t embits, size_t hlen, size_t slen){
    unsigned char mHash[32];
    sha256(M, mlen, mHash);
    size_t emlen = (embits + 7) / 8;
    if(emlen < hlen + slen + 2){
        printf("encoding error");
        return;
    }
    unsigned char M_prime[hlen + slen + 8];
    for(size_t i=0 ; i<8 ; i++){
        M_prime[i] = 0x00;
    }
    for(size_t i=0 ; i<hlen ; i++){
        M_prime[8 + i] = mHash[i];
    }
    for(size_t i=0 ; i<slen ; i++){
        M_prime[8 + hlen + i] = salt[i];
    }
    unsigned char H[32];
    sha256(M_prime, 8 + hlen + slen, H);
    size_t pslen = emlen - slen - hlen - 2;
    unsigned char PS[pslen]; unsigned char DB[emlen - hlen - 1];
    for(size_t i=0 ; i<pslen ; i++){
        PS[i] = 0x00;
    }
    for(size_t i=0 ; i<pslen ; i++){
        DB[i] = PS[i];
    }
    DB[pslen] = 0x01;
    for(size_t i=0 ; i<slen ; i++){
        DB[pslen + 1 + i] = salt[i];
    }
    size_t dbmasklen = emlen - hlen - 1;
    unsigned char dbmask[dbmasklen]; unsigned char maskedDB[dbmasklen];
    MGF1(H, hlen, dbmask, dbmasklen);
    for(size_t i=0 ; i<dbmasklen ; i++){
        maskedDB[i] = DB[i] ^ dbmask[i];
    }
    size_t unused_bits = 8 * emlen - embits;
    maskedDB[0] &= 0xFF >> unused_bits;
    for(size_t i=0 ; i<dbmasklen ; i++){
        EM[i] = maskedDB[i];
    }
    for(size_t i=0 ; i<hlen ; i++){
        EM[dbmasklen + i] = H[i];
    }
    EM[emlen-1] = 0xbc;
}
void RSASP1(mpz_t m, mpz_t d, mpz_t n, mpz_t s){
    if(mpz_sgn(m) < 0){
        printf("message representative out of range");
        return;
    }
    else if(mpz_cmp(n, m) <= 0){
        printf("message representative out of range");
        return;
    }
    mpz_powm(s, m, d, n);
}
int EMSA_PSS_verify(unsigned char *M, size_t mlen, unsigned char *EM, size_t embits, size_t hlen, size_t emlen, size_t slen){
    unsigned char mHash[32];
    sha256(M, mlen, mHash);
    if(EM[emlen - 1] != 0xbc){
        return 0;
    }
    size_t dbmasklen = emlen - hlen - 1;
    unsigned char maskedDB[dbmasklen]; unsigned char H[32];
    for(size_t i=0 ; i<dbmasklen ; i++){
        maskedDB[i] = EM[i];
    }
    for(size_t i=0 ; i<hlen ; i++){
        H[i] = EM[dbmasklen + i];
    }
    size_t unused_bits = 8 * emlen - embits;
    if((maskedDB[0] & (0xFF << (8 - unused_bits))) != 0){
        return 0;
    }
    unsigned char dbMask[dbmasklen]; unsigned char DB[dbmasklen];
    MGF1(H, hlen, dbMask, dbmasklen);
    for(size_t i=0 ; i<dbmasklen ; i++){
        DB[i] = maskedDB[i] ^ dbMask[i];
    }
    DB[0] &= 0xFF >> unused_bits;
    size_t pslen = emlen - hlen - slen - 2;
    for(size_t i=0; i<pslen; i++){
        if(DB[i] != 0x00){
            return 0;
        }
    }
    if(DB[pslen] != 0x01){
        return 0;
    }
    unsigned char tempsalt[slen]; unsigned char m_prime[8 + hlen + slen];
    unsigned char H_prime[hlen];
    for(size_t i=0; i<slen; i++){
        tempsalt[i] = DB[pslen + 1 + i];
    }
    for(size_t i=0 ; i<8 ; i++){
        m_prime[i] = 0x00;
    }
    for(size_t i=0 ; i<hlen ; i++){
        m_prime[8 + i] = mHash[i];
    }
    for(size_t i=0 ; i<slen ; i++){
        m_prime[8 + hlen + i] = tempsalt[i];
    }
    sha256(m_prime, 8 + hlen + slen, H_prime);
    for(size_t i=0; i<hlen; i++){
        if(H[i] != H_prime[i]){
            return 0;
        }
    }
    return 1;
}
void RSAVP1(mpz_t s, mpz_t e, mpz_t n, mpz_t m){
    if(mpz_sgn(s) < 0){
        printf("message representative out of range");
        return;
    }
    else if(mpz_cmp(n, s) <= 0){
        printf("message representative out of range");
        return;
    }
    mpz_powm(m, s, e, n);
}
int main(void){
    mpz_init(p); mpz_init(q); mpz_init(n); mpz_init(e); mpz_init(d); mpz_init(x); mpz_init(s);
    mpz_init(lambda); mpz_init(limit); mpz_init(m); mpz_init(c); mpz_init(counter_mpz);
    mpz_init(temp); mpz_init(temp1); mpz_init(temp2); mpz_init(temp3); mpz_init(temp4);

    mpz_set_str(p,
    "E3845B0948142FF75095389BB17FC7B2991B7FA5DB91D8CA66EE5E1C277CA9E6"
    "0FC3389D4443B90CC139EDC9EB4A8F9E7723AD8DAAB8706D053B35089D548CB5",
    16);
    mpz_set_str(q,
    "DE1D3800B7F8671DD23A88F0CA4CB92095460A02B887FE53490778088A11469B"
    "CC37E291D376ACC0C63C573181D7F58517C08170487CE68F8894290AE4006BF3",
    16);
    mpz_set_ui(e, 65537);
    RSA_key();
    unsigned char L[] = ""; unsigned char M[] = "hello";

    size_t Llen = 0; size_t mlen = sizeof(M) - 1; size_t hlen = 32;
    size_t k = (mpz_sizeinbase(n, 2) + 7) / 8;
    unsigned char lHash[32]; unsigned char EM[k]; unsigned char C[k];
    OAEP_Encode(L, Llen, lHash, M, k, hlen, mlen, EM);
    OS2IP(EM, k, m);
    RSAEP(c, m, e, n);
    I2OSP(C, k, c);
    for(size_t i=0; i<k; i++){
        printf("%02x ", C[i]);
        if((i + 1) % 16 == 0){
            printf("\n");
        }
    }

    printf("===========================\n");
    
    size_t slen = 32; size_t modbits = mpz_sizeinbase(n, 2); size_t embits = modbits - 1; 
    size_t emlen = (embits + 7) / 8;
    unsigned char S[k];
    EMSA_PSS(M, mlen, EM, embits, hlen, slen);
    OS2IP(EM, emlen, m);
    RSASP1(m, d, n, s);
    I2OSP(S, k, s);
    for(size_t i=0; i<k; i++){
        printf("%02x ", S[i]);
        if((i + 1) % 16 == 0){
            printf("\n");
        }
    }

    printf("===========================\n");

    unsigned char EM_verify[emlen];
    OS2IP(S, k, s);
    RSAVP1(s, e, n, m);
    I2OSP(EM_verify, emlen, m);
    if(EMSA_PSS_verify(M, mlen, EM_verify, embits, hlen, emlen, slen)){
        printf("valid signature\n");
    }
    else{
        printf("invalid signature\n");
    }
}