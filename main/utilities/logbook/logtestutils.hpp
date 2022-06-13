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

/** @file:  logtestutils.hpp
 *  @brief: Logbook test utilities.
 */
#ifndef LOGTESTUTILS_H
#define LOGTESTUTILS_H
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>

#include "LogBookPerson.h"

static std::string tempFilename(const char* templateName)
{
    char name[500];
    strncpy(name, templateName, sizeof(name));
    int fd = mkstemp(name);
    ASSERT(fd >= 0);
    close(fd);
    unlink(name);
    
    return std::string(name);
}

static std::ostream& operator<<(std::ostream& s, LogBookPerson& p)
{
    s << "ID:    " << p.id() << std::endl;
    s << "Last:  " << p.lastName() << std::endl;
    s << "First: " << p.firstName() << std::endl;
    s << "Salut: " << p.salutation() << std::endl;
    
    return s;
}

static int equals(const LogBookPerson& lhs, const LogBookPerson& rhs)
{
    return (
        (std::string(lhs.lastName()) == std::string(rhs.lastName())) &&
        (std::string(lhs.firstName()) == std::string(rhs.firstName())) &&
        (std::string(lhs.salutation()) == std::string(rhs.salutation()))
    );
}

#endif