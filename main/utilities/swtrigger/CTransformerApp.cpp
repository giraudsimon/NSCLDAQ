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

/** @file:  CTransformerApp.cpp
 *  @brief: Implements the non-trivial utility methods of the  application
 *          (common code between all application classes).
 */

#include "CTransformerApp.h"
#include <dlfcn.h>
#include <string>
#include <stdexcept>


/**
 * getExtenderFactory
 *     - Loads the user .so
 *     - Locates the factory method in the .so and returns it.
 *
 * @return pointer to the factory funcstion
 * @throw  std::runtime_error for various error conditions.
 */
ExtenderFactory
CTransformerApp::getExtenderFactory()
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
    void* rawFactory = dlsym(soHandle, "createExtender");
    char* error = dlerror();
    if (error != nullptr) {
        std::string msg = "Unable to locate 'createExtender' in  ";
        msg += m_params.classifier_arg;
        msg += " ";
        msg += error;
        msg += " be sure it's delcared extern \"C\"";
        throw std::runtime_error(msg);
    }

    ExtenderFactory result = reinterpret_cast<ExtenderFactory>(rawFactory);
    return result;
   
}
