
#ifndef CRINGCOMMAND_H
#define CRINGCOMMAND_H
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


#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/*!
   Provides access to ring buffers from Tcl. The specific command ensemble supported are:

   \verbatim
   ringbuffer create  name ?size ?maxconsumers??
   ringbuffer format name ?maxconsumers?
   ringbuffer disconnect producer name
   ringbuffer disconnect consumer name index
   ringbuffer usage name
   ringbuffer remove name
\endverbatim

  The meaning of these should be reasonably obvious...though the actual data returned by the
  usage may  not.  The return value is a list of the form

\verbatim
bufferspace putspace maxconsumers producer-pid maxgetspace mingetspace consumers
\endverbatim

Where the consumers element is itself a list of two element sublists that are the
PID and available data size for each consumer client.


*/
class CRingCommand : public CTCLObjectProcessor
{
  // Constructors and canonicals.

public:
  CRingCommand(CTCLInterpreter& interp, std::string command = std::string("ringbuffer"));
  virtual ~CRingCommand();

private:
  CRingCommand(const CRingCommand& rhs);
  CRingCommand& operator=(const CRingCommand& rhs);
  int operator==(const CRingCommand& rhs) const;
  int operator!=(const CRingCommand& rhs) const;

public:

  // Declaration of the command processor that we implement:

  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  // Individual command processors for the ensemble subcommands:

protected:

  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int format(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int disconnect(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);
  int usage(CTCLInterpreter& interp,
	    std::vector<CTCLObject>& objv);
  int remove(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int list(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);

  // private utilities:
private:
  std::string CommandUsage();
};

#endif
