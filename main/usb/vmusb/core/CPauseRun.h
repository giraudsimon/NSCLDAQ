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

#ifndef CPAUSERUN_H
#define CPAUSERUN_H

#include <TCLObjectProcessor.h>
#include <string>
#include <vector>

class CTCLInterpreter;
class CTCLObject;

/*!
   Command object that processa a pause command that puts a temporary
   pause to data taking for a run.
*/
class CPauseRun : public CTCLObjectProcessor
{
  // Cannonicals
public:
  CPauseRun(CTCLInterpreter& interp);
  virtual ~CPauseRun();

private:
  CPauseRun(const CPauseRun& rhs);
  CPauseRun& operator=(const CPauseRun& rhs);
  int operator==(const CPauseRun& rhs) const;
  int operator!=(const CPauseRun& rhs) const;
public:

  // command processing

protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
};

#endif
