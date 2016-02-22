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

#ifndef ZIPPKCRACKER_H
#define	ZIPPKCRACKER_H

#include "ZIPFormat.h"
#include <zlib.h>

/**
 * Base class for Cracking PKZIP Stream Cipher
 */
class ZIPPKCracker {
public:
    ZIPPKCracker(std::vector<ZIPInitData> *data);
    ZIPPKCracker(const ZIPPKCracker& orig);
    virtual ~ZIPPKCracker();
    /**
     * PKZIP Stream Cipher keys
     */
    struct ZIPKeys{
        uint32_t key0;
        uint32_t key1;
        uint32_t key2;
    };
    
protected:
    /**
     * Update keys (more in ZIP APPNOTE)
     * @param keys
     * @param c
     */
    void updateKeys(ZIPKeys* keys, uint8_t c);
    /**
     * Initialize PKZIP Stream Cipher
     * @param keys 
     * @param passwd password
     * @param len password length
     * @param rndBuf first 12 bytes of data stream
     * @return the highest byte of encrypted file CRC
     */
    uint8_t initKeys(ZIPKeys* keys, const unsigned char* passwd, uint32_t len, uint8_t rndBuf[]);
    /**
     * Returns next byte in keystream
     * @param keys
     * @return 
     */
    uint8_t decryptByte(ZIPKeys* keys);
    /**
     * Update current crc after byte is added
     * @param inCrc32 current CRC32
     * @param buf byte to add
     * @return updated CRC32
     */
    uint32_t crc32( uint32_t inCrc32, uint8_t buf);
    /**
     * Build crc32 table
     */
    void createCRC32Table();
    
    /**
     * Verifies if current keys are correct
     * @param keys keyst to check
     * @param data encrypted data stream
     * @return true if file was decrypted and decopressed correctly
     */
    bool verify(ZIPKeys* keys, ZIPInitData *data);
    
    /**
     * Vector of data stream
     */
    std::vector<ZIPInitData> *data;
    
    /**
     * Buffer for decrypted data
     */
    uint8_t *decryptBuffer;
    /**
     * Struct for ZLIB
     */
    z_stream zlibStrm;
    
    /**
     * CRC32 table
     */
    static uint32_t crc32_table[256];

};

#endif	/* ZIPPKCRACKER_H */

