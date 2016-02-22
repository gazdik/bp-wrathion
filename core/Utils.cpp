/* 
 * File:   Utils.cpp
 * Author: Honza
 * 
 * Created on 10. duben 2014, 13:07
 */


#include "Utils.h"
#ifdef __WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#endif


uint64_t Utils::getFileModificationTS(std::string& filename) {
#ifdef __WIN32
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    FILETIME ftCreate, ftAccess, ftWrite;
    
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
        return 0;
    
    CloseHandle(hFile);
    
    uint64_t res;
    res = ftWrite.dwHighDateTime;
    res <<= 32;
    res |= ftWrite.dwLowDateTime;
    return res;
#else
    uint64_t res;	
    struct stat attrib;
        
    res = stat(filename.c_str(), &attrib);
    res = attrib.st_mtim.tv_sec;
    return res;
#endif
}

uint64_t Utils::pow(uint64_t x, uint64_t y) {
    if(y == 0)
        return 1;
    else
        return Utils::pow(x,y-1);
}


