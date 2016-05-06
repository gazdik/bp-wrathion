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

#include "GPUCracker.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <cmath>

GPUCracker::GPUCracker():GPUPassGenSeeded(false) {
    initOpenCL();
    passgenExhausted = false;
    localSize = cl::NDRange(64);
}

GPUCracker::GPUCracker(const GPUCracker& orig) {
}

GPUCracker::~GPUCracker() {
}

void GPUCracker::setDevice(DeviceConfig& config){
    this->deviceConfig = config;
}

void GPUCracker::initOpenCL(){
    if(init_done)
        return;
    cl::Platform::get(&platforms);
    if(platforms.size()==0){
        std::cout<<" No OpenCL platforms found. Check OpenCL installation!\n";
        init_done = true;
        return;
    }
    std::vector<cl::Device> all_devices;
    PlatformDevices p_devices;
    for(std::vector<cl::Platform>::iterator i = platforms.begin();i != platforms.end();i++){
        //std::cout << "Platform[]: "<< *i.getInfo<CL_PLATFORM_NAME>() << std::endl;
        (*i).getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
        p_devices.platform = *i;
        p_devices.devices.clear();
        for(std::vector<cl::Device>::iterator j = all_devices.begin();j != all_devices.end();j++){
            //std::cout << "Device[]: "<< (*j).getInfo<CL_DEVICE_NAME>() << std::endl;
            //std::cout << "Device[]: "<< (*j).getInfo<CL_DEVICE_ENDIAN_LITTLE>() << std::endl;
            p_devices.devices.push_back(*j);
        }
        devices.push_back(p_devices);
    }
    init_done = true;
}

void GPUCracker::destroyOpenCL() {
    for(std::vector<PlatformDevices>::iterator i = devices.begin();i != devices.end();i++){
        (*i).devices.clear();
    }
    devices.clear();
}


std::string GPUCracker::availableDevices(){
    initOpenCL();
    std::stringstream ss;
    int p = 0;
    for(std::vector<PlatformDevices>::iterator i = devices.begin();i != devices.end();i++){
        ss << "Platform[" << p++ << "]: "<< (*i).platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
        int d = 0;
        for(std::vector<cl::Device>::iterator j = (*i).devices.begin();j != (*i).devices.end();j++){
            ss << "\tDevice[" << d++ << "]: "<< (*j).getInfo<CL_DEVICE_NAME>() << std::endl;
        }
    }
    return ss.str();
}

void GPUCracker::preparePasswords(){
    if(!GPUPassGen){
        uint8_t entry_length = passgen->maxPassLen() + 1;
        uint32_t len;

        if (!passgen->getPassword(&passwdsToGPU[0*entry_length + 1], &len)) {
          passgenExhausted = true;
          return;
        }

        for(uint32_t i = 1; i< deviceConfig.globalWorkSize; i++){
            if(!passgen->getPassword(&passwdsToGPU[i*entry_length + 1],&len)){
                break;
            }
            passwdsToGPU[i*entry_length] = len & 0xFF;
        }
    } else{
        if(!passgen->nextKernelStep())
          passgenExhausted = true;
    }
}

void GPUCracker::updatePasswords() {
    if(!GPUPassGen){
        uint8_t entry_length = passgen->maxPassLen() + 1;
        que.enqueueWriteBuffer(passwordBuffer,CL_TRUE,0,sizeof(char)*entry_length*deviceConfig.globalWorkSize,passwdsToGPU);
    }else{
        que.enqueueNDRangeKernel(passgenKernel,cl::NullRange,cl::NDRange(deviceConfig.globalWorkSize),localSize);
    }
}


void GPUCracker::debugKernel(int clFinishRes) {

}

bool GPUCracker::verifyPassword(std::string &pass) {
    return true;
}


bool GPUCracker::initDevice(){
    device = devices[deviceConfig.platform].devices[deviceConfig.device];
    context = cl::Context(device);    
    
    // check passgen GPU support
    PassGen::KernelCode *passGpuCode = passgen->getKernelCode();
    if(passGpuCode != NULL){
        loadKernel(passGpuCode->filename, passGpuCode->name, &this->passgenKernel, &this->passgenProgram);
        GPUPassGen = true;
        passgen->setKernelGWS(deviceConfig.globalWorkSize);
    }
    
    que = cl::CommandQueue(context,device);
    
    loadKernel(kernelFile, kernelName, &this->kernel, &this->program);
    uint32_t max_passwds_len = deviceConfig.globalWorkSize*(passgen->maxPassLen()+1);
    passwdsToGPU = new char[max_passwds_len];
    passwdsFromGPU = new char[max_passwds_len];
    
    return true;
}

