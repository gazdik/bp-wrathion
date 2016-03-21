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

#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>

#include "PassGen.h"

#define MIN_PASS_RESERVATION 1024

PassGen::PassGen():gpu_mode(false) {
    passBuffer = new char[256];
    verbose = false;
}

PassGen::PassGen(const PassGen& o) :
		gws { o.gws }, gpu_mode { o.gpu_mode },
		verbose { o.verbose }
{
		passBuffer = new char[256];
}

PassGen::~PassGen() {
    delete[] passBuffer;
}

PassGen::KernelCode* PassGen::getKernelCode(){
    return NULL;
}

void PassGen::setKernelGWS(uint64_t gws) {
    this->gws = gws;
    this->gpu_mode = true;
}

uint64_t PassGen::getKernelStep() {
    return 0;
}

uint8_t PassGen::maxPassLen() {
    return 0;
}

bool PassGen::getPassword(std::string* pass){
    uint32_t len;
    bool res = getPassword(passBuffer,&len);
    pass->assign(passBuffer,len);
    return res;
}

PassGen* PassGen::createGenerator() {
    return NULL;
}

bool PassGen::isFactory() {
    return false;
}

void PassGen::loadState(std::string filename) {
    
}

void PassGen::saveState(std::string filename) {

}

void PassGen::setVerbose() {
    verbose = true;
}


void PassGen::setStep(unsigned step)
{
}

