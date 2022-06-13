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


   

/*! \class: CPacketCreator           
    \file:  .h
Creational class for read order objects.  Matches against the 
specified module type and produces a ReadOrder Module.
*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//


#ifndef CPACKETCREATOR_H  //Required for current class
#define CPACKETCREATOR_H

//
// Include files:
//

#include "CModuleCreator.h"
#include <string>

// Forward definitions:
//
class CDigitizerDictionary;
class CTCLInterpreter;
class CTCLResult;


class CPacketCreator  : public CModuleCreator        
{
private:
  CDigitizerDictionary* m_pModules;
   
public:
  // COnstructors and other cannonical operations:


 CPacketCreator (const std::string& rType, CDigitizerDictionary* pDictionary); //!< Construtor..
 virtual ~ CPacketCreator ( );	                   //!< destructor

public:

 virtual CReadableObject* Create (const char* name, CTCLInterpreter& rInterp);
};
#endif
