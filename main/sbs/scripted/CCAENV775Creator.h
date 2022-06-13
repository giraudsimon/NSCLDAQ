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


// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CCAENV775CREATOR_H  //Required for current class
#define CCAENV775CREATOR_H


#include "CModuleCreator.h"
#include <string>



/*!
  Creational class for CAEN V775 module adc.  This class
  gets registered with the Module command object to 
  allow it to respond to commands to generate readout objects
  for the CAEN V775 module.  Our module type string is 
  caenv775. 
*/
class CCAENV775Creator  : public CModuleCreator        
{
public:
  //   Constructors and other cannonical operations.
  
  CCAENV775Creator ();
  virtual ~CCAENV775Creator ( );
  
public:
  
  virtual CReadableObject* Create(const char* name, CTCLInterpreter& rInterp); 
  
};

#endif
