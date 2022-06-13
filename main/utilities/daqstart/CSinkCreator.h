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

#ifndef CSINKCREATOR_H  //Required for current class
#define CSINKCREATOR_H

//
// Include files:
//

#include <CCreator.h>        // Generic factory.
#include <string>
#include <utility>

// Forward definitions:

class CSink;

// The CSinkCreator:

class CSinkCreator : public CCreator<CSink>
{
public:

  CSinkCreator () {}		//!< Constructor.
  

// Class operations:

public:
  virtual CSink*  operator()(void* pUserData) {
    // adaptor to Create:
    
    std::pair<const char*, const char*>* userData =
      reinterpret_cast<std::pair<const char*, const char*>*>(pUserData);
    std::string sCommand = userData->first;
    std::string sName    = userData->second;
    if (!isNameLegal(sName)) return nullptr;
    return Create(sCommand, sName);
  }
  virtual   bool   isNameLegal (std::string sName)   = 0 ; 
  virtual   CSink* Create (std::string sCommand, std::string sName)   = 0 ; 
  virtual   std::string describe() const = 0;

  
};

#endif
