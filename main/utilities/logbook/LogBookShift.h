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

/** @file:  LogBookShift.h
 *  @brief: Define class for log book shift (collection of users)
 */
#ifndef LOGBOOKSHIFT_H
#define LOGBOOKSHIFT_H

#include <string>
#include <vector>

class LogBookPerson;
class CSqlite;

/**
 * @class LogBookShift
 *    A shift is a collection of people that are working together at the same
 *    time on an experiment.  Shifts have names and they have members.
 *    This header defines an API that allows clients to:
 *    -  Create a new shift.
 *    -  Add members to a shift
 *    -  List the shifts
 *    -  Get a shift given its name.
 *    -  Get the members of a shift.
 *
 *  See as well LogBook.h which provides a top-level API that only requires
 *  having a LogBook object.
 */
class LogBookShift
{
private:
    int           m_id;                    // Primary key of a wrapped shift.
    std::string   m_name;                  // Name of the shift.
    std::vector<LogBookPerson*> m_members; // Who's on the shift.

public:
    LogBookShift(CSqlite& db, int id);    // Construct from database.
    virtual ~LogBookShift();
    
private:
    LogBookShift(
        int id, const unsigned char* name,
        const std::vector<LogBookPerson*>& members
    );                                  // Construct after lookup.
public:
    int                               id() const;
    const char*                       name() const;
    const std::vector<LogBookPerson*>& members() const;
    
    void addMember(CSqlite& db, LogBookPerson* person);
    void removeMember(CSqlite& db, LogBookPerson* person);
    void setCurrent(CSqlite& db);
    
    
public:
    static LogBookShift* create(
        CSqlite& db,
        const char* shiftName, const std::vector<LogBookPerson*>& people
    );
    static LogBookShift* create(CSqlite& db, const char* shiftName);
    static std::vector<LogBookShift*> list(CSqlite& db);
    static LogBookShift* find(CSqlite& db, const char* name);
    static LogBookShift* getCurrent(CSqlite& db);
};


#endif
