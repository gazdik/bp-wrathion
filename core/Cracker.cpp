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

#include <iostream>
#include "Cracker.h"
#include <cmath>
#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

Cracker::Cracker():stopReason(UNKNOWN),running(false),pass_found(false) {
#ifdef WRATHION_MPI
    mpi_enabled = false;
#endif
}

Cracker::Cracker(const Cracker& orig) {
}

Cracker::~Cracker() {
}


uint64_t Cracker::getSpeed(){
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
        
    double elapsed = (end.tv_sec - speedClock.tv_sec);
    elapsed += (end.tv_nsec - speedClock.tv_nsec) / 1000000000.0;
    speedClock = end;
    uint64_t res = (uint64_t)ceil(passwdsCount*elapsed);
    passwdsCount = 0;
    last_speed = res;
    return res;
}

void Cracker::sharedDataInit(){
    
}

void Cracker::sharedDataDestroy(){
    
}

void Cracker::runInThread(){
    this->stop_work = false;
    this->running = true;
    this->passwdsCount = 0;
    clock_gettime(CLOCK_MONOTONIC, &this->speedClock);
    this->run();
    this->running = false;
}

void Cracker::stop(){
    this->stop_work = true;
}

StopReason Cracker::getStopReason(){
    return stopReason;
}

void Cracker::setPassGen(PassGen* pass_gen){
    passgen = pass_gen;
}

std::string Cracker::getPassword(){
    return password;
}

bool Cracker::passFound(){
    return pass_found;
}

bool Cracker::isRunning(){
    return running;
}

void Cracker::run() {
    bool internal_stop = false;
#ifdef WRATHION_MPI
    //defines number of passwords after which node speed is sent to master(0)
    int mpi_communication_thr = 500;
#endif
    pass_found = false;
    password.reserve(passgen->maxPassLen());
    
    while(!pass_found && !stop_work && !internal_stop){
        if(!passgen->getPassword(&password)){
            stopReason = PASS_EXHAUSTED;
            break;
        }
        CheckResult res = checkPassword(&password);
        switch(res){
            case CR_PASSWORD_MATCH:
                pass_found = true;
                stopReason = PASS_FOUND;
                break;
            case CR_ERROR_STOP:
                stopReason = INTERNAL_ERROR;
                internal_stop = true;
                break;
            default:
                break;
        }
        passwdsCount++;
#ifdef WRATHION_MPI
        if(mpi_enabled){
            if(mpi_proc_id > 0){
                //not master
                mpi_communication_thr--;
                if(mpi_communication_thr == 0){
                    MPI_Isend();
                    mpi_communication_thr = last_speed;//send stats every second
                }
            }else{
                //master
                MPI_Irecv();
            }
        }
#endif
    }
    if(stop_work){
        stopReason = STOP_COMMAND;
    }
}

CheckResult Cracker::checkPassword(const std::string* password) {
    return CR_PASSWORD_WRONG;
}

#ifdef WRATHION_MPI
    void Cracker::mpiEnable(bool enabled){
        mpi_enabled = enabled;
    }
#endif


