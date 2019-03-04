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

#ifndef CGETCOMMAND_H
#define CGETCOMMAND_H

#include <TCLObjectProcessor.h>

#include <vector>
#include <string>

class CCtlConfiguration;
class CTCLObject;
class CTCLInterpreter;
class CVMUSB;

/*!
   CGetCommand implements the Get command this command
   retrieves the value of a control point.
   The single form is:

   Get name what

   Where:
   - name is the name of a control object
   - what is the name of a control point within the object.
*/

class CGetCommand : public CTCLObjectProcessor
{
  CCtlConfiguration&   m_config;	// Tcl server that is running us.
  CVMUSB&              m_Vme;
public:
  CGetCommand(CTCLInterpreter&   interp,
      	      CCtlConfiguration& config,
	            CVMUSB&            vme);
  virtual ~CGetCommand();
private:
  CGetCommand(const CGetCommand& rhs);
  CGetCommand& operator=(const CGetCommand& rhs);
  int operator==(const CGetCommand& rhs) const;
  int operator!=(const CGetCommand& rhs) const;
public:

  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);


};



#endif
