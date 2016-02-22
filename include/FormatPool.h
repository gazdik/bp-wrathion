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

#ifndef FORMATPOOL_H
#define	FORMATPOOL_H

#include <vector>
#include <iostream>

#include "FileFormat.h"

/**
 * Class to select the best format for input
 */
class FormatPool {
public:
    FormatPool();
    FormatPool(const FormatPool& orig);
    virtual ~FormatPool();
    /**
     * Add format to pool
     * @param format format to add
     */
    void addFormat(FileFormat* format);
    /**
     * Return best file format for defined file
     * @param filename
     * @return 
     */
    FileFormat* getFileFormat(std::string filename);
    /**
     * Return best file format for defined stream. Currently stream is written to file and then processed as file.
     * @param datastream
     * @return 
     */
    FileFormat* getFileFormat(std::istream datastream);
    /**
     * Sets the verbose mode
     */
     void setVerbose();
private:
    /**
     * Formats in pool
     */
    std::vector<FileFormat*> formats;
protected:
    bool verbose;
};

#endif	/* FORMATPOOL_H */

