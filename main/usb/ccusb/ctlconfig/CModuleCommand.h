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

#ifndef CMODULECOMMAND_H
#define CMODULECOMMAND_H

#include <TCLObjectProcessor.h>

#include <vector>
#include <string>


class CTCLInterpreter;
class CTCLObject;
class CCtlConfiguration;
/*!
    Implements the module command.  This class is subclassed from 
    CTclObjectProcessor  The module command is the command that
    each setup script uses to define the set of contrrol modules
    that exist.  At present, only a single type of control module
    is supported: CJTECGDG  Jtec/Wiener Gate and Delay generator.
*/
class CModuleCommand : public CTCLObjectProcessor
{
  // local member data:
private:
  CCtlConfiguration&  m_config;
  
  // Canonical functions:

public:
  CModuleCommand(CTCLInterpreter& interp,
		 CCtlConfiguration&       server);
  virtual ~CModuleCommand();

private:
  CModuleCommand(const CModuleCommand& rhs);
  CModuleCommand& operator=(const CModuleCommand& rhs);
  int operator==(const CModuleCommand& rhs) const;
  int operator!=(const CModuleCommand& rhs) const;
public:

  // Virtual function overrides and implementations.
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);
private:
  int create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int configure(CTCLInterpreter& interp,
		std::vector<CTCLObject>& objv);
  int cget(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);
  


};


#endif
