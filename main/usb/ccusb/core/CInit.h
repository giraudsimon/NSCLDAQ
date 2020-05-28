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


#ifndef CINIT_H
#define CINIT_H

#include <TCLObjectProcessor.h>
#include <vector>
#include <string>

class CTCLInterpreter;
class CTCLInterpreterObject;

/*!
    This class provides the begin command for the readout main
    command interpreter.  This command starts a data taking run.
    See operator() documentation for a blow by blow of what happens.
    The syntax is simply:

\verbatim
   begin
\endverbatim
*/
class CInit : public CTCLObjectProcessor
{
  // Canonicals:
public:
  CInit(CTCLInterpreter& interp);
  virtual ~CInit();
private:
  CInit(const CInit& rhs);
  CInit& operator=(const CInit& rhs);
  int operator==(const CInit& rhs) const;
  int operator!=(const CInit& rhs) const;
public:

  // Process the command:
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

};


#endif
