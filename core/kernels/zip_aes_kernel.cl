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

#define ROL(x,c) rotate((uint)x,(uint)c)

#define ROUNDTAIL(a,b,e,f,i,k,w)  \
	e += ROL(a,5) + f + k + w[i];  \
	b = ROL(b,30);

#define F1(b,c,d) (d ^ (b & (c ^ d)))
#define F2(b,c,d) (b ^ c ^ d)
#define F3(b,c,d) ((b & c) ^ (b & d) ^ (c & d))
#define F4(b,c,d) (b ^ c ^ d)


#define LOADSCHEDULE(i, w, block)\
        w[i] = (block+i*4)[0] << 24 | (block+i*4)[1] << 16 | (block+i*4)[2] << 8 | (block+i*4)[3];

#define SCHEDULE(i, w) \
        w[i] = ROL((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);

#define ROUND0s(a,b,c,d,e,i,w,block) \
        LOADSCHEDULE(i, w, block)\
        ROUNDTAIL(a, b, e, F1(b, c, d), i, 0x5A827999, w)

#define ROUND0(a,b,c,d,e,i,w) \
        SCHEDULE(i, w) \
        ROUNDTAIL(a, b, e, F1(b, c, d), i, 0x5A827999, w)

#define ROUND1(a,b,c,d,e,i,w) \
        SCHEDULE(i, w) \
        ROUNDTAIL(a, b, e, F2(b, c, d), i, 0x6ED9EBA1, w)

#define ROUND2(a,b,c,d,e,i,w) \
        SCHEDULE(i, w) \
        ROUNDTAIL(a, b, e, F3(b, c, d), i, 0x8F1BBCDC, w)

#define ROUND3(a,b,c,d,e,i,w) \
        SCHEDULE(i, w) \
        ROUNDTAIL(a, b, e, F4(b, c, d), i, 0xCA62C1D6, w)

void inline sha1(const uchar* msg, unsigned int len, uchar* output){
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xEFCDAB89;
    unsigned int h2 = 0x98BADCFE;
    unsigned int h3 = 0x10325476;
    unsigned int h4 = 0xC3D2E1F0;
    
    unsigned int chunks = ((len+9)/64)+1;
    
    unsigned char msg_pad_space[2*64] = {0};
    unsigned char *msg_pad = msg_pad_space;
    unsigned int w[80];
    
    unsigned char pos = 62;
    
    if(len > 2*64-9){
        return;
    }
    for(int i = 0;i<len;i++)
        msg_pad[i] = msg[i];
    msg_pad[len] = 0x80;
    if(len > 54)
        pos = 126;

    
    unsigned long bit_len = len*8;
    msg_pad[pos++] = (bit_len >> 8) & 0xFF;
    msg_pad[pos] = bit_len & 0xFF;
    
    for(int chunk = 0;chunk<chunks;chunk++){

        unsigned int a = h0;
        unsigned int b = h1;
        unsigned int c = h2;
        unsigned int d = h3;
        unsigned int e = h4;

        
        ROUND0s(a, b, c, d, e,  0, w, msg_pad)
	ROUND0s(e, a, b, c, d,  1, w, msg_pad)
	ROUND0s(d, e, a, b, c,  2, w, msg_pad)
	ROUND0s(c, d, e, a, b,  3, w, msg_pad)
	ROUND0s(b, c, d, e, a,  4, w, msg_pad)
	ROUND0s(a, b, c, d, e,  5, w, msg_pad)
	ROUND0s(e, a, b, c, d,  6, w, msg_pad)
	ROUND0s(d, e, a, b, c,  7, w, msg_pad)
	ROUND0s(c, d, e, a, b,  8, w, msg_pad)
	ROUND0s(b, c, d, e, a,  9, w, msg_pad)
	ROUND0s(a, b, c, d, e, 10, w, msg_pad)
	ROUND0s(e, a, b, c, d, 11, w, msg_pad)
	ROUND0s(d, e, a, b, c, 12, w, msg_pad)
	ROUND0s(c, d, e, a, b, 13, w, msg_pad)
	ROUND0s(b, c, d, e, a, 14, w, msg_pad)
	ROUND0s(a, b, c, d, e, 15, w, msg_pad)
	ROUND0(e, a, b, c, d, 16, w)
	ROUND0(d, e, a, b, c, 17, w)
	ROUND0(c, d, e, a, b, 18, w)
	ROUND0(b, c, d, e, a, 19, w)
	ROUND1(a, b, c, d, e, 20, w)
	ROUND1(e, a, b, c, d, 21, w)
	ROUND1(d, e, a, b, c, 22, w)
	ROUND1(c, d, e, a, b, 23, w)
	ROUND1(b, c, d, e, a, 24, w)
	ROUND1(a, b, c, d, e, 25, w)
	ROUND1(e, a, b, c, d, 26, w)
	ROUND1(d, e, a, b, c, 27, w)
	ROUND1(c, d, e, a, b, 28, w)
	ROUND1(b, c, d, e, a, 29, w)
	ROUND1(a, b, c, d, e, 30, w)
	ROUND1(e, a, b, c, d, 31, w)
	ROUND1(d, e, a, b, c, 32, w)
	ROUND1(c, d, e, a, b, 33, w)
	ROUND1(b, c, d, e, a, 34, w)
	ROUND1(a, b, c, d, e, 35, w)
	ROUND1(e, a, b, c, d, 36, w)
	ROUND1(d, e, a, b, c, 37, w)
	ROUND1(c, d, e, a, b, 38, w)
	ROUND1(b, c, d, e, a, 39, w)
	ROUND2(a, b, c, d, e, 40, w)
	ROUND2(e, a, b, c, d, 41, w)
	ROUND2(d, e, a, b, c, 42, w)
	ROUND2(c, d, e, a, b, 43, w)
	ROUND2(b, c, d, e, a, 44, w)
	ROUND2(a, b, c, d, e, 45, w)
	ROUND2(e, a, b, c, d, 46, w)
	ROUND2(d, e, a, b, c, 47, w)
	ROUND2(c, d, e, a, b, 48, w)
	ROUND2(b, c, d, e, a, 49, w)
	ROUND2(a, b, c, d, e, 50, w)
	ROUND2(e, a, b, c, d, 51, w)
	ROUND2(d, e, a, b, c, 52, w)
	ROUND2(c, d, e, a, b, 53, w)
	ROUND2(b, c, d, e, a, 54, w)
	ROUND2(a, b, c, d, e, 55, w)
	ROUND2(e, a, b, c, d, 56, w)
	ROUND2(d, e, a, b, c, 57, w)
	ROUND2(c, d, e, a, b, 58, w)
	ROUND2(b, c, d, e, a, 59, w)
	ROUND3(a, b, c, d, e, 60, w)
	ROUND3(e, a, b, c, d, 61, w)
	ROUND3(d, e, a, b, c, 62, w)
	ROUND3(c, d, e, a, b, 63, w)
	ROUND3(b, c, d, e, a, 64, w)
	ROUND3(a, b, c, d, e, 65, w)
	ROUND3(e, a, b, c, d, 66, w)
	ROUND3(d, e, a, b, c, 67, w)
	ROUND3(c, d, e, a, b, 68, w)
	ROUND3(b, c, d, e, a, 69, w)
	ROUND3(a, b, c, d, e, 70, w)
	ROUND3(e, a, b, c, d, 71, w)
	ROUND3(d, e, a, b, c, 72, w)
	ROUND3(c, d, e, a, b, 73, w)
	ROUND3(b, c, d, e, a, 74, w)
	ROUND3(a, b, c, d, e, 75, w)
	ROUND3(e, a, b, c, d, 76, w)
	ROUND3(d, e, a, b, c, 77, w)
	ROUND3(c, d, e, a, b, 78, w)
	ROUND3(b, c, d, e, a, 79, w)
        
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        msg_pad += 64;
    }
    output[0] = h0 >> 24;
    output[1] = (h0 >> 16) & 0xFF;
    output[2] = (h0 >> 8) & 0xFF;
    output[3] = h0 & 0xFF;
    
    output[4] = h1 >> 24;
    output[5] = (h1 >> 16) & 0xFF;
    output[6] = (h1 >> 8) & 0xFF;
    output[7] = h1 & 0xFF;
    
    output[8] = h2 >> 24;
    output[9] = (h2 >> 16) & 0xFF;
    output[10] = (h2 >> 8) & 0xFF;
    output[11] = h2 & 0xFF;
    
    output[12] = h3 >> 24;
    output[13] = (h3 >> 16) & 0xFF;
    output[14] = (h3 >> 8) & 0xFF;
    output[15] = h3 & 0xFF;
    
    output[16] = h4 >> 24;
    output[17] = (h4 >> 16) & 0xFF;
    output[18] = (h4 >> 8) & 0xFF;
    output[19] = h4 & 0xFF;
    
}

void inline hmac_sha1(const uchar* msg, unsigned int msgLen, const uchar* key, unsigned int keyLen, uchar* output){
    uchar key_pad[128];
    
    #pragma unroll
    for(uchar i = 0;i<64;i++) key_pad[i] = 0x36;
    
    for(uchar i = 0;i<keyLen; i++){
        key_pad[i] ^= key[i];
    }

    for(ushort i = 0;i<msgLen;i++) key_pad[i+64] = msg[i];
    sha1(key_pad,64+msgLen,output);
    #pragma unroll
    for(uchar i = 0;i<64;i++) key_pad[i] = 0x5C;
    for(uchar i = 0;i<keyLen; i++){
        key_pad[i] ^= key[i];
    }
    #pragma unroll
    for(int i = 0;i<20;i++) key_pad[i+64] = output[i];
    sha1(key_pad,84,output);
}

#define ADD_XOR(res, in) \
        _Pragma("unroll") \
        for (int i = 0;i<20;i++) \
             res[i] ^= in[i];

void pbkdf2_sha1_zip_aes256(const uchar* pass, unsigned int passLen, constant uchar* in_salt, uchar* output){

    uchar prevU[20] = {0};
    uchar Fres[20];    
    

    #pragma unroll
    for(uchar i = 0;i<20;i++) Fres[i] = 0;
    #pragma unroll
    for(int i = 0;i<16;i++) prevU[i] = in_salt[i];

    prevU[19] = 4;

    hmac_sha1(prevU,20,pass,passLen,prevU);
    ADD_XOR(Fres,prevU);
    for(ushort c=1;c<1000;c++){
        hmac_sha1(prevU,20,pass,passLen,prevU);
        ADD_XOR(Fres,prevU);
    }
    #pragma unroll
    for(uchar i = 0;i<2;i++) output[i] = Fres[4+i]; 
}

void pbkdf2_sha1_zip_aes192(const uchar* pass, unsigned int passLen, const uchar* in_salt, uchar* output){

    uchar prevU[20] = {0};
    uchar Fres[20];    
    

    #pragma unroll
    for(uchar i = 0;i<20;i++) Fres[i] = 0;
    #pragma unroll
    for(int i = 0;i<12;i++) prevU[i] = in_salt[i];

    prevU[15] = 3;

    hmac_sha1(prevU,16,pass,passLen,prevU);

    ADD_XOR(Fres,prevU);
    for(ushort c=1;c<1000;c++){
        hmac_sha1(prevU,20,pass,passLen,prevU);
        ADD_XOR(Fres,prevU);
    }
    #pragma unroll
    for(uchar i = 0;i<2;i++) output[i] = Fres[8+i]; 
}

void pbkdf2_sha1_zip_aes128(const uchar* pass, unsigned int passLen, const uchar* in_salt, uchar* output){

    uchar prevU[20] = {0};
    uchar Fres[20];    
    

    #pragma unroll
    for(uchar i = 0;i<20;i++) Fres[i] = 0;
    #pragma unroll
    for(int i = 0;i<8;i++) prevU[i] = in_salt[i];

    prevU[11] = 2;

    hmac_sha1(prevU,12,pass,passLen,prevU);

    ADD_XOR(Fres,prevU);
    for(ushort c=1;c<1000;c++){
        hmac_sha1(prevU,20,pass,passLen,prevU);
        ADD_XOR(Fres,prevU);
    }
    #pragma unroll
    for(uchar i = 0;i<2;i++) output[i] = Fres[12+i]; 
}

/**
 * @author Honza
 */
kernel void zip_aes256_kernel(global uchar* passwords, uchar pass_len, global uchar *found_flag, global uint *found_vector, constant uchar *salt, constant uchar *verifier) {
    int id = get_global_id(0);
    
    uchar my_pass_len = passwords[id*pass_len];
    uchar pass_buffer[32];
    uchar mverifier[2];
    
    for(int i = 0;i<my_pass_len;i++){
        pass_buffer[i] = passwords[id*pass_len+1+i];
    }

    pbkdf2_sha1_zip_aes256(pass_buffer,my_pass_len,salt,mverifier);
    
    for(int i = 0;i<2;i++){
        if(mverifier[i] != verifier[i]){
            return;
        }
    }
    
    *found_flag = 1;
    uint big_pos = id/32;
    uint small_pos = id%32;
    uint val = 0x80000000 >> small_pos;
    atomic_or(found_vector+big_pos,val);
}

kernel void zip_aes192_kernel(global uchar* passwords, uchar pass_len, global uchar *found_flag, global uint *found_vector, constant uchar *salt, constant uchar *verifier) {
    int id = get_global_id(0);
    
    uchar my_pass_len = passwords[id*pass_len];
    uchar pass_buffer[32];
    uchar psalt[16];
    uchar mverifier[2];
    
    for(int i = 0;i<my_pass_len;i++){
        pass_buffer[i] = passwords[id*pass_len+1+i];
    }

    for(int i = 0;i<16;i++){
        psalt[i] = salt[i];
    }

    pbkdf2_sha1_zip_aes192(pass_buffer,my_pass_len,psalt,mverifier);
    
    for(int i = 0;i<2;i++){
        if(mverifier[i] != verifier[i]){
            return;
        }
    }
    
    *found_flag = 1;
    uint big_pos = id/32;
    uint small_pos = id%32;
    uint val = 0x80000000 >> small_pos;
    atomic_or(found_vector+big_pos,val);
}

kernel void zip_aes128_kernel(global uchar* passwords, uchar pass_len, global uchar *found_flag, global uint *found_vector, constant uchar *salt, constant uchar *verifier) {
    int id = get_global_id(0);
    
    uchar my_pass_len = passwords[id*pass_len];
    uchar pass_buffer[32];
    uchar psalt[16];
    uchar mverifier[2];
    
    for(int i = 0;i<my_pass_len;i++){
        pass_buffer[i] = passwords[id*pass_len+1+i];
    }

    for(int i = 0;i<16;i++){
        psalt[i] = salt[i];
    }

    pbkdf2_sha1_zip_aes128(pass_buffer,my_pass_len,psalt,mverifier);
    
    for(int i = 0;i<2;i++){
        if(mverifier[i] != verifier[i]){
            return;
        }
    }
    
    *found_flag = 1;
    uint big_pos = id/32;
    uint small_pos = id%32;
    uint val = 0x80000000 >> small_pos;
    atomic_or(found_vector+big_pos,val);
}