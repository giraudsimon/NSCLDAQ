/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";
/*! \class CModuleCreator  abstract 
           CLASS_PACKAGE
           Abstract base class of a set of factory like (creational) modules
           that make data acquisition module classes.  Each of these has
           a string module type.  And the following key members:
           - Match - return true if an input string matches the module type.
           - Create - Returns a new module.
           - Help    - Returns stringified help about the type of module
                          created by this class.
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CModuleCreator.h"    	
#include "CReadableObject.h"
#include <TCLInterpreter.h>

/**
 * Completely rewritten for daqdev/NSCLDAQ#510 - in terms of extensible
 * factory.
 */

/**
 * constructor
 *    Construct this base class.
 */
CModuleCreator::CModuleCreator()
{}
/**
 * virtual destructor to allow destructor chaining.
 */
CModuleCreator::~CModuleCreator()
{}

/**
 * operator()
 *    The extensible factory calls this to create a new object
 *    for this type.  We do the impedance matching needed to invoke
 *    the Create method.
 *
 * @param userData - actually a std::pair<const char*, CTCLInterpreter*>*
 *      The character is the module name/command while the interpreter is the
 *      interpreter in which the command representing the module is registered.
 * @return CReadableObject* - the return value from Create.
 */
CReadableObject*
CModuleCreator::operator()(void* userData)
{
  std::pair<const char*, CTCLInterpreter*>* pData =
   reinterpret_cast<std::pair<const char*, CTCLInterpreter*>*>(userData);
   
  const char*      pName   = pData->first;
  CTCLInterpreter* pInterp = pData->second;
  return Create(pName, *pInterp);
}
/**
 * describe
 *    Returns text to describe the thingy we create.  Derived classes can
 *    build this up and store it in the protected m_helpText string.
 *  @return std::string
 */
std::string
CModuleCreator::describe() const
{
 return m_helpText;
}