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

#ifndef CRACKER_H
#define	CRACKER_H

#include <pthread.h>
#include <string>
#include <vector>
#include <time.h>

#include "PassGen.h"
#include "UnicodePassGen.h"

/**
 * Reason why cracking thread stops
 */
enum StopReason{
    /**
     * Unknown error, possibly unhandled state
     */
    UNKNOWN,
    /**
     * Thread recieved command to stop
     */
    STOP_COMMAND,
    /**
     * Thread found password
     */
    PASS_FOUND,
    /**
     * All passwords has been tested
     */
    PASS_EXHAUSTED,
    /**
     * OpenCL platform for this thread does not exists
     */
    PLATFORM_NOT_EXISTS,
    /**
     * OpenCL device for this thread does not exists
     */
    DEVICE_NOT_EXISTS,
    /**
     * Thread internal error
     */
    INTERNAL_ERROR,
};

/**
 * Result of password check
 */
enum CheckResult{
    /**
     * Password was found
     */
    CR_PASSWORD_MATCH,
    /**
     * Password did not match
     */
    CR_PASSWORD_WRONG,
    /**
     * Check error, but can continue to next password
     */
    CR_ERROR_CONTINUE,
    /**
     * Chcek error and can't continue
     */
    CR_ERROR_STOP,
};


/**
 * Base class for all cracker types
 */
class Cracker {
public:
    Cracker();
    Cracker(const Cracker& orig);
    virtual ~Cracker();
    /**
     * Sets password generator, which must be thread-safe. This passgen also can't be passgen factory.
     * @param passgen 
     */
    void setPassGen(PassGen *passgen);
    /**
     * Gets currently cracking password
     * @note not implemented
     * @return
     */
    std::string getCurrentPassword();
    /**
     * Gets current password length
     * @note not implemented
     * @return 
     */
    unsigned int getCurrentLength();
    /**
     * Retuns current speed (passwords/second)
     * @return current speed
     */
    uint64_t getSpeed();
    /**
     * Gets cracked password if exists.
     * @return 
     */
    std::string getPassword();
    /**
     * Gets estimated tame to complete current passwords length
     * @return 
     * @note not implemented
     */
    unsigned int getETA();
    /**
     * Checks if password was found
     * @return 
     */
    bool passFound();
    /**
     * Checks if thread is running
     * @return 
     */
    bool isRunning();
    /**
     * Sends stop command
     */
    void stop();
    /**
     * Entry point for thread run
     */
    void runInThread();
    /**
     * Run cracking in loop
     */
    virtual void run();
    /**
     * Check positiveness of password
     * @param password password to chcek
     * @return State of check
     */
    virtual CheckResult checkPassword(const std::string *password);
    /**
     * Initialise shared data for all threads of this task type. This must be called befor first thread starts.
     */
    virtual void sharedDataInit();
    /**
     * Destroy shared data. This mus be called after all threads stops.
     */
    virtual void sharedDataDestroy();
    /**
     * Returns reason why thread stops its work.
     * @return 
     */
    StopReason getStopReason();
#ifdef WRATHION_MPI
    void mpiEnable(bool enabled);
#endif
protected:
    /**
     * Buffer for password
     */
    std::string password;
    /**
     * Flag set if thread should stop
     */
    bool stop_work;
    /**
     * State of stop
     */
    StopReason stopReason;
    /**
     * Password generator for current thread
     */
    PassGen *passgen;
    /**
     * Counter for speed measure
     */
    uint32_t passwdsCount;
    /**
     * Flag is set if thread is running
     */
    bool running;
    /**
     * Flag is set of passford was found
     */
    bool pass_found;
    /**
     * Last speed of current thread
     */
    uint64_t last_speed;
#ifdef WRATHION_MPI
    bool mpi_enabled;
    int mpi_proc_id;
#endif
private:
    /**
     * Clock for measuring speed
     */
    struct timespec speedClock;
    

};

#endif	/* CRACKER_H */

