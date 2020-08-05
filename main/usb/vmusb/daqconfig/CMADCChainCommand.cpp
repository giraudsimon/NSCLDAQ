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
#include <CMADCChainCommand.h>
#include "CConfiguration.h"
#include <CReadoutModule.h>
#include <CMADCChain.h>
#include "tclUtil.h"

using namespace std;

/*!
 * Construct the command.  We are going to delegate everything to the
 * base class constructor.
 * @param interp - Reference to the CTCLInterpreter that will run this command.
 * @param config - Reference to the configuratino that we will poopulate.
 * @param commandName - Name of the command we are defining.
 *
 * \note the base class constructor really takes care of all of the details:
 */
CMADCChainCommand::CMADCChainCommand(CTCLInterpreter& interp,
				     CConfiguration&  config,
				     string           commandName) :
  CADCCommand(interp, config, commandName)
{}

/*!
 * Destroy the command.
 * - This will never happen in the first place.
 * - In the second place, the base class constructor takes care of all the work needed
 *   to do this.
 */
CMADCChainCommand::~CMADCChainCommand()
{
}

/*!
 * Creates a new madc chain.  
 *  @param interp - The interpreter that is running this command.
 *  @param objv   - Vector of Tcl_Obj encapsulated command parameters.
 */
int
CMADCChainCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{

	CConfiguration* pConfig = getConfiguration();
  string   name    = tclUtil::newName(interp, pConfig, objv);
	if (name == "") return TCL_ERROR;
	
  if (objv.size() < 4) {
    Usage(interp, "Not enough parameters for create subcommand", objv);
    return TCL_ERROR;
  }

  // At this point everything should work just fine:

  CMADCChain*      pChain = new CMADCChain;
  CReadoutModule*  pModule= new CReadoutModule(name, *pChain);

  pConfig->addAdc(pModule);
  pConfig->setResult(name);
  return TCL_OK;
 
}

/*
   The usage command needs to return a usage specific to our requirements.
*/
void
CMADCChainCommand::Usage(string msg, vector<CTCLObject>& objv)
{
  std::string usage("Usage\n");
  usage += "    madcchain create name\n";
  usage += "    madcchain config name config-params\n";
  usage += "    madcchain cget   name\n";
	
	
  tclUtil::Usage(*getInterpreter(), msg, objv, usage);
}

