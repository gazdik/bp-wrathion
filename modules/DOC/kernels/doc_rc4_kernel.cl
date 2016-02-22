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

#pragma OPENCL EXTENSION cl_amd_printf : enable

#define ROL(x,c) rotate((uint)x,(uint)c)

#define F1(B,C,D) ((D) ^ ((B)&((C)^(D))))
#define F2(B,C,D) ((C) ^ ((D) & ((B)^(C))))
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

void inline to_bytes(uint val,uchar *bytes)
{
    bytes[0] = (uchar) val;
    bytes[1] = (uchar) (val >> 8);
    bytes[2] = (uchar) (val >> 16);
    bytes[3] = (uchar) (val >> 24);
}

void inline md5_1block(uchar *input, uchar length,uchar *buffer){
    
    uint4 IV = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    
    if(length > 64-9){
        return;
    }
    
    input[length] = 0x80;
    for(int i = length+1;i<64-8;i++){
        input[i] = 0;
    }
    *((ulong*)(input+64-8)) = length*8;
    
    uint words[16];
    for(uchar i = 0;i<16;i++){ 
        words[i] = *((uint*)(input+i*4));
    }

    uint4 S;
    S = IV;

#define A S.s0
#define B S.s1
#define C S.s2
#define D S.s3

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

#undef A
#undef B
#undef C
#undef D

    IV += S;

    to_bytes(IV.s0, buffer);
    to_bytes(IV.s1, buffer + 4);
    to_bytes(IV.s2, buffer + 8);
    to_bytes(IV.s3, buffer + 12);

}

void inline md5_6blocks(uchar *input, ushort length,uchar *buffer){
    
    uint4 IV = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    
    if(length > 384-9){
        return;
    }
    
    uchar *msg = input;
    
    //padding
    input[length] = 0x80;
    for(int i = length+1;i<384-8;i++){
        input[i] = 0;
    }
    
    //length
    *((ulong*)(input+384-8)) = length*8;
    
    for(int i = 0;i<6;i++){
        uint words[16];
        for(uchar j = 0;j<16;j++){ 
            words[j] = *((uint*)(msg+j*4));
        }

        uint4 S;
        S = IV;

#define A S.s0
#define B S.s1
#define C S.s2
#define D S.s3

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

#undef A
#undef B
#undef C
#undef D

        IV += S;
        msg += 64;
    }
    to_bytes(IV.s0, buffer);
    to_bytes(IV.s1, buffer + 4);
    to_bytes(IV.s2, buffer + 8);
    to_bytes(IV.s3, buffer + 12);

}

typedef struct{
    uchar S[256];
    uchar i;
    uchar j;
} RC4State; 

#define SWAP(A,B){uchar tmp = A; A = B; B = tmp; }

void RC4Init128(local RC4State *State,uchar16 Key){
    State->i = 0;
    State->j = 0;
    #pragma unroll
    for (ushort i = 0;i<256;i++)
        State->S[i] = i;
    uchar j = 0;
    
    for (ushort i = 0;i<256;i+=16){
        j += State->S[i] + Key.s0;
        SWAP(State->S[i],State->S[j]);
        j += State->S[i+1] + Key.s1;
        SWAP(State->S[i+1],State->S[j]);
        j += State->S[i+2] + Key.s2;
        SWAP(State->S[i+2],State->S[j]);
        j += State->S[i+3] + Key.s3;
        SWAP(State->S[i+3],State->S[j]);
        j += State->S[i+4] + Key.s4;
        SWAP(State->S[i+4],State->S[j]);
        j += State->S[i+5] + Key.s5;
        SWAP(State->S[i+5],State->S[j]);
        j += State->S[i+6] + Key.s6;
        SWAP(State->S[i+6],State->S[j]);
        j += State->S[i+7] + Key.s7;
        SWAP(State->S[i+7],State->S[j]);
        j += State->S[i+8] + Key.s8;
        SWAP(State->S[i+8],State->S[j]);
        j += State->S[i+9] + Key.s9;
        SWAP(State->S[i+9],State->S[j]);
        j += State->S[i+10] + Key.sa;
        SWAP(State->S[i+10],State->S[j]);
        j += State->S[i+11] + Key.sb;
        SWAP(State->S[i+11],State->S[j]);
        j += State->S[i+12] + Key.sc;
        SWAP(State->S[i+12],State->S[j]);
        j += State->S[i+13] + Key.sd;
        SWAP(State->S[i+13],State->S[j]);
        j += State->S[i+14] + Key.se;
        SWAP(State->S[i+14],State->S[j]);
        j += State->S[i+15] + Key.sf;
        SWAP(State->S[i+15],State->S[j]);
    }
}

uchar inline RC4Keystream(local RC4State *State){
    uchar i = State->i + 1;
    uchar j = State->j + State->S[i];
    uchar Si,Sj;
    Si = State->S[i];
    Sj = State->S[j];
    SWAP(Si,Sj);
    uchar pos = Si + Sj;
    State->i = i;
    State->j = j;
    State->S[i] = Si;
    State->S[j] = Sj;
    return State->S[pos];
}

/**
 * @author Honza
 */
kernel void doc_rc4_kernel(global uchar* passwords, uchar pass_len, global uchar *found_flag, global uint *found_vector, constant uchar *salt, constant uchar *encVerifier, constant uchar *encVerifierHash) {
    int id = get_global_id(0);
    int lid = get_local_id(0);
    uchar my_pass_len = passwords[id*pass_len];
    uchar buffer[64];
    uchar bufferLarge[384];
    
    //copy and create Unicode16
    for(int i = 0;i<my_pass_len;i++){
        buffer[i*2] = passwords[id*pass_len+1+i];
        buffer[i*2+1] = 0;
    }
    my_pass_len *= 2;

    
    //hash raw password
    md5_1block(buffer,my_pass_len,buffer);
    
    //first 5 bytes of hash concatenate with 16bytes salt
    for(int i = 0;i<16;i++){
        buffer[i+5] = salt[i];
    }
    
    //repeat salted hash 16 times;
    for(int i = 0;i<16;i++){
        for(int j = 0;j<21;j++){
            bufferLarge[21*i+j] = buffer[j];
        }
    }
    
    //hash repeated salted hash
    md5_6blocks(bufferLarge,336,buffer);
    
    //create key for block 0 from first 5 bytes of hash and 4 bytes of block number in little-endian
    for(int i = 5;i<9;i++)
        buffer[i] = 0;
    
    md5_1block(buffer,9,buffer);

    uchar16 RC4_key = (uchar16)( buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15]);
    
    //reuse no longer needed workspace in fast local memory 
    local RC4State rc4state[64];
    
    //decrypt verifier
    RC4Init128(&rc4state[lid],RC4_key);
    for(int i = 0;i<16;i++){
        buffer[i] = encVerifier[i] ^ RC4Keystream(&rc4state[lid]);
    }
    
    //hash verifier
    md5_1block(buffer,16,buffer);
    
    for(int i = 0;i<16;i++){
        uchar hash_byte = encVerifierHash[i] ^ RC4Keystream(&rc4state[lid]);
        if(hash_byte != buffer[i]){
            return;
        }
    }
    *found_flag = 1;
    uint big_pos = id/32;
    uint small_pos = id%32;
    uint val = 0x80000000 >> small_pos;
    atomic_or(found_vector+big_pos,val);
}
