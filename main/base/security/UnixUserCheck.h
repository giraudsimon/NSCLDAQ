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

// Class: CUnixUserCheck                     //ANSI C++
//
// Authenticates a unix username and password
// for the host system.  Uses the interactor to 
// obtain two records of information:
//   Username,
//   Password
// Username is used to do a getpwentry,
// Password is encrytped and compared to
// the password in the pwentry returned.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved UnixUserCheck.h
//

#ifndef UNIXUSERCHECK_H  //Required for current class
#define UNIXUSERCHECK_H

                               //Required for base classes
#include "Authenticator.h"
#include <string>


class CUnixUserCheck  : public CAuthenticator        
{                       
			
   std::string m_sUserPrompt;	//Username Prompt string
   std::string m_sPasswordPrompt;	//Password Prompt string.
   bool m_fPromptUser;	//If true usename is prompted for.
   bool m_fPromptPassword;	//If false, password is prompted for.        

protected:

public:

   // Constructors and other cannonical operations:

  CUnixUserCheck ()    : 
    CAuthenticator(),
    m_sUserPrompt(std::string("Username: ")),   
    m_sPasswordPrompt(std::string("Password: ")),   
    m_fPromptUser(true),   
    m_fPromptPassword(true) 
  { 
    
  } 
   ~ CUnixUserCheck ( )  // Destructor 
     { }  
  CUnixUserCheck (const std::string& am_sUserPrompt,  
		  const std::string& am_sPasswordPrompt,  
		  bool am_fPromptUser,  bool am_fPromptPassword  ) 
      : CAuthenticator()   ,
	m_sUserPrompt(am_sUserPrompt),
	m_sPasswordPrompt(am_sPasswordPrompt),
	m_fPromptUser(am_fPromptUser),
	m_fPromptPassword(am_fPromptPassword)
  { 
  }    

  
   //Copy constructor 

  CUnixUserCheck (const CUnixUserCheck& aCUnixUserCheck )   : 
    CAuthenticator (aCUnixUserCheck) 
  { 
   
    m_sUserPrompt = aCUnixUserCheck.m_sUserPrompt;
    m_sPasswordPrompt = aCUnixUserCheck.m_sPasswordPrompt;
    m_fPromptUser = aCUnixUserCheck.m_fPromptUser;
    m_fPromptPassword = aCUnixUserCheck.m_fPromptPassword;
     
  }                                     

   // Operator= Assignment Operator 

  CUnixUserCheck& operator= (const CUnixUserCheck& aCUnixUserCheck) {
    if(this != &aCUnixUserCheck) {
      CAuthenticator::operator=(aCUnixUserCheck);
      m_sUserPrompt = aCUnixUserCheck.m_sUserPrompt;
      m_sPasswordPrompt = aCUnixUserCheck.m_sPasswordPrompt;
      m_fPromptUser = aCUnixUserCheck.m_fPromptUser;
      m_fPromptPassword = aCUnixUserCheck.m_fPromptPassword;
    }
    return *this;
  }
 
   //Operator== Equality Operator 
private:
  int operator== (const CUnixUserCheck& aCUnixUserCheck) const;
public:

// Selectors:

public:

  std::string getUserPrompt() const
  { return m_sUserPrompt;
  }
  std::string getPasswordPrompt() const
  { return m_sPasswordPrompt;
  }
  bool getPromptUser() const
  { return m_fPromptUser;
  }
  bool getPromptPassword() const
  { return m_fPromptPassword;
  }
                       
// Mutators:

protected:

  void setUserPrompt (const std::string& am_sUserPrompt)
  { m_sUserPrompt = am_sUserPrompt;
  }
  void setPasswordPrompt (const std::string& am_sPasswordPrompt)
  { m_sPasswordPrompt = am_sPasswordPrompt;
  }
  void setPromptUser (bool am_fPromptUser)
  { m_fPromptUser = am_fPromptUser;
  }
  void setPromptPassword (bool am_fPromptPassword)
  { m_fPromptPassword = am_fPromptPassword;
  }
       
public:

  virtual   bool Authenticate (CInteractor& rInteractor)    ;
  void SetPrompting (bool fUserPrompt=true, 
		     bool fPasswordPrompt=true)    ;
  void SetUserPrompt (const std::string& rNewPrompt=std::string("Username: "))    ;
  void SetPasswordPrompt (const std::string& rNewPrompt=std::string("Password: "))    ;
 
protected:
  void   PutPrompt(CInteractor& rInteractor, const std::string& rPrompt);
  bool Validate(const std::string& sUser, const std::string& sPassword);

};

#endif
