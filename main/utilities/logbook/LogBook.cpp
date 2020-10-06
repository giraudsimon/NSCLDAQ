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

/** @file:  LogBook.cpp
 *  @brief: Implement the logBook Class.
 */
#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "LogBookRun.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <CSqliteException.h>
#include <sqlite3.h>
#include <sstream>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <vector>

static const char* DBVersion="1.0";

////////////////////////////////////////////////////////////////
// Implement LogBook::Exception

/**
 * Exception
 *    @param message - the entirety of the error messag.
 */
LogBook::Exception::Exception(const char* message) noexcept :
    m_message(message)
{}
LogBook::Exception::Exception(const std::string& message) noexcept :
    m_message(message)
{}

/**
 * Exception
 *   @param reason  - Sqlite exception we want to rethrow.
 *   @param doing   - String that provide context for the Sqlite error
 */
LogBook::Exception::Exception(const CSqliteException& e, const char* doing) noexcept
{
    std::stringstream errorMessage;
    errorMessage << "CSqlite Exception caught while "
        << doing << " : " << e.what();
    m_message = errorMessage.str();
}
LogBook::Exception::Exception(const CSqliteException& e, const std::string& doing) noexcept :
    Exception(e, doing.c_str())
{
    
}
/**
 * copy constructor:
 *  @param rhs - what we're constructing from:
 */
LogBook::Exception::Exception(const Exception& rhs) noexcept :
    m_message(rhs.m_message)
{}
/**
 * assignment:
 *   @param rhs - what we're assigning from.
 *   @return LogBook::Exception& *this.
 */
LogBook::Exception&
LogBook::Exception::operator=(const Exception& rhs) noexcept
{
    if (this != &rhs) {
        m_message = rhs.m_message;
    }
    return *this;
}
/**
 * destrutor
 */
LogBook::Exception::~Exception() {}

/**
 * what
 *    Return the exception string.
 * @return const char* - exception string.
 */
const char*
LogBook::Exception::what() const noexcept
{
    return m_message.c_str();
}

// Utilities:

/**
 * rethrowSqliteException
 *   Given an SqliteException we're presumably handling,
 *   construct a new LogBook::Exception and throw it instead.
 * @param e - CSqliteException being handled.
 * @param pDoing - context string
 */
void
LogBook::Exception::rethrowSqliteException(
    CSqliteException& e, const char* pDoing
)
{
    LogBook::Exception rethrow(e, pDoing);
    throw rethrow;
}
void
LogBook::Exception::rethrowSqliteException(
    CSqliteException& e, std::string doing
)
{
    rethrowSqliteException(e, doing.c_str());
}
/////////////////////////////////////////////////////////
// Logbook class implementation

/**
 * m_tempdir is the location where temporary
 *           files will be written.
 */

std::string LogBook::m_tempdir = LogBook::computeTempDir();




/**
 * create
 *    Create a new logbook:
 *
 *
 * @param pFilename - Name of the new logbook file. Must not
 *                    exist.
 * @param pExperiment - Experiment identifier (e.g. "e17011").
 * @param pSpokesperson - Name of the spokesperson.
 * @param purpose    - Purpose of the experiment.
 * @throw LogBook::Exception if errors.
 */
void
LogBook::create(
    const char* pFilename, const char* pExperiment,
    const char* pSpokesperson, const char* purpose
)
{
    // Cannot exist:
    
    if (access(pFilename, F_OK) == 0) {
        std::stringstream msg;
        msg << "The file: " << pFilename << " already exists!";
        std::string m(msg.str());
        throw Exception(m);
    }
    try {
        CSqlite Connection(
            pFilename, CSqlite::readwrite | CSqlite::create
        );
        createSchema(Connection);
        initialize(Connection, pExperiment, pSpokesperson, purpose);
    }
    catch(CSqliteException& e) {
        Exception::rethrowSqliteException(e, "Creating database");
    }
}

/**
 * constructor
 *   Open an existing logbook for read.
 *
 * @param pFilename - filename containing the logbook.
 */
