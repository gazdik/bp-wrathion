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
#include "UnicodePassGen.h"

#define MIN_PASS_RESERVATION 1024

PassGen::PassGen():gpu_mode(false) {
    passBuffer = new char[256];
    verbose = false;
}

//PassGen::PassGen(const PassGen& o) :
//		gws { o.gws }, gpu_mode { o.gpu_mode },
//		verbose { o.verbose }
//{
//		passBuffer = new char[256];
//}

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

bool PassGen::getPassword(char * pass, uint32_t * len)
{
  return (false);
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


void PassGen::initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context) {
}

void PassGen::setVerbose() {
    verbose = true;
}

TestPassGen::TestPassGen(std::string pass):pass(pass){    
}

TestPassGen::~TestPassGen() {

}

uint8_t TestPassGen::maxPassLen(){
    return pass.length();
}

bool TestPassGen::getPassword(std::string* pass){
    pass->assign(this->pass);
    return true;
}

bool TestPassGen::getPassword(char* pass, uint32_t* len) {
    ::memcpy(pass,this->pass.c_str(),this->pass.length());
    *len = this->pass.length();
    return true;
}


BrutePassGen::BrutePassGen(char* chars, int max_len):chars(chars),maxLen(max_len),exhausted(false){
    this->chars = chars;
    this->maxLen = max_len;
    this->chars_count = strlen(chars);
    this->state = new unsigned char[max_len];
    for(int i = 0;i<max_len;i++){
        this->state[i] = 0;
    }
    this->first_char = max_len-1;
}

BrutePassGen::~BrutePassGen(){
    delete this->state;
}

bool BrutePassGen::getPassword(char* pass, uint32_t* len) {
    char carry = 0;
    int out_i, in_i;
    
    if(exhausted)
        return false;
    
    for(out_i=0, in_i=first_char;in_i<this->maxLen;in_i++,out_i++){
        pass[out_i] = chars[state[in_i]];
    }

    *len = out_i;
    state[this->maxLen-1]++;
    
    for(int i = this->maxLen-1;i>=0;i--){
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
    return true;
}


uint8_t BrutePassGen::maxPassLen(){
    return this->maxLen;
}

void BrutePassGen::loadState(std::string filename) {
    std::ifstream in_file;
    in_file.open(filename,std::ios_base::binary);
    char ID;
    if(in_file.is_open()){
        in_file.read(&ID,1);
        if(ID != PASSGEN_ID_BRUTE){
            in_file.close();
            return;
        }
        int maxLen;
        
        in_file.read((char*)&maxLen,sizeof(maxLen));
        if(maxLen != this->maxLen){
            in_file.close();
            return;
        }
        this->maxLen = maxLen;
        in_file.read((char*)state,maxLen);
        in_file.read((char*)&first_char,sizeof(first_char));
    }
}

void BrutePassGen::saveState(std::string filename) {
    std::ofstream out_file;
    out_file.open(filename,std::ios_base::binary);
    char ID = PASSGEN_ID_BRUTE;
    if(out_file.is_open()){
        out_file.write(&ID,1);
        out_file.write((char*)&maxLen,sizeof(maxLen));
        out_file.write((char*)state,maxLen);
        out_file.write((char*)&first_char,sizeof(first_char));
        out_file.close();
    }
}


DictionaryPassGen::DictionaryPassGen(std::string filename){
    this->file = new std::ifstream(filename.c_str());
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutex_init(&mutex,&mutexAttr);
}

DictionaryPassGen::~DictionaryPassGen(){
    if(this->file != NULL){
        this->file->close();
        delete this->file;
    }
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&mutexAttr);
}

bool DictionaryPassGen::getPassword(char* pass, uint32_t* len) {
    if(file->bad())
        return false;
    pthread_mutex_lock(&mutex);
    this->file->getline(buffer,128);
    pthread_mutex_unlock(&mutex);
    int read = strlen(buffer);
    *len = read>maxPassLen()?maxPassLen():read;
    if(read == 0)
        return false;
    
    ::memcpy(pass,buffer,*len);
    return true;
}

