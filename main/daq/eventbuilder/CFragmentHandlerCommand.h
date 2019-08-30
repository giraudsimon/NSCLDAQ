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
#ifndef __CFRAGMENTHANDLERCOMMAND_H
#define __CFRAGMENTHANDLERCOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#include <tcl.h>
#include <map>
#include <string>

/**
 * The CFragmentHandlerCommand class provides the EVB::handleFragment
 * command.  This is called after a fragment header has been seen.
 * The command is responsible for reading the body and generating
 * a fragment chain from it.
 *
 * Syntax is:
 *
 * \verbatim
 * EVB::handleFragment socket
 * \verbatim
 */
class CFragmentHandlerCommand : public CTCLObjectProcessor
{
private:
    std::map<std::string, Tcl_Channel> m_channelMap;
    uint8_t*    m_messageBuffer;
    size_t      m_bufSize;
    size_t      m_usedSize;
    
public:
  CFragmentHandlerCommand(CTCLInterpreter& interp,
			 std::string name,
			 bool registerMe = true);
  virtual ~CFragmentHandlerCommand();

protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
private:
    Tcl_Channel getChannel(const std::string& name);
    uint8_t*    getBuffer(size_t nBytes);
    void        readHeader(std::string& header, Tcl_Channel chan);
    void        readBlock(Tcl_Channel chan);
    uint32_t    readPartSize(Tcl_Channel chan);

    void        sendOk(Tcl_Channel chan);
    void        sendError(Tcl_Channel chan);
    
};

#endif