LogBook::LogBook(const char* pFilename) :
    m_pConnection(nullptr)
{
    if (access(pFilename, R_OK | W_OK)) {
        int e = errno;          // Save the error code
        std::stringstream msg;
        msg << "Could not open the database file " << pFilename
            << ": " << strerror(e);
        std::string m = msg.str();
        throw Exception(m);
    }
    try {
        m_pConnection = new CSqlite(pFilename, CSqlite::readwrite);       
    }
    catch (CSqliteException& e) {
        Exception::rethrowSqliteException(e, "Opening database file");
    }
    
}
/**
 * destructor just kills off the connection
 */
LogBook::~LogBook()
{
    delete m_pConnection;
}

//////////////////////////////////////////////////////////
// API for registered people:

/**
 * createPerson
 *    Create a new person
 *
 *  @param lastName - person's last name.
 *  @param firstName - Person's first name.
 *  @param salutation - Salutation.
 *  @return - new encapsulated person caller must delete this
 *            when no longer needed.
*/
LogBookPerson*
LogBook::addPerson(
    const char* lastName, const char* firstName, const char* salutation
)
{
    return LogBookPerson::create(
        *m_pConnection,lastName, firstName, salutation
    );
}
/**
 * findPeople
 *   Find people that match a where clause. Note that listPeople
 *   is just this with no where clause.
 * @param pWhere - the where clause (without the WHERE keyword).
 *                 This is vulnerable to injection attacks so
 *                 it should not be exposed directly in web code.
 *                 If null, no WHERE clause is generated.
 * @return std::vector<LogBookPerson*> - results
 * @note the LogBookPerson objects are dynamically allocated and
 *           must be deleted when the caller no longer needs them.
 */
std::vector<LogBookPerson*>
LogBook::findPeople(const char* pWhere)
{
    return LogBookPerson::find(*m_pConnection, pWhere);
}
/**
 * listPeople
 *   @return std::vector<LogBookPeople*> - all people defined.
 */
std::vector<LogBookPerson*>
LogBook::listPeople()
{
    return LogBookPerson::list(*m_pConnection);
}
/**
 * getPerson
 *   Return an existing person by id.
 * @param id - the primary key.
 * @return LogBookPerson* - pointer to the logbook person fetched.
 */
LogBookPerson*
LogBook::getPerson(int id)
{
    return new LogBookPerson(*m_pConnection, id);
}
/////////////////////////////////////////////////////////
// Shift API

/**
 * getShift
 *    Return the shift with the specified primary key value.
 *  @param id - primary key.
 *  @return LogBookShift* - dynamically allocated matching shift.
 *  @throw LogBook::Exception - no such shift.
 * 
 */
LogBookShift*
LogBook::getShift(int id)
{
    return new LogBookShift(*m_pConnection, id);
}
/**
 * createShift
 *    Create an empty shift (no members) the shift can have
 *    members added via the addMember object method of the
 *    resulting shift.
 * @param name - name of the new shift.
 * @return LogBookShift* - new shift (dynamically allocated).
 * @throw LogBook::Exception - if a shift by that name already exists.
 */
LogBookShift*
LogBook::createShift(const char* name)
{
    return LogBookShift::create(*m_pConnection, name);
}
/**
 * createShift
 *    Same as above but:
 *  @param members - provides shift members.
 */
LogBookShift*
LogBook::createShift(
    const char* name, const std::vector<LogBookPerson*>& members
)
{
    return LogBookShift::create(*m_pConnection, name, members);
}
/**
 * listShifts
 *    Returns a list of the shifts.
 * @return std::vector<LogBookShift*> - each member is dynamically
 *              allocated and, therefore must be deleted when
 *              no longer needed.
 */
std::vector<LogBookShift*>
LogBook::listShifts()
{
    return LogBookShift::list(*m_pConnection);
}
/**
 * findShift
 *
 * @param name - name of the shift to find.
 * @return LogBookShift*  dynamically allocated shift with the
 *                        requested name. nullptr if there's no
 *                        matching shift.
 */
