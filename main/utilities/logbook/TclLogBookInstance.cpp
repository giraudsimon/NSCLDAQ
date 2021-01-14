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
#include "TclRunInstance.h"
#include "TclNoteInstance.h"
#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "LogBookRun.h"
#include "LogBookNote.h"
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
 * constructor
 *    BUt with the logbook already in a shared ptr:
 */
TclLogBookInstance::TclLogBookInstance(
    CTCLInterpreter* pInterp, const char* cmd, std::shared_ptr<LogBook> book
) :
    CTCLObjectProcessor(*pInterp, cmd, true),
    m_logBook(book)
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
        } else if (subcommand == "getShift") {
            getShift(interp, objv);
        } else if (subcommand == "addShiftMember") {
            addShiftMember(interp, objv);
        } else if (subcommand == "removeShiftMember") {
            removeShiftMember(interp, objv);
        } else if (subcommand == "listShifts") {
            listShifts(interp, objv);
        } else if (subcommand == "findShift") {
            findShift(interp, objv);
        } else if (subcommand  == "setCurrentShift") {
            setCurrentShift(interp, objv);
        } else if (subcommand == "getCurrentShift") {
            getCurrentShift(interp, objv);
        } else if (subcommand == "begin") {
            
            beginRun(interp, objv);
        } else if (subcommand == "end") {
            endRun(interp, objv);    
        } else if (subcommand == "pause") {
            pauseRun(interp, objv);
        } else if (subcommand == "resume") {
            resumeRun(interp, objv);
        } else if (subcommand == "emergencyStop") {
            emergencyEnd(interp, objv);
        } else if (subcommand == "listRuns") {
            listRuns(interp, objv);
        } else if (subcommand == "findRun") {
            findRun(interp, objv);
        } else if (subcommand == "runId") {
            runId(interp, objv);
        } else if (subcommand == "currentRun") {
            currentRun(interp, objv);
            
        } else if (subcommand == "createNote") {
            createNote(interp, objv);
        } else if (subcommand == "getNote") {
            getNote(interp, objv);
        } else if (subcommand == "listAllNotes") {
            listAllNotes(interp, objv);
        } else if (subcommand == "listNotesForRunId") {
            listNotesForRunId(interp, objv);
        } else if (subcommand == "listNotesForRunNumber") {
            listNotesForRunNumber(interp, objv);
        } else if (subcommand == "listNotesForRun") {
            listNotesForRun(interp, objv);
        } else if (subcommand == "listNonRunNotes") {
            listNonRunNotes(interp, objv);
        } else if (subcommand == "getNoteRun") {
            getNoteRun(interp, objv);
        } else if (subcommand == "getNoteAuthor") {
            getNoteAuthor(interp, objv);
        } else if (subcommand == "kvExists") {
            kvExists(interp, objv);
        } else if (subcommand == "kvGet") {
            kvGet(interp, objv);
        } else if (subcommand == "kvSet") {
            kvSet(interp, objv);
        } else if (subcommand == "kvCreate") {
            kvCreate(interp, objv);
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
    for (auto p : people) {
        CTCLObject item;
        item.Bind(interp);
        std::string wrappedPerson = wrapPerson(interp, p);
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
            LogBookPerson* pPerson =
                TclPersonInstance::getCommandObject(cmdName)->getPerson();
            // Need to push a duplicate as remove/destroy deletes:
            people.push_back(
                m_logBook->getPerson(pPerson->id())
            );
        }
        
    }
    // Create the shift and wrap it:
    
    LogBookShift* pShift = m_logBook->createShift(name.c_str(), people);
    std::string result = wrapShift(interp, pShift);
    
    
    interp.setResult(result);
    
}
/**
 * getShift
 *    Returns a command object that encapsulates the shift whose
 *    id is provided on the command line.
 * @param interp  - interpreter running the command.
 * @param objv    - The command words.
 */
