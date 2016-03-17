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

#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <vector>
//#include <ctime>
#include <sys/time.h>

#include "FormatPool.h"

#ifndef WRATHION_DLL_MODULES //compile modules directly 
#include "PDFFormat.h"
#include "ZIPFormat.h"
#include "DOCFormat.h"
#endif

#include "GPUCracker.h"
#include "CrackerRunner.h"
#include "Module.h"
#include <csignal>
#include <ctime>
#include "UnicodeParser.h"
#include "MarkovPassGen.h"

#ifdef WRATHION_MPI
#include <mpi.h>
#endif

/*
 * Maximum size of possible UTF8 characters per each UTF32 character
 * (For the purpose of UnicodePassGen)
 */
#define UTF8_CHAR_MAXSIZE 4

#define CLOCKS_PER_MSEC (CLOCKS_PER_SEC/1000)
#define CLOCKS_PER_USEC (CLOCKS_PER_SEC/1000000)

using namespace std;

const string help = "wrathion [OPTIONS]\n"
"    -h - prints this help\n"
"    -f - input file\n"
"    --modules -l - list loaded modules\n"
"    --devices -s - list platforms and devices\n"
"    --cpu -c - prefer CPU over GPU\n"
"    --map -d - devices to use <platform>:<device>[:<GWS>][,<platform>:<device>[:<GWS>],...]\n"
"    --chars -p - chars for creating passwords (default: abcdefghijklmnopqrstuvwxyz)\n"
"    -u (alternative of --chars) - file with unicode characters in hex form\n"
"    -m - maximum length of password (default: 10)\n"
"    --dict=file, -r - dictionary for dictinary attack\n"

#ifdef WRATHION_MPI
"    --mpi - run in MPI mode\n"
#endif
"    --threads=NUMTHREADS, -t - number of threads for CPU Cracking\n"
"    -v - verbose mode (more information is displayed)\n"
"\nMarkov attack\n"
"    --stat=file - stat file for Markov attack\n"
"    --model=type - type of Markov model:\n"
"           - classic - First-order Markov model (default)\n"
"           - layered - Layered Markov model\n"
"    --threshold=value - number of characters per position (default 5)\n"
"    --min=value - minimal length of password (default 1)\n"
"    --max=value - maximal length of password (default 64)\n"
"    --limits - comma-sepparated threshold values\n"
"    --mask=mask - mask\n";

struct opts : MarkovPassGenOptions {
    opts():
        help(false),
        show_modules(false),
        show_devices(false),
        prefer_cpu(false),
        max_pass_len(10),
        stdin_mode(true),
#ifdef WRATHION_MPI       
        mpi(false),
#endif
        verbose(false)
    {}
    bool help;
    string input_file;
    bool show_modules;
    bool show_devices;
    bool prefer_cpu;
    string devices_mapping;
    char chars[256] = {0};
    UnicodeParser unicodeParser;
    int max_pass_len = 10;
    int threads = 0;
    string dict;
    string unicode_file;
    bool stdin_mode;
#ifdef WRATHION_MPI
    bool mpi;
#endif
    bool verbose;
};

bool stop = false;

void sigint_handler (int param)
{
  stop = true;
  cout << "CTRL+C catched" << endl;
}


inline long long int getMilliSecs() { 
    timeval t;    
    gettimeofday(&t, NULL);
    return (t.tv_sec) * 1000 + (t.tv_usec) / 1000;
}


/*
 * 
 */
