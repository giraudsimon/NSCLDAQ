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
#include <tcl.h>
#include <TCLInterpreter.h>

#include "CRingCommand.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

static const char* version = "1.1";

/* Provide the ring package that provides Tcl access to ring buffers. */

extern "C" {
  int Ringbuffer_Init(Tcl_Interp* pInterp) 
  {
    Tcl_PkgProvide(pInterp, "ring", version );


    // Wrap the interpreter in an interpreter object and 
    // create the commands.

    CTCLInterpreter& interp(*(new CTCLInterpreter(pInterp)));
    CRingCommand* pAddedCommand = new CRingCommand(interp, "ringbuffer");

    return TCL_OK;

 
  }
}

void* gpTCLApplication;
