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
#include "tclUtil.h"

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

  
  CConfiguration* pConfig = getConfiguration();
	std::string name = tclUtil::newName(interp, pConfig, objv);
  
	  // At this point everything should work just fine:

  CCAENChain*      pChain = new CCAENChain;
  CReadoutModule*  pModule= new CReadoutModule(name, *pChain);

  pConfig->addAdc(pModule);
  pConfig->setResult(name);
  return TCL_OK;

}
