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

#ifndef CBARRIERTRACECOMMAND_H
#define CBARRIERTRACECOMMAND_H

/**
 * @file CBarrierTraceCommand.h
 * @brief Define the barriertrace command class.
 */

#include "TCLObjectProcessor.h"


// forward definitions:

class CTCLInterpreter;
class CTCLObject;
class CCompleteBarrierCallback;
class CPartialBarrierCallback;

/**
 * @class CBarrierTraceCommand
 *
 *  This class implements the barriertrace command.  The command allows you to specify
 *  scripts that will be executed when different types of barrier events are processed.
 *  The legal forms of the command are:
 *
 * \verbatim
 *  barriertrace complete script
 *  barriertrace complete
 *  barriertrace complete ""
 *
 *  barriertrace incomplete script
 *  barriertrace incomplete 
 *  barriertrace incomplete ""
 *
 * \endverbatim
 *
 *  The second word of the command must be either \e complete or \e incomplete and 
 *  specifies the type of trace, either a complete or an incomplete barrier.
 *  If the third word is missing,the current trace is returned (an empty string if
 *  none is established).  If the third word is non-empty it replaces the current
 *  script.  If the third word is blank, any trace is removed.
 *
 *  In all cases the value of the command is the trace script (or an empty string
 *  if there was no trace) prior to command processing.
 */
class CBarrierTraceCommand : public CTCLObjectProcessor
{
  // private data

private:
  CCompleteBarrierCallback* m_pComplete;
  CPartialBarrierCallback*  m_pPartial;

  // Callble canonicals:
public:
  CBarrierTraceCommand(CTCLInterpreter& interp, std::string command);
  virtual ~CBarrierTraceCommand();

  // canonicals that may not be called:

private:
  CBarrierTraceCommand(const  CBarrierTraceCommand&);
  CBarrierTraceCommand& operator=(const  CBarrierTraceCommand&);
  int operator==(const CBarrierTraceCommand&) const;
  int operator!=(const  CBarrierTraceCommand&) const;

  // public interface (command execution):

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // private utilities:

private:

  // This templated class really handles the the command for both
  // complete and incomplete given the right type of observer.

  template <class T>
  std::string  newCallback(T** old, const char* pCommand = 0) {
    std::string result;
    if (*old) result = (*old)->getCommand();
    if (pCommand) {
      delete *old;
      *old = 0;
      if (*pCommand) {
	*old = new T(*(getInterpreter()), std::string(pCommand));
      }
    }
    return result;
  }
};



#endif
