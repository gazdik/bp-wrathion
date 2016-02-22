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

#ifndef CRACKERRUNNER_H
#define	CRACKERRUNNER_H

#include <pthread.h>
#include <stdint.h>

#include "Cracker.h"
#include "FileFormat.h"
#include "GPUCracker.h"
#include <vector>

#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/**
 * Class for running crackers on GPU or CPU
 */
class CrackerRunner {
public:
    CrackerRunner();
    CrackerRunner(const CrackerRunner& orig);
    virtual ~CrackerRunner();
    /**
     * Sets cracker factory for spawning crackers
     * @param c
     */
    void setCrackerFactory(CrackerFactory* c);
    /**
     * Explicitly sets number of threads, 0 means spawn number of threads equal to cpu count
     * @param threads number of threads to spawn
     */
    void setNumThreads(uint32_t threads);
    /**
     * Map threads to OpenCL devices. First device in vector goes to the firts thread.
     * @param confs Devices to map
     */
    void setDeviceConfig(std::vector<DeviceConfig> &confs);
    /**
     * Start cracking. Spawns all crackers
     */
    void start();
    /**
     * Stop cracking. Send all thread command to stop.
     */
    void stop();
    /**
     * Returns speeds of all threads.
     * @return speed of each thread in vector
     */
    std::vector<uint64_t> *getSpeeds();
    /**
     * Set Password generator for cracking
     * @param passgen passgen tp use
     */
    void setPassGen(PassGen *passgen);
    /**
     * Checks wether all threads are running.
     * @return true if all threads running
     */
    bool allRunning();
    /**
     * Chcek wether at least one thread is running.
     * @return true if atleast one thread is running
     */
    bool someRunning();
    /**
     * Retruns found password.
     * @return Found password if exists, else returns empty string
     */
    std::string getPassword();
    /**
     * Checks if password was found.
     * @return true if it's found
     */
    bool passFound();
    /**
     * Returns number of spawned threads
     * @return 
     */
    uint32_t getNumThreads();
    /**
     * Suspends execution of current thread for amount of time
     * @param sleep milliseconds to sleep
     */
    static void sleep(uint32_t sleep);
    /**
     * Decode text device config
     * @param str string of configs in format <platform>:<device>[:<GWS>][,<platform>:<device>[:<GWS>]]...
     * @return defice config structsin vector
     */
    static std::vector<DeviceConfig> decodeConfig(std::string str);
    /**
     * Sets verbose mode of cracker runner
     */
    void setVerbose();
#ifdef WRATHION_MPI 
    void mpiEnable(bool mode);
#endif
private:
    /**
     * Entry point for thread
     * @param arg initialised ThreadArg
     * @return nothing 
     */
    static void* start_thread(void *arg);
    /**
     * Get devices currently mapped 
     * @return 
     */
    std::vector<DeviceConfig> getMappedDevices();
    /**
     * Factory for creating crackers
     */
    CrackerFactory *factory;
    /**
     * Returns number of processors in system
     * @return number of CPUs
     */
    uint32_t getCPUCount();
    /**
     * Explicitly set number of threads
     */
    uint32_t explicitThreads;
    /**
     * Current number of threads
     */
    uint32_t threads;
    /**
     * Pointer to password generator
     */
    PassGen* passgen;
    /**
     * flag set if password found
     */
    bool pass_found;
    /**
     * Argument struct for thread arg
     */
    struct ThreadArg{
        /**
         * Pointer to cracker to run
         */
        Cracker* cracker;
        /**
         * Pointer to runner which spawns this cracker
         */
        CrackerRunner* runner;
    };
    
    /**
     * vector of mapped devices
     */
    std::vector<DeviceConfig> mappedDevices;
    /**
     * Attribute for  posix thread
     */
    pthread_attr_t thread_attr;
    /**
     * Pointers to spawned posic threads
     */
    std::vector<pthread_t*> threads_ref;
    /**
     * Pointers to Spawned crackers
     */
    std::vector<Cracker*> crackers;
    /**
     * Vector of current speeds
     */
    std::vector<uint64_t> speeds;
    /**
     * Verbose mode
     */
    bool verbose;

#ifdef WRATHION_MPI 
    int mpi_process_id;
    int mpi_enabled;
#endif
};

#endif	/* CRACKERRUNNER_H */

