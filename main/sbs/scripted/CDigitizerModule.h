/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/*!
  This class is largely obsoleted by CReadableObject. From now on
  derive directly from CReadable object.
  This class is provided for compatibility with the prior structure of the
  framework.
*/
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CDIGITIZERMODULE_H  //Required for current class
#define CDIGITIZERMODULE_H

#include "CReadableObject.h"

class CTCLInterpreter;




// Forward class definitions:



class CDigitizerModule : public CReadableObject    
{
 
public:

//   Construtors and other canonical operations.

  CDigitizerModule (const std::string& rName,
                    CTCLInterpreter& rInterp); 
  virtual ~CDigitizerModule ( );

private:
  CDigitizerModule (const CDigitizerModule& rhs );
  CDigitizerModule& operator= (const CDigitizerModule& rhs);
  int operator== (const CDigitizerModule& rhs) const;
  int operator!= (const CDigitizerModule& rhs) const;
public:


};

#endif
