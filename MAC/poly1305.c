#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

unsigned char key[32] = {
    0x85, 0xd6, 0xbe, 0x78, 0x57, 0x55, 0x6d, 0x33,
    0x7f, 0x44, 0x52, 0xfe, 0x42, 0xd5, 0x06, 0xa8,
    0x01, 0x03, 0x80, 0x8a, 0xfb, 0x0d, 0xb2, 0xfd,
    0x4a, 0xbf, 0xf6, 0xaf, 0x41, 0x49, 0xf5, 0x1b
};
unsigned char message[34] = {
    0x43, 0x72, 0x79, 0x70, 0x74, 0x6f, 0x67, 0x72,
    0x61, 0x70, 0x68, 0x69, 0x63, 0x20, 0x46, 0x6f,
    0x72, 0x75, 0x6d, 0x20, 0x52, 0x65, 0x73, 0x65,
    0x61, 0x72, 0x63, 0x68, 0x20, 0x47, 0x72, 0x6f,
    0x75, 0x70
};
unsigned char tag[16];
unsigned char r[16];
unsigned char s[16];
unsigned char block1[17];
unsigned char block2[17];
unsigned char block3[3];
uint32_t r0 = 0;
uint32_t r1 = 0;
uint32_t r2 = 0;
uint32_t r3 = 0;
uint32_t r4 = 0;
uint32_t a0 = 0;
uint32_t a1 = 0;    
uint32_t a2 = 0;
uint32_t a3 = 0;
uint32_t a4 = 0;
uint64_t d0 = 0;
uint64_t d1 = 0;    
uint64_t d2 = 0;
uint64_t d3 = 0;
uint64_t d4 = 0;
uint32_t p0 = 0x3fffffb;
uint32_t p1 = 0x3ffffff;
uint32_t p2 = 0x3ffffff;
uint32_t p3 = 0x3ffffff;
uint32_t p4 = 0x3ffffff;
uint64_t ans = 0;
uint32_t f0 = 0;
uint32_t f1 = 0;
uint32_t f2 = 0;
uint32_t f3 = 0;
int borrow = 0;