uint8_t DictionaryPassGen::maxPassLen(){
    return 20;
}

void DictionaryPassGen::loadState(std::string filename) {
    std::ifstream in_file;
    in_file.open(filename,std::ios_base::binary);
    char ID;
    if(in_file.is_open()){
        in_file.read(&ID,1);
        if(ID != PASSGEN_ID_DICTIONARY){
            in_file.close();
            return;
        }
        uint64_t cur_pos;
        in_file.read((char*)&cur_pos,sizeof(uint64_t));
        in_file.close();
        file->seekg(cur_pos);
    }
}

void DictionaryPassGen::saveState(std::string filename) {
    std::ofstream out_file;
    out_file.open(filename,std::ios_base::binary);
    char ID = PASSGEN_ID_DICTIONARY;
    if(out_file.is_open()){
        out_file.write(&ID,1);
        uint64_t cur_pos= file->tellg();
        out_file.write((char*)&cur_pos,sizeof(uint64_t));
        out_file.close();
    }
}


ThreadedBrutePassGen::ThreadedBrutePassGen(char* chars, int max_len, int childId):BrutePassGen(chars,max_len),childId(childId),passLeft(0),nextChildId(0) {
    minResSize = MIN_PASS_RESERVATION;
    if(childId == -1){
        pthread_mutexattr_init(&mutexattr);
        pthread_mutex_init(&mutex,&mutexattr);
        countersMax = 0;
    }else{
        addState = new unsigned char[max_len];
        myPosition = 0;
    }
}

ThreadedBrutePassGen::~ThreadedBrutePassGen() {
    if(childId == -1){
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy(&mutexattr);
        for(std::vector<ThreadedBrutePassGen*>::iterator i = children.begin();i != children.end();i++){
            delete *i;
        }
        children.clear();
    }else{
        delete[] addState;
    }
}

bool ThreadedBrutePassGen::isFactory() {
    return childId == -1;
}

bool ThreadedBrutePassGen::getPassword(char* pass, uint32_t* len) {
    if(passLeft == 0){
        reservePasswords();
    }
    passLeft--;
    return BrutePassGen::getPassword(pass,len);
}


PassGen* ThreadedBrutePassGen::createGenerator() {
    if(childId == -1){
        ThreadedBrutePassGen *child = new ThreadedBrutePassGen(chars,maxLen,nextChildId++);
        children.push_back(child);
        return child;
    }
    return NULL;
}


uint64_t ThreadedBrutePassGen::getKernelStep() {
    if(myPosition == gws)
        reservationSize = 4*gws;
    if(passLeft == 0){
        uint64_t my_state = myPosition;
    
        pthread_mutex_lock(&mutex);//protect countersMax
        uint64_t my_state_diff = countersMax - my_state + reservationSize;
        myStartPosition = countersMax;
        countersMax = my_state + my_state_diff;
        pthread_mutex_unlock(&mutex);
        
        myPosition = my_state + my_state_diff;
        uint64_t state_change = my_state_diff - reservationSize + gws;
        passLeft = reservationSize - gws;
        return state_change;
    }else{
        passLeft -= gws;
        return gws;
    }
}

void ThreadedBrutePassGen::setKernelGWS(uint64_t gws) {
    PassGen::setKernelGWS(gws);
    minResSize = gws;
}

PassGen::KernelCode* ThreadedBrutePassGen::getKernelCode() {
    gpuCode.filename = "kernels/brute_passgen.cl";
    gpuCode.name = "brute_passgen";
    return &gpuCode;
}

