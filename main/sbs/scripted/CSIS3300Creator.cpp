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

using namespace std;

/*!
   Construct the class, we just need to construct the base class.
*/
CSIS3300Creator::CSIS3300Creator(const string& type) :
  CModuleCreator(type)
{}

/*!
   Destruction is a nooop supplied to complete the chain of
   virtual destructors back to the base class.
*/
CSIS3300Creator::~CSIS3300Creator()
{}

/*!
   Copy constructor.. .invoke the base class copy constr.
*/
CSIS3300Creator::CSIS3300Creator(const CSIS3300Creator& rhs) :
  CModuleCreator(rhs)
{}
/*!
   Assignment... invoke the base class assign.
*/
CSIS3300Creator&
CSIS3300Creator::operator=(const CSIS3300Creator& rhs)
{
  if (this != &rhs) {
    CModuleCreator::operator=(rhs);
  }
  return *this;
}
/*!
   Equality compare.. done via base class.
*/
int
CSIS3300Creator::operator==(const CSIS3300Creator& rhs) const
{
  return CModuleCreator::operator==(rhs);
}
/*!
   Inequality by convention is always the logical inverse of equality.
*/
int
CSIS3300Creator:: operator!=(const CSIS3300Creator& rhs) const
{
  return *this != rhs;
}

/*!
   Provide a help string for those who care about it.
*/
string
CSIS3300Creator::Help()
{
  return string( "Struck (SIS) 3300/3301 flash adc");
}

/*!
   Create a new module
*/
CReadableObject*
CSIS3300Creator::Create(CTCLInterpreter& rInterp, CTCLResult& rResult,
			int nArgs, char** pArgs)  
{ 
	assert(nArgs >= 2);
	string mName(*pArgs);       // Module name.
	nArgs --;
	pArgs++;
	assert(*pArgs == getModuleType());
	nArgs--;
	pArgs++;
	
	CSIS3300Module* pModule= new CSIS3300Module(mName, rInterp);
	if(nArgs) {
	  int status = pModule->Configure(rInterp, rResult, nArgs, pArgs);
	  if(status != TCL_OK) { // Configure failed..
	    delete pModule;	 //  Configure filled in result. 
	    pModule = (CSIS3300Module*) NULL; 
	  }
	}
	return pModule;
	
}  
