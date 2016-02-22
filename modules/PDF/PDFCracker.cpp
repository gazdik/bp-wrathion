/* 
 * Copyright (C) 2014 Jan Schmied, Radek Hranicky
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 */

#include "PDFCracker.h"
#include "sha256.h"

PDFCracker::PDFCracker(PDFInitData &data):data(data) {
    
}

PDFCracker::PDFCracker(const PDFCracker& orig):data(orig.data) {
}

PDFCracker::~PDFCracker() {
}

#define SWAP(A,B){ __typeof__(A) tmp = A; A = B; B = tmp; }

void PDFCracker::RC4Init(uint8_t* key, uint16_t keylen){
    rc4state.i = 0;
    rc4state.j = 0;
    for (int i = 0;i<256;i++)
        rc4state.S[i] = i;
    uint8_t j = 0;
    for (int i = 0;i<256;i++){
        j += rc4state.S[i] + key[i % keylen];
        SWAP(rc4state.S[i],rc4state.S[j]);
    }
}

void PDFCracker::RC4Process(uint8_t *data, uint32_t len){
    uint8_t key_stream[20];
    uint8_t *S = rc4state.S;
    for(uint32_t i = 0;i<len;i++){
        rc4state.i = (rc4state.i + 1);
        rc4state.j = (rc4state.j + S[rc4state.i]);
        SWAP(S[rc4state.i],S[rc4state.j]);
        uint8_t pos = (S[rc4state.i] + S[rc4state.j]);
        key_stream[i] = S[pos];
    }
    for(uint32_t i = 0;i<len;i++){
        data[i] ^= key_stream[i];
    }
}

const uint8_t PDFCracker::passpad[] = {
    0x28,0xBF,0x4E,0x5E,0x4E,0x75,0x8A,0x41,0x64,0x00,0x4E,0x56,0xFF,0xFA,0x01,0x08,
    0x2E,0x2E,0x00,0xB6,0xD0,0x68,0x3E,0x80,0x2F,0x0C,0xA9,0xFE,0x64,0x53,0x69,0x7A
};

/* ======= MD5 ======= */

#define ROL(x,c) (((x) << (c)) | ((x) >> (32 - (c))))

#define F1(B,C,D) ((B & C) | ((~B) & D))
#define F2(B,C,D) ((D & B) | ((~D) & C))
#define F3(B,C,D) ((B) ^ (C) ^ (D))
#define F4(B,C,D) ((C) ^ ((B) | (~D)))

#define G1(x) (x)
#define G2(x) ((5*(x) + 1) % 16)
#define G3(x) ((3*(x) + 5) % 16)
#define G4(x) ((7*(x)) % 16)

#define MD5_ROUND(A,B,C,D,G,F,K,S,M,I) \
        A += (F(B,C,D) + K[I] + M[G(I)]); \
        A = ROL((A), S[I]); \
        A += B;

void to_bytes(uint32_t val, uint8_t *bytes)
{
    bytes[0] = (uint8_t) val;
    bytes[1] = (uint8_t) (val >> 8);
    bytes[2] = (uint8_t) (val >> 16);
    bytes[3] = (uint8_t) (val >> 24);
}

