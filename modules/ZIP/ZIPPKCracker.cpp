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

#include "ZIPPKCracker.h"
#include <cstring>

ZIPPKCracker::ZIPPKCracker(std::vector<ZIPInitData> *data):data(data) {
    /* allocate inflate state */
    zlibStrm.zalloc = Z_NULL;
    zlibStrm.zfree = Z_NULL;
    zlibStrm.opaque = Z_NULL;
    zlibStrm.avail_in = 0;
    zlibStrm.next_in = Z_NULL;
    inflateInit2(&zlibStrm, -MAX_WBITS);
}

ZIPPKCracker::ZIPPKCracker(const ZIPPKCracker& orig) {
}

ZIPPKCracker::~ZIPPKCracker() {
    inflateEnd(&zlibStrm);
}

void ZIPPKCracker::updateKeys(ZIPKeys* keys, uint8_t c){
    keys->key0 = crc32(keys->key0,c);
    keys->key1 = keys->key1 + (keys->key0 & 0xFF);
    keys->key1 = keys->key1 * 0x08088405 + 1;
    keys->key2 = crc32(keys->key2,keys->key1 >> 24);
}

uint8_t ZIPPKCracker::initKeys(ZIPKeys* keys, const unsigned char* passwd, uint32_t len, uint8_t rndBuf[]){
    keys->key0 = 0x12345678;
    keys->key1 = 0x23456789;
    keys->key2 = 0x34567890;
    
    for(uint32_t i = 0;i<len;i++){
        updateKeys(keys,passwd[i]);
    }
    uint8_t C = 0;
    for(uint8_t i = 0;i<12;i++){
        C = rndBuf[i] ^ decryptByte(keys);
        updateKeys(keys,C);
    }
    return C;
}

uint8_t ZIPPKCracker::decryptByte(ZIPKeys* keys){
    uint16_t temp;
    temp = keys->key2 | 2;
    return (temp * (temp ^ 1)) >> 8;
}


uint32_t ZIPPKCracker::crc32( uint32_t crc, uint8_t c){
    crc = (crc >> 8) ^ crc32_table[(crc & 0xff) ^ c];
    return crc;
}

bool ZIPPKCracker::verify(ZIPKeys* keys, ZIPInitData *data) {
    
#define CHUNK 16384
    uint8_t uncopressed[CHUNK];
    uint8_t compressed[CHUNK];
    int chunk_no = 0; 
    int remain;
    int ret;
    do{
        remain = data->dataLen-chunk_no*CHUNK;
        int toDecompress = 0;
        if(remain > CHUNK){
            ::memcpy(compressed,data->encData+chunk_no*CHUNK,CHUNK);
            toDecompress = CHUNK;
        }else{
            ::memcpy(compressed,data->encData+chunk_no*CHUNK,remain);
            toDecompress = remain;
        }
        for(int i = 0;i<toDecompress;i++){
            uint8_t C = compressed[i];
            uint8_t t = C ^ decryptByte(keys);
            updateKeys(keys,t);
            compressed[i] = t;
        }
        zlibStrm.avail_in = toDecompress;
        zlibStrm.next_in = compressed;

        do {
            zlibStrm.avail_out = CHUNK;
            zlibStrm.next_out = uncopressed;
            ret = inflate(&zlibStrm, Z_SYNC_FLUSH);
            if(ret != Z_OK && ret != Z_STREAM_END){
                inflateReset(&zlibStrm);
                return false;
            }
        }while(zlibStrm.avail_out == 0);
        
        chunk_no++;
    }while(ret != Z_STREAM_END);
    bool result = zlibStrm.total_out == data->uncompressedSize;
    inflateReset(&zlibStrm);
    return result;
}


void ZIPPKCracker::createCRC32Table() {
    uint32_t rem;
    int i, j;
    /* Calculate CRC table. */
    for (i = 0; i < 256; i++) {
        rem = i;  /* remainder from polynomial division */
        for (j = 0; j < 8; j++) {
            if (rem & 1) {
                rem >>= 1;
                rem ^= 0xedb88320;
            } else
                rem >>= 1;
        }
        crc32_table[i] = rem;
    }
}

uint32_t ZIPPKCracker::crc32_table[256];
