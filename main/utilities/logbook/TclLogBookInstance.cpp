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
    
    std::string newCommand = TclLogbook::createObjectName("person");
    
    // TOD: Create TclLogBookPersonIntance....
    
    interp.setResult(newCommand);
}