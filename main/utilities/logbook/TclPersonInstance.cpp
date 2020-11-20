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

/** @file:  TclLogbookPersonInstance.cpp
 *  @brief: Implement the logbook person instance command ensemble.
 */
#include "TclPersonInstance.h"
#include "LogBookPerson.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <sstream>

std::map<std::string, TclPersonInstance*> TclPersonInstance::m_personCommands;
/**
 * constructor.
 *   @param interp - interpreter on which the cmd is registered.
 *   @param name   - command name under which the object is registered.
 *   @param pPerson - Pointer to the person to be encapsulated
 */
TclPersonInstance::TclPersonInstance(
    CTCLInterpreter& interp, const char* name, LogBookPerson* pPerson
) :
    CTCLObjectProcessor(interp, name, true),
    m_person(pPerson)
{
    // Save a lookup entry for the command.
    
    m_personCommands[name] = this;        
}
/**
 * destructor
 */
TclPersonInstance::~TclPersonInstance()
{
    // Destroy a lookup entry for the command.
    
    auto mapEntry = m_personCommands.find(getName());
    if (mapEntry != m_personCommands.end()) {
        m_personCommands.erase(mapEntry);
    }
}

/**
 * operator()
 *   Called to process instance command for a person instance.
 *   See the header for the list and syntaxes of the subcommands.
 * @param interp - interpreter running the command.
 * @param objv   - the command words.
 */
int TclPersonInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    /* Processing is done in a try/catch block to simplify error handling */
    
    try {
        bindAll(interp, objv);
        requireAtLeast(
            objv, 2,
            "Person run instance commands require at least a sub-command"
        );
        std::string subcommand(objv[1]);
        if (subcommand == "destroy")  {
            delete this;
        } else if (subcommand == "lastName") {
            lastName(interp, objv);
        } else if (subcommand == "firstName") {
            firstName(interp, objv);    
        } else if (subcommand == "salutation") {
            salutation(interp, objv);
        } else if (subcommand == "id") {
            id(interp, objv);
        } else  {
            std::stringstream msg;
            msg << "'" << subcommand << "' is an invalid subcommand for "
                << std::string(objv[0]);
            std::string e = msg.str();
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
/**
 * getCommanObject [static]
 *    Lookup a command object by name.
 *  @param name - name of the command.
 *  @return TclPersonInstance* - pointer to the command.
 *  @throw std::out_of_range - if there's no mathcing entry.
 */
TclPersonInstance*
TclPersonInstance::getCommandObject(const std::string& name)
{
    auto p = m_personCommands.find(name);
    if (p == m_personCommands.end()) {
        std::stringstream msg;
        msg << name << " is not an existing person instance command";
        std::string e = msg.str();
        throw std::out_of_range(e);
    }
    return p->second;
}
////////////////////////////////////////////////////////////////////////////////
// Private subcommand methods

/**
 * lastName
 *   Set the result with the person's last name.
 * @param interp - interpreter executing the command.
 * @param objv   - command line words.
 */
void
TclPersonInstance::lastName(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <person-instance> lastName");
    interp.setResult(m_person->lastName());
}
/**
 * firstName
 *    set the result to the person's first name.
 *  @param interp - interpreter runnig the command
 *  @param objv   - The command words.
 */
void TclPersonInstance::firstName(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <person-instance> firstName");
    interp.setResult(m_person->firstName());
}
/**
 * salutation
 *    Set the result to the person's salutation.
 * @param interp  - interpreter running the command.
 * @param objv    - the command words.
 */
void
TclPersonInstance::salutation(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <person-instance> salutation");
    interp.setResult(m_person->salutation());
}
/**
 * id
 *   Returns the person's primary key in the root table describing people
 *    in the database.
 * @param interp - interpreter running the command.
 * @param objv   - the command words.
 */
void
TclPersonInstance::id(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "USage: <person-instance> id");
    CTCLObject result;
    result.Bind(interp);
    result = m_person->id();
    interp.setResult(result);
}