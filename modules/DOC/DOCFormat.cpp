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

#include "DOCFormat.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "CrackerFactoryTemplate.tcc"
#include "DOCRC4CrackerCPU.h"
#include "DOCRC4CrackerGPU.h"

typedef CrackerFactoryTemplate<DOCRC4CrackerCPU,DOCInitData> DOCRC4CrackerCPUFactory;
typedef CrackerFactoryTemplate<DOCRC4CrackerGPU,DOCInitData,true> DOCRC4CrackerGPUFactory;

DOCFormat::DOCFormat() {
    signature = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1";
    ext = "doc";
    name = "MS WORD DOC";
}

DOCFormat::DOCFormat(const DOCFormat& orig) {
}

DOCFormat::~DOCFormat() {
}

CrackerFactory* DOCFormat::getCPUCracker() {
    return new DOCRC4CrackerCPUFactory(data);
}

CrackerFactory* DOCFormat::getGPUCracker() {
    return new DOCRC4CrackerGPUFactory(data);
}

void DOCFormat::init(std::string& filename) {
    FibBase fibBase;
    std::ifstream filestream(filename,std::ios::binary);
    uint16_t sectorSize=1;
    uint32_t miniStrMax=0;
    uint32_t firstDirSect=0;
    char16_t wideBuffer[32];
    
    //read sector size
    filestream.seekg(0x1e,std::ios::beg);
    filestream.read(reinterpret_cast<char *>(&sectorSize),sizeof(uint16_t));
    sectorSize = 1 << sectorSize;
    
    //directory sector
    filestream.seekg(0x30,std::ios::beg);
    filestream.read(reinterpret_cast<char *>(&firstDirSect),sizeof(uint32_t));
    
    filestream.seekg(0x38,std::ios::beg);
    filestream.read(reinterpret_cast<char *>(&miniStrMax),sizeof(uint32_t));
    
    filestream.seekg(sectorSize,std::ios::beg);//skip MS OLE header
    filestream.read(reinterpret_cast<char *>(&fibBase),sizeof(fibBase));
    if(!fibBase.fEncrypted){
        is_encrypted = false;
        return;
    }
    is_encrypted = true;
    //seek to directory sector
    filestream.seekg((firstDirSect+1)*sectorSize,std::ios::beg);
    
    uint32_t tableSector = 0;
    std::u16string dirName;
    std::u16string tableName = u"0Table";
    if(fibBase.fWhichTblStm){
        tableName[0] = u'1';
    }
    int dirNameLen = 0;
    do{
        filestream.read(reinterpret_cast<char *>(wideBuffer),32*sizeof(char16_t));
        dirNameLen = std::char_traits<char16_t>::length(wideBuffer);
        dirName.assign(wideBuffer,dirNameLen);
        if(dirName == tableName){
            //seek to startsector entry
            filestream.seekg(0x74-64,std::ios::cur);
            filestream.read(reinterpret_cast<char *>(&tableSector),sizeof(uint32_t));
            break;
        }
        filestream.seekg(128-64,std::ios::cur);
    }while(dirNameLen > 0);
    
    if(tableSector == 0){
        std::cout << "Unable to locate data Table" << std::endl; 
        return;
    }
    
    //seek to stream table
    filestream.seekg((tableSector+1)*sectorSize,std::ios::beg);
    
    //read encryption version
    Version ver;
    filestream.read(reinterpret_cast<char *>(&ver),sizeof(ver));
    if (verbose) {
        // Print ZIP encryption information obtained from the file
        std::cout << "======= DOC information =======" << std::endl;
        std::cout << "Version major: " << (int)(ver.vMajor) << std::endl;
        std::cout << "Version minor: " << (int)(ver.vMinor) << std::endl;
        if (ver.vMajor == 1 && ver.vMinor == 1) {
            std::cout << "Algorithm: 40-bit RC4" << std::endl;
        }
        std::cout << "===============================" << std::endl;
    }
    if(ver.vMajor != 1 || ver.vMinor != 1 ){
        std::cout << "Usupported encryption version" << std::endl;
        return;
    }
    is_supported = true;
    RC4EncryptionHeader encHeader;
    filestream.read(reinterpret_cast<char *>(&encHeader),sizeof(encHeader));
    ::memcpy(data.salt,encHeader.salt,16);
    ::memcpy(data.encVerifier,encHeader.encryptedVerifier,16);
    ::memcpy(data.encVerifierHash,encHeader.encryptedVerifierHash,16);
    
}

DOCInitData::DOCInitData(){
    
}

DOCInitData::DOCInitData(const DOCInitData& orig) {
    ::memcpy(this->salt,orig.salt,16);
    ::memcpy(this->encVerifier,orig.encVerifier,16);
    ::memcpy(this->encVerifierHash,orig.encVerifierHash,16);
}


