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

#ifndef CRESUMERUN_H
#define CRESUMERUN_H

#include <TCLObjectProcessor.h>
#include <string>
#include <vector>

class CTCLInterpreter;
class CTCLObject;

/*!
   Command processor object that processes the resume run
   command.  This restarts data taking that was halted using
   pause.
*/
class CResumeRun : public CTCLObjectProcessor
{
  // Cannonicals:
public:
  CResumeRun(CTCLInterpreter& interp);
  virtual ~CResumeRun();

private:
  CResumeRun(const CResumeRun& rhs);
  CResumeRun& operator=(const CResumeRun& rhs);
  int operator==(const CResumeRun& rhs) const;
  int operator!=(const CResumeRun& rhs) const;
public:

  // command processing

protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
};


#endif
