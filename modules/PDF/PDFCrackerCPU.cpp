/* 
 * Copyright (C) 2014 Jan Schmied, Radek Hranicky
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


#include <string.h>
#include <iostream>
#include <deque>
#include <iomanip>

#include "PDFCrackerCPU.h"

#include <sstream>

#include "sha256.h"

PDFCrackerCPU::PDFCrackerCPU(PDFInitData &init):Cracker(),PDFCracker(init) {
        
}

PDFCrackerCPU::~PDFCrackerCPU() {
}


pthread_mutex_t PDFCrackerCPU::passgenMutex;
pthread_mutexattr_t PDFCrackerCPU::passgenMutexAttr;

void PDFCrackerCPU::sharedDataInit(){
    pthread_mutexattr_init(&passgenMutexAttr);
    pthread_mutex_init(&passgenMutex,&passgenMutexAttr);
}

void PDFCrackerCPU::sharedDataDestroy(){
    pthread_mutex_destroy(&passgenMutex);
    pthread_mutexattr_destroy(&passgenMutexAttr);
}

CheckResult PDFCrackerCPU::checkPassword(const std::string* password) {
    uint8_t digest[16];
    uint8_t RC4_key[16];
    std::string input;
    int n;
    uint8_t crypt_input[16];
    uint8_t RC4_key_actual[16];
    
    input.assign(*password);

    if (data.R <= 4){
        /* Revision 1 to 4 */
        if(input.length() < 32){
            for(int i = 0; input.length() < 32;i++){
                input.append(1,passpad[i]);
            }
        }else if(input.length() > 32){
            input.resize(32);
        }

        input.append(data.O);
        input.append(reinterpret_cast<char*>(&data.P),4);
        input.append(data.ID1);
        if(data.R >= 4 && !data.MetaEncrypted){
            uint32_t max = 0xFFFFFFFF;
            input.append(reinterpret_cast<char*>(&max),4);
        }

        MD5(reinterpret_cast<const uint8_t*>(input.c_str()),input.length(), digest);
        n = data.length/8;
        if(data.R >= 3){
            for(int i = 0;i<50;i++){
                MD5( digest, n, digest);
            }
        }

        ::memcpy(RC4_key,digest,n);


        if (data.R == 2){
            RC4Init(RC4_key,n);
            ::memcpy(crypt_input,passpad,16);
            RC4Process(crypt_input,16);
        } else if(data.R == 3 || data.R == 4){
            input.assign((char*)passpad,32);
            input.append(data.ID1);
            MD5(reinterpret_cast<const uint8_t*>(input.c_str()),input.length(), digest);
            ::memcpy(crypt_input,digest,16);
            for(char i = 0;i<20;i++){
                for(int k = 0;k<n;k++){
                    RC4_key_actual[k] = RC4_key[k] ^ i;
                }
                RC4Init(RC4_key_actual,n);
                RC4Process(crypt_input,16);;
            }        
        }
        if(::memcmp(data.U.c_str(),crypt_input,16) == 0){
            return CR_PASSWORD_MATCH;
        }
        
    } else if(data.R == 5) {
        /* PDF Extension level 3 */
        input.append((const char *)data.U_valid_salt, 8);             // SHA input = password + user valid salt
        uint8_t hash[32];                                             // prepare the hash
        sha256f((const uint8_t *)input.c_str(), input.size(), hash);  // compute fast SHA256
        if(::memcmp(data.U.c_str(),hash,32) == 0){                    // compare the HASH with first 32B of U
            return CR_PASSWORD_MATCH;
        }
    } else if(data.R == 6) {
        /* PDF Extension level 8 */
        // ISO 32000-2  *** !!! TODO - Implement the password verification algorithm !!! ***
        //
    } else if(data.R > 6) {
        /* Unsupported (currently non-existing) encryption revision */
    }
    return CR_PASSWORD_WRONG;
}
