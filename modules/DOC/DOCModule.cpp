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
#include "DOCFormat.h"

class DOCModule: public Module{
public:

    virtual std::string getName(){
        return "DOC";
    }

    virtual std::string getAuthor(){
        return "Jan Schmied";
    }
    

    virtual int getBuild(){
        return 1;
    }

    virtual std::string getDescription(){
        return "Module supports cracking DOC files encrypted with MS RC4 cipher";
    }
    

    virtual FileFormat* getFileFormat(){
        return new DOCFormat();
    }
    

    virtual std::string getVersionText(){
        return "1.0";
    }

};

#ifdef WRATHION_DLL_MODULES
WRATHION_MODULE(DOCModule)
#endif