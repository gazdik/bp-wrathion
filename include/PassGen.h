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

#ifndef PASSGEN_H
#define	PASSGEN_H

#include <string>
#include <fstream>
#include <map>
#include <CL/cl.hpp>


const unsigned MIN_PASS_LENGTH = 1;
const unsigned MAX_PASS_LENGTH = 64;
const unsigned ASCII_CHARSET_SIZE = 256;


/**
 * ID of password generator saved in state file
 */
enum PassGenID{
    PASSGEN_ID_UNKNOWN = 0,
    PASSGEN_ID_DICTIONARY = 1,
    PASSGEN_ID_BRUTE = 2,
    PASSGEN_ID_THREADED_BRUTE = 3,
    PASSGEN_ID_MARKOV = 4,
};

/**
 * Base class for password generators
 */
class PassGen {
public:
    
    /**
     */
    struct KernelCode{
        std::string filename;
        std::string name;
    };
    
    PassGen();
    PassGen(const PassGen & o);

    virtual ~PassGen();
    /**
     * Get next password. 
     * @param pass pointer to string where to put password
     * @return false if this is last password
     */
    virtual bool getPassword(std::string* pass);
    /**
     * Get next password.
     * @param pass pointer to char array where to pus password
     * @param len length of returned password
     * @return false if this is last password
     */
    virtual bool getPassword(char* pass, uint32_t *len);
    /**
     * Returns code which can be run in OpenCL 
     * @return 
     */
    virtual KernelCode* getKernelCode();
    /**
     * Set size of globalWorkSize 
     * @param gws
     */
    virtual void setKernelGWS(uint64_t gws);
    /**
     * Returns how many passwords to advance on kernel
     * @return 
     */
    virtual uint64_t getKernelStep();
    /**
     * Set arguments to kernel for next generator's step.
     * @return
     */
    virtual bool nextKernelStep();
    /**
     * Initializes generator in OpenCL kernel
     * @param kernel
     * @param que
     * @param context
     */
    virtual void initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context);
    /**
     * Returns maximum password length
     * @return 
     */
    virtual uint8_t maxPassLen();
    /**
     * Return true if this is only factory for password generator and not generator itself.
     * @return 
     */
    virtual bool isFactory();
    /**
     * Create new generator only if this is factory
     * @return 
     */
    virtual PassGen* createGenerator();
    
    /**
     * Set step for generators if this is factory
     */
    virtual void setStep(unsigned step);

    /**
     * Save current state of generatro
     * @param filename filename where to save state
     */
    virtual void saveState(std::string filename);
    
    /**
     * Load saved state into generator for resume cracking
     * @param filename load from this file
     */
    virtual void loadState(std::string filename);
    
    /**
     * Set verbose mode
     */
    void setVerbose();
    
protected:
    /** global work size, useful for GPU cracking*/
    uint64_t gws;
    /** true if generator is in GPU*/
    bool gpu_mode;
    bool verbose;
private:
    char* passBuffer;
};

/**
 * Generator loading passwords from file
 */
class DictionaryPassGen: public PassGen{
public:
    DictionaryPassGen(std::string filename);
    virtual ~DictionaryPassGen();
    //bool getPassword(std::string* pass);
    bool getPassword(char* pass, uint32_t *len);
    uint8_t maxPassLen();
    virtual void saveState(std::string filename);
    virtual void loadState(std::string filename);
protected:
    std::ifstream *file;
    char buffer[128];
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexAttr;
};

/**
 * Bruteforce password generator. Single-thread code, not thread-safe.
 */
class BrutePassGen: public PassGen{
public:
    BrutePassGen(char *chars, int max_len);
    virtual ~BrutePassGen();
    //virtual bool getPassword(std::string* pass);
    virtual bool getPassword(char* pass, uint32_t *len);
    uint8_t maxPassLen();
    virtual void saveState(std::string filename);
    virtual void loadState(std::string filename);
protected:
    char* chars;
    int maxLen;
    unsigned char *state;
    int chars_count;
    int first_char;
    char passBuffer[64];
    bool exhausted;
};

/**
 * Password generator generating only one specified password. Best for testing puroposes.
 */
class TestPassGen: public PassGen{
public:
    TestPassGen(std::string password);
    virtual ~TestPassGen();
    bool getPassword(std::string* pass);
    bool getPassword(char* pass, uint32_t *len);
    uint8_t maxPassLen();
protected:
    std::string pass;
    
};

/**
 * Bruteforce multithreaded generator, completely threadsafe and pretty fast
 */
class ThreadedBrutePassGen: public BrutePassGen{
public:
    ThreadedBrutePassGen(char *chars, int max_len, int childId = -1);
    virtual ~ThreadedBrutePassGen();
    //virtual bool getPassword(std::string* pass);
    virtual bool getPassword(char* pass, uint32_t *len);
    virtual bool isFactory();
    virtual void setKernelGWS(uint64_t gws);
    virtual void initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context);
    virtual uint64_t getKernelStep();
    virtual KernelCode* getKernelCode(); 
    virtual PassGen* createGenerator();
    virtual void saveState(std::string filename);
    virtual void loadState(std::string filename);
protected:
    virtual void reservePasswords();
    
    KernelCode gpuCode;
    cl::Buffer charsBuffer;
    cl::Buffer charPosBuffer;
    cl::Buffer powersBuffer;
    int childId;
    int nextChildId;
    unsigned char *addState;
    uint64_t passLeft;
    uint64_t myPosition;
    uint64_t reservationSize;
    uint64_t myStartPosition;
    uint32_t minResSize;
    static pthread_mutex_t mutex;
    static pthread_mutexattr_t mutexattr;
    static uint64_t countersMax;
    struct timespec speedClock;
    std::vector<ThreadedBrutePassGen*> children;
};

/**
 * Multithreaded cluster password generator using OpenMPI
 * @note Experimental 
 */
class ClusterBrutePassGen: public ThreadedBrutePassGen{
public:
    ClusterBrutePassGen(char *chars, int max_len, int childId = -1);
    virtual ~ClusterBrutePassGen();
protected:
    virtual void reservePasswords();
};

#endif	/* PASSGEN_H */

