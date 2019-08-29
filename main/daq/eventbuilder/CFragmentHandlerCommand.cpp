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
#include "CFragmentHandlerCommand.h"
#include "fragment.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>

#include <stdint.h>
#include <Exception.h>
#include <stdexcept>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>

/**
 * Construct the object:
 * @param interp - reference to an encpasulated interpreter.
 * @param name   - Name to give to the command.
 * @param registerMe - Optional parameter which if true (default) autoregisters the command.
 * 
 */
CFragmentHandlerCommand::CFragmentHandlerCommand(CTCLInterpreter& interp,
						std::string name,
						bool registerMe) :
  CTCLObjectProcessor(interp, name, registerMe), m_messageBuffer(nullptr), m_bufSize(0)
{
  
}

/**
 * Destructor
 */
CFragmentHandlerCommand::~CFragmentHandlerCommand() {
  delete []m_messageBuffer;
}

/**
 * Command processor
 * - Ensure a channel name is present.
 * - Drain the message body from the channel
 *
 * @param interp - reference to the encapsulated interpreter.
 * @param objv   - reference to a vetor of encpasulated Tcl_Obj*'s.
 *
 * @return int
 * @retval TCL_OK - success.
 * @retval TCL_ERROR -Failure.
 *
 * @note: TODO:  Deal with running on a big endian system:
 */
int
CFragmentHandlerCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // objv must have the command name and a socket name:
  
  int status = TCL_OK;
  uint8_t* msgBody(0);

  try {
    
    if (objv.size() != 2) {
      interp.setResult(std::string("Incorrect number of parameters"));
      return TCL_ERROR;
    }
    // Translate the channel name to a Tcl_Channel: 
    
    objv[1].Bind(interp);
    std::string channelName = objv[1];
    Tcl_Channel pChannel = getChannel(channelName);
    if (pChannel == NULL) {
      interp.setResult(std::string("Tcl does not know about this channel name"));
      return TCL_ERROR;
    }
    

    // Read the message length:

    uint32_t msgLength;
    int n = Tcl_Read(pChannel, reinterpret_cast<char*>(&msgLength), sizeof(msgLength));
    if (n != sizeof(msgLength))  {
      interp.setResult("Message Length read failed");
      return TCL_ERROR;
    }

    // ..and the body itself.


    if (msgLength > 0) {
      msgBody = getBuffer(msgLength);
      n    = Tcl_Read(pChannel, reinterpret_cast<char*>(msgBody), msgLength);
      if(n != msgLength) {
        throw std::string("Message Body could not be completely read");
      }
      
      // Dispatch the body as the flattened fragments they are:
      
      CFragmentHandler* pHandler = CFragmentHandler::getInstance();
      pHandler->addFragments(msgLength, reinterpret_cast<EVB::pFlatFragment>(msgBody));
    }
    


    
    
    
  }
  catch (const char* m) {
    interp.setResult(m);
    status = TCL_ERROR;
  }
  catch (std::string msg) {
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (CException& e) {
    std::string msg = e.ReasonText();
    msg += ": ";
    msg += e.WasDoing();
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (std::exception& e) {
    interp.setResult(e.what());
    status = TCL_ERROR;
  }
  catch (int i) {
    char msg[1000];
    sprintf(msg, "Integer exception: %d if errno: %s\n", i, strerror(i));
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (...) {
    interp.setResult("Unanticipated exception in fragment handler");
    status = TCL_ERROR;
  }
  return status;
  
}
/////////////////////////////////////////////////////////////////////////
// Internal utility methods.
//

/**
 * getChannel
 *     Given a channel name return the underlying Tcl_Channel.  Note that
 *     the channel is cached in the m_channelMap for presumably faster lookup
 *     next time around.
 *
 * @param name - channel name e.g. file123
 * @return Tcl_Channel nullptr if there's no matching channel.
 */
Tcl_Channel
CFragmentHandlerCommand::getChannel(const std::string& name)
{
  auto p = m_channelMap.find(name);
  if (p != m_channelMap.end()) {
    return p->second;
  } else {
    Tcl_Channel result =
      Tcl_GetChannel(getInterpreter()->getInterpreter(), name.c_str(), NULL);
    if (result) {
      m_channelMap[name] = result;
    }
    return result;
  }
}
/**
 * If needed, expand the buffer.
 *
 * @param nBytes number of byte needed.
 * @retrun uint8_t* - pointer to the buffer.
 */
uint8_t*
CFragmentHandlerCommand::getBuffer(size_t nBytes)
{
  if (m_bufSize < nBytes) {
    delete []m_messageBuffer;
    m_messageBuffer = new uint8_t[nBytes];
  }
  return m_messageBuffer;

}