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

#include "ZIPPKCrackerCPU.h"
#include <string.h>

ZIPPKCrackerCPU::ZIPPKCrackerCPU(std::vector<ZIPInitData> *data):ZIPPKCracker(data) {
    files_count = data->size();
    high_CRC = new uint8_t[files_count];
    for(uint32_t i = 0;i<files_count;i++){
        high_CRC[i] = (*data)[i].crc32 >> 24;
        if((*data)[i].dataLen > 0){
            verify_data = &(*data)[i];
        }
    }
}

ZIPPKCrackerCPU::ZIPPKCrackerCPU(const ZIPPKCrackerCPU& orig):ZIPPKCracker(orig) {
}

ZIPPKCrackerCPU::~ZIPPKCrackerCPU() {
    delete high_CRC;
}

CheckResult ZIPPKCrackerCPU::checkPassword(const std::string* password) {
    ZIPKeys keys;
    for(uint32_t i = 0;i< files_count;i++){
        uint8_t lastb = initKeys(&keys,reinterpret_cast<const unsigned char*>(password->c_str()),password->length(),(*data)[i].streamBuffer);
        if(lastb != high_CRC[i]){
            break;
        }else if(i == files_count-1){
            initKeys(&keys,reinterpret_cast<const unsigned char*>(password->c_str()),password->length(),verify_data->streamBuffer);
            if(verify(&keys,verify_data)){
                return CR_PASSWORD_MATCH;
            }
        }
    }
    return CR_PASSWORD_WRONG;
}

void ZIPPKCrackerCPU::sharedDataInit() {
    createCRC32Table();

}