bool GPUCracker::loadKernel(std::string& filename, std::string& kernelName, cl::Kernel *kernel, cl::Program *program) {
    
    cl::Program::Sources sources;
    cl::Program::Binaries binaries;
    
    std::ifstream input_program;
    std::ofstream output_program;
    
    std::stringstream ss;
    bool loadBinary = false;
    
    ss << filename << "." << deviceConfig.platform << "_" << deviceConfig.device << ".clbin";
    
    std::string binrayFilename = ss.str();
    
    uint64_t binaryChangedTS = Utils::getFileModificationTS(binrayFilename);
    uint64_t sourceChangedTS = Utils::getFileModificationTS(filename);
    
    //loadBinary = binaryChangedTS > sourceChangedTS;
    
    // open compiled program if exists
    if(loadBinary){
        input_program.open(binrayFilename,std::ios::binary);
        std::string kernel_code((std::istreambuf_iterator<char>(input_program)),
                     std::istreambuf_iterator<char>());
        input_program.close();
        
        std::pair<const char*, ::size_t> binary;
        binary.first = kernel_code.c_str();
        binary.second = kernel_code.length();
        binaries.push_back(binary);
        
        std::vector<cl::Device> devices;
        devices.push_back(device);
        std::vector<cl_int> status;
        status.push_back(CL_INVALID_BINARY);
        cl_int err;
        *program = cl::Program(context,devices,binaries,&status,&err);
        if(status[0] != CL_SUCCESS){
            *kernel = NULL;
            return false;
        }
        
        if(program->build(devices)!=CL_SUCCESS){
            std::cerr<<" Error building: \n"<<(*program).getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)<< std::endl;
            *kernel = NULL;
            return false;
        }
        
    }else{
        input_program.open(filename);
        std::string kernel_code((std::istreambuf_iterator<char>(input_program)),
                     std::istreambuf_iterator<char>());
        input_program.close();

        std::pair<const char*, ::size_t> source;
        source.first = kernel_code.c_str();
        source.second = kernel_code.length();
        sources.push_back(source);

        *program = cl::Program(context,sources);

        std::vector<cl::Device> devices;
        devices.push_back(device);
        int build_res = program->build(devices);
        if(build_res !=CL_SUCCESS){
            std::cerr<<" Error building: \n"<<(*program).getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)<< std::endl;
            *kernel = NULL;
            return false ;
        }
        
        //save compiled program
        output_program.open(binrayFilename,std::ios::binary);
        uint32_t binary_size = program->getInfo<CL_PROGRAM_BINARY_SIZES>()[0];
        char* binary = program->getInfo<CL_PROGRAM_BINARIES>()[0];
        output_program.write(binary,binary_size);
        output_program.close();
        
    }
    *kernel = cl::Kernel(*program,kernelName.c_str());
    return true;
}


bool GPUCracker::passFound(){
    this->que.enqueueReadBuffer(this->foundFlagBuffer,CL_TRUE,0,sizeof(char),&foundFlag);
    return foundFlag == 1;
}

void GPUCracker::loadPositvePasswords(){
    uint32_t entry_length = passgen->maxPassLen() + 1;
    uint32_t bitmapSize = deviceConfig.globalWorkSize/32;
    this->que.enqueueReadBuffer(this->foundBitmapBuffer,CL_FALSE,0,sizeof(uint32_t)*bitmapSize,foundBitmap);
    this->que.enqueueReadBuffer(this->passwordBuffer,CL_TRUE,0,sizeof(char)*entry_length*deviceConfig.globalWorkSize,passwdsFromGPU);
    std::string pass;
    for(uint32_t i = 0;i<bitmapSize;i++){
        if(foundBitmap[i] > 0){
            uint32_t passpos = i*32;
            uint32_t val = foundBitmap[i];
            for(uint8_t j = 0;val > 0;j++){
                if((val & 0x80000000) != 0){
                    char* passptr = passwdsFromGPU+entry_length*(passpos+j);
                    pass.assign(passptr+1 , *passptr);
                    positivePasswords.push_back(pass);
                }
                val <<= 1;
            }
            foundBitmap[i] = 0;
        }
    }
    foundFlag = 0;
    que.enqueueWriteBuffer(foundFlagBuffer,CL_FALSE,0,sizeof(char),&foundFlag);
    que.enqueueWriteBuffer(foundBitmapBuffer,CL_FALSE,0,sizeof(uint32_t)*bitmapSize,foundBitmap);
}

