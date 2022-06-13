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
#include "CCAENV775Creator.h"    	
#include "CReadableObject.h"
#include "CCAENV775.h"	
#include <assert.h>
#include <string>	

using namespace std;

	
/*!
   Construct an instance of a CAEN V 775 module creator.  The module
   creator can then be registered withy CModuleCommand::AddCreator.
   */
CCAENV775Creator::CCAENV775Creator ()
{
 m_helpText = "caenv755 - Creates a CAEN V 775 module.";
} 

/*
   Destroys an instance of a V775 creator.
   */
 CCAENV775Creator::~CCAENV775Creator ( ) 
{
}
/**
 * Create
 *    Creates the new object.
 * @param name -name of the module - which is also a new Tcl command.
 * @param interp - references the interpreter on which the module is registered.
 * @return CReadableObject* - The new module object.
 */
CReadableObject*
CCAENV775Creator::Create(const char* name, CTCLInterpreter& interp)
{
 return new CCAENV775(name, interp);
}
