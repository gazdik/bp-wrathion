/* 
 * Copyright (C) 2014 Jan Schmied
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

#include "DOCRC4CrackerCPU.h"
#include <cstring>
#include <iostream>
#include <iomanip>

DOCRC4CrackerCPU::DOCRC4CrackerCPU(DOCInitData &data):data(data) {
}

DOCRC4CrackerCPU::DOCRC4CrackerCPU(const DOCRC4CrackerCPU& orig) {
}

DOCRC4CrackerCPU::~DOCRC4CrackerCPU() {
}

CheckResult DOCRC4CrackerCPU::checkPassword(const std::string* password) {
    uint8_t intermediateBuffer[384];
    uint8_t len = password->length()*2;
    createU16(password,intermediateBuffer);
    md5_1block(intermediateBuffer,len,intermediateBuffer);
    ::memcpy(intermediateBuffer+5,data.salt,16);
    for(int i = 1;i<16;i++){
        ::memcpy(intermediateBuffer+i*21,intermediateBuffer,21);
    }
    md5_6blocks(intermediateBuffer,336,intermediateBuffer);
    *(int*)(&intermediateBuffer[5]) = 0;

    md5_1block(intermediateBuffer,9,intermediateBuffer);

    RC4State rc4state;


    RC4Init128(&rc4state,intermediateBuffer);
    for(int i = 0;i<16;i++)
        intermediateBuffer[i] = data.encVerifier[i] ^ RC4KeystreamByte(&rc4state);

    md5_1block(intermediateBuffer,16,intermediateBuffer);
    for(int i = 0;i<16;i++){
        uint8_t hash_byte = data.encVerifierHash[i] ^ RC4KeystreamByte(&rc4state);
        if(hash_byte != intermediateBuffer[i]){
            return CR_PASSWORD_WRONG;
        }
    }
    return CR_PASSWORD_MATCH;
}


void DOCRC4CrackerCPU::createU16(const std::string* str, uint8_t* unicodeBuffer) {
    const char *asciistr = str->c_str();
    uint32_t len = str->length();
    for(uint32_t i = 0;i<len;i++){
        unicodeBuffer[i*2] = asciistr[i];
        unicodeBuffer[i*2+1] = 0;
    }
}


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
        A += (F(B,C,D) + K + M[G(I)]); \
        A = ROL((A), S); \
        A += B;

#define SWAP(A,B){ __typeof__(A) tmp = A; A = B; B = tmp; }

void inline DOCRC4CrackerCPU::to_bytes(uint32_t val, uint8_t *bytes)
{
    bytes[0] = (uint8_t) val;
    bytes[1] = (uint8_t) (val >> 8);
    bytes[2] = (uint8_t) (val >> 16);
    bytes[3] = (uint8_t) (val >> 24);
}

void inline DOCRC4CrackerCPU::md5_1block(uint8_t *input, uint32_t length, uint8_t *digest){
    
    uint32_t a0 = 0x67452301;   //A
    uint32_t b0 = 0xefcdab89;   //B
    uint32_t c0 = 0x98badcfe;   //C
    uint32_t d0 = 0x10325476;   //D
    

    if(length > 64-9){
        return;
    }
    
    input[length] = 0x80;
    for(uint8_t i = length+1; i<64-8;i++){
        input[i] = 0;
    }
    
    *((uint64_t*)(input+64-8)) = length*8;
    
    uint32_t words[16];
    for(uint8_t i = 0;i<16;i++){
        words[i] = *((int*)(input+i*4));
    }

    uint32_t A = a0;
    uint32_t B = b0;
    uint32_t C = c0;
    uint32_t D = d0;

    MD5_ROUND(A,B,C,D,G1,F1,0xd76aa478, 7,words, 0);
    MD5_ROUND(D,A,B,C,G1,F1,0xe8c7b756,12,words, 1);
    MD5_ROUND(C,D,A,B,G1,F1,0x242070db,17,words, 2);
    MD5_ROUND(B,C,D,A,G1,F1,0xc1bdceee,22,words, 3);
    MD5_ROUND(A,B,C,D,G1,F1,0xf57c0faf, 7,words, 4);
    MD5_ROUND(D,A,B,C,G1,F1,0x4787c62a,12,words, 5);
    MD5_ROUND(C,D,A,B,G1,F1,0xa8304613,17,words, 6);
    MD5_ROUND(B,C,D,A,G1,F1,0xfd469501,22,words, 7);
    MD5_ROUND(A,B,C,D,G1,F1,0x698098d8, 7,words, 8);
    MD5_ROUND(D,A,B,C,G1,F1,0x8b44f7af,12,words, 9);
    MD5_ROUND(C,D,A,B,G1,F1,0xffff5bb1,17,words,10);
    MD5_ROUND(B,C,D,A,G1,F1,0x895cd7be,22,words,11);
    MD5_ROUND(A,B,C,D,G1,F1,0x6b901122, 7,words,12);
    MD5_ROUND(D,A,B,C,G1,F1,0xfd987193,12,words,13);
    MD5_ROUND(C,D,A,B,G1,F1,0xa679438e,17,words,14);
    MD5_ROUND(B,C,D,A,G1,F1,0x49b40821,22,words,15);

    MD5_ROUND(A,B,C,D,G2,F2,0xf61e2562, 5,words,16);
    MD5_ROUND(D,A,B,C,G2,F2,0xc040b340, 9,words,17);
    MD5_ROUND(C,D,A,B,G2,F2,0x265e5a51,14,words,18);
    MD5_ROUND(B,C,D,A,G2,F2,0xe9b6c7aa,20,words,19);
    MD5_ROUND(A,B,C,D,G2,F2,0xd62f105d, 5,words,20);
    MD5_ROUND(D,A,B,C,G2,F2,0x02441453, 9,words,21);
    MD5_ROUND(C,D,A,B,G2,F2,0xd8a1e681,14,words,22);
    MD5_ROUND(B,C,D,A,G2,F2,0xe7d3fbc8,20,words,23);
    MD5_ROUND(A,B,C,D,G2,F2,0x21e1cde6, 5,words,24);
    MD5_ROUND(D,A,B,C,G2,F2,0xc33707d6, 9,words,25);
    MD5_ROUND(C,D,A,B,G2,F2,0xf4d50d87,14,words,26);
    MD5_ROUND(B,C,D,A,G2,F2,0x455a14ed,20,words,27);
    MD5_ROUND(A,B,C,D,G2,F2,0xa9e3e905, 5,words,28);
    MD5_ROUND(D,A,B,C,G2,F2,0xfcefa3f8, 9,words,29);
    MD5_ROUND(C,D,A,B,G2,F2,0x676f02d9,14,words,30);
    MD5_ROUND(B,C,D,A,G2,F2,0x8d2a4c8a,20,words,31);

    MD5_ROUND(A,B,C,D,G3,F3,0xfffa3942, 4,words,32);
    MD5_ROUND(D,A,B,C,G3,F3,0x8771f681,11,words,33);
    MD5_ROUND(C,D,A,B,G3,F3,0x6d9d6122,16,words,34);
    MD5_ROUND(B,C,D,A,G3,F3,0xfde5380c,23,words,35);
    MD5_ROUND(A,B,C,D,G3,F3,0xa4beea44, 4,words,36);
    MD5_ROUND(D,A,B,C,G3,F3,0x4bdecfa9,11,words,37);
    MD5_ROUND(C,D,A,B,G3,F3,0xf6bb4b60,16,words,38);
    MD5_ROUND(B,C,D,A,G3,F3,0xbebfbc70,23,words,39);
    MD5_ROUND(A,B,C,D,G3,F3,0x289b7ec6, 4,words,40);
    MD5_ROUND(D,A,B,C,G3,F3,0xeaa127fa,11,words,41);
    MD5_ROUND(C,D,A,B,G3,F3,0xd4ef3085,16,words,42);
    MD5_ROUND(B,C,D,A,G3,F3,0x04881d05,23,words,43);
    MD5_ROUND(A,B,C,D,G3,F3,0xd9d4d039, 4,words,44);
    MD5_ROUND(D,A,B,C,G3,F3,0xe6db99e5,11,words,45);
    MD5_ROUND(C,D,A,B,G3,F3,0x1fa27cf8,16,words,46);
    MD5_ROUND(B,C,D,A,G3,F3,0xc4ac5665,23,words,47);

    MD5_ROUND(A,B,C,D,G4,F4,0xf4292244, 6,words,48);
    MD5_ROUND(D,A,B,C,G4,F4,0x432aff97,10,words,49);
    MD5_ROUND(C,D,A,B,G4,F4,0xab9423a7,15,words,50);
    MD5_ROUND(B,C,D,A,G4,F4,0xfc93a039,21,words,51);
    MD5_ROUND(A,B,C,D,G4,F4,0x655b59c3, 6,words,52);
    MD5_ROUND(D,A,B,C,G4,F4,0x8f0ccc92,10,words,53);
    MD5_ROUND(C,D,A,B,G4,F4,0xffeff47d,15,words,54);
    MD5_ROUND(B,C,D,A,G4,F4,0x85845dd1,21,words,55);
    MD5_ROUND(A,B,C,D,G4,F4,0x6fa87e4f, 6,words,56);
    MD5_ROUND(D,A,B,C,G4,F4,0xfe2ce6e0,10,words,57);
    MD5_ROUND(C,D,A,B,G4,F4,0xa3014314,15,words,58);
    MD5_ROUND(B,C,D,A,G4,F4,0x4e0811a1,21,words,59);
    MD5_ROUND(A,B,C,D,G4,F4,0xf7537e82, 6,words,60);
    MD5_ROUND(D,A,B,C,G4,F4,0xbd3af235,10,words,61);
    MD5_ROUND(C,D,A,B,G4,F4,0x2ad7d2bb,15,words,62);
    MD5_ROUND(B,C,D,A,G4,F4,0xeb86d391,21,words,63);

    a0 += A;
    b0 += B;
    c0 += C;
    d0 += D;
    to_bytes(a0, digest);
    to_bytes(b0, digest + 4);
    to_bytes(c0, digest + 8);
    to_bytes(d0, digest + 12);
    
}

void inline DOCRC4CrackerCPU::md5_6blocks(uint8_t *input, uint32_t length, uint8_t *digest){
    
    uint32_t a0 = 0x67452301;   //A
    uint32_t b0 = 0xefcdab89;   //B
    uint32_t c0 = 0x98badcfe;   //C
    uint32_t d0 = 0x10325476;   //D
    
    if(length > 6*64-9){
        return;
    }
    
    input[length] = 0x80;
    for(uint16_t i = length+1; i<384-8;i++){
        input[i] = 0;
    }

    
    *((uint64_t*)(input+384-8)) = length*8;
    
    uint8_t *msg = input;
    for(uint8_t i = 0;i<6;i++){
        
        uint32_t words[16];
        for(uint8_t j = 0;j<16;j++){
            words[j] = *((int*)(msg+j*4));
        }
        
        uint32_t A = a0;
        uint32_t B = b0;
        uint32_t C = c0;
        uint32_t D = d0;

        MD5_ROUND(A,B,C,D,G1,F1,0xd76aa478, 7,words, 0);
        MD5_ROUND(D,A,B,C,G1,F1,0xe8c7b756,12,words, 1);
        MD5_ROUND(C,D,A,B,G1,F1,0x242070db,17,words, 2);
        MD5_ROUND(B,C,D,A,G1,F1,0xc1bdceee,22,words, 3);
        MD5_ROUND(A,B,C,D,G1,F1,0xf57c0faf, 7,words, 4);
        MD5_ROUND(D,A,B,C,G1,F1,0x4787c62a,12,words, 5);
        MD5_ROUND(C,D,A,B,G1,F1,0xa8304613,17,words, 6);
        MD5_ROUND(B,C,D,A,G1,F1,0xfd469501,22,words, 7);
        MD5_ROUND(A,B,C,D,G1,F1,0x698098d8, 7,words, 8);
        MD5_ROUND(D,A,B,C,G1,F1,0x8b44f7af,12,words, 9);
        MD5_ROUND(C,D,A,B,G1,F1,0xffff5bb1,17,words,10);
        MD5_ROUND(B,C,D,A,G1,F1,0x895cd7be,22,words,11);
        MD5_ROUND(A,B,C,D,G1,F1,0x6b901122, 7,words,12);
        MD5_ROUND(D,A,B,C,G1,F1,0xfd987193,12,words,13);
        MD5_ROUND(C,D,A,B,G1,F1,0xa679438e,17,words,14);
        MD5_ROUND(B,C,D,A,G1,F1,0x49b40821,22,words,15);

        MD5_ROUND(A,B,C,D,G2,F2,0xf61e2562, 5,words,16);
        MD5_ROUND(D,A,B,C,G2,F2,0xc040b340, 9,words,17);
        MD5_ROUND(C,D,A,B,G2,F2,0x265e5a51,14,words,18);
        MD5_ROUND(B,C,D,A,G2,F2,0xe9b6c7aa,20,words,19);
        MD5_ROUND(A,B,C,D,G2,F2,0xd62f105d, 5,words,20);
        MD5_ROUND(D,A,B,C,G2,F2,0x02441453, 9,words,21);
        MD5_ROUND(C,D,A,B,G2,F2,0xd8a1e681,14,words,22);
        MD5_ROUND(B,C,D,A,G2,F2,0xe7d3fbc8,20,words,23);
        MD5_ROUND(A,B,C,D,G2,F2,0x21e1cde6, 5,words,24);
        MD5_ROUND(D,A,B,C,G2,F2,0xc33707d6, 9,words,25);
        MD5_ROUND(C,D,A,B,G2,F2,0xf4d50d87,14,words,26);
        MD5_ROUND(B,C,D,A,G2,F2,0x455a14ed,20,words,27);
        MD5_ROUND(A,B,C,D,G2,F2,0xa9e3e905, 5,words,28);
        MD5_ROUND(D,A,B,C,G2,F2,0xfcefa3f8, 9,words,29);
        MD5_ROUND(C,D,A,B,G2,F2,0x676f02d9,14,words,30);
        MD5_ROUND(B,C,D,A,G2,F2,0x8d2a4c8a,20,words,31);

        MD5_ROUND(A,B,C,D,G3,F3,0xfffa3942, 4,words,32);
        MD5_ROUND(D,A,B,C,G3,F3,0x8771f681,11,words,33);
        MD5_ROUND(C,D,A,B,G3,F3,0x6d9d6122,16,words,34);
        MD5_ROUND(B,C,D,A,G3,F3,0xfde5380c,23,words,35);
        MD5_ROUND(A,B,C,D,G3,F3,0xa4beea44, 4,words,36);
        MD5_ROUND(D,A,B,C,G3,F3,0x4bdecfa9,11,words,37);
        MD5_ROUND(C,D,A,B,G3,F3,0xf6bb4b60,16,words,38);
        MD5_ROUND(B,C,D,A,G3,F3,0xbebfbc70,23,words,39);
        MD5_ROUND(A,B,C,D,G3,F3,0x289b7ec6, 4,words,40);
        MD5_ROUND(D,A,B,C,G3,F3,0xeaa127fa,11,words,41);
        MD5_ROUND(C,D,A,B,G3,F3,0xd4ef3085,16,words,42);
        MD5_ROUND(B,C,D,A,G3,F3,0x04881d05,23,words,43);
        MD5_ROUND(A,B,C,D,G3,F3,0xd9d4d039, 4,words,44);
        MD5_ROUND(D,A,B,C,G3,F3,0xe6db99e5,11,words,45);
        MD5_ROUND(C,D,A,B,G3,F3,0x1fa27cf8,16,words,46);
        MD5_ROUND(B,C,D,A,G3,F3,0xc4ac5665,23,words,47);

        MD5_ROUND(A,B,C,D,G4,F4,0xf4292244, 6,words,48);
        MD5_ROUND(D,A,B,C,G4,F4,0x432aff97,10,words,49);
        MD5_ROUND(C,D,A,B,G4,F4,0xab9423a7,15,words,50);
        MD5_ROUND(B,C,D,A,G4,F4,0xfc93a039,21,words,51);
        MD5_ROUND(A,B,C,D,G4,F4,0x655b59c3, 6,words,52);
        MD5_ROUND(D,A,B,C,G4,F4,0x8f0ccc92,10,words,53);
        MD5_ROUND(C,D,A,B,G4,F4,0xffeff47d,15,words,54);
        MD5_ROUND(B,C,D,A,G4,F4,0x85845dd1,21,words,55);
        MD5_ROUND(A,B,C,D,G4,F4,0x6fa87e4f, 6,words,56);
        MD5_ROUND(D,A,B,C,G4,F4,0xfe2ce6e0,10,words,57);
        MD5_ROUND(C,D,A,B,G4,F4,0xa3014314,15,words,58);
        MD5_ROUND(B,C,D,A,G4,F4,0x4e0811a1,21,words,59);
        MD5_ROUND(A,B,C,D,G4,F4,0xf7537e82, 6,words,60);
        MD5_ROUND(D,A,B,C,G4,F4,0xbd3af235,10,words,61);
        MD5_ROUND(C,D,A,B,G4,F4,0x2ad7d2bb,15,words,62);
        MD5_ROUND(B,C,D,A,G4,F4,0xeb86d391,21,words,63);

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

void inline DOCRC4CrackerCPU::RC4Init128(RC4State *rc4state, uint8_t* key){
    rc4state->i = 0;
    rc4state->j = 0;
    for (int i = 0;i<256;i++)
        rc4state->S[i] = i;
    uint8_t j = 0;
    for (int i = 0;i<256;i++){
        j += rc4state->S[i] + key[i & 0xF];
        SWAP(rc4state->S[i],rc4state->S[j]);
    }
}

void DOCRC4CrackerCPU::RC4Process(RC4State *rc4state, uint8_t *data, uint32_t len){
    uint8_t key_stream[20];
    uint8_t *S = rc4state->S;
    for(uint32_t i = 0;i<len;i++){
        rc4state->i = (rc4state->i + 1);
        rc4state->j = (rc4state->j + S[rc4state->i]);
        SWAP(S[rc4state->i],S[rc4state->j]);
        uint8_t pos = (S[rc4state->i] + S[rc4state->j]);
        key_stream[i] = S[pos];
    }
    for(uint32_t i = 0;i<len;i++){
        data[i] ^= key_stream[i];
    }
}

uint8_t inline DOCRC4CrackerCPU::RC4KeystreamByte(RC4State* rc4state) {
    uint8_t *S = rc4state->S;
    rc4state->i = (rc4state->i + 1);
    rc4state->j = (rc4state->j + S[rc4state->i]);
    SWAP(S[rc4state->i],S[rc4state->j]);
    uint8_t pos = (S[rc4state->i] + S[rc4state->j]);
    return S[pos];
}


