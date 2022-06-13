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
#include "tclUtil.h"
#include "CReadoutModule.h"
#include "CConfiguration.h"
#include <XXUSBConfigurableObject.h>

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CReadoutModule.h>
#include <CConfiguration.h>

#include <tcl.h>


using std::vector;
using std::string;

namespace tclUtil {

  /*!  
    Set the result for a tcl interpreeter.
    \param interp : CTCLInterpreter
       The interpreter whose result will be set.
    \param msg : std::string
      The message to set.
  */
  void
  setResult(CTCLInterpreter& interp, string msg)
  {
    Tcl_Obj* pResult = Tcl_NewStringObj(msg.c_str(), -1);
    Tcl_SetObjResult(interp.getInterpreter(), pResult);
  }

  /*!
    Report a usage message through the interpreter result.
    The message will be of he form:
    \verbatim
      ERROR:msg\ncommand\nusage.
    \endverbatim
    \param interp : CTCLInterpreter&
       Reference to the interpreter whose result code will get set.
    \param msg : std::string
       msg in the above format.
    \param objv std::vector<CTCLObject>
      command words, used to reconstruct the failing command.
    \param usage : std::string
       usage string for the command that shows appropriate usage.
   */
  void
  Usage(CTCLInterpreter&    interp,
   string              msg,
   vector<CTCLObject>&  objv,
   string              usage)
  {
    string result("ERROR: ");
    result += msg;
    result += "\n";
    for (int i = 0; i < objv.size(); i++) {
      result += string(objv[i]);
      result += " ";
    }
    result += "\n";
    result += usage;
    setResult(interp, result);
  }
  /**
  * getTclTraceback
  *  daqdev/NSCLDAQ#1011 - Provide tracebacks on script errors.
  *    Return the Tcl error traceback. This can be used to obtain
  *    traceback information when reporting errors in script execution.
  *    This method depends on the global variable "errorInfo" containing
  *    the traceback information
  *
  *  @param interp  - The interpreter for which we want error traceback.
  *  @return std::string - the error traceback.
  */
 std::string
 getTclTraceback(CTCLInterpreter& interp)
 {
     const char* trace = Tcl_GetVar(
        interp.getInterpreter(), "errorInfo",  TCL_GLOBAL_ONLY
      );
     if (trace == nullptr) {
      trace = " - no traceback information available";
     }
     
     return std::string(trace);
 }
 /**
 * Generate a swig pointer from the C++ Pointer and its type.
 * This is of the form _address_p_typename
 * @param obj - pointer to the object.
 * @param type - Type name.
 *`
 * @return std::string
 */
std::string
swigPointer(void* p, std::string  type)
{

  char result [10000];
  std::string hexified;		// Bigendian.

  uint8_t* s = reinterpret_cast<uint8_t*>(&p); // Point to the bytes of the pointer

  // Note that doing the byte reversal this way should be
  // 64 bit clean..and in fact should work for any sized ptr.

  static const char hex[17] = "0123456789abcdef";
  register const unsigned char *u = (unsigned char *) &p;
  register const unsigned char *eu =  u + sizeof(void*);
  for (; u != eu; ++u) {
    register unsigned char uu = *u;
    hexified += hex[(uu & 0xf0) >> 4];
    hexified += hex[uu & 0xf];
  }

  sprintf(result, "_%s_p_%s", hexified.c_str(), type.c_str());

  return std::string(result);


}
/**
 * getModule
 *    Locates a module by name along with a pile of other checking.
 *  @param config - the configuration that has the module.
 *  @param interp - The interpreter running the command that wants to know.
 *  @param objv   - The command words for the command being executed.
 *  @param predicate - Condition that must be false to have enough objv elements.
 *  @return CReadoutModule*
 *  @retval nullptr - if the module could not be fetched for any reason.
 *  @note The form of the command being processed is:
 *         type subcommand name ...
 *         Where:
 *         - type - is the command which is the module type.
 *         - subcommand - is the subcommand (e.g. config).
 *         - name - is the name of the module being operated on.
 */
CReadoutModule*
getModule(
  CConfiguration& config,
  CTCLInterpreter& interp, std::vector<CTCLObject>& objv, bool predicate
)
{
  // Valid number of parameters?
  if(predicate) {
    std::string type = objv[0];
    std::string sc  = objv[1];
    std::string msg = "Incorrect number of commands for: ";
    msg += type; msg+= " ";
    msg += sc;
    interp.setResult(msg);
    return nullptr;
  }
  // Try to find it:

  string          name     = objv[2];
  CReadoutModule* pModule  = config.findAdc(name);
  if(!pModule) {
    std::string msg = objv[0];
    msg += " named ";
    msg += name;
    msg += " does not exist.";
    interp.setResult(msg);
    return nullptr;
  }
  return pModule;
}
/**
 * listConfig
 *   Set the interpreter result with the configuration of a
 *   readout module.
 * @param interp - intepreter who's result is set.
 * @param pModule - pointer to module that will be listed.
 */
void
listConfig(CTCLInterpreter& interp, CReadoutModule* pModule)
{
  auto config = pModule->cget();

  Tcl_Obj* pResult = Tcl_NewListObj(0, NULL);


  for (int i =0; i < config.size(); i++) {
    Tcl_Obj* key   = Tcl_NewStringObj(config[i].first.c_str(), -1);
    Tcl_Obj* value = Tcl_NewStringObj(config[i].second.c_str(), -1);

    Tcl_Obj* sublist[2] = {key, value};
    Tcl_Obj* sl = Tcl_NewListObj(2, sublist);
    Tcl_ListObjAppendElement(interp.getInterpreter(), pResult, sl);
  }
  Tcl_SetObjResult(interp.getInterpreter(), pResult);  
}
/**
 * newName
 *    Return a name for a new adc module or else set an error.
 * @param inter - the interpreter.
 * @param pConfig - pointer to the current configuration object.
 * @param objv    - Command line parameter.
 * @return std::string
 * @retval "" - Error string is in result, report an error.
 */
std::string
newName(CTCLInterpreter& interp, CConfiguration* pConfig, std::vector<CTCLObject>& objv)
{
  if (objv.size() < 3) {
    interp.setResult("Incorrect parameter count for create subcommand");
    return "";
  }

  // Get the chain name.  This must not be the name of an existing 'adc' module.

  std::string   name    = objv[2];

  if (pConfig->findAdc(name)) {
    std::string result("Duplicate module creation attempted - ");
    result += name;
    interp.setResult(result);
    return "";
  }
  return name;
  
}

};
