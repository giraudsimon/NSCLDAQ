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
#include <sstream>

std::map<std::string, TclPersonInstance*> TclPersonInstance::m_personCommands;
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
{
    // Save a lookup entry for the command.
    
    m_personCommands[name] = this;        
}
/**
 * destructor
 */
TclPersonInstance::~TclPersonInstance()
{
    // Destroy a lookup entry for the command.
    
    auto mapEntry = m_personCommands.find(getName());
    if (mapEntry != m_personCommands.end()) {
        m_personCommands.erase(mapEntry);
    }
}

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
/**
 * getCommanObject [static]
 *    Lookup a command object by name.
 *  @param name - name of the command.
 *  @return TclPersonInstance* - pointer to the command.
 *  @throw std::out_of_range - if there's no mathcing entry.
 */
TclPersonInstance*
TclPersonInstance::getCommandObject(const std::string& name)
{
    auto p = m_personCommands.find(name);
    if (p == m_personCommands.end()) {
        std::stringstream msg;
        msg << name << " is not an existing person instance command";
        std::string e = msg.str();
        throw std::out_of_range(e);
    }
    return p->second;
}