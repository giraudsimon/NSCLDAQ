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
 * The first command object is the socket -  used by the fragment handler.
 * The second the message body containing the fragments.
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
    if (objv.size() != 3) {
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
    // The second object is a byte array object containing the message body:
		
    int msgLength;
    const unsigned char* msg;
    
    msg = Tcl_GetByteArrayFromObj(objv[2].getObject(), &msgLength);
    
    // Dispatch the body as the flattened fragments they are:
    
    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->addFragments(msgLength, reinterpret_cast<const EVB::FlatFragment*>(msg));
    



    
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

#ifdef COMPILEDHANDLER
int
CFragmentHandlerCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);
  
  // Note that all errors are handled by the caller and indicate the connection
  // should be marked closed/lost.  That too is done by the caller.
  
  try {
    requireExactly(objv, 2, "Fragment handler needs exactly 2 parameters");
    std::string chanName = objv[1];
    Tcl_Channel channel = getChannel(chanName);
    
    std::string messageType;
    readHeader(messageType, channel);

    if (messageType == "FRAGMENTS") {
      sendOk(channel);                // do it here to let sender start marshalling next block.
      readBlock(channel);            // m_usedSize bytes Block in m_messageBuffer
      CFragmentHandler* pHandler = CFragmentHandler::getInstance();
      pHandler->addFragments(
	  m_usedSize, reinterpret_cast<EVB::pFlatFragment>(m_messageBuffer)
      );
      
    } else  if (messageType == "DISCONNECT") {
      sendOk(channel);
      throw "Peer Disconnected";
    } else {
      sendError(channel);
      std::string message = "Expected FRAGMENTS message type got: ";
      message += messageType;
      throw message;
    }
        
  }
  catch (const std::string& msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  } catch (const char* msg) {
    interp.setResult(msg);
    return TCL_ERROR;
    
  } catch (CException& e) {
    interp.setResult(e.ReasonText());
    return TCL_ERROR;
  } catch (std::exception& e) {
    interp.setResult(e.what());
    return TCL_ERROR;
  } catch (...) {
    
  }
  
  return TCL_OK;
}
#endif
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
  nBytes++;                        // This allows the caller to null terminate a read.
  if (m_bufSize < nBytes) {
    delete []m_messageBuffer;
    m_messageBuffer = new uint8_t[nBytes];
    m_bufSize = nBytes;
  }
  return m_messageBuffer;

}
/**
 * readHeader
 *    Read a part into an std::string - this string is, presumably
 *    the header string:
 * @param header - references the string (so we have no  copy construction).
 * @param chan   - the Tcl Channel to read from.
 * @throw const char* - If the read does not complete normally.
 */
void
CFragmentHandlerCommand::readHeader(std::string& header, Tcl_Channel chan)
{
  readBlock(chan);
  m_messageBuffer[m_usedSize] = 0;
  header = reinterpret_cast<char*>(m_messageBuffer);
  
}
/**
 * readBlock
 *    Read a block of data into m_messageBuffer
 *    If necessary the block is increased in size (by getBuffer)
 *    If the reads fail, then we throw a const char* exception
 *
 * @param chan - the channel to read from
 */
void
CFragmentHandlerCommand::readBlock(Tcl_Channel chan)
{
  uint32_t s = readPartSize(chan);
  uint8_t* p = getBuffer(s);
  int      n = Tcl_Read(chan, reinterpret_cast<char*>(m_messageBuffer), s);
  if (n != s) {
    throw "Failed to read data from a data source";
  }
  m_usedSize = s;
  
}
/**
 * readPartSize
 *    Reads and returns the size of a message part.  Each message part is
 *    has a size and then the body of the message.
 *
 *  @param chan - the Tcl Channel.
 *  @return uint32_t - the size read
 *  @thrw const char* - if the read failed.
 */
uint32_t
CFragmentHandlerCommand::readPartSize(Tcl_Channel chan)
{
  uint32_t result;
  int s = Tcl_Read(chan, reinterpret_cast<char*>(&result), sizeof(result));
  if (s != sizeof(result)) {
    throw "Failed to read part size from a data source";
  }
  return result;
}


/**
 * sendOk
 *    Send the OK ack string to the peer.
 *
 * @param chan - the channel on which to send.
 * @throw const char* the write failed.
 */
void
CFragmentHandlerCommand::sendOk(Tcl_Channel chan)
{
    static const char* msg = "OK\n";
    static const int s     = strlen(msg);
    
    if (Tcl_Write(chan, msg, s) != s) {
      throw "Positive ACK (OK) send failed";
    }
    Tcl_Flush(chan);                 // Not sure this is needed given buffering none
}
/**
 * sendError
 *    Send error to  a peerk.
 * @param chan - the chan to write on.
 * @throw const char*  - if the write failed.
 */
void
CFragmentHandlerCommand::sendError(Tcl_Channel chan)
{
    static const char* msg = "ERROR\n";
    static const int s     = strlen(msg) +1;
    
    if (Tcl_Write(chan, msg, s) != s) {
      throw "Positive ACK (OK) send failed";
    }
    Tcl_Flush(chan);                 // Not sure this is needed given buffering none  
}
