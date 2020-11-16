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

/** @file:  TclLogbookPersonInstance.cpp
 *  @brief: Implement the logbook person instance command ensemble.
 */
#include "TclPersonInstance.h"
#include "LogBookPerson.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>


/**
 * constructor.
 *   @param interp - interpreter on which the cmd is registered.
 *   @param name   - command name under which the object is registered.
 *   @param pPerson - Pointer to the person to be encapsulated
 */
TclPersonInstance::TclPersonInstance(
    CTCLInterpreter& interp, const char* name, LogBookPerson* pPerson
) :
    CTCLObjectProcessor(interp, name, true),
    m_person(pPerson)
{}
/**
 * destructor
 */
TclPersonInstance::~TclPersonInstance()
{}

/**
 * operator()
 *   STUB
 */
int TclPersonInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    return TCL_OK;
}