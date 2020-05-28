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
	    
*/
#include <config.h>
#include "CSIS3300Creator.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include "CSIS3300Module.h"


/*!
   Construct the class, we just need to construct the base class.
*/
CSIS3300Creator::CSIS3300Creator()
{
  m_helpText  =  "csis3300 - Struck (SIS) 3300/3301 flash adc";
}

/*!
   Destruction is a nooop supplied to complete the chain of
   virtual destructors back to the base class.
*/
CSIS3300Creator::~CSIS3300Creator()
{}

/**
 * Create
 *    Creates a new instance.
 * @param name - name of the module and its instance command.
 * @param interp - interpreter the command is registered on.
 * @return CReadoutObject* - pointer to new CSIS3300Module object.
 */
CReadableObject*
CSIS3300Creator::Create(const char* name, CTCLInterpreter& interp)
{
  return new CSIS3300Module(name, interp);
}