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
/*!
  Creates a CCAENV830 readout module.   This module is suitable for use
  in the main readout section for systems that require e.g. precise absolute
  timing to be read with an event.  Typically, this class will be instantiated
  and registered with a module command creational object. e.g.
  \verbatim
  ...
  // Make a new interpreter:

  pInterp = new CTCLInterpreter();
  Tcl_Init(pInterp->getInterpreter());

  // Create the module creation infrastructure:

  pDictionary = new CDigitizerDictionary;
  pReader     = new CReadOrder(pInterp, pDictionary);
  pCreator    = new CModuleCommand(pInterp,
				   pDictionary,
				   pReader);
  pCreator->AddCreator(new CCAENV830);
  ...
  \endverbatim

  The creator is then used by the Module command to match
  the module type recognized by the CCAENV830Creator ("caenv830")
  to instantiate modules that can then be configured by the user
  through TCL scripting.

*/
#ifndef CCAENV830CREATOR_H  //Required for current class
#define CCAENV830CREATOR_H

//
// Include files:
//

                               //Required for base classes
#include "CModuleCreator.h"
#include <string>

class CTCLInterpreter;
class CTCLResult;
class CReadableObject;


class CCAENV830Creator  : public CModuleCreator        
{
private:
   
	/* Constructors and other canonical operations: */
	
public:
  CCAENV830Creator ();
  virtual ~CCAENV830Creator ( );

   virtual CReadableObject* Create (const char* name, CTCLInterpreter& rInterp);

};

#endif