void
TclLogBookInstance::getShift(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logbookinstance> getShift shift-id");
    int id = objv[2];
    LogBookShift* pShift = m_logBook->getShift(id);
    interp.setResult(wrapShift(interp, pShift));
}
/**
 * addShiftMember
 *    Adds a new person to an existing shift that's been wrapped.
 * @param interp -interpreter the command is running on.
 * @param objv   - The command words.
 */
void
TclLogBookInstance::addShiftMember(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 4,
        "Usage: <logbook-instance> addShiftMember shift-command person-command"
    );
    std::string shiftCmd(objv[2]);
    std::string personCmd(objv[3]);
    
    // Get the shift and person objects - note that these calls will throw
    // if there are errors:
    
    LogBookShift* pShift =
        (TclShiftInstance::getCommandObject(shiftCmd))->getShift();
    LogBookPerson* pPerson =
        (TclPersonInstance::getCommandObject(personCmd))->getPerson();
    // need to add a copy of the instance as removing/destroying will
    // destroy the person
    
    pPerson = m_logBook->getPerson(pPerson->id());
    m_logBook->addShiftMember(pShift, pPerson);
    
    interp.setResult(shiftCmd);          // To make it look like the LogBook class.
}
/**
 * removeShiftMember
 *     Remove a member from a shift. The required parameters are a shift
 *     command and a person command representing the person to remove
 *     from the shift.
 * @param interp - interpreter the command is runing under.
 * @param objv   - command words.
 */
void
TclLogBookInstance::removeShiftMember(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 4,
        "Usage: <logbook-instance> removeShiftMember shift-command person-command"
    );
    
    std::string shiftCmd(objv[2]);
    std::string personCmd(objv[3]);
    
    LogBookShift* pShift =
        (TclShiftInstance::getCommandObject(shiftCmd))->getShift();
    LogBookPerson* pPerson =
        (TclPersonInstance::getCommandObject(personCmd))->getPerson();
    
    m_logBook->removeShiftMember(pShift, pPerson);
    
    interp.setResult(shiftCmd);
}
/**
 * listShifts
 *    Return a (possibly empty) list of wrapped shifts that list all of the
 *    shifts in the database.
 * @param interp - interpreter the command is running under.
 * @param objv   - vector of command words that make up the command.
 */
void
TclLogBookInstance::listShifts(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <logbook-instance> listShifts");
    auto rawShifts = m_logBook->listShifts();
    
    CTCLObject result;
    result.Bind(interp);
    for (auto p : rawShifts) {
        CTCLObject element;
        element.Bind(interp);
        std::string shiftCommand = wrapShift(interp, p);
        
        element = std::string(shiftCommand);
        result += element;
    }
    
    interp.setResult(result);
}
/**
 * findShift
 *    Return an encapsulated shift given the shift name.
 *    If {} is returned, then there's no matching shift.
 * @param interp - interpreter running the command.
 * @param objv   - Vector of command words.
 */
void
TclLogBookInstance::findShift(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> findShift <shiftname>");
    std::string shiftName(objv[2]);
    auto pShift = m_logBook->findShift(shiftName.c_str());
    std::string result("");
    if (pShift) {
        result = wrapShift(interp, pShift);
    }
    interp.setResult(result);
   
}
/**
 * setCurrentShift
 *    Set the current shift.
 *  @param interp -interpreter that's running the command.
 *  @param objv   - Command words
 */
void
TclLogBookInstance::setCurrentShift(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "Usage : <logbook-instance> setCurrentShift <shift-name>"
    );
    
    std::string name(objv[2]);
    auto pShift = m_logBook->findShift(name.c_str());
    if (!pShift) {
        std::stringstream msg;
        msg << "setCurrentShift error the shift '" <<  name
            << "' does not exist";
        std::string e = msg.str();
        throw std::invalid_argument(e);
    }
    m_logBook->setCurrentShift(pShift);
    delete pShift;
}
/**
 * getCurrentShift
 *    Returns the command wrapped current shift.
 *    This will be an empty string if there is no current shift.
 * @param interp - interpreter on which the command is running.
 * @param objv   - Command words.
 */
