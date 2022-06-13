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

static const char* Copyright = "(C) Ron Fox 2002, All rights reserved";
////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CCAENV792Creator.h" 
#include "CReadableObject.h"   	
#include "CCAENV792.h"	
#include <assert.h>
#include <string>	

// Completely rewritten for daqdev/NSCLDAQ#510 - use extensible factory pattern.

/**
 * consrutor
 *   - just build the help string.
 */
CCAENV792Creator::CCAENV792Creator()
{
 m_helpText = "v792 - Creates a CAEN V 792 module.";
}
/**
 * Create
 *    Return a newly created object.
 * @param name - module and module command name.,
 * @param interp - reference to interpreter object on which the command is created.
 * @return CReadoutModule* the created module.
 */
CReadableObject*
CCAENV792Creator::Create(const char* name, CTCLInterpreter& interp)
{
 return new CCAENV792(name, interp);
}