int main(int argc, char** argv) {
    long long int starttime, elapsedtime;
    if (argc < 2){
        cout << help;
        return 0;
    }
    int opt,opt_index;
    opts o;
    static struct option long_options[] =
             {
               {"modules", no_argument, 0, 'l'},
               {"devices", no_argument, 0, 's'},
               {"dict",    required_argument, 0, 'r'},
               {"threads",  required_argument, 0, 't'},
               {"cpu",  no_argument, 0, 'c'},
               {"map",  required_argument, 0, 'd'},
               {"chars",    required_argument, 0, 'p'},
#ifdef WRATHION_MPI       
               {"mpi", no_argument, 0, 'z'},
#endif
							 // Markov attack
							 {"stat", required_argument, 0, 1},
							 {"threshold", required_argument, 0, 2},
							 {"min", required_argument, 0, 3},
							 {"max", required_argument, 0, 4},
							 {"model", required_argument, 0, 5},
							 {"mask", required_argument, 0, 6},
							 {"limits", required_argument, 0, 7},
               {0, 0, 0, 0}
             };
    while ((opt = getopt_long(argc, argv, "hf:lscd:p:u:r:t:vm:", long_options,&opt_index)) != -1){
        switch(opt){
        	  case 1:
        	  	o.stat_file = optarg;
        	  	break;
        	  case 2:
        	  	o.threshold = atoi(optarg);
        	  	break;
        	  case 3:
        	  	o.min_length = atoi(optarg);
        	  	break;
        	  case 4:
        	  	o.max_length = atoi(optarg);
        	  	break;
        	  case 5:
        	  	o.model = optarg;
        	  	break;
        	  case 6:
        	  	o.mask = optarg;
        	  	break;
        	  case 7:
        	  	o.limits = optarg;
        	  	break;
            case 'h':
                o.help = true; break;
            case 'f':
                o.input_file.assign(optarg); break;
            case 'l':
                o.show_modules = true; break;
            case 's':
                o.show_devices = true; break;
            case 'c':
                o.prefer_cpu = true; break;
            case 'd':
                o.devices_mapping.assign(optarg); break;
            case 'p':
                ::strcpy(o.chars,optarg); break;
            case 'u':
                o.unicode_file.assign(optarg); break;
            case 'm':
                o.max_pass_len = atoi(optarg); break;
            case 'r':
                o.dict.assign(optarg); break;
            case 't':
                o.threads = atoi(optarg); break;
            case 'v':
                o.verbose = true; break;
#ifdef WRATHION_MPI  
            case 'z':
                o.mpi = true; break;
#endif                  
        }
    }
    
    if (o.chars[0] != 0 && !o.unicode_file.empty()) {
        cout << "Please specify either -p or -u, not both." << endl;
        return 0;
    }
    
    signal(SIGINT, sigint_handler);
    
    if(o.chars[0] == 0){
        ::strcpy(o.chars,"abcdefghijklmnopqrstuvwxyz");
    }
    
    FormatPool pool;
    if (o.verbose){
        pool.setVerbose();
    }
    
#ifdef WRATHION_DLL_MODULES //load modules form shared libs 
    vector<Module*> modules = Module::loadModules();
    for(int i = 0;i<modules.size();i++){
        pool.addFormat(modules[i]->getFileFormat());
    }
#else
    pool.addFormat(new PDFFormat());
    pool.addFormat(new ZIPFormat());
    pool.addFormat(new DOCFormat());
#endif   
    bool run = false;
    FileFormat* format = NULL;
    CrackerFactory *crackerFactory = NULL;
    PassGen *passgen = NULL;
    CrackerRunner runner;
    
    if (o.verbose){
        runner.setVerbose();
    }
            
    if(o.help){
        cout << help;
        return 0;
    }
    if(o.show_modules){
        Module::printLoadedModules(&cout);
        return 0;
    }
    if(o.show_devices){
        cout << GPUCracker::availableDevices();
        GPUCracker::destroyOpenCL();
        return 0;
    }
    if(!o.input_file.empty()){
        format = pool.getFileFormat(o.input_file); // Detection of file format
        if(format == NULL){
            cout << "No suitable format found" << endl;
        }else{
            if (!format->isEncrypted()) {
                cout << "This file is NOT encrypted." << endl;
                return 0;
            }
            if(o.prefer_cpu){
                cout << "Trying to get CPU cracker" << endl;
                crackerFactory = format->getCPUCracker();
                if(crackerFactory == NULL){
                    cout << "CPU cracker not found" << endl << "Trying to get GPU cracker" << endl;
                    crackerFactory = format->getGPUCracker();
                    if(crackerFactory == NULL){
                        cout << "GPU cracker not found" << endl << "No suitable cracker found" << endl << "Bye" << endl;
                    }else{
                        cout << "Using GPU cracker" << endl;
                        run = true;
                    }
                }else{
                    cout << "Using CPU cracker" << endl;
                    run = true;
                }
            }else{
                cout << "Trying to get GPU cracker" << endl;
                crackerFactory = format->getGPUCracker();
                if(crackerFactory == NULL){
                    cout << "GPU cracker not found" << endl << "Trying to get CPU cracker" << endl;
                    crackerFactory = format->getCPUCracker();
                    if(crackerFactory == NULL){
                        cout << "CPU cracker not found" << endl << "No suitable cracker found" << endl << "Bye" << endl;
                    }else{
                        cout << "Using CPU cracker" << endl;
                        run = true;
                    }
                }else{
                    cout << "Using GPU cracker" << endl;
                    run = true;
                }
            }
        }
    }else{
        cout << "No input file" << endl;
        return 0;
    }
    
    // In case of GPU cracker we need additional 1 uchar point for current password length
    if (run && crackerFactory->isGPU()) {
        o.max_pass_len++;
    }
    
    if(run){
        if (!o.unicode_file.empty()) {
            int chars_count;
            if (!o.unicodeParser.processFile(o.unicode_file, &chars_count)) {
                return 1;
            }
            o.unicodeParser.getCharsPtr();
            passgen = new UnicodePassGen(o.unicodeParser.getCharsPtr(), chars_count, o.max_pass_len*UTF8_CHAR_MAXSIZE, o.max_pass_len);
        } else if (!o.dict.empty()){
            passgen = new DictionaryPassGen(o.dict);
        } else if (not o.stat_file.empty()) {
        	passgen = new MarkovPassGen(o);
        } else {
            passgen = new ThreadedBrutePassGen(o.chars, o.max_pass_len);
        }
        passgen->loadState(o.input_file+".passgen");
        
        if(o.devices_mapping != ""){
            // In the case we have any manually mapped device
            std::vector<DeviceConfig> confs = CrackerRunner::decodeConfig(o.devices_mapping);
            if(confs.size() > 0){
                runner.setDeviceConfig(confs);
            }
        }
        
        runner.setCrackerFactory(crackerFactory);  // set cracker factory
        runner.setPassGen(passgen);                // set password generator
        if(o.threads > 0){
            runner.setNumThreads(o.threads);
        }
        // Time measurement
        if (o.verbose){
            starttime = getMilliSecs();
        }
        //
        runner.start();
        cout << "Running " << runner.getNumThreads() << " threads" << endl;
        CrackerRunner::sleep(1000);
        while(runner.someRunning() && !stop){
            cout << "\r";
            vector<uint64_t> *speeds = runner.getSpeeds();
            uint64_t sum = 0;
            uint32_t n = 1;
            cout << "Speed: ";
            for(vector<uint64_t>::iterator i = speeds->begin();i != speeds->end();i++){
                cout << "Thread " << n << ": " << *i << " p/s, ";
                sum += *i;
                n++;
            }
            cout << "Total: " << sum << " p/s";
            CrackerRunner::sleep(1000);
        }
        // Write elapsed time
        if (o.verbose){
            elapsedtime =  getMilliSecs() - starttime;
            cout << "Total time spent by cracking:" << endl;
            cout << (int)(elapsedtime/60000) << "m "
                 << ((double)(elapsedtime % 60000) / 1000.0) << "s "
                 << endl;
        }
        //
        cout << endl;
        if(runner.passFound()){
            cout << "Password found: '" << runner.getPassword() << "'" << endl;
        }else{
            if(stop){
                cout << "Saving generator state" << endl;
                runner.stop();
                passgen->saveState(o.input_file+".passgen");
            }else{
                cout << "No password found" << endl;
            }
        }
        CrackerRunner::sleep(1000);
        delete passgen;
        delete crackerFactory;
    }
    GPUCracker::destroyOpenCL();
    return 0;
}

