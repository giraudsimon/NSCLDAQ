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
# @file   CXonXoffCallbackCommand.cpp
# @brief  Implement command to manipulate Xon/Xoff Tclscript callbacks.
# @author <fox@nscl.msu.edu>
*/

#include "CXonXOffCallbackCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CFragmentHandler.h"


/**
 *  Observer class we will use as a container for the handler scripts.
 */
class TclFlowObserver : public CFragmentHandler::FlowControlObserver
{
private:
    std::string m_XonCommand;
    std::string m_XoffCommand;
    CTCLInterpreter& m_interp;
    
    // Canonicals:
public:
    TclFlowObserver(CTCLInterpreter& interp, std::string xoncmd, std::string xoffcmd);
    
    // The FlowControlObserver interface:
    
    void Xon();
    void Xoff();
    
    // Additional public methods:
    
public:
    std::string xonCommand() const {return m_XonCommand;}
    std::string xoffCommand() const {return m_XoffCommand;}
    
    // Private utilities.
private:
    void dispatch(std::string cmdBase);
    
    
};
// Implementing the observer class.

TclFlowObserver::TclFlowObserver(CTCLInterpreter& interp, std::string xoncmd, std::string xoffcmd) :
    m_XonCommand(xoncmd), m_XoffCommand(xoffcmd), m_interp(interp) {}
    
/**
 *  Xon
 *   Flow control allows more data:
 */
void
TclFlowObserver::Xon()
{
    dispatch(m_XonCommand);
}
/**
 * Xoff
 *  Stop the flow of data
 */
void
TclFlowObserver::Xoff()
{
    dispatch(m_XoffCommand);
}
/**
 * dispatch
 *   Run a script
 *
 *   @param cmdBase - Name of command to run,.
 */
void
TclFlowObserver::dispatch(std::string cmdBase)
{
    m_interp.GlobalEval(cmdBase);
}


/*----------------------------------------------------------------------------
 *  Implementation of the main class.
 *----------------------------------------------------------------------------
 
 
 /**
 * constructor
 *    Create/register the command.
 *
 * @param interp - reference to the interpreter on which the command will be
 *                 registered.
 * @param command - Command string.
 */
 CXonXoffCallbackCommand::CXonXoffCallbackCommand(CTCLInterpreter& interp, std::string command) :
    CTCLObjectProcessor(interp, command, true) {}
    
/**
 * destructor
 *    The list will delete itself but we need to delete the items in the list as they are
 *    dynamically allocated.
 */
CXonXoffCallbackCommand::~CXonXoffCallbackCommand()
{
    std::list<TclFlowObserver*>::iterator p =
        m_observers.begin();
    while (p != m_observers.end()) {
        delete *p;
        p++;
    }
    // Let the std::list clean up its own contents.
}

/**
 * operator()
 *    Gets control when the command is entered  We need to ensure there's exactly
 *    4 command line arguments:  command add|remove  xonscript xoffscript
 *    We then pull out the subcommand and dispatch based on it.
 *
 * @param interp - The interpreter on which the command is running.
 * @param objv   - The vector of wrapped Tcl_Obj*s that make up the command.
 *
 * @return int  TCL_OK on success TCL_ERROR on failure with an error message
 *              in the result on failure.
 */
int
CXonXoffCallbackCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 4, "Incorrect number of command line parameters");
        std::string subcommand = objv[1];
        
        if (subcommand == "add") {
            add(interp, objv);
        } else if (subcommand == "remove") {
            remove(interp, objv);
        } else {
            throw std::string("Invalid sub-command keyword, expected add | remove");
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * add
 *    Adds a new script handler.
 *    * Create a new observer wrapper
 *    * Add the wrapper to the set of Xon/Xoff flow control handlers.
 *    * Add it to our list of handlers so we  can look it up later.
 *
 *  @param interp - TCL Interpreter that is running the command.
 *  @param objv   - The command line parameters.
 */
void
CXonXoffCallbackCommand::add(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    std::string xon  = objv[2];
    std::string xoff = objv[3];
    TclFlowObserver* pObserver = new TclFlowObserver(interp, xon, xoff);
    m_observers.push_back(pObserver);
    CFragmentHandler::getInstance()->addFlowControlObserver(pObserver);
}
/**
 * remove
 *   Removes an existing observer
 *   Locate the matching observer in the list and get its pointer (error if not found)
 *   Remove the observer from the fragment handler.
 *   Remove the observer from ourlist
 *   
 *  @param interp - TCL Interpreter that is running the command.
 *  @param objv   - The command line parameters.
 */
void
CXonXoffCallbackCommand::remove(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    std::string xon   = objv[2];
    std::string xoff  = objv[3];
    
    std::list<TclFlowObserver*>::iterator p = m_observers.begin();
    while (p != m_observers.end()) {
        TclFlowObserver* pObserver = *p;
        CFragmentHandler::getInstance()->removeFlowControlObserver(pObserver);
        m_observers.erase(p);
        delete pObserver;
        return;
    }
    throw std::string("No handler has been established for this pair of scripts");
    
}