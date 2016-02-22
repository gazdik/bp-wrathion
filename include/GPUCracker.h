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

#ifndef GPUCRACKER_H
#define	GPUCRACKER_H

#include "Cracker.h"
#include <map>
#include <CL/cl.hpp>

struct PlatformDevices{
    cl::Platform platform;
    std::vector<cl::Device> devices;
};

struct DeviceConfig{
    uint32_t platform;
    uint32_t device;
    uint32_t globalWorkSize;
};

/**
 * Base class for GPU (OpenCL) Cracking
 */
class GPUCracker: public Cracker {
public:
    GPUCracker();
    GPUCracker(const GPUCracker& orig);
    virtual ~GPUCracker();
    /**
     * Returns formated string of all available OpenCL devices
     * @return 
     */
    static std::string availableDevices();
    /**
     * Destroys all references to OpenCL data and structs
     */
    static void destroyOpenCL();
    /**
     * Sets device for this cracker
     * @param config
     */
    void setDevice(DeviceConfig &config);
    virtual void run();
    
protected:
    /**
     * After initOpenCL is called this contains vector of platforms, its name and devices
     * @see initOpenCL
     */
    static std::vector<PlatformDevices> devices;
    static std::vector<cl::Platform> platforms;
    
    /**
     * Prepares passwords for update OpenCL device. Can be called while Kernel running.
     */
    virtual void preparePasswords();
    
    /**
     * Updates passwords in OpenCL device
     */
    virtual void updatePasswords();
    
    /**
     * Initializes device and loads kernels.
     * @return 
     */
    virtual bool initDevice();
    
    /**
     * Initializes data common for all crackers.
     * @return 
     */
    virtual bool initCommonData();
    
    /**
     * Pure virtual method for initialize cracker specific data.
     * @return true if all data initialised successfully
     */
    virtual bool initData() = 0;
    
    /**
     * Checks if password is realy useful passford or false postitive.
     * @note If cracker can generate false positive pass module MUST override this method. By default returns always true.
     * @param pass password to check
     * @return returns true if password is realy valid
     */
    virtual bool verifyPassword(std::string &pass);
    
    /**
     * Checks if at least one kernel found matching password
     * @return true if password is found
     */
    virtual bool passFound();
    
    /**
     * Used for kernel debugging. If framework is compiled without NDEBUG, this is called after each kernel run (clFinsih).
     * If compiled with NDEBUG, this method is never called.
     * @param clFinishRes result of clFinish function
     */
    virtual void debugKernel(int clFinishRes);
    
    /**
     * Loads all found passwords from OpenCL device into positivePasswords vector
     */
    virtual void loadPositvePasswords();
    
    /**
     * Loads kernel form source or binary file if exists. 
     * If binary version is out-of-date, compiles program from source and saves binary.
     * 
     * @param filename source code filename
     * @param kernelName name of kernel function
     * @param kernel loaded kernel
     * @param program loaded program
     * @return true if kernel was loaded succesully
     */
    virtual bool loadKernel(std::string& filename, std::string& kernelName, cl::Kernel *kernel, cl::Program *program);
    
    /**
     * Maximum length of password. Used to allocate mem in GPU
     */
    uint32_t maxPassLen;
    /**
     * Flag set if passgen is placed in GPU
     */
    bool GPUPassGen;
    /**
     * Flag set if initial passgen seed has been completed
     */
    bool GPUPassGenSeeded;
    /**
     * Next advance for passgen in GPU
     */
    uint64_t nextGPUPassGenStep;
    /**
     * Last advance for passgen in GPU
     */
    uint64_t lastGPUPassGenStep;
    
    /**
     * Mapped device
     */
    DeviceConfig deviceConfig;
    
    /**
     * Filename of kernel source code
     */
    std::string kernelFile;
    /**
     * Name of kernel function
     */
    std::string kernelName;
    
    /**
     * OpenCL Context
     */
    cl::Context context;
    /**
     * Device for cracking
     */
    cl::Device device;
    /**
     * Que for sending commands
     */
    cl::CommandQueue que;
    
    /**
     * Cracking program
     */
    cl::Program program;
    /**
     * Passgen program
     */
    cl::Program passgenProgram;
    
    /**
     * Cracking kernel
     */
    cl::Kernel kernel;
    /**
     * Passgen kernel
     */
    cl::Kernel passgenKernel;
    
    /**
     * Buffer for passwords
     */
    cl::Buffer passwordBuffer;
    /**
     * Buffer for global found flag
     */
    cl::Buffer foundFlagBuffer;
    /**
     * Buffer for found bitfield
     */
    cl::Buffer foundBitmapBuffer;
    
    /**
     * GWS
     */
    uint32_t chunkSize;
    
    /**
     * Task local size (can be set by initData())
     * @see initData
     */
    cl::NDRange localSize;
    
    /**
     * Pointer to array of password to be sent to GPU
     */
    char *passwdsToGPU;
    /**
     * Pointer to passwords loaded from GPU
     */
    char *passwdsFromGPU;
    /**
     * flag set if passgen was exhausted
     */
    bool passgenExhausted;
    /**
     * Index of kernel param from which user params can be passed. Usualy value of 4. Means user can use param index 4 or greater.
     */
    uint8_t userParamIndex;
    /**
     * Global found flag loaded from GPU
     */
    char foundFlag;
    /**
     * Bitfield found flag from GPU
     */
    uint32_t* foundBitmap;
    
    /**
     * Vector of positive passwords loaded from GPU based on found bitfield
     * @see foundBitmap
     */
    std::vector<std::string> positivePasswords;
    
private:
    /**
     * Initializes OpenCL platforms and devices
     * @see destroyOpenCL
     */
    static void initOpenCL();
    /**
     * Flag set if initOpenCL is invoked
     * @see initOpenCL
     */
    static bool init_done;
    
};

#endif	/* GPUCRACKER_H */

