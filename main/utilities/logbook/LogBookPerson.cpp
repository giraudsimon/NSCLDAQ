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

/** @file:  LogBookPerson.cpp
 *  @brief: LogBookPerson implementation.
 */
#include "LogBookPerson.h"
#include "LogBook.h"

#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <CSqliteException.h>
#include <sstream>

/**
 * constructor:
 *   Fetches the data for the object by id from the person table:
 *
 * @param db   - database handle
 * @param id   - Primary key of the record to pull in.
 * @throw LogBook::Exception - if there's no such person.
 */
LogBookPerson::LogBookPerson(CSqlite& db, int id) :
    m_id(id)
{
    try {
        CSqliteStatement get(
            db,
            "SELECT lastname, firstname, salutation FROM person   \
                WHERE id=?"
        );
        get.bind(1, id);
        ++get;
        if(get.atEnd()) {                 // Nobody with that ID:
            std::stringstream msg;
            msg << "There is no person with the id "
                << id;
            std::string m(msg.str());
            throw LogBook::Exception(m);
        }
        // Store the rest of the object from our record:
        // Note the primary key is unique so there's only one hit:
        
        m_lastName = get.getString(0);
        m_firstName = get.getString(1);
        m_salutation = get.getString(2);
    } catch(CSqliteException& e) {
        LogBook::Exception::rethrowSqliteException(
            e, "Looking up person by id"
        );
    }
    
}

/**
 * copy constructor
 *    @param rhs - thing we're copying from.
 */
LogBookPerson::LogBookPerson(const LogBookPerson& rhs)
{
    copyIn(rhs);
}

/**
 * private constructor
 *   In general, this is used to construct from the result
 *   set of a query, as such it's private.
 * @param id        - primary key value of the record.
 * @param lastName  - Last name 
 * @param firstName - First name
 * @param salutation- Salutation.
 * @note The types are of the sort you'd get from getText().
 */
LogBookPerson::LogBookPerson(
    int id, unsigned const char* lastName, unsigned const char* firstName,
    unsigned const char* salutation
) :
    m_id(id),
    m_lastName(reinterpret_cast<const char*>(lastName)),
    m_firstName(reinterpret_cast<const char*>(firstName)),
    m_salutation(reinterpret_cast<const char*>(salutation))
{}

/**
 * assignment is allowed
 *   @param rhs - what we're assigning from
 *   @return *this
 */
LogBookPerson&
LogBookPerson::operator=(const LogBookPerson& rhs)
{
    if (this != &rhs) {
        copyIn(rhs);
    }
    return *this;
}
/**
 * equality test -it's really enough to check the id:
 *   @param rhs - object we're comparing to.
 *   @return int- non zero for equality.
 */
int
LogBookPerson::operator==(const LogBookPerson& rhs) const
{
    return m_id == rhs.m_id;
}
/**
 *  Inequality tes just invert equality:
*/
int
LogBookPerson::operator!=(const LogBookPerson& rhs) const
{
    return !(m_id == rhs.m_id);
}
/**
 * getters - get field values
 */

const char*
LogBookPerson::lastName() const
{
    return m_lastName.c_str();
}
const char*
LogBookPerson::firstName() const
{
    return m_firstName.c_str();
}
const char*
LogBookPerson::salutation() const
{
    return m_salutation.c_str();
}
int
LogBookPerson::id() const
{
    return m_id;
}
/**
 * create
 *    Create a log book person
 *  @param db  - database object in which we're creating the person.
 *  @param lastName - last name to use.
 *  @param firstName -first name ot use
 *  @param salutation- Salutation to use.
 *  @return LogBookPerson* - new person wrapped. Note this
 *                   is dynamically allocated and must be deleted by the
 *                   caller.
 *   @throw LogBook::Exception.
 */
LogBookPerson*
LogBookPerson::create(
    CSqlite& db,
    const char* lastName, const char* firstName, const char* salutation
)
{
    try {
        CSqliteStatement insert(
            db,
            "INSERT INTO person (lastname, firstname, salutation) \
                VALUES(?,?,?)"
        );
        insert.bind(1, lastName, -1, SQLITE_STATIC);
        insert.bind(2, firstName, -1, SQLITE_STATIC);
        insert.bind(3, salutation, -1, SQLITE_STATIC);
        ++insert;
        return new LogBookPerson(
            insert.lastInsertId(),
            reinterpret_cast<const unsigned char*>(lastName),
            reinterpret_cast<const unsigned char*>(firstName),
            reinterpret_cast<const unsigned char*>(salutation)
        );
    }
    catch (CSqliteException& e) {
        LogBook::Exception::rethrowSqliteException(
            e, "Attempting to insert a new person record"
        );
    }
    throw LogBook::Exception(
        "LogBookPerson constructor - DEFECT should not have reached end of method"
    );
}
/**
 * find
 *   Finds the people that match the search specification.
 *   This call should probably not be exposed to web applications
 *   as I _think_ even though a prepared statement is constructed,
 *   it may be vunlerable to sql injection attacks.
 * @param db   - database to earch
 * @param where - where clause to use if null, the search is unconditional.
 * @return std::vector<LogBookPerson*> - Matching people Note that
 *                all people are dynamically allocated and must be
 *                deleted when no longer needed.
 * @throws LogBook::Exception
 * @note the "WHERE" keyword should not be part of the clause
 * @note that a null where parameter makes this method identical to
 *        list.
 * @note - naturally the vector could be empty if there are no matches.
 * @toodo - There's unfortunately no way to bind parameters into the
 *          WHERE clause.
 *
 *  Example:
 *  \verbatim
 *      auto results = LogBookPerson::find(db, "lastname = 'Fox' AND firstname = 'Ron'");
 *  \endverbatim
*/
std::vector<LogBookPerson*>
LogBookPerson::find(CSqlite& db, const char* where)
{
    // Buld the select statement:
    
    std::string statement("SELECT id, lastname, firstname, salutation \
        FROM person");
    
    if (where) {
        statement += " WHERE ";
        statement += where;
    }
    // Now try to execute it:
    
    try  {
        std::vector<LogBookPerson*> result;
        CSqliteStatement find(db, statement.c_str());
        while (!(++find).atEnd()) {
            result.push_back(
                new LogBookPerson(
                    find.getInt(0),
                    find.getText(1), find.getText(2), find.getText(3)
                    
                )
            );
        }
        return result;
    }
    catch (CSqliteException& e) {
        LogBook::Exception::rethrowSqliteException(
            e, "Finding people in LogBookPerson::find"
        );
    }
    // Should not get here:
    
    throw LogBook::Exception(
        "LogBookPerson::Find - DEFECT - control reached end of method."
    );
}
/**
 * list
 *    Return all of the people:
 *    
 *   @param db   - data base
 *   @return std::vector<LogBookPerson*> - contains all people wrapped.
 *   @throw LogBook::Exception
 *   @note elements of the returned vector are dynamically allocated
 *         and must be deleted when no longer needed.
 */
std::vector<LogBookPerson*>
LogBookPerson::list(CSqlite& db)
{
    return find(db);    // select with no where clause.
}
/////////////////////////////////////////////////////////////
// Private utilities.

/**
 * copyIn
 *    Copy tho this from rhs
 */
void
LogBookPerson::copyIn(const LogBookPerson& rhs)
{
    m_id         = rhs.m_id;
    m_lastName   = rhs.m_lastName;
    m_firstName  = rhs.m_firstName;
    m_salutation = rhs.m_salutation;
}