LogBookShift*
LogBook::findShift(const char* name)
{
    return LogBookShift::find(*m_pConnection, name);
}
/**
 *   setCurrentShift
 * set the current shift to a specied one.
 *
 * @param pShift - pointer to the shift to set a current.
 */
void
LogBook::setCurrentShift(LogBookShift* pShift)
{
    pShift->setCurrent(*m_pConnection);
}
/**
 * getCurrentShift
 *    Return  the current shift (if there is one).
 *
 * @return LogBookShift* pointer to the current shift.
 * @retval nullptr - there's no current shift.
 */
LogBookShift*
LogBook::getCurrentShift()
{
    return LogBookShift::getCurrent(*m_pConnection);
}

/////////////////////////////////////////////////////////
// API For Runs:

/**
 * transitionId
 *    Given a named transition, this method retunrs
 *    the id of that transition.
 * @param db   - database reference.
 * @param name - Transition name (e.g. BEGIN).
 * @return int  - Id of the transition.
 * @throw LogBook::Exception if there is no valid id.
 */
int
LogBook::transitionId(CSqlite& db, const char* name)
{
    CSqliteStatement find(
        db,
        "SELECT id FROM valid_transitions where type = ?"
    );
    find.bind(1, name, -1, SQLITE_STATIC);
    ++find;
    if (find.atEnd()) {
        std::stringstream msg;
        msg << name << " is not a valid run transition";
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    return find.getInt(0);
}

/**
 *  currentRun
 * @return The current run.
 * @retval nullptr - there is no currently active run.
 */
LogBookRun*
LogBook::currentRun()
{
    return LogBookRun::currentRun(*m_pConnection);
}
/**
 * runId
 *   Return the id of a run given its run number.
 * @param number - the run number to get.
 * @return int   - run number's id. Failures result in LogBookException.
 */
int
LogBook::runId(int number)
{
    return LogBookRun::runId(*m_pConnection, number);
}
/**
 * begin
 *    Begins a new run in the database.
 * @param number - the run number.
 * @param title  - Title to apply to the run.
 * @param remark - Short plain text remark to associate with the run.
 * @return LogBookRun*  - Encapsulated run.  This is dynamically genearated
 *                 so delete must be called to kill it off at the right time.
 */
LogBookRun*
LogBook::begin(int number, const char* title, const char* remark)
{
    int id  = LogBookRun::begin(*m_pConnection, number, title, remark);
    return new LogBookRun(*m_pConnection, id);
}
/**
 * end
 *   Ends the run.
 *
 *  @param pRun - Run to end- must be current.
 *  @param remark - remark to associate with the end of run.
 *  @note - pRun must point to a dynamically allocated object and
 *          the pointer will be replaced by a new dynamically allocated object
 *          that reflects the proper final state of the run.
 */
void
LogBook::end(LogBookRun*& pRun, const char* remark)
{
    int id = pRun->getRunInfo().s_id;
    LogBookRun::end(*m_pConnection, id, remark);
    delete pRun;
    pRun = new LogBookRun(*m_pConnection, id);
}
/**
 * pause
 *    Pauses the run.
 * @param pRun - pointer to the run object.
 * @param remark - pointer to the remark to attach to the end.
 *  @note - pRun must point to a dynamically allocated object and
 *          the pointer will be replaced by a new dynamically allocated object
 *          that reflects the proper final state of the run.
 */
void
LogBook::pause(LogBookRun*& pRun, const char* remark)
{
    int id = pRun->getRunInfo().s_id;
    LogBookRun::pause(*m_pConnection, id, remark);
    delete pRun;
    pRun = new LogBookRun(*m_pConnection, id);
}
/**
 * resume
 *    Resumes a run.
 *  @param pRun - pointer to the run object to resume.
 *  @param remark - remark to associate with the resumption.,
 *  @note - pRun must point to a dynamically allocated object and
 *          the pointer will be replaced by a new dynamically allocated object
 *          that reflects the proper final state of the run.
 */
void
LogBook::resume(LogBookRun*& pRun, const char* remark)
{
    int id = pRun->getRunInfo().s_id;
    LogBookRun::resume(*m_pConnection, id, remark);
    delete pRun;
    pRun = new LogBookRun(*m_pConnection, id);
}
/**
 * emergencyStop
 *   Does an emergency stop on the run. This is a transtion
 *   that can be performed on a run that is not current.
 *
 * @param pRun - pointer to the run object.
 * @param remark - remark to associate with the operation.
 *  @note - pRun must point to a dynamically allocated object and
 *          the pointer will be replaced by a new dynamically allocated object
 *          that reflects the proper final state of the run.
 */
void
LogBook::emergencyStop(LogBookRun*& pRun, const char* remark)
{
    int id = pRun->getRunInfo().s_id;
    LogBookRun::emergency_end(*m_pConnection, id, remark);
    delete pRun;
    pRun = new LogBookRun(*m_pConnection, id);
}
/**
 * listRuns
 *    Returns a list of the run objects.
 *
 * @return std::vector<LogBookRun*> - must be deleted when done.
 */
std::vector<LogBookRun*>
LogBook::listRuns()
{
    return LogBookRun::list(*m_pConnection);
}
/**
 * findRun
 *    Finds the run with the selected run number.
 *  @param number - number of the run to find.
 *  @return LogBookRun* - pointer to the matching run..null if there's no match.
 */
LogBookRun*
LogBook::findRun(int number)
{
    return LogBookRun::find(*m_pConnection, number);
}
/////////////////////////////////////////////////////////
// Private methods



/**
 * computeTempDir
 *    Compute where temporary files are written.
 * @return std::string
 */
std::string
LogBook::computeTempDir()
{
    const char* Home = getenv("HOME");
    assert(Home);              // We're busted if this fails.
    
    std::string path(Home);
    path += "/";
    path += ".nscl-logbook";
    
    // Make the directory... if this fails and it's not
    // because this exists, that's fatal too:
    
    if(mkdir(path.c_str(), 0755)) {
        assert(errno == EEXIST);
    }
    return path;    
}
/**
 * createSchema
 *    Create the databaseSchema intoi a connection.
 *
 *  @param db  - connection to the database.
 */
void
LogBook::createSchema(CSqlite& db)
{
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS kvstore (           \
            id   INTEGER PRIMARY KEY,                   \
            key  TEXT,                                  \
            value TEXT                                  \
        )"
    );
    
    // Store people.
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS person  (                \
            id         INTEGER PRIMARY KEY,                  \
            lastname   TEXT,                                  \
            firstname  TEXT,                                  \
            salutation TEXT                                  \
        )"
    );
    
    // Store shifts:
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS shift (                 \
            id    INTEGER PRIMARY KEY,                      \
            name TEXT                                       \
        )"
    );
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS shift_members (        \
            shift_id   INTEGER,                            \
            person_id  INTEGER                             \
        )"
    );
    
    // Current shift:
    
    CSqliteStatement::execute(
        db, 
        "CREATE TABLE IF NOT EXISTS current_shift (  \
            shift_id  INTEGER                              \
        )"
    );
    // Runs and their state transitions.
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS run (             \
            id   INTEGER PRIMARY KEY,                 \
            number INTEGER,                           \
            title TEXT                                \
        )"
    );
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS run_transitions ( \
            id   INTEGER PRIMARY KEY,                  \
            run_id INTEGER,                            \
            transition_type INTEGER,                   \
            time_stamp      INTEGER,                   \
            shift_id        INTEGER,                   \
            short_comment   TEXT                      \
        )"
    );
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS valid_transitions ( \
            id      INTEGER PRIMARY KEY,              \
            type    TEXT                              \
        )"
    );
    stockValidTransitions(db);
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS  valid_state_transitions (\
            from_id   INTEGER,                                   \
            to_id     INTEGER                                    \
        )"
    );
    stockStateMachine(db);
    
    // Finally the current run, if there is one.
    
    CSqliteStatement::execute(
        db,
        "CREATE TABLE IF NOT EXISTS current_run (id INTEGER)"
    );
}

