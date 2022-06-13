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
#include "CCAENChainCommand.h"
#include "CConfiguration.h"
#include <CReadoutModule.h>
#include <CCAENChain.h>

using namespace std;


/*!
   Construct the command.. we're going to delegate everything to the
   base class constructor.
   \param interp  CTCLInterpreter&  
          Reference to the interpreter that will run this command.
   \param config  CConfiguration&
          Reference to the configuration that we will populate.
   \param commandName std::string
          Name of the command to register.
      
*/
CCAENChainCommand::CCAENChainCommand(CTCLInterpreter& interp,
				     CConfiguration&  config,
				     string           commandName) :
  CADCCommand(interp, config, commandName)
{}

/*!
   There is nothing for us to do for destruction.  Let the base classes
   deal with that.
*/
CCAENChainCommand::~CCAENChainCommand()
{}


/*  Process the create subcommand.. this is the only way in which
    we differ from the adc command:
    - We don't need a base address.
    - We create a CCAENChain not a C785.

*/
int
CCAENChainCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // need exactly 3 elements, command, create, name.

  if (objv.size() != 3) {
    Usage("Incorrect parameter count for create subcommand", objv);
    return TCL_ERROR;
  }

  // Get the chain name.  This must not be the name of an existing 'adc' module.

  string   name    = objv[2];
  CConfiguration* pConfig = getConfiguration();

  if (pConfig->findAdc(name)) {
    Usage("Duplicate module creation attempted", objv);
    return TCL_ERROR;
  }

  // At this point everything should work just fine:

  CCAENChain*      pChain = new CCAENChain;
  CReadoutModule*  pModule= new CReadoutModule(name, *pChain);

  pConfig->addAdc(pModule);
  pConfig->setResult(name);
  return TCL_OK;

}
/*
   The usage command needs to return a usage specific to our requirements.
*/
void 
CCAENChainCommand::Usage(string msg, vector<CTCLObject>& objv)
{
  string result("ERROR: ");
  result += msg;
  result += "\n";
  for (int i =0; i < objv.size(); i++) {
    result += string(objv[i]);
    result += ' ';
  }
  result += "\n";

  result += "Usage\n";
  result += "    caenchain create name\n";
  result += "    caenchain config name config-params\n";
  result += "    caenchain cget   name\n";

  getConfiguration()->setResult(result);
}
