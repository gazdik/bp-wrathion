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

#include "ZIPFormat.h"
#include "ZIPAESCrackerCPU.h"
#include "ZIPAESCrackerGPU.h"
#include "ZIPPKCrackerCPU.h"
#include "ZIPPKCrackerGPU.h"
#include "CrackerFactoryTemplate.tcc"
#include <fstream>
#include <cstring>
#include <iostream>

typedef CrackerFactoryTemplate<ZIPAESCrackerCPU,std::vector<ZIPInitData>*> ZIPAESCrackerCPUFactory;
typedef CrackerFactoryTemplate<ZIPPKCrackerCPU,std::vector<ZIPInitData>*> ZIPPKCrackerCPUFactory;

typedef CrackerFactoryTemplate<ZIPPKCrackerGPU,std::vector<ZIPInitData>*, true> ZIPPKCrackerGPUFactory;
typedef CrackerFactoryTemplate<ZIPAESCrackerGPU,std::vector<ZIPInitData>*, true> ZIPAESCrackerGPUFactory;

ZIPFormat::ZIPFormat() {
    signature = "PK\x03\x04";
    ext = "zip";
    name = "ZIP";
    
    /*
     * We are unable to detect if ZIP format is encrypted,
     * so we suppose, it is.
     */
    is_encrypted = true;
}

ZIPFormat::ZIPFormat(const ZIPFormat& orig):FileFormat(orig) {
}

ZIPFormat::~ZIPFormat() {
}

ZIPInitData ZIPFormat::readOneFile(std::ifstream *stream) {
    uint16_t ext_fields_len,filename_len,flags;
    uint32_t compressed_size;
    ZIPInitData data;
    stream->seekg(6,stream->cur); // skip signature, min version for decompress
    stream->read(reinterpret_cast<char*>(&flags),sizeof(uint16_t));
    stream->read(reinterpret_cast<char*>(&data.compression),sizeof(uint16_t));
    stream->seekg(4,stream->cur); // skip date and time
    stream->read(reinterpret_cast<char*>(&data.crc32),sizeof(uint32_t));
    stream->read(reinterpret_cast<char*>(&compressed_size),sizeof(uint32_t));
    stream->read(reinterpret_cast<char*>(&data.uncompressedSize),sizeof(uint32_t));
    stream->read(reinterpret_cast<char*>(&filename_len),sizeof(uint16_t));
    stream->read(reinterpret_cast<char*>(&ext_fields_len),sizeof(uint16_t));
    stream->seekg(filename_len,stream->cur); // skip filename
    
    if(!(flags & 0x1)){ // check encryption flag not set
        data.type = NONE;
    }else{
        if(data.compression == 99){ // AES
            data.type = AES;
            uint16_t len = ext_fields_len;
            while(len > 0){
                uint16_t ext_type;
                uint16_t ext_len;
                stream->read(reinterpret_cast<char*>(&ext_type),sizeof(uint16_t));
                stream->read(reinterpret_cast<char*>(&ext_len),sizeof(uint16_t));
                if(ext_type == 0x9901){
                    stream->seekg(4,stream->cur); // skip 4 boring bytes
                    uint8_t temp;
                    stream->read(reinterpret_cast<char*>(&temp),sizeof(uint8_t));
                    switch(temp){
                        case 1:
                            data.keyLength = 128;
                            data.saltLen = 8;
                            break;
                        case 2:
                            data.keyLength = 192;
                            data.saltLen = 12;
                            break;
                        case 3:
                            data.keyLength = 256;
                            data.saltLen = 16;
                            break;
                    }
                    // 2 bytes of actual compression method
                    stream->seekg(2,stream->cur);
                }else{
                    // skip unknown extension
                    stream->seekg(ext_len,stream->cur);
                }
                len-=ext_len+4;
            }
            
        }else{
            data.type = PKSTREAM;
            uint16_t len = ext_fields_len;
            while(len > 0){
                uint16_t ext_len;
                stream->seekg(2,stream->cur);
                stream->read(reinterpret_cast<char*>(&ext_len),sizeof(uint16_t));
                stream->seekg(ext_len,stream->cur);
                len-=ext_len+4;
            }
        }
        if(data.type == AES){
            stream->read(reinterpret_cast<char*>(&data.salt),data.saltLen);
            stream->read(reinterpret_cast<char*>(&data.verifier),sizeof(uint8_t)*2);
            data.dataLen = compressed_size-data.saltLen-2-10;
            data.encData = new uint8_t[data.dataLen];
            stream->read(reinterpret_cast<char*>(data.encData),data.dataLen);
            stream->read(reinterpret_cast<char*>(data.authCode),10);
        }else if(data.type == PKSTREAM){
            stream->read(reinterpret_cast<char*>(data.streamBuffer),12);
            data.dataLen = compressed_size-12;
            data.encData = new uint8_t[data.dataLen];
            stream->read(reinterpret_cast<char*>(data.encData),data.dataLen);
        }
    }
    return data;
}


