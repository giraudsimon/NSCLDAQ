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

/*!
	Creational class for CAEN V785 module adc.  This class
	gets registered with the Module command object to 
	allow it to respond to commands to generate readout objects
	for the CAEN V785 module.  Our module type string is 
	caenv785.
*/
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CCAENV785CREATOR_H  //Required for current class
#define CCAENV785CREATOR_H

//
// Include files:
//

                               //Required for base classes
#include "CModuleCreator.h"
#include <string>

class CReadableObject;
 

class CCAENV785Creator  : public CModuleCreator        
{
public:
	//   Constructors and other cannonical operations.
	
  CCAENV785Creator ();
  ~CCAENV785Creator ();

   virtual   CReadableObject* Create (const char* name, CTCLInterpreter& rInterp);
						   
};

#endif
