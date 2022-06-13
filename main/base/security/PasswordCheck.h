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

// Class: CPasswordCheck                     //ANSI C++
//
// Performs password access control
// Note that the password is internally stored as
// clear text since it is assumed that external
// forces will be the ones attempting to break
// this loose security.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved PasswordCheck.h
//

#ifndef PASSWORDCHECK_H  //Required for current class
#define PASSWORDCHECK_H

                               //Required for base classes
#include "Authenticator.h"
#include <string>

class CPasswordCheck  : public CAuthenticator        
{                       
   static const std::string m_EmptyString;
   std::string m_sPassword;		//Password in clear text.
   std::string m_sPromptString;	// Promptstring.
   bool m_fWithPrompt;	//TRUE if need to prompt for password.        

protected:

public:

   // Constructors and other cannonical operations:

	CPasswordCheck (const std::string& rPassword,
	      std::string rPrompt = m_EmptyString,
		  bool WithPrompt = false)    : 
    m_sPassword(rPassword),
    m_sPromptString(rPrompt),
    m_fWithPrompt(WithPrompt)
  { 
  } 
  ~ CPasswordCheck ( )  // Destructor 
  { }  
  
   //Copy constructor 

  CPasswordCheck (const CPasswordCheck& aCPasswordCheck )   : 
    CAuthenticator (aCPasswordCheck),
    m_sPassword(aCPasswordCheck.m_sPassword),
    m_sPromptString(aCPasswordCheck.m_sPromptString),
    m_fWithPrompt(aCPasswordCheck.m_fWithPrompt)
  {
  }                                     

   // Operator= Assignment Operator 

  CPasswordCheck& operator= (const CPasswordCheck& aCPasswordCheck) {
    m_sPassword     = aCPasswordCheck.m_sPassword;
    m_sPromptString = aCPasswordCheck.m_sPromptString;
    m_fWithPrompt   = aCPasswordCheck.m_fWithPrompt;
    return (CPasswordCheck&)CAuthenticator::operator=(aCPasswordCheck);
  }
private:
  int operator== (const CPasswordCheck& aCPasswordCheck) const;
public:

// Selectors:

public:

  std::string getPassword() const
  { return m_sPassword;
  }
  std::string getPromptString() const
  { return m_sPromptString;
  }
  bool getWithPrompt() const
  { return m_fWithPrompt;
  }
                       
// Mutators (public!)


  void setPassword (const std::string am_sPassword)
  { m_sPassword = am_sPassword;
  }
  void setPromptString (const std::string am_sPromptString)
  { m_sPromptString = am_sPromptString;
  }
protected:

  void setWithPrompt (const bool am_fWithPrompt)
  { m_fWithPrompt = am_fWithPrompt;
  }
       
public:

  virtual   bool Authenticate (CInteractor & rInteractor)    ;

  void DisablePrompt () {
    m_fWithPrompt = false;
  }

  void EnablePrompt () {
    m_fWithPrompt = true;
  }
  
protected:
  std::string GetUserPassword(CInteractor& rInteractor) {
    return GetLine(rInteractor);
  }
};

#endif




