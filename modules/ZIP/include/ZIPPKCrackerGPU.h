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

#ifndef ZIPPKCRACKERGPU_H
#define	ZIPPKCRACKERGPU_H

#include "ZIPFormat.h"
#include "GPUCracker.h"
#include "ZIPPKCracker.h"

/**
 * Class for PKZIP Stream Cipher cracking on GPU
 */
class ZIPPKCrackerGPU: public GPUCracker, protected ZIPPKCracker {
public:
    ZIPPKCrackerGPU(std::vector<ZIPInitData> *data);
    ZIPPKCrackerGPU(const ZIPPKCrackerGPU& orig);
    virtual ~ZIPPKCrackerGPU();
    
    virtual void sharedDataInit();

    virtual bool verifyPassword(std::string& pass);
    virtual bool initData();
    

    virtual void debugKernel(int clFinishRes);


private:
    cl::Buffer crcTable_buffer;
    cl::Buffer lastCRCByte_buffer;
    cl::Buffer randomStream_buffer;
    cl::Buffer debug_buffer;

};

#endif	/* ZIPPKCRACKERGPU_H */