void ThreadedBrutePassGen::initKernel(cl::Kernel *kernel, cl::CommandQueue *que, cl::Context *context) {
    
    charsBuffer = cl::Buffer(*context,CL_MEM_READ_ONLY,sizeof(char)*chars_count);
    charPosBuffer = cl::Buffer(*context,CL_MEM_READ_ONLY,sizeof(char)*256);
    powersBuffer = cl::Buffer(*context,CL_MEM_READ_ONLY,sizeof(cl_ulong)*chars_count);
    
    uint8_t charsPos[256] = {0};
    cl_ulong *powers = new cl_ulong[chars_count];
    for(int i = 0;i<chars_count;i++){
        charsPos[(uint8_t)chars[i]] = i;
        powers[i] = pow(chars_count,i);
    }
    
    que->enqueueWriteBuffer(charsBuffer,CL_FALSE,0,chars_count*sizeof(char),chars);
    que->enqueueWriteBuffer(charPosBuffer,CL_FALSE,0,sizeof(char)*256,charsPos);
    que->enqueueWriteBuffer(powersBuffer,CL_TRUE,0,sizeof(cl_ulong)*chars_count,powers);
    
    delete[] powers;
    
    kernel->setArg(2,maxLen);
    kernel->setArg(3,charsBuffer);
    kernel->setArg(4,chars_count);
    kernel->setArg(5,powersBuffer);
    kernel->setArg(6,charPosBuffer);
    
}

void ThreadedBrutePassGen::reservePasswords() {
    for(int i = 0;i<maxLen;i++){
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
    
    uint64_t my_state = myPosition;
    
    pthread_mutex_lock(&mutex); //protect countersMax
    uint64_t my_state_diff = countersMax - my_state + reservationSize;
    myStartPosition = countersMax;
    countersMax = my_state + my_state_diff;
    pthread_mutex_unlock(&mutex);
    
    myPosition = my_state + my_state_diff;
    uint64_t state_change = my_state_diff - reservationSize;
    passLeft = reservationSize;
    if (state_change == 0) {
        return;
    }
    
    // create diff state
    int32_t pos = maxLen-1;
    int carry = 0;
    int lowest = this->maxLen-1;
    
    while(state_change > 0 && pos >= 0){
        
        uint64_t toAdd,canAdd;
        canAdd = 0;
        // prevedeme co muzeme pricist bez preteceni, do soustavy se kterou se dobre pocita
        for(int i = first_char;i<=lowest;i++){
            canAdd += ((chars_count-1)-state[i])*pow(chars_count,lowest-i); // prevod z n-kove soustavy do 10kove
        }
        
        if(state_change > canAdd)
            toAdd = canAdd;
        else
            toAdd = state_change;
        
        state_change -= toAdd;
        
        // pricteme co muzeme
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
        
        // pokud stale neco zbyva, nechame to pretect
        if(state_change > 0){
            state[lowest]++;
            state_change--;
            carry = 0;
            for(int i = this->maxLen-1;i>=0;i--){
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

void ThreadedBrutePassGen::loadState(std::string filename) {
    if(childId != -1)
        return;
    std::ifstream in_file;
    in_file.open(filename,std::ios_base::binary);
    char ID;
    if(in_file.is_open()){
        in_file.read(&ID,sizeof(ID));
        if(ID != PASSGEN_ID_THREADED_BRUTE){
            in_file.close();
            return;
        }
        in_file.read((char*)&countersMax,sizeof(countersMax));
        in_file.close();
    }
}

void ThreadedBrutePassGen::saveState(std::string filename) {
    if(childId != -1)
        return;
    uint64_t minPosition = UINT64_MAX;
    for(std::vector<ThreadedBrutePassGen*>::iterator i = children.begin();i != children.end();i++){
        if ((*i)->myStartPosition < minPosition){
            minPosition = (*i)->myStartPosition;
        }
    }
    
    if(minPosition == UINT64_MAX)
        return;
    
    std::ofstream out_file;
    out_file.open(filename,std::ios_base::binary);
    char ID = PASSGEN_ID_THREADED_BRUTE;
    
    if(out_file.is_open()){
        out_file.write(&ID,sizeof(ID));
        out_file.write((char*)&minPosition,sizeof(uint64_t));
        out_file.close();
    }
}

pthread_mutex_t ThreadedBrutePassGen::mutex;
pthread_mutexattr_t ThreadedBrutePassGen::mutexattr;
uint64_t ThreadedBrutePassGen::countersMax;

void PassGen::setStep(unsigned step)
{
}

bool PassGen::nextKernelStep()
{
  return(false);
}
