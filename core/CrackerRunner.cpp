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

#include "CrackerRunner.h"
#include <iostream>
#include <vector>
#include <cstdint>

CrackerRunner::CrackerRunner() {
    pthread_attr_init(&thread_attr);
    explicitThreads = 0;
    verbose = false;
#ifdef WRATHION_MPI 
    mpi_enabled = false;
#endif

}

CrackerRunner::CrackerRunner(const CrackerRunner& orig) {
}

CrackerRunner::~CrackerRunner() {
    pthread_attr_destroy(&thread_attr);
}

uint32_t CrackerRunner::getCPUCount(){
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void CrackerRunner::sleep(uint32_t ms){
#ifdef __WIN32
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}

void CrackerRunner::setCrackerFactory(CrackerFactory* c){
    factory = c;
    if (verbose) {
        factory->setVerbose();
    }
}

void CrackerRunner::setPassGen(PassGen* passgen){
    this->passgen = passgen;
    if (verbose) {
        this->passgen->setVerbose();
    }
}

std::vector<uint64_t> *CrackerRunner::getSpeeds(){
    speeds.clear();
    for(std::vector<Cracker*>::iterator i = crackers.begin();i != crackers.end();i++){
        speeds.push_back((*i)->getSpeed());
    }
    return &speeds;
}

void CrackerRunner::start(){
    pass_found = false;
    if(explicitThreads>0)
        this->threads = explicitThreads;
    else
        this->threads = getCPUCount();
    
    Cracker* cracker;
    pthread_attr_setdetachstate(&thread_attr,1);
    
    if(factory->isGPU()){
        if(mappedDevices.size() == 0){
            std::cout << "No devices mapped, using 0:0:65536" << std::endl;
            DeviceConfig d;
            d.globalWorkSize = 65536;
            d.device = 0;
            d.platform = 0;
            mappedDevices.push_back(d);
        }
        this->threads = mappedDevices.size();
    }
    
    for(uint32_t i = 0; i<this->threads;i++){
        pthread_t *thread = new pthread_t;
        ThreadArg *arg = new ThreadArg;
        
        cracker = factory->createCracker();
#ifdef WRATHION_MPI
        if(mpi_enabled){
            cracker->mpiEnable(true);
        }
#endif
        if(i == 0){
            cracker->sharedDataInit();
        }
        if(passgen->isFactory()){
        		passgen->setStep(this->threads);
            cracker->setPassGen(passgen->createGenerator());
        }else{
            cracker->setPassGen(passgen);
        }
        if(factory->isGPU()){
            GPUCracker* gpu_cracker = dynamic_cast<GPUCracker*>(cracker);
            gpu_cracker->setDevice(mappedDevices[i]);
        }
        arg->cracker = cracker;
        arg->runner = this;
        pthread_create(thread,&thread_attr,&CrackerRunner::start_thread,arg);
        threads_ref.push_back(thread);
        crackers.push_back(cracker);
    }
}

void CrackerRunner::stop(){
    for(std::vector<Cracker*>::iterator i = crackers.begin();i != crackers.end();i++){
        (*i)->stop();
    }
    // MPI: send stop to all machines
    while(someRunning()){
        sleep(100);
    }
    
}


void CrackerRunner::setNumThreads(uint32_t threads){
    explicitThreads = threads;
}

uint32_t CrackerRunner::getNumThreads(){
    return threads;
}

bool CrackerRunner::allRunning(){
    for(std::vector<Cracker*>::iterator i = crackers.begin();i != crackers.end();i++){
        if(!(*i)->isRunning()){
            return false;
        }
    }
    return true;
}

bool CrackerRunner::someRunning(){
    for(std::vector<Cracker*>::iterator i = crackers.begin();i != crackers.end();i++){
        if((*i)->isRunning()){
            return true;
        }
    }
    return false;
}

bool CrackerRunner::passFound(){
    return pass_found;
}

std::string CrackerRunner::getPassword(){
    for(std::vector<Cracker*>::iterator i = crackers.begin();i != crackers.end();i++){
        if((*i)->passFound()){
            return (*i)->getPassword();
        }
    }
    return "";
}

void* CrackerRunner::start_thread(void* arg){
    ThreadArg* c = reinterpret_cast<ThreadArg*>(arg);
    c->cracker->runInThread();
    switch(c->cracker->getStopReason()){
        case UNKNOWN:
            std::cerr << "Thread exited unexpectedly" << std::endl;
            break;
        case PASS_FOUND:
            c->runner->pass_found = true;
            c->runner->stop();
            break;
        case PASS_EXHAUSTED:
          break;
        case STOP_COMMAND:
            break;
        case DEVICE_NOT_EXISTS:
            std::cerr << "Device does not exists" << std::endl;
            break;
        case PLATFORM_NOT_EXISTS:
            std::cerr << "Platform does not exists" << std::endl;
            break;
        case INTERNAL_ERROR:
            std::cerr << "Internal Error" << std::endl;
            break;
            
    }
    delete c;
    return NULL;
}

std::vector<DeviceConfig> CrackerRunner::decodeConfig(std::string str) {
    std::vector<DeviceConfig> res;
    int cur_pos = 0, found_pos = -1, len;
    do{
        cur_pos = found_pos+1;
        
        found_pos = str.find_first_of(',', cur_pos);
        if(found_pos != std::string::npos)
            len = found_pos-cur_pos;
        else
            len = found_pos;
        std::string config_str = str.substr(cur_pos,len);
        bool hasGWS = false;
        
        DeviceConfig devConf;
        
        int cur_conf_pos = 0, found_conf_pos = -1;
        found_conf_pos = config_str.find_first_of(':',cur_conf_pos);
        if(found_conf_pos == std::string::npos)
            continue;
        devConf.platform = atoi(config_str.substr(cur_conf_pos,found_conf_pos-cur_conf_pos).c_str());
        
        cur_conf_pos = found_conf_pos+1;
        found_conf_pos = config_str.find_first_of(':',cur_conf_pos);
        if(found_conf_pos != std::string::npos)
            hasGWS = true;
        devConf.device = atoi(config_str.substr(cur_conf_pos,found_conf_pos-cur_conf_pos).c_str());
        
        if(hasGWS){
            cur_conf_pos = found_conf_pos+1;
            devConf.globalWorkSize = atoi(config_str.substr(cur_conf_pos).c_str());
        }else{
            devConf.globalWorkSize = 65535;
        }
        res.push_back(devConf);
        
        
    }while(found_pos != std::string::npos);
    return res;
}

void CrackerRunner::setDeviceConfig(std::vector<DeviceConfig> &confs) {
    mappedDevices = confs;
}

void CrackerRunner::setVerbose() {
    verbose = true;
}

#ifdef WRATHION_MPI 
void CrackerRunner::mpiEnable(bool mode){
    mpi_enabled = mode;
}
#endif

