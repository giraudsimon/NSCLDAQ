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