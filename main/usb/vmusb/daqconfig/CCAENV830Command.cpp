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

#include <config.h>
#include "CCAENV830Command.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"

#include <C830.h>
#include <CReadoutModule.h>

#include <stdlib.h>
#include <errno.h>

using std::string;
using std::vector;


////////////////////////////////////////////////////////////////////////
///////////////////////////////// Canonicals ///////////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command.
   \param interp : CTCLInterpreter&
      Reference to the interpreter on which the command will be registered
   \param config : CConfiguration&
       Configuration we will be maintaining.
*/
CCAENV830Command::  CCAENV830Command(CTCLInterpreter& interp, CConfiguration& config,
				     string name) :
  CDeviceCommand(interp, name.c_str(), config),
  m_Config(config)
{
}
/*!
   The destructor is just needed to provide a virtual chain back to the
   base class destructor.
*/
CCAENV830Command::~CCAENV830Command()
{
}

///////////////////////////////////////////////////////////////////////
////////////////////////// Command processing /////////////////////////
///////////////////////////////////////////////////////////////////////



/*!
   Handle the creation of the module:

*/
int
CCAENV830Command::create(CTCLInterpreter& interp, 
			 std::vector<CTCLObject>& objv)
{
  if(objv.size() < 4) {
    interp.setResult("Insufficient parameters for V830 create command");
    return TCL_ERROR;
  }
  string subcommand = objv[1];
  string name       = objv[2];
  string sBase      = objv[3];

  CReadoutModule*  pModule = m_Config.findAdc(name);
  if (pModule) {
    Usage(interp, "Attempted to create a duplicate scaler", objv);
    return TCL_ERROR;
  }
  
  CReadoutHardware* pHardware = new C830;
  pModule  = new CReadoutModule(name, *pHardware);
  pModule->configure(string("-base"), sBase);  // set base address
  
  
  if(objv.size() > 4) {                     // Additional optional config
    int stat = configure(interp, pModule, objv, 5);
    if (stat != TCL_OK) {
      return stat;
    }
  }
  m_Config.addAdc(pModule);   // ADC/Scaler no difference.

  m_Config.setResult(name);
  return TCL_OK;
}