bool GPUCracker::initCommonData(){
    
    foundFlag = 0;
    uint32_t bitmapSize = ceil(deviceConfig.globalWorkSize/32.0);
    foundBitmap = new uint32_t[bitmapSize];
    for(uint32_t i = 0;i<bitmapSize;i++){
        foundBitmap[i] = 0;
    }
    if(GPUPassGen)
        passwordBuffer = cl::Buffer(context,CL_MEM_READ_WRITE,sizeof(char)*deviceConfig.globalWorkSize*(passgen->maxPassLen()+1));
    else
        passwordBuffer = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(char)*deviceConfig.globalWorkSize*(passgen->maxPassLen()+1));
    
    foundFlagBuffer = cl::Buffer(context,CL_MEM_WRITE_ONLY,sizeof(char));
    foundBitmapBuffer = cl::Buffer(context,CL_MEM_WRITE_ONLY,sizeof(uint32_t)*bitmapSize);
    
    que.enqueueWriteBuffer(foundFlagBuffer,CL_FALSE,0,sizeof(char),&foundFlag);
    que.enqueueWriteBuffer(foundBitmapBuffer,CL_TRUE,0,sizeof(uint32_t)*bitmapSize,foundBitmap);
    
    cl_uchar pass_entry_length = passgen->maxPassLen() + PASS_EXTRA_BYTES;
//    cl_uchar pass_entry_length = passgen->maxPassLen();

    kernel.setArg(0,passwordBuffer);
    kernel.setArg(1,pass_entry_length);
    kernel.setArg(2,foundFlagBuffer);
    kernel.setArg(3,foundBitmapBuffer);
    userParamIndex = 4;

    if(GPUPassGen)
    {
      passgenKernel.setArg(0,passwordBuffer);
      passgenKernel.setArg(1,pass_entry_length);
      passgen->initKernel(&passgenKernel, &que, &context);
    }
    return true;
}

void GPUCracker::run(){
    if(devices.size() <= deviceConfig.platform){
        stopReason = PLATFORM_NOT_EXISTS;
        return;
    }
    if(devices[deviceConfig.platform].devices.size() <= deviceConfig.device){
        stopReason = DEVICE_NOT_EXISTS;
        return;
    }
    if(!initDevice()){
        stopReason = INTERNAL_ERROR;
        return;
    }
    if(!initCommonData()){
        stopReason = INTERNAL_ERROR;
        return;
    }
    if(!initData()){
        stopReason = INTERNAL_ERROR;
        return;
    }

  cl::NDRange globalSize = cl::NDRange(deviceConfig.globalWorkSize);
  pass_found = false;

  // Prepare passwords for first run
  preparePasswords();
  while (!pass_found && !stop_work)
  {
    // Update passwords in in kernel's buffer and run kernel
    updatePasswords();
    this->que.enqueueNDRangeKernel(this->kernel, cl::NullRange, globalSize,
                                   localSize);

    // Verify positive passwords from previous iteration
    if (!positivePasswords.empty())
    {
      for (std::vector<std::string>::iterator i = positivePasswords.begin();
          i != positivePasswords.end(); i++)
      {
        if (verifyPassword(*i))
        {
          pass_found = true;
          password.assign(*i);
          break;
        }
      }
      positivePasswords.clear();
    }

    // Stop if generator has stopped
    if (passgenExhausted)
      break;

    // Prepare passwords for next iteration
    preparePasswords();

    // Wait until the cracker's finished
    int res = this->que.finish();
#ifndef NDEBUG
    this->debugKernel(res);
#endif
    // Load positive passwords from kernel
    if (!pass_found && passFound())
    {
      loadPositvePasswords();
    }

    // Increment counter of cracked passwords
    passwdsCount += deviceConfig.globalWorkSize;
  }

  // Wait until a kernel's finished
  que.finish();

  if (pass_found)
    stopReason = PASS_FOUND;
  else if (passgenExhausted)
    stopReason = PASS_EXHAUSTED;
  else if (stop_work)
    stopReason = STOP_COMMAND;
}

bool GPUCracker::init_done = false;

std::vector<PlatformDevices> GPUCracker::devices;

std::vector<cl::Platform> GPUCracker::platforms;