void PDFCracker::MD5(const uint8_t *input, uint32_t length, uint8_t *digest){
    const uint8_t S[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };
    
    const uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 
    };
    
    uint32_t a0 = 0x67452301;   //A
    uint32_t b0 = 0xefcdab89;   //B
    uint32_t c0 = 0x98badcfe;   //C
    uint32_t d0 = 0x10325476;   //D
    
    uint8_t msg_space[128] = {0};
    uint8_t *msg = msg_space;
    if(length > 2*64-9){
        return;
    }
    
    for(uint8_t i = 0;i<length;i++){
        msg_space[i] = input[i];
    }
    msg_space[length] = 0x80;
    uint8_t pos = 56;
    uint8_t chunks = 1;
    if(length > 54){
        pos = 120;
        chunks = 2;
    }
    
    *((int*)(msg_space+pos)) = length*8;
    
    for(uint8_t i = 0;i<chunks;i++){
        
        uint32_t words[16];
        for(uint8_t i = 0;i<16;i++){
            words[i] = *((int*)(msg+i*4));
        }
        
        uint32_t A = a0;
        uint32_t B = b0;
        uint32_t C = c0;
        uint32_t D = d0;

        MD5_ROUND(A,B,C,D,G1,F1,K,S,words, 0);
        MD5_ROUND(D,A,B,C,G1,F1,K,S,words, 1);
        MD5_ROUND(C,D,A,B,G1,F1,K,S,words, 2);
        MD5_ROUND(B,C,D,A,G1,F1,K,S,words, 3);
        MD5_ROUND(A,B,C,D,G1,F1,K,S,words, 4);
        MD5_ROUND(D,A,B,C,G1,F1,K,S,words, 5);
        MD5_ROUND(C,D,A,B,G1,F1,K,S,words, 6);
        MD5_ROUND(B,C,D,A,G1,F1,K,S,words, 7);
        MD5_ROUND(A,B,C,D,G1,F1,K,S,words, 8);
        MD5_ROUND(D,A,B,C,G1,F1,K,S,words, 9);
        MD5_ROUND(C,D,A,B,G1,F1,K,S,words,10);
        MD5_ROUND(B,C,D,A,G1,F1,K,S,words,11);
        MD5_ROUND(A,B,C,D,G1,F1,K,S,words,12);
        MD5_ROUND(D,A,B,C,G1,F1,K,S,words,13);
        MD5_ROUND(C,D,A,B,G1,F1,K,S,words,14);
        MD5_ROUND(B,C,D,A,G1,F1,K,S,words,15);

        MD5_ROUND(A,B,C,D,G2,F2,K,S,words,16);
        MD5_ROUND(D,A,B,C,G2,F2,K,S,words,17);
        MD5_ROUND(C,D,A,B,G2,F2,K,S,words,18);
        MD5_ROUND(B,C,D,A,G2,F2,K,S,words,19);
        MD5_ROUND(A,B,C,D,G2,F2,K,S,words,20);
        MD5_ROUND(D,A,B,C,G2,F2,K,S,words,21);
        MD5_ROUND(C,D,A,B,G2,F2,K,S,words,22);
        MD5_ROUND(B,C,D,A,G2,F2,K,S,words,23);
        MD5_ROUND(A,B,C,D,G2,F2,K,S,words,24);
        MD5_ROUND(D,A,B,C,G2,F2,K,S,words,25);
        MD5_ROUND(C,D,A,B,G2,F2,K,S,words,26);
        MD5_ROUND(B,C,D,A,G2,F2,K,S,words,27);
        MD5_ROUND(A,B,C,D,G2,F2,K,S,words,28);
        MD5_ROUND(D,A,B,C,G2,F2,K,S,words,29);
        MD5_ROUND(C,D,A,B,G2,F2,K,S,words,30);
        MD5_ROUND(B,C,D,A,G2,F2,K,S,words,31);

        MD5_ROUND(A,B,C,D,G3,F3,K,S,words,32);
        MD5_ROUND(D,A,B,C,G3,F3,K,S,words,33);
        MD5_ROUND(C,D,A,B,G3,F3,K,S,words,34);
        MD5_ROUND(B,C,D,A,G3,F3,K,S,words,35);
        MD5_ROUND(A,B,C,D,G3,F3,K,S,words,36);
        MD5_ROUND(D,A,B,C,G3,F3,K,S,words,37);
        MD5_ROUND(C,D,A,B,G3,F3,K,S,words,38);
        MD5_ROUND(B,C,D,A,G3,F3,K,S,words,39);
        MD5_ROUND(A,B,C,D,G3,F3,K,S,words,40);
        MD5_ROUND(D,A,B,C,G3,F3,K,S,words,41);
        MD5_ROUND(C,D,A,B,G3,F3,K,S,words,42);
        MD5_ROUND(B,C,D,A,G3,F3,K,S,words,43);
        MD5_ROUND(A,B,C,D,G3,F3,K,S,words,44);
        MD5_ROUND(D,A,B,C,G3,F3,K,S,words,45);
        MD5_ROUND(C,D,A,B,G3,F3,K,S,words,46);
        MD5_ROUND(B,C,D,A,G3,F3,K,S,words,47);

        MD5_ROUND(A,B,C,D,G4,F4,K,S,words,48);
        MD5_ROUND(D,A,B,C,G4,F4,K,S,words,49);
        MD5_ROUND(C,D,A,B,G4,F4,K,S,words,50);
        MD5_ROUND(B,C,D,A,G4,F4,K,S,words,51);
        MD5_ROUND(A,B,C,D,G4,F4,K,S,words,52);
        MD5_ROUND(D,A,B,C,G4,F4,K,S,words,53);
        MD5_ROUND(C,D,A,B,G4,F4,K,S,words,54);
        MD5_ROUND(B,C,D,A,G4,F4,K,S,words,55);
        MD5_ROUND(A,B,C,D,G4,F4,K,S,words,56);
        MD5_ROUND(D,A,B,C,G4,F4,K,S,words,57);
        MD5_ROUND(C,D,A,B,G4,F4,K,S,words,58);
        MD5_ROUND(B,C,D,A,G4,F4,K,S,words,59);
        MD5_ROUND(A,B,C,D,G4,F4,K,S,words,60);
        MD5_ROUND(D,A,B,C,G4,F4,K,S,words,61);
        MD5_ROUND(C,D,A,B,G4,F4,K,S,words,62);
        MD5_ROUND(B,C,D,A,G4,F4,K,S,words,63);

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
        msg += 64;
    }
    to_bytes(a0, digest);
    to_bytes(b0, digest + 4);
    to_bytes(c0, digest + 8);
    to_bytes(d0, digest + 12);
    
}

/* ======= SHA-2 ======= */

std::string PDFCracker::sha2(int type, std::string input) {
    uint8_t hash[32];
    if (type == V_256) {
        sha256f((const uint8_t *)input.c_str(), input.size(), hash);
        return std::string((char *)hash);
    } else if (type == V_384) {
        
    } else if (type == V_512) {
        
    } else {
        return std::string("12345678901234567890123456789012");
    }
}
