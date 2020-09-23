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
#include <CSqlite.h>
#include <CSqliteException.h>
#include <sstream>

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