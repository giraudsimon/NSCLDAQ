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
#include "CDocumentedVars.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLVariable.h>
#include "CVarList.h"
#include "RunState.h"

#include <string.h>
#include <tcl.h>
#include <assert.h>

using namespace std;

/*!
   Construct the command object.  The command object is a bit strange because
   it registers itself for 2 commands; 'runvar' and 'statevar'.

   @param interp - The interpreter on which the object will be registered.
*/
CDocumentedVars::CDocumentedVars(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "runvar", true),
  m_StateVariables(*(new CVarList(interp))),
  m_RunVariables(*(new CVarList(interp)))
{
  m_statevarToken = RegisterAs("statevar");
}
/*!
   Destruction involves unregistering the 'statevar' binding ('runvar' is automatically
   unregistered.  In addtion:
   - Strings in the m_PriorValues map are destroyed and corresponding traces removed.
   - m_StateVariables is released.
   - m_runVaribles are released.
*/
CDocumentedVars::~CDocumentedVars()
{

  // Unregister traces and.
  // deallocate string pointers.  The map itself gets destroyed automatically.
  
  while (! m_PriorValues.empty()) {
    map<string, string*>::iterator i = m_PriorValues.begin();
    string name = i->first;
    unprotectVariable(name);	// Deletes the storage and entry.
  }

  unregisterAs(m_statevarToken);


  // Delete the variable lists:

  delete &m_StateVariables;
  delete &m_RunVariables;
}

/*!
   Return the state variables as a vector of name value pairs.
   
   @return CDocumentedVars::NameValuePairs (vector<pair<string, string> >)
   @retval Each element of the return vector is a pair. The first element
           of each pair is the name of a state variable. The second element
	   it's value.  If the variable has not yet been set it will have a value
	   *Not Set*
*/
CDocumentedVars::NameValuePairs
CDocumentedVars::getStateVars() const
{
  return getVars(m_StateVariables);
}

/*!
  Same as getStateVars but gets the run variables instead.
*/
CDocumentedVars::NameValuePairs
CDocumentedVars::getRunVars() const
{
  return getVars(m_RunVariables);
}

