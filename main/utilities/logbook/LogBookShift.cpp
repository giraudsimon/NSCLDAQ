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

/** @file:  LogBookShift.cpp
 *  @brief: Implement the LogBookShift class.
 */
#include "LogBookShift.h"
#include "LogBook.h"
#include "LogBookPerson.h"

#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>
#include <CSqliteWhere.h>
#include <CSqliteException.h>
#include <sstream>

/**
 * construction from database:
 *   -  Lookup the shift and its join-table elements.
 *   -  Form an IN clause to locate the people on the shift.
 *   -  Query for the people on the shift.
 *   -  Populate our object.
 *
 *  @param db   - database connection
 *  @param id   - Id of the shift.
 *  @note - we consider an empty name to be an impossibility.
 */
LogBookShift::LogBookShift(CSqlite& db, int id) :
    m_id(id)
{
    // Find the shift top level record:
    
    CSqliteStatement fshift(
        db,
        "SELECT name FROM shift WHERE id = ?"
    );
    fshift.bind(1, id);
    ++fshift;
    if(fshift.atEnd()) {
        std::stringstream msg;
        msg << "There is no shift with the id: " << id;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    m_name = fshift.getString(0);
    
    // Find the members:
    
    CSqliteStatement fmembers(
        db,
        "SELECT person_id FROM shift_members WHERE shift_id = ?"
    );
    fmembers.bind(1, id);
    
    std::vector<int> personIds;
    while(!(++fmembers).atEnd()) {
        personIds.push_back(fmembers.getInt(0));
    }
    
    // Now create the IN clause for our LogBookPerson find:
    if (!personIds.empty()) {
        CInFilter filter("id", personIds);
        m_members = LogBookPerson::find(db, filter.toString().c_str());
    }
}
/**
 * destructor
 *   We have to delete the members:
 */
LogBookShift::~LogBookShift()
{
    for (int i =0; i < m_members.size(); i++) {
        delete m_members[i];
    }
    m_members.clear();                // Not strictly needed.
}
/**
 * constructor from lookup.
 *
 * @param id       - Primary key.
 * @param name     - Name of the shift.
 * @param members  - Members of the shift; ownership is transferred
 *                   to this.
 */
LogBookShift::LogBookShift(
    int id, const unsigned char* name,
    const std::vector<LogBookPerson*>& members
) :
    m_id(id), m_name(reinterpret_cast<const char*>(name)),
    m_members(members)
{}

/**
 * id
 *     Return the primary key from the cached record:
 *  @return int
 */
int
LogBookShift::id() const
{
    return m_id;
}
/**
 * name
 *  @return const char* - the name of the shift.
 */
const char*
LogBookShift::name() const
{
    return m_name.c_str();
}
/**
 * members
 *   Return members.
 * @param const std::vector<const LogBookPerson*>&
 */
const std::vector<LogBookPerson*>&
LogBookShift::members() const
{
    return m_members;
}

/**
 * addMember
 *    Add a new member to a shift.
 * @param db  - database.
 * @param person - Person to add. Ownership is transferred to this
 *                   
 */
void
LogBookShift::addMember(CSqlite& db, LogBookPerson* person)
{
    m_members.push_back(person);     // Ajust us.
    CSqliteStatement insert(
        db,
        "INSERT INTO shift_members (shift_id, person_id)   \
            VALUES (?,?)"
    );
    insert.bind(1, m_id);
    insert.bind(2, person->id());
    ++insert;
}
/**
 * removeMember
 *    Remove a member from the shift.
 * @param db  - database id.
 * @param person - Person to remove.
 * @throw LogBook::Exception - if there's no such person.
 * @note if person was gotten frfom the members() method on this,
 *       it is invalid after we return.
 */
void
LogBookShift::removeMember(CSqlite& db, LogBookPerson* person)
{
    int id = person->id();
    std::vector<LogBookPerson*>::iterator p = m_members.begin();
    for( ; p != m_members.end(); p++) {
        if ((*p)->id() == id) {
            break;
        }
    }
    if (p == m_members.end()) {  // No match.
        std::stringstream msg;
        msg << person->salutation() << " " << person->firstName() << " "
            << person->lastName() << " is not a member the " << m_name << " shift";
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    // Delete from database:
    
    // Delete from collection.
    LogBookPerson* member = *p;
    delete member;
    m_members.erase(p);
    
    CSqliteStatement remove(
        db,
        "DELETE FROM shift_members                    \
            WHERE shift_id = ? AND person_id = ?"
    );
    remove.bind(1, m_id);
    remove.bind(2, id);
    ++remove;   
}
/**
 * setCurrent
 *   Set this shift to be the current one.
 *
 * @param db - Database object reference.
 */
void
LogBookShift::setCurrent(CSqlite& db)
{
    CSqliteTransaction t(db);
    try {
        CSqliteStatement::execute(
            db,
            "DELETE FROM current_shift"
        );                       // Kill any current shift.
        CSqliteStatement s(
            db,
            "INSERT INTO current_shift (shift_id) VALUES (?)"
        );
        s.bind(1, m_id);
        ++s;
    }
    catch(CSqliteException& e) {
        t.scheduleRollback();
        LogBook::Exception::rethrowSqliteException(
            e, "Attempting to set the current shift"
        );
    }
    catch (...) {
        t.scheduleRollback();
        throw;
    }
    
    // Commits the changes.
}

///////////////////////////////////////////////////////////////////////////////
// Static methods:

/**
 * create
 *    Create a new shift with members:
 * @param db        - database connection
 * @param shiftName - name of the new shift.
 * @param people    - People in the shift - ownership transferred to the returned object
 * @return LogBookShift* - pointer to dynamically allocated LogBookShift object
 *                   encapsulating the new record.
 * @throw LogBook::Exception if duplicate shift name.
 * @note - the shift is created in a transaction so the creation is atomic.
 */                   
LogBookShift*
LogBookShift::create(
    CSqlite& db, const char* name, const std::vector<LogBookPerson*>& members
)
{
    // Unique:
    
    CSqliteStatement find(
        db,
        "SELECT * FROM shift WHERE name = ?"
    );
    find.bind(1, name, -1, SQLITE_STATIC);
    ++find;
    if (!(find.atEnd())) {
        std::stringstream msg;
        msg << "There is already a shift named: " << name;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    CSqliteTransaction t(db);
    int id(-1);
    try {
        CSqliteStatement insertRoot(
            db,
            "INSERT INTO shift (name) VALUES (?)"
        );
        insertRoot.bind(
            1, name, -1, SQLITE_STATIC
        );
        ++insertRoot;
        id = insertRoot.lastInsertId();
        
        CSqliteStatement insertMember(
            db,
            "INSERT INTO shift_members (shift_id, person_id)  \
                VALUES (?,?)"
        );
        insertMember.bind(1, id);
        for (int i = 0; i < members.size(); i++) {
            insertMember.bind(2, members[i]->id());
            ++insertMember;
            insertMember.reset();
        }
        
    }
    catch (CSqliteException& e) {
        t.scheduleRollback();
        LogBook::Exception::rethrowSqliteException(e, "Creating a new shift");
    }
    catch (...) {
        t.scheduleRollback();
        throw;
    }
    
    return new LogBookShift(
        id, reinterpret_cast<const unsigned char*>(name), members
    );
}
/**
 * create
 *    Create an empty shift  People need to be addeed to this shift
 *    by using addMember.
 */
LogBookShift*
LogBookShift::create(CSqlite& db, const char* shiftName)
{
    std::vector<LogBookPerson*> members;        // Empty members array.
    return create(db, shiftName, members);
}
/**
 * list
 *    List the shifts that are defined in the database.
 *  @return std::vector<logBookShift*>
 */
std::vector<LogBookShift*>
LogBookShift::list(CSqlite& db)
{
    std::vector<LogBookShift*> result;
    
    CSqliteStatement shifts(
        db, "SELECT id FROM shift"
    );
    while(!(++shifts).atEnd()) {
        int id = shifts.getInt(0);
        result.push_back(new LogBookShift(db, id));
    }
    return result;
}
/**
 * find
 *    Find a shift by name.
 *  @param db  - database.
 *  @param name -name of theshift.
 *  @return LogBookShfit* - pointer to the dynamically allocated shift.
 *  @retval nullptr       - if there's no matching shift.
 */
LogBookShift*
LogBookShift::find(CSqlite& db, const char* name)
{
    LogBookShift* result(nullptr);
    
    CSqliteStatement find(
        db,
        "SELECT id FROM shift WHERE name = ?"
    );
    find.bind(1, name, -1, SQLITE_STATIC);
    ++find;
    if (!find.atEnd()) {
        result = new LogBookShift(db, find.getInt(0));
    }
    
    return result;
}
/**
 * getCurrent
 *   Gets the current shift.
 *
 * @param db - database object reference.
 * @return LogBookShift*
 * @retval nullptr - there's no current shift.
 */
LogBookShift*
LogBookShift::getCurrent(CSqlite& db)
{
    LogBookShift* result(nullptr);
    CSqliteStatement c(
        db,
        "SELECT shift_id FROM CURRENT_SHIFT"
    );
    ++c;
    if (!c.atEnd()) {
        result = new LogBookShift(db, c.getInt(0));
    }
    
    return result;
}