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

#ifndef CSETCOMMAND_H
#define CSETCOMMAND_H

#include <TCLObjectProcessor.h>

#include <vector>
#include <string>


class CCtlConfiguration;
class CTCLObject;
class CTCLInterpreter;
class CVMUSB;

/*!
  CSetCommand implements the Set command.
  This command sets the value of a control point.
  The command has a single form:

  "Set name what value"

   where:
   - name is the name of a control object.
   - what is the name of a control point within that object.
   - value is the new value for that control point.
   
   Consider for example an 8 channel gate and delay generator named
   gdg1  A command like:

   Set gdg1 delay0 10

   Might set the delay for channel 0 to 10  in that module.
*/
class CSetCommand : public CTCLObjectProcessor
{
private:
  CCtlConfiguration&   m_config;	// Tcl server that is running us.
  CVMUSB&      m_Vme;
public:
  CSetCommand(CTCLInterpreter&   interp,
      	      CCtlConfiguration& config,
	            CVMUSB&            vme);
  virtual ~CSetCommand();
private:
  CSetCommand(const CSetCommand& rhs);
  CSetCommand& operator=(const CSetCommand& rhs);
  int operator==(const CSetCommand& rhs) const;
  int operator!=(const CSetCommand& rhs) const;
public:

  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);


};

#endif
