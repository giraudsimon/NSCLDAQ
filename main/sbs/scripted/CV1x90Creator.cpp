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

using namespace std;

/*!
  Constructing the object delegates:
  \parm type - the type of object we create.
*/
CV1x90Creator::CV1x90Creator(string type) :
  CModuleCreator(type)
{}

// Pretty much all the remaining canonicals delegate or are trivial.

CV1x90Creator::CV1x90Creator(const CV1x90Creator& rhs) :
  CModuleCreator(rhs) {}

CV1x90Creator::~CV1x90Creator() {}
CV1x90Creator& 
CV1x90Creator::operator=(const CV1x90Creator& rhs)
{
  CModuleCreator::operator=(rhs);
  return *this;
}
int
CV1x90Creator::operator==(const CV1x90Creator& rhs) const
{
  return CModuleCreator::operator==(rhs);
}
int
CV1x90Creator::operator!=(const CV1x90Creator& rhs) const
{
  return CModuleCreator::operator!=(rhs);
}

///////////////////////////////////////////////////////////////////////////////////////////
//

/*!
   Create a new module and configure it with the optional parameters.
   \param rInterp   - The interpreter running the 'module' command that's calling us.
   \param rResult   - The interpreter result object.
   \param nArgs     - Number of command line arguments.
   \param pArgs     - Pointers to the command line arguments.
   \return CReadableObject*
   \retval non-null Pointer to a dynamically created readable object of actual detailed type
           CV1x90Module.
   \retval NULL     - Some error was detected configuring the module.

   \note the module commandis no longer inthe nargs/args list.  The first argument is the
         module name.
*/
CReadableObject*
CV1x90Creator::Create(CTCLInterpreter& rInterp, 
		      CTCLResult& rResult, 
		      int nArgs, char** pArgs) 
{
  assert(nArgs >= 2);

  CReadableObject* pModule = new CV1x90Module(string(*pArgs), rInterp);
  nArgs -= 2;			// Get rid of module name and type.
  pArgs += 2;

  // If there are any remaining args, configure the mdoule:

  if (nArgs) {
    int status = pModule->Configure(rInterp, rResult,
				    nArgs, pArgs);
    if (status != TCL_OK) {
      delete pModule;
      pModule = (CReadableObject*)NULL;
    }

  }
  return pModule;
}
/*!
  Returns the string describing the module type:
*/
string
CV1x90Creator::Help()
{
  return string("Creates a CAEN V1190 or CAEN V1290 module");
}
