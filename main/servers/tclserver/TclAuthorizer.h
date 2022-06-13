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

// Class: CTclAuthorizer                     //ANSI C++
//
// Manages authentication for the TclServer component.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved TclAuthorizer.h
//


#ifndef TCLAUTHORIZER_H  //Required for current class
#define TCLAUTHORIZER_H

#include <libtclplusdatatypes.h>

#include <string>        //Required for include files
#include <TCLInterpreter.h>        //Required for include files
#include <TCLProcessor.h>
#include <TCLVariable.h>        //Required for include files
#include <TCLList.h>        //Required for include files
#include <TCLResult.h>        //Required for include files
                               
class CTclAuthorizer : public CTCLProcessor     
{                       
  CTCLInterpreter* m_pInterpreter;
  CTCLVariable* m_pHostNames; //List of allowed hostnames.
  CTCLVariable* m_pHostIps; //List of allowed host IPs.        
  CTCLResult*   m_pResult;
protected:

public:

   // Constructors and other cannonical operations:

  CTclAuthorizer (Tcl_Interp* pInterp);
  ~ CTclAuthorizer ( )  // Destructor 
  {
    delete m_pHostNames;
    delete m_pHostIps;
    delete m_pInterpreter;
  }  

  
   //Copy constructor 
private:
  CTclAuthorizer (const CTclAuthorizer& aCTclAuthorizer ) ;
  CTclAuthorizer& operator= (const CTclAuthorizer& aCTclAuthorizer);
  int operator== (const CTclAuthorizer& aCTclAuthorizer) const;
public:
	
// Selectors:

public:

  const CTCLVariable* getHostNames() const
  { 
    return m_pHostNames;
  }
  const CTCLVariable* getHostIps() const
  { 
    return m_pHostIps;
  }
                       
// Mutators:

protected:

  void setHostNames (CTCLVariable* am_pHostNames)
  { 
    m_pHostNames = am_pHostNames;
  }
  void setHostIps (CTCLVariable* am_pHostIps)
  { 
    m_pHostIps = am_pHostIps;
  }
       
public:

  virtual   int operator() (CTCLInterpreter& rInterp, CTCLResult& rResult, 
			    int nArgs, char* pArgs[])    ;
  TCLPLUS::Bool_t AddHost (const std::string& HostOrIp)    ;
  TCLPLUS::Bool_t RemoveHost (const std::string& NameOrIP)    ;
  std::string ListHosts ()    ;
  TCLPLUS::Bool_t Authenticate (const std::string& rNameOrIp)    ;

protected:
  int   Process(CTCLInterpreter& rInterp, CTCLResult& rResult, 
		  int nArgs, char* pArgs[])    ;
  TCLPLUS::Bool_t  HostToIp(std::string& rName);
  TCLPLUS::Int_t   GetIndex (const std::string& rHostOrIp)   ;
  TCLPLUS::Bool_t ConvertHost(const std::string& rInName, 
	                 std::string& rOutname, std::string& rCanonicalIP)   ;

  int  Usage(CTCLResult& rResult);
private:
	void setResult(const std::string& rResult) {
    if(m_pResult) *m_pResult = rResult;
  }
  void setResult(const char* pResult) {
    if(m_pResult) *m_pResult = pResult;
  }
};

#endif
