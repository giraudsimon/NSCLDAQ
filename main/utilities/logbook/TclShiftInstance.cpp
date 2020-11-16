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

/** @file:  TclShiftInstance.cpp
 *  @brief: Implement the shift instance command.
 */
#include "TclShiftInstance.h"
#include "LogBookShift.h"

/**
 * constructor
 *   @param interp -interpreter on which the command object is registered.
 *   @param name   - name of the new command.
 *   @param pShift - Shift being encapsulated.
 */
TclShiftInstance::TclShiftInstance(
    CTCLInterpreter& interp, const char* name, LogBookShift* pShift
) :
    CTCLObjectProcessor(interp, name, true),
    m_shift(pShift)
{}

/**
 * destructor
 */
TclShiftInstance::~TclShiftInstance() {}


/**
 * operator()
 *    extract the subcommand and execute it
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 * @return int - TCL_OK - success, TCL_ERROR failed.
 */
int
TclShiftInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    return TCL_OK;                 // STUB.
    
}