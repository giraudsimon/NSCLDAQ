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
#include "COrdererOutput.h"
#include <CBufferedOutput.h>
#include <TCLInterpreter.h>
#include <iostream>
#include <io.h>
#include <iostream>

#include <string>
#include "fragment.h"

static const int BUFFERSIZE=8192;  // Hard coded for now.
/*------------------------------------------------------------------------
**  Canonical methods
*/

/**
 * Constructor:
 *    Translates the channel name into a Tcl_Channel.  If that fails throws
 *    an exception string.
 *    Gets an instance of the Fragment handler and registers ourselves as an 
 *    observer so that we'll get fragments.
 *    we also ensure the -encoding is binary and the -translation binary
 *
 * @param fd  - File descriptor on which to write dta.
 */
COrdererOutput::COrdererOutput(int fd) :
  m_OutputChannel(fd),
  m_Output(*(new io::CBufferedOutput(fd, BUFFERSIZE))) 
{

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addObserver(this);

}
/**
 * Destructor:
 *    Since this will be invalid, we need to remove ourselves from the listener list:
 */
COrdererOutput::~COrdererOutput()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  delete &m_Output;
  pHandler->removeObserver(this);
}
/*------------------------------------------------------------------------
** Public methods.
*/

/**
 * operator()
 *    Receives a vector of events from the fragment handler.
 *    These events are time ordered, unless there are time order
 *    errors.  The events are output the file descriptor m_OutputChannel
 *    via the buffered I/O package.
 *
 * @param event - vector of fragments to output.
 */
void
COrdererOutput::operator()(const std::list<std::pair<time_t, EVB::pFragment> >& event)
{
  
  for (auto i =event.begin(); i != event.end(); i++) {
    EVB::pFragment p = i->second;;

    try {
      m_Output.put(&(p->s_header), sizeof(EVB::FragmentHeader));
      m_Output.put(p->s_pBody, p->s_header.s_size);
       //io::writeData(m_OutputChannel, &(p->s_header), sizeof(EVB::FragmentHeader));
       //io::writeData(m_OutputChannel, p->s_pBody, p->s_header.s_size);
    }
    catch(int er) {
      std::cerr << " Caught an output exception " << er << std::endl;
      throw;
    }
  }
  m_Output.flush();


}

/*-----------------------------------------------------------------------------
** Utiltity methods
*/

/**
 * Throw an error string given an error with errno:
 *.
 * @param prefixMessage - prefixes the error message.
 */
void
COrdererOutput::ThrowErrnoString(const char* prefixMessage) const
{
  int error = Tcl_GetErrno();
  std::string msg(prefixMessage);
  msg += Tcl_ErrnoMsg(error);
  
  throw msg;
}
