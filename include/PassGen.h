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
    virtual bool getPassword(char* pass, uint32_t *len) = 0;
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
     * Initializes generator in OpenCL kernel
     * @param kernel
     * @param que
     * @param context
     */
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


#endif	/* PASSGEN_H */

