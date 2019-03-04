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


#ifndef __CPACKETCREATOR_H  //Required for current class
#define __CPACKETCREATOR_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CMODULECREATOR_H     //CModuleCreator
#include "CModuleCreator.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

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

  CPacketCreator (const std::string& rType,
		  CDigitizerDictionary* pDictionary); //!< Construtor..
 virtual ~ CPacketCreator ( );	                   //!< destructor
 CPacketCreator (const CPacketCreator& aCPacketCreator ); //!< copy
 CPacketCreator& operator= (const CPacketCreator& aCPacketCreator); //!< assign
 int operator== (const CPacketCreator& aCPacketCreator) const; //!< compare.
 int operator!= (const CPacketCreator& aCPacketCReator) const; //!< inequal compare


public:

 virtual   CReadableObject* Create (CTCLInterpreter& rInterp,
				    CTCLResult&      rResult,
				    int nArgs, char** pArgs)   ; //!< Create a module.
 virtual   std::string Help ()   ;     	  //!< Contribute to help. 
 
};

#endif
