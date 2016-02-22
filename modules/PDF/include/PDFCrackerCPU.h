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

#ifndef PDFCRACKERCPU_H
#define	PDFCRACKERCPU_H

#include <cstdint>
#include <string>

#include "Cracker.h"
#include "PDFFormat.h"
#include "PDFCracker.h"

/**
 * Class for PDF cracking on CPU 
 */
class PDFCrackerCPU:public Cracker, protected PDFCracker {
public:
    PDFCrackerCPU(PDFInitData &init);
    virtual ~PDFCrackerCPU();
protected:
    virtual CheckResult checkPassword(const std::string* password);
    virtual void sharedDataInit();
    virtual void sharedDataDestroy();

    static pthread_mutex_t passgenMutex;
    static pthread_mutexattr_t passgenMutexAttr;
};

#endif	/* PDFCRACKERCPU_H */

