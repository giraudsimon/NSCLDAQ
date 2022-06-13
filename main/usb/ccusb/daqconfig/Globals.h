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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <tcl.h>


class CConfiguration;
class CCCUSB;
class TclServer;
class CTCLInterpreter;
class CTheApplication;



#include <stdint.h>

/*!
  This namespace defines global variables.  We've tried to keep this to minimum.
  Here's what we define/need
  - pConfig : CConfigurtation*           Will hold the daq configuration 
                                         (adcs and scalers).
  - configurationFilename : std::string  Holds the daq configuration filename
  - controlConfigFilename : std::string  Holds the controllable object configuration 
                                         filename.
  - pUSBController        : CCCUSB*      Points to the VMUSB controller object.
*/

namespace Globals {
  extern CConfiguration* pConfig;
  extern std::string     configurationFilename;
  extern std::string     controlConfigFilename;
  extern CCCUSB*         pUSBController;
  extern uint32_t        bufferSize;
  extern unsigned        scalerPeriod;
  extern unsigned        sourceId;
  extern char*           pTimestampExtractor;
  extern TclServer*      pTclServer;
  extern CTCLInterpreter* pMainInterpreter;
  extern Tcl_ThreadId           mainThread;
  extern CTheApplication*  pApplication;
};

#endif
