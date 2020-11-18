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

/** @file:  TclNoteInstance.cpp
 *  @brief: Implement the Tcl wrapping of a logbook note.
 */
#include "TclNoteInstance.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "LogBookNote.h"
#include "LogBook.h"
#include <stdexcept>
#include <sstream>

std::map<std::string, TclNoteInstance*> TclNoteInstance::m_instanceMap;

/**
 * constructor:
 *    @param interp - interpreter on which the command will be registered.
 *    @param name   - name of the new command.
 *    @param pBook  - Pointer to a logbook we will include in our encapsulation
 *    @param pNote  - Pointer to the note object we are encapsulating
 */
TclNoteInstance::TclNoteInstance(
    CTCLInterpreter& interp, const char* name, LogBook* pBook,
        LogBookNote* pNote
) :
    CTCLObjectProcessor(interp, name, true),
    m_logBook(pBook), m_note(pNote)
{
    // Register the command in the instance map:
    
    m_instanceMap[name] = this;
}
/**
 * destructor
 *    Just remove ourself from the instance map.  The smart pointers
 *    will take care of any additional object destruction we may need.
 */
TclNoteInstance::~TclNoteInstance()
{
    auto p = m_instanceMap.find(getName());
    if (p != m_instanceMap.end()) m_instanceMap.erase(p);
}
/**
 * operator()
 *    Called when the command ensemble's base command is issued.
 *    Currently a stub
 *  @param interp - interpreter running the command.
 *  @param objv   - command line words.
 *  @return int   - TCL_OK - success, TCL_ERROR -failure.
 */
int
TclNoteInstance::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    return TCL_OK;
}
//////////////////////////////////////////////////////////////////////////////
// Static methods

/**
 * getCommandObject
 *    Return a pointer to the command object given its name
 * @param name -command name string.
 * @return TclNoteInstance* - pointer to the command object with that name.
 */
TclNoteInstance*
TclNoteInstance::getCommandObject(const std::string& name)
{
    auto p = m_instanceMap.find(name);
    if (p == m_instanceMap.end()) {
        std::stringstream msg;
        msg << "There is no Tcl note instance command: " << name;
        std::string e(msg.str());
        throw std::out_of_range(e);
    }
    return p->second;
}