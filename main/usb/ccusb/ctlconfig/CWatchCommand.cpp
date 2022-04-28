/*
     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansi
*/

#include "CWatchCommand.h"

#include "CCtlConfiguration.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>

#include <algorithm>

/**
 *  The set of variables modified are accumulated in this 
 *  set.  The monitor loop will normally just 
 *  get the elements of this set periodically and 
 *  generate the output buffer from that.
 */
std::set<CWatchCommand::TCLVariableName> CWatchCommand::m_ModifiedVariables;

/**
 * The set of variables that are being watched is accumulated in another set:
 */

std::set<std::string> CWatchCommand::m_WatchedVariables;


/*----------------------------------------------------------------------------
 * CWatchCommand::CVisitor - interior private class.
 */

/**
 * CWatchCommand::CVisitor::CVisitor(CTCLInterpreter& interp)
 *
 *  Construct the object.  This just saves the interpreter so that it is
 *  available at each visit of an element.
 *
 * @param interp - Reference to an encapsulated Tcl interpreter object that
 *                 is used to:
 *                 - Determine the existence of a variable.
 *                 - Enumerate array indices if the variable is an array.
 */
CWatchCommand::CVisitor::CVisitor(CTCLInterpreter& interp) : 
  m_interp(interp)
{}

/**
 * CWatchCommand::CVisitor::get() const
 *
 *  Return the final vector of variable names to the caller.
 * 
 * @return std::vector<CWatchCommand::TCLVariableName> 
 * @retval vector of variable names and indices of defined watched variables.
 */
std::vector<CWatchCommand::TCLVariableName>
CWatchCommand::CVisitor::get() const
{
  return m_variables;
}
/**
 * void operator()(std::string varName)
 *
 *   The actual visitor function.  varName is checked for existence via 
 *   [info exists].   If it exists it is checked for the existence of 
 *    indices as an array via [array names].  If it is a scaler, a single
 *   entry is created in m_variables.  If an array, an entry is made for
 *   each index.
 *
 * @param varName - The name of a watched variable.
 */
void
CWatchCommand::CVisitor::operator()(std::string varName)
{
  // Build/exists a Tcl command to see if the variable exists:

  std::string existsCommand = "info exists ";
  existsCommand            += varName;
  std::string exists        = m_interp.GlobalEval(existsCommand);
  

  if (exists == "1") {

    // See if there are any indices:

    std::string namesCommand = "array names ";
    namesCommand            += varName;
    std::string nameString   = m_interp.GlobalEval(namesCommand);

    CTCLObject indexList;
    indexList.Bind(m_interp);
    indexList = nameString;
    if (indexList.llength() == 0) {
      // scaler variable.
      
      CWatchCommand::TCLVariableName name(varName, "");
      m_variables.push_back(name);
     
    } else {
      // array variable:

      std::vector<CTCLObject> indices = indexList.getListElements();
      for (int i =0; i < indices.size(); i++) {
	CTCLObject index = indices[i];
	index.Bind(m_interp);
	CWatchCommand::TCLVariableName name(varName, (std::string)(index));
	m_variables.push_back(name);
      }

    }
  }
}


/*----------------------------------------------------------------------------
** Canonical methods:
*/

/**
 * CWatchCommand::CWatchCommand(CTCLInterpreter& interp, CCtlConfiguration& server)
 *
 * Construct the object.  This means creating and registering us as a
 * command on the specified interpreter.
 *
 * @param interp - Interpreter on which we shouildb e registered.
 */
CWatchCommand::CWatchCommand(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "watch", true)
{}

/**
 * CWatchCommand::~CWatchCommand()
 *
 * A no-op.
 */
CWatchCommand::~CWatchCommand() {

}
/*---------------------------------------------------------------------------
 * Accessor methods - Provide controlled access to attributes.
 */

/**
 * CWatchCommand::getModifications()
 *
 * Returns the variables that have been modified since the last
 * call to clearModifications below.  
 *
 * @return std::vector<TCLVariableName>
 * @retval  The elements of the returned vectors pairs.  The first item in each pair
 *          is always non-empty and is the name of the variable.  If the second
 *          item of the pair is non empty, the variable is an array and the
 *          second item is the specific array index of the variable being
 *          modified, if empty, the first item names a scalar variable.
 *
 * Implicits:
 *    m_ModifiedVariables   - read access.
 *
 */
std::vector<CWatchCommand::TCLVariableName> 
CWatchCommand::getModifications()
{
  std::vector<TCLVariableName> result;
  for (std::set<TCLVariableName>::iterator p = m_ModifiedVariables.begin(); 
       p != m_ModifiedVariables.end(); p++) {
    result.push_back(*p);
  }
  return result;
}

/**
 * CWatchCommand::clearModifications()
 *
 * Clear the modification set, making it totally empty.
 *
 * Implicits:
 *   m_ModifiedVariables - write access.
 *
 */