/**
 * initialize
 *    Initialize the database by adding the following
 *    to the key value store:
 *
 *    -  experiment - experiment id;
 *    -  spokesperson - name of spokesperson.
 *    -  purpose      - Purpose of experiment.
 *
 *  @param db - database connecion.
 *  @param pExperiment - experiment identifier.
 *  @param pSpokesperson- name of the spokesperson
 *  @param purpose   - purpose of the experiment.
 *
 */

void
LogBook::initialize(
    CSqlite& db, const char* pExperiment,
       const char* pSpokesperson, const char* purpose
)
{
    CSqliteStatement insert(
        db,
        "INSERT INTO kvstore (key, value) VALUES (?,?)"
    );
    
    insert.bind(1, "experiment", -1, SQLITE_STATIC);
    insert.bind(2, pExperiment, -1, SQLITE_STATIC);
    ++insert;
    
    insert.reset();
    insert.bind(1, "spokesperson", -1, SQLITE_STATIC);
    insert.bind(2, pSpokesperson,-1,  SQLITE_STATIC);
    ++insert;
    
    insert.reset();
    insert.bind(1, "purpose", -1, SQLITE_STATIC);
    insert.bind(2, purpose, -1, SQLITE_STATIC);
    ++insert;
 
    insert.reset();
    insert.bind(1, "version", -1, SQLITE_STATIC);
    insert.bind(2, DBVersion, -1, SQLITE_STATIC);
    ++insert;
    
}
/**
 * stockValidTransitions
 *    Stocks the valid transitions table with the set of
 *    allowed run state transitions.
 *
 *  @param db  reference to the database handle.
 */
