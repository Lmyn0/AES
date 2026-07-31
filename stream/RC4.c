#include <stdio.h>
#include <string.h>

unsigned char S[256];
unsigned char key[] = {
    0x01, 0x02, 0x03, 0x04, 0x05
};
unsigned char keystream[16];
unsigned char plain[] = {

};
size_t L = sizeof(key);

void KSA(){
    unsigned char temp[256] = {0x00};
    int j = 0;

    for(int i=0 ; i<256 ; i++){
        S[i] = (unsigned char)i;
    }
    for(int i=0 ; i<256; i++){
        j = (j + S[i] + key[i % L]) % 256;

        temp[i] = S[i];
        S[i] = S[j];
        S[j] = temp[i];
    }

}

void PRGA(){
    int i = 0;
    int j = 0;
    int t = 0;
    
    unsigned char temp[256] = {0x00};

    for(int x=0 ; x<256 ; x++){
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;

        temp[i] = S[i];
        S[i] = S[j];
        S[j] = temp[i];

        t = (S[i] + S[j]) % 256;
        keystream[x] = S[t];
    }
}

int main(void){
    KSA();
    PRGA();

    for (int i = 0; i < 16; i++) {
        printf("%02x ", keystream[i]);
    }

    return 0;
}
