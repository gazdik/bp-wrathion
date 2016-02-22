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

#include "ZIPPKCrackerGPU.h"

ZIPPKCrackerGPU::ZIPPKCrackerGPU(std::vector<ZIPInitData> *data):ZIPPKCracker(data) {
    kernelFile = "kernels/zip_pk_kernel.cl";
    kernelName = "zip_pk_kernel";
}

ZIPPKCrackerGPU::ZIPPKCrackerGPU(const ZIPPKCrackerGPU& orig):ZIPPKCracker(orig) {
}

ZIPPKCrackerGPU::~ZIPPKCrackerGPU() {
}

bool ZIPPKCrackerGPU::initData() {
    int32_t files_count = data->size();
    lastCRCByte_buffer = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*files_count);
    crcTable_buffer = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(int)*256);
    randomStream_buffer = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*12*files_count);
    
    uint8_t *buffer = new uint8_t[12*files_count];
    for(int i = 0;i<data->size();i++){
        buffer[i] = (*data)[i].crc32 >> 24;
    }
    que.enqueueWriteBuffer(crcTable_buffer,CL_TRUE,0,sizeof(int)*256,crc32_table);
    que.enqueueWriteBuffer(lastCRCByte_buffer,CL_TRUE,0,sizeof(char)*files_count,buffer);
    for(int i = 0;i<files_count;i++){
        for(int j = 0;j<12;j++){
            buffer[i*12+j] = (*data)[i].streamBuffer[j];
        }
    }
    que.enqueueWriteBuffer(randomStream_buffer,CL_TRUE,0,sizeof(char)*12*files_count,buffer);
    kernel.setArg(userParamIndex, files_count);
    kernel.setArg(userParamIndex+1, randomStream_buffer);
    kernel.setArg(userParamIndex+2, lastCRCByte_buffer);
    kernel.setArg(userParamIndex+3, crcTable_buffer);
    delete[] buffer;
    return true;
}

void ZIPPKCrackerGPU::debugKernel(int clFinishRes) {
    uint8_t debug[64];
    que.enqueueReadBuffer(debug_buffer,CL_TRUE,0,sizeof(char)*64,debug);
    int t = 1;
}

void ZIPPKCrackerGPU::sharedDataInit() {
    createCRC32Table();
}


bool ZIPPKCrackerGPU::verifyPassword(std::string& pass) {
    ZIPKeys keys;
    initKeys(&keys,reinterpret_cast<const unsigned char*>(pass.c_str()),pass.length(),(*data)[0].streamBuffer);
    return verify(&keys,&(*data)[0]);
}



