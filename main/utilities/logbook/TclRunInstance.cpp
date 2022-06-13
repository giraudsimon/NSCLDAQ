/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  TclRunInstance.pp
 *  @brief:  Implements run instances
 */

#include "TclRunInstance.h"
#include "TclLogBookInstance.h"
#include "TclLogbook.h"

#include <TCLObject.h>
#include <TCLInterpreter.h>

#include "LogBookRun.h"
#include "LogBookShift.h"
#include "LogBook.h"
#include <stdexcept>
#include <sstream>
#include <Exception.h>

#include <sstream>

std::map<std::string, TclRunInstance*> TclRunInstance::m_instanceRegistry;

/**
 * constructor
 *    @param interp - intepreter on which the command will be registered.
 *    @param name   - command name.
 *    @param pRun   - run to wrap.
 *    @param pLogbook - logbook the run lives in.
 */
TclRunInstance::TclRunInstance(
    CTCLInterpreter& interp, const char* name, LogBookRun* pRun, std::shared_ptr<LogBook>& pBook
) :
    CTCLObjectProcessor(interp, name, true),
    m_pRun(pRun),
    m_logbook(pBook)
{
    m_instanceRegistry[name] = this;        
}

/**
 * destructor
 */
TclRunInstance::~TclRunInstance()
{
    delete m_pRun;
    auto p = m_instanceRegistry.find(getName());
    if (p != m_instanceRegistry.end()) {
        m_instanceRegistry.erase(p);
    }
}


/**
 * operator()
 *   Process commands.
 *
 * @param interp - interpreter on which we're running.
 * @param objv   - Command word objects.
 * @return int   - TCL_OK on success, TCL_ERROR on failure.
 */
int
TclRunInstance::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Usage: <run-instance> <subcommand> ?...?");
        std::string subcommand(objv[1]);
        if (subcommand == "destroy" ) {
            delete this;
        } else if (subcommand == "id") {
            id(interp, objv);
        } else if (subcommand == "title") {
            title(interp, objv);
        } else if (subcommand == "number") {
            number(interp, objv);
        } else if (subcommand == "transitions") {
            transitions(interp, objv);
        } else if (subcommand == "isActive") {
            isActive(interp, objv);
        } else {
            std::stringstream msg;
            msg << subcommand
                << " is not a valid subcommand of the run instance command "
                << std::string(objv[0]);
            std::string e(msg.str());
            throw e;
        }
    }
    catch (std::string& msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
        
    }
    catch (std::exception& e) {       // Note LogBook::Exception is derived from this
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult(
            "Unexpected exception type caught in TclPersonInstance::operator()"
        );
        return TCL_ERROR;
    }
    return TCL_OK;
}

//////////////////////////////////////////////////////////////////////////////
// static methods

/**
 * getCommandObject
 *    Looks up a command instance object by command name
 * @param name -name of the command to lookup
 * @return TclRunInstance*  - pointer to the command object.
 * @throw std::out_of_range if there's no match.
 */
TclRunInstance*
TclRunInstance::getCommandObject(const std::string& name)
{
    auto p = m_instanceRegistry.find(name);
    if (p == m_instanceRegistry.end()) {
        std::stringstream msg;
        msg << "There is no run intance command object named: '"
            << name << "'";
        std::string e(msg.str());
        throw std::out_of_range(e);
    }
    return p->second;
}
////////////////////////////////////////////////////////////////////////////////
// private command processing methods.

/**
 * id
 *     Return the primary key of the root run record as the subcommand result.  This can be used
 *     to create references back to the run.
 *  @param interp -interpreter running the command.
 *  @param objv   - The command words.
 */
void
TclRunInstance::id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <run-instance id");
    CTCLObject result;
    result.Bind(interp);
    result = m_pRun->getRunInfo().s_id;
    interp.setResult(result);
    
}
/**
 * title
 *     Set the run's title as the command result.
 *  @param interp -interpreter running the command.
 *  @param objv   - The command words.
 */
void
TclRunInstance::title(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <run-instance> title");
    interp.setResult(m_pRun->getRunInfo().s_title);
}
/**
 * number
 *    Set the run's run number as the command result.
 *  @param interp -interpreter running the command.
 *  @param objv   - The command words.
 */
