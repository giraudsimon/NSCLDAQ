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
#include <config.h>
#include "RunState.h"
#include <string.h>

// Static member data:

RunState*   RunState::m_pInstance(0); // The instance data.

static const char* InitialTitle="Set New Title";

// The constructor is implemented but private:
// Destruction is not allowed:

RunState::RunState() :
  m_state(RunState::inactive),
  m_runNumber(0),
  m_pTitle(0)
{
  m_pTitle=  new char[strlen(InitialTitle) + 1];
  strcpy(m_pTitle, InitialTitle);
}

/*!
  Return the instance pointer to the singleton object:
*/
RunState*
RunState::getInstance()
{
  if (!m_pInstance) {
    m_pInstance = new RunState;
  }
  return m_pInstance;
}

/*!
  Return the stringified version of the current state:
*/
std::string
RunState::stateName() const
{
  return stateName(m_state);
}
std::string
RunState::stateName(RunState::State state)
{
  switch (state) {
  case inactive:
    return "Inactive";
  case active:
    return "Active";
  case paused:
    return "Paused";
  }
  return "Invalid state";
}
