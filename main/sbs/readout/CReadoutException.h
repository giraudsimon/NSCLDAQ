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


#ifndef CREADOUTEXCEPTION_H  
#define CREADOUTEXCEPTION_H
                               
#include <Exception.h>

/*!
   This class is an ABC for the exceptions which can be thrown by the readout subsystem.
   It is provided to allow catching CReadoutException&  in order to get all readout exceptions
   
 */		
class CReadoutException : public CException     
{ 
public:
	// Constructors, destructors and other cannonical operations: 
  
  CReadoutException (const char* pszAction);
  CReadoutException(const CReadoutException& rhs);
  virtual  ~ CReadoutException ( ) {}

  CReadoutException& operator= (const CReadoutException& rhs);
  int         operator==(const CReadoutException& rhs);
  int         operator!=(const CReadoutException& rhs) {
    return !(operator==(rhs));
  }


};

#endif