void
TclLogBookInstance::getCurrentShift(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage : <logbook-instancde> getCurrentShift");
    
    auto pShift = m_logBook->getCurrentShift();
    std::string result;
    if (pShift) {
        result = wrapShift(interp, pShift);
    }
    interp.setResult(result);
}
////////////////////////////////////////////////////////////////////////////////
// Run API

/**
 * beginRun
 *    Begin a new run.  This creates a new run object and a wrapping command
 *    object. The name of the wrapping command object is returned.
 * @param interp - interpreter on which the command is running.
 * @param objv   - the command words.
 */
void
TclLogBookInstance::beginRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    // We need a run number a title and an optional note.
    
    const char* Usage =
     "Usage: <logbook-instance> begin number title ?remark?";
    requireAtLeast(objv, 4, Usage);
    requireAtMost(objv, 5, Usage);
    
    int run(objv[2]);
    std::string title(objv[3]);
    const char* remark(nullptr);
    std::string remarkString;
    if (objv.size() == 5) {
        remarkString = std::string(objv[4]);
        remark = remarkString.c_str();
    }
    // Read to make and wrap the new run object (unless there are objectsions
    // from the underlying API):
    
    LogBookRun* pRun = m_logBook->begin(run, title.c_str(), remark);
    std::string result = wrapRun(interp, pRun);
    interp.setResult(result);
}
/**
 * endRun
 *    Ends an existing run given its command and an optional remark.
 * @param interp - interpreter running the command.
 * @param objv   - Command words.
 */
void
TclLogBookInstance::endRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    const char* usage = "Usage: <logbookinstance> end <run-command> ?remark?";
    requireAtLeast(objv, 3, usage);
    requireAtMost(objv, 4, usage);
    
    std::string run(objv[2]);
    const char* remark(nullptr);
    std::string remarkString;
    if (objv.size() == 4) {
        remarkString= std::string(objv[3]);
        remark  = remarkString.c_str();
    }
    // Get the run object:
    
    TclRunInstance *pInstance = TclRunInstance::getCommandObject(run);
    LogBookRun* pRun = pInstance->getRun();
    m_logBook->end(pRun, remark);
    
    // THe run object has been replaced; replace the encapsulated object.
    
    pInstance->setRun(pRun);
    
    // Return the run instance command as well.
    
    interp.setResult(run);
}
/**
 * pauseRun
 *   Pause the specified run.
 * @param interp -interpreter running the command.
 * @param objv   - the command words.
 */
void
TclLogBookInstance::pauseRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    const char* usage = "Usage: <logbookinstance> pause <run-command> ?remark?";
    requireAtLeast(objv, 3, usage);
    requireAtMost(objv, 4, usage);
    
    std::string run(objv[2]);
    const char* remark(nullptr);
    std::string remarkString;
    if (objv.size() == 4) {
        remarkString= std::string(objv[3]);
        remark  = remarkString.c_str();
    }
    // Get the run object:
    
    TclRunInstance *pInstance = TclRunInstance::getCommandObject(run);
    LogBookRun* pRun = pInstance->getRun();
    m_logBook->pause(pRun, remark);
    
    pInstance->setRun(pRun);
    interp.setResult(run);
}
/**
 * resumeRun
 *    Resume the named run.
 *
 *  @param interp  - interpreter running the command.
 *  @param objv    - The command words.
 */
