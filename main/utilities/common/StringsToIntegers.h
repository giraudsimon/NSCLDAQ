#ifndef __STRINGSTOINTEGERS_H
#define __STRINGSTOINTEGERS_H

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

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CINVALIDARGUMENTEXCEPTION_H
#include <CInvalidArgumentException.h>
#endif



/*! Convert list of strings to integers.
 */
std::vector<int>
stringListToIntegers(std::string items) ;
#endif
