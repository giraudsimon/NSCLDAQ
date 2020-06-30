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

#ifndef CCAENV785_H  //Required for current class
#define CCAENV785_H

#include "CDigitizerModule.h"
#include <string>
#include "CCAENModule.h"

using namespace std;





// Forward definitions:

class CAENcard;

  

/*!
  Encapsulates the CAEN 792 QDC (Charge digitizing
ADC).  This module is an extension of the CCAENModule
class.  It supports all of the configuration parameters
in that class and adds an integer parameter:

- Ipedestal (int) the capacitor leakage current 
  compensation offset charge.  This value is 
  set directly in the register value.  The units are
  not documented in the module manual.


*/
class CCAENV792  : public CCAENModule
{
private:



public:
  // Construtors and other cannonical functions:

  CCAENV792(const std::string &rName, CTCLInterpreter& rInterp);
 ~CCAENV792 ( );
private:
  CCAENV792 (const CCAENV792& aCCAENV792 );
  CCAENV792& operator= (const CCAENV792& aCCAENV792);
  int operator== (const CCAENV792& aCCAENV792) const;
  int operator!=(const CCAENV792& rhs) const;
public:

  // Class operations:

public:

  virtual   void Initialize (); // 
  virtual   void Prepare (); // 
  virtual   void Clear ();
  virtual   std::string getType() const {     
    return std::string("caenv792");
  }

};

#endif
