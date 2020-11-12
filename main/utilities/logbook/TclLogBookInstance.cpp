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

/** @file:  TclLogBookInstance.cpp
 *  @brief: implement logbook instance commands
 */

#include "TclLogBookInstance.h"
#include "LogBook.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <string>
#include <sstream>


/**
 * constructor
 *    @param pInterp - pointer to the interpreter we are going to register on.
 *    @param cmd     - Command name string.
 *    @param pBook   - Logbook we're encapsulating.
 */
TclLogBookInstance::TclLogBookInstance(
    CTCLInterpreter* pInterp, const char* cmd, LogBook* pBook
) :
    CTCLObjectProcessor(*pInterp, cmd, true),
    m_logBook(pBook)
{
        
}
/**
 * destructor
 */
TclLogBookInstance::~TclLogBookInstance()
{}


/**
 * operator()
 *    Stub for now.
 */
int
TclLogBookInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    return TCL_OK;
}