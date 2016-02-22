/* 
 * File:   Utils.h
 * Author: Honza
 *
 * Created on 10. duben 2014, 13:07
 */

#ifndef UTILS_H
#define	UTILS_H

#include <cstdint>
#include <string>

class Utils {
public:
    /**
     * Returns modification time of file
     * @param filename file to check
     * @return timestamp
     */
    static uint64_t getFileModificationTS(std::string &filename);
    /**
     * Computes x^y
     * @param x
     * @param y
     * @return 
     */
    static uint64_t pow(uint64_t x, uint64_t y);
};

#endif	/* UTILS_H */

