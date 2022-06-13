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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";// Class: CTCLAuthorizer

//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "CTCLAuthorizer.h"    				
#include <TCLString.h>
#include <TCLVariable.h>        
#include <TCLInterpreter.h>     
#include <TCLObject.h>
#include <TCLList.h> 


#include <string.h>
#include <assert.h>
#include <algorithm>


#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>



using namespace std;

static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved TclAuthorizer.cpp \n";

// Functions for class CTCLAuthorizer

static const std::string NameVariable("ServerHostNames");
static const std::string IPVariable("ServerHostIps");


//////////////////////////////////////////////////////////////////////////////
//
//  Function:
//     CTCLAuthorizer (Tcl_Interp* pInterp)
//  Operation Type:
//     Constructor.
//
CTCLAuthorizer::CTCLAuthorizer(CTCLInterpreter* pInterp) :
  CTCLObjectProcessor(*pInterp, "serverauth"),
  m_pInterpreter(pInterp)
{
  m_pHostNames = new CTCLVariable(m_pInterpreter, NameVariable, TCLPLUS::kfFALSE);
  m_pHostIps   = new CTCLVariable(m_pInterpreter, IPVariable, TCLPLUS::kfFALSE);

  // If those variables are not yet defined, make them empty strings:

  if (!m_pHostNames->Get()) {
    m_pHostNames->Set("");
  }
  if (!m_pHostIps->Get()) {
    m_pHostIps->Set("");
  }
}

CTCLAuthorizer::~CTCLAuthorizer()
{
  delete m_pHostNames;
  delete m_pHostIps;
}  


//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     operator()(CTCLInterpreter& rInterp, CTCLResult& rResult,
//                int nArgs, char* pArgs[])
//  Operation Type: 
//     Behavioral override.

/*!
   Gets control when the userauth command is received. Dispatches
   control the the appropriate AddHost, RemoveHost, or ListHosts
   member depending on the subcommand.
   \param interp - Interpreter that is exeucting this command.
   \param objv   - Encapsulated Tcl_Obj's that make up the
                   command line.
*/


int CTCLAuthorizer::operator()(CTCLInterpreter& rInterp,
			       vector<CTCLObject>& objv)

