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

/**
 * This file is mostly boilerplate that provides a compiled package for C++
 * for clients of the TclPlus library.
 *
 */

#include <tcl.h>
#include <TCLApplication.h>
#include <TCLInterpreter.h>
#include <unistd.h>
#include "CFragmentHandlerCommand.h"
#include "CInputStatsCommand.h"
#include "COrdererOutput.h"
#include "COutputStatsCommand.h"
#include "CDLateStatsCommand.h"
#include "COnLateDataCommand.h"
#include "CBarrierTraceCommand.h"
#include "CSourceCommand.h"
#include "CDeadSourceCommand.h"
#include "CReviveSocketCommand.h"
#include "CFlushCommand.h"
#include "CResetCommand.h"
#include "CBarrierStatsCommand.h"
#include "CDuplicateTimeStatCommand.h"
#include "CConfigure.h"
#include "CXonXOffCallbackCommand.h"
#include "COutOfOrderTraceCommand.h"
#include "CFragmentHandler.h"
#include "BarrierAbortCommand.h"
#include "COutOfOrderStatsCommand.h"

static const char* version = "1.0"; // package version string.

/**
 * Package entry point.  The package is named
 * EvbOrderer that determines the entry point name:
 *
 *
 */
extern "C"
int Eventbuilder_Init(Tcl_Interp* pInterp)
{
  Tcl_PkgProvide(pInterp, "EvbOrderer", version);
  
  // Wrap pInterp in a CTCLInterpretr object and create the command extensions:

  CTCLInterpreter* pInterpObject = new CTCLInterpreter(pInterp);
  
  
  
  new CFragmentHandlerCommand(*pInterpObject, "EVB::handleFragments");
  new CInputStatsCommand(*pInterpObject, "EVB::inputStats");
  new COutputStatsCommand(*pInterpObject, "EVB::outputStats");
  new CDLateStatsCommand(*pInterpObject, "EVB::dlatestats");
  new COnLateDataCommand(*pInterpObject, "EVB::onDataLate");
  new CBarrierTraceCommand(*pInterpObject, "EVB::barriertrace");
  new CSourceCommand(*pInterpObject, "EVB::source"); //  namespace prevents conflict with core source
  new CDeadSourceCommand(*pInterpObject,"EVB::deadsource");
  new CReviveSocketCommand(*pInterpObject, "EVB::reviveSocket");
  new CFlushCommand(*pInterpObject, "EVB::flushqueues");
  new CResetCommand(*pInterpObject, "EVB::reset");
  new CBarrierStatsCommand(*pInterpObject, "EVB::barrierstats"); 
  new CConfigure(*pInterpObject, "EVB::config");
  new CDuplicateTimeStatCommand(*pInterpObject, "EVB::dupstat");
  new CXonXoffCallbackCommand(*pInterpObject, "EVB::onflow");
  new COutOfOrderTraceCommand(*pInterpObject, "EVB::ootrace");
  new CBarrierAbortCommand(*pInterpObject, "EVB::abortbarrier");
  new COutOfOrderStatsCommand(*pInterpObject, "EVB::getoostats");
  // Setup the output stage:

  
  CFragmentHandler* pInstance = CFragmentHandler::getInstance();
  new COrdererOutput(STDOUT_FILENO);

  return TCL_OK;
}
CTCLApplication* gpTCLApplication = 0;