void
CWatchCommand::clearModifications()
{
  m_ModifiedVariables.clear();

}
/**
 * getWatchedVariables(CTCLInterpreter& interp)
 *
 *  Return the set of variables that are being watched.  If a variable is an
 *  array, all currently defined array elements are returned.
 *  This is intended to allow clients to do an initial variable dump
 *  of everyting at the beginning of a run.
 *
 * @param interp - Reference to an encapsulated interpreter used to look up 
 *                 the array elements.  Note that there is an assumption
 *                 this method is being called when the interpreter is at
 *                 global scope.
 *
 * @return std::vector<CWatchCommand::TCLVariableName>
 *   Array of watched variables that have been defined (undefined variables
 *   will have been filtered out of this list).
 *
 */
std::vector<CWatchCommand::TCLVariableName>
CWatchCommand::getWatchedVariables(CTCLInterpreter& interp)
{
  CVisitor visitor(interp);
  CVisitor& visref(visitor);	// Need ref or else different object at each iteration.

  std::for_each(m_WatchedVariables.begin(), m_WatchedVariables.end(), visref);
  return visitor.get();
}
/*-----------------------------------------------------------------------
*   Virtual function overrides:
*/


/**
 * operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
 * 
 *  Add a variable to the watched variable list.
 *  This means adding a Tcl trace on the variable named.
 *
 * @param interp - Interpreter on which the command is running.
 * @param objv   - vector of encpasulated Tcl_Obj's that make up the command.
 *
 * @return int
 * @retval TCL_OK    - Successful completion.
 * @retval TCL_ERROR - The command failed.  The result is a string that
 *                     describes why.
 */ 
int
CWatchCommand::operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv)
{
  // Require exactly two command line parameters:

  if(objv.size() != 2) {
    std::string error = "Incorrect number of command parameters\n";
    error            += usage();
    interp.setResult(error);
    return TCL_ERROR;
  }

  // Obtain the name of the variable:

  objv[1].Bind(interp);
  std::string varName = (std::string)objv[1]; // The cast deals with selecting string vs const char*

  // If the variable is already being watched that's an error too:

  if (m_WatchedVariables.find(varName) != m_WatchedVariables.end()) {
    std::string error =  "The variable: ";
    error            +=  varName;
    error            +=  " is already being watched";
    interp.setResult(error);
    return TCL_ERROR;
  }


  // Attempt to add the trace:

  int status = Tcl_TraceVar(interp.getInterpreter(), varName.c_str(),
			    TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
			    CWatchCommand::traceRelay, reinterpret_cast<ClientData>(this));

  if (status == TCL_OK) {
    m_WatchedVariables.insert(varName);
  }

  // If the status is not TCL_OK, the result has already been set by Tcl_TraceVar.

  return status;
}

/*-------------------------------------------------------------------------
 * Private utility function.
 */

/**
 * usage() const
 * 
 * Returns the correct command usage string.
 *
 * @return std::string
 */

std::string
CWatchCommand::usage() const
{
  std::string result = "watch variable\n";
  result            += "    variable - the name of a variable, array or array element\n";
  result            += "               to watch.";

  return result;
}

/**
 * logModification(CTCLInterpreter* pInterp, char* name1, char* name2, int flags)
 *
 * Invoked from the traceRelay which in turn is connected to Tcl variable traces.
 * created via the watch command:
 * - ensure this is a write (flags | TCL_TRACE_WRITES != 0).
 * - Create the TCLVariableName object from name1, name2.
 * - insert the reslting item in the m_ModifiedVariables set.
 * - Return NULL to satisfy the needs of Tcl traces.
 *
 * @param pInterp - Pointer; to the encapsulated Tcl Interpreter object that
 *                  fired the trace.
 * @param name1   - The base name of the variable which fired the trace.
 * @param name2   - If name1 was an array name, the index of the array being written to.
 * @param flags   - Describe the operation that is being traced.
 *
 * @return char*
 * @retval NULL
 */

char*
CWatchCommand::logModification(CTCLInterpreter* pInterp, 
			       const char* name1, const char* name2, int flags)
{
  if (flags | TCL_TRACE_WRITES) {
    std::string baseName = name1;
    std::string index    = name2 ? name2 : ""; // map null to an empty string.
    TCLVariableName nameDesc(baseName, index);
    m_ModifiedVariables.insert(nameDesc);
  }
  return NULL;
}
/**
 * traceRelay(ClientData object, Tcl_Interp* pInterp, char* name1, char* name2, int flags)
 *
 * Static member that is the target of the trace operation.
 * The purpose of this method is to transition to object context so that
 * logModification can be invokec.
 *
 * @param object -   Actually a pointer to the CWatchCommand object
 *                   that established the trace.
 * @param pInterp-   Pointer to the Tcl Interpreter that fired the trace.
 * @param name1   - The base name of the variable which fired the trace.
 * @param name2   - If name1 was an array name, the index of the array being written to.
 * @param flags   - Describe the operation that is being traced.
 *
 * @return char*
 * @retval - whatever logModification returns.
 */
char*
CWatchCommand::traceRelay(ClientData object, Tcl_Interp* pInterp, const char* name1, const char* name2, int flags)
{
  CTCLInterpreter interp(pInterp);
  CWatchCommand* pObject = reinterpret_cast<CWatchCommand*>(object);

  return pObject->logModification(&interp, name1, name2, flags);
  
}