void
TclLogBookInstance::resumeRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    const char* usage = "Usage: <logbookinstance> resume <run-command> ?remark?";
    requireAtLeast(objv, 3, usage);
    requireAtMost(objv, 4, usage);
    
    std::string run(objv[2]);
    const char* remark(nullptr);
    std::string remarkString;
    if (objv.size() == 4) {
        remarkString= std::string(objv[3]);
        remark  = remarkString.c_str();
    }
    // Get the run object:
    
    TclRunInstance *pInstance = TclRunInstance::getCommandObject(run);
    LogBookRun* pRun = pInstance->getRun();
    m_logBook->resume(pRun, remark);
    
    pInstance->setRun(pRun);
    interp.setResult(run);
}
/**
 * emergencyEnd
 *    Logs an emergency stop on a run.
 * @param interp  - interpreter we're running on
 * @param objv    - command words.
 */
void
TclLogBookInstance::emergencyEnd(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    const char* usage = "Usage: <logbookinstance> emergencyStop <run-command> ?remark?";
    requireAtLeast(objv, 3, usage);
    requireAtMost(objv, 4, usage);
    
    std::string run(objv[2]);
    const char* remark(nullptr);
    std::string remarkString;
    if (objv.size() == 4) {
        remarkString= std::string(objv[3]);
        remark  = remarkString.c_str();
    }
    // Get the run object:
    
    TclRunInstance *pInstance = TclRunInstance::getCommandObject(run);
    LogBookRun* pRun = pInstance->getRun();
    m_logBook->emergencyStop(pRun, remark);
    
    pInstance->setRun(pRun);
    interp.setResult(run);
}
/**
 * listRuns
 *    Return a list (possibily empty) of all of the runs in the run database.
 *    The runs are returned as encapsulating commands.
 * @param interp - interpreter running the command.
 * @param objv   - the command words.
 */
void
TclLogBookInstance::listRuns(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <logbook-instance> listRuns");
    
    auto rawRuns = m_logBook->listRuns();
    CTCLObject result;
    result.Bind(interp);
    
    for(auto run : rawRuns) {
        CTCLObject item;
        item.Bind(interp);
        item = std::string(wrapRun(interp, run));
        
        result += item;
    }
    
    interp.setResult(result);
}
/**
 * findRun
 *   Locate a  run by run  number and wrap it.  If there's no matching run,
 *   "" is returned.
 *
 * @param interp -  interpreter running the command.
 * @param objv   - The words of the command.
 * */
void
TclLogBookInstance::findRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> findRun <run-number>");
    int runNumber = objv[2];
    
    LogBookRun* pRun = m_logBook->findRun(runNumber);
    std::string result;
    if (pRun) {
        result = wrapRun(interp, pRun);
    }
    
    interp.setResult(result);
}
/**
 * runId
 *   Return the id (primary key) of a run given it's command.
 * @param interp - interpreter on which the command is running.
 * @param objv   - command words.
 */
void
TclLogBookInstance::runId(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> runId <runcommand>");
    std::string runCommand(objv[2]);
    
    LogBookRun* pRun = TclRunInstance::getCommandObject(runCommand)->getRun();
    CTCLObject result;
    result.Bind(interp);
    result = pRun->getRunInfo().s_id;
    interp.setResult(result);
}
/**
 *  currentRun
 *     Returns a command ensemble encapsulated run for the current run.
 *     "" is returned if there is no current run.  There is a current run
 *     iff there's a run that is not ended (in progress).  That is the current
 *     run is established by beginning a run and removed by ending or emergencyStop
 *     ing it.
 *  @param interp - interpreter on which the command is executing.
 *  @param objv   - Command words.
 */
void
TclLogBookInstance::currentRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv , 2, "Usage: <logbook-instance> currentRun");
    LogBookRun* pRun = m_logBook->currentRun();
    std::string result;
    if (pRun) {
        result = wrapRun(interp, pRun);
    }
    interp.setResult(result);
}
///////////////////////////////////////////////////////////////////////////////
// Notes API

/**
 * createNote
 *    Create a new note with optional image lists, and associated run.
 * @param interp - interpreter runnig the command.
 * @param objv   - Command words.
 */ 