{
  // Processes the serverauth command.
  //  This command has the forms:
  //
  //   serverauth add hostorip          ;# Allows host or IP address
  //   serverauth remove hostorip    ;# Disallows host or IP address
  //   serverauth list                         ;# Lists the authorized set.
  //

  if(objv.size() < 2) {
    return Usage(rInterp);
  }

  for (int i =0; i < objv.size(); i++) {
    objv[i].Bind(rInterp);
  }

  string subcommand = objv[1];

  if (subcommand == string("add")) {
    if(objv.size() != 3) {
      return Usage(rInterp);
    }
    return AddHost((string)objv[2]) ? TCL_OK : TCL_ERROR;
  }
  else if (subcommand == string("remove")) {
    if(objv.size() != 3) {
      return Usage(rInterp);
    }
    return RemoveHost(std::string(objv[2])) ? TCL_OK: TCL_ERROR;
  }
  else if (subcommand == string("list")) {
    if(objv.size() != 2) {
      return Usage(rInterp);
    }
    rInterp.setResult(ListHosts());
    return TCL_OK;
  }
  else {
    return Usage(rInterp);
  }
  rInterp.setResult(std::string("Bug in CTCLAuthorizer::operator() decode section"));
  return TCL_ERROR;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     AddHost(const string& HostOrIp)
//  Operation Type: 
//     
TCLPLUS::Bool_t CTCLAuthorizer::AddHost( const std::string& HostOrIp)  
{
  // Adds a specified host to the access lists.
  // The parameter is first analyzed as a host name.
  //  if the resolver can get an IP for it the host and
  //  ip are entered in the lists.  If not, the
  //  parameter is analyzed as a dotted IP address
  //  If it can be parsed in that way, the IP address
  //  is entered along with a hostname of >unresolved<
  //
  
  // First: Duplicates are not allowed:

  if(GetIndex(HostOrIp) >= 0) {	// Duplicate...
    getInterpreter()->setResult("Duplicate host or ip address");
    return TCLPLUS::kfFALSE;
  }
  std::string hostname, hostip;
  if(!ConvertHost(HostOrIp, hostname, hostip)) {
    getInterpreter()->setResult("Invalid host or IP address");
    return TCLPLUS::kfFALSE;
  }
  // Append the hostname and hostip to the list:
  //
  StringArray hosts;
  StringArray ips;
  CTCLList HostList(m_pInterpreter,std::string(""));
  CTCLList IpList(m_pInterpreter,std::string(""));

  if(m_pHostNames->Get(TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG)) {
    CTCLList     HostList(m_pInterpreter, 
			  m_pHostNames->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
    HostList.Split(hosts);
    
    CTCLList     IpList(m_pInterpreter, 
			m_pHostIps->Get(TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG));
    IpList.Split(ips);
  }
  hosts.push_back(hostname);
  ips.push_back(hostip);

  m_pHostNames->Set(HostList.Merge(hosts), TCL_LEAVE_ERR_MSG |TCL_GLOBAL_ONLY);
  m_pHostIps->Set(IpList.Merge(ips), TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  
  return TCLPLUS::kfTRUE;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     RemoveHost(const string& NameOrIP)
//  Operation Type: 
//     
TCLPLUS::Bool_t CTCLAuthorizer::RemoveHost(const std::string& NameOrIp)  
{
  // An attempt is made to locate the host in 
  // the name list.  If it is found it and the 
  // like indexed item in the ip address list are
  // removed.  
  //  If the name is not found, it is treated like
  // an IP address, and searched for in the IP address
  // list.  If found, it and the corresponding name entry are
  // removed.
  //
  
  TCLPLUS::Int_t idx = GetIndex(NameOrIp);
  if(idx < 0) {			// Not in the table:
    getInterpreter()->setResult("Host or IP address is not in the authorization list");
    return TCLPLUS::kfFALSE;
  }
  
  auto authInfo = getAuthInfo();
  StringArray& hostnames(authInfo.first);
  StringArray& hostips(authInfo.second);
  
  int nLast = hostnames.size() - 1; // Index of last item.
 
  // The delete is done by copying the last entry into the
  // entry the which is to be deleted and doing a pop_back() to kill off
  // the last entry:

  hostnames[idx] = hostnames[nLast];
  hostips[idx]   = hostips[nLast];
  hostnames.pop_back();
  hostips.pop_back();
  CTCLList IpList(m_pInterpreter);
  CTCLList HostList(m_pInterpreter);
  
  m_pHostIps->Set(IpList.Merge(hostips), TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  m_pHostNames->Set(HostList.Merge(hostnames),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  return TCLPLUS::kfTRUE;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     ListHosts()
//  Operation Type: 
//     
std::string CTCLAuthorizer::ListHosts()  
{
  // Returns a TCL formatted list whose elements are two entry
  // lists containing {hostname ipaddress}
  // Note that if the IP address is not resolvable,
  // the name has no real meaning.
  //

  CTCLString Result;

  // The result list is built up in Result and then returned.
  // This cannot fail.

  auto authInfo = getAuthInfo();
  StringArray& hostnames(authInfo.first);
  StringArray& hostips(authInfo.second);
  
  assert(hostips.size() == hostnames.size());

  for(int i=0; i < hostips.size(); i++) {
    Result.StartSublist();
    Result.AppendElement(hostnames[i]);
    Result.AppendElement(hostips[i]);
    Result.EndSublist();
  }
  std::string sResult = Result;
  return sResult;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Authenticate(const string& rNameOrIp)
//  Operation Type: 
//     
TCLPLUS::Bool_t CTCLAuthorizer::Authenticate(const std::string& rNameOrIp)  
{
  // The parameter is converted to canonical ip form
  // and searched for in the ip list.
  //


  int idx = GetIndex(rNameOrIp);
  return (idx >= 0);
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     HostToIp(string& rName)
//  Operation Type: 
//     
TCLPLUS::Bool_t CTCLAuthorizer::HostToIp(std::string& rName)  
{
  // Converts a string to IP address in 'canonical dotted' form
  // Cannonical dotted form is:  %03d.%03d.%03d.%03d
  // If the input is a host name, it is converted to ip address via
  // the resolver.  If not it is decoded as a dotted ip address
  // and canonicalized.
  //   Result is returned in the parameter.  Return value indicates
  // success or failure.. Input is not 'harmed' on False return.
  //
  

  struct in_addr IpAddress;
  char Result[100];

  // First the string is treated as a hostname for the resolver to
  // turn into an IP address:

  struct hostent* pHostInfo = gethostbyname(rName.c_str());
  if(! pHostInfo) {		// Try as dotted ip
    IpAddress.s_addr = inet_addr(rName.c_str());
  }
  else {
    memcpy(&IpAddress, pHostInfo->h_addr,4);
  }
  union {
    uint32_t along;
    uint8_t  bytes[4];
  } IpMyOrder;
  IpMyOrder.along = ntohl(IpAddress.s_addr);
  sprintf(Result, "%03d.%03d.%03d.%03d",
	  IpMyOrder.bytes[3], IpMyOrder.bytes[2],
	  IpMyOrder.bytes[1], IpMyOrder.bytes[0]);

  std::string R(Result);
  rName = R;
  return TCLPLUS::kfTRUE;

}

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetIndex(const string& rHostOrIp)
//  Operation Type: 
//     
TCLPLUS::Int_t CTCLAuthorizer::GetIndex(const std::string& rHostOrIp)  
{
  // Returns the index of the entry corresponding
  // to an input IP name or Address or -1 if no match.
  // This utility contains common code between 
  // Add, Delete and Authenticate.

  std::string Host(rHostOrIp);
  if(!HostToIp(Host)) return -1; // Invalid host.
  if(!m_pHostIps->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY)) return -1;


  CTCLList IpList(m_pInterpreter, 
		  m_pHostIps->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  StringArray ipvector;
  IpList.Split(ipvector);
  for(int i = 0; i < ipvector.size(); i++) {
    if(ipvector[i] == Host) return i;
  }
  return -1;
}
///////////////////////////////////////////////////////////////////////
//
// Function:
//   TCLPLUS::Bool_t ConvertHost(const string& rInName, 
//		        string& rOutname, string& rCanonicalIP)
// Operation type:
//   Utility:
//
TCLPLUS::Bool_t 
CTCLAuthorizer::ConvertHost(const std::string& rInName, 
							std::string& rOutname, 
							std::string& rCanonicalIP)
{
  // Converts the input name into the information required to
  // insert it into the host access list.

  std::string myname(rInName);
  rCanonicalIP = rInName;
  if(!HostToIp(rCanonicalIP)) return TCLPLUS::kfFALSE;

  struct hostent* pEntry = gethostbyname(myname.c_str());
  rOutname = pEntry ? myname : std::string(">unresolved<");
  return TCLPLUS::kfTRUE;
}
/////////////////////////////////////////////////////////////////////
//
// Function:
//   int Usage(CTCLResult& rResult)
// OPeration type:
//   Utility
//
int
CTCLAuthorizer::Usage(CTCLInterpreter& interp)
{
  // Fills the result string with command usage information.
  // returns TCL_ERROR making calls like
  //  return Usage(rResult)
  // a useful simplification.

  string rResult;
  rResult   = std::string("Usage:\n");
  rResult  += std::string("  serverauth add hostorip\n");
  rResult  += std::string("  serverauth remove hostorip\n");
  rResult  += std::string("  serverauth list\n");
  interp.setResult(rResult);


  return TCL_ERROR;
}
/**
 * getAuthInfo
 *    Get the authorized host names and their IP strings.
 *  @return std::pair<std::vector< std::string>, std::vector<std::string>>
 *     First element is the list of host names, second the list of
 *     host IP strings.
 *
 */
std::pair<std::vector<std::string>, std::vector<std::string>>
CTCLAuthorizer::getAuthInfo()
{
  std::pair<std::vector<std::string>, std::vector<std::string>> result;
  StringArray& hostnames(result.first);
  CTCLList    HostList(m_pInterpreter, 
		       m_pHostNames->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  HostList.Split(hostnames);

  StringArray& hostips(result.second);
  CTCLList    IpList(m_pInterpreter, 
		     m_pHostIps->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  IpList.Split(hostips);

  return result;
}
