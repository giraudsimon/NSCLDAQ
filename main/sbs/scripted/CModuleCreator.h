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

 \class  CModuleCreator           
Abstract base class of a set of factory like (creational) modules
that make data acquisition module classes.  Each of these has
a string module type.  And the following key members:
- Match - return true if an input string matches the module type.
- Create - Returns a new module.
- Help    - Returns stringified help about the type of module
               created by this class.
*/

//
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//

#ifndef CMODULECREATOR_H  //Required for current class
#define CMODULECREATOR_H




//
// Include files:
//
#include <CExtensibleFactory.h>
#include <string>
#include <CReadableObject.h>

using CModuleCreatorType = CCreator<CReadableObject>;


// Forward class definitions:

class CReadableObject;
class CTCLInterpreter;
class CTCLResult;


class CModuleCreator    : public CModuleCreatorType  
{
protected:
  
  std::string m_helpText;
public:
	// Constructors and other cannonical operations:
	
   CModuleCreator ();
   virtual  ~CModuleCreator ( );  

  CReadableObject* operator()(void* userData);
  virtual CReadableObject* Create (const char* name, CTCLInterpreter& rInterp) = 0;
  std::string describe() const;
};

#endif
