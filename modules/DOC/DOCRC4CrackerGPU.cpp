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

#include "DOCRC4CrackerGPU.h"
#include <iostream>
#include <iomanip>

DOCRC4CrackerGPU::DOCRC4CrackerGPU(DOCInitData &data):data(data) {
    kernelFile = "kernels/doc_rc4_kernel.cl";
    kernelName = "doc_rc4_kernel";
}

DOCRC4CrackerGPU::DOCRC4CrackerGPU(const DOCRC4CrackerGPU& orig) {
}

DOCRC4CrackerGPU::~DOCRC4CrackerGPU() {
}

bool DOCRC4CrackerGPU::initData() {
    
    buffer_salt = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*16);
    buffer_encVerifier = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*16);
    buffer_encVerifierHash = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*16);
    
    que.enqueueWriteBuffer(buffer_salt,CL_TRUE,0,sizeof(char)*16,data.salt);
    que.enqueueWriteBuffer(buffer_encVerifier,CL_TRUE,0,sizeof(char)*16,data.encVerifier);
    que.enqueueWriteBuffer(buffer_encVerifierHash,CL_TRUE,0,sizeof(char)*16,data.encVerifierHash);
    
    kernel.setArg(userParamIndex,buffer_salt);
    kernel.setArg(userParamIndex+1,buffer_encVerifier);
    kernel.setArg(userParamIndex+2,buffer_encVerifierHash);
    return true;
}

void DOCRC4CrackerGPU::debugKernel(int clFinishRes) {
}