void
TclRunInstance::number(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <run-instance> number");
    CTCLObject result;
    result.Bind(interp);
    result = m_pRun->getRunInfo().s_number;
    interp.setResult(result);
}
/**
 * isActive
 *    Returns a boolean that indicates if the run is active.
 **  @param interp -interpreter running the command.
 *  @param objv   - The command words.
 */
void
TclRunInstance::isActive(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <run-instance> isActive");
    CTCLObject result;
    result.Bind(interp);
    result = m_pRun->isActive() ? 1 : 0;
    interp.setResult(result);
}
/**
 * transitions
 *    Returns a list of transitions in the result.
 *    Each transition is returne as a dict that's described in the header file.
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 */
void
TclRunInstance::transitions(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <run-instance> transitions");
    CTCLObject result;
    result.Bind(interp);
    size_t n = m_pRun->numTransitions();
    for (int i =0; i < n; i++) {
        CTCLObject item = makeTransitionDict(interp, (*m_pRun)[i]);
        result += item;
    }
    
    interp.setResult(result);
}
///////////////////////////////////////////////////////////////////////////////
// Private utilities.

/**
 * makeTransitionDict
 *    Create a dictionary that describes a transition  See the header for the
 *    key value pairs this has.
 *
 * @param interp - interpreter the command is running on.
 * @param transition - transition struct from the run
 * @return CTCLObject - Encapsulated dict.
 */
CTCLObject
TclRunInstance::makeTransitionDict(
    CTCLInterpreter& interp, const LogBookRun::Transition& transition
)
{
    Tcl_Obj* pDict = Tcl_NewDictObj();
    Tcl_Interp* pInterp = interp.getInterpreter();
    
     
    
    Tcl_Obj* key;
    Tcl_Obj* value;

    // id
    
    key = Tcl_NewStringObj("id", -1);
    value = Tcl_NewIntObj(transition.s_id);
    
    Tcl_DictObjPut(pInterp, pDict, key, value);

    
    // transition(code)
    key = Tcl_NewStringObj("transition", -1);
    value = Tcl_NewIntObj(transition.s_transition);
    
    Tcl_DictObjPut(pInterp, pDict, key, value);
    
    // transitionName
    
    key = Tcl_NewStringObj("transitionName", -1);
    value = Tcl_NewStringObj(transition.s_transitionName.c_str(), -1);
    
    Tcl_DictObjPut(pInterp, pDict, key, value);

    
    // transitionComment
    
    key = Tcl_NewStringObj("transitionComment", -1);
    value = Tcl_NewStringObj(transition.s_transitionComment.c_str(), -1);
    
    Tcl_DictObjPut(pInterp, pDict, key, value);

    // transitionTime:
    
    key = Tcl_NewStringObj("transitionTime", -1);
    value = Tcl_NewWideIntObj(transition.s_transitionTime);
    
    Tcl_DictObjPut(pInterp, pDict, key, value);
    
    // shift
    
    key = Tcl_NewStringObj("shift", -1);
    std::string shiftCommand = copyShift(interp, transition.s_onDuty);
    value = Tcl_NewStringObj(shiftCommand.c_str(), -1);
    
    Tcl_DictObjPut(pInterp, pDict, key, value);

        
    CTCLObject result(pDict);
    result.Bind(interp);
    return result;
    
}
/**
 * copyShift
 *   Given a shift, create a copy of it and wrap that copy.
 *   Copying is needed because shift command deletion can delete the
 *   underlying shift pointer which might still be in play in the
 *   underlying LogBookRun transition.
 *
 * @param shift - reference to the shift to copy.
 * @return std::string - the command name that wraps the new copy of the shift.
 */
std::string
TclRunInstance::copyShift(CTCLInterpreter& interp, LogBookShift* pShift)
{
    LogBookShift* pCopy = m_logbook->getShift(pShift->id());
    
    // This is a bit filthy but gets the job done:
    
    TclLogBookInstance tempInstance(
        &interp, TclLogbook::createObjectName( "__temp__").c_str(), m_logbook
    );
    return tempInstance.wrapShift(interp, pCopy);

}