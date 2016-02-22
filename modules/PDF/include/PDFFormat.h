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

#ifndef PDFFORMAT_H
#define	PDFFORMAT_H

#include <string>

#include "FileFormat.h"

struct PDFInitData{
    PDFInitData();
    PDFInitData(const PDFInitData &orig);
    
    std::string ID1;
    std::string ID2;
    std::string O;
    uint8_t O_valid_salt[8];
    uint8_t O_key_salt[8];
    std::string U;
    uint8_t U_valid_salt[8];
    uint8_t U_key_salt[8];
    int32_t length;
    uint32_t P;
    int8_t R;
    int8_t V;
    bool MetaEncrypted;
};

/**
 * Class for reading PDF files
 */
class PDFFormat: public FileFormat {
public:
    PDFFormat();
    PDFFormat(const PDFFormat& orig);
    virtual ~PDFFormat();
    virtual void init(std::string& filename);
    virtual CrackerFactory* getCPUCracker();
    virtual CrackerFactory* getGPUCracker();
private:
    PDFInitData data;
};

#endif	/* PDFFORMAT_H */

