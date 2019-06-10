/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <tcl.h>
#include "TCLInterpreter.h"
#include "CImmediateListCommand.h"
#include <string>


/**
 * @file CVMUSBPackage.cpp
 * 
 * Provides package initialization code for the vmusb package.
 * - initializes the vmusb namespace.
 * - adds the commands in it.
 */


/**
 * Vmusb_Init
 *
 * Initialize the package in an interpreter.
 * - Advertise the package.
 * - Load the commands into the interpreter.
 *
 *
 * @param pInterp - Tcl_Interp* that's loadig the package.
 * @return int
 * @retval TCL_OK - success.
 * @retval TCL_ERROR - failure with message in result.
 */ 
extern "C" {
  int Vmusbserver_Init(Tcl_Interp* pInterp)
  {
    int status;

    // Advertise the package.

    if((status = Tcl_PkgProvide(pInterp, "vmusbservero", "1.0")) != TCL_OK)  {
      return status;
    }

    // Add the commands:

    CTCLInterpreter*  pEncapsulatedInterp = new CTCLInterpreter(pInterp);
    CTCLInterpreter& rInterp(*pEncapsulatedInterp);

    try {
      new CImmediateListCommand(rInterp); // For now just use the first.
    }
    catch (std::string msg) {
      rInterp.setResult(msg);
      return TCL_ERROR;
    }
    catch (const char* msg) {
      rInterp.setResult(msg);
      return TCL_ERROR;
    }
    catch (...) {
      rInterp.setResult(
         "Unanticipated exception type caught initializing vmusb package"
			);
      return TCL_ERROR;
    }

    return TCL_OK;
  }
}


/**
 * Vmusb_SafeInit 
 *
 * Initialize the package in a safe interpreter.  At this point
 * this is the same as initializing in a  normal interp:
 *
 * @param pInterp - Tcl_Interp* that's loadig the package.
 * @return int
 * @retval TCL_OK - success.
 * @retval TCL_ERROR - failure with message in result.
 */ 
extern "C" {
  int Vmusb_SafeInit(Tcl_Interp* pInterp) 
  {
    return Vmusbserver_Init(pInterp);
  }
}
