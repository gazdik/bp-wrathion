/* 
 * Copyright (C) 2015 Radek Hranicky
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
 
/* Password buffer size
 * default: 56 + 1 (56B is maximum for fast-SHA256)
 */
#define PASS_BUFFER_SIZE 57
#define SHA256_HASH_SIZE 32
#define U_VALID_SALT_SIZE 8
#define ROTR(x, n) (( x >> n ) | ( x << (32 - n)))
#define Choice(x, y, z) ( z ^ ( x & ( y ^ z )))
#define Majority(x, y, z) (( x & y ) ^ ( z & ( x ^ y )))
#define Sigma0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define Sigma1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))
#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))

#define ROUND(a,b,c,d,e,f,g,h,k,data)			\
    h += Sigma1(e) + Choice(e, f, g) + k + data;		\
    d += h;						\
    h += Sigma0(a) + Majority(a ,b ,c);


inline static void pdf_r5_sha256HashBlock(const uchar *blk, uint *hash) {
    uint W[64];
    uint A, B, C, D, E, F, G, H;
    int i;

    #pragma unroll
    for (i = 0; i < 16; ++i) {
        W[i] = (blk[i*4    ] << 24) 
            | (blk[i*4 + 1] << 16) 
            | (blk[i*4 + 2] << 8) 
            |  blk[i*4 + 3];
    }
    for (; i < 64; ++i) {
        W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];
    }

    A = hash[0];
    B = hash[1];
    C = hash[2];
    D = hash[3];
    E = hash[4];
    F = hash[5];
    G = hash[6];
    H = hash[7];
  
    ROUND(A, B, C, D, E, F, G, H, 0x428a2f98, W[ 0]);
    ROUND(H, A, B, C, D, E, F, G, 0x71374491, W[ 1]);
    ROUND(G, H, A, B, C, D, E, F, 0xB5C0FBCF, W[ 2]);
    ROUND(F, G, H, A, B, C, D, E, 0xE9B5DBA5, W[ 3]);
    ROUND(E, F, G, H, A, B, C, D, 0x3956C25B, W[ 4]);
    ROUND(D, E, F, G, H, A, B, C, 0x59F111F1, W[ 5]);
    ROUND(C, D, E, F, G, H, A, B, 0x923F82A4, W[ 6]);
    ROUND(B, C, D, E, F, G, H, A, 0xAB1C5ED5, W[ 7]);
    ROUND(A, B, C, D, E, F, G, H, 0xD807AA98, W[ 8]);
    ROUND(H, A, B, C, D, E, F, G, 0x12835B01, W[ 9]);
    ROUND(G, H, A, B, C, D, E, F, 0x243185BE, W[10]);
    ROUND(F, G, H, A, B, C, D, E, 0x550C7DC3, W[11]);
    ROUND(E, F, G, H, A, B, C, D, 0x72BE5D74, W[12]);
    ROUND(D, E, F, G, H, A, B, C, 0x80DEB1FE, W[13]);
    ROUND(C, D, E, F, G, H, A, B, 0x9BDC06A7, W[14]);
    ROUND(B, C, D, E, F, G, H, A, 0xC19BF174, W[15]); 
    ROUND(A, B, C, D, E, F, G, H, 0xE49B69C1, W[16]);
    ROUND(H, A, B, C, D, E, F, G, 0xEFBE4786, W[17]);
    ROUND(G, H, A, B, C, D, E, F, 0x0FC19DC6, W[18]);
    ROUND(F, G, H, A, B, C, D, E, 0x240CA1CC, W[19]);
    ROUND(E, F, G, H, A, B, C, D, 0x2DE92C6F, W[20]);
    ROUND(D, E, F, G, H, A, B, C, 0x4A7484AA, W[21]);
    ROUND(C, D, E, F, G, H, A, B, 0x5CB0A9DC, W[22]);
    ROUND(B, C, D, E, F, G, H, A, 0x76F988DA, W[23]);
    ROUND(A, B, C, D, E, F, G, H, 0x983E5152, W[24]);
    ROUND(H, A, B, C, D, E, F, G, 0xA831C66D, W[25]);
    ROUND(G, H, A, B, C, D, E, F, 0xB00327C8, W[26]);
    ROUND(F, G, H, A, B, C, D, E, 0xBF597FC7, W[27]);
    ROUND(E, F, G, H, A, B, C, D, 0xC6E00BF3, W[28]);
    ROUND(D, E, F, G, H, A, B, C, 0xD5A79147, W[29]);
    ROUND(C, D, E, F, G, H, A, B, 0x06CA6351, W[30]);
    ROUND(B, C, D, E, F, G, H, A, 0x14292967, W[31]);
    ROUND(A, B, C, D, E, F, G, H, 0x27B70A85, W[32]);
    ROUND(H, A, B, C, D, E, F, G, 0x2E1B2138, W[33]);
    ROUND(G, H, A, B, C, D, E, F, 0x4D2C6DFC, W[34]);
    ROUND(F, G, H, A, B, C, D, E, 0x53380D13, W[35]);
    ROUND(E, F, G, H, A, B, C, D, 0x650A7354, W[36]);
    ROUND(D, E, F, G, H, A, B, C, 0x766A0ABB, W[37]);
    ROUND(C, D, E, F, G, H, A, B, 0x81C2C92E, W[38]);
    ROUND(B, C, D, E, F, G, H, A, 0x92722C85, W[39]);
    ROUND(A, B, C, D, E, F, G, H, 0xA2BFE8A1, W[40]);
    ROUND(H, A, B, C, D, E, F, G, 0xA81A664B, W[41]);
    ROUND(G, H, A, B, C, D, E, F, 0xC24B8B70, W[42]);
    ROUND(F, G, H, A, B, C, D, E, 0xC76C51A3, W[43]);
    ROUND(E, F, G, H, A, B, C, D, 0xD192E819, W[44]);
    ROUND(D, E, F, G, H, A, B, C, 0xD6990624, W[45]);
    ROUND(C, D, E, F, G, H, A, B, 0xF40E3585, W[46]);
    ROUND(B, C, D, E, F, G, H, A, 0x106AA070, W[47]);
    ROUND(A, B, C, D, E, F, G, H, 0x19A4C116, W[48]);
    ROUND(H, A, B, C, D, E, F, G, 0x1E376C08, W[49]);
    ROUND(G, H, A, B, C, D, E, F, 0x2748774C, W[50]);
    ROUND(F, G, H, A, B, C, D, E, 0x34B0BCB5, W[51]);
    ROUND(E, F, G, H, A, B, C, D, 0x391C0CB3, W[52]);
    ROUND(D, E, F, G, H, A, B, C, 0x4ED8AA4A, W[53]);
    ROUND(C, D, E, F, G, H, A, B, 0x5B9CCA4F, W[54]);
    ROUND(B, C, D, E, F, G, H, A, 0x682E6FF3, W[55]);
    ROUND(A, B, C, D, E, F, G, H, 0x748F82EE, W[56]);
    ROUND(H, A, B, C, D, E, F, G, 0x78A5636F, W[57]);
    ROUND(G, H, A, B, C, D, E, F, 0x84C87814, W[58]);
    ROUND(F, G, H, A, B, C, D, E, 0x8CC70208, W[59]);
    ROUND(E, F, G, H, A, B, C, D, 0x90BEFFFA, W[60]);
    ROUND(D, E, F, G, H, A, B, C, 0xA4506CEB, W[61]);
    ROUND(C, D, E, F, G, H, A, B, 0xBEF9A3F7, W[62]);
    ROUND(B, C, D, E, F, G, H, A, 0xC67178F2, W[63]);
  
    hash[0] += A;
    hash[1] += B;
    hash[2] += C;
    hash[3] += D;
    hash[4] += E;
    hash[5] += F;
    hash[6] += G;
    hash[7] += H;
}


