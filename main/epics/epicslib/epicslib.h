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

/** @file:  epicslib.h
 *  @brief: Header defining EPICS library functions.
 */

// This file and the functions were motivated by daqdev/NSCLDAQ#700

#ifndef EPICSLIB_H
#define EPICSLIB_H
#include <string>

namespace epicslib {
    void checkChanStatus(int status, const char* chan, const char* format);
    void startRepeater(int argc, char** argv);

}
#endif
