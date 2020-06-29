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

 
// Class: CAuthenticator                     //ANSI C++
//
// Abstract base class for authenticators.  Authenticators
// interact with an interactor to get authentication data
// and check them against criteria for valid 'connections'.
//
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved Authenticator.h
//

#ifndef AUTHENTICATOR_H  //Required for current class
#define AUTHENTICATOR_H

#include "Interactor.h"
#include <string>

class CAuthenticator      
{                       
			
protected:

public:

   // Constructors and other cannonical operations:

  CAuthenticator ()  
  { 
  } 
  virtual  ~ CAuthenticator ( )  // Destructor 
  { }  

  CAuthenticator (const CAuthenticator& aCAuthenticator ) 
  {
  }                                     

  CAuthenticator& operator= (const CAuthenticator& aCAuthenticator) {
    return *this;
  }
 
private:
  int operator== (const CAuthenticator& aCAuthenticator) const;
public:	
 virtual   bool Authenticate (CInteractor& rInteractor)   = 0  ;
protected:
   std::string GetLine(CInteractor& rInteractor) {
   std::string pwd;
   char   c;
   bool done(false);
   while(!done) {
     int nb = rInteractor.Read(1, &c);
     if(nb == 1) {
       if(c == '\n') {
	 done = true;
       }
       else {
	 pwd += c;
       }
     }
     else {
       done = true;
     }
   }
   return pwd;
 }
 
};

#endif
