/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include "CV1x90Creator.h"
#include "CV1x90Module.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>

// Rewrite for daqdev/NSCLDAQ#510 - recast in the extensible factory pattern.

/*!
  Constructing the object delegates:
  
*/
CV1x90Creator::CV1x90Creator() 
  
{
  m_helpText = "v1x90 - Creates a CAEN V1190 or CAEN V1290 module";    
}
CV1x90Creator::~CV1x90Creator()
{
  
}

/**
 * Create
 *     Create  new module.
 * @param name - name of the module - will also be its instance command.
 * @param interp -interpreter on which the instance command is registered.
 * @return CReadableObject* pointer to the new module object.
 */
CReadableObject*
CV1x90Creator::Create(const char* name, CTCLInterpreter& interp)
{
  return new CV1x90Module(name, interp);
}
