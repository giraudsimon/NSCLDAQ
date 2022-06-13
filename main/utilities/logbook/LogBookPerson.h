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

/** @file:  LogBookPerson.h
 *  @brief: Encapsulate a person record in the database.
 */

#ifndef LOGBOOKPERSON_H
#define LOGBOOKPERSON_H
#include <string>
#include <vector>

class CSqlite;

/**
 * @class LogBookPerson
 *    This class encapsulates a person. Normally,
 *    clients don't directly instantiate it but rather
 *    use the API in LogBook to create, search for and otherwise
 *    fetch entries from the person table.
 */
class LogBookPerson
{
private:
    int         m_id;                  // Primary key.
    std::string m_lastName;            // Person's last name.
    std::string m_firstName;           // Person's first name.
    std::string m_salutation;          // Person's salutation (e.g. Mr. Ms. Dr.)

public:
    LogBookPerson(CSqlite& db, int id);  // Fetch an existing person.
    LogBookPerson(const LogBookPerson& rhs);
private:
    LogBookPerson(
        int id, unsigned const char* lastName, unsigned const char* firstName,
        unsigned const char* salutation
    );
public:
    LogBookPerson& operator=(const LogBookPerson& rhs);
    int operator==(const LogBookPerson& rhs) const;
    int operator!=(const LogBookPerson& rhs) const;
    // Getters:
    
    const char* lastName() const;
    const char* firstName() const;
    const char* salutation() const;
    int         id() const;
    
    static LogBookPerson* create(
        CSqlite& db,
        const char* lastName, const char* firstName, const char* salutation
    );
    static std::vector<LogBookPerson*> find(CSqlite& db, const char* where=nullptr);
    static std::vector<LogBookPerson*> list(CSqlite& db);
    
private:
    void copyIn(const LogBookPerson& rhs);
};


#endif