void
TclLogBookInstance::createNote(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    // Command processing is a bit complicated.  There are
    // four possible casees:
    //  *  4 parameters:  author, text no images, no associated run.
    //  *  5 parameters:  author, text, associated run command.
    //  *  6 parameters   author, text, image list and image offset list no
    //                    associated run command.
    //  * 7 parameter, author, texst, image list, image offset list, associated run command.
    //
    // Furthermore, if there is an associated image list, the length of that
    // list must be the same as the associated offset list.
    //
    const char* usage =
        "Usage: <logbook-instance> createNote author text ?images imageoffsets? ?run?";
    requireAtLeast(objv, 4, usage);
    requireAtMost(objv, 7, usage);
    // Direct parameters -- sort of.
    
    std::string authorCmd;
    std::string note;
    std::vector<std::string> imageFiles;
    std::vector<size_t>      imageOffsets;
    std::string runCmd;
    
    // Parameters we derive:
    
    LogBookPerson* pPerson(nullptr);
    LogBookRun*    pRun(nullptr);
    std::pair<std::vector<std::string>, std::vector<size_t>> imageInfo;
    
    // The author command and note are in 'standard' spots.
    // Convert the author into a person (could throw).
    
    authorCmd = std::string(objv[2]);
    note      = std::string(objv[3]);
    pPerson =
        TclPersonInstance::getCommandObject(authorCmd)->getPerson();
    
    // What we do next depends on the command word count as described above:
    
    switch (objv.size()) {
    case 5:                           // There's just an associated run:
        runCmd = std::string(objv[4]);
        pRun   = TclRunInstance::getCommandObject(runCmd)->getRun();
        break;
    case 7:                       // image information and run
        runCmd    = std::string(objv[6]);
        pRun     = TclRunInstance::getCommandObject(runCmd)->getRun();
    case 6:                        // there's image information but no run:
        imageInfo = getImageInformation(interp, objv[4], objv[5]);
    
        break;
    default:                        // four parameters already dealt with.
        break;                      // other counts have thrown by now.
    }
    imageFiles   = imageInfo.first;               // could be emtpy.
    imageOffsets = imageInfo.second;              // already checked for same size.
    
    // Create the note and wrap it before returning the new command name:
    
    LogBookNote* pNote = m_logBook->createNote(
        *pPerson, note.c_str(), imageFiles, imageOffsets, pRun
    );
    std::string noteCmd = wrapNote(interp, pNote);
    interp.setResult(noteCmd);
}
/**
 * getNote
 *    Given a note id, wraps that note in a command object. Error if there's
 *    no matching id.
 * @param interp - interpreter running the command.
 * @param objv   - Command words.
 */
void
TclLogBookInstance::getNote(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logboook-instance> getNote <noteid>");
    int id(objv[2]);
    
    LogBookNote* pNote = m_logBook->getNote(id);   // Throws if no match.
    interp.setResult(wrapNote(interp, pNote));
}
/**
 * listAllNotes
 *    Returns a (possibly empty) list containing command encapsulations
 *    of all notes currently in the logbook.
 * @parma interp - interpreter running the command.
 * @param objv   - the command words.
 */
void
TclLogBookInstance::listAllNotes(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "Usage: <logbook-instance> listAllNotes");
    std::vector<LogBookNote*> notes = m_logBook->listAllNotes();
    
    notesVectorToResultList(interp, notes);
}
/**
 * listNotesForRunId
 *
 *     Makes a command encapsulated list for all notes for the run identified
 *     by primary key.
 *  @param interp -interpreter running the command.
 *  @param objv   -Command words.
 */
