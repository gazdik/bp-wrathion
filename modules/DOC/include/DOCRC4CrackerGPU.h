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

#ifndef DOCRC4CRACKERGPU_H
#define	DOCRC4CRACKERGPU_H

#include "DOCFormat.h"
#include "GPUCracker.h"
/**
 * Class for MS DOC GPU Cracking
 */
class DOCRC4CrackerGPU: public GPUCracker {
public:
    DOCRC4CrackerGPU(DOCInitData &data);
    DOCRC4CrackerGPU(const DOCRC4CrackerGPU& orig);
    virtual ~DOCRC4CrackerGPU();
    virtual bool initData();
protected:

    virtual void debugKernel(int clFinishRes);

private:
    DOCInitData data;
    cl::Buffer buffer_salt;
    cl::Buffer buffer_encVerifier;
    cl::Buffer buffer_encVerifierHash;
    cl::Buffer buffer_debug;
};

#endif	/* DOCRC4CRACKERGPU_H */

