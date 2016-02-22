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

#include "Module.h"
#include <dirent.h>
#include <iostream>
#include <string>

std::vector<Module*> Module::loadModules() {
    std::vector<Module*> res;
    std::string modulesDir = ".";
    Module *mod;
    DIR *dir = opendir(modulesDir.c_str()); 
    std::string filename;
    if(dir){ 
        struct dirent *ent; 
        while((ent = readdir(dir)) != NULL){ 
            filename.assign(ent->d_name);
            std::string ext = filename.substr(filename.find_last_of(".")+1);
            if(filename.find("_module_") != std::string::npos && ext == LIBRARY_EXT){
                mod = loadModule(modulesDir+"/"+filename);
                if(mod != NULL){
                    mod->filename = filename;
                    res.push_back(mod);
                    loadedModules.push_back(mod);
                }
            }
        } 
    }
    return res;
}

void Module::printLoadedModules(std::ostream* stream) {
    for(std::vector<Module*>::iterator i = loadedModules.begin();i!= loadedModules.end();i++){
        Module *curMod = *i;
        (*stream) << "Module '" << curMod->getName() << "'" << std::endl;
        (*stream) << "  Loaded from file: '" << curMod->filename << "'" << std::endl;
        (*stream) << "  Author: " << curMod->getAuthor() << std::endl;
        (*stream) << "  Version: " << curMod->getVersionText() << " (" << curMod->getBuild() << ")" << std::endl;
        (*stream) << "  Description: " << curMod->getDescription() << "" << std::endl;
    }
}

    
Module* Module::loadModule(std::string filename){
    LIBRARY_HANDLE library = LIBRARY_OPEN(filename.c_str());
    APIVerFn apiverfn = NULL;
    ModuleFn modulefn = NULL;
    apiverfn = reinterpret_cast<APIVerFn>(LIBRARY_FUNC(library,"getAPIVersion"));
    if(apiverfn == NULL){
        LIBRARY_FREE(library);
        return NULL;
    }
    if(apiverfn() != WRATHION_MODULE_API_VERSION){
        LIBRARY_FREE(library);
        return NULL;
    }
    modulefn = reinterpret_cast<ModuleFn>(LIBRARY_FUNC(library,"getModule"));
    if(modulefn == NULL){
        LIBRARY_FREE(library);
        return NULL;
    }
    loadedLibraries.push_back(library);
    Module *mod = modulefn();
    return mod;
}

void Module::unloadModules() {
    for(int i = 0;i<loadedLibraries.size();i++){
        LIBRARY_FREE(loadedLibraries[i]);
    }
    for(int i = 0;i<loadedModules.size();i++){
        delete loadedModules[i];
    }
    loadedModules.clear();
}

std::vector<LIBRARY_HANDLE> Module::loadedLibraries;

std::vector<Module*> Module::loadedModules;




