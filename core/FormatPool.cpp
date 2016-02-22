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

#include "FormatPool.h"
#include <fstream>
#include <cstring>


FormatPool::FormatPool() {
    verbose = false;
}

FormatPool::FormatPool(const FormatPool& orig) {
}

FormatPool::~FormatPool() {
    for(std::vector<FileFormat*>::iterator i = formats.begin();i != formats.end();i++){
        delete *i;
    }
    formats.clear();
}

void FormatPool::addFormat(FileFormat* format){
    if (verbose) {
        format->setVerbose();
    }
    this->formats.push_back(format);
}

void FormatPool::setVerbose() {
    verbose = true;
}

FileFormat* FormatPool::getFileFormat(std::string filename){
    std::string signature;
    std::ifstream filestream(filename,std::ios::binary);
    char buffer[32];
    std::string ext = filename.substr(filename.find_last_of(".")+1);
    if(!filestream){
        std::cerr << "Error: " << strerror(errno) << std::endl;
    }
    int readed = filestream.readsome(buffer,32);

    if(!filestream)
        std::cerr << "Error: " << strerror(errno) << std::endl;
    filestream.close();
    signature.assign(buffer,32);
    
    for(std::vector<FileFormat*>::iterator i = formats.begin();i != formats.end();i++){
        if((*i)->checkFileExtension(ext) && (*i)->checkFileSignature(signature)){
            (*i)->init(filename);
            if((*i)->isSupported())
                return *i;
        }
    }
    return NULL;
}

FileFormat* FormatPool::getFileFormat(std::istream datastream){
    
}


