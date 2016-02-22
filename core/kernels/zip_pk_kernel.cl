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

inline uint crc32(uint crc, uchar c, constant uint *table){
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ c];
    return crc;
}

inline void updateKeys(uint3 *keys, uchar c, constant uint *CRCTable){
    (*keys).x = crc32((*keys).x,c,CRCTable);
    (*keys).y = (*keys).y + ((*keys).x & 0xFF);
    (*keys).y = (*keys).y * 0x08088405 + 1;
    (*keys).z = crc32((*keys).z,(*keys).y >> 24,CRCTable);
}

inline uchar decryptByte(uint3 *keys){
    ushort temp;
    temp = (*keys).z | 2;
    return (temp * (temp ^ 1)) >> 8;
}

inline uchar initKeys(uint3 *keys, const uchar* pass, uint passlen, constant uchar *randomBuffer, constant uint *CRCTable){
    (*keys).x = 0x12345678;
    (*keys).y = 0x23456789;
    (*keys).z = 0x34567890;
    
    for(uint i = 0;i<passlen;i++){
        updateKeys(keys,pass[i],CRCTable);
    }
    uchar C = 0;
    for(uint i = 0;i<12;i++){
        C = randomBuffer[i] ^ decryptByte(keys);
        updateKeys(keys,C,CRCTable);
    }
    return C;
}

/**
 * @author Honza
 */
kernel void zip_pk_kernel(global uchar* passwords, uchar pass_len, global uchar *found_flag, global uint *found_vector, uint files_count, constant uchar *randomBuffer, constant uchar *lastCRCByte, constant uint *CRCTable) {
    int id = get_global_id(0);
    
    uchar my_pass_len = passwords[id*pass_len];
    uchar pass_buffer[32];
    uint3 keys;
    
    for(int i = 0;i<my_pass_len;i++){
        pass_buffer[i] = passwords[id*pass_len+1+i];
    }
    
    for(uint i = 0;i<files_count;i++){
        uchar lastb = initKeys(&keys,pass_buffer,my_pass_len,randomBuffer+(i*12),CRCTable);
        if(lastb != lastCRCByte[i]){
            return;
        }
    }
    
    *found_flag = 1;
    uint big_pos = id/32;
    uint small_pos = id%32;
    uint val = 0x80000000 >> small_pos;
    atomic_or(found_vector+big_pos,val);
}
