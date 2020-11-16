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

/** @file:  TclLogBookInstance.cpp
 *  @brief: implement logbook instance commands
 */

#include "TclLogBookInstance.h"
#include "TclLogbook.h"
#include "TclPersonInstance.h"
#include "TclShiftInstance.h"
#include "LogBook.h"
#include "LogBookPerson.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>

#include <string>
#include <sstream>


/**
 * constructor
 *    @param pInterp - pointer to the interpreter we are going to register on.
 *    @param cmd     - Command name string.
 *    @param pBook   - Logbook we're encapsulating.
 */
TclLogBookInstance::TclLogBookInstance(
    CTCLInterpreter* pInterp, const char* cmd, LogBook* pBook
) :
    CTCLObjectProcessor(*pInterp, cmd, true),
    m_logBook(pBook)
{
        
}
/**
 * destructor
 */
TclLogBookInstance::~TclLogBookInstance()
{}


/**
 * operator()
 *    Require a subcommand and dispatch it to the appropriate service method.
 *    Note we establish a try/catch block around the method and all service methods
 *    report errors via exception throws.  This dovetails nicely with the
 *    fact that the base API uses exceptions to report errors other than
 *    search failures.
 *
 *  @param interp -interpreter running the command.
 *  @param objv   - command words that make up the command.
 *  @return int   - TCL_OK on success, TCL_ERROR otherwise.
 *    
 */
int
TclLogBookInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "A a Subcommand is required for this command");
        
        std::string subcommand = objv[1];
        
        if (subcommand == "destroy") {
            delete this;
        } else if (subcommand == "addPerson") {
            addPerson(interp, objv);
        } else if (subcommand == "findPeople") {
            findPeople(interp, objv);
        } else if (subcommand == "listPeople") {
            listPeople(interp, objv);
        } else if (subcommand == "getPerson" ) {
            getPerson(interp, objv);
        } else if (subcommand == "createShift") {
            createShift(interp, objv);
        } else {
            std::stringstream msg;
            msg << "Invalid subcommand for " << std::string(objv[0]) << " : "
                << subcommand;
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
            "Unexpected exception type caught in TclLogbookInstance::operator()"
        );
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
///////////////////////////////////////////////////////////////////////////////
// Subcommand executor methods.

/////////////////////////// API for people ///////////////////////////////////
/**
 * addPerson
 *   Adds a new person to the logbook.  See the .h file for the syntax
 *   required:
 *   -  Ensure we have exactly the right number of parameters.
 *   -  pull out the bits and pieces.
 *   -  Create a new logbook person.
 *   -  Wrap that new logbook person in a new command
 *   -  Return the new command name as the result.
 *
 * @param interp - the interpreter on which the command is running.
 * @param objv   - The command words.
 */
void
TclLogBookInstance::addPerson(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    std::string usage=
        "Usage: <cmdname> addPerson <lastname> <firstname> ?<salutation>?";
    requireAtLeast(objv, 4, usage.c_str());
    requireAtMost(objv, 5, usage.c_str());
    
    std::string salutation;                         // Defaults to empty.
    std::string lastName = objv[2];
    std::string firstName = objv[3];
    if (objv.size() == 5) salutation = std::string(objv[4]);
    
    // Create the actual record:
    
    LogBookPerson* pPerson = m_logBook->addPerson(
        lastName.c_str(), firstName.c_str(), salutation.c_str()
    );
    
    // Wrap a new TclPersonInstance in the person object.
    
    std::string newCommand = wrapPerson(interp, pPerson);
    
    interp.setResult(newCommand);
}
/**
 * findPeople
 *    Return a (possibly empty) list of commands that wrap people
 *    in the database that satisfy an optional where clause.
 *    e.g. <instance> findPeople {lastname = 'Fox'}
 *    - must be at most three command words.
 *    - call the logbook::findPeople method.
 *    - wrap all the results in commands that are inserted into the
 *      list set as the result.
 *      
 *  @param interp - interpreter on which the command is executing.
 *  @param objv  - The command words.
 */
