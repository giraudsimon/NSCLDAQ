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

/** @file:  CControlHardware.cpp
 *  @brief: VMUSB specific utilities in CVMUSBControlHardware.
 */
#include "CControlHardware.h"
#include <CVMUSBReadoutList.h>
#include <sstream>

/**
 * doList
 *   Performs a list operation with error checking; reporting
 *   errors as string exceptions.
 * @param ctlr - Reference to the VMUSB controller that will
 *               perform the list.
 * @param list - References the VMUSB list to execute.
 * @param data - Pointer to where returned data should go.
 * @param expectedSize - number of bytes of data expected.
 * @param actualSize - pointer to where actual read size should go.
 * @param msg  - Base of error message to throw in case of list error.
 */
void
CVMUSBControlHardware::doList(
    CVMUSB& ctlr, CVMUSBReadoutList& list,
    void* data, size_t expectedSize, size_t* actualSize,
    const char* msg
)
{
  int status = ctlr.executeList(list, data, expectedSize, actualSize);

  if (status<0) {
    std::stringstream errmsg;
    errmsg << msg << " - executeList returned status = ";
    errmsg << status;

    throw errmsg.str();
  }    
}