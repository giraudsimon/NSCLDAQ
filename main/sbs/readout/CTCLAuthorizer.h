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


#ifndef CTCLAUTHORIZER_H  //Required for current class
#define CTCLAUTHORIZER_H


#include <TCLObjectProcessor.h>
#include <daqdatatypes.h>
#include <string>        //Required for include files
#include <vector>

class CTCLObject;
class CTCLVariable;
class CTCLInterpreter;

/*!
 Provides an authorizer for tcl server based on the
 userauth command.

\verbatim
userauth add host-or-ip
userauth list
userauth delete host-or-ip

\endverbatim

The Authenticate member determines if a candidate host is allowed 
to connect.

*/
            
class CTCLAuthorizer : public CTCLObjectProcessor     
{                       
  CTCLInterpreter* m_pInterpreter;
  CTCLVariable* m_pHostNames; //List of allowed hostnames.
  CTCLVariable* m_pHostIps; //List of allowed host IPs.        

protected:

public:

   // Constructors and other cannonical operations:

  CTCLAuthorizer (CTCLInterpreter* pInterp);
  ~ CTCLAuthorizer ( );

  
   //Copy constructor 
private:
  CTCLAuthorizer (const CTCLAuthorizer& aCTCLAuthorizer ) ;
  CTCLAuthorizer& operator= (const CTCLAuthorizer& aCTCLAuthorizer);
  int operator== (const CTCLAuthorizer& aCTCLAuthorizer) const;
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

  virtual   int operator() (CTCLInterpreter& rInterp,
			    std::vector<CTCLObject>& objv)    ;
  TCLPLUS::Bool_t AddHost (const std::string& HostOrIp)    ;
  TCLPLUS::Bool_t RemoveHost (const std::string& NameOrIP)    ;
  std::string ListHosts ()    ;
  TCLPLUS::Bool_t Authenticate (const std::string& rNameOrIp)    ;

protected:

  TCLPLUS::Bool_t  HostToIp(std::string& rName);
  TCLPLUS::Int_t   GetIndex (const std::string& rHostOrIp)   ;
  TCLPLUS::Bool_t ConvertHost(const std::string& rInName, 
	                 std::string& rOutname, std::string& rCanonicalIP)   ;

  TCLPLUS::Int_t   Usage(CTCLInterpreter& rInterp);
  std::pair<std::vector<std::string>, std::vector<std::string>> getAuthInfo();

};

#endif
