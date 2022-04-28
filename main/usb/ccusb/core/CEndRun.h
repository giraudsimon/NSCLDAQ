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

#ifndef __CENDRUN_H
#define __CENDRUN_H

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
