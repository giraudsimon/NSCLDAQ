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

#include <TCLInterpreter.h>
#include <TCLObject.h>
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
  std::string hexified;         // Bigendian.

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

};
