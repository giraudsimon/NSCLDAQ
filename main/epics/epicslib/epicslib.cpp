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

/** @file:  epicslib.cpp
 *  @brief: implement epicslib.
 */
#include "epicslib.h"
#include <cadef.h>
#include <stdio.h>
#include <string.h>

/**
 * checkChanStatus
 *    Given a status if it's not success, throw an std::string.
 *
 *   @param status - Status value from Epics CA.
 *   @param chan   - Name of the relevant channel
 *   @param format - sprintf format string. It must have
 *                   two slots in order for the channel name and
 *                   detailed message string (ca_message return value).
 *   @throws std::string - the message if not normal
 */
void
epicslib::checkChanStatus(int status, const char* chan, const char* format)
{
    std::string result;
    char       resultBuf[1024];
    if (status != ECA_NORMAL) {
        sprintf(resultBuf, format, chan, ca_message(status));
        result = resultBuf;
        throw result;
    }
    
}