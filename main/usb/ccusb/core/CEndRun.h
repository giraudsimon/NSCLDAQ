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

#ifndef CENDRUN_H
#define CENDRUN_H

#include <TCLObjectProcessor.h>
#include <vector>
#include <string>

class CTCLInterpreter;
class CTCLInterpreterObject;

/*!
   Class to implement the end command.  This command will
   end an active physics run.  For a blow by blow of what is
   involved in that, see the operator() comments.
*/
class CEndRun : public CTCLObjectProcessor
{
  // Canonicals:
public:
  CEndRun(CTCLInterpreter& interp);
  virtual ~CEndRun();
private:
  CEndRun(const CEndRun& rhs);
  CEndRun& operator=(const CEndRun&);
  int operator==(const CEndRun& rhs) const;
  int operator!=(const CEndRun& rhs) const;
public:

  // Process the command.
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
};


#endif
