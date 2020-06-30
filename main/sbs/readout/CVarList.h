#ifndef CVARLIST_H
#define CVARLIST_H
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
#include <map>
#include <string>
#include <vector>
#include <TCLVariable.h>

class CTCLInterpreter;

/*!
   This class contains a map of Tcl variables and management
   functions for those variables.  Typically a varmap will be used
   to maintain a a set of variables that will be written to the output
   stream of the program to document the values of those variables at
   some chosen time.

   Remember when using this within Tcl threaded applications, the apartment
   model must be obeyed with respect to this map.

*/
class CVarList  // <final>
{
public:
  typedef std::map<std::string, CTCLVariable>  VariableMap;
  typedef VariableMap::iterator                 VariableIterator;
private:
  VariableMap      m_Variables;
  CTCLInterpreter* m_pInterpreter;

  // constructor and other canonicals.
  // note that since CTCLVariables are well behaved with respect to deep copies
  // all canonicals are ok.
public:
  CVarList(CTCLInterpreter& interp);
  ~CVarList();
  CVarList(const CVarList& rhs);
  CVarList& operator=(const CVarList& rhs);
  int operator==(const CVarList& rhs) const;
  int operator!=(const CVarList& rhs) const;

  // Manipulate the variable list:

  void add(std::string name);
  void remove(std::string name);
  
  // search and traverse:

  VariableIterator find(std::string name);
  VariableIterator begin();
  VariableIterator end();
  size_t           size();

  // Produce formatted output.
  // the formatted output is a vector of strings.  Each string a Tcl command
  // that will reproduce the value of the variable
  //

  std::vector<std::string> format();

};

#endif
