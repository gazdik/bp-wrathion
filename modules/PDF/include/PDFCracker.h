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

#ifndef PDFCRACKER_H
#define	PDFCRACKER_H

#define	V_256 0
#define	V_384 1
#define	V_512 2

#include <stdint.h>
#include "PDFFormat.h"

struct RC4State{
    uint8_t S[256];
    uint8_t i;
    uint8_t j;
};

/**
 * Base class for PDF cracking.
 */
class PDFCracker {
public:
    PDFCracker(PDFInitData &data);
    PDFCracker(const PDFCracker& orig);
    virtual ~PDFCracker();
    /**
     * Init internal RC4 State
     * @param key key for encrypt/decrypt
     * @param keylen length of key
     */
    void RC4Init(uint8_t* key, uint16_t keylen);
    /**
     * Encrypt/decrypt data
     * @param data data to process (in/out)
     * @param len length of data
     */
    void RC4Process(uint8_t *data, uint32_t len);
    /**
     * Calculate MD5 hash
     * @param data input to hash
     * @param len length of input
     * @param res result hash
     */
    void MD5(const uint8_t *data,uint32_t len,uint8_t *res);
    std::string sha2(int type, std::string input);
protected:
    /**
     * Value used for password pad
     */
    static const uint8_t passpad[];
    PDFInitData data;
    RC4State rc4state;

};

#endif	/* PDFCRACKER_H */

