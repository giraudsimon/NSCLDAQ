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

/** @file:  TclLogbook.cpp
 *  @brief: Implements the TclLogbook command processor.
 */
#include "TclLogbook.h"
#include "LogBook.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>
#include <Exception.h>
#include <sstream>

#include "TclLogBookInstance.h"

int TclLogbook::m_instanceCounter(0);
/**
 * constructor
 *    Create and register the command as per the caller:
 *
 *  @param interp - interpreter on which the command is registered.
 *  @param name   - Full path name of the command e.g. logbook::logbook.
 */
TclLogbook::TclLogbook(CTCLInterpreter* pInterp, const char* pCommand) :
    CTCLObjectProcessor(*pInterp, pCommand, true)
{}
/**
 * destructor
 *    Currently nothing.
 */
TclLogbook::~TclLogbook()
{
}

/**
 * operator()
 *     Process the command:
 *     - Wrap processing in a try catch block.
 *     - bind all the commando objects to the interpreter.
 *     - Ensure there's a subcommand.
 *     - Extract the subcommand and call the appropriate command processor.
 * @param interp  - interpreter running the command.
 * @param objv    - Reference to a vector of wrapped objects that are the command line
 *                  words.
 * @return int - status, TCL_OK for success, and TCL_ERROR on failure.
 */
int
TclLogbook::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, usage(objv, "Missing subcommand").c_str());
        std::string subcommand = objv[1];
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "open") {
            open(interp, objv);
        } else if (subcommand == "tempdir") {
            tempdir(interp, objv);
        } else {
            throw usage(objv, "Invalid subcommand");
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
            "Unexpected exception type caught in TclLogbook::operator()"
        );
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * create
 *    Create a new database.  The caller must supply in order:
 *    - Filename into which the schema will be created.
 *    - experiment - experiment identification
 *    - spokesperson - the experiment spokes person.
 *    - purpose   - The experiment purpose.  These are stored in the
 *                  database where they could be retrieved later.
 * @param interp - reference to the interpreter this command is running on.
 * @param objv   - reference to the command words.
 * @note all errors are reported via exceptions.
 */
void
TclLogbook::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(
        objv, 6,
        usage(
            objv,
            "create subcommand requires a file, experiment, spokesperson and purpose"
        ).c_str()
    );
    
    std::string filename = objv[2];
    std::string experiment = objv[3];
    std::string spokesperson = objv[4];
    std::string purpose = objv[5];
    
    LogBook::create(
        filename.c_str(), experiment.c_str(), spokesperson.c_str(),
        purpose.c_str()
    );
                                  
}
/**
 * open
 *    Creates a new logbook instance command from the logbook in the filename
 *    provided.
 * @param interp - Interpreter executing the command.
 * @param objv - Command words.
 */
void
TclLogbook::open(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(
        objv, 3, usage(objv, "open subcommand requires only a filename").c_str()
    );
    
    std::string filename = objv[2];
    std::string commandName = createObjectName("logbook");
    
    LogBook* pBook = new LogBook(filename.c_str());
    
    new TclLogBookInstance(&interp, commandName.c_str(), pBook);
    
    interp.setResult(commandName);
}
/**
 * tempdir
 *  Sets the result with the name of the logbook temporary directory.
 *
 * @param interp -interpreter running the command.
 * @param obvjv - command line word objects.
 */
void
TclLogbook::tempdir(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, usage(objv, "Too many command parameters").c_str());
    interp.setResult(LogBook::m_tempdir);
                                
}

////////////////////////////////////////////////////////////////////////////////
// Static methods:

/**
 * createObjectName
 *   Given a prefix and the value of m_instanceCounter, creates an application
 *   unique command name.
 * @param prefix - the command name prefix.
 * @return std::string - the new command name.
 */
std::string
TclLogbook::createObjectName(const char* prefix)
{
    std::stringstream strResult;
    strResult << "::logbook::" << prefix << '_' << m_instanceCounter++;
    std::string result = strResult.str();    // I've had failure returning str().
    return result;
}
/**
 * reconstructCommand
 *    Given an objv reconstructs the original command - after substitutions
 *    were performed that is
 * @param objv - vector of command word objects.
 * @note  Elements of objv must be bound to an interpreter.
 * @return std::string - reconstructed command.
 * @note - there will be a trailing space on the back end of the returned string.
 */
std::string
TclLogbook::reconstructCommand(std::vector<CTCLObject>& objv) {
    std::string result;
    for (int i = 0; i < objv.size(); i++) {
        result += std::string(objv[i]);
        result += " ";
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//  private methods.


/**
 * usage
 *    returns a usage like string.
 * @param objv - the command words.
 * @param msg  - the message
 * @return std::string a error message.
 */
std::string
TclLogbook::usage(std::vector<CTCLObject>& objv, const char* msg)
{
    std::string result("ERROR - commmand: ");
    result += reconstructCommand(objv);
    
    result += "\n";
    result += msg;
    
    return result;
}

