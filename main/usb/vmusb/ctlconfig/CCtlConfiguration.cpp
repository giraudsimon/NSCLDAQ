/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CCtlConfiguration.h"
#include <Globals.h>
#include <CMonCommand.h>
#include <CWatchCommand.h>
#include <CModuleCommand.h>
#include <make_unique.h>

#include <TCLInterpreter.h>
#include <TCLVariable.h>
#include <TCLObjectProcessor.h>
#include <Exception.h>

#include <tcl.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>

using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////// Canonicals //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CCtlConfiguration::CCtlConfiguration()
{}

CCtlConfiguration::~CCtlConfiguration()
{
}

////////////////////////////////////////////////////////////////////////////
//////////////////// Support for configuration /////////////////////////////
////////////////////////////////////////////////////////////////////////////

void CCtlConfiguration::addCommand(unique_ptr<CTCLObjectProcessor> pCommand)
{
  m_Commands.push_back( move(pCommand) );
}

void CCtlConfiguration::addModule(CControlModule* pModule)
{
  m_Modules.push_back( pModule );
}

CControlModule* CCtlConfiguration::findModule(const string& name)
{
  return find(m_Modules, name);
}

////////////////////////////////////////////////////////////////////////////
////////////////////// Private utilities ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CControlModule* 
CCtlConfiguration::find(const std::vector<CControlModule* >& modules,
		                    std::string name)
{
  auto itFound = find_if(modules.begin(), modules.end(), MatchName(name));
      
  if(itFound != modules.end()) {
    return *itFound;
  } 
  else {
    return nullptr;
  }
}


bool
CCtlConfiguration::MatchName::operator()(const CControlModule* pModule)
{
  return (pModule->getName() == m_name);
}

