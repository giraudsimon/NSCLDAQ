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
#include <TCLInterpreter.h>
#include <iostream>
#include <io.h>
#include <iostream>

#include <string>
#include "fragment.h"

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
  m_OutputChannel(fd)
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
  pHandler->removeObserver(this);
}
/*------------------------------------------------------------------------
** Public methods.
*/

/**
 * operator()
 *    Receives a vector of events from the fragment handler.
 *    These events are time ordered, unless there are time order
 *    errors.  The events are output the Tcl_Channel m_OutputChannel.
 *
 * @param event - vector of fragments to output.
 */
void
COrdererOutput::operator()(const std::vector<EVB::pFragment>& event)
{
  std::cerr << "Outputting " << event.size() << " fragments\n";
  for (int i = 0; i < event.size(); i++) {
    EVB::pFragment p = event[i];
//    std::cerr << "Frag @ " << p->s_header.s_timestamp << std::endl;

    try {
       io::writeData(m_OutputChannel, &(p->s_header), sizeof(EVB::FragmentHeader));
       io::writeData(m_OutputChannel, p->s_pBody, p->s_header.s_size);
    }
    catch(int er) {
      std::cerr << " Caught an output exception " << er << std::endl;
      throw;
    }
  }


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
