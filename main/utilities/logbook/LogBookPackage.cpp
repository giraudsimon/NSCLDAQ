/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  LogBookPackage.cpp
 *  @brief: Package registrationfor the Tcl bindings package for logbook.
 */
#include <tcl.h>
#include <TCLInterpreter.h>
#include "TclLogbook.h"


static const char* version="1.0";

/**
 * Package must be etern "C" as specific names are assumed to be where
 * the package initialization entry point is.
 */
extern "C" {
   int Tcllogbook_Init(Tcl_Interp* interp)
   {
        int status;
        status = Tcl_PkgProvide(interp, "logbook", version);
        if (status != TCL_OK) return status;
        
        // All the commands will fit into the logbook namespace:
        
        Tcl_Namespace* pNamespace =
            Tcl_CreateNamespace(interp, "::logbook", nullptr, nullptr);
         if (!pNamespace) return TCL_ERROR;
        
        CTCLInterpreter* pInterp = new CTCLInterpreter(interp);
        
        // Register package commands here.
        
        new TclLogbook(pInterp, "::logbook::logbook");
        
        return TCL_OK;
   }
}