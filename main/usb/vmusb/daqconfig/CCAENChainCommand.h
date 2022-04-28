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
#ifndef __CCAENCHAINCOMMAND_H
#define __CCAENCHAINCOMMAND_H


#ifndef __CADCCOMMAND_H
#include "CADCCommand.h"
#endif


/*!
    This class creates and configures CAEN Chains for the VM-USB readout software.
    The command is derived from the CADCCommand and therefore supports the following 
    syntaxes:

    \verbatim
       caenchain create chain-name
       caenchain config chain-name config-params
       caenchain cget   chain-name
    \endverbatim

    The create form is used to create a new caenchain, where caenchain's represent
    CBLT readout chains.  The chain-name will be used to refer to this chain in other
    operations.

    caenchain config configures a chain.  At present, the valid configuration options are:

    - -base  the value is a uint32_t that represents the MCST/CBLT address that will be 
             programmed into the modules that make up the chain.
    - -modules The value is a properly formatted list of C785 ADC modules that will make up the
               chain.  The modules must have already been made.  The modules must be C785 devices
               (you cannot nest chains).  There must be at least 2 modules in a chain
               (required by the hardware).  The modules must occupy a contiguous set of slots
               in the backplane (hardware requirement).  The first module listed must be the
               left most module in the crate.  The last module the right most.  It is recommmended
               therfore that modules be listed left to right.

    The cget command returns the entire module configuration.

*/

class CCAENChainCommand : public CADCCommand
{
public:
  CCAENChainCommand(CTCLInterpreter& interp,
		    CConfiguration&  config,
		    std::string      commandName = std::string("caenchain"));
  virtual ~CCAENChainCommand();

  // forbidden caonicals.

private:
  CCAENChainCommand(const CCAENChainCommand& rhs);
  CCAENChainCommand& operator=(const CCAENChainCommand& rhs);
  int operator==(const CCAENChainCommand& rhs) const;
  int operator!=(const CCAENChainCommand& rhs) const;
public:

private:
  virtual int create(CTCLInterpreter& interp,
		     std::vector<CTCLObject>& objv); // We just need to be sure a chain is created.
  virtual void Usage(std::string msg, std::vector<CTCLObject>& obvj);
};
#endif
