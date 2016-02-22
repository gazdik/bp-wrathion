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
 
#ifndef UCPARSER_H
#define	UCPARSER_H

#define	MAX_CHARS 256

#include <string>

/**
 * Base class for Unicode input file parser
 */
class UnicodeParser {
public:
    UnicodeParser();
    /**
     * Get next password.
     * @param file with definition of unicode characters used for password generation
     */
    bool processFile(std::string fileName, int* chars_count);
    
    /**
     * Returns a pointer to unicode characters
     */
    uint32_t* getCharsPtr();
    
    /**
     * Trim - Removes leading and ending whitespace from a string
     * @param input string
     * @return trimmed string
     */
    std::string trim(const std::string input);
    
    
protected:
    uint32_t uc_chars[MAX_CHARS];
    

};

#endif	/* UCPARSER_H */

