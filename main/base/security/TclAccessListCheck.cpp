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

static const char* Copyright= "(C) Copyright Michigan State University 1936, All rights reserved";// Class: CTclAccessListCheck
// Like CAccessListCheck, but the strings are in Tcl lists.
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
#include "TclAccessListCheck.h"    				
#include "AccessListCheck.h"
#include "TCLList.h"

using namespace std;


static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved TclAccessListCheck.cpp \n";

// Functions for class CTclAccessListCheck

///////////////////////////////////////////////////////////////////////////
//
//  Function:
//     CTclAccessListCheck (Tcl_Interp* pInterp, const string& rName)
//  Operation Type:
//     Constructor.
//
CTclAccessListCheck::CTclAccessListCheck(Tcl_Interp* pInterp, 
										 const std::string& rName) :
  CAuthenticator(),
  m_pAccessList(0),
  m_pInterp(0)
{
  m_pInterp     = new CTCLInterpreter(pInterp);
  m_pAccessList = new CTCLVariable(m_pInterp, rName, TCLPLUS::kfFALSE); 
}

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Authenticate(CInteractor& rInteractor)
//  Operation Type: 
//     
bool CTclAccessListCheck::Authenticate(CInteractor& rInteractor)  
{
  // Checks a string against a list of TCL strings, if there's a match, the
  // client is authenticated else, not.
  // This is done by converting the list in the tcl variable into
  // entries in a CAccessListCheck authenticator and then authenticating
  // against that:

  CAccessListCheck* pAccessList   = GenerateList();
  bool result = pAccessList->Authenticate(rInteractor);
  delete pAccessList;

  return result;
  
  
}
///////////////////////////////////////////////////////////////////////
//
// Function:
//    CAccessListCheck* GenerateList()
// Operation Type:
//    utility.
//
CAccessListCheck* 
CTclAccessListCheck::GenerateList()
{
  CAccessListCheck* pAccessList = new CAccessListCheck;
  std::string            VarValue(m_pAccessList->Get());
  CTCLList          List(m_pInterp, VarValue);
  StringArray       ListValues;
  List.Split(ListValues);

  for(int i = 0; i < ListValues.size(); i ++) {
    pAccessList->AddAclEntry(ListValues[i]);
  }
  return pAccessList;
}
