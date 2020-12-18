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

/** @file:  TclShiftInstance.cpp
 *  @brief: Implement the shift instance command.
 */
#include "TclShiftInstance.h"
#include "LogBookShift.h"
#include "LogBookPerson.h"
#include "LogBook.h"
#include "TclLogBookInstance.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

std::map<std::string, TclShiftInstance*> TclShiftInstance::m_shifts;

/**
 * constructor
 *   @param interp -interpreter on which the command object is registered.
 *   @param name   - name of the new command.
 *   @param pShift - Shift being encapsulated.
 */
TclShiftInstance::TclShiftInstance(
    CTCLInterpreter& interp, const char* name,
    LogBookShift* pShift,
    std::shared_ptr<LogBook>& pLogBook
) :
    CTCLObjectProcessor(interp, name, true),
    m_shift(pShift),
    m_logBook(pLogBook)
{
    m_shifts[name] = this;        
}

/**
 * destructor
 */
TclShiftInstance::~TclShiftInstance()
{
    auto p = m_shifts.find(getName());
    if (p != m_shifts.end()) m_shifts.erase(p);
}


/**
 * operator()
 *    extract the subcommand and execute it
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 * @return int - TCL_OK - success, TCL_ERROR failed.
 */
int
TclShiftInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Usage: <shift-instance> <subcommand> ...");
        std::string subcommand(objv[1]);
        if (subcommand == "name") {
            name(interp, objv);
        } else if (subcommand == "members") {
            members(interp, objv);
        } else if (subcommand == "id") {
            id(interp, objv);
        } else if (subcommand == "destroy") {
            delete this;
        } else {
            std::stringstream msg;
            msg << "Invalid shift subcommand: " << subcommand;
            std::string m(msg.str());
            throw m;
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
            "Unexpected exception type caught in TclShiftInstance::operator()"
        );
        return TCL_ERROR;
    }

    return TCL_OK;
    
}

///////////////////////////////////////////////////////////////////////////////
// Static methods

/**
 * getCommandObject
 *    Return a pointer to a command object that has the specifiec command name.
 *  @param name - name of the command - must be a shift command.
 *  @return TclShiftInstance* - Pointer to the command object.
 *  @throw std::out_of_range  - No such shift command instance.
 */
TclShiftInstance*
TclShiftInstance::getCommandObject(const std::string& name)
{
    auto p = m_shifts.find(name);
    if (p == m_shifts.end()) {
        std::stringstream msg;
        msg << name << " is not an instance command for a shift";
        std::string e = msg.str();
        throw std::out_of_range(e);
    }
    return p->second;
}
///////////////////////////////////////////////////////////////////////////
// Command processors.

/**
 * name
 *    Return the name of the shift as the result.
 * @param interp -interpreter running the command.
 * @param objv   - The command words.
 */
void
TclShiftInstance::name(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <shift-instance> name");
    return interp.setResult(m_shift->name());
}
/**
 * members
 *    Return Tcl encapsulated members of the shift.  Each member of the result
 *    list is a person command ensemble.
 * @param interp  - interpreter running the command.
 * @param objv    - The command words.
 */
void
TclShiftInstance::members(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <shift-instance> members");
    auto members = m_shift->members();
    
    // Since shift manipulation can destroy its person instances, we need
    // to make copies of the people on the shift as we wrap them.
    CTCLObject result;
    result.Bind(interp);
    for (auto pMember : members) {
        CTCLObject item;
        item.Bind(interp);
        LogBookPerson* pPerson = m_logBook->getPerson(pMember->id());
        std::string personCmd = TclLogBookInstance::wrapPerson(interp, pPerson);
        item = personCmd;
        result += item;
    }
    interp.setResult(result);
    
}
/**
 *' id
 *     Return the shift's primary key as the command's result.
 *  @param interp - interpreter running the command.
 *  @param objv   - command words.
 */
void
TclShiftInstance::id(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Usage: <shift-instance> id");
    int id = m_shift->id();
    CTCLObject result;
    result.Bind(interp);
    result = id;
    interp.setResult(result);
}