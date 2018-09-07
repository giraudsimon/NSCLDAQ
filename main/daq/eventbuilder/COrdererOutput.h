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
#ifndef __CORDEREROUTPUT_H
#define __CORDEREROUTPUT_H

#ifndef __CFRAGMENTHANDLER_H
#include "CFragmentHandler.h"
#endif

#ifndef __TCL_H
#include <tcl.h>
#ifndef __TCL_H
#define __TCL_H
#endif
#endif

// Forward definitions:

class CTCLInterpreter;
namespace io {
class CBufferedOutput;
}

/**
 * The orderer output stage is instantiated on a 
 * Tcl Handle name  and attaches itself as an observer to the
 * CFragmentHandler.  As such it is informed of vectors of
 * totally ordered fragments being ready for output and
 * has a chance to send them out the handle passed in at
 * construction time.
 */

class COrdererOutput : public CFragmentHandler::Observer
{
private:
  int              m_OutputChannel;	// where we write the data.
  io::CBufferedOutput& m_Output;  

  // canonicals:

public:
  COrdererOutput(int chan);
  virtual ~COrdererOutput();

  // Unsupported  canonicals:

private:
  COrdererOutput(const COrdererOutput&);
  COrdererOutput& operator=(const COrdererOutput&);
  int operator==(const COrdererOutput&) const;
  int operator!=(const COrdererOutput&) const;

  // Entries required of observers:

public:
  virtual void operator()(const std::list<std::pair<time_t, EVB::pFragment> >& event);

  // Utilities:
private:

  void ThrowErrnoString(const char* prefixMessage) const;
  
};

#endif
