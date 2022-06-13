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



//! \class: CStringConfigParam           
//! \file:  .h
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CSTRINGCONFIGPARAM_H  //Required for current class
#define CSTRINGCONFIGPARAM_H

//
// Include files:
//

#include "CConfigurationParameter.h"
#include <string>
 
// Forward class defintions.

class CTCLInterpreter;
class CTCLResult;
 
/*!
Represents  a string valued configuration parameter.

*/
class CStringConfigParam  : public CConfigurationParameter        
{

public:
	// Constructors and other cannnonical member functions.
	
    CStringConfigParam (const std::string& keyword);
    virtual  ~CStringConfigParam ( );  //Destructor - Delete any pointer data members that used new in constructors 
  
  CStringConfigParam(const CStringConfigParam& aCStringConfigParam );
  CStringConfigParam& operator= (const CStringConfigParam& rhs);
  int operator== (const CStringConfigParam& rhs) const;
  int operator!= (const CStringConfigParam& rhs) const {
	return !(operator==(rhs));
  }

  // Class functions:
 public:
    std::string getOptionValue ()   ; // 
    virtual   int SetValue (CTCLInterpreter& rInterp, CTCLResult& rResult, 
				    const char* value)   ; // 
    virtual std::string GetParameterFormat();

};

#endif

