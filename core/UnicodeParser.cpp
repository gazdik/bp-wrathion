/* 
 * Copyright (C) 2014 Radek Hranicky
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
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include "UnicodeParser.h"

UnicodeParser::UnicodeParser() {
}

bool UnicodeParser::processFile(std::string fileName, int* chars_count) {
    int charPos = 0;
    std::string line;
    std::ifstream infile(fileName);
    if (!infile.is_open())
    {
        std::cerr << "Error opening file with Unicode character definition." << std::endl;
        return false;
    }
    while (std::getline(infile, line))
    {
        uint32_t uc_symbol = 0;
        std::size_t found = line.find_first_of('#');    // remove # comments
        if (found != std::string::npos) {
            line = line.substr(0, found);
        }
        line = trim(line);                              // trim input
        if (found != std::string::npos) {
            line = line.substr(0, found);
        }
        if (line.length() == 0) {                       // skip blank lines
            continue;
        }
        if (line.length() >= 2) {                       // remove U+ u+ 0x prefixes
            std::string beg = line.substr(0, 2);
            if (beg == "U+" || beg == "u+" || beg == "0x") {
                line = line.substr(2, line.length() - 2);
            }
            if (line.length() == 0) {
                continue;
            }
        }
        // Check for forbidden characters
        if (line.find_first_not_of("0123456789ABCDEF") != std::string::npos) {
            std::cerr << "Error - not a correct HEX format: " << line << std::endl;
            return false;
        }
   
        // Convert HEX format to uint32_t (UTF-32)
        std::stringstream ss;
        ss << std::hex << line;
        ss >> uc_symbol;
        uc_chars[charPos] = uc_symbol;
        charPos++;
        if (charPos == MAX_CHARS) {
            std::cerr << "Wrathion does not support more than " << MAX_CHARS << " Unicode symbols!" << std::endl;
            std::cerr << "To increase maximum number of characters modify MAX_CHARS macro" << std::endl;
            std::cerr << "in include/UnicodeParser.h and compile Wrathion again." << std::endl;
            return false;
        }
    }
    *chars_count = charPos;
    return true;
}

std::string UnicodeParser::trim(const std::string input) {
    const std::string whitespace = " \t";
    const auto strBegin = input.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return "";
    const auto strEnd = input.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    return input.substr(strBegin, strRange);
}

uint32_t* UnicodeParser::getCharsPtr() {
    return uc_chars;
}
