#include <stdio.h>

unsigned char sbox[16][16] = {
    {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
    {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
    {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
    {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
    {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
    {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
    {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
    {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
    {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
    {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
    {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
    {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
    {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
    {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
    {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
    {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}
};
/*1. 상위 4비트와 하위 4비트를 구분해서 
  2. 상위 4비트는 행의 인덱스, 하위 4비트는 열의 인덱스로 사용하여서 값을 치환*/
void sub_bytes(unsigned char state[4][4]){
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            state[i][j] = sbox[(state[i][j] >> 4) & 0x0f][state[i][j] & 0x0f];
        }
    }
}
//왼쪽 순환 함수 (1열은 그대로, 2열은 1칸/3열은 2칸/4열은 3칸 왼쪽으로 이동)
void shift_rows(unsigned char state[4][4]){
    unsigned char temp;
    for(int i=1; i<4; i++){
        for(int j=0; j<i; j++){
            temp = state[i][0]; // 맨 앞의 값을 저장
            for(int k=0; k<3; k++){
                state[i][k] = state[i][k+1];
            } // 나머지 값을 왼쪽으로 한 칸씩 이동
            state[i][3] = temp; // 마지막 값을 맨 앞의 값으로 채움
        }
    }
}
/*1. 바이트를 왼쪽으로 1비트 이동(2를 곱함)
  2. 기존 값의 최상위 비트가 1이었다면 0x1b와, 1이 아니었다면 0x00과 XOR*/
unsigned char xtime(unsigned char x){
    return (unsigned char)((x << 1) ^ ((x & 0x80) ? 0x1b : 0x00));
    }
/*1. [ 01 : 그대로 / 02 : xtime / 03 : 02 ^ 01 ] 정해진 규칙
  2. 열을 기준으로 해서 정해진 규칙과 곱함 */
void MixColumns(unsigned char state[4][4])
{
    unsigned char temp[4][4];
    
    for (int i = 0; i < 4; i++) {
        temp[0][i] =
            xtime(state[0][i]) ^
            (xtime(state[1][i]) ^ state[1][i]) ^
            state[2][i] ^
            state[3][i];
        temp[1][i] =
            state[0][i] ^
            xtime(state[1][i]) ^
            (xtime(state[2][i]) ^ state[2][i]) ^
            state[3][i];
        temp[2][i] =
            state[0][i] ^
            state[1][i] ^
            xtime(state[2][i]) ^
            (xtime(state[3][i]) ^ state[3][i]);
        temp[3][i] =
            (xtime(state[0][i]) ^ state[0][i]) ^
            state[1][i] ^
            state[2][i] ^
            xtime(state[3][i]);
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = temp[i][j];
        }
    }
}
// 미리 생성된 키확장으로 생성된 해당 라운드의Roundkey를 입력받아 state와 XOR해서 state를 갱신
void AddRoundKey(unsigned char state[4][4], unsigned char roundKey[4][4]){
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            state[i][j] ^= roundKey[i][j];
        }
    }    
}
// 4바이트 워드를 왼쪽으로 1바이트 순환 이동
void RotWord(unsigned char word[4]){
    unsigned char temp = word[0];
    
    for(int i=0; i<3; i++){
        word [i] = word [i+1];
    }
    word [3] = temp;
}
/* 1. 상위 4비트와 하위 4비트로 구분
   2. sbox의 값과 치환*/
void SubWord(unsigned char word[4]){
    for(int i=0; i<4; i++){
        unsigned char row = (word[i] >> 4) & 0x0f;
        unsigned char column = (word[i]) & 0x0f;

        word[i] = sbox[row][column];
    }
}
// 키 확장에서 사용하는 라운드 상수
static const unsigned char rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10,
    0x20, 0x40, 0x80, 0x1b, 0x36
};
/* 원래 16바이트 키를 받아서 176바이트로 키를 확장
   1. 0~15 까지는 기존 키 사용
   2. 이후 16개씩 끊어서 각 라운드마다 키 확장 */
void keyExpansion(const unsigned char key[4][4], unsigned char expandedKey[176] ){
    unsigned char temp[4];
    int bcount = 16;
    int rconIndex = 0;

    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            expandedKey[i*4 + j] =
                key[j][i];
        }
    }

    while (bcount < 176) {
        for (int i = 0; i < 4; i++) {
            temp[i] =
                expandedKey[bcount - 4 + i];
        }
        if (bcount % 16 == 0) {
            RotWord(temp);
            SubWord(temp);
            temp[0] ^= rcon[rconIndex];
            rconIndex++;
        }
        for (int i = 0; i < 4; i++) {
            expandedKey[bcount] =
                expandedKey[bcount - 16] ^ temp[i];
            bcount++;
        }
    }
}
/* 1. 라운드 키의 시작 위치를 구함
   2. 확장키[176]에 저장된 16바이트를 읽음
   3. 열 우선으로 라운드키에 배치*/
void GetRoundKey(const unsigned char expandedKey[176], int round, unsigned char roundKey[4][4]){
    int start = round * 16;
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            roundKey[j][i] =
                expandedKey[start + i*4 + j];
        }
    }
}
// 16 바이트 평문 배열을 state[4][4] 배열로 열 우선 배치
void load_state(unsigned char plain[16], unsigned char state[4][4]){   
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            state[j][i] = plain[4*i+j]; 
        }
    }
}

void Cipher_AES(unsigned char plain[16], unsigned char key[4][4], unsigned char outblock[16]){
    
    unsigned char state[4][4];
    unsigned char expandedKey[176];
    unsigned char roundKey[4][4];

    keyExpansion(key, expandedKey);
    load_state(plain, state);

    AddRoundKey(state, key);

    for(int round=1; round<10 ; round++){
        sub_bytes(state);
        shift_rows(state);
        MixColumns(state);
        GetRoundKey(expandedKey, round, roundKey);
        AddRoundKey(state, roundKey);
    }

    sub_bytes(state);
    shift_rows(state);

    GetRoundKey(expandedKey, 10, roundKey);
    AddRoundKey(state, roundKey);
    
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            outblock[4*i + j] = state[j][i];
        }
    }
}