inline void pdf_r5_sha256f(const uchar *msg, const int msgLen, uchar *hash) {
    uchar blk[64];
    uint H[8];
    int blkLen, i;

    H[0] = 0x6a09e667;
    H[1] = 0xbb67ae85;
    H[2] = 0x3c6ef372;
    H[3] = 0xa54ff53a;
    H[4] = 0x510e527f;
    H[5] = 0x9b05688c;
    H[6] = 0x1f83d9ab;
    H[7] = 0x5be0cd19;

    blkLen = msgLen;
    for (i = 0; i < blkLen; i++) {
        blk[i] = msg[i];
    }

    blk[blkLen++] = 0x80;

    while (blkLen < 56) {
        blk[blkLen++] = 0;
    }

    blk[56] = 0;
    blk[57] = 0;
    blk[58] = 0;
    blk[59] = 0;
    blk[60] = (uchar)(msgLen >> 21);
    blk[61] = (uchar)(msgLen >> 13);
    blk[62] = (uchar)(msgLen >> 5);
    blk[63] = (uchar)(msgLen << 3);
    pdf_r5_sha256HashBlock(blk, H);

    #pragma unroll
    for (i = 0; i < 8; ++i) {
        hash[i*4    ] = (uchar)(H[i] >> 24);
        hash[i*4 + 1] = (uchar)(H[i] >> 16);
        hash[i*4 + 2] = (uchar)(H[i] >> 8);
        hash[i*4 + 3] = (uchar)(H[i]);
    }
}


/**
 * @author Radek
 */
kernel void pdf_r5_kernel(global uchar* passwords, uchar max_len, global uchar *found_flag, global uint *found_vector, constant const uchar* U, constant const uchar* U_valid_salt) {
    
    int id = get_global_id(0);
    int lid = get_local_id(0);
    
    uchar my_pass_len = passwords[id*max_len];
    if (my_pass_len == 0 || my_pass_len > max_len) {
        // overflowed invalid password
        return;
    }
    uchar pass_buffer[PASS_BUFFER_SIZE];
    uchar sha256_hash[SHA256_HASH_SIZE];
    
    // Copy password to buffer
    for (int i = 0; i < my_pass_len; i++){
        pass_buffer[i] = passwords[id*max_len+1+i];
    }
    
    // Append password with User validation salt
    #pragma unroll
    for (int i = 0; i < U_VALID_SALT_SIZE; i++) {
        pass_buffer[my_pass_len + i] = U_valid_salt[i];
    }
    
    // Perform SHA256
    pdf_r5_sha256f(pass_buffer, my_pass_len + U_VALID_SALT_SIZE, sha256_hash);
    
    // Check SHA256 hash with U value
    for (uchar i = 0; i < SHA256_HASH_SIZE; i++){
        if(U[i] != sha256_hash[i])
            return;
    }
    
    *found_flag = 1;
    uint big_pos = id/32;
    uint small_pos = id%32;
    uint val = 0x80000000 >> small_pos;
    atomic_or(found_vector+big_pos,val);
}
