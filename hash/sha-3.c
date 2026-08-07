#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

unsigned char message[] = "The quick brown fox jumps over the lazy dog";
unsigned char padded[136];
unsigned char output[32];
uint64_t A[5][5];

size_t message_len;
size_t padded_len = sizeof(padded);

uint8_t shiftr(uint8_t x){
    return (x >> 1);
}
uint64_t rotl(uint64_t C, uint64_t s){
    if(s==0){
        return C;
    }
    else{
        return (C << s) | (C >> (64 - s));
    }    
}
// 136 바이트로 패딩
void padding(){
    for(size_t i=0 ; i<padded_len ; i++){
        padded[i] = 0x00;
    }
    for(size_t i=0 ; i<message_len ; i++){
        padded[i] = message[i];
    }
    padded[message_len] = 0x06;
    padded[padded_len - 1] |= 0x80;
}
// 136 바이트 블록을 8 바이트씩 나눔
// 각 lane 블록을 state의 rate 부분과 XOR
void absorb(){
    uint64_t lane = 0;
    for(int i=0 ; i<17 ; i++){
        lane = 0;
        for(int j=0 ; j<8 ; j++){
            lane |= ((uint64_t)padded[(i*8) + j] << (8 * j));
        }
        int x = i % 5; 
        int y = i / 5;
        A[x][y] ^= lane;
    }
}
// 핵심 함수 theta, rho, pi, chi, iota
void theta(){
    uint64_t C[5];
    uint64_t D[5];

    for(int i=0 ; i<5 ; i++){
        C[i] = A[i][0] ^ A[i][1] ^ A[i][2] ^ A[i][3] ^ A[i][4];
    }
    for(int i=0 ; i<5 ; i++){
        D[i] = C[(i+4) % 5] ^ rotl(C[(i+1) % 5], 1);
    }
    for(int i=0 ; i<5 ; i++){
        for(int j=0 ; j<5 ; j++){
            A[i][j] ^= D[i];
        }
    }
}
void rho(){
    int x = 1;
    int y = 0;
    int temp;

    for(int t=0 ; t<24 ; t++){
        int r = ((t+1)*(t+2)/2) % 64;
        A[x][y] = rotl(A[x][y], r);
        temp = x;
        x = y;
        y = (2*temp + 3*y) % 5;
    }
}
void pi(){
    uint64_t tempA[5][5];
    for(int x=0 ; x<5 ; x++){
        for(int y=0 ; y<5 ; y++){
            tempA[x][y] = A[x][y];
        }
    }
    for(int x=0 ; x<5 ; x++){
        for(int y=0 ; y<5 ; y++){
            A[x][y] = tempA[(x+3*y) % 5][x];
        }
    }
}
void chi(){
    uint64_t tempA[5][5];
    for(int x=0 ; x<5 ; x++){
        for(int y=0 ; y<5 ; y++){
            tempA[x][y] = A[x][y];
        }
    }
    for(int x=0 ; x<5 ; x++){
        for(int y=0 ; y<5 ; y++){
            A[x][y] = tempA[x][y] ^ (~(tempA[(x+1) % 5][y]) & tempA[(x+2) % 5][y]);
        }
    }
}
int rc(int t){
    if(t % 255 == 0){
        return 1;
    }
    else{
        uint8_t R = 0x80;
        for(int i=0 ; i<t%255 ; i++){
            uint8_t tempR = R & 1;
            R = shiftr(R);
            if(tempR == 1){
                R ^= 0x8E;
            }
        }
        return (R >> 7) & 1;
    }
}
void iota(int ir){
    uint64_t RC = 0;
    for(int j=0 ; j<7 ; j++){
        uint8_t bit = rc(j + 7 * ir);
        uint8_t position = (1 << j) - 1; 
        if(bit == 1){
            RC |= (1ULL << position);
        }
    }
    A[0][0] ^= RC; 
}
void keccakp(){
    for(int i=0 ; i<24 ; i++){
        theta();
        rho();
        pi();
        chi();
        iota(i);
    }
}
void squeeze(){
    for(int i=0 ; i<4 ; i++){
        for(int j=0 ; j<8 ; j++){
            output[8*i + j] = (A[i][0] >> (8*j)) & 0xFF;
        }    
    }
    for(int i=0 ; i<32 ; i++){
        printf("%02x", output[i]);
        if((i+1) % 8 == 0){
            printf("\n");
        }
    }
}
int main(){
    
    message_len = strlen((char *)message);
    
    padding();
    absorb();
    keccakp();
    squeeze();
}