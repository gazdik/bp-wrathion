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

#ifndef ZIPAESCRACKERGPU_H
#define	ZIPAESCRACKERGPU_H

#include "GPUCracker.h"
#include "ZIPFormat.h"
#include <vector>


class ZIPAESCrackerGPU: public GPUCracker {
public:
    ZIPAESCrackerGPU(std::vector<ZIPInitData> *data);
    ZIPAESCrackerGPU(const ZIPAESCrackerGPU& orig);
    virtual ~ZIPAESCrackerGPU();

    virtual bool initData();
    virtual bool verifyPassword(std::string& pass);
protected:
    /**
     * Calculate HMAC-SHA1 of message
     * @param msg input to HMAC
     * @param msgLen input length
     * @param key key to auth
     * @param keyLen key length
     * @param output result HMAC
     */
    void hmac_sha1(const uint8_t* msg,unsigned int msgLen, const uint8_t* key,unsigned int keyLen,uint8_t* output);
    /**
     * Calculate SHA1 hash of message
     * @param msg input to hash
     * @param len input legth
     * @param output result hash
     */
    void sha1(const uint8_t* msg,unsigned int len,uint8_t* output);
    /**
     * Create key using PBKDF2 method
     * @param pass password
     * @param passLen passwoed length
     * @param in_salt salt
     * @param saltLen salt length
     * @param iterations number of iterations
     * @param dkLen desired output length
     * @param output result key
     */
    void pbkdf2_sha1(const uint8_t* pass, unsigned int passLen, const uint8_t* in_salt,unsigned int saltLen, unsigned int iterations, unsigned int dkLen, uint8_t* output);
    ZIPInitData data;
    
    cl::Buffer salt_buffer;
    cl::Buffer verifier_buffer;
};

#endif	/* ZIPAESCRACKERGPU_H */

