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

#ifndef ZIPAESCRACKERCPU_H
#define	ZIPAESCRACKERCPU_H

#include "Cracker.h"
#include "ZIPFormat.h"
#include <pthread.h>


/**
 * Class for ZIP AES Cracking
 */
class ZIPAESCrackerCPU: public Cracker {
public:
    ZIPAESCrackerCPU(std::vector<ZIPInitData> *data);
    ZIPAESCrackerCPU(const ZIPAESCrackerCPU& orig);
    virtual ~ZIPAESCrackerCPU();
    //virtual void run();

    virtual CheckResult checkPassword(const std::string* password);

protected:
    /**
     * Calculate SHA1 hash of message
     * @param msg input to hash
     * @param len input legth
     * @param output result hash
     */
    void sha1(const uint8_t* msg,unsigned int len,uint8_t* output);
    /**
     * Calculate SHA1 hash of message up to two blocks (119 bytes)
     * @param msg input to hash
     * @param len input legth
     * @param output result hash
     */
    void sha1_fast(const uint8_t* msg,unsigned int len,uint8_t* output);
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
     * Calculate HMAC-SHA1 of message up to 1 block (55 bytes)
     * @param msg input to HMAC
     * @param msgLen input length
     * @param key key to auth
     * @param keyLen key length
     * @param output result HMAC
     */
    void hmac_sha1_fast(const uint8_t* msg,unsigned int msgLen, const uint8_t* key,unsigned int keyLen,uint8_t* output);
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
    /**
     * Create two verification bytes from PBKDF2 for 256bit keys
     * @param pass password
     * @param passLen password length
     * @param in_salt salt
     * @param output two verification bytes
     * @see pbkdf2_sha1_zip_aes192
     * @see pbkdf2_sha1_zip_aes128
     */
    void pbkdf2_sha1_zip_aes256(const uint8_t* pass, unsigned int passLen, const uint8_t* in_salt,uint8_t* output);
    /**
     * Create two verification bytes from PBKDF2 for 192bit keys
     * @param pass password
     * @param passLen password length
     * @param in_salt salt
     * @param output two verification bytes
     * @see pbkdf2_sha1_zip_aes256
     * @see pbkdf2_sha1_zip_aes128
     */
    void pbkdf2_sha1_zip_aes192(const uint8_t* pass, unsigned int passLen, const uint8_t* in_salt,uint8_t* output);
    /**
     * Create two verification bytes from PBKDF2 for 128bit keys
     * @param pass password
     * @param passLen password length
     * @param in_salt salt
     * @param output two verification bytes
     * @see pbkdf2_sha1_zip_aes256
     * @see pbkdf2_sha1_zip_aes192
     */
    void pbkdf2_sha1_zip_aes128(const uint8_t* pass, unsigned int passLen, const uint8_t* in_salt,uint8_t* output);
    
    std::vector<ZIPInitData> *data;
    ZIPInitData check_data;
};

#endif	/* ZIPAESCRACKERCPU_H */

