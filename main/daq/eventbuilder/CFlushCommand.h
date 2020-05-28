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
/**
 * @file CFlushCommand.h
 * @brief Define the class that implements EVB::flush  - flush event queues.
 */

#ifndef CFLUSHCOMMAND_H
#define CFLUSHCOMMAND_H

#include <TCLObjectProcessor.h>

// Forward class/type definitions:

class CTCLInterpreter;
class CTCLObject;

/**
 * CFlushCommand
 *
 *  This class, is a TCL Command processor intended to support the EVB::flush comand.
 *  When issued, all fragment queues are immediately flushed tot he empty state.
 *
 * Command format:
 *
 * \code {.tcl}
 *      EVB::flush
 * \endcode
 */
class CFlushCommand : public CTCLObjectProcessor
{
  // Canonicals that are allowed:

public:
  CFlushCommand(CTCLInterpreter& interp, std::string commandName);
  virtual ~CFlushCommand();

  // disallowed canonicals:
private:
  CFlushCommand(const CFlushCommand&);
  CFlushCommand& operator=(const CFlushCommand&);
  int operator==(const CFlushCommand&) const;
  int operator!=(const CFlushCommand&) const;


  // Public entries:

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};
#endif