void
LogBook::stockValidTransitions(CSqlite& db)
{
    const char* transitionNames[] = {
        "BEGIN", "END", "PAUSE", "RESUME", "EMERGENCY_END",
        nullptr
    };
    CSqliteStatement put(
        db,
        "INSERT INTO valid_transitions (type) VALUES(?)"
    );
    const char** p = transitionNames;
    while (*p) {
        put.reset();
        put.bind(1, *p, -1, SQLITE_STATIC);
        ++put;
        ++p;
    }
}
/**
 * stockStateMachine
 *    while the valid_transitions table provides a
 *    name->id mapping for the valid state transitions, which
 *    ones are legal in which states defines a state machine that's
 *    stored in valid_state_transitions.  That table has
 *    from/to pairs for all valid state transitions.
 *
 *    This method stocks that table with the valid transitions.
 *    that is given the last transition for a run, which
 *    next transitions are allowed.  Note begin is a bit strange
 *    as it's entered when the run is created so there are
 *    no legal transitions that allow a run to be begun, once
 *    it's already been created.
 *
 *    Similarly, once a run has been ended, there are no valid
 *    transitions (can't restart a run).
 *    
 * @param db - References the data base who's valid_state_transitions
 *             table is being stocked.
 */
void
LogBook::stockStateMachine(CSqlite& db)
{
    // From/to transition names:
    std::vector<std::pair<std::string, std::string>> legal =
    {
        {"BEGIN", "END"}, {"BEGIN", "PAUSE"}, {"BEGIN", "EMERGENCY_END"},
        {"PAUSE", "RESUME"}, {"PAUSE", "END"}, {"PAUSE", "EMERGENCY_END"},
        {"RESUME", "END"}, {"RESUME", "PAUSE"}, {"RESUME", "EMERGENCY_END"},
    };
    
    CSqliteStatement insert(
        db,
        "INSERT INTO valid_state_transitions (from_id, to_id) \
             VALUES (?,?)"
    );
    for (int i = 0; i < legal.size(); i++) {
        auto from = legal[i].first;
        auto to    = legal[i].second;
        
        int from_id = transitionId(db, from.c_str());
        int to_id   = transitionId(db, to.c_str());
        
        insert.reset();
        insert.bind(1, from_id);
        insert.bind(2, to_id);
        ++insert;
    }
}