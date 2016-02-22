/* 
 * Copyright (C) 2014 Radek Hranicky
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

#ifndef UCPASSGEN_H
#define	UCPASSGEN_H

#define	UC_CHARSIZE 32

#include <iomanip>
#include <string>
#include <fstream>
#include <map>
#include <CL/cl.hpp>

#include "PassGen.h"

/**
 * Base class for Unicode Brute Threaded Password Generator
 */

class UnicodePassGen: public ThreadedBrutePassGen {
public:

     /**
     * Constructor of the Unicode password generator.
     * @param file with unicode characters used for password generation
     * @param maximum length of a password
     * @param identifier of a child within the passgen factory (-1 = factory)
     */
    UnicodePassGen(uint32_t* ucChars, int charsCount, int max_len, int utf32_max_len, int childId = -1);
    
    /**
     * Get next password.
     * @param pass pointer to char array where to pus password
     * @param len length of returned password
     * @return false if this is last password
     */
    virtual bool getPassword(char* pass, uint32_t *len);
    
    /**
     * Create new generator only if this is factory
     * @return 
     */
    virtual PassGen* createGenerator();
    
    /**
     * Returns code which can be run in OpenCL 
     * @return 
     */
    virtual KernelCode* getKernelCode();
    
    /**
     * Initializes generator in OpenCL kernel
     * @param kernel
     * @param que
     * @param context
     */
    virtual void initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context);
   
protected:
    virtual void reservePasswords();

    uint32_t* uc_chars;
    uint32_t utf32_pw[64];
    int utf32_maxLen; // Number of characters in UTF32 (<= number of characters in UTF8)
    cl::Buffer unicodeCharsBuffer;
    
};

/*
 * Debug: structures for unsigned char in hex:
 */
struct HexCharStruct
{
  unsigned char c;
  HexCharStruct(unsigned char _c) : c(_c) { }
};

inline std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs)
{
  return (o << std::setw(2) << std::hex << std::setfill('0') << (int)hs.c);
}

inline HexCharStruct uchar_hex(unsigned char _c)
{
  return HexCharStruct(_c);
}

/* ******************************************** */

#endif	/* UCPASSGEN_H */

