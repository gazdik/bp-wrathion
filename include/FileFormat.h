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

#ifndef FILEFORMAT_H
#define	FILEFORMAT_H

#include <fstream>
#include "Cracker.h"
#include <cstdint>

//enum SupportedCrackers{
//    CPU_CRACKER=1,
//    GPU_CRACKER=2,
//};

/**
 * Base for Cracker factories
 */
class CrackerFactory{
public:
    /**
     * Constructor
     * @param isGPU true if created cracker uses GPU
     */
    CrackerFactory(bool isGPU=false);
    /**
     * Creates new cracker
     * @return cracker
     */
    virtual Cracker* createCracker() = 0;
    virtual ~CrackerFactory();
    /**
     * Check if factory is creating GPU crackers
     * @return 
     */
    bool isGPU();
    /**
     * Sets the verbose mode
     */
    void setVerbose();
private:
    /**
     * Flag set if facory creating GPU crackers
     */
    bool gpu;
protected:
    bool verbose;
};

/**
 * Base class for reading file formats
 */
class FileFormat {
public:
    FileFormat();
    FileFormat(const FileFormat& orig);
    virtual ~FileFormat();
    /**
     * Checks if this class supports file with this signature
     * @param signature max 32 byte signature of file
     * @return true if supports this signature
     */
    virtual bool checkFileSignature(std::string &signature);
    /**
     * Checks if this class supports file with this file extension
     * @param ext extension of file
     * @return true if this extension is supported
     */
    virtual bool checkFileExtension(std::string &ext);
    /**
     * Get GPU crackers factory.  
     * @return pointer to factory, NULL if cracker does not exists.
     */
    virtual CrackerFactory* getGPUCracker();
    /**
     * Get CPU crackers factory.  
     * @return pointer to factory, NULL if cracker does not exists.
     */
    virtual CrackerFactory* getCPUCracker();
    //uint8_t getSupportedCrackers();
    //std::string getName();
    //virtual void printFileInfo();
    /**
     * Initializes data form file
     * @param filename
     */
    virtual void init(std::string& filename) = 0;
    /**
     * Chcek if contents of file is supported. Must be called after init.
     * @return 
     */
    bool isSupported();
    /**
     * Chceks if contents of file is encrypted. Must be called after init.
     * @return 
     */
    bool isEncrypted();
    /**
     * Sets the verbose mode
     */
    void setVerbose();
protected:
    std::string signature;
    std::string ext;
    std::string name;
    //uint8_t crackers;
    bool is_encrypted;
    bool is_supported;
    bool verbose;
};


#endif	/* FILEFORMAT_H */