void
TclLogBookInstance::findPeople(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3, "Usage: <logbookinstance> findPeople ?where-clause?");
    const char* whereCstring(nullptr);
    std::string whereClause;
    if (objv.size() == 3) {
        whereClause = std::string(objv[2]);
        whereCstring = whereClause.c_str();
    }
    
    auto people = m_logBook->findPeople(whereCstring);
    CTCLObject result;
    result.Bind(interp);
    for (int i =0; i < people.size(); i++) {
        CTCLObject item;
        item.Bind(interp);
        std::string wrappedPerson = wrapPerson(interp, people[i]);
        item = std::string(wrappedPerson);
        result += item;
    }
    interp.setResult(result);
}
/**
 * listPeople
 *   This is essentially findPeople with an enforced lack of a WHERE clause
 *
 * @param interp - intepreter on which we're running.
 * @param objv   - command words.
 */
void
TclLogBookInstance::listPeople(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <logbookinstance> listPeople");
    findPeople(interp, objv);
}
/**
 * getPerson
 *    Retrieve a person object by id (database primary key).
 *
 * @param interp - interpreter on which the command is executing.
 * @param objv   - command words.
 */
void
TclLogBookInstance::getPerson(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logbookinstance> getPerson id");
    int id(objv[2]);
    LogBookPerson* pPerson = m_logBook->getPerson(id);
    std::string newCommand = wrapPerson(interp, pPerson);
    interp.setResult(newCommand);
}
//////////////////////////// API For shifts ///////////////////////////////////

/**
 * createShift
 *     Create a new shift and wrap it in a shift instance command.
 *     The shift requires one or two additional parameters:
 *     -  The first, mandatory, parameter is the name of the new shift.
 *     -  The second, optional, parameter is a list of the names of person
 *         command instances that are in the shift.
 *     -  Any person command names are converted to LogBookPerson instance pointers.
 *     -  The logbook shift is creasted.
 *     -  It is then wrapped in the shift command.
 *     -  The name of the shift command is returned as the result.
 * @param interp - interpreter running the command.
 * @param objv   - the command words.
 */
void
TclLogBookInstance::createShift(
    CTCLInterpreter&interp, std::vector<CTCLObject>& objv
)
{
    std::string usage = "Usage: <logbook instance> createShift name ?people?";
    std::string name;
    std::vector<LogBookPerson*> people;
    requireAtLeast(objv, 3, usage.c_str());
    requireAtMost(objv, 4, usage.c_str());
    
    name = std::string(objv[2]);
    if (objv.size() == 4) {
        // Marshall the names -> personCommands.
        // If there's a failure then an exception will get thrown.
        
        int peopleCount = objv[3].llength();
        for (int i =0; i < peopleCount; i++) {
            CTCLObject o = objv[3].lindex(i);
            o.Bind(interp);
            std::string cmdName = std::string(o);
            people.push_back(
                (TclPersonInstance::getCommandObject(cmdName))->getPerson()
            );
        }
        
    }
    // Create the shift and wrap it:
    
    LogBookShift* pShift = m_logBook->createShift(name.c_str(), people);
    std::string result = wrapShift(interp, pShift);
    
    
    interp.setResult(result);
    
}
///////////////////////////////////////////////////////////////////////////////
// Private utilities:

/**
 * wrapPerson
 *    Wrap a LogBookPerson instance in a TclPersonInstance.
 *  @param interp -interpreter on which to register the command.
 *  @param pPerson - pointer to the person.
 *  @return std::string -new command name.
 */
std::string
TclLogBookInstance::wrapPerson(CTCLInterpreter& interp, LogBookPerson* pPerson)
{
    std::string newCommand = TclLogbook::createObjectName("person");
    new TclPersonInstance(interp, newCommand.c_str(), pPerson);
    
    return newCommand;
}
/**
 * wrapShift
 *    Take a shift object and wrap it in a TclShiftInstance command
 * @param interp   - interpreter on which the shift command is registered.
 * @param pShift   - Pointer to the shift to wrap.
 * @return std::string -name of the shift command.
 */
std::string
TclLogBookInstance::wrapShift(CTCLInterpreter& interp, LogBookShift* pShift)
{
    std::string newCommand = TclLogbook::createObjectName("shift");
    new TclShiftInstance(interp, newCommand.c_str(), pShift);
    return newCommand;
}