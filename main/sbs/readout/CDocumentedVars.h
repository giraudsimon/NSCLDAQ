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

#ifndef CDOCUMENTEDVARS_H
#define CDOCUMENTEDVARS_H

#include <TCLObjectProcessor.h>
#include <map>
#include <string>



class CTCLInterpreter;
class CTCLObject;
class CVarList;

/*!
   This class acutally handles a pair of commands;
   the statevar and runvar commands.  These commands are respsonsible for
   managing variables that will be written out in response to a few different trigger
   conditions.  The only difference between a statevariable and a runvariable is that
   state variables are write protected when the run is in progress.  This is done by
   establishing a write trace on the variable which rejects any variable modification
   when the run state is no inactive.  Note that state variables _can_ be modified,
   if they are linked to ordinary C/C++ variables.

*/
class CDocumentedVars : public CTCLObjectProcessor
{
public:
  typedef std::vector<std::pair<std::string, std::string> > NameValuePairs;
private:
  CVarList&               m_StateVariables;
  CVarList&               m_RunVariables;
  
  Tcl_Command             m_statevarToken;
  std::map<std::string, std::string*> m_PriorValues;
  
  // Canonicals:  The base class does not support deep canonicals so we can't either.

public:
  CDocumentedVars(CTCLInterpreter& interp);
  virtual ~CDocumentedVars();


private:
  CDocumentedVars(const CDocumentedVars& rhs);
  CDocumentedVars& operator=(const CDocumentedVars& rhs);
  int operator==(const CDocumentedVars& rhs) const;
  int operator!=(const CDocumentedVars& rhs) const;

  // Selectors that allow the production of state variable and run variable
  // events.

public:
  NameValuePairs getStateVars() const;
  NameValuePairs getRunVars()   const;
  
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

  // The following are common code shared by both of the commands:

private:

  int Create(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int Delete(CTCLInterpreter& interp,
	     std::vector<CTCLObject>& objv);
  int List(CTCLInterpreter& interp,
	   std::vector<CTCLObject>& objv);



  CVarList*  whichVariableList(std::vector<CTCLObject>& objv);
  void writeProtectVariable(std::string name);
  void unprotectVariable(std::string name);
  bool runActive();

  // Static member function that processes write protection:

  static char* writeProtectionTrace(ClientData       cd,
				    Tcl_Interp*      interp,
				    const char*       name1,
				    const char*       name2,
				    int               flags);

  static std::string usage(std::vector<CTCLObject>& objv);

  NameValuePairs getVars(CVarList& list) const;
  
};

#endif
