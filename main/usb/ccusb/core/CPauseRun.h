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

#ifndef __CPAUSERUN_H
#define __CPAUSERUN_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

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
