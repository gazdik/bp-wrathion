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

#ifndef MODULE_H
#define	MODULE_H

#include "FileFormat.h"
#include <vector>
#include <string>
#include <ostream>
#ifdef __WIN32
#include <windows.h>
#define LIBRARY_EXT "dll"
#define LIBRARY_HANDLE HMODULE
#define LIBRARY_OPEN(name) LoadLibrary(name)
#define LIBRARY_FUNC(lib, name) GetProcAddress(lib, name)
#define LIBRARY_FREE(lib) FreeLibrary(lib)
#else
#include <dlfcn.h>
#define LIBRARY_EXT "so"
#define LIBRARY_HANDLE void*
#define LIBRARY_OPEN(name) dlopen(name,RTLD_LAZY)
#define LIBRARY_FUNC(lib, name) dlsym(lib, name)
#define LIBRARY_FREE(lib) dlclose(lib)
#endif

#define WRATHION_MODULE_API_VERSION 1

#define WRATHION_MODULE(MODCLS)                 \
extern "C" {                                   \
    void* getModule(){   \
        return new MODCLS();                   \
    }                                          \
                                               \
    int getAPIVersion(){ \
        return WRATHION_MODULE_API_VERSION;     \
    }                                          \
}                                              \



/**
 * Class representing interface for writing plug-in cracking module
 */
class Module {
public:
    /**
     * Gets module name
     * @return name
     */
    virtual std::string getName() = 0;
    /**
     * Gets module authors name
     * @return string authors name
     */
    virtual std::string getAuthor() = 0;
    /**
     * Gets text representation of module version
     * @return 
     */
    virtual std::string getVersionText() = 0;
    /**
     * Gets build number of module 
     * @return 
     */
    virtual int getBuild() = 0;
    /**
     * Gets text description of module
     * @return 
     */
    virtual std::string getDescription() = 0;
    /**
     * Gets FileFormat object created by this module
     * @return 
     */
    virtual FileFormat* getFileFormat() = 0;
    /**
     * Loads all available cracking modules 
     * @return 
     * @note used only in core cracking application
     */
    static std::vector<Module*> loadModules();
    
    /**
     * Reslease all loaded modules
     */
    static void unloadModules();
    
    /**
     * Prints info form loaded modules
     * @param stream where to send output
     */
    static void printLoadedModules(std::ostream *stream);
    /**
    * type for definition of C funtion for loading module API version
    */
    typedef int (*APIVerFn)();
    /**
    * type for definition of C funtion for loading module
    */
    typedef Module* (*ModuleFn)();
private:
    /**
     * filename of module 
     */
    std::string filename;
    
    /**
     * Loads module from file 
     * @param filename filename of .dll or .so library
     * @return 
     * @note used only in core cracking application
     */
    static Module* loadModule(std::string filename);
    
    /**
     * Pointers to loaded Modules
     */
    static std::vector<Module*> loadedModules;
    
    /**
     * Handles to loaded modules for system unload
     */
    static std::vector<LIBRARY_HANDLE> loadedLibraries;


};

#endif	/* MODULE_H */