/*!
   This function takes over when either the runvar or the statevar command
   is input.  The commands have the following form:
\verbatim

command ?-create? name ?initialvalue?
command -delete name
command -list ?pattern?

\endverbatim

   With a few minor differences processing the runvar and statevar commands
   are identical, hence the merger of those commands into this module.

   @param interp   - The interpreter that's executing this command.
   @param objv     - Vector of wrapped Tcl_Obj's that are the command words.
                     note that objv[0] is the command name itself.
   @return int
   @retval TCL_OK  - Everything worked correctly.
   @retval TCL_ERROR - Something failed.

   @note The Tcl result string will contain something that depends on the 
         actual subcommand and the status of the command completion.

*/
int
CDocumentedVars::operator()(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{

  // Things are so much simpler if we bind the objv elements now:
  //
  for (int i=0; i < objv.size(); i++) {
    objv[i].Bind(interp);
  }

  // Each command requires a minimum of one word other than the command itself.
  // this allows for the subcommand, or new variable name in case -create is
  // implicit.

  string command = objv[0];
  if (objv.size() < 2) {
    string result;
    result   += "Insufficient number of command parameters\n";
    result   += usage(objv);
    interp.setResult(result);
    return TCL_ERROR;
  }

  // Get the subcommand keyword.  If the subcommand keyword and use the if-chain below
  // to branch out.
  
  string subcommand = objv[1];
  if (subcommand == "-delete") {
    return Delete(interp, objv);
  }
  if (subcommand == "-list") {
    return List(interp, objv);
  }
  // Must be -create.. explicit or implicit.

  return Create(interp, objv);

}
/*!
 *  List a set of variables whose names match a specific optional pattern.
 *  The command keywords determines which of the variable lists is listed.
 *  @param interp  - Encapsulation of the interpreter that's running this command.
 *  @param objv    - Vector of encapsulated command Tcl_Obj's.
 *  @return int
 *  @retval TCL_OK - function succeeded the result string is the list of variables that
 *                   match the pattern.
 *  @retval TCL_ERROR - function failed, the result string is the error message 
 *                      associated with that failure.
 */
int
CDocumentedVars::List(CTCLInterpreter& interp,
		      std::vector<CTCLObject>& objv)
{
  string pattern = "*";		// The default pattern matches everything.
  if(objv.size() > 3) {		// Too many keywords
    string result;
    result += "Too many command parameters for the 'list' keyword\n";
    result += usage(objv);
    interp.setResult(result);
    return TCL_ERROR;
  }

  if (objv.size() == 3) {
    pattern = string(objv[2]); // User provided a match pattern.
  }

  CVarList& theList(*whichVariableList(objv));
  CVarList::VariableIterator i = theList.begin();
  CTCLObject result;
  result.Bind(interp);  
  while(i != theList.end()) {

    string name = i->first;
    if (Tcl_StringMatch(name.c_str(), pattern.c_str())) {
      result += name;
    }
    
    i++;
  }
  interp.setResult(result);
  return TCL_OK;
}
/*! Deletes an existing run or state variable. Note that state variables cannot be deleted
 * while a run is in progress. Assuming all is legal:
 * - Removes the variable from the appropriate list.
 * - If the variable was a run variable remores it fromt he prior values map.
 * - If the variable was a run variable, removes the write trace that was set on it.
 *
 *  @param interp  - Encapsulation of the interpreter that's running this command.
 *  @param objv    - Vector of encapsulated command Tcl_Obj's.
 *  @return int
 *  @retval TCL_OK - The delete succeeded. In this case, the Tcl result string is empty.
 *  @retval TCL_ERROR - The delate failed and the result string dscribes why.
 */
int
CDocumentedVars::Delete(CTCLInterpreter& interp,
			std::vector<CTCLObject>& objv)
{
  // We need exactly 3 parameters the command word, the -delete switch and the
  // name of the var to operate on.

  if (objv.size() != 3) {
    string result;
    result += "Incorrect number of parameters\n";
    result += usage(objv);
    interp.setResult(result);
    return TCL_ERROR;
  }

  //  Get command name, variable name and list to operate on:

  string    command = objv[0];
  string    varname = objv[2];
  CVarList* pList   = whichVariableList(objv);

  // If the variable does not exist in the list, nothing else matters:

  CVarList::VariableIterator p = pList->find(varname);
  if (p == pList->end()) {
    string result;
    result += command;
    result += ": Variable ";
    result += varname;
    result += " Does not exist";
    interp.setResult(result);

    return TCL_ERROR;
  }
  // If the command was a runvar, the run must be inactive:

  if ((command == "runvar") && runActive()) {
    string result;
    result += "runvar variables can only be deleted when the run is inactive";
    interp.setResult(result);
    
    return TCL_ERROR;
  }

  // Set about deleting the stuff we need to delete.

  pList->remove(varname);		// kill from the variable list.
  if (command == "runvar") {

    unprotectVariable(varname);
  }

  return TCL_OK;
}

/*!
 * Create a new variable.  This is complicated a bit, but not much by the fact that the
 * variable name could be the second or third command word.  The following restrictions must
 * be applied:
 *  - The variable cannot already exist in either list; A variable cannot be both a runvar
 *    and a statevar at the same time.
 *  - If a runvar is being created, the run must not be in progress.
 * The new variable is
 *  - entered in the appropriate list.
 *  - If a runvar, the variable's current value is gotten and entered in 
 *    the prior value list.
 *  - If a runvar the variable has a trace set on it.
 *
 *  @param interp  - The interpreter that's running the commande
 *  @param objv    - Vector of command words.
 *  @return int
 *  @retval TCL_OK - If the command succeeds.  result is empty.
 *  @retval TCL_ERROR - If the command fails.  result is an error message.
 */
int
CDocumentedVars::Create(CTCLInterpreter& interp,
			std::vector<CTCLObject>& objv)
{
  string command = objv[0];
  
  // If -create is present there must be 3 or four parameters
  // if not 2 or three.

  int minparam = 3;
  int maxparam = 4;
  int nameindex= 2;
  if (string(objv[1]) != string("-create")) {
    minparam--;
    maxparam--;
    nameindex--;
  }

  // nameindex  - index of name parameter.
  // minparam   - minimum require number of parameters.
  // maxparam   - Maximum number of allowed parameters.

  if ((objv.size() < minparam) || (objv.size() > maxparam)) {
    string result;
    result += "Invalid number of command line parameters\n";
    result += usage(objv);
    interp.setResult(result);
    return TCL_ERROR;
  }
  // Pull out the name of the variable being created.

  string varname = objv[nameindex];

  // The name can't be in either list... and get the list to act on.
  //

  if ( (m_StateVariables.find(varname) != m_StateVariables.end())  ||
       (m_RunVariables.find(varname)   != m_RunVariables.end())) {
    string result;
    result += "The variable ";
    result += varname;
    result += " is already either a statevar or a runvar, and cannot be redefined\n";
    result += "Either remove it from one of the variable list or choose another name";
    interp.setResult(result);
    return TCL_ERROR;
  }
  CVarList* pTheList = whichVariableList(objv);
  
  // If this is a runvar, we must have a halted run:

  if ((command == "runvar") && runActive()) {
    string result;
    result += "runvar variables can only be created when the run is inactive.";
    interp.setResult(result);
    return TCL_ERROR;
  }

  // Give the variable an initial value if necessary:

  if (objv.size() == maxparam) {
    CTCLVariable newVariable(&interp, varname, false);
    newVariable.Set(string(objv[nameindex+1]).c_str());
  }

  // Add the variable to the appropriate list and if necessary set a trace on it
  // to writeprotect it when the run goes active.

  pTheList->add(varname);
  if(command == "runvar") {
    writeProtectVariable(varname);
  }

  interp.setResult(varname);

  return TCL_OK;

}
/*--------------------------------------------------------------------------*/

/*
** Return a pointer to the correct variable list depending on the command name.
** if the command name does not match a NULL Is returned.
*/
CVarList*
CDocumentedVars::whichVariableList(std::vector<CTCLObject>& objv)
{
  string command(objv[0]);
  if (command == "runvar") {
    return &m_RunVariables;
  }
  if (command == "statevar") {
    return &m_StateVariables;
  }
  return reinterpret_cast<CVarList*>(0);
}

/*
 * Protect a variable by establishing a write trace on it.  If the run is active,
 * the trace will prevent the variable from being written.  A prerequisite for this is to
 * know the prior value of the variable.  Note that the variable may not have a value yet.
 * prior values are maintained in the m_priorValues map.
 */
void
CDocumentedVars::writeProtectVariable(string name)
{
  // Get the prior value, save it in the map.. vars without a value are given a null pointer.

  CTCLVariable var(getInterpreter(), name, false);
  const char* value = var.Get(TCL_GLOBAL_ONLY);
  string*    pValue(0);
  if (value) {
    pValue = new string(value);
  }
  m_PriorValues[name] = pValue;

  // Set the trace to writeProtectionTrace with this as the client data:

  Tcl_Interp* interp = getInterpreter()->getInterpreter();
  Tcl_TraceVar(interp, name.c_str(), 
	       TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY |TCL_TRACE_RESULT_DYNAMIC,
	       writeProtectionTrace, reinterpret_cast<ClientData>(this));
  
}
/*
 *  Remove the trace on a variable... we also remove the entry from the prior values map.
 */
void
CDocumentedVars::unprotectVariable(string name)
{
  Tcl_Interp* interp = getInterpreter()->getInterpreter();
  Tcl_UntraceVar(interp, name.c_str(),	       
		 TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY |TCL_TRACE_RESULT_DYNAMIC,
		 writeProtectionTrace, reinterpret_cast<ClientData>(this));

  map<string, string*>::iterator p = m_PriorValues.find(name);
  if (p != m_PriorValues.end()) {
    delete p->second;		// free the string.
    m_PriorValues.erase(p);
  }
  else {
    // Should not happen!!
    
    assert(p != m_PriorValues.end());
    
  }
		 
}

/*
 * Returns true if the run is not inactive.
 */
bool
CDocumentedVars::runActive()
{
  RunState* pState = RunState::getInstance();

  return (pState->m_state != RunState::inactive);
}

/*
** Returnthe command usage.  Note that objv is used to get the command name:
*/
string
CDocumentedVars::usage(vector<CTCLObject>& objv)
{
  string command = objv[0];
  string result;
  result   = "Usage:\n";
  result  += "  ";
  result  += command + " ?-create? varname ?initial-value?\n";
  result  += command + " -delete varname\n";
  result  += command + " -list ?glob-pattern?";

  return result;
}

/*
** This function gains control when a variable trace fires.
** If the variable is being deleted, resources associated with the variable
** trace are released.  Otherwise.
** - If the run is inactive nothing happens.
** - If the run is active the value of the variable is restored from the prior value
**   cache and an error is returned.
**
** There is one wrinkle.  If an unset variable is set, we unset it again rather 
** than restoring it.
**
** Parameters:
**  ClientData    cd     - Actually a pointer to the CDocumentedVars object that 
**                         established the trace.
**  Tcl_Interp*   interp - Interpreter that is firing the trace.  Note this is not
**                         encapsulated.
**  const char*   name1 -  Name of the variable being traced.
**  const char*   name2 -  NULL given how we establish the traces.
**  int           flags -  Flags that describe the trace as follows (just the ones we
**                         care about:
**                         TCL_TRACE_WRITES   - The trace is due to a write.
**                         TCL_TRACE_UNSETS   - The trace is due to an unset.
**                         TCL_TRACE_DESTROYED - Trace is being destroyed.
**                         TCL_INTERP_DESTROYED- Interpreter is being destroyed.
** Returns:
**    NULL               - It's ok to do the write.
**    char* somethign    - Pointer to dynamically allocated storage (via Tcl_Alloc)
**                         containing an error string when the write is disallowed.
*/
char*
CDocumentedVars::writeProtectionTrace(ClientData       cd,
				      Tcl_Interp*      interp,
				      const char*       name1,
				      const char*       name2,
				      int               flags)
{
  CDocumentedVars& o(*(reinterpret_cast<CDocumentedVars*>(cd)));

  // nothing to do at all if we don't have an entry for the variable:

  std::map<string,string*>::iterator p = o.m_PriorValues.find(name1);
  if (p == o.m_PriorValues.end()) {
    return reinterpret_cast<char*>(NULL);
  }
  


  // If the interpreter is being destroyed just kill off the
  // entry for this var in m_PriorValues and return null:

  if (flags &  TCL_INTERP_DESTROYED) {
    o.m_PriorValues.erase(p);
    return reinterpret_cast<char*>(NULL);
  }

  // If the run is active we must write protect the varriable
  // otherwise if the trace is an unset we must clean up.
  //
  if (o.runActive()) {

    // writes and unsets have the same code really.. since both restore the
    // prior value.  Note that if the variable's prior state was unset, we
    // just unset it.

    string* prior = p->second;
    if (prior) {
      // have a prior value to set:

      Tcl_SetVar(interp, name1, prior->c_str(), TCL_GLOBAL_ONLY);
    }
    else {
      // No prior value.. unset:

      Tcl_UnsetVar(interp, name1, TCL_GLOBAL_ONLY);
    }
    // However if the trace is being destroyed we need to re-assert it:

    if (flags & TCL_TRACE_DESTROYED) {
      Tcl_TraceVar(interp, name1,
		   TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY |TCL_TRACE_RESULT_DYNAMIC, 
		   CDocumentedVars::writeProtectionTrace, cd);
    }

    // Make an nice descriptive error message:

    string result;
    result +=  "Attempted to write or unset: ";
    result += name1;
    result += " which cannot be modified while the run is active";
    char* pResult = Tcl_Alloc(result.size() +1);
    strcpy(pResult, result.c_str());
    return pResult;

  }
  else if (flags & TCL_TRACE_UNSETS) {
    o.m_PriorValues[name1] = reinterpret_cast<string*>(NULL); // Not set.
    return reinterpret_cast<char*>(NULL);
  }
  else {
    if (o.m_PriorValues[name1]) {
      delete o.m_PriorValues[name1];
      o.m_PriorValues[name1] = reinterpret_cast<string*>(NULL);
    }
    o.m_PriorValues[name1] = new string(Tcl_GetVar(interp, name1, TCL_GLOBAL_ONLY));
    return reinterpret_cast<char*>(NULL);
  } 
}

/*
** Return a variable list as a set of name value pairs.  If the value is not
** defined the value *Not Set* will be used.
*/
CDocumentedVars::NameValuePairs
CDocumentedVars::getVars(CVarList& list) const
{
  CVarList::VariableIterator p;
  NameValuePairs             result;

  for (p =  list.begin(); p != list.end(); p++) {
    string        name  = p->first;
    const char*  value = p->second.Get();
    if (!value) {
      value = "*Not Set*";
    }
    pair<string, string> item(name, string(value));
    result.push_back(item);
  }

  return result;
}
