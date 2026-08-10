#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "refersha2.h"

unsigned char HK[20] = {
    0x0b, 0x0b, 0x0b, 0x0b, 
    0x0b, 0x0b, 0x0b, 0x0b, 
    0x0b, 0x0b, 0x0b, 0x0b, 
    0x0b, 0x0b, 0x0b, 0x0b, 
    0x0b, 0x0b, 0x0b, 0x0b
};
unsigned char text[] = "Hi There";
unsigned char K0[64] = {0x00};

void make_K0(const unsigned char *key, size_t HK_length, unsigned char K0[64]){
    if(HK_length == 64){
        for(size_t i=0 ; i<HK_length ; i++){
            K0[i] = key[i];
        }
    }
    else if(HK_length > 64){
        sha256(key, HK_length, K0);
        for(size_t i=32 ; i<64 ; i++){
            K0[i] = 0x00;
        }
    }
    else{
        for(size_t i=0 ; i<HK_length ; i++){
            K0[i] = key[i];
        }
        for(size_t i=HK_length ; i<64 ; i++){
            K0[i] = 0x00;
        }
    }
}

void xoripad(unsigned char K0[64], unsigned char *text, size_t text_length, unsigned char *input){
    for(int i=0 ; i<64 ; i++){
        input[i] = K0[i] ^ 0x36;
    }
    for(int i=64 ; i<64 + text_length ; i++){
        input[i] = text[i - 64];
    }
}

void xoropad(unsigned char K0[64], unsigned char *output){
    for(int i=0 ; i<64 ; i++){
        output[i] = K0[i] ^ 0x5c;
    }
}

int main(void)
{   
    size_t HK_length = sizeof(HK);
    size_t text_length = strlen((char *)text);

    unsigned char input[64 + text_length];
    unsigned char output[96];
    unsigned char input_sha[32];
    unsigned char output_sha[32];

    size_t input_length = sizeof(input);
    size_t output_length = sizeof(output);
    make_K0(HK, HK_length, K0);
    xoripad(K0, text, text_length, input);
    xoropad(K0, output);
    
    //H((K0 ^ ipad) || text)
    sha256(input, input_length, input_sha);
    
    //(K0 ^ opad) || H (위에꺼)
    for(int i=64 ; i<96 ; i++){
        output[i] = input_sha[i - 64];
    }
    
    // H (위에꺼)
    sha256(output, output_length, output_sha);

    for(int i=0 ; i<32 ; i++){
        printf("%02x ", output_sha[i]);
    }
    return 0; 
}