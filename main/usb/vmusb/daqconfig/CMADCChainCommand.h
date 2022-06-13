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
#ifndef __CMADCCHAINCOMMAND_H
#define __CMADCCHAINCOMMAND_H

#ifndef __CADCCOMMAND_H
#include "CADCCommand.h"
#endif


/*!
   Creates and configures MADC32 CBLT Chains.  The command is 
   derived from the CADCCommand class and therefor supports the
   following syntaxes

\verbatim
   madcchain create chain-name
   madcchain config chain-name  option value ?...?
   madcchain cget    chain-name
\endverbatim

  The create subcommand is used to define a new chain.
  The config subcommand is used to configure that chain.
  See CMADCChain.h/cpp for the configuration options that are accepted 
  by that command.    The cget subcommand retrieves the current configuration.
  as a list of option-name option-value pairs.

  All of the subcommnds are handled by the base class while this
  class only needs to create and provide command usage.

*/
class CMADCChainCommand : public CADCCommand
{
public:

  CMADCChainCommand(CTCLInterpreter& interp,
		    CConfiguration&  config,
		    std::string      commandName = std::string("madcchain"));
  virtual ~CMADCChainCommand();

  // Forbidden canonicals:

private:
  CMADCChainCommand(const CMADCChainCommand& rhs);
  CMADCChainCommand& operator=(const CMADCChainCommand& rhs);
  int operator==(const CMADCChainCommand& rhs) const;
  int operator!=(const CMADCChainCommand& rhs) const;
public:

  // These virtual functions are private because the base class's 
  // operator() dispatches to them.

private:
  virtual int create(CTCLInterpreter& interp,
		     std::vector<CTCLObject>& objv); // We just need to be sure a chain is created.
  virtual void Usage(std::string msg, std::vector<CTCLObject>& obvj);
    
};
#endif
