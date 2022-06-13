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

#ifndef CRESUMECOMMAND_H
#define CRESUMECOMMAND_H


#include <TCLPackagedObjectProcessor.h>
#include <vector>
#include <string>


// Forward class defs:

class CTCLInterpreter;
class CTCLObject;

/*!
   This class provides the resume command.  The resume command
   restarts a paused data taking run.  We are a packaged command
   that is part of the Run Control package.

*/
class CResumeCommand : public CTCLPackagedObjectProcessor
{
  // Canonicals, the various copy like things are not allowed:

public:
  CResumeCommand(CTCLInterpreter& interp);
  virtual ~CResumeCommand();

private:
  CResumeCommand(const CResumeCommand& rhs);
  CResumeCommand& operator=(const CResumeCommand& rhs);
  int operator==(const CResumeCommand& rhs) const;
  int operator!=(const CResumeCommand& rhs) const;

  // Command entry point:

public:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

private:
  std::string usage();
};

#endif