void devidekey(){
    for(int i=0 ; i<16 ; i++){
        r[i] = key[i];
    }
    for(int i=16 ; i<32 ; i++){
        s[i - 16] = key[i];
    }
}
void clamp(){
    for(int i=0 ; i<4 ; i++){
        r[i*4 + 3] &= 0x0f;
    }
    for(int i=1 ; i<4 ; i++){
        r[i*4] &= 0xfc;
    }
}
void rlimb(){
    // r0 -> 26 bits
    for(int i=0 ; i<3 ; i++){
        r0 |= (r[i] << (8 * i)); 
    }
    r0 |= ((r[3] & 0x03) << 24);
    // r1 -> 26 bits
    r1 |= ((r[3] & 0xfc) >> 2);
    for(int i=4 ; i<6 ; i++){
        r1 |= ((r[i] << (6 + (8 * (i-4)))));
    }
    r1 |= ((r[6] & 0x0f) << 22);
    // r2 -> 26 bits
    r2 |= ((r[6] & 0xf0) >> 4);
    for(int i=7 ; i<9 ; i++){
        r2 |= ((r[i] << (4 + (8 * (i-7)))));
    }
    r2 |= ((r[9] & 0x3f) << 20);
    // r3 -> 26 bits
    r3 |= ((r[9] & 0xc0) >> 6);
    for(int i=10 ; i<13 ; i++){
        r3 |= ((r[i] << (2 + (8 * (i-10)))));
    }
    // r4 -> 26 bits
    for(int i=13 ; i<16 ; i++){
        r4 |= ((r[i] << (8 * (i-13))));
    }
}
void makeblock(){
    for(int i=0 ; i<16; i++){
        block1[i] = message[i];
    }
    block1[16] = 0x01;
    
    for(int i=0 ; i<16 ; i++){
        block2[i] = message[i + 16];
    }
    block2[16] = 0x01;
    
    for(int i=0 ; i<2 ; i++){
        block3[i] = message[i + 32];
    }
    block3[2] = 0x01;
}
// message[0~16] -> 26 bits
void mlimb(unsigned char *block){
    uint32_t n0 = 0;
    for(int i=0 ; i<3 ; i++){
        n0 |= (block[i] << (8 * i)); 
    }
    n0 |= (((block[3] & 0x03)) << 24);
    uint32_t n1 = 0;
    n1 |= ((block[3] & 0xfc) >> 2);
    for(int i=4 ; i<6 ; i++){
        n1 |= ((block[i] << (6 + (8 * (i-4)))));
    }
    n1 |= ((block[6] & 0x0f) << 22);
    uint32_t n2 = 0;
    n2 |= ((block[6] & 0xf0) >> 4);
    for(int i=7 ; i<9 ; i++){
        n2 |= ((block[i] << (4 + (8 * (i-7)))));
    }
    n2 |= ((block[9] & 0x3f) << 20);
    uint32_t n3 = 0;
    n3 |= ((block[9] & 0xc0) >> 6);
    for(int i=10 ; i<13 ; i++){
        n3 |= ((block[i] << (2 + (8 * (i-10)))));
    }
    uint32_t n4 = 0;
    for(int i=13 ; i<16; i++){
        n4 |= ((block[i] << (8 * (i-13))));
    }
    n4 |= ((block[16] & 0x03) << 24);

    a0 += n0;
    a1 += n1;
    a2 += n2;
    a3 += n3;
    a4 += n4;
}
void small_mlimb(unsigned char *block){
    uint32_t n0 = 0;
    for(int i=0 ; i<3 ; i++){
        n0 |= (block[i] << (8 * i)); 
    }

    a0 += n0;
}
void multiply(){
    d0 = ((uint64_t)a0*r0) + (5*(uint64_t)a1*r4) + (5*(uint64_t)a2*r3) + (5*(uint64_t)a3*r2) + (5*(uint64_t)a4*r1);
    d1 = ((uint64_t)a0*r1) + ((uint64_t)a1*r0) + (5*(uint64_t)a2*r4) + (5*(uint64_t)a3*r3) + (5*(uint64_t)a4*r2);
    d2 = ((uint64_t)a0*r2) + ((uint64_t)a1*r1) + ((uint64_t)a2*r0) + (5*(uint64_t)a3*r4) + (5*(uint64_t)a4*r3);
    d3 = ((uint64_t)a0*r3) + ((uint64_t)a1*r2) + ((uint64_t)a2*r1) + ((uint64_t)a3*r0) + (5*(uint64_t)a4*r4);
    d4 = ((uint64_t)a0*r4) + ((uint64_t)a1*r3) + ((uint64_t)a2*r2) + ((uint64_t)a3*r1) + ((uint64_t)a4*r0);
}
void carry(){
    uint64_t c;
    c = d0 >> 26;
    d0 &= 0x3ffffff;
    d1 += c;

    c = d1 >> 26;
    d1 &= 0x3ffffff;
    d2 += c;

    c = d2 >> 26;
    d2 &= 0x3ffffff;
    d3 += c;
    
    c = d3 >> 26;
    d3 &= 0x3ffffff;
    d4 += c;

    c = d4 >> 26;
    d4 &= 0x3ffffff;
    d0 += c * 5;

    c = d0 >> 26;
    d0 &= 0x3ffffff;
    d1 += c;

    a0 = d0;
    a1 = d1;
    a2 = d2;
    a3 = d3;
    a4 = d4;
}
void reduce(){
    int greater_or_equal = 0;
    if(a4 > p4){
        greater_or_equal = 1;
    }
    else if(a4 == p4){
        if(a3 > p3){
            greater_or_equal = 1;
        }
        else if(a3 == p3){
            if(a2 > p2){
                greater_or_equal = 1;
            }
            else if(a2 == p2){
                if(a1 > p1){
                    greater_or_equal = 1;
                }
                else if(a1 == p1){
                    if(a0 >= p0){
                        greater_or_equal = 1;
                    }
                }
            }
        }
    }
    if(greater_or_equal == 1){
        if(a0 >= p0){
            a0 -= p0;
            borrow = 0;
        }
        else{
            a0 = a0 + 0x4000000 - p0;
            borrow = 1;
        }
        if(a1 >= p1 + borrow){
                a1 = a1 - p1 - borrow;
                borrow = 0;
            }
        else{
            a1 = a1 + 0x4000000 - p1 - borrow;
            borrow = 1;
        }
        if(a2 >= p2 + borrow){
                a2 = a2 - p2 - borrow;
                borrow = 0;
            }
        else{
            a2 = a2 + 0x4000000 - p2 - borrow;
            borrow = 1;
        }
        if(a3 >= p3 + borrow){
                a3 = a3 - p3 - borrow;
                borrow = 0;
            }
        else{
            a3 = a3 + 0x4000000 - p3 - borrow;
            borrow = 1;
        }
        if(a4 >= p4 + borrow){
                a4 = a4 - p4 - borrow;
                borrow = 0;
            }
        else{
            a4 = a4 + 0x4000000 - p4 - borrow;
            borrow = 1;
        }
    }   
}
void makef(){
    f0 = a0 | ((a1 & 0x3f) << 26);
    f1 = ((a1 >> 6) | ((a2 & 0xfff) << 20));
    f2 = ((a2 >> 12) | ((a3 & 0x3ffff) << 14));
    f3 = ((a3 >> 18) | ((a4 & 0xffffff) << 8));
}
void final_add(){
    uint32_t s0 = 0;
    for(int i=0 ; i<4 ; i++){
        s0 |= (uint32_t)s[i] << (8 * i);
    }
    uint32_t s1 = 0;
    for(int i=4 ; i<8 ; i++){
        s1 |= (uint32_t)s[i] << (8 * (i-4));
    }
    uint32_t s2 = 0;
    for(int i=8 ; i<12 ; i++){
        s2 |= (uint32_t)s[i] << (8 * (i-8));
    }
    uint32_t s3 = 0;
    for(int i=12 ; i<16 ; i++){
        s3 |= (uint32_t)s[i] << (8 * (i-12));
    }
    uint64_t temp = 0;
    uint32_t overflow = 0;
    temp = (uint64_t)f0 + s0;
    f0 = temp & 0xffffffff;
    overflow = temp >> 32;

    temp = (uint64_t)f1 + s1 + overflow;
    f1 = temp & 0xffffffff;
    overflow = temp >> 32;

    temp = (uint64_t)f2 + s2+ overflow;
    f2 = temp & 0xffffffff;
    overflow = temp >> 32;

    temp = (uint64_t)f3 + s3 + overflow;
    f3 = temp & 0xffffffff;
}
void f_to_tag(){
    for(int i=0 ; i<4 ; i++){
        tag[i] = f0 >> (8 * i);
    }
    for(int i=4 ; i<8 ; i++){
        tag[i] = f1 >> (8 * (i-4));
    }
    for(int i=8 ; i<12 ; i++){
        tag[i] = f2 >> (8 * (i-8));
    }
    for(int i=12 ; i<16 ; i++){
        tag[i] = f3 >> (8 * (i-12));
    }

    for(int i=0 ; i<16 ; i++){
        printf("%02x ", tag[i]);
    }
}
int main(){

    devidekey();
    clamp();
    rlimb();
    makeblock();

    mlimb(block1);
    multiply();
    carry();
    
    mlimb(block2);
    multiply();
    carry();
    
    small_mlimb(block3);
    multiply();
    carry();

    reduce();
    makef();
    final_add();
    f_to_tag();
}