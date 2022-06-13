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

#ifndef RUNSTATE_H
#define RUNSTATE_H

#include <stdint.h>
#include <string>




/*!
  RunState is a singleton struct.   Since structs don't actually
  support private constructors, this is implemented as a class with
  all member data public,  constructor private, and the usual machinery
  for creating/maintaining the instance.
*/

class RunState
{
  // Data types:

public:
  // Possible run state values:

  typedef enum _State {
    inactive,
    active,
    paused
  } State;

  // The instance pointer remains private:
private:
  static RunState*   m_pInstance;

  // All other data are public:
public:
  State        m_state;
  uint32_t     m_runNumber;
  uint32_t     m_timeOffset;
  char*        m_pTitle;	// std::string can't be shared between threads

  // All constructors are private as are assignment comparisons etc.

private:
  RunState();
  ~RunState();
  RunState(const RunState&);
  RunState& operator=(const RunState&);
  int operator==(const RunState&) const;
  int operator!=(const RunState&) const;

  // But we have a public instance retrieval method;

public:
  static RunState*  getInstance();

  std::string stateName() const;
  static std::string stateName(State state);
};

#endif