void
TclLogBookInstance::listNotesForRunId(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> listNotesForRunId <id>");
    
    int id = (objv[2]);
    
    auto notes = m_logBook->listNotesForRunId(id);
    
    notesVectorToResultList(interp, notes);
}
/**
 * listNotesForRunNumber
 *   List all of the notes that were entered for a specific run number.
 *
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 */
void
TclLogBookInstance::listNotesForRunNumber(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "Usage: <logbook-instance> listNotesForRunNumber <runnum>"
    );
    int runNumber(objv[2]);
    
    // If this throws, return an empty list because it means the run could
    // not be found:
    
    try {
        auto notes = m_logBook->listNotesForRun(runNumber);
        notesVectorToResultList(interp, notes);
    } catch (...) {
        interp.setResult("");
    }
}
/**
 * listNotesForRun
 *   Tcl-ish binding  given a run instance command, lists the runs
 *   associated with it.
 *
 * @param interp - interpreer running the command.
 * @param objv   - Command words.
 */
void
TclLogBookInstance::listNotesForRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "Usage: <logbook-instancea> listNotesForRun <run-instance>"
    );
    std::string runCmd(objv[2]);
    
    LogBookRun* pRun = TclRunInstance::getCommandObject(runCmd)->getRun();
    
    auto notes = m_logBook->listNotesForRunId(pRun->getRunInfo().s_id);
    notesVectorToResultList(interp, notes);
}
/**
 * listNonRunNotes
 *    Create a list of the notes that are not associated with any run.
 *  @param interp - interpreter the command is running on.
 *  @param objv   - COmmand words.
 */
void
TclLogBookInstance::listNonRunNotes(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv,2, "Usage: <logbook-instance> listNonRunNotes");
    auto notes = m_logBook->listNonRunNotes();
    notesVectorToResultList(interp, notes);
}
/**
 * getNoteRun
 *    If a note is associated with a run, this method sets the result to
 *     Tcl encapsulated run instance for the associated run.  If not,
 *     an empty string is returned.
 * @param interp - interpreter running the command.
 * @param objv   - command line words.
 */
void
TclLogBookInstance::getNoteRun(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> getNoteRun <noteinstance>");
    std::string noteCmd(objv[2]);
    LogBookNote* pNote  = TclNoteInstance::getCommandObject(noteCmd)->getNote();
    auto pRun = m_logBook->getNoteRun(*pNote);
    if (pRun) {
        interp.setResult(wrapRun(interp, pRun));
    } else {
        interp.setResult("");
    }
}
/**
 * getNoteAuthor
 *    Set the result with a Tcl encapsulated command ensemble that respresents
 *    the author of a note.
 *
 * @param interp - interpreter running this command.
 * @param objv   - Command line words.
 */
void
TclLogBookInstance::getNoteAuthor(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "Usage: <logbook-instance> getNoteAuthor <note-instance>"
    );
    std::string noteCmd(objv[2]);
    LogBookNote* pNote = TclNoteInstance::getCommandObject(noteCmd)->getNote();
    LogBookPerson* pPerson = m_logBook->getNoteAuthor(*pNote);
    
    interp.setResult(wrapPerson(interp, pPerson));
}
///////////////////////////////////////////////////////////////////////////////
// Key value store API:

/**
 * kvExists
 *    Returns a true result if the specified key exists.
 * @param interp - interpreter reference that running the command.
 * @param obvj   - Command line parameters.
 */
void
TclLogBookInstance::kvExists(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> kvExists key");
    std::string key(objv[2]);
    interp.setResult(m_logBook->kvExists(key.c_str()) ? "1" : "0");
}
/**
 * kvGet
 *   Returns the value of a key in the key value store.
 * @param interp - interpreter running the command.
 * @param objv   - command keyword.
 */
