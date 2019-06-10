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
#include <config.h>
#include "CVarList.h"
#include <CDuplicateSingleton.h>
#include <CNoSuchObjectException.h>
#include <TCLList.h>

using namespace std;
/*!
   Construction is pretty much null:
*/
CVarList::CVarList(CTCLInterpreter& interp) :
  m_pInterpreter(&interp)
{}
/*!
  As is destruction:
*/
CVarList::~CVarList() {}
/*!
   Copy construction is a deep construction:
*/
CVarList::CVarList(const CVarList& rhs) 
{
  m_Variables = rhs.m_Variables;
  m_pInterpreter   = rhs.m_pInterpreter;
}
/*!
  Assignment is the same as copy construction except that:
  - It is proteced from self copy.
  - It retursn *this
*/
CVarList&
CVarList::operator=(const CVarList& rhs)
{
  if (this != &rhs) {
    m_Variables = rhs.m_Variables;
    m_pInterpreter = rhs.m_pInterpreter;
  }

  return *this;
}
/*!
   Comparison relies on fact that the list has an operator== and that
   each element has an operator==.
*/
int
CVarList::operator==(const CVarList& rhs) const
{
  return (m_Variables == rhs.m_Variables)    &&
         (m_pInterpreter  == (rhs.m_pInterpreter));
}
/*!
   And inequality is always the logical inverse from ==
*/
int
CVarList::operator!=(const CVarList& rhs) const
{
  return !(*this == rhs);
}

/*!
   Add an item to the variable list.  It is an error to add the same item
   twice. A CDuplicateSingleton will be thrown in that event.
   \param name  - Name of the variable to add.
*/
void
CVarList::add(string name)
{
  if (find(name) == end()) {
    CTCLVariable var(m_pInterpreter, name, false);
    m_Variables[name]  = var;
  }
  else {
    throw CDuplicateSingleton("Attempted add of a duplicate variable to a variable list",
			      name);
  }
}
/*!
   Remove an item from the variable list.  If the item does not exist,
   a CNoSuchObjectException will be thrown.

   \param name - Name of the variable to remove.
*/
void
CVarList::remove(string name)
{
  VariableIterator i = find(name);
  if (i != end()) {
    m_Variables.erase(i);
  }
  else {
    throw CNoSuchObjectException("Attempting to remove variable from list", name);
  }
}

/*!
   Locates a variable and returns an iterator that allows you to reference it.
   \param name - name of the variable to find.
   \return CVarList::VariableIterator
   \retval  This is a pair.  The first item is a 'pointer' to the name,
   the second itema pointer to the CTCLVariable object.
   \retval end() if the object is not found.
*/
CVarList::VariableIterator
CVarList::find(string name)
{
  return m_Variables.find(name);
}

/*!
   Return a begin iterator.  Each iterator is a pair. The first element of the pair
   'points' to the name, while the second 'points' to the CTCLVariable of that name.
*/
CVarList::VariableIterator
CVarList::begin()
{
  return m_Variables.begin();
}
/*!
  Returns an end of iteration iterator.  Note that this iterator is a sentinell that
allows one to know when an iterator has run off the end of the collection.
*/
CVarList::VariableIterator
CVarList::end()
{
  return m_Variables.end();
}



/*!
  \return size_t
  \retval number of elements in the list.
*/
size_t
CVarList::size()
{
  return m_Variables.size();
}

/*!
   Return a vector of strings.  Each string is a Tcl command that
   sets one of the variables to its current value.
   - The entire collection of strings sets all variables to their current values
   - If a variable does not have a value, it is set to --Not Set--.
*/
vector<string>
CVarList::format()
{
  vector<string> result;
  VariableIterator i = begin();

  while(i != end()) {

    string name       = i->first;
    const char* value = i->second.Get();
    if (!value) {
      value = "--Not Set--";
    }
    // TCL commands are lists so this chunk of code gets the quoting and special
    // character handling right:

       
    StringArray words;
    words.push_back("set");
    words.push_back(name);
    words.push_back(value);

    CTCLList command(m_pInterpreter);
    command.Merge(words);
    result.push_back(command.getList());

    i++;
  }
  return result;
}

