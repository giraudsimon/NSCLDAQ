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
static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";
/*! \class CCAENV785Creator   
	Implementation of CCAENV785Creator.  See the .h file for more information.
*/

////////////////////////// FILE_NAME.cpp ////////////////////////////////////////
#include <config.h>
#include "CCAENV785Creator.h"   
#include "CReadableObject.h"
#include "CCAENV785.h"


#include <assert.h>


/**
 * Completely rewritten per daqdev/NSCLDAQ#510 - use base/factories extensible
 * factories rather than custom factory.
 */

/**
 * constructor
 *    Just set the help text:
 */
CCAENV785Creator::CCAENV785Creator()
{
	m_helpText = "v785 - Creates a CAEN V785 module.";
}
/**
 * destructor
 *   for now nothing.
 */
CCAENV785Creator::~CCAENV785Creator()
{
}
/**
 * Create the new module.
 *
 *  @param name - name of the module.
 *  @param interp - interpreter on which the module command is installed.
 *  @return CReadabelObject* Pointer to the created object.
 */
CReadableObject*
CCAENV785Creator::Create(const char* name, CTCLInterpreter& interp)
{
	return new CCAENV785(name, interp);
}

