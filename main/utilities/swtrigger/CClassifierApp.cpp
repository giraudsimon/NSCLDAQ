/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CClassifierApp.cpp
 *  @brief: Implement common code shared by all classifier apps:
 */
#include "CClassifierApp.h"
#include <dlfcn.h>
#include <string>
#include <stdexcept>

/**
 * getClassifierFactory
 *    Returns a pointer to the classfier factory in the users's --classifier
 *    library.
 *
 *    This is an extern "C" function called createClassifier
 *    it must have the signatrure:
 *
 * \verbatim
 *       CRingMarkingWorker::Classifier* createClassifier();
 * \endverbatim
 *
 *   @return pointer to the factory function.
 *   @throw std::runtime_error - if we can't get that function for whatever
 *                               reason.
 */
CClassifierApp::ClassifierFactory
CClassifierApp::getClassifierFactory()
{
    void* soHandle = dlopen(m_params.classifier_arg, RTLD_NOW |RTLD_GLOBAL);
    if (!soHandle) {
        std::string error = dlerror();
        std::string msg   = "Failed to open shared library: ";
        msg += m_params.classifier_arg;
        msg += " ";
        msg += error;
        throw std::runtime_error(msg);
    }
    dlerror();                         // Clear errors (from manpage example).
    void* rawFactory = dlsym(soHandle, "createClassifier");
    char* error = dlerror();
    if (error != nullptr) { 
        std::string msg = "Unable to locate 'createClassifier' in  ";
        msg += m_params.classifier_arg;
        msg += " ";
        msg += error;
        msg += " be sure it's delcared extern \"C\"";
        throw std::runtime_error(msg);
    }
    
    ClassifierFactory result = reinterpret_cast<ClassifierFactory>(rawFactory);
    return result;
}