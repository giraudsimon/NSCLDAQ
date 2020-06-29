/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CRESETCOMMAND_H
#define CRESETCOMMAND_H

/**
 * @file CResetCommand.h
 * @brief Define the processor for EVB::Reset - reset timestamps etc.
 */

#include <TCLObjectProcessor.h>


// Forward definitions.

class CTCLInterpreter;
class CTCLObject;

/**
 * CResetCommand
 *
 * This class executes the EVB::reset command.  The command resets the
 * event orderer's timestamps back to their original state from their
 * current state.
 */
class CResetCommand : public CTCLObjectProcessor 
{
  // Canonical methods that are implemented:
public:
  CResetCommand(CTCLInterpreter& interp,std::string cmd);
  virtual  ~CResetCommand();

  // Forbidden canonicals:

private:
  CResetCommand(const  CResetCommand&);
  CResetCommand& operator=(const  CResetCommand&);
  int operator==(const  CResetCommand&) const;
  int operator!=(const  CResetCommand&) const;

  // Public interfaces (the command processor):

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
