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

#ifndef CDEADSOURCECOMMAND_H
#define CDEADSOURCECOMMAND_H
/**
 * @file CDeadSourceCommand.h
 * @brief Implements Tcl command to mark an event source dead by id.
 */

#include <TCLObjectProcessor.h>


// Forward definitions:

class CTCLInterpereter;
class CTCLObjectProcessor;
class CTCLObject;

/**
 * @class CDeadSourceCommand
 *
 * Provides a command that marks an event source as dead with the
 * fragment processing singleton.  The format of the command is:
 *
 * \verbatim
 *   deadsource id
 *  \endverbatim
 *
 *  Where id is the source id of a source that just died.  Note that
 *  if data comes from a source it is no longer considered dead.
 */
class CDeadSourceCommand : public CTCLObjectProcessor
{
  // Allowed canonicals.

public:
  CDeadSourceCommand(CTCLInterpreter& interp, std::string command);
  ~CDeadSourceCommand();

  // Canonicals that are not allowed
private:
  CDeadSourceCommand(const CDeadSourceCommand&);
  CDeadSourceCommand& operator=(const CDeadSourceCommand&);
  int operator==(const CDeadSourceCommand&) const;
  int operator!=(const CDeadSourceCommand&) const;

  // public interfaces:

  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};  

#endif
