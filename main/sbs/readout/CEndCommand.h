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

#ifndef CENDCOMMAND_H
#define CENDCOMMAND_H


#include <TCLPackagedObjectProcessor.h>
#include <vector>
#include <string>
#include <tuple>

// Forward class defs:

class CTCLInterpreter;
class CTCLObject;

/*!
   This class provides the end command.  The end command
   permanently ends a data taking run.  We are a packaged command
   that is part of the Run Control package.

*/
class CEndCommand : public CTCLPackagedObjectProcessor
{
  // Canonicals, the various copy like things are not allowed:

public:
  CEndCommand(CTCLInterpreter& interp);
  virtual ~CEndCommand();

private:
  CEndCommand(const CEndCommand& rhs);
  CEndCommand& operator=(const CEndCommand& rhs);
  int operator==(const CEndCommand& rhs) const;
  int operator!=(const CEndCommand& rhs) const;

  // Command entry point:

public:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

protected:
  std::tuple<int, std::string> end();

private:
  std::string usage();
};

#endif
