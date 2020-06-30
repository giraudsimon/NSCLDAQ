#ifndef STRINGSTOINTEGERS_H
#define STRINGSTOINTEGERS_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#include <string>
#include <vector>
#include <CInvalidArgumentException.h>



/*! Convert list of strings to integers.
 */
std::vector<int>
stringListToIntegers(std::string items) ;
#endif
