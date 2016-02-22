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

#include "PDFCrackerGPU.h"
#include "PDFFormat.h"
#include <iostream>

PDFCrackerGPU::PDFCrackerGPU(PDFInitData &init):GPUCracker(),PDFCracker(init) {
    if (data.R <= 4) {
        kernelFile = "kernels/pdf_r3_kernel.cl";
        kernelName = "pdf_r3_kernel";
    } else {
        kernelFile = "kernels/pdf_r5_kernel.cl";
        kernelName = "pdf_r5_kernel";
    }
}

PDFCrackerGPU::~PDFCrackerGPU() {
}

bool PDFCrackerGPU::verifyPassword(std::string password){
    return true;
}

bool PDFCrackerGPU::initData(){
        uint8_t pad_ID1[48];
        uint8_t pad_ID1_digest[16];
        char passpadding[52];
        
    if (data.R <= 4) {
        memcpy(pad_ID1,passpad,32);
        memcpy(pad_ID1+32,data.ID1.c_str(),16);
        MD5(pad_ID1,48,pad_ID1_digest);
        
        memcpy(passpadding,data.O.c_str(),32);
        memcpy(passpadding+32,&data.P,4);
        memcpy(passpadding+36,data.ID1.c_str(),16);
        
        buffer_U = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*16);
        buffer_Passpadding = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*52);
        buffer_Padhash = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*16);
        
        que.enqueueWriteBuffer(buffer_U,CL_TRUE,0,sizeof(char)*16,data.U.c_str());
        que.enqueueWriteBuffer(buffer_Passpadding,CL_TRUE,0,sizeof(char)*52,passpadding);
        que.enqueueWriteBuffer(buffer_Padhash,CL_TRUE,0,sizeof(char)*16,pad_ID1_digest);
        
        kernel.setArg(userParamIndex,buffer_U);
        kernel.setArg(userParamIndex+1,data.length/8);
        kernel.setArg(userParamIndex+2,buffer_Passpadding);
        kernel.setArg(userParamIndex+3,buffer_Padhash);
    } else {
        buffer_U = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*32);
        buffer_U_valid_salt = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*8);
        
        que.enqueueWriteBuffer(buffer_U,CL_TRUE,0,sizeof(char)*32,data.U.c_str());
        que.enqueueWriteBuffer(buffer_U_valid_salt,CL_TRUE,0,sizeof(char)*8,data.U_valid_salt);
        
        kernel.setArg(userParamIndex,buffer_U);
        kernel.setArg(userParamIndex+1,buffer_U_valid_salt);
    }
    
    return true;
}



