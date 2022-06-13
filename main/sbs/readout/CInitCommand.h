/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCLDAQ Development Group  
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CINITCOMMAND_H
#define CINITCOMMAND_H


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
class CInitCommand : public CTCLPackagedObjectProcessor
{
  // Canonicals, the various copy like things are not allowed:

public:
  CInitCommand(CTCLInterpreter& interp);
  virtual ~CInitCommand();

private:
  CInitCommand(const CInitCommand& rhs);
  CInitCommand& operator=(const CInitCommand& rhs);
  int operator==(const CInitCommand& rhs) const;
  int operator!=(const CInitCommand& rhs) const;

  // Command entry point:

public:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

private:
  std::string usage();
};

#endif