void
TclLogBookInstance::kvGet(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Usage: <logbook-instance> kvGet key");
    std::string key(objv[2]);
    
    interp.setResult(m_logBook->kvGet(key.c_str()));

}
/**
 * kvSet
 *    Set the value of a key.  If the key exists, it will be given a different
 *    value. Otherwise a new key is created.
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 */
void
TclLogBookInstance::kvSet(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4, "Usage: <logbook-instance> kvSet key value");
    std::string key(objv[2]);
    std::string value(objv[3]);
    
    m_logBook->kvSet(key.c_str(), value.c_str());
}
/**
 * kvCreate
 *    Create a new key value pair -- it's an error to use an existing key.
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 */
void
TclLogBookInstance::kvCreate(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4, "Usage: <logbook-instance> kvCreate key value");
    std::string key(objv[2]);
    std::string value(objv[3]);
    
    m_logBook->kvCreate(key.c_str(), value.c_str());
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
    new TclShiftInstance(interp, newCommand.c_str(), pShift, m_logBook);
    return newCommand;
}
/**
 * wrapRun
 *   Wrap a run object in a tcl command ensemble.
 * @param interp - interpreter the command gets registered on.
 * @param pRun   - The run object to wrap.
 * @return std::string - the new command name.
 */
std::string
TclLogBookInstance::wrapRun(CTCLInterpreter& interp, LogBookRun* pRun)
{
    std::string newCommand = TclLogbook::createObjectName("run");
    new TclRunInstance(interp, newCommand.c_str(), pRun, m_logBook);
    return newCommand;
}
/**
 * wrapNote
 *   Wrap a note object in a new command.
 * @param interp - interpreter running the commannd that needs wrapping
 * @param note   - pointer to the note.
 * @return std::string - new command name.
 */
std::string
TclLogBookInstance::wrapNote(CTCLInterpreter& interp, LogBookNote* pNote)
{
    std::string newCommand = TclLogbook::createObjectName("note");
    new TclNoteInstance(interp, newCommand.c_str(), m_logBook, pNote);
    return newCommand;
}
/**
 * getImageInformation.
 *    Given a TclObj reference to the image list and offsets list,
 *    marshall those into a vector each:
 *    -  We don't check for the existence of files, that's done by the
 *       note creation methods.
 *    -  We don't check that the images and offsets vectors have the same
 *       length.  That too is done by noe creation.
 *  @param interp - interpreter on which the command is running.
 *  @param images - Tcl Object containing a list of images.
 *  @param offsets - Tcl Object containing a list of image offsets.
 *  @return std::pair<std::vector<std::string>, std::vector<size_t>>
 *              - .first is the vector of image names and .second the vector of
 *                offsets.
 */
std::pair<std::vector<std::string>, std::vector<size_t>>
TclLogBookInstance::getImageInformation(
    CTCLInterpreter& interp, CTCLObject& images, CTCLObject& offsets
)
{
    std::pair<std::vector<std::string>, std::vector<size_t>> result;
    std::vector<std::string> filenames;
    std::vector<size_t>      offsetsV;
    
    // Extract the filenames:
    
    for (int i = 0; i < images.llength(); i++) {
        CTCLObject name;
        name.Bind(interp);
        name =  images.lindex(i);
        std::string filename = std::string(name);
        filenames.push_back(filename);
    }
    // Extract the offsets:
    
    for (int i = 0; i < offsets.llength(); i++) {
        CTCLObject off;
        off.Bind(interp);
        off =  offsets.lindex(i);
        int offset = int(off);
        offsetsV.push_back(offset);
    }
    
    result.first = filenames;
    result.second = offsetsV;
    return result;
}
/**
 * notesVectorToResultList
 *   Converts a vector of notes into a list of note instance commands
 *   which are set as the interpreter result.
 * @param interp - the interpreter whose result is set.
 * @param notes - vector of note instance pointers.
 */
void TclLogBookInstance::notesVectorToResultList(
    CTCLInterpreter& interp, const std::vector<LogBookNote*>& pNotes
)
{
    CTCLObject result;
    result.Bind(interp);
    for(auto n : pNotes) {
        CTCLObject item;
        item.Bind(interp);
        item = std::string(wrapNote(interp, n));
        
        result += item;
    }
    interp.setResult(result);    
}