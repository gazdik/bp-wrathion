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

#ifndef DOCRC4CRACKERCPU_H
#define	DOCRC4CRACKERCPU_H

#include "DOCFormat.h"

/**
 * RC4 State struct
 */
struct RC4State{
    uint8_t S[256];
    uint8_t i;
    uint8_t j;
};

/**
 * Class for CPU DOC cracking
 */
class DOCRC4CrackerCPU: public Cracker {
public:
    DOCRC4CrackerCPU(DOCInitData &data);
    DOCRC4CrackerCPU(const DOCRC4CrackerCPU& orig);
    virtual ~DOCRC4CrackerCPU();

    virtual CheckResult checkPassword(const std::string* password);

protected:
    /**
     * Calculate 1 block hash
     * @param input array of length between 0 and 55 bytes 
     * @param length length of input
     * @param digest result
     */
    void md5_1block(uint8_t *input, uint32_t length, uint8_t *digest);
    /**
     * Calculate 6 blocks hash
     * @param input array of length between 320 and 375 bytes
     * @param length length of input
     * @param digest result
     */
    void md5_6blocks(uint8_t *input, uint32_t length, uint8_t *digest);
    /**
     * Initializes RC4 state from key
     * @param rc4state state to init
     * @param key encrypt/decrypt key
     */
    void RC4Init128(RC4State *rc4state, uint8_t* key);
    /**
     * Process input data (encrypr/decrypt). Data is modified in-place 
     * @param rc4state state of RC4
     * @param data data to process
     * @param len  length of data
     */
    void RC4Process(RC4State *rc4state, uint8_t *data, uint32_t len);
    /**
     * Get next byte of RC4 keystream
     * @param rc4state State of RC4
     * @return next byte keystream 
     */
    uint8_t RC4KeystreamByte(RC4State *rc4state);
    /**
     * Convert 32bit integer to 4bytes array
     * @param val value to convert
     * @param bytes result
     */
    void to_bytes(uint32_t val, uint8_t *bytes);
    /**
     * Transform ASCII chars to UTF-16LE
     * @param str input ASCII string
     * @param unicodeBuffer output UTF-16LE string
     */
    void createU16(const std::string* str, uint8_t* unicodeBuffer);
private:
    DOCInitData data;
};

#endif	/* DOCRC4CRACKERCPU_H */

