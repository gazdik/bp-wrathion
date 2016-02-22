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

#ifndef DOCFORMAT_H
#define	DOCFORMAT_H

#include "FileFormat.h"

struct DOCInitData{
    DOCInitData();
    DOCInitData(const DOCInitData& orig);
    
    uint8_t salt[16];
    uint8_t encVerifier[16];
    uint8_t encVerifierHash[16];
};

/**
 * MS-DOC FibBase struct (http://msdn.microsoft.com/en-us/library/dd944620(v=office.12).aspx)
 */
struct FibBase{
    uint16_t wIdent;
    uint16_t nFib;
    uint16_t unused;
    uint16_t lid;
    uint16_t pnNext;
    uint8_t fDot :1;
    uint8_t fGlsy :1;
    uint8_t fComplex :1;
    uint8_t fHasPic :1;
    uint8_t cQuickSaves :4;
    uint8_t fEncrypted :1;
    uint8_t fWhichTblStm :1;
    uint8_t fReadOnlyRecommended :1;
    uint8_t fWriteReservation :1;
    uint8_t fExtChar :1;
    uint8_t fLoadOverride :1;
    uint8_t fFarEast :1;
    uint8_t fObfuscated :1;
    uint16_t nFibBack;
    uint32_t lKey;
    uint8_t envr;
    uint8_t fMac :1;
    uint8_t fEmptySpecial :1;
    uint8_t fLoadOverridePage :1;
    uint8_t reserved1 :1;
    uint8_t reserved2 :1;
    uint8_t fSpare0 :3;
    uint16_t reserved3;
    uint16_t reserved4;
    uint32_t reserved5;
    uint32_t reserved6;
} __attribute__((gcc_struct)) __attribute__((packed));

/**
 * Encryption version
 */
struct Version{
    uint16_t vMajor;
    uint16_t vMinor;
};

/**
 * Encryption header (http://msdn.microsoft.com/en-us/library/dd908560(v=office.12).aspx)
 */
struct RC4EncryptionHeader{
    uint8_t salt[16];
    uint8_t encryptedVerifier[16];
    uint8_t encryptedVerifierHash[16];
};
/**
 * File format for reading DOC files
 */
class DOCFormat: public FileFormat {
public:
    DOCFormat();
    DOCFormat(const DOCFormat& orig);
    virtual ~DOCFormat();
    virtual CrackerFactory* getCPUCracker();
    virtual CrackerFactory* getGPUCracker();
    virtual void init(std::string& filename);

private:
    DOCInitData data;
};

#endif	/* DOCFORMAT_H */

