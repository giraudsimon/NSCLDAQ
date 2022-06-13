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


#ifndef __CINIT_H
#define __CINIT_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



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
