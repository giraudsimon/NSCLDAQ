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

/** @file:  CBuiltRingItemEditorApp.cpp
 *  @brief: Common code for all editor applications
 */

#include "CBuiltRingItemEditorApp.h"
#include <dlfcn.h>
#include <string>
#include <stdexcept>

/**
 ** getEditorFactory
 **    Return the factory for editor objects from the user's
 **    shared object.
 **/
EditorFactory
CBuiltRingItemEditorApp::getEditorFactory()
{
  void* soHandle = dlopen(m_args.classifier_arg, RTLD_NOW |RTLD_GLOBAL);
  if (!soHandle) {
    std::string error = dlerror();
    std::string msg   = "Failed to open shared library: ";
    msg += m_args.classifier_arg;
    msg += " ";
    msg += error;
    throw std::runtime_error(msg);
  }
  dlerror(); // Clear errors (from manpage example).
  void* rawFactory = dlsym(soHandle, "createEditor");
  char* error = dlerror();
  if (error != nullptr) {
    std::string msg = "Unable to locate 'createEditor' in  ";
    msg += m_args.classifier_arg;
    msg += " ";
    msg += error;
    msg += " be sure it's delcared extern \"C\"";
    throw std::runtime_error(msg);
  }

  EditorFactory result = reinterpret_cast<EditorFactory>(rawFactory);
  return result;   
}