void ZIPFormat::init(std::string& filename){
    std::ifstream stream;
    stream.open(filename,std::ios_base::binary);
    char buffer[4];
    do{
        ZIPInitData filedata = readOneFile(&stream);
        stream.read(reinterpret_cast<char*>(buffer),4);
        data.push_back(filedata);
        filterData();
        if(::memcmp(buffer,"PK\x03\x04",4) == 0){
            stream.seekg(-4,stream.cur);
        }else{
           break;
        }
    }while(true);
    if (verbose) {
        // Print ZIP encryption information obtained from the file
        std::cout << "======= ZIP information =======" << std::endl;
        if (data[0].type == AES) {
            std::cout << "Encryption method: AES" << std::endl;
            std::cout << "Key length: " << (int)(data[0].keyLength) << std::endl;
            std::cout << "Salt length: " << (int)(data[0].saltLen) << std::endl;
        } else if (data[0].type == PKSTREAM) {
            std::cout << "Encryption method: ZIP 2.0 (Legacy) PKZIP" << std::endl;
            
        } else {
            std::cout << "Encryption method is currently not supported by Wrathion." << std::endl;
        }
        std::cout << "===============================" << std::endl;
    }
    stream.close();
    is_supported = true;
}

void ZIPFormat::filterData() {
    if(data.size() < 2)
        return;
    
    uint32_t minSize = UINT32_MAX;
    uint32_t minIndex = 0;
    for(int i = 0;i<data.size();i++){
        if(data[i].dataLen > 0 && minSize > data[i].dataLen){
            minSize = data[i].dataLen;
            minIndex = i;
        }
    }
    
    for(int i = 0;i<data.size();i++){
        if(i == minIndex)
            continue;
        
        data[i].dataLen = 0;
        delete[] data[i].encData;
        data[i].encData = NULL;
    }
}


CrackerFactory* ZIPFormat::getCPUCracker(){
    switch(data[0].type){
        case AES: return new ZIPAESCrackerCPUFactory(&data);
        case PKSTREAM: 
            if(data[0].compression == 8){ // cracking only DEFLATE
                return new ZIPPKCrackerCPUFactory(&data);
            }else
                return NULL;
        default: return NULL;
    }
}

CrackerFactory* ZIPFormat::getGPUCracker(){
    switch(data[0].type){
        case AES: return new ZIPAESCrackerGPUFactory(&data);
        case PKSTREAM: 
            if(data[0].compression == 8){ // cracking only DEFLATE
                return new ZIPPKCrackerGPUFactory(&data);
            }else
                return NULL;
        default: return NULL;
    }
}

ZIPInitData::ZIPInitData(){
    
}

ZIPInitData::ZIPInitData(const ZIPInitData& orig){
    this->crc32 = orig.crc32;
    this->keyLength = orig.keyLength;
    this->type = orig.type;
    ::memcpy(this->verifier,orig.verifier,2);
    this->saltLen = orig.saltLen;
    ::memcpy(this->salt,orig.salt,16);
    ::memcpy(this->authCode,orig.authCode,10);
    ::memcpy(this->streamBuffer,orig.streamBuffer,12);
    this->dataLen = orig.dataLen;
    this->encData = orig.encData;
    this->uncompressedSize = orig.uncompressedSize;
    this->compression = orig.compression;
}

