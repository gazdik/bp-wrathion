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

#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <iterator>

#include "PassGen.h"
#include "UnicodePassGen.h"
#include "utf8.h"

#define MIN_PASS_RESERVATION 1024

UnicodePassGen::UnicodePassGen(uint32_t* ucChars, int charsCount, int max_len, int utf32_max_len, int childId):ThreadedBrutePassGen((char*)ucChars /* ! fix ! */, max_len, childId) {
    this->uc_chars = ucChars;
    this->chars_count = charsCount;
    this->maxLen = max_len;
    this->utf32_maxLen = utf32_max_len;
}

bool UnicodePassGen::getPassword(char* pass, uint32_t* len) {
    if(passLeft == 0){
        reservePasswords();
    }
    
    passLeft--;

    char carry = 0;
    int out_i, in_i;
    
    if(exhausted)
        return false;
    
    for(out_i=0, in_i=first_char; in_i<this->utf32_maxLen; in_i++, out_i++){
        utf32_pw[out_i] = uc_chars[state[in_i]];
    }
    state[this->utf32_maxLen-1]++;
    /*
     *  In this moment, out_i is set to the number of UTF-32 characters in password
     *  This final password length may be different in UTF-8 representation
     */
    
    for(int i = this->utf32_maxLen-1;i>=0;i--){
        if(i<first_char){
            first_char = i;
            state[i]--;
        }
        state[i] += carry;
        if(state[i]>chars_count-1){
            state[i] %= chars_count;
            carry = 1;
        }else{
            break;
        }
        if(i == 0 && carry == 1)
            exhausted = true;
    }

    /* Convert generated UTF-32 password to UTF-8 */
    char *output_it;
    output_it = (char*)utf8::utf32to8(utf32_pw, utf32_pw + out_i, pass);
    *len = output_it - pass;
    
    return true;
}

PassGen* UnicodePassGen::createGenerator() {
    if(childId == -1){
        UnicodePassGen *child = new UnicodePassGen(uc_chars, chars_count, maxLen, utf32_maxLen, nextChildId++);
        children.push_back(child);
        return child;
    }
    return NULL;
}

UnicodePassGen::KernelCode* UnicodePassGen::getKernelCode() {
    gpuCode.filename = "kernels/unicode_passgen.cl";
    gpuCode.name = "unicode_passgen";
    return &gpuCode;
}

void UnicodePassGen::initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context) {
    
    unicodeCharsBuffer = cl::Buffer(*context,CL_MEM_READ_ONLY,sizeof(uint32_t)*chars_count);
    charPosBuffer = cl::Buffer(*context,CL_MEM_READ_ONLY,sizeof(char)*256);
    powersBuffer = cl::Buffer(*context,CL_MEM_READ_ONLY,sizeof(cl_ulong)*chars_count);
    
    uint8_t charsPos[256] = {0};
    cl_ulong *powers = new cl_ulong[chars_count];
    for(int i = 0;i<chars_count;i++){
        charsPos[(uint8_t)chars[i]] = i;
        powers[i] = pow(chars_count,i);
    }
    
    que->enqueueWriteBuffer(unicodeCharsBuffer,CL_FALSE,0,chars_count*sizeof(uint32_t),uc_chars);
    que->enqueueWriteBuffer(charPosBuffer,CL_FALSE,0,sizeof(char)*256,charsPos);
    que->enqueueWriteBuffer(powersBuffer,CL_TRUE,0,sizeof(cl_ulong)*chars_count,powers);
    
    delete[] powers;
    
    kernel->setArg(2,maxLen);              // Maximum length of final UTF8 passwords
    kernel->setArg(3,unicodeCharsBuffer);
    kernel->setArg(4,chars_count);
    kernel->setArg(5,powersBuffer);
    kernel->setArg(6,charPosBuffer);
}

void UnicodePassGen::reservePasswords() {
    for(int i = 0;i<utf32_maxLen;i++){
        this->addState[i] = 0;
    }
    
    //make reservation    
    if(myPosition == 0){
        reservationSize = minResSize;
        clock_gettime(CLOCK_MONOTONIC, &speedClock);
    }else{
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double elapsed = (end.tv_sec - speedClock.tv_sec);
        elapsed += (end.tv_nsec - speedClock.tv_nsec) / 1000000000.0;
        
        uint64_t speed = reservationSize/elapsed;
        speedClock = end;
        uint64_t newResSize = speed/2;
        uint64_t maxRes = reservationSize*10;
        if(newResSize > maxRes){
            newResSize = maxRes;
        }
        reservationSize = newResSize;
        if(reservationSize<minResSize)
            reservationSize = minResSize;
    }
    
    
    
    //reservationSize = minResSize;
    uint64_t my_state = myPosition;
    
    pthread_mutex_lock(&mutex);//protect countersMax
    uint64_t my_state_diff = countersMax - my_state + reservationSize;
    myStartPosition = countersMax;
    countersMax = my_state + my_state_diff;
    pthread_mutex_unlock(&mutex);
    
    myPosition = my_state + my_state_diff;
    uint64_t state_change = my_state_diff - reservationSize;
    passLeft = reservationSize;
    if(state_change == 0)
        return;
    
    //create diff state
    int32_t pos = utf32_maxLen-1;
    

    int carry = 0;
    int lowest = this->utf32_maxLen-1;
    
    while(state_change > 0 && pos >= 0){
        
        uint64_t toAdd,canAdd;
        canAdd = 0;
        //prevedeme co muzeme pricist bez preteceni, do soustavy se kterou se dobre pocita
        for(int i = first_char;i<=lowest;i++){
            canAdd += ((chars_count-1)-state[i])*pow(chars_count,lowest-i);//prevod z n-kove soustavy do 10kove
        }
        
        if(state_change > canAdd)
            toAdd = canAdd;
        else
            toAdd = state_change;
        
        state_change -= toAdd;
        
        //pricteme co muzeme
        carry = 0;
        for(int i = lowest;toAdd>0 || carry;i--){
            char state_add = toAdd % chars_count;
            toAdd /= chars_count;
            state[i]+=state_add + carry;
            if(state[i] >= chars_count){
                carry = 1;
                state[i] %= chars_count;
            }else{
                carry = 0;
            }
        }
        
        //pokud stale neco zbyva, nechame to pretect
        if(state_change > 0){
            state[lowest]++;
            state_change--;
            carry = 0;
            for(int i = this->utf32_maxLen-1;i>=0;i--){
                if(i<first_char){
                    first_char = i;
                    state[i]--;
                }
                state[i] += carry;
                if(state[i]>chars_count-1){
                    state[i] %= chars_count;
                    carry = 1;
                }else{
                    break;
                }
            }
        }
    }
}

