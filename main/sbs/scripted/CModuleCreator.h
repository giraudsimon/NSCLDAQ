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

#include <string>

// Forward class definitions:

class CReadableObject;
class CTCLInterpreter;
class CTCLResult;


class CModuleCreator      
{
private:
  
  // Private Member data:

   std::string m_sModuleType;                  //!<  Module type string
   
public:
	// Constructors and other cannonical operations:
	
   CModuleCreator (const std::string& rType);
   virtual  ~ CModuleCreator ( );  
   CModuleCreator (const CModuleCreator& aCModuleCreator );
   CModuleCreator& operator= (const CModuleCreator& rhs);
   int operator== (const CModuleCreator& rhs)  const;
   int operator!=  (const CModuleCreator& rhs) const {
      return !(operator==(rhs));
   }
   
// Selectors:

public:

          //Get accessor function for non-static attribute data member
  std::string getModuleType() const
  {
     return m_sModuleType;
  }   

// Attribute mutators:

protected:

          //Set accessor function for non-static attribute data member
  void setModuleType (const std::string am_sModuleType)
  { 
     m_sModuleType = am_sModuleType;
  }   

  // Class operations:

public:

   bool    Match (const std::string& rType) const;
   virtual CReadableObject* 
	   Create (CTCLInterpreter& rInterp, 
		   CTCLResult& rResult, 
		   int nArgs, 
		   char** pArgs)   = 0 ; 
   virtual  std::string  Help ()   = 0 ; 

};

#endif
