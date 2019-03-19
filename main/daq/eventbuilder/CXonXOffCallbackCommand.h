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
# @file   CXonXoffCallbackCommand.h
# @brief  Defines a command class to establish XON/XOFF Callback handlers.
# @author <fox@nscl.msu.edu>
*/
#ifndef __CXONXOFFCALLBACKCOMMAND_h
#define __CXONXOFFCALLBACKCOMMAND_h
#include <TCLObjectProcessor.h>

#include <list>

class CTCLInterpreter;
class CTCLObject;
class CFragmentHandler;
class TclFlowObserver;

/**
 * @class CXonXoffCallbackCommand
 *
 * Implements a command that allows the registration and removal
 * of Tcl scripts that execute for flow control events
 *
 * This is a command ensemble with the subcommands:
 * *  add xonscript xoffscript - Adds the specified callbacks.
 * *  remove xonscript xoffscript - Remomes the specified callbacks.
 */
class CXonXoffCallbackCommand : public CTCLObjectProcessor
{
private:

    std::list<TclFlowObserver*> m_observers;
    // Valid/legal canonicals:
public:
    CXonXoffCallbackCommand(CTCLInterpreter& interp, std::string command);
    virtual ~CXonXoffCallbackCommand();
    
    // invalid/illegal canonicals:
private:
    CXonXoffCallbackCommand(const CXonXoffCallbackCommand& rhs);
    CXonXoffCallbackCommand& operator=(const CXonXoffCallbackCommand& rhs);
    int operator==(const CXonXoffCallbackCommand& rhs) const;
    int operator!=(const CXonXoffCallbackCommand& rhs) const;
    
    // The CTCLObjectProcessor interface:

public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Subcommand processors:
protected:
    void add(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void remove(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    //Private utilities:
    
private:
    void dispatch(std::string cmdBase);
};

#endif
