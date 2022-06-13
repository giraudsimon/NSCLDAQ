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


// Class: CTclAuthorizer
// Manages authentication for the TclServer component.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
//////////////////////////.cpp file/////////////////////////////////////////////////////

#include <config.h>
#include <libtclplusdatatypes.h>
#include "TclAuthorizer.h"    				
#include "TCLString.h"

#include <string.h>
#include <assert.h>
#include <algorithm>

#ifndef WIN32
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>

#ifdef WIN32
#include <winsock.h>
#define uint32_t    unsigned long
#define uint8_t     unsigned char
#endif


using namespace std;



static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved TclAuthorizer.cpp \n";

// Functions for class CTclAuthorizer

static const string NameVariable("ServerHostNames");
static const string IPVariable("ServerHostIps");


//////////////////////////////////////////////////////////////////////////////
//
//  Function:
//     CTclAuthorizer (Tcl_Interp* pInterp)
//  Operation Type:
//     Constructor.
//
CTclAuthorizer::CTclAuthorizer(Tcl_Interp* pInterp) :
  CTCLProcessor("serverauth", m_pInterpreter = new CTCLInterpreter(pInterp)),
  m_pResult(0)
{
  m_pHostNames = new CTCLVariable(m_pInterpreter, NameVariable, TCLPLUS::kfFALSE);
  m_pHostIps   = new CTCLVariable(m_pInterpreter, IPVariable, TCLPLUS::kfFALSE);
  Register();
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     operator()(CTCLInterpreter& rInterp, CTCLResult& rResult,
//                int nArgs, char* pArgs[])
//  Operation Type: 
//     Behavioral override.
int CTclAuthorizer::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			       int nArgs, char* pArgs[])
{
  m_pResult = &rResult;
  int         retval = Process(rInterp, rResult, nArgs, pArgs);
  m_pResult = (CTCLResult*)TCLPLUS::kpNULL;
  return retval;
}
int CTclAuthorizer::Process(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			       int nArgs, char* pArgs[])  
{
  // Processes the serverauth command.
  //  This command has the forms:
  //
  //   serverauth add hostorip          ;# Allows host or IP address
  //   serverauth remove hostorip    ;# Disallows host or IP address
  //   serverauth list                         ;# Lists the authorized set.
  //
  nArgs--; pArgs++;
  if(nArgs < 1) {
    return Usage(rResult);
  }
  if(strcmp("add", pArgs[0]) == 0) {
    nArgs--;
    pArgs++;
    if(nArgs != 1) {
      return Usage(rResult);
    }
    return AddHost(string(pArgs[0])) ? TCL_OK : TCL_ERROR;
  }
  else if (strcmp("remove", pArgs[0]) == 0) {
    nArgs--;
    pArgs++;
    if(nArgs != 1) {
      return Usage(rResult);
    }
    return RemoveHost(string(pArgs[0])) ? TCL_OK : TCL_ERROR;
  }
  else if (strcmp("list", pArgs[0]) == 0) {
    nArgs--;
    pArgs++;
    if(nArgs != 0) {
      return Usage(rResult);
    }
    rResult = ListHosts();
    return TCL_OK;
  }
  else {
    return Usage(rResult);
  }
  rResult = string("Bug in CTclAuthorizer::operator() decode section");
  return TCL_ERROR;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     AddHost(const string& HostOrIp)
//  Operation Type: 
//     
TCLPLUS::Bool_t CTclAuthorizer::AddHost( const string& HostOrIp)  
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
    setResult("Duplicate host or ip address");
    return TCLPLUS::kfFALSE;
  }
  string hostname, hostip;
  if(!ConvertHost(HostOrIp, hostname, hostip)) {
    setResult("Invalid host or IP address");
    return TCLPLUS::kfFALSE;
  }
  // Append the hostname and hostip to the list:
  //
  StringArray hosts;
  StringArray ips;
  CTCLList HostList(m_pInterpreter,string(""));
  CTCLList IpList(m_pInterpreter,string(""));

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
TCLPLUS::Bool_t CTclAuthorizer::RemoveHost(const string& NameOrIp)  
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
    setResult("Host or IP address is not in the authorization list");
    return TCLPLUS::kfFALSE;
  }
  StringArray hostnames;
  CTCLList    HostList(m_pInterpreter, 
		       m_pHostNames->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  HostList.Split(hostnames);

  StringArray hostips;
  CTCLList    IpList(m_pInterpreter, 
		     m_pHostIps->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  IpList.Split(hostips);

  int nLast = hostnames.size() - 1; // Index of last item.
 
  // The delete is done by copying the last entry into the
  // entry the which is to be deleted and doing a pop_back() to kill off
  // the last entry:

  hostnames[idx] = hostnames[nLast];
  hostips[idx]   = hostips[nLast];
  hostnames.pop_back();
  hostips.pop_back();
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
string CTclAuthorizer::ListHosts()  
{
  // Returns a TCL formatted list whose elements are two entry
  // lists containing {hostname ipaddress}
  // Note that if the IP address is not resolvable,
  // the name has no real meaning.
  //

  CTCLString Result;

  // The result list is built up in Result and then returned.
  // This cannot fail.

  StringArray hostnames;
  CTCLList    HostList(m_pInterpreter, 
		       m_pHostNames->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  HostList.Split(hostnames);

  StringArray hostips;
  CTCLList    IpList(m_pInterpreter, 
		     m_pHostIps->Get(TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY));
  IpList.Split(hostips);

  assert(hostips.size() == hostnames.size());

  for(int i=0; i < hostips.size(); i++) {
    Result.StartSublist();
    Result.AppendElement(hostnames[i]);
    Result.AppendElement(hostips[i]);
    Result.EndSublist();
  }
  string sResult = Result;
  return sResult;
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Authenticate(const string& rNameOrIp)
//  Operation Type: 
//     
TCLPLUS::Bool_t CTclAuthorizer::Authenticate(const string& rNameOrIp)  
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
TCLPLUS::Bool_t CTclAuthorizer::HostToIp(string& rName)  
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

  struct hostent* pHostInfo;
  struct hostent  entry;
  char            buffer[1024];
  int             error;
  if (gethostbyname_r(rName.c_str(),
		      &entry, buffer, sizeof(buffer),
		       &pHostInfo, &error)) {

    IpAddress.s_addr = inet_addr(rName.c_str());
  }
  else {
    memcpy(&IpAddress, entry.h_addr, sizeof(IpAddress));
  }
  union {
    uint32_t along;
    uint8_t  bytes[4];
  } IpMyOrder;
  IpMyOrder.along = ntohl(IpAddress.s_addr);
  sprintf(Result, "%03d.%03d.%03d.%03d",
	  IpMyOrder.bytes[3], IpMyOrder.bytes[2],
	  IpMyOrder.bytes[1], IpMyOrder.bytes[0]);

  string R(Result);
  rName = R;
  return TCLPLUS::kfTRUE;

}

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetIndex(const string& rHostOrIp)
//  Operation Type: 
//     
TCLPLUS::Int_t CTclAuthorizer::GetIndex(const string& rHostOrIp)  
{
  // Returns the index of the entry corresponding
  // to an input IP name or Address or -1 if no match.
  // This utility contains common code between 
  // Add, Delete and Authenticate.

  string Host(rHostOrIp);
  if(!HostToIp(Host)) return -1; // Invalid host.
  int flags = TCL_LEAVE_ERR_MSG;
  flags    |= TCL_GLOBAL_ONLY;
  if(!m_pHostIps->Get(flags)) return -1;


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
//   Bool_t ConvertHost(const string& rInName, 
//		        string& rOutname, string& rCanonicalIP)
// Operation type:
//   Utility:
//
TCLPLUS::Bool_t 
CTclAuthorizer::ConvertHost(const string& rInName, 
							string& rOutname, 
							string& rCanonicalIP)
{
  // Converts the input name into the information required to
  // insert it into the host access list.

  string myname(rInName);
  rCanonicalIP = rInName;
  if(!HostToIp(rCanonicalIP)) return TCLPLUS::kfFALSE;

  struct hostent* pEntry;
  struct hostent  entry;
  char            data[1024];
  int             error;
  int status = gethostbyname_r(myname.c_str(),
			       &entry, data, sizeof(data),
			       &pEntry, &error);




  rOutname = status ? string(">unresolved<") : myname;
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
CTclAuthorizer::Usage(CTCLResult& rResult)
{
  // Fills the result string with command usage information.
  // returns TCL_ERROR making calls like
  //  return Usage(rResult)
  // a useful simplification.

  rResult   = string("Usage:\n");
  rResult  += string("  serverauth add hostorip\n");
  rResult  += string("  serverauth remove hostorip\n");
  rResult  += string("  serverauth list\n");
  return TCL_ERROR;
}
