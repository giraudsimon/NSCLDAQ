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
#include "CScalerCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"

#include <C3820.h>
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
CScalerCommand::  CScalerCommand(CTCLInterpreter& interp, CConfiguration& config) :
  CDeviceCommand(interp, "sis3820", config),
  m_Config(config)
{
}
/*!
   The destructor is just needed to provide a virtual chain back to the
   base class destructor.
*/
CScalerCommand::~CScalerCommand()
{
}

///////////////////////////////////////////////////////////////////////
////////////////////////// Command processing /////////////////////////
///////////////////////////////////////////////////////////////////////


/*!
   Process the command:
/
//--ddc may15 NOT  - Ensure there are exactly 4 command parameters.
   - Ensure there are at LEAST 4 command parameters.
   - Ensure the subcommand is create.
   - Ensure the module name does not already exists.
   - Ensure the base address is a valid uint32_t.
   - Create and configure the scaler.
   - Add it to the configuration.

   @param interp : CTCLInterpreter&
       Referes to the interpreter that is running this command.
   @param objv    : std::vector<CTCLObject>&
       reference to a vector of TCL Objects that are the command words.

   @return int
   @retval TCL_ERROR - If the command failed for some reason.
   @retval TCL_OK    - If the command succeeded.

  @note Side effects:   result is modified.
  - If the command succeeded, the result is the name of the new module.
  - If the command failed (returned TCL_ERROR), the result is an error
    messages string that begins with the text "ERROR"
    
  *  Took out a bunch of checking that ddc put in because this is done by the
     config system by declaring -base as an integer.
  *  Subclassed as a Device Command which gets us the rest for free.
*/
int
CScalerCommand:: create(CTCLInterpreter& interp, 
			 std::vector<CTCLObject>& objv)
{
  //--ddc may15, make changes to accept the modification for -timestamp
  //NOTE this did not have 'config' option before, and so the timestamp is
  //minimally handled in the create function.

  if (objv.size() < 4) {
    Usage(interp, "Invalid parameter count", objv);
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


  CReadoutHardware* pHardware = new C3820;
  pModule  = new CReadoutModule(name, *pHardware);
  pModule->configure(string("-base"), sBase);


  if(objv.size() > 5){
    
    int status = configure(interp, pModule, objv, 4);
    if (status == TCL_ERROR) return status;

  }


  m_Config.addAdc(pModule);
  
  m_Config.setResult(name);
  return TCL_OK;

}
