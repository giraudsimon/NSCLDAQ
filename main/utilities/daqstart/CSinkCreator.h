// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
/*!
  \class CSinkCreator
  \file  .h

  Abstract base class for the set of sink creators
  supported at any given time by the sink factory.
  This class establishes an interface specification for the 
  SinkCreator hierarchy.

*/

#ifndef __CSINKCREATOR_H  //Required for current class
#define __CSINKCREATOR_H

//
// Include files:
//

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward definitions:

class CSink;

// The CSinkCreator:

class CSinkCreator      
{
public:

  CSinkCreator () {}		//!< Constructor.
  virtual  ~ CSinkCreator ( ) {} //!< Destructor.
  CSinkCreator (const CSinkCreator& rSource ) {} //!< Copy construction.
  CSinkCreator& operator= (const CSinkCreator& rhs) { //!< Assignment.
    return *this;
  }
  int operator== (const CSinkCreator& rhs) const { //!< == comparison.
    return (int)true;
  }
  int operator!= (const CSinkCreator& rhs) const { //!< != comparison.
    return !(operator==(rhs));
  }

// Class operations:

public:
  
  virtual   bool   isNameLegal (std::string sName)   = 0 ; 
  virtual   CSink* Create (std::string sCommand, std::string sName)   = 0 ; 
  

  
};

#endif
