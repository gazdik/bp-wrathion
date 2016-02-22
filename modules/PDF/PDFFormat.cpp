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

#include <iostream>
#include <sstream>
#include  <iomanip>
using namespace std;

#include "PDFFormat.h"
#include "CrackerFactoryTemplate.tcc"
#include "PDFCrackerCPU.h"
#include "PDFCrackerGPU.h"
#include <poppler/PDFDocFactory.h>

typedef CrackerFactoryTemplate<PDFCrackerCPU,PDFInitData> PDFCrackerCPUFactory;
typedef CrackerFactoryTemplate<PDFCrackerGPU,PDFInitData, true> PDFCrackerGPUFactory;

PDFFormat::PDFFormat():FileFormat() {
    signature = "%PDF-";
    ext = "pdf";
    name = "PDF";
}

PDFFormat::PDFFormat(const PDFFormat& orig):FileFormat(orig) {
}

PDFFormat::~PDFFormat() {
}

CrackerFactory* PDFFormat::getCPUCracker(){
    return new PDFCrackerCPUFactory(data);
}

CrackerFactory* PDFFormat::getGPUCracker(){
    return new PDFCrackerGPUFactory(data);
}

void PDFFormat::init(std::string& filename){
    PDFDoc *doc = PDFDocFactory().createPDFDoc(GooString(filename.c_str()));
    
    //GBool enc = doc->isEncrypted();
    GooString ID;

    Object obj,id_obj;
    Dict* trailer = doc->getXRef()->getTrailerDict()->getDict();
    trailer->lookupNF("ID",&obj);
    
    obj.getArray()->getNF(0,&id_obj);
    data.ID1.assign(id_obj.getString()->getCString(),16);
    
    obj.getArray()->getNF(1,&id_obj);
    data.ID2.assign(id_obj.getString()->getCString(),16);
    
    GBool enc = trailer->hasKey("Encrypt");
    if(enc){
        is_encrypted = true;
        trailer->lookupNF("Encrypt",&obj);
        Object encObj;
        doc->getXRef()->fetch(obj.getRef().num,obj.getRef().gen,&encObj);
        encObj.getDict()->lookupNF("Filter",&obj);
        encObj.getDict()->lookupNF("R",&obj);
        this->data.R = obj.getInt();
        encObj.getDict()->lookupNF("V",&obj);
        this->data.V = obj.getInt();
        encObj.getDict()->lookupNF("P",&obj);
        this->data.P = obj.getInt();
        encObj.getDict()->lookupNF("Length",&obj);
        this->data.length = obj.getInt();
        encObj.getDict()->lookupNF("O",&obj);
        if ((int)this->data.R <= 4) {
            /* Revision 4 or less => 32Byte O string */
            this->data.O.assign(obj.getString()->getCString(),32);
        } else {
            /* Revision 5 or 6 => 48Byte O string */
            this->data.O.assign(obj.getString()->getCString(),48);
            ::memcpy(this->data.O_valid_salt, this->data.O.substr(32, 8).c_str(), 8);
            ::memcpy(this->data.O_key_salt, this->data.O.substr(40, 8).c_str(), 8);
            this->data.O.resize(32);
        }
        encObj.getDict()->lookupNF("U",&obj);
        if ((int)this->data.R <= 4) {
            /* Revision 4 or less => 32Byte U string */
            this->data.U.assign(obj.getString()->getCString(),32);
        } else {
            /* Revision 5 or 6 => 48Byte U string */
            this->data.U.assign(obj.getString()->getCString(),48);
            ::memcpy(this->data.U_valid_salt, this->data.U.substr(32, 8).c_str(), 8);
            ::memcpy(this->data.U_key_salt, this->data.U.substr(40, 8).c_str(), 8);
            this->data.U.resize(32);
        }
        
        if(encObj.getDict()->hasKey("EncryptMetadata")){
            encObj.getDict()->lookupNF("EncryptMetadata", &obj);
            this->data.MetaEncrypted = obj.getBool();
        }
        
        int v_major = doc->getPDFMajorVersion();
        int v_minor = doc->getPDFMinorVersion();
        
        if (verbose) {
            // Print PDF encryption information from EncObj
            std::cout << "======= PDF information =======" << std::endl;
            std::cout << "PDF version: " << v_major << "." << v_minor << std::endl;
            if ((int)this->data.R == 5) {
                std::cout << "Extension level 3 detected." << std::endl;
            } else if ((int)this->data.R == 6) {
                std::cout << "Extension level 5 detected." << std::endl;
            } else if ((int)this->data.R > 6) {
                std::cout << "Warning: Unknown (unsupported) security revision!" << std::endl;
            }
            std::cout << "Security revision: " << (int)this->data.R << std::endl;
            std::cout << "Encryption alg. version: " << (int)this->data.V << std::endl;
            std::cout << "Key length: " << (int)this->data.length << std::endl;
            std::cout << "Metadata encrypted: ";
            if (this->data.MetaEncrypted) {
                std::cout << "yes";
            } else {
                std::cout << "no";
            }
            std::cout << std::endl;
            std::cout << "===============================" << std::endl;
        }
            
            
    }else{
        is_encrypted = false;
    }
    is_supported = true;

}

PDFInitData::PDFInitData(const PDFInitData& orig){
    this->ID1 = orig.ID1;
    this->ID2 = orig.ID2;
    this->O = orig.O;
    ::memcpy(this->O_valid_salt, orig.O_valid_salt, 8);
    ::memcpy(this->O_key_salt, orig.O_key_salt, 8);
    this->P = orig.P;
    this->R = orig.R;
    this->U = orig.U;
    ::memcpy(this->U_valid_salt, orig.U_valid_salt, 8);
    ::memcpy(this->U_key_salt, orig.U_key_salt, 8);
    this->V = orig.V;
    this->length = orig.length;
    this->MetaEncrypted = orig.MetaEncrypted;
}

PDFInitData::PDFInitData(){
    this->MetaEncrypted = true;
}

