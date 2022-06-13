/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   COutOfOrderTraceCommand.h
# @brief  Command to establish Tcl handlers for out of order input data.
# @author <fox@nscl.msu.edu>
*/

#ifndef COUTOFORDERTRACECOMMAND_H
#define COUTOFORDERTRACECOMMAND_H

#include <TCLObjectProcessor.h>
#include "CFragmentHandler.h"

class CTCLInterpreter;
class CTCLObject;

/**
* @class COutOfOrderTraceCommand
*
*    This command maintains a list of Tcl command string prefixes
*    that are executed whenever an input queue gets an out of order
*    timestamp.  The commands have the source id, the prior timestamp
*    and the bad timestamp appended to them in that order prior to being
*    executed.
*
*    This is all done by hooking an observer into the non monotonic timestamp
*    observer hook in the fragment handler.
*
*    *  ootrace add  command
*    *  ootrace delete command
*    *  ootrace list
*/
class COutOfOrderTraceCommand : public CTCLObjectProcessor {
    // local data:
    
private:
    CFragmentHandler::NonMonotonicTimestampObserver* m_pObserver;
    
    // canonicals:
    
public:
    COutOfOrderTraceCommand(CTCLInterpreter& interp, const char* pcommand);
    virtual ~COutOfOrderTraceCommand();
    
    // Interface required by the base class:
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Funtional utilities:
    
private:
    void add(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void remove(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
};